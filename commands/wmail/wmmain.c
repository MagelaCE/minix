/*
 * WMAIL -	MicroWalt Extended Mail Agent.
 *		This is the MicroWalt Mail Agent; which is derived
 *		from  the  "Mini-Mail" written by Peter S. Housel.
 *		This version  has a better  user-interface, and it
 *		also "knows" about things like forwarding, replies
 *		etc. Overall, it looks like the Mail (uppercase M)
 *		on our local DYNIX(tm) (==BSD) system...
 *		The paging-code (for paging letters on the screen)
 *		was taken from "more.c", by Brandon Allbery.
 *
 * Author:	Fred van Kempen, MicroWalt Corporation
 *
 * To Do:
 */
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include "wmail.h"


char *Version = VERSION;
int remote = FALSE;			/* use RMAIL to deliver (if any) */
int loclink = FALSE;			/* LMAIL: local delivery only! */
int old_uid, old_gid;			/* UID/GID of calling user */
int printmode = FALSE;			/* print-and-exit mode */
int immediate = FALSE;			/* send remote immediately! */
int quitmode = FALSE;			/* take interrupts */
int usedrop = TRUE;			/* read the maildrop (no -f given) */
int verbose = FALSE;			/* pass "-v" flag on to mailer */
int needupdate = FALSE;			/* need to update mailbox */
int sayall = FALSE;			/* include line with all recipients */
int checkonly = FALSE;			/* only chack if there is mail */
char sender[PATHLEN];			/* who sent the message? */
char forward[PATHLEN];			/* who is the message forwarded to? */
char adressee[PATHLEN];			/* current recipient */
char recipients[PATHLEN];		/* also to... users */
char mailbox[PATHLEN];			/* user's mailbox/maildrop */
char subject[PATHLEN];			/* subject of message */
char msg_temp[PATHLEN];			/* temporary file */
char findbuff[128];
char inbuff[512];
char lbuff[512];
FILE *infp = (FILE *)NULL;		/* current message input file */
FILE *boxfp = (FILE *)NULL;		/* mailbox file */
char *progname;				/* program name */
jmp_buf printjump;			/* for quitting out of letters */
LETTER *firstlet, *lastlet;		/* message pointers for readbox() */
int numlet, nextlet, seqno;		/* number of active letters */
unsigned oldmask;			/* saved umask() */ 


extern char *optarg;			/* from the GETOPT(3) package */
extern int getopt(), optind;
extern int errno;			/* from STDIO */


void onint(void)
{
  longjmp(printjump, 1);
}


/*
 * Chop off the last (file) part of a filename.
 */
char *basename(name)
char *name;
{
  char *p;
   char *strrchr();

  p = strrchr(name, '/');
  if (p == NIL) return(name);	/* no pathname */
    else return(p + 1);
}


/*
 * Chop off the last (user) part of a UUCP path-name.
 * This is needed for the summary() routine.
 */
char *basepath(name)
char *name;
{
  char *p;
   char *strrchr();

  p = strrchr(name, '!');
  if (p == NIL) return(name);	/* no pathname */
    else return(p + 1);
}


/*
 * return ASCII text of our login-name.
 */
char *whoami(void)
{
  struct passwd *pw;

  if ((pw = getpwuid(getuid())) != (struct passwd *)NULL)
						return(pw->pw_name);
    else return("nobody");
}


/*
 * Find the given antry in the mail-header
 * Search for the first occurence of string 'text' in the header.
 * Copy the text following it into the 'let' structure.
 * Return buffer if found, else NULL.
 */
char *find_string(let, text)
LETTER *let;
char *text;
{
  int all;
  off_t curr, limit;
  register char *sp;

  fseek(boxfp, let->location, 0);
  limit = (off_t) -1L;
  if (let->next != NIL_LET) limit = let->next->location;

  all = 0;
  curr = let->location;
  while (curr != limit && !all) {
	if (fgets(inbuff, sizeof(inbuff), boxfp) == NIL) all = 1;
      	if (inbuff[0] == '\0') all = 1; /* end-of-header */

      	if (!strncmp(inbuff, text, strlen(text))) {
		sp = &inbuff[0];		/* remove '\n' */
      		while (*sp && *sp!='\n') sp++;
      		*sp = '\0';
		sp = &inbuff[0] + strlen(text);	/* copy to static buff */
		strcpy(findbuff, sp);
		return(findbuff);		/* return adress of buff */
       	}

	curr += (off_t) strlen(inbuff);		/* update message offset */

	if (!all && limit > 0L)			/* quit if past message */
		if (curr >= limit) all = 1;
  }
  return(NIL);
}


/*
 * Check is the first line of the mailbox contains a line like
 *
 *	Forward to XXXX
 *
 * then all mail for the calling user is being forwarded
 * to user XXXX. Return a 1 value if this is the case.
 * Otherwise, return 0 (or -1 for error).
 */
int chk_box(void)
{
  char xbuf[128];
  char *bp;
  FILE *fp;

  if (access(mailbox, 4) < 0 || 
    (fp = fopen(mailbox, "r")) == (FILE *)NULL) {
	if (usedrop && errno==ENOENT) return(-1);
     	fprintf(stderr, "%s: cannot access mailbox ", progname);
      	perror(mailbox);
      	exit(1);
  }

  bp = fgets(xbuf, sizeof(xbuf), fp);
  fclose(fp);

  if (bp!=NIL && !strncmp(xbuf, "Forward to ", 11)) {
	strcpy(forward, strrchr(xbuf, ' ') + 1);	/* get username */
	forward[strlen(forward)-1] = '\0';		/* remove \n */
	return(1);
  }
  return(0);
}


