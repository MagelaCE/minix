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
 *		L O C A L    D E L I V E R Y    M O D U L E
 *
 * Author:	Fred van Kempen, MicroWalt Corporation
 *
 * To Do:
 *		- Aliases.
 */
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include "wmail.h"


extern int errno;


/*
 * Update the mail-box file.
 */
void updatebox(void)
{
  char cpbuff[1024];		/* copy buffer */
  FILE *copyfp;			/* fp for tempfile */
  char lockname[PATHLEN];	/* maildrop lock */
  char copytemp[PATHLEN];	/* temporary copy file */
  int locktries = 0;		/* tries when box is locked */
  LETTER *let;			/* current letter */
  int c;

  strcpy(copytemp, COPYTEMP);
  mktemp(copytemp);

  sprintf(lockname, LOCKNAME, sender);
  
  /* create a new mailbox-file */
  if ((copyfp = fopen(copytemp, "w")) == (FILE *)NULL) {
	fprintf(stderr, "%s: cannot create temp file \"%s\"\n",
						progname, copytemp);
	return;
  }
    
  /* copy letters from old file to new file */
  for (let = firstlet; let != NIL_LET; let = let->next) {
	if (let->status != DELETED) printlet(let, copyfp);
  }

  if ((copyfp = freopen(copytemp, "r", copyfp)) == (FILE *)NULL) {
	sprintf(cpbuff, "%s: temporary file write error", progname);
	perror(cpbuff);
	if (usedrop) unlink(copytemp); 
	return;
  }

  /* shut off signals during the update */
  signal(SIGINT, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);

  if (usedrop) while(link(mailbox, lockname) != 0) {
	if (++locktries >= LOCKTRIES) {
		fprintf(stderr, "%s: could not lock maildrop for update\n",
								progname);
		return;
	}
	sleep(LOCKWAIT);
  }

  if ((boxfp = freopen(mailbox, "w", boxfp)) == (FILE *)NULL) {
	sprintf(cpbuff, "%s: could not reopen maildrop", progname);
       	fprintf(stderr, "%s\nMail may have been lost; look in %s\n",
							cpbuff, copytemp);
       	unlink(lockname);
       	return;
  }

  /* copy temp. file to mailbox */
  while (TRUE) {
	if (fgets(cpbuff, sizeof(cpbuff), copyfp) == (char *)NULL) break;
	fwrite(cpbuff, sizeof(char), strlen(cpbuff), boxfp);
  }
  fflush(copyfp); fflush(boxfp);
  fclose(boxfp); fclose(copyfp);
  unlink(copytemp); 

  if (usedrop) unlink(lockname);
}


/*
 * Send a message to a remote user.
 * Define UMAILER if your mailer knows about the "-i datafile" option.
 */
