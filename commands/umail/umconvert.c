/*
 * UMAIL -	MINIX Remote Domain-adressing Mail Router
 *
 *		This version of RMAIL handles message-headers in a much
 *		more "standard" way. It can handle bang-adresses, plus
 *		the new-style Internet adressing (user@host.domain).
 *		It is called by programs as "Mail" and "Uuxqt".
 *
 *	    	A D R E S S   C O N V E R S I O N   M O D U L E
 *
 * 		Convert adresses into manageable chunks.
 *
 * 		We understand the following notations:
 *
 *		1. user@host.domain	--> waltje@minixug.UUCP
 *	
 *		2. host!user		--> minixug!waltje
 *	
 *		3. user			--> waltje
 *
 * Return TRUE (1) if OK, or FALSE (0) if an error was encountered.
 *
 * Author:	F. van Kempen, Jul-Oct '89 (waltje@minixug.nluug.nl)
 */
#include <stdio.h>
#include <alloc.h>
#include <string.h>
#include "umail.h"


/*
 * Convert adress 'adr' into more manageable chunks.
 * Stuff the output into 'mailaddr' (the final user)
 * and 'mailcmd' (the mailer command to use).
 * Return NILBOX if an error ocurred.
 */
BOX *convert(adr)
char *adr;
{
  static BOX box;
  char temp[1024];
  register char *bp, *sp, *cp;
  register ROUTE *rp;
  register HOST *hp;

  strcpy(mailaddr, adr);
  box.domain[0] = box.host[0] = '\0';
  box.user[0] = temp[0] = '\0';

  /*
   * Rule 1: Check for user@host.domain
   */
  sp = strrchr(mailaddr, '@');
  if (sp != NULL) {
	*sp++ = '\0';
	strcpy(box.user, mailaddr);
	strcpy(temp, sp);	
	strlwr(temp);		/* convert domain to lower case: RFC822 */

	/*
	 * Rule 1A: Now check for "." in the domain part.
	 * 	    This indicates that a host name was given.
	 */
	sp = strchr(temp, '.');		/* syntax host.domain ?? */
	if (sp == NULL) {		/* no, onlky 'host' part */
		hp = gethost(temp);	/* is this a local host? */
		if (hp != NILHOST) {	/* yes! */
			strcpy(box.host, temp);
			box.domain[0] = '\0';
			return(&box);
		} else {		/* no, must be a domain.... */
			strcpy(box.domain, temp);
			box.host[0] = '\0';
			return(&box);
	  	  }
	} else {			/* domain and host given! */
		*sp++ = '\0';
		strcpy(box.host, temp);
		strcpy(box.domain, sp);
		return(&box);
	  }
  }

  /*
   * Rule 2: Check for host!user
   */
  sp = strchr(mailaddr, '!');
  if (sp != NULL) {
	*sp++ = '\0';
	strcpy(box.host, mailaddr);
	strcpy(box.user, sp);
	return(&box);
  }

  /*
   * Rule 3: Must be local user.
   */
  strcpy(box.user, mailaddr);
  box.host[0] = '\0';
  box.domain[0] = '\0';
  return(&box);
}

