/*
 * UMAIL -	MINIX Remote Domain-adressing Mail Router
 *
 *		This version of RMAIL handles message-headers in a much
 *		more "standard" way. It can handle bang-adresses, plus
 *		the new-style Internet adressing (user@host.domain).
 *		It is called by programs as "Mail" and "Uuxqt".
 *
 *	    	M A I L   H E A D E R   C O N V E R S I O N   M O D U L E
 *
 * Headers:	This module reads the header part of a message into
 *		memory, and then rearranges it into RFC-822 order.
 *		The preferred order of fields is:
 *
 *		1.  From (old-style V6/V7)
 *		2.  From:
 *		3.  Received:	(top to bottom)
 *		4.  Sender:
 *
 *		5.  Unknown fields (user-defined)
 *
 *		6.  To:
 *		7.  Cc:
 *		8.  Bcc:
 *		9.  Subject:
 *		10. Message-ID:
 *		11. Date:
 *
 *		This order may be changed and/or expanded in the future,
 *		especially the "Resent-" fields should be added.
 *
 * Author:	F. van Kempen, Jul-Oct '89 (waltje@minixug.nluug.nl)
 */
#include <stdio.h>
#include <alloc.h>
#include <string.h>
#include <time.h>
#include "umail.h"


typedef struct __hdr {
	struct __hdr *next;
	int done;		/* 1==READ, 0==UNREAD */
	int count;		/* 1==FIRST, 2.. == NEXT PART */
	int std;		/* 1==RFC, 0==USER_DEFINED */
	char *name;		/* field name */
	char *value;		/* field value */
} HEADER;
#define NILHEAD ((HEADER *)NULL)


static HEADER *hlist = NILHEAD;		/* old message header */
static char *rfcfields[] = {		/* list of RFC-822 fields */
	"FROM ",	"FROM",		"RECEIVED",	"SENDER",
	"TO",		"CC",		"BCC",		"SUBJECT",
	"MESSAGE-ID",	"DATE",		NULL
};
static char olduser[1024];		/* old user */
static char olddate[48];		/* old date */
static char oldhost[48];		/* old host machine */


/*
 * Look for a field in the header table in memory.
 * If found, return its value, otherwise return NULL.
 */
static char *hfind(name)
char *name;
{
  register HEADER *hp;
  char tmp[48];

  hp = hlist;
  while (hp != NILHEAD) {
	strcpy(tmp, hp->name);
	strupr(tmp);
	if (!strcmp(tmp, name)) return(hp->value);
	hp = hp->next;
  }
  return(NULL);
}


/*
 * Look for a field in the header table in memory.
 * If found, mark the field as READ, and return its adress.
 * Otherwise, return NILHEAD.
 */
static HEADER *hsearch(name)
char *name;
{
  register HEADER *hp;
  char tmp[48];

  hp = hlist;
  while (hp != NILHEAD) {
	strcpy(tmp, hp->name);
	strupr(tmp);
	if (!strcmp(tmp, name)) {
		if (hp->done == 0) {
			hp->done = 1;
			return(hp);
		}
	}
	hp = hp->next;
  }
  return(NILHEAD);
}


/*
 * Decode an old-style (V6/V7) mail header.
 * This has a syntax like:
 *
 *	From <user> <date> [remote from <host>]
 *	To: <user>
 *	Subject: <text>
 *
 * We want to find out the <user>, <date> and possible <date> fields.
 * Return TRUE for OK, or FALSE if error.
 */
