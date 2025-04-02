static	char	RCSid[] =
"$Header: unix.c,v 1.5 88/10/31 13:11:03 tony Exp $";

/*
 * System-dependent routines for UNIX System V or Berkeley.
 *
 * $Log:	unix.c,v $
 * Revision 1.5  88/10/31  13:11:03  tony
 * Added code (optional) to support the use of termcap.
 * 
 * Revision 1.4  88/10/27  08:16:52  tony
 * Added doshell() to support ":sh" and ":!".
 * 
 * Revision 1.3  88/10/06  10:14:36  tony
 * Added fixname() routine, which does nothing under UNIX.
 * 
 * Revision 1.2  88/06/20  14:51:36  tony
 * Merged in changes for BSD Unix sent in by Michael Lichter.
 * 
 * Revision 1.1  88/03/20  21:11:02  tony
 * Initial revision
 * 
 *
 */

#include "stevie.h"
#ifdef BSD
#include <sgtty.h>
#else
#include <termio.h>
#endif

/*
 * inchar() - get a character from the keyboard
 */
int
inchar()
{
	char	c;

	flushbuf();		/* flush any pending output */

	do {
		while (read(0, &c, 1) != 1)
			;
	} while (c == NUL);

	return c;
}

#define	BSIZE	2048
static	char	outbuf[BSIZE];
static	int	bpos = 0;

void
flushbuf()
{
	if (bpos != 0)
		write(1, outbuf, bpos);
	bpos = 0;
}

/*
 * Macro to output a character. Used within this file for speed.
 */
#define	outone(c)	outbuf[bpos++] = c; if (bpos >= BSIZE) flushbuf()

/*
 * Function version for use outside this file.
 */
void
outchar(c)
register char	c;
{
	outbuf[bpos++] = c;
	if (bpos >= BSIZE)
		flushbuf();
}

void
outstr(s)
register char	*s;
{
	while (*s) {
		outone(*s++);
	}
}

void
beep()
{
	outone('\007');
}

/*
 * remove(file) - remove a file
 */
void
remove(file)
char *file;
{
	unlink(file);
}

/*
 * rename(of, nf) - rename existing file 'of' to 'nf'
 */
void
rename(of, nf)
char	*of, *nf;
{
	unlink(nf);
	link(of, nf);
	unlink(of);
}

void
delay()
{
	/* not implemented */
}

#ifdef BSD
static	struct	sgttyb	ostate;
#else
static	struct	termio	ostate;
#endif

/*
 * Go into cbreak mode
 */
void
set_tty()
{
#ifdef BSD
	struct	sgttyb	nstate;

	ioctl(0, TIOCGETP, &ostate);
	nstate = ostate;
	nstate.sg_flags &= ~(XTABS|CRMOD|ECHO);
	nstate.sg_flags |= CBREAK;
	ioctl(0, TIOCSETN, &nstate);
#else
	struct	termio	nstate;

	ioctl(0, TCGETA, &ostate);
	nstate = ostate;
	nstate.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL);
	nstate.c_cc[VMIN] = 1;
	nstate.c_cc[VTIME] = 0;
	ioctl(0, TCSETAW, &nstate);
#endif
}

/*
 * Restore original terminal modes
 */
void
reset_tty()
{
#ifdef BSD
	ioctl(0, TIOCSETP, &ostate);
#else
	ioctl(0, TCSETAW, &ostate);
#endif
}

void
windinit()
{
#ifdef	TERMCAP
	if (t_init() != 1) {
		fprintf(stderr, "unknown terminal type\n");
		exit(1);
	}
#else
	Columns = 80;
	P(P_LI) = Rows = 24;
#endif

	set_tty();
}

void
windexit(r)
int r;
{
	reset_tty();
	exit(r);
}

#define	outone(c)	outbuf[bpos++] = c; if (bpos >= BSIZE) flushbuf()

void
windgoto(r, c)
register int	r, c;
{
#ifdef	TERMCAP
	char	*tgoto();
#else
	r += 1;
	c += 1;
#endif

	/*
	 * Check for overflow once, to save time.
	 */
	if (bpos + 8 >= BSIZE)
		flushbuf();

#ifdef	TERMCAP
	outstr(tgoto(T_CM, c, r));
#else
	outbuf[bpos++] = '\033';
	outbuf[bpos++] = '[';
	if (r >= 10)
		outbuf[bpos++] = r/10 + '0';
	outbuf[bpos++] = r%10 + '0';
	outbuf[bpos++] = ';';
	if (c >= 10)
		outbuf[bpos++] = c/10 + '0';
	outbuf[bpos++] = c%10 + '0';
	outbuf[bpos++] = 'H';
#endif
}

FILE *
fopenb(fname, mode)
char	*fname;
char	*mode;
{
	return fopen(fname, mode);
}

char *
fixname(s)
char	*s;
{
	return s;
}

/*
 * doshell() - run a command or an interactive shell
 */
void
doshell(cmd)
char	*cmd;
{
	char	*cp, *getenv();
	char	cline[128];

	outstr("\r\n");
	flushbuf();

	if (cmd == NULL) {
		if ((cmd = getenv("SHELL")) == NULL)
			cmd = "/bin/sh";
		sprintf(cline, "%s -i", cmd);
		cmd = cline;
	}

	reset_tty();
	system(cmd);
	set_tty();

	wait_return();
}
