static	char	RCSid[] =
"$Header: minix.c,v 1.3 88/10/29 14:07:36 tony Exp $";

/*
 * System-dependent routines for Minix-ST
 *
 * modifications by:  Robert Regn	   rtregn@faui32.uucp
 *
 * $Log:	minix.c,v $
 * Revision 1.3  88/10/29  14:07:36  tony
 * Added optional support for termcap.
 * 
 * Revision 1.2  88/10/27  08:20:12  tony
 * Added doshell() (same as the unix version), and a no-op version of fixname().
 * 
 * Revision 1.1  88/10/25  20:14:00  tony
 * Initial revision
 * 
 *
 */

#include "stevie.h"
#include <sgtty.h>

/*
 * inchar() - get a character from the keyboard
 */
int
inchar()
{
	char	c[4];
	short n;

	flushbuf();		/* flush any pending output */

	while ( (n=read(0, c, 3)) < 1)
		;
	if (n > 1) {
		if (n == 3 && c[0] == 033 && c[1] == '[')  /*  cursor button */
			switch (c[2]) {
				case 'A':	return K_UARROW;
				case 'B':	return K_DARROW;
				case 'C':	return K_RARROW;
				case 'D':	return K_LARROW;
				default: /* .. stuffin */ ;
			}
		c[n] = NUL;
		stuffin ( c+1);
		}

	return *c;
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

static	struct	sgttyb	ostate;

/*
 * Go into cbreak mode
 */
void
set_tty()
{
	struct	sgttyb	nstate;

	 ioctl(0, TIOCGETP, &ostate);
	 nstate = ostate;
	 nstate.sg_flags &= ~(XTABS|ECHO);
	 nstate.sg_flags |= CBREAK;
	 ioctl(0, TIOCSETP, &nstate);
}

/*
 * Restore original terminal modes
 */
void
reset_tty()
{
	ioctl(0, TIOCSETP, &ostate);
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

	char	*term, *getenv();
	if ((term = getenv("TERM")) == NULL || strcmp(term, "minix") != 0) {
		fprintf(stderr, "Invalid terminal type '%s'\n", term);
		exit(1);
	}
	Columns = 80;
	P(P_LI) = Rows = 25;
#endif

	set_tty();
}

void
windexit(r)
int r;
{
	flushbuf();
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
strchr(s, c)
char	*s;
char	c;
{
	char *index();

	return index(s, c);
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
	reset_tty();

	if (cmd == NULL 	/* don't say  sh sh  */
	 || *cmd == NUL ) {	/* handle :!<return> */
		if ((cmd = getenv("SHELL")) == NULL)
			cmd = "/bin/sh";
		switch (fork() ) {
			case	0:	
					execl(cmd, "sh", "-i", 0);
					emsg("exec failed - ");
					exit(1);

			case	-1:	emsg("fork failed - ");
					break;

			default:	wait(0);
		}

	}

	else	if (system(cmd) == -1)
		outstr("execution of command failed - ");
	set_tty();

	wait_return();
}