void send_remote(name)
char *name;
{
  char cmdbuff[128];

#ifdef UMAILER		/* faster than redirecting! */
  sprintf(cmdbuff, "exec %s -i%s %s %s %s >/dev/null", RMAIL, msg_temp,
#else
  sprintf(cmdbuff, "exec %s <%s %s %s %s >/dev/null", RMAIL, msg_temp,
#endif UMAILER
	immediate ? "-n" : "", verbose ? "-v" : "", adressee);
  system(cmdbuff);				/* call the mailer! */
}


/*
 * Deliver a message to a user.
 * First of all, check if we can use a more intelligent mailer
 * for this job. If not, deliver it !
 */
int deliver(count, vec)
int count; char *vec[];
{
  char cpbuff[1024];			/* copy buffer */
  register int i, c;
  int errs = 0;				/* count of errors */
  int dropfd;				/* file descriptor for user's drop */
  int created = 0;			/* true if we created the maildrop */
  FILE *mailfp;				/* fp for mail */
  struct stat stb;			/* for checking drop modes, owners */
  int (*sigint)(), (*sighup)(), (*sigquit)();	/* saving signal state */
  char lockname[PATHLEN];		/* maildrop lock */
  int locktries;			/* tries when box is locked */
  struct passwd *pw;			/* sender and recipent */

  if (count > MAXRCPT) {
	fprintf(stderr, "%s: too many recipients\n", progname);
	return(-1);
  }

  strcpy(recipients, "");
  if (sayall) {
	for (i=0; i<count; i++) {
		strcat(recipients, vec[i]);
		/* RFC-822: separate with comma */
		strcat(recipients, ",");
  	}
  	recipients[strlen(recipients)-1] = '\0';  /* kill last comma */
  } else strcat(recipients, vec[0]);     

  mailfp = edit_mail();	/* input the message */

  /* shut off signals during the delivery */
  sigint = signal(SIGINT, SIG_IGN);
  sighup = signal(SIGHUP, SIG_IGN);
  sigquit = signal(SIGQUIT, SIG_IGN);

  /*
   * We have the message, now deliver it to all recipients!
   * Do this on a per-user basis!
   */
  for (i = 0; i < count; ++i) {
	if (count > 1) rewind(mailfp);		/* rewind data-file */
	strcpy(adressee, vec[i]); 		/* get name of adressee */
	forward[0] = '\0';			/* Clear 'Forward' flag */

      	/* OK, 'adressee' is the recipient. Check if local. */
      	if (strchr(adressee, '!') || strchr(adressee, '@')) {
		remote = 1;		/* the guy is remote... */
        	send_remote(adressee);	/* call RMAIL or SMAIL for this... */
		continue;
       	}

	/* Hmm, it is a local user. Do we know him? */
      	if ((pw = getpwnam(adressee)) == (struct passwd *)NULL) {
		fprintf(stderr, "%s: user %s unknown\n", progname, adressee);
		++errs;
		dead_letter();
		continue;
       	} else {
	   	sprintf(mailbox, DROPNAME, adressee);
 	       	sprintf(lockname, LOCKNAME, adressee);
	  }

	/* OK, we know him. Check if we have to forward his mail. */
      	if (chk_box() == 1) {		/* forward messages to 'forward' */
		strcpy(adressee, forward);

        	/* We now have the final adressee.
		 * Check for remote users again.
		 */
		if (strchr(adressee, '!') || strchr(adressee, '@')) {
			remote = 1;
          		send_remote(adressee);	/* No, call RMAIL/SMAIL. */
	  		continue;
         	} else {	/* Check if this guy is known. */
		       	if ((pw = getpwnam(adressee)) ==
						(struct passwd *)NULL) {
		   		fprintf(stderr,
					"%s: forward-user %s unknown\n",
							progname, adressee);
		   		++errs;
				dead_letter();
		   		continue;
	          	} else {
		         	sprintf(mailbox, DROPNAME, adressee);
	 	          	sprintf(lockname, LOCKNAME, adressee);
		          }
		  }
	}

      	/*
       	 * We now have a local user 'adressee' who exists.
         * Lock the maildrop while we're messing with it. Races are possible
         * (though not very likely) when we have to create the maildrop, but
         * not otherwise. If the box is already locked, wait awhile and try
         * again.
         */
      	locktries = created = 0;
trylock:
      	if (link(mailbox, lockname) != 0) {
		if (errno == ENOENT) {	/* user doesn't have a drop yet */
			if ((dropfd = creat(mailbox, 0600)) < 0) {
				fprintf(stderr,
				 "%s: could not create maildrop for %s\n",
		      					  progname, vec[i]);
				++errs;
				dead_letter();
				continue;
          		}
	  		++created;
	  		goto trylock;
         	} else {   /* somebody else has it locked, it seems - wait */
	  	 	if (++locktries >= LOCKTRIES) {	
				fprintf(stderr,
				  "%s: could not lock maildrop for %s\n",
		      					progname, vec[i]);
				++errs;
				dead_letter();
	      	   		continue;
	     	  	}
	   	 	sleep(LOCKWAIT);
	   	 	goto trylock;
	  	  }
	}

      	if (created) {
        	chown(mailbox, pw->pw_uid, pw->pw_gid);
        	boxfp = fdopen(dropfd, "a");
       	} else boxfp = fopen(mailbox, "a");

      	if (boxfp==(FILE *)NULL || stat(mailbox, &stb) < 0) {
		fprintf(stderr, "%s: serious maildrop problems for %s\n",
							progname, vec[i]);
        	unlink(lockname);
        	++errs;
		dead_letter();
        	continue;
       	}

      	if (stb.st_uid != pw->pw_uid || (stb.st_mode & S_IFMT) != S_IFREG) {
		fprintf(stderr, "%s: mailbox for user %s is illegal\n",
							progname, vec[i]);
		unlink(lockname);
		++errs;
		dead_letter();
		continue;
       	}

	/* copy temp. file to mailbox */
	while (TRUE) {
		if (fgets(cpbuff, sizeof(cpbuff), mailfp) == (char *)NULL)
								      break;
		fwrite(cpbuff, sizeof(char), strlen(cpbuff), boxfp);
	}

	/* to make sure! */
	fputc('\n', boxfp);
      	if (ferror(boxfp) || fclose(boxfp) != 0) {
		fprintf(stderr, "%s: error delivering to user %s",
							progname, vec[i]);
		perror("");
		dead_letter();
		++errs;
       	}

      	unlink(lockname);
  }

  fclose(mailfp);

  /* put signals back the way they were */
  signal(SIGINT, sigint);
  signal(SIGHUP, sighup);
  signal(SIGQUIT, sigquit);

  return (errs == 0) ? 0 : -1;
}


/* 
 * Save the current message to file 'dead.letter'.
 * This is sometimes needed (when delivery fails!).
 */
void dead_letter(void)
{
  char cpbuff[1024];
  char fname[PATHLEN];
  char *sp;
  register FILE *inf, *outf;

  fname[0] = '\0';
  inf = fopen(msg_temp, "r");
  if ((sp = getenv("HOME")) != NULL) {
	strcpy(fname, sp);
	if (strlen(sp) > 1)
		strcat(fname, "/");
  }
  strcat(fname, DEADLETTER);

  inf = fopen(msg_temp, "r");
  if (inf == (FILE *)NULL) {
	fprintf(stderr, "%s: cannot open \"%s\"\n", progname, msg_temp);
	return;
  }

  outf = fopen(fname, "w");
  if (outf == (FILE *)NULL) {
	fprintf(stderr, "%s: cannot create \"%s\"\n", progname, fname);
	return;
  }

  /* copy temp. file to dead.letter */
  while (TRUE) {
	if (fgets(cpbuff, sizeof(cpbuff), inf) == (char *)NULL) break;
	fwrite(cpbuff, sizeof(char), strlen(cpbuff), outf);
  }

  fclose(inf);
  fclose(outf);

  chown(fname, old_uid, old_gid);
  fprintf(stderr, "%s: dumped message on file \"%s\"\n", progname, fname);
}
