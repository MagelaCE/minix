/*
 * UUCP.H-	DCP: A UUCP clone.
 * 		Definitions for the UUCP package
 *
 * Copyright Richard H. Lamb 1985,1986,1987
 * Copyright S. R. Sampson, August 1989
 * Copyright F. N. G. van Kempen Jul-Oct '89
 */

#ifndef TRUE
#	define FALSE	0
#	define TRUE	1
#endif

#define LSYS		         "/usr/lib/uucp/L.sys"
#define LDEVICE		     "/usr/lib/uucp/L-devices"
#define UUCICO			"/usr/lib/uucp/uucico"
#define UUXQT			 "/usr/lib/uucp/uuxqt"
#define RMAIL			      	       "rmail"	/* Remote Mailer */
#define SMAIL			               "smail"	/* Internet Mailer */
#define LMAIL			               "lmail"	/* Local Mailer */
#define SYSLOG          "/usr/lib/uucp/Log/uucico.log"
#define XQTLOG           "/usr/lib/uucp/Log/uuxqt.log" 
#define PUBDIR		       "/usr/spool/uucppublic"
#define SPOOLDIR	             "/usr/spool/uucp"
#define SPOOLSEQ	      "/usr/lib/uucp/SPOOLSEQ"
#define LSPOOLSEQ	  "/usr/lib/uucp/SPOOLSEQ.LCK"
#define LOCKFILE            "/usr/spool/locks/LCK..%s"	/* terminal LOCKfile */ 
#define GLOCKFILE         "/usr/spool/locks/GLOCK..%s"	/* terminal LOCKfile */ 
#define NODENAME		       "/etc/uucpname"
#define CALLFILE				"C.%s"
#define XQTFILE					"X.%s"
#define MAILFILE				"B.%s"

#define UUCPUSER			  	"uucp"
#define ERRUSER				  "postmaster"

#define UUCPUID					    40	/* RMAIL needs these */
#define UUCPGID					    40
#define POSTUID					    41	/* RMAIL needs these */
#define POSTGID					    40

#define SITENAMELEN	 32
#define PATHLEN		256

#define MSGTIME          20
#define MAXPACK         256

/* L.sys field defines */
#define	FLD_REMOTE	  0	/* remote system name */
#define	FLD_CCTIME	  1	/* legal call times */
#define	FLD_DEVICE	  2	/* device, or ACU for modem */
#define	FLD_SPEED	  3	/* bit rate */
#define FLD_PHONE	  4	/* phone number */
#define	FLD_EXPECT	  5	/* first login "expect" field */
#define FLD_SEND	  6	/* first login "send" field */

