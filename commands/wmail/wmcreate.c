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
 *		M A I L    G E N E R A T I O N    M O D U L E
 *
 * Author:	Fred van Kempen, MicroWalt Corporation
 */
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include "wmail.h"


/*
 * Create a message, including the Mail-header and the final signature.
 * If this is a link to LMAIL and/or if the message is read from a file
 * leave out the signature.
 * This routine copies lines of text from 'stdin' to 'tempfp', until an
 * EOF is typed, or a line containing only a '.'.
 * We complicate things by not setting a line limit.
 * Define V7MAIL to skip the "To:"-field.
 */
FILE *edit_mail(void)
{
  register FILE *tempfp, *sigfp;	/* temp file used for copy */
  register char *sp, *bp;
  register int c;
  char cpbuff[1024];			/* copy buffer */
  char tmp[PATHLEN];
  int state, done;

  if ((tempfp = fopen(msg_temp, "w")) == (FILE *)NULL) {
	fprintf(stderr, "%s: cannot create temp file \"%s\"\n",
							progname, msg_temp);
	return((FILE *)NULL);
  }

  /* 
   * Create the header. This has the form:
   *
   *  From <user> <date>
   *  Subject: <text>	(if -s on command line or stdin)
   *  To: <userlist>	(all recipients)
   */
  if (loclink == FALSE) {	/* only create header if not LMAIL! */
	fprintf(tempfp, "From %s %s\n", sender, xtime());
	if (subject[0]=='\0') {
		if (isatty(fileno(infp))) {
			fprintf(stderr, "Subject: ");
			gets(subject);
       			sp = strrchr(subject, '\n');
			if (sp != NIL) *sp = '\0';
			fprintf(stderr, "\n");
       		}
	}
#ifndef V7MAIL
	fprintf(tempfp, "To: %s\n", recipients);
#endif
	if (subject[0] != '\0') fprintf(tempfp, "Subject: %s\n", subject);

	fputc('\n', tempfp);
	done = state = 0;
	do {
      	    if ((c = fgetc(infp)) != EOF) {	
		if (c=='.' && state==1) {
			done = 1;
		} else {
			if (c == '\n') {
				state = 1;
		    	} else state = 0;
		  }
    		if (done == 0) fputc(c, tempfp);
      	    } else done = 1;
  	} while (done == 0);

	/*
	 * Add a .signature file after the message
	 * Skip this if infp is a file.
	 */
	if (isatty(fileno(infp))) {
		sp = getenv("SIGNATURE");
		if (sp == NIL) {
			sp = getenv("HOME");
	      		if (sp != NIL) {
                		strcpy(tmp, sp);
                		if (strlen(tmp) > 1) strcat(tmp, "/");
               		} else strcpy(tmp, "");
              		strcat(tmp, SIGNATURE);
	 	} else strcpy(tmp, sp);

		if ((sigfp = fopen(tmp, "r")) != (FILE *)NULL) {
			while ((c = fgetc(sigfp)) != EOF) fputc(c, tempfp);
	        	fclose(sigfp);
		}
  	}
  } else {	/* fast file copy for local delivery */
	  /* copy temp. file to mailbox */
	  while (TRUE) {
		if (fgets(cpbuff, sizeof(cpbuff), infp) == (char *)NULL)
								      break;
		fwrite(cpbuff, sizeof(char), strlen(cpbuff), tempfp);
	  }
    }

  if (ferror(tempfp) || fclose(tempfp)) {
	fprintf(stderr, "%s: could not copy letter to temporary file\n",
								progname);
      	return((FILE *)NULL);
  }

  fclose(tempfp);
  tempfp = fopen(msg_temp, "r");
  return(tempfp);
}

