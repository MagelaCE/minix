/*
 * UMAIL -	MINIX Remote Domain-adressing Mail Router
 *
 *		This version of RMAIL handles message-headers in a much
 *		more "standard" way. It can handle bang-adresses, plus
 *		the new-style Internet adressing (user@host.domain).
 *		It is called by programs as "Mail" and "Uuxqt".
 *
 * Usage:	umail [-c <config>] [-d] [-i <infile>] [-n] <user> ...
 *
 * Author:	F. van Kempen, Jul-Oct '89 (waltje@minixug.nluug.nl)
 */
#include <stdio.h>
#include <string.h>
#include <alloc.h>
#include <ctype.h>
#include <time.h>
#include <pwd.h>
#include "umail.h"


char *Version = VERSION;	/* UMAIL version ID */
int immediate, debug = FALSE;	/* commandline option flags */
int restrict = FALSE;		/* restricted (UUCP) use only */
int aremote;			/* is the adressee REMOTE or LOCAL ? */
char dfile[128], infile[128];	/* names of message temp-files */
char errmsg[512];		/* global error message */
char mailsender[1024];		/* who sent the message? */
char mailaddr[1024];		/* final routed adress to use. */
char mailhost[64];		/* which host to send to */
char mailcmd[512];		/* command to use to send the mail */
char mailopts[64];		/* which options the mailer uses */
NAME *namelist = NILNAME;	/* list of my network names */
VAR *varlist = NILVAR;		/* list of configuration variables */
HOST *hostlist = NILHOST;	/* list of reacheable host names */
ROUTE *routemap = NILROUTE;	/* list of domain routes */

/* configuration settings */
char *myname = NULL;		/* my UUCP site name */
char *mydomain = NULL;		/* my UUCP domain name */
char *myorg = NULL;		/* Name of my organization */
int oldmailer = FALSE;		/* does our mailer use old From-lines? */
int escape = FALSE;		/* can we offer a routing-escape? */


extern int getopt(), optind;	/* from standard library */
extern char *optarg, *fgets();
extern long ftell();


/* 
 * Convert strings S to upper case. 
 */
char *strupr(s)
char *s;
{
  register char *sp;

  sp = s;
  while (*sp) {
	if (*sp>='a' && *sp<='z') *sp = _toupper(*sp);
  	sp++;
  }
  return(s);
}


/* 
 * Convert strings S to lower case. 
 */
char *strlwr(s)
char *s;
{
  register char *sp;

  sp = s;
  while (*sp) {
	if (*sp>='A' && *sp<='Z') *sp = _tolower(*sp);
  	sp++;
  }
  return(s);
}


/*
 * Add 'NAME' to the list of our names.
 */
void add_name(name)
char *name;
{
  register NAME *np, *xp;

  np = (NAME *) malloc(sizeof(NAME));       /* allocate new variable */
  if (namelist == NILNAME) {                /* first variable */
    	namelist = np;
  } else {
          xp = namelist;
          while (xp->next != NILNAME) xp = xp->next;
          xp->next = np;
    }

  np->next = NILNAME;
  np->name = (char *) malloc(strlen(name) + 2);

  strcpy(np->name, name);
}


/*
 * Add host 'NAME' to the list of hosts.
 */
void add_host(name, smart, cmd, opts)
char *name;
int smart;
char *cmd;
char *opts;
{
  register HOST *hp, *xp;

  hp = (HOST *) malloc(sizeof(HOST));       /* allocate new variable */
  if (hostlist == NILHOST) {                /* first variable */
    	hostlist = hp;
  } else {
          xp = hostlist;
          while (xp->next != NILHOST) xp = xp->next;
          xp->next = hp;
    }

  hp->next = NILHOST;
  hp->name = (char *) malloc(strlen(name) + 2);
  hp->command = (char *) malloc(strlen(cmd) + 2);
  hp->opts = (char *) malloc(strlen(opts) + 2);

  strcpy(hp->name, name);
  strcpy(hp->command, cmd);
  strcpy(hp->opts, opts);
  hp->smart = smart;
}


/*
 * Add route 'DOMAIN' to the routing table.
 */