static int get_oldhdr(rmt)
int *rmt;		/* REMOTE or LOCAL mail? */
{
  register char *bp, *sp, *cp;

  sp = hfind("FROM ");
  if (sp == NULL) {	/* No From-line??? */
	sprintf(errmsg, "%s: no From line", dfile);
       	return(FALSE);
  }
  
  strcpy(olduser, sp);		/* stuff field into 'user' */
  sp = olduser;			/* skip until <date> field */
  while (*sp && *sp!=' ' && *sp!='\t') sp++;
  *sp++ = '\0';			/* mark 'end-of-user' */

  /*
   * SP now contains <date> and (possible) <remote> fields.
   * Parse line to seek out "remote from".
   */
  cp = sp;		/* save the Date-pointer */
  while (TRUE) {
	bp = strchr(sp++, 'r');
	if (bp != NULL) {	 /* we found an 'r' */
		if (!strncmp(bp, "remote from ", 12)) break;
        } else break;
  }

  if (bp != NULL) {		/* remote host found --> remote mail */
	sp = strrchr(bp, ' ');	/* seek to start of "remote from" text */
	*(bp - 1) = '\0';	/* mark end-of-date */
       	strcpy(olddate, cp);	/* set old date */
	strcpy(oldhost, ++sp);	/* set host name */
	sprintf(mailsender, "%s!%s", oldhost, olduser);
	*rmt = TRUE;
  } else {
	  strcpy(olddate, cp);	/* set old date */
	  strcpy(oldhost, "");	/* no remote host */
	  strcpy(mailsender, olduser);
	  *rmt = FALSE;	
    }
  return(TRUE);
}


/*
 * Analyze the current header.
 *
 * See if this mail was generated locally or came from somewhere else.
 * Note, that old-style mailers use "postmarks" (i.e. header lines
 * looking like "From <user> <date>" with a possible suffix of
 * "remote from <host>". New-style mailers (should) only use the
 * "From: <path>" and "Date: <date>" lines in their headers.
 * UMAIL knows both types. By default it uses new-style headers,
 * but it can use (and generate) old headers by defining OLDMAILER.
 *
 * Return TRUE if we think this mail has been generated remotely,
 * or FALSE if this message was generated by local mail.
 */
static int chk_hdr(void)
{
  int remmail;				/* remote mail? */
  long now;
  register char *sp, *bp;		/* fast scanning pointers */

  bp = hfind("FROM");			/* get RFC-From: field */
  sp = hfind("DATE");			/* get RFC-Date: field */

  if (sp==NULL || bp==NULL) {		/* should have both or none! */
	if (oldmailer == TRUE) {	/* try old-style header */
		if (get_oldhdr(&remmail) == FALSE) {
			strcat(errmsg, "\n\nBad adress or header!\n");
			return(FALSE);
		}
	}
  } else {	/* only use new-style From:=lines */
	  strcpy(olddate, sp);	/* Save the DATE field */
	  strcpy(oldhost, bp);
	  sp = oldhost;		/* skip comments */
	  while (*sp && *sp!=' ' && *sp!='\t') sp++;
	  *sp = '\0';
	  strcpy(mailsender, oldhost);

	  sp = strchr(oldhost, '!');	/* check for pathname! */
	  if (sp != NULL) {	/* found one; this was remote! */
		remmail = TRUE;
		*sp++ = '\0';
		strcpy(olduser, sp);
	  } else {
		  remmail = FALSE;
		  strcpy(olduser, bp);
		  strcpy(oldhost, "");
	    }
    }
  return(remmail);
}


/*
 * Create a new RFC-822 message header.
 * This is necessary because we are processing
 * a locally-generated message.
 * The header should become:
 *
 *      From <host!user> <date> remote from <here>
 *      From: <host>!<user>
 *	Received: by <here> with <proto>;
 *                <receive-date>
 *	Sender: <user>@<host>.<domain> (Real Name)
 *	To: <user>
 *	Subject: <text>
 */
static void new_hdr(outfp)
FILE *outfp;
{
  long rcvtm;				/* current time */
  char *date;				/* current date in MET */
  char *fmt1 = "%s:%s\n";
  char *fmt2 = "%s\n";
  register char *sp;

  /* get the current date and time */
  time(&rcvtm); date = maketime(&rcvtm);

  if (oldmailer == TRUE) {
	fprintf(outfp, "From %s %s remote from %s\n",
				olduser, xtime(&rcvtm), myname);
  }
  fprintf(outfp, "From: %s!%s (%s)\n", myname, olduser, realname(olduser));
  fprintf(outfp, "Received: by %s.%s (UMAIL %s) with UUCP;\n          %s\n",
				myname, mydomain, Version, date);
  fprintf(outfp, "Sender: %s (%s)\n", full_id(olduser), realname(olduser));
  if (myorg != NULL) fprintf(outfp, "Organization: %s\n", myorg);
  if ((sp = hfind("TO")) != NULL) fprintf(outfp, "To: %s\n", sp);
  if ((sp = hfind("SUBJECT")) != NULL) fprintf(outfp, "Subject: %s\n", sp);
  fprintf(outfp, "Date: %s\n", olddate);
  fprintf(outfp, "\n");
}


