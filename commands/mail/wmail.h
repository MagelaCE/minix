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
 *		D E F I N I T I O N S
 *
 * Author:	Fred van Kempen, MicroWalt Corporation
 */
#include <sys/types.h>
#include <setjmp.h>


#define VERSION			"2.6 (10/24/89)"

#define DROPNAME 	     "/usr/spool/mail/%s"   /* User Mailbox */
#define LOCKNAME	"/usr/spool/mail/%s.lock"   /* lockfile for box */
#define MAILTEMP		"/tmp/mailXXXXXX"   /* temp. file */
#define COPYTEMP		"/tmp/mcpyXXXXXX"   /* temp update file */
#define SHELL		      		"/bin/sh"
#define RMAIL			 "/usr/bin/rmail"   /* the Internet Mailer */
#define SAVEFILE		     	   "mbox"   /* default mailsave */
#define SIGNATURE		     ".signature"
#define DEADLETTER		    "dead.letter"
#define PROMPT				 "[%d]& "

#ifndef PATHLEN
#	define PATHLEN	128
#endif PATHLEN

#ifndef TRUE
#	define FALSE	  0
#	define TRUE	  1
#endif

#define LOCKWAIT	  5		/* seconds to wait after collision */
#define LOCKTRIES	  4		/* maximum number of collisions */
#define MAXRCPT		100		/* maximum number of recipients */
#define LINELEN		512

#define UNREAD		  1		/* 'not read yet' status */
#define DELETED		  2		/* 'deleted' status */
#define READ		  3		/* 'has been read' status */


typedef struct _letter{
    struct _letter *prev;	/* linked letter list: previous letter */
    struct _letter *next;	/* linked letter list: next letter */
    off_t location;		/* location within mailbox file */
    int status;			/* letter status */
    int seqno;			/* letter number */
    char subject[80];		/* subject of message */
    char sender[512];		/* sender of message */
    char date[32];		/* date of message */
} LETTER;

#define NIL_LET	(LETTER *)NULL
#define NIL	  (char *)NULL


extern char *Version;
extern int remote;			/* use RMAIL to deliver (if any) */
extern int loclink;			/* LMAIL: local delivery only! */
extern int old_uid, old_gid;		/* UID/GID of calling user */
extern int printmode;			/* print-and-exit mode */
extern int immediate;			/* send remote immediately! */
extern int quitmode;			/* take interrupts */
extern int usedrop;			/* read the maildrop (no -f given) */
extern int verbose;			/* pass "-v" flag on to mailer */
extern int needupdate;			/* need to update mailbox */
extern int sayall;			/* include line with all recipients */
extern int checkonly;			/* only chack if there is mail */
extern char sender[PATHLEN];		/* who sent the message? */
extern char forward[PATHLEN];		/* who is the message forwarded to? */
extern char adressee[PATHLEN];		/* current recipient */
extern char recipients[PATHLEN];	/* also to... users */
extern char mailbox[PATHLEN];		/* user's mailbox/maildrop */
extern char subject[PATHLEN];		/* subject of message */
extern char msg_temp[PATHLEN];		/* temporary file */
extern char findbuff[128];
extern char inbuff[512];
extern char lbuff[512];
extern FILE *infp;			/* current message input file */
extern FILE *boxfp;			/* mailbox file */
extern char *progname;			/* program name */
extern jmp_buf printjump;		/* for quitting out of letters */
extern LETTER *firstlet, *lastlet;	/* message pointers for readbox() */
extern int numlet, nextlet, seqno;	/* number of active letters */
extern unsigned oldmask;		/* saved umask() */ 


extern char *getenv();
extern FILE *fdopen();
extern long ftell();


extern void onint(/* void */);
extern char *basename(/* char *name */);
extern char *basepath(/* char *name */);
extern char *whoami(/* void */);
extern void showlet(/* LETTER *let */);
extern void printlet(/* LETTER *let, FILE *tofp */);
extern void printall(/* void */);
extern off_t skiphead(/* LETTER *let */);
extern void dead_letter(/* void */);
extern char *find_string(/* LETTER *let, char *text */);
extern void updatebox(/* void */);
extern void interact(/* void */);
extern void send_remote(/* char *name */);
extern FILE *edit_mail(/* void */);
extern int chk_box(/* void */);
extern int deliver(/* int count, char *vec[] */);
extern int readbox(/* void */);
extern char *xtime(/* void */);			/* the new ctime() */