void add_route(domain, host, route)
char *domain;
char *host;
char *route;
{
  register ROUTE *rp, *xp;

  rp = (ROUTE *) malloc(sizeof(ROUTE));     /* allocate new route */
  if (routemap == NILROUTE) {               /* first route */
	routemap = rp;
  } else {
          xp = routemap;
          while (xp->next != NILROUTE) xp = xp->next;
          xp->next = rp;
    }

  rp->next = NILROUTE;
  rp->domain = (char *) malloc(strlen(domain) + 2);
  rp->host = (char *) malloc(strlen(host) + 2);
  rp->route = (char *) malloc(strlen(route) + 2);

  strcpy(rp->domain, domain);
  strcpy(rp->host, host);
  strcpy(rp->route, route);
}


/*
 * Add variable 'NAME' to the variable list.
 */
void add_var(name, val)
char *name;
char *val;
{
  register VAR *vp, *xp;

  strupr(name);

  vp = (VAR *) malloc(sizeof(VAR));         /* allocate new variable */
  if (varlist == NILVAR) {                  /* first variable */
    	varlist = vp;
  } else {
          xp = varlist;
          while (xp->next != NILVAR) xp = xp->next;
          xp->next = vp;
    }

  vp->next = NILVAR;
  vp->name = (char *) malloc(strlen(name) + 2);
  vp->value = (char *) malloc(strlen(val) + 2);

  strcpy(vp->name, name);
  strcpy(vp->value, val);
}


/*
 * Get a variable from the variable list.
 * Return NULL if not defined.
 */
char *lookup(what)
char *what;
{
  register VAR *vp;

  vp = varlist;
  while (vp != NILVAR) {
    	if (!strcmp(vp->name, what)) return(vp->value);
    	vp = vp->next;
  }
  return(NULL);
}


/*
 * Return TRUE or FALSE value, depending on
 * the value of the given variable.
 */
int boolean(ascii)
char *ascii;
{
  strupr(ascii);
  if (ascii==NULL || !strcmp(ascii, "FALSE")) return(FALSE);
    else if (!strcmp(ascii, "TRUE")) return(TRUE);
           else fprintf(stderr, "Bad value of boolean: \"%s\"\n", ascii);
  return(FALSE);
}


/*
 * Lookup a host in our hosts-table.
 */
HOST *gethost(host)
char *host;
{
  register HOST *hp;

  hp = hostlist;
  while (hp != NILHOST) {
	if (!strcmp(hp->name, host)) return(hp);
	hp = hp->next;
  }
  return(NILHOST);
}


/*
 * Lookup a domain in our domain-table.
 */
ROUTE *getdomain(domain)
char *domain;
{
  register ROUTE *rp;

  rp = routemap;
  while (rp != NILROUTE) {
	if (!strcmp(rp->domain, domain)) return(rp);
	rp = rp->next;
  }
  return(NILROUTE);
}


/*
 * mfgets (modified fgets)
 * Same as fgets() only this version deletes '\n'
 */
char *mfgets(s, n, iop)
char *s;
register int n;
register FILE *iop;
{
  register int c;
  register char *cs;

  cs = s;
  while (--n > 0 && (c = getc(iop)) != EOF) {
	if (c == '\n') {
		*cs = '\0';
		break;
        } else *cs++ = c;
  }
  return((c == EOF && cs == s) ? (char *)NULL : s);
}


/*
 * Return the full UUCP ID of the calling user
 */
char *full_id(user)
char *user;
{
  static char fullbuf[48];

  sprintf(fullbuf, "%s@%s.%s", user, myname, mydomain);
  return(fullbuf);
}


/*
 * Return the Real Name of the calling user
 */
char *realname(who)
char *who;
{
  struct passwd *pw;

  if ((pw = getpwnam(who)) != NULL) return(pw->pw_gecos);
    else return("unknown flag");
}


/*
 * Make a decent DATE/TIME string.
 * Note, that there are TWO possible date formats:
 *
 *	Sat, 12 Oct 89 20:29:00\0
 * and
 *	Sat 12 Oct 20:29:00 1989\0
 *
 * Most Internet mailers use this first form, so we try
 * to this also. We use the function xtime() for the work...
 */