/*
 * Update the current header.
 * This is necessary because the message comes from
 * a remote system without RFC-conforming mailer...
 * We should include ALL RFC-822 fields in this routine!
 */
static void upd_hdr(outfp)
FILE *outfp;
{
  long rcvtm;				/* current time */
  char *date;				/* current date in MET */
  char *fmt1 = "%s: %s\n";
  char *fmt2 = "%s\n";
  register char *sp;
  register HEADER *hp;

  /* get the current date and time */
  time(&rcvtm); date = maketime(&rcvtm);

  /* First of all, get the Old V6/V7 From-line */
  if (oldmailer == TRUE) {
	hp = hsearch("FROM ");		/* to make it DONE */
	if (oldhost[0] == '\0')
		fprintf(outfp, "From %s %s remote from %s\n",
					olduser, xtime(&rcvtm), myname);
    	  else fprintf(outfp, "From %s!%s %s remote from %s\n",
				oldhost, olduser, xtime(&rcvtm), myname);
  }

  /*
   * Write the modified From:-line
   * Note, that we must only add our name if the mail is to be forwarded
   * to another system. If it will be delivered locally, leave it.
   */
  hp = hsearch("FROM");
  if (hp != NILHEAD) {
	if (aremote == TRUE) {	/* adressee is REMOTE. add our name! */
		fprintf(outfp, "%s: %s!%s\n", hp->name, myname, hp->value);
	} else {		/* adressee is LOCAL */	
		fprintf(outfp, "%s: %s\n", hp->name, hp->value);
	  }
  }

  /* Our own Receive:-line */
  fprintf(outfp, "Received: by %s.%s (UMAIL %s) with UUCP;\n          %s\n",
				myname, mydomain, Version, date);

  /* Next, all other Received:-lines */
  while ((hp = hsearch("RECEIVED")) != NILHEAD) {
	if (hp->count > 1) fprintf(outfp, fmt2, hp->value);
	  else fprintf(outfp, fmt1, hp->name, hp->value);
  }

  /* The old Sender:-line */
  hp = hsearch("SENDER");
  if (hp != NILHEAD) {
	if (hp->count > 1) fprintf(outfp, fmt2, hp->value);
	  else fprintf(outfp, fmt1, hp->name, hp->value);
  }

  /* insert all unknown fields here */
  hp = hlist;
  while (hp != NILHEAD) {
	if (hp->std == 0) {
		hp->done = 1;
		fprintf(outfp, fmt1, hp->name, hp->value);
	}
	hp = hp->next;
  }

  /* Write the To:-line too */
  hp = hsearch("TO");
  if (hp != NILHEAD) {
	if (hp->count > 1) fprintf(outfp, fmt2, hp->value);
	  else fprintf(outfp, fmt1, hp->name, hp->value);
  }

  /* The CarbonCopy Cc:-line */
  hp = hsearch("CC");
  if (hp != NILHEAD) {
	if (hp->count > 1) fprintf(outfp, fmt2, hp->value);
	  else fprintf(outfp, fmt1, hp->name, hp->value);
  }

  /* And the BlindCarbonCopy as well */
  hp = hsearch("BCC");
  if (hp != NILHEAD) {
	if (hp->count > 1) fprintf(outfp, fmt2, hp->value);
	  else fprintf(outfp, fmt1, hp->name, hp->value);
  }

  /* Finally, the old Subject:-line */
  hp = hsearch("SUBJECT");
  if (hp != NILHEAD) {
	if (hp->count > 1) fprintf(outfp, fmt2, hp->value);
	  else fprintf(outfp, fmt1, hp->name, hp->value);
  }

  /* A message ID */
  hp = hsearch("MESSAGE-ID");
  if (hp != NILHEAD) {
	if (hp->count > 1) fprintf(outfp, fmt2, hp->value);
	  else fprintf(outfp, fmt1, hp->name, hp->value);
  }

  /* And the old date of sending */
  hp = hsearch("DATE");
  if (hp != NILHEAD) {
	if (hp->count > 1) fprintf(outfp, fmt2, hp->value);
	  else fprintf(outfp, fmt1, hp->name, hp->value);
  } else fprintf(outfp, fmt1, "Date: ", date);

  /* an empty line marks the end of the header! */
  fprintf(outfp, "\n");
}


