/*
 * UMAIL -	MINIX Remote Domain-adressing Mail Router
 *
 *		This version of RMAIL handles message-headers in a much
 *		more "standard" way. It can handle bang-adresses, plus
 *		the new-style Internet adressing (user@host.domain).
 *		It is called by programs as "Mail" and "Uuxqt".
 *
 * 		M A I L   T R A N S P O R T   M O D U L E
 *
 * Author:	F. van Kempen, Jul-Oct '89 (waltje@minixug.nluug.nl)
 */
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <uucp.h>
#include "umail.h"


/*
 * Check the host machine name.
 *
 * returns FALSE if not found, else TRUE
 */
int KnowHost(name)
char *name;
{
  register HOST *hp;

  hp = gethost(name);
  return((hp==NILHOST) ? FALSE : TRUE);
}		


/*
 * Check if this is one of our local names.
 * Return 1 if TRUE, or 0 if FALSE.
 */
int islocal(name)
char *name;
{
  register NAME *np;

  np = namelist;
  while (np != NILNAME) {
	if (!strcmp(np->name, name)) return(TRUE);
	np = np->next;
  }
  return(FALSE);
}


/*
 * Creates a unique UUCP file name, and returns a pointer
 * to it. The filename is a combination of prefix, grade, system name
 * and a sequential number taken from SPOOLSEQ.
 */
char *genname(prefix, grade, sysname)
int prefix, grade;
char *sysname;
{
  static char _gen_buf[128];
  int i = 0;
  char seqline[10];
  char *seqname = SPOOLSEQ;		/* to save some string space */
  char *lseqname = LSPOOLSEQ;		/* to save some string space */
  FILE *spoolseq;

  if (access(seqname, 0) != 0) close(creat(seqname, 0600));

  while(link(seqname, lseqname) != 0) {
  	sleep(5);
  	if (++i > 5) return(NULL);
  }

  spoolseq = fopen(seqname, "r");
  fgets(seqline, sizeof(seqline), spoolseq);
  fclose(spoolseq);
  unlink(lseqname);

  i = (atoi(seqline) + 1);

  if ((spoolseq = fopen(seqname, "w")) == (FILE *)NULL) return(NULL);
  fprintf(spoolseq, "%d\n", i);
  fclose(spoolseq);

  if (grade == 0) sprintf(_gen_buf, "%c.%.7s%04.4x", prefix, sysname, i);
    else sprintf(_gen_buf, "%c.%.7s%c%04.4x", prefix, sysname, grade, i);

  return(_gen_buf);
}


/*
 * Deliver this message to a local user.
 * We do this by calling "LMAIL" (which is actually
 * a link to "Mail"; the Local Mail Agent.
 */
int send_local(user, data)
char *user;
char *data;
{
  struct passwd *pw;
  char tmpbuf[128];

  /* See if destination user name exists on this machine */
  pw = (struct passwd *) getpwnam(user);
  if (pw == (struct passwd *)NULL) {
	sprintf(tmpbuf, "%s ... unknown user at %s", user, myname);
	errmail(tmpbuf, FALSE);
  }

#ifdef WMAILER	/* faster than redirecting! */
  sprintf(tmpbuf, "exec %s -i%s %s", LMAIL, data, user);
#else
  sprintf(tmpbuf, "exec %s <%s %s", LMAIL, data, user);
#endif WMAILER

  return(system(tmpbuf));
}


/*
 * Deliver this message to a remote user.
 * We do this by creating the spoolfiles needed by UUCICO.
 * Then we call that program daemon to do the real work.
 */
