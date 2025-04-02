static	char	RCSid[] =
"$Header: edit.c,v 1.7 88/11/10 13:29:15 tony Exp $";

/*
 * The main edit loop as well as some other simple cursor movement routines.
 *
 * $Log:	edit.c,v $
 * Revision 1.7  88/11/10  13:29:15  tony
 * Added a call to do_mlines() to check for mode lines at initialization.
 * 
 * Revision 1.6  88/10/06  10:11:50  tony
 * Fixed a bug in scrolldown() involving the special case of the cursor
 * being on the last line, and the line being scrolled onto the screen
 * being long. The special case code needs to deal with physical lines,
 * not logical.
 * 
 * Revision 1.5  88/08/30  20:35:53  tony
 * After much prodding from Mark, I finally added support for replace mode.
 * 
 * Revision 1.4  88/08/26  08:44:55  tony
 * Misc. changes to make lint happy.
 * 
 * Revision 1.3  88/05/01  20:08:47  tony
 * Fixed some problems with the new auto-indent feature.
 * 
 * Revision 1.2  88/04/30  19:59:43  tony
 * Added support for auto-indent option.
 * 
 * Revision 1.1  88/03/20  21:07:10  tony
 * Initial revision
 * 
 *
 */

#include "stevie.h"

/*
 * This flag is used to make auto-indent work right on lines where only
 * a <RETURN> or <ESC> is typed. It is set when an auto-indent is done,
 * and reset when any other editting is done on the line. If an <ESC>
 * or <RETURN> is received, and did_ai is TRUE, the line is truncated.
 */
bool_t	did_ai = FALSE;

void
edit()
{
	int c;
	char *p, *q;

	Prenum = 0;

	/* position the display and the cursor at the top of the file. */
	*Topchar = *Filemem;
	*Curschar = *Filemem;
	Cursrow = Curscol = 0;

	do_mlines();		/* check for mode lines before starting */

	for ( ;; ) {

	/* Figure out where the cursor is based on Curschar. */
	cursupdate();

	windgoto(Cursrow,Curscol);

	c = vgetc();

	if (State == NORMAL) {

		/* We're in the normal (non-insert) mode. */

		/* Pick up any leading digits and compute 'Prenum' */
		if ( (Prenum>0 && isdigit(c)) || (isdigit(c) && c!='0') ){
			Prenum = Prenum*10 + (c-'0');
			continue;
		}
		/* execute the command */
		normal(c);
		Prenum = 0;

	} else {

		/*
		 * Insert or Replace mode.
		 */
		switch (c) {

		case ESC:	/* an escape ends input mode */

			/*
			 * If we just did an auto-indent, truncate the
			 * line, and put the cursor back.
			 */
			if (did_ai) {
				Curschar->linep->s[0] = NUL;
				Curschar->index = 0;
				did_ai = FALSE;
			}

			set_want_col = TRUE;

			/* Don't end up on a '\n' if you can help it. */
			if (gchar(Curschar) == NUL && Curschar->index != 0)
				dec(Curschar);

			/*
			 * The cursor should end up on the last inserted
			 * character. This is an attempt to match the real
			 * 'vi', but it may not be quite right yet.
			 */
			if (Curschar->index != 0 && !endofline(Curschar))
				dec(Curschar);

			State = NORMAL;
			msg("");

			/* construct the Redo buffer */
			p=Redobuff;
			q=Insbuff;
			while ( q < Insptr )
				*p++ = *q++;
			*p++ = ESC;
			*p = NUL;
			updatescreen();
			break;

		case CTRL('D'):
			/*
			 * Control-D is treated as a backspace in insert
			 * mode to make auto-indent easier. This isn't
			 * completely compatible with vi, but it's a lot
			 * easier than doing it exactly right, and the
			 * difference isn't very noticeable.
			 */
		case BS:
			/* can't backup past starting point */
			if (Curschar->linep == Insstart->linep &&
			    Curschar->index <= Insstart->index) {
				beep();
				break;
			}

			/* can't backup to a previous line */
			if (Curschar->linep != Insstart->linep &&
			    Curschar->index <= 0) {
				beep();
				break;
			}

			did_ai = FALSE;
			dec(Curschar);
			if (State == INSERT)
				delchar(TRUE);
			/*
			 * It's a little strange to put backspaces into
			 * the redo buffer, but it makes auto-indent a
			 * lot easier to deal with.
			 */
			*Insptr++ = BS;
			Ninsert++;
			cursupdate();
			updateline();
			break;

		case CR:
		case NL:
			*Insptr++ = NL;
			Ninsert++;
			opencmd(FORWARD, TRUE);		/* open a new line */
			break;

		default:
			did_ai = FALSE;
			insertchar(c);
			break;
		}
	}
	}
}