/*
 * Read the message-header into memory.
 */
static int read_hdr(infp)
register FILE *infp;
{
  char hdrbuf[1024];
  char lastf[128];
  int i, numfields = 0;
  int lastc = 1;
  register HEADER *hp, *xp;
  register char *bp, *sp;

  while (TRUE) {
	if (mfgets(hdrbuf, 1024, infp) == NULL) break;	/* end of file */
	if (hdrbuf[0] == '\0') break;	/* end of header */

	numfields++;
	bp = hdrbuf;

	/* first check if this is the V6/V7 From-line */
	if (strncmp(hdrbuf, "From ", 5)) {
		/* No From-line. */
		if (*bp==' ' || *bp=='\t') {
			lastc++;	/* next part of previous field */	
			bp = lastf;	/* previous field */
			sp = hdrbuf;	/* value */
		} else {
			sp = strchr(bp, ':');	/* plain field, get sepa */
			if (sp != NULL) {	/* do we have one? */
				*sp++ = '\0';		/* end it */
				while (*sp && (*sp==' ' || *sp=='\t')) sp++;
				strcpy(lastf, bp);    /* set as prev field */
				lastc = 1;	
			} else sp = bp;	      /* no sepa, use entire field */
	  	  }
	} else {
		bp = "From ";
		sp = &hdrbuf[5];
	  }

	/* Add a new header field to the message header in memory */
	hp = (HEADER *) malloc(sizeof(HEADER)); 	/* allocate new variable */
	if (hlist == NILHEAD) {			/* first variable */
    		hlist = hp;
  	} else {
          	xp = hlist;
          	while (xp->next != NILHEAD) xp = xp->next;
          	xp->next = hp;
    	  }

  	hp->next = NILHEAD;
  	hp->name = (char *) malloc(strlen(bp) + 2);
  	hp->value = (char *) malloc(strlen(sp) + 2);

  	strcpy(hp->name, bp);
  	strcpy(hp->value, sp);
  	hp->done = 0;		/* not yet read */
  	hp->count = lastc;	/* folding level */
  	hp->std = 0;		/* standard field? */

	/* now see if this field is an RFC-822 field */
	i = 0;
  	sp = rfcfields[i];
	strcpy(hdrbuf, hp->name);	/* convert field name to uppercase */
	strupr(hdrbuf);
  	while (sp != NULL) {
		if (!strcmp(sp, hdrbuf)) break;
 		sp = rfcfields[++i];
  	}
  	if (sp != NULL) hp->std = 1;
  }
}


/*
 * Read the header from the input file 'infd', and adapt some
 * fields to the new values.
 * Then, sort the entries and generate a new header.
 * Put that new header into file 'outfp'.
 * Return TRUE if REMOTE, FALSE if LOCAL mail.
 */
int header(infp, outfp)
register FILE *infp;
register FILE *outfp;
{
  int remote;
  char *sp;

  (void) read_hdr(infp);		/* read in the current header */
	
  remote = chk_hdr(outfp);		/* analyze old header */

  if (remote == FALSE) new_hdr(outfp);	/* locally-generated mail */
    else upd_hdr(outfp);

  return(remote);
}