/* 
 * Read the contents of the Mail-Box into memory.
 */
int readbox(void)
{
  register LETTER *let;
  register char *sp, *bp;
  off_t current;
 
  firstlet = lastlet = NIL_LET;
  numlet = 0;
  seqno = 1;

  if (chk_box() == 1) return(1);	/* mail is being forwarded... */

  if ((boxfp = fopen(mailbox, "r")) == (FILE *)NULL) {
	if (usedrop && errno==ENOENT) return(-1);
      	fprintf(stderr, "%s: cannot access mailbox ", progname);
      	perror(mailbox);
      	exit(1);
  }

  /*
   * Determine where all messages start.
   * This should be done with an index file in the future!
   */
  current = 0L;
  while(fgets(lbuff, sizeof(lbuff), boxfp) != NIL) {
	current = ftell(boxfp);
      	if (!strncmp(lbuff, "From ", 5)) {
		if ((let = (LETTER *)malloc(sizeof(LETTER))) == NIL_LET) {
			fprintf(stderr, "%s: out of memory.\n", progname);
			exit(1);
	 	}
        	if (lastlet == NIL_LET) {
			firstlet = let;
			let->prev = NIL_LET;
	 	} else {
	 		let->prev = lastlet;
	     	 	lastlet->next = let;
	    	  }
		lastlet = let;
		let->next = NIL_LET;

		let->status = UNREAD;
		let->location = current - (off_t) strlen(lbuff);
		let->seqno = seqno++;
		numlet++;
	}
  }

  /*
   * We now know where the messages are, read message headers.
   */
  let = firstlet;
  while (let != NIL_LET) {
	sp = find_string(let, "From ");
	if (sp == NIL) sp = "<unknown>";	/* should never occur! */
	strncpy(let->sender, sp, 511);
	sp = let->sender;
	while (*sp && *sp!=' ') sp++;
	*sp = '\0';
	
	sp = find_string(let, "Subject: ");
	if (sp == NIL) sp = "<none>";
	strncpy(let->subject, sp, 79);

	sp = find_string(let, "From ");
	while (*sp && *sp!=' ') sp++;
	sp++;
	strncpy(let->date, sp, 31);
	
	let = let->next;
  }
}


/*
 * Check if there is any mail for the calling user.
 * Return 0 if there is mail, or 1 for NO MAIL.
 */
static int chk_mail(void)
{
  FILE *fp;
  char temp[512];

  if ((fp = fopen(mailbox, "r")) != (FILE *)NULL) {
	if (fgets(temp, sizeof(temp), fp) == NIL) {
		fclose(fp);	/* empty mailbox, no mail! */
		return(1);
	}
      	if (!strncmp(temp, "Forward to ", 11)) {
		fclose(fp);	/* FORWARD line in mailbox */
		return(2);	/* so no mail. */
	}
	fclose(fp);	/* another line, so we have mail! */
	return(0);
  }
  return(1);
}


static void usage(void)
{
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "\t%s [-epqrv] [-f file]\n", progname);
  fprintf(stderr, "\t%s [-dtv] [-i file] [-s subject] user ...\n", progname);
  fprintf(stderr, "\t%s [-lv] [-i file] user\n\n", progname);
}


main(argc, argv)
int argc; char *argv[];
{
  int c, st;

  strcpy(sender, whoami());
  old_uid = getuid();			/* get calling user and save */
  old_gid = getgid();
  setuid(geteuid());			/* set UID to ROOT (SU) */
  setgid(getegid());			/* set GID to ROOT (SU) */

  progname = basename(argv[0]);		/* how are we called? */
  if (*progname == 'l') {
	remote = 0;			/* 'lmail' link? */
	loclink = 1;
  }
  strcpy(msg_temp, MAILTEMP);
  mktemp(msg_temp);			/* name the temp file */
  oldmask = umask(077);			/* change umask for security */
  infp = stdin;				/* set INPUT to stdin */
  
  while ((c = getopt(argc, argv, "def:i:lpqrs:tv")) != EOF) switch(c) {
	case 'd':
		immediate++;
		break;
	case 'e':
		checkonly++;
		break;
	case 'f':
		usedrop = 0;
		strncpy(mailbox, optarg, PATHLEN - 1);
		break;
	case 'i':
		infp = fopen(optarg, "r");
		if (infp == (FILE *)NULL) {
			fprintf(stderr,
				"%s: cannot open %s\n", progname, optarg);
			exit(-1);
		}
		break;
	case 'l':
		loclink = 1;
		break;
	case 'p':
		printmode++;
		break;
	case 'q':
		quitmode++;
		break;
	case 'r':
		break;		/* no reverse order supported! */
	case 's':
		strcpy(subject, optarg);
		break;
	case 't':
		sayall++;
		break;
	case 'v':
		verbose++;
		break;
	default:
		usage();
		exit(2);
  }

  if (optind >= argc) {
	  if (usedrop) sprintf(mailbox, DROPNAME, sender);

	  if (checkonly) {
		st = chk_mail();
		exit(st);
	  }

	  if ((st = readbox()) == 1) {
		fprintf(stderr, "Your mail is being forwarded to %s.\n",
								forward);
		exit(1);
	  } else {
		  st = 0;
	 	  if (printmode) printall();
		    else interact();

		  if (needupdate) updatebox();
	    }
    } else st = deliver(argc - optind, argv + optind);

  unlink(msg_temp);
  exit(st);    
}
