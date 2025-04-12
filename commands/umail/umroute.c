/*
 * UMAIL -	MINIX Remote Domain-adressing Mail Router
 *
 *		This version of RMAIL handles message-headers in a much
 *		more "standard" way. It can handle bang-adresses, plus
 *		the new-style Internet adressing (user@host.domain).
 *		It is called by programs as "Mail" and "Uuxqt".
 *
 *	    	M A I L   R O U T E R   M O D U L E
 *
 * 		Some rules exist for routing to other machines.
 *
 *		1. Check for local system names.
 *
 *		2. If we have a domain, check it for validity.
 *
 *		3. If we have a hostname, check it for validity.
 *
 *		4. Local user: check existence.
 *
 * 		Examples are:
 *
 *			user@host.domain@newdomain -
 *				-> ast@cs.vu@nluug.nl
 *			host!user@domain
 *				--> cs.vu!ast@nluug.nl
 *
 *		If we cannot find a matching domain or system name,
 *		then we get stuck. Fortunately, we can define the
 *		ESCAPE parameter, which allows us to send all mail
 *		for unknown hosts or domains to a "default" domain.
 *		This is the domain called "." in the tables.
 *		If the ESCAPE parameter is FALSE, we cannot deliver
 *		the message, so return it to the sender!
 *
 * Return TRUE (1) if OK, or FALSE (0) if an error was encountered.
 *
 * Author:	F. van Kempen, Jul-Oct '89 (waltje@minixug.nluug.nl)
 */
#include <stdio.h>
#include <alloc.h>
#include <pwd.h>
#include <string.h>
#include "umail.h"


/*
 * Route the adress in BOX into a ready-to-run
 * UUCP adress.
 */
int route(b)
BOX *b;
{
  char temp[1024];
  register char *bp, *sp;
  register ROUTE *rp;
  register HOST *hp;
  struct passwd *pw;

  /* 
   * Rule 1: check for local system names.
   *	     convert to LOCAL if possible.
   */
  if (b->domain[0] == '\0') strcpy(temp, b->host);
    else sprintf(temp, "%s.%s", b->host, b->domain);
  if (islocal(temp)) {
	b->host[0] = '\0';
	b->domain[0] = '\0';
  }

  /*
   * Rule 2: Do we have a domain?
   */
  if (b->domain[0] != '\0') {			/* domain given? */
	if (b->host[0]) sprintf(temp, "%s.%s", b->host, b->domain);
	  else strcpy(temp, b->domain);
	strcpy(b->host, temp);
	bp = temp;
	rp = NILROUTE;
	while (TRUE) {			/* iterate on domain fragments */
		sp = bp;
		rp = getdomain(bp);
		if (rp != NILROUTE) {		/* a matching domain! */
			strcpy(b->domain, bp);
			break;
		}
		bp = strchr(sp, '.');				
		if (bp == NULL) break;
		  else bp++;
	}

	/*
 	 * We now have checked the DOMAIN table for a matching domain.
	 * If we failed to find a match, there is a problem.
	 * If the mailer was defined with the ESCAPE parameter, we can
	 * try to route it to the default (".") domain.
	 * Otherwise, we cannot deliver the message...
	 */

	/* check if we found a matching domain */
	if (rp == NILROUTE) {	/* we did not. try to re-route it */
		if (escape == FALSE) {
			sprintf(errmsg, "%s ... domain unknown", b->domain);
			return(FALSE);
		}
		rp = getdomain(".");	/* get default domain! */
		if (rp == NILROUTE) {
			sprintf(errmsg, "%s ... domain unknown", b->domain);
			strcat(errmsg, "\n\nESCAPE domain not found.");
			return(FALSE);
		}
	}

	/*
	 * At this point we have all the information we
 	 * need to build an UUCP-adress.
	 * We have a HOST as well.
	 * Check if we can indeed reach that host.
	 */
	hp = gethost(rp->host);
	if (hp == NILHOST) {
		sprintf(errmsg, "%s ... host unreacheble", rp->host);
		return(FALSE);
	}

	/* is the host smart enough to get "@"-adresses?? */
	if (hp->smart == TRUE) {	/* yes, it is! */
		if (*(rp->route) == '@') {
			if (b->host[0] == '\0') sprintf(mailaddr,"%s@%s",
				    	    		b->user, b->domain);
  	  		  else sprintf(mailaddr, "%s@%s", b->user, b->host);
		} else {
			if (b->host[0] == '\0') sprintf(mailaddr,"%s!%s@%s",
					rp->route, b->user, b->domain);
  	  		  else sprintf(mailaddr, "%s!%s@%s.%s",
				rp->route, b->user, b->host, b->domain);
		  }
	} else {
		if (b->host[0] == '\0') sprintf(mailaddr,"%s!%s",
							rp->route, b->user);
		  else sprintf(mailaddr, "%s!%s!%s",
				rp->route, b->host, b->user);
	  }
	strcpy(b->host, rp->host);

	/*
	 * We now have a HOST and an ADRESS.
	 */
	strcpy(mailcmd, hp->command);
	strcpy(mailopts, hp->opts);
	strcpy(mailhost, b->host);
	aremote = TRUE;
	return(TRUE);
  }

  /*
   * Rule 3: Do we have a host name ?
   */
  if (b->host[0] != '\0') {			/* host name given? */
	b->domain[0] = '\0';			/* signal 'no routing' */
	hp = gethost(b->host);
	if (hp == NILHOST) {
		if (escape == FALSE) {
			sprintf(errmsg, "%s ... host unknown", b->host);
			return(FALSE);
		}
		rp = getdomain(".");	/* get default domain! */
		if (rp == NILROUTE) {
			sprintf(errmsg, "%s ... host unknown", b->host);
			strcat(errmsg, "\n\nESCAPE domain not found.");
			return(FALSE);
		}

		/*
		 * We now have a HOST as well.
		 * Check if we can indeed reach that host.
		 */
		strcpy(b->domain, rp->host);
		hp = gethost(rp->host);
		if (hp == NILHOST) {
			sprintf(errmsg, "%s ... host unreacheble", rp->host);
			return(FALSE);
		}
	}

	/*
	 * USER	contains the user-part
	 * HOST contains the (old) host-part
	 * DOMAIN now contains the new hostname
	 */

	/* is the host smart enough to get "@"-adresses?? */
	if (b->domain[0] != '\0') {	/* are we routing? */
		if (hp->smart == TRUE) sprintf(mailaddr, "%s@%s",
							b->user, b->host);
		  else sprintf(mailaddr, "%s!%s", b->host, b->user);
		strcpy(b->host, b->domain);
	} else {	/* no, ordinary case */
		strcpy(mailaddr, b->user);
	  }

	strcpy(mailhost, b->host);
	strcpy(mailcmd, hp->command);
	strcpy(mailopts, hp->opts);
	aremote = TRUE;
	return(TRUE);
  }

  /*
   * Rule 4: Check for local user.
   */
  if ((pw = getpwnam(b->user)) == (struct passwd *)NULL) {
	sprintf(errmsg, "%s ... user unknown", b->user);
	return(FALSE);
  }
  hp = gethost(myname);
  strcpy(mailaddr, b->user);
  mailhost[0] = '\0';
  strcpy(mailcmd, hp->command);
  strcpy(mailopts, hp->opts);
  aremote = FALSE;
  return(TRUE);
}