char *maketime(salt)
long *salt;
{
  static char datetime[48];		/* date and time in MET format */
  char *sp;

  sp = lookup("TIMEZONE");		/* get Time Zone from config file */
  if (sp == NULL) sp = "";		/* must have SOME pointer! */

  sprintf(datetime, "%s %s", xtime(salt), sp);

  return(datetime);
}


/*
 * Copy a file from 'inf' to 'outf'.
 */
void fcopy(inf, outf)
register FILE *inf, *outf;
{
  char cpbuff[1024];

  while (TRUE) {
	if (fgets(cpbuff, sizeof(cpbuff), inf) == (char *)NULL) break;
	fwrite(cpbuff, sizeof(char), strlen(cpbuff), outf);
  }
}


/*
 * Load the configuration parameters into their variables.
 */
static void setup(cfg)
char *cfg;
{
  if (scanner(cfg) != 0) {		/* read the configuration file */
          perror(cfg);
          exit(1);
  }

  myname = lookup("SYSTEM");
  if (myname == NULL) {
	fprintf(stderr, "Missing SYSTEM definition\n");
	exit(-1);
  }
  mydomain = lookup("DOMAIN");
  if (mydomain == NULL) {
	fprintf(stderr, "Missing DOMAIN definition\n");
	exit(-1);
  }
  myorg = lookup("ORGANIZATION");
  oldmailer = boolean(lookup("OLDMAILER"));
  escape = boolean(lookup("ESCAPE"));
}


/*
 * Something went wrong.
 * Tell the caller how we should be called!
 */
static void usage()
{
  fprintf(stderr,
	"Usage: umail [-c <config>] [-d] [-i <infile>] [-n] <users>\n");
}


main(argc, argv)
int argc;
char *argv[];
{
  FILE *fdfile, *infp;			/* message file pointers */
  BOX *box;				/* conversion/routing adresses */
  char *cfgfile = CONFIG;		/* config file; to save space */
  register int st;			/* error status, to exit() */
  
  if (argv[0][0] == 'r') {		/* 'rmail' link? */
 	restrict = TRUE;		/* yes, restrict usage */
  }

  while ((st = getopt(argc, argv, "c:di:n")) != EOF) switch(st) {
	case 'c':			/* use non-standard CONFIGH file */
		cfgfile = optarg;
		break;

	case 'd':			/* turn on DEBUG mode */
		debug = TRUE;
		break;
	case 'i':			/* use non-stdin input file */
		strncpy(infile, optarg, 128 - 1);
		break;
	case 'n':			/* call UUCICO after processing */
		immediate = TRUE;
		break;
	default:
		usage();
		exit(1);
  }

  if (optind >= argc) {			/* we need another parameter! */
    	usage();			/* (the adressee ) */
	exit(-1);
  }

  umask(0117);				/* change umask to -rw-rw---- */

  setup(cfgfile);			/* read CONFIG and setup */

  strcpy(dfile, "/tmp/umXXXXXX");	/* create temp. message file */
  mktemp(dfile);
  if ((fdfile = fopen(dfile, "w")) == (FILE *)NULL) {
	perror("rmail 1");
	exit(1);
  }

  box = convert(argv[optind]);		/* convert Internet adress to UUCP */
  if (box == NILBOX) st = FALSE;
    else st = route(box);		/* run it through routing tables */

  if (infile[0] != '\0') {		/* open input file if -i option */
	infp = fopen(infile, "r");
	if (infp == (FILE *)NULL) {
		perror(infile);
		exit(1);
	}
  } else infp = stdin;			/* otherwise use stdin! */

  header(infp, fdfile);			/* analyze message header */

  fcopy(infp, fdfile);			/* copy message to the temp. file */

  fclose(fdfile);
  if (infp != stdin) fclose(infp);

  if (st == FALSE) {			/* conversion/routing went wrong? */ 
	errmail(errmsg, FALSE);		/* yes; return the message! */
	st = -1;
  } else st = sendit(mailaddr, mailhost, mailcmd, mailopts, dfile);

  unlink(dfile);			/* remote data file */
  exit(st);				/* and exit! */
}
