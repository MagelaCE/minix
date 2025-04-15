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
 *		R E A D    M A I L    M O D U L E
 *
 * Author:	Fred van Kempen, MicroWalt Corporation
 *
 * To Do:
 *		- TERMCAP/TERMINFO use !
 */
#include <stdio.h>
#include <string.h>
#include <sgtty.h>
#include "wmail.h"


static int mline;		/* current terminal line */
static int mcol;		/* current terminal column */
static char mobuf[512];		/* output buffer */
static int mobc;		/* position in output buffer (== chars in) */
static int misdone;		/* flag: return EOF next read even if not */


/*
 * Write one line of text to the screen.
 * Return 1 if a Quit command was given.
 */
static int lwrite(fd, buf, len)
int fd;
char *buf;
unsigned len;
{
  unsigned here, start;
  char cmd;

  start = 0;
  here = 0;
  while (here != len) {
	cmd = '\0';
      	switch (buf[here++]) {
		case '\n':
		    mcol = 0;
		    if (++mline == 23) {
			fflush(stdout);
		     	write(1, buf + start, here - start);
		     	fflush(stdout);
		     	fprintf(stderr, "\007\033[7m--More--\033[0m");
		     	do {
			    read(fd, &cmd, 1);
       			   } while (strchr(" \r\nqQ'nN", cmd)
						== (char *)NULL);
		        fprintf(stderr, "\r\033[K");
		        mline = 0;
		        start = here;
 	  	    }
		   break;
		case '\r':
		    mcol = 0;
		    break;
	case '\b': 
		    if (mcol != 0) mcol--;
		      else {
			    mline--;
			    mcol = 80 - 1;
		      }
		    break;
	case '\t':
		    do {
		        mcol++;
		    } while (mcol % 8 != 0);
		    break;
	default:   
		    if ( buf[here-1] < ' ' || (buf[here-1] & 0x80) )
		  					 buf[here-1] = '?';
		    if (++mcol == 80) {
		     	mcol = 0;
		     	if (++mline == 23) {
		       		fflush(stdout);
		       		write(1, buf + start, here - start);
		       		fflush(stdout);
		       		fprintf(stderr, 
					"\007\033[7m--More--\033[0m");
		       		do {
			   	    read(fd, &cmd, 1);
       			  	} while (strchr(" \r\nqQ'nN", cmd)
							== (char *)NULL);
		       		fprintf(stderr, "\r\033[K");
		       		mline = 0;
		       		start = here;
		      	}
		    }
       	}
      	switch (cmd) {
		case '\0':
			break;
		case ' ':
			mline = 0;
		   	break;
		case '\r':
		case '\n':
			mline = 23 - 1;
		   	break;
		case 'q':
		case 'Q':
			misdone = 1;
		   	return(1);
		case 'n':
		case 'N':
			misdone = 1;
			return(1);
		default:
			break;
	}
  }
  if (here != start) {
	fflush(stdout);
	write(1, buf + start, here - start);
	fflush(stdout);
  }
}


/*
 * Display the given letter on the screen.
 * Do this on a per-page basis...
 */
void showlet(let)
LETTER *let;
{
  struct sgttyb ttymode;
  off_t curr, limit;
  int c, fd, st;

  if ((fd = open("/dev/tty", 0)) == -1) {
	fprintf(stderr, "%s: cannot open /dev/tty\n", progname);
	return;
  }
  ioctl(fd, TIOCGETP, &ttymode);
  ttymode.sg_flags |= CBREAK;
  ttymode.sg_flags &= ~ECHO;
  ioctl(fd, TIOCSETP, &ttymode);

  fseek(boxfp, (curr = let->location), 0);
  limit = (let->next != NIL_LET) ? let->next->location : -1L;

  st = mline = mcol = mobc = misdone = 0;
  printf("Message %d:\n", let->seqno);
  while(curr != limit && ((c = fgetc(boxfp)) != EOF) && st==0) {
	if (mobc == sizeof(mobuf)) {
		st = lwrite(fd, mobuf, strlen(mobuf));
        	mobc = 0;
       	}
      	mobuf[mobc++] = (char) c;
      	++curr;
  }
  if (st == 0) lwrite(fd, mobuf, mobc);
  mobc = 0;
  fflush(stdout);

  ttymode.sg_flags &= ~CBREAK;
  ttymode.sg_flags |= ECHO;
  ioctl(fd, TIOCSETP, &ttymode);
  close(fd);
}


/*
 * Print the contents of letter 'let' to file 'tofp'
 */
void printlet(let, tofp)
LETTER *let;
FILE *tofp;
{
  off_t curr, limit, oldpos;
  int c;

  if (tofp==stdout && !printmode) {
	showlet(let);
	return;
  }

  oldpos = ftell(boxfp);
  fseek(boxfp, (curr = let->location), 0);
  limit = (let->next != NIL_LET) ? let->next->location : -1L;

  if (tofp == stdout) printf("Message %d:\n", let->seqno);
  while(curr != limit && (c = fgetc(boxfp)) != EOF) {
	fputc(c, tofp);
	++curr;
  }
  fflush(tofp);
  fseek(boxfp, oldpos, 0);
}


/*
 * Print all letters and quit.
 */
void printall()
{
  LETTER *let;

  let = firstlet;
  if (let == NIL_LET) {
	fprintf(stderr, "No mail for %s.\n", sender);
	return;
  }

  while(let != NIL_LET) {
      	printlet(let, stdout);
      	let = let->next;
  }
}
