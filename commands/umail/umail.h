/*
 * UMAIL -	MINIX Remote Domain-adressing Mail Router
 *
 *		This version of RMAIL handles message-headers in a much
 *		more "standard" way. It can handle bang-adresses, plus
 *		the new-style Internet adressing (user@host.domain).
 *		It is called by programs as "Mail" and "Uuxqt".
 *
 * 		D E F I N I T I O N S
 *
 * Author:	F. van Kempen, Jul-Oct '89 (waltje@minixug.nluug.nl)
 */

#ifdef MINIX
#	define VERSION         	  "2.5/MINIX"	/* 10/19/89 */
#	define CONFIG		"/usr/lib/uucp/umail.cf"
#endif

#ifdef UNIX
#	define VERSION             "2.5/UNIX"	/* 10/19/89 */
#	define CONFIG		"/usr/lib/uucp/umail.cf"
#endif

#ifndef TRUE
#	define FALSE	0
#	define TRUE	1
#endif

typedef struct __name {
    struct __name *next;
    char *name;
} NAME;

typedef struct __var {
    struct __var *next;
    char *name;
    char *value;
} VAR;

typedef struct __host {
    struct __host *next;
    char *name;
    int smart;
    char *command;
    char *opts;
} HOST;

typedef struct __routemap {
    struct __routemap *next;
    char *domain;
    char *host;
    char *route;
} ROUTE;

typedef struct {
    char user[256];			/* user name of adress */
    char host[256];			/* host name of adress */
    char domain[256];			/* domain name of adress */
} BOX;

#define NILNAME      ((NAME *)NULL)
#define NILVAR        ((VAR *)NULL)
#define NILHOST      ((HOST *)NULL)
#define NILROUTE    ((ROUTE *)NULL)
#define NILBOX	      ((BOX *)NULL)


/* globals in ummain.c */
extern char *Version;			/* UMAIL version ID */
extern int immediate, debug;		/* commandline option flags */
extern int restrict;			/* restricted (UUCP) use only */
extern int aremote;			/* is adressee REMOTE or LOCAL ? */
extern char dfile[], infile[];		/* names of message temp-files */
extern char errmsg[];			/* global error message */
extern char mailsender[];		/* who sent the message? */
extern char mailaddr[];			/* final routed adress to use. */
extern char mailhost[];			/* which host to send to */
extern char mailcmd[];			/* command to use to send the mail */
extern char mailopts[];			/* which options the mailer uses */
extern NAME *namelist;			/* list of my network names */
extern VAR *varlist;			/* list of configuration variables */
extern HOST *hostlist;			/* list of reacheable host names */
extern ROUTE *routemap;			/* list of domain routes */

/* configuration settings */
extern char *myname;			/* my UUCP site name */
extern char *mydomain;			/* my UUCP domain name */
extern char *myorg;			/* Name of my organization */
extern int oldmailer;			/* mailer uses old From-lines? */
extern int escape;			/* routing ESCAPE enable */

/* external routines */
extern char *xtime(/* time_t *salt */);
extern BOX *convert(/* char *adr */);
extern int route(/* BOX *box */);
extern int header(/* FILE *infp, FILE *outfp */);
extern char *strupr(/* char *s */);
extern char *strlwr(/* char *s */);
extern void add_name(/* char *name */);
extern void add_host(/* char *name, int smart, char *cmd, char *opts */);
extern void add_route(/* char *domain, char *host, char *route */);
extern void add_var(/* char *name, char *val */);
extern char *lookup(/* char *what */);
extern int boolean(/* char *ascii */);
extern HOST *gethost(/* char *host */);
extern ROUTE *getdomain(/* char *domain */);
extern char *mfgets(/* char *s, int n, FILE *iop */);
extern char *whoami(/* void */);
extern char *full_id(/* char *user */);
extern char *realname(/* char *who */);
extern char *maketime(/* long *salt */);
extern void fcopy(/* FILE *inf, FILE *outf */);
extern int scanner(/* char *fname */);
extern int KnowHost(/* char *name */);
extern int islocal(/* char *name */);
extern char *genname(/* int prefix, int grade, char *sysname */);
extern int send_local(/* char *user, char *data */);
extern int send_remote(/* char *rmtname, char *rmtuser, char *data */);
extern void errmail(/* char *str, int mgronly */);
extern int sendit(/* char *who, char *host, char *cmd, char *opts, char *data */);