int send_remote(rmtname, rmtuser, data)
char *rmtname;
char *rmtuser;
char *data;
{
  char tmpbuf[128];
  char Bfile[128], Cfile[128], Dfile[128], Xfile[128];
  FILE *fcfile, *fbfile, *fdfile, *fp;

  if (KnowHost(rmtname) == FALSE) {
	sprintf(tmpbuf, "%s ... unknown host machine", rmtname);
	errmail(tmpbuf, FALSE);
  }

  /* make the spool files for uucico */
  strcpy(Bfile, genname('B', 0, rmtname));
  strcpy(Cfile, genname('C', 'N', rmtname));
  strcpy(Dfile, genname('D', 0, myname));
  strcpy(Xfile, genname('X', 'N', rmtname));

  /* Copy the temp-file to the UUCP data file (D.???) */
  if ((fdfile = fopen(Dfile, "w")) == (FILE *)NULL) {
	perror("rmail 4");
      	exit(1);
  } else {
	  fp = fopen(data, "r");	/* open temp-file */
	  fcopy(fp, fdfile);
	  fclose(fdfile);
	  fclose(fp);
    }
    
  if ((fbfile = fopen(Bfile, "w")) == (FILE *)NULL) {
	perror("rmail 4");
	exit(1);
  } else {
	  fprintf(fbfile, "U %s %s\nF %s\nI %s\nC rmail %s\n",
    				UUCPUSER, myname, Dfile, Dfile, rmtuser);
    	  fclose(fbfile);
    }

  if ((fcfile = fopen(Cfile, "w")) == (FILE *)NULL) {
	perror("rmail 5");
	exit(1);
  } else {
	  fprintf(fcfile,"S %s %s %s - %s 0666\nS %s %s %s - %s 0666\n",
		Dfile, Dfile, UUCPUSER, Dfile, Bfile, Xfile, UUCPUSER, Bfile);
          fclose(fcfile);
    }

  /* RMAIL is setUID root... UUCP cannot read these files! */
  chown(Bfile, UUCPUID, UUCPGID);
  chown(Cfile, UUCPUID, UUCPGID);
  chown(Dfile, UUCPUID, UUCPGID);
  chown(Xfile, UUCPUID, UUCPGID);

  if (immediate == TRUE) {	/* call uucico now! */
	strcpy(tmpbuf, UUCICO);
	sprintf(tmpbuf, "exec %s -s%s -x1 >/dev/null &", UUCICO, rmtname);
	system(tmpbuf);
  }

  return(FALSE);
}


/*
 * Perform the mail-transport.
 * We do this by calling the appropriate mailer.
 * If the name of the mailer is "$$" then we can use
 * this program to deliver. This saves a lot of memory.
 */
int sendit(who, host, cmd, opts, data)
char *who;			/* who is the adressee? */
char *host;			/* on which machine? */
char *cmd;			/* what command should we use? */
char *opts;			/* which options? */
char *data;			/* name of data (message) file */
{
  char cmdline[512];
  char tmpbuff[512];

  chdir(SPOOLDIR); 			/* Change to UUCP directory */

  if (!strcmp(cmd, "$$")) {		/* run our own mail routines */
	if (*host == '\0') send_local(who, data);
          else send_remote(host, who, data);
  } else {
	  sprintf(tmpbuff, "exec %s %s ", cmd, opts);
	  sprintf(cmdline, tmpbuff, data);	/* create commandline */
	  strcat(cmdline, who);			/* add user adress */
	  system(cmdline);			/* execute command (mailer) */
    }
}


/*
 * Send mail to system manager upon errors
 *
 * Mail is contained in a file referenced
 * by Global variable 'dfile'
 */
void errmail(str, mgronly)
char *str;
int mgronly;
{
  FILE *fp, *tp;
  long now;
  char fname[32];
  char tmp[128];

  strcpy(fname, "/tmp/umeXXXXXX");
  mktemp(fname);

  tp = fopen(fname, "w");
  fp = fopen(dfile, "r");

  time(&now);

  /* create header of the report-message */
  fprintf(tp, "From %s %s\n", ERRUSER, xtime(&now));	
  if (mailsender != NULL) fprintf(tp, "To: %s\n", mailsender);
  fprintf(tp, "Subject: Returned mail\n\n");

  /* create an error transcript */
  fprintf(tp, "   ---- Transcript of session follows ----\n\n");
  fprintf(tp, "%s\n", str);
  fprintf(tp, "\n   ---- Unsent message follows ----\n");

  /* copy the message */
  while (mfgets(tmp, sizeof(tmp), fp) != (char *)NULL)
				fprintf(tp, "> %s\n", tmp);

  fclose(tp);	/* flush and close message file */
  fclose(fp);	/* flush and close orig. file */

  /* Return mail to system manager (and sender if mgronly == FALSE) */
  if (mgronly == FALSE) sendit(mailsender, "", RMAIL, " <%s", fname);

  /* send mail to UUCP administrator */
  sendit(ERRUSER, "", "$$", "", fname);

  unlink(fname);	/* remove data files */
  unlink(dfile);

  exit(1);		/* and exit! */
}