/*
 * Special characters in this context are those that need processing other
 * than the simple insertion that can be performed here. This includes ESC
 * which terminates the insert, and CR/NL which need special processing to
 * open up a new line. This routine tries to optimize insertions performed
 * by the "redo" command, so it needs to know when it should stop and defer
 * processing to the "normal" mechanism.
 */
#define	ISSPECIAL(c)	((c) == NL || (c) == CR || (c) == ESC)

void
insertchar(c)
int c;
{
#if 0
	char *p;

	if ( ! anyinput() ) {
#endif
		inschar(c);
		*Insptr++ = c;
		Ninsert++;
		/*
		 * The following kludge avoids overflowing the statically
		 * allocated insert buffer. Just dump the user back into
		 * command mode, and print a message.
		 */
		if (Insptr+10 >= &Insbuff[1024]) {
			stuffin(mkstr(ESC));
			emsg("No buffer space - returning to command mode");
			sleep(2);
		}
#if 0
	}
	else {
		/* If there's any pending input, grab it all at once. */
		p = Insptr;
		*Insptr++ = c;
		Ninsert++;
		for (c = vpeekc(); !ISSPECIAL(c) ;c = vpeekc()) {
			c = vgetc();
			*Insptr++ = c;
			Ninsert++;
		}
		*Insptr = '\0';
		insstr(p);
	}
#endif
	updateline();
}

void
getout()
{
	windgoto(Rows-1,0);
	putchar('\r');
	putchar('\n');
	windexit(0);
}

void
scrolldown(nlines)
int nlines;
{
	register LPTR	*p;
	register int	done = 0;	/* total # of physical lines done */

	/* Scroll up 'nlines' lines. */
	while (nlines--) {
		if ((p = prevline(Topchar)) == NULL)
			break;
		done += plines(p);
		*Topchar = *p;
		/*
		 * If the cursor is on the bottom line, we need to
		 * make sure it gets moved up the appropriate number
		 * of lines so it stays on the screen.
		 */
		if (Curschar->linep == Botchar->linep->prev) {
			int	i = 0;
			while (i < done) {
				i += plines(Curschar);
				*Curschar = *prevline(Curschar);
			}
		}
	}
	s_ins(0, done);
}

void
scrollup(nlines)
int nlines;
{
	register LPTR	*p;
	register int	done = 0;	/* total # of physical lines done */
	register int	pl;		/* # of plines for the current line */

	/* Scroll down 'nlines' lines. */
	while (nlines--) {
		pl = plines(Topchar);
		if ((p = nextline(Topchar)) == NULL)
			break;
		done += pl;
		if (Curschar->linep == Topchar->linep)
			*Curschar = *p;
		*Topchar = *p;

	}
	s_del(0, done);
}

/*
 * oneright
 * oneleft
 * onedown
 * oneup
 *
 * Move one char {right,left,down,up}.  Return TRUE when
 * sucessful, FALSE when we hit a boundary (of a line, or the file).
 */

bool_t
oneright()
{
	set_want_col = TRUE;

	switch (inc(Curschar)) {

	case 0:
		return TRUE;

	case 1:
		dec(Curschar);		/* crossed a line, so back up */
		/* fall through */
	case -1:
		return FALSE;
	}
	/*NOTREACHED*/
}

bool_t
oneleft()
{
	set_want_col = TRUE;

	switch (dec(Curschar)) {

	case 0:
		return TRUE;

	case 1:
		inc(Curschar);		/* crossed a line, so back up */
		/* fall through */
	case -1:
		return FALSE;
	}
	/*NOTREACHED*/
}

void
beginline(flag)
bool_t	flag;
{
	while ( oneleft() )
		;
	if (flag) {
		while (isspace(gchar(Curschar)) && oneright())
			;
	}
	set_want_col = TRUE;
}

bool_t
oneup(n)
{
	LPTR p, *np;
	int k;

	p = *Curschar;
	for ( k=0; k<n; k++ ) {
		/* Look for the previous line */
		if ( (np=prevline(&p)) == NULL ) {
			/* If we've at least backed up a little .. */
			if ( k > 0 )
				break;	/* to update the cursor, etc. */
			else
				return FALSE;
		}
		p = *np;
	}
	*Curschar = p;
	/* This makes sure Topchar gets updated so the complete line */
	/* is one the screen. */
	cursupdate();
	/* try to advance to the column we want to be at */
	*Curschar = *coladvance(&p, Curswant);
	return TRUE;
}

bool_t
onedown(n)
{
	LPTR p, *np;
	int k;

	p = *Curschar;
	for ( k=0; k<n; k++ ) {
		/* Look for the next line */
		if ( (np=nextline(&p)) == NULL ) {
			if ( k > 0 )
				break;
			else
				return FALSE;
		}
		p = *np;
	}
	/* try to advance to the column we want to be at */
	*Curschar = *coladvance(&p, Curswant);
	return TRUE;
}
