static	char	RCSid[] =
"$Header: misccmds.c,v 1.9 88/09/06 06:50:42 tony Exp $";

/*
 * Various routines to perform specific editing operations or return
 * useful information about the file.
 *
 * $Log:	misccmds.c,v $
 * Revision 1.9  88/09/06  06:50:42  tony
 * Fixed a bug with shifts that was introduced when replace mode was added.
 * 
 * Revision 1.8  88/08/30  20:36:51  tony
 * After much prodding from Mark, I finally added support for replace mode.
 * 
 * Revision 1.7  88/08/26  08:45:18  tony
 * Misc. changes to make lint happy.
 * 
 * Revision 1.6  88/06/26  14:50:19  tony
 * Added a new parameter to delline() to control whether screen updates
 * may be performed while deleting the line(s).
 * 
 * Revision 1.5  88/05/02  07:34:46  tony
 * Fixed a bug in the last change to plines(). The column counter wasn't
 * being initialized to zero, so plines() returned garbage.
 * 
 * Revision 1.4  88/05/01  20:09:13  tony
 * Modified plines() to support the new "number" parameter.
 * 
 * Revision 1.3  88/04/30  20:00:04  tony
 * Added code to openfwd() and openbwd() to support auto-indent.
 * 
 * Revision 1.2  88/04/23  20:39:21  tony
 * Reworked openfwd() to avoid a problem when opening a line at the end of
 * the file when on the last line of the screen. Opening a line now inserts
 * or scrolls appropriately and deals with opens correctly when the cursor
 * is on a long line.
 * 
 * Revision 1.1  88/03/20  21:08:41  tony
 * Initial revision
 * 
 *
 */

#include "stevie.h"

static	void	openfwd(), openbwd();

extern	bool_t	did_ai;

/*
 * opencmd
 *
 * Add a blank line above or below the current line.
 */

void
opencmd(dir, can_ai)
int	dir;
int	can_ai;			/* if true, consider auto-indent */
{
	if (dir == FORWARD)
		openfwd(can_ai);
	else
		openbwd(can_ai);
}

static void
openfwd(can_ai)
int	can_ai;
{
	LINE	*l;
	LPTR	*next;
	char	*s;		/* string to be moved to new line, if any */
	int	newindex = 0;	/* index of the cursor on the new line */

	/*
	 * If we're in insert mode, we need to move the remainder of the
	 * current line onto the new line. Otherwise the new line is left
	 * blank.
	 */
	if (State == INSERT || State == REPLACE)
		s = &Curschar->linep->s[Curschar->index];
	else
		s = "";

	if ((next = nextline(Curschar)) == NULL)	/* open on last line */
		next = Fileend;

	/*
	 * By asking for as much space as the prior line had we make sure
	 * that we'll have enough space for any auto-indenting.
	 */
	if ((l = newline(strlen(Curschar->linep->s) + SLOP)) == NULL)
		return;

	if (*s != NUL)
		strcpy(l->s, s);		/* copy string to new line */

	else if (can_ai && P(P_AI) && !anyinput()) {
		char	*p;

		/*
		 * Copy prior line, and truncate after white space
		 */
		strcpy(l->s, Curschar->linep->s);

		for (p = l->s; *p == ' ' || *p == TAB ;p++)
			;
		*p = NUL;
		newindex = p - l->s;

		/*
		 * If we just did an auto-indent, then we didn't type
		 * anything on the prior line, and it should be truncated.
		 */
		if (did_ai)
			Curschar->linep->s[0] = NUL;

		did_ai = TRUE;
	}

	/* truncate current line at cursor */
	if (State == INSERT || State == REPLACE)
		*s = NUL;
			

	Curschar->linep->next = l;	/* link neighbors to new line */
	next->linep->prev = l;

	l->prev = Curschar->linep;	/* link new line to neighbors */
	l->next = next->linep;

	if (next == Fileend)			/* new line at end */
		l->num = Curschar->linep->num + LINEINC;

	else if ((l->prev->num) + 1 == l->next->num)	/* no gap, renumber */
		renum();

	else {					/* stick it in the middle */
		unsigned long	lnum;
		lnum = ((long)l->prev->num + (long)l->next->num) / 2;
		l->num = lnum;
	}

	/*
	 * Get the cursor to the start of the line, so that 'Cursrow'
	 * gets set to the right physical line number for the stuff
	 * that follows...
	 */
	Curschar->index = 0;
	cursupdate();

	/*
	 * If we're doing an open on the last logical line, then
	 * go ahead and scroll the screen up. Otherwise, just insert
	 * a blank line at the right place. We use calls to plines()
	 * in case the cursor is resting on a long line.
	 */
	if (Cursrow + plines(Curschar) == (Rows - 1))
		scrollup(1);
	else
		s_ins(Cursrow+plines(Curschar), 1);

	*Curschar = *nextline(Curschar);	/* cursor moves down */
	Curschar->index = newindex;

	updatescreen();		/* because Botchar is now invalid... */

	cursupdate();		/* update Cursrow before insert */
}

static void
openbwd(can_ai)
int	can_ai;
{
	LINE	*l;
	LINE	*prev;
	int	newindex = 0;

	prev = Curschar->linep->prev;

	if ((l = newline(strlen(Curschar->linep->s) + SLOP)) == NULL)
		return;

	Curschar->linep->prev = l;	/* link neighbors to new line */
	prev->next = l;

	l->next = Curschar->linep;	/* link new line to neighbors */
	l->prev = prev;

	if (can_ai && P(P_AI) && !anyinput()) {
		char	*p;

		/*
		 * Copy current line, and truncate after white space
		 */
		strcpy(l->s, Curschar->linep->s);

		for (p = l->s; *p == ' ' || *p == TAB ;p++)
			;
		*p = NUL;
		newindex = p - l->s;

		did_ai = TRUE;
	}

	Curschar->linep = Curschar->linep->prev;
	Curschar->index = newindex;

	if (prev == Filetop->linep)		/* new start of file */
		Filemem->linep = l;

	renum();	/* keep it simple - we don't do this often */

	cursupdate();			/* update Cursrow before insert */
	if (Cursrow != 0)
		s_ins(Cursrow, 1);		/* insert a physical line */

	updatescreen();
}

int
cntllines(pbegin,pend)
LPTR *pbegin, *pend;
{
	LINE *lp;
	int lnum = 1;

	for (lp = pbegin->linep; lp != pend->linep ;lp = lp->next)
		lnum++;

	return(lnum);
}

/*
 * plines(p) - return the number of physical screen lines taken by line 'p'
 */
int
plines(p)
LPTR	*p;
{
	register int	col = 0;
	register char	*s;

	s = p->linep->s;

	if (*s == NUL)		/* empty line */
		return 1;

	for (; *s != NUL ;s++) {
		if ( *s == TAB && !P(P_LS))
			col += P(P_TS) - (col % P(P_TS));
		else
			col += chars[(unsigned)(*s & 0xff)].ch_size;
	}

	/*
	 * If list mode is on, then the '$' at the end of
	 * the line takes up one extra column.
	 */
	if (P(P_LS))
		col += 1;
	/*
	 * If 'number' mode is on, add another 8.
	 */
	if (P(P_NU))
		col += 8;

	return ((col + (Columns-1)) / Columns);
}

void
fileinfo()
{
	long	l1, l2;
	char	buf[80];

	if (bufempty()) {
		l1 = 0;
		l2 = 1;	/* sonst div by zero */
	}
	else {
		l1 = cntllines(Filemem, Curschar);
		l2 = cntllines(Filemem, Fileend) - 1;
	}

	l1 = cntllines(Filemem, Curschar);
	l2 = cntllines(Filemem, Fileend) - 1;
	sprintf(buf, "\"%s\"%s line %ld of %ld -- %ld %% --",
		(Filename != NULL) ? Filename : "No File",
		Changed ? " [Modified]" : "",
		l1, l2, (l1 * 100)/l2);
	msg(buf);
}

/*
 * gotoline(n) - return a pointer to line 'n'
 *
 * Returns a pointer to the last line of the file if n is zero, or
 * beyond the end of the file.
 */
LPTR *
gotoline(n)
int n;
{
	static	LPTR	l;

	l.index = 0;

	if ( n == 0 )
		l = *prevline(Fileend);
	else {
		LPTR	*p;

		for (l = *Filemem; --n > 0 ;l = *p)
			if ((p = nextline(&l)) == NULL)
				break;
	}
	return &l;
}

void
inschar(c)
int	c;
{
	register char	*p, *pend;

	/* make room for the new char. */
	if ( ! canincrease(1) )
		return;

	if (State != REPLACE) {
		p = &Curschar->linep->s[strlen(Curschar->linep->s) + 1];
		pend = &Curschar->linep->s[Curschar->index];

		for (; p > pend ;p--)
			*p = *(p-1);

		*p = c;

	} else {	/* replace mode */
		/*
		 * Once we reach the end of the line, we are effectively
		 * inserting new text, so make sure the string terminator
		 * stays out there.
		 */
		if (gchar(Curschar) == NUL)
			Curschar->linep->s[Curschar->index+1] = NUL;
		pchar(Curschar, c);
	}

	/*
	 * If we're in insert mode and showmatch mode is set, then
	 * check for right parens and braces. If there isn't a match,
	 * then beep. If there is a match AND it's on the screen, then
	 * flash to it briefly. If it isn't on the screen, don't do anything.
	 */
	if (P(P_SM) && State == INSERT && (c == ')' || c == '}' || c == ']')) {
		LPTR	*lpos, csave;

		if ((lpos = showmatch()) == NULL)	/* no match, so beep */
			beep();
		else if (LINEOF(lpos) >= LINEOF(Topchar)) {
			updatescreen();		/* show the new char first */
			csave = *Curschar;
			*Curschar = *lpos;	/* move to matching char */
			cursupdate();
			windgoto(Cursrow, Curscol);
			delay();		/* brief pause */
			*Curschar = csave;	/* restore cursor position */
			cursupdate();
		}
	}

	inc(Curschar);
	CHANGED;
}

#if 0
void
insstr(s)
register char *s;
{
	register char *p, *endp;
	register int k, n = strlen(s);

	/* Move everything in the file over to make */
	/* room for the new string. */
	if (!canincrease(n))
		return;

	endp = &Curschar->linep->s[Curschar->index];
	p = Curschar->linep->s + strlen(Curschar->linep->s) + 1 + n;

	for (; p>endp ;p--)
		*p = *(p-n);

	p = &Curschar->linep->s[Curschar->index];
	for ( k=0; k<n; k++ ) {
		*p++ = *s++;
		inc(Curschar);
	}
	CHANGED;
}
#endif

bool_t
delchar(fixpos)
bool_t	fixpos;		/* if TRUE, fix the cursor position when done */
{
	register int i;

	/* Check for degenerate case; there's nothing in the file. */
	if (bufempty())
		return FALSE;

	if (lineempty())	/* can't do anything */
		return FALSE;

	/* Delete the char. at Curschar by shifting everything */
	/* in the line down. */
	for ( i=Curschar->index+1; i < Curschar->linep->size ;i++)
		Curschar->linep->s[i-1] = Curschar->linep->s[i];

	/* If we just took off the last character of a non-blank line, */
	/* we don't want to end up positioned at the newline. */
	if (fixpos) {
		if (gchar(Curschar)==NUL && Curschar->index>0 && State!=INSERT)
			Curschar->index--;
	}
	CHANGED;

	return TRUE;
}


void
delline(nlines, can_update)
int	nlines;
bool_t	can_update;
{
	register LINE *p, *q;
	int	doscreen;		/* if true, update the screen */

	doscreen = can_update;
	/*
	 * There's no point in keeping the screen updated if we're
	 * deleting more than a screen's worth of lines.
	 */
	if (nlines > (Rows - 1) && can_update) {
		doscreen = FALSE;
		s_del(Cursrow, Rows-1);	/* flaky way to clear rest of screen */
	}

	while ( nlines-- > 0 ) {

		if (bufempty())			/* nothing to delete */
			break;

		if (buf1line()) {		/* just clear the line */
			Curschar->linep->s[0] = NUL;
			Curschar->index = 0;
			break;
		}

		p = Curschar->linep->prev;
		q = Curschar->linep->next;

		if (p == Filetop->linep) {	/* first line of file so... */
			Filemem->linep = q;	/* adjust start of file */
			Topchar->linep = q;	/* and screen */
		}
		p->next = q;
		q->prev = p;

		clrmark(Curschar->linep);	/* clear marks for the line */

		/*
		 * Delete the correct number of physical lines on the screen
		 */
		if (doscreen)
			s_del(Cursrow, plines(Curschar));

		/*
		 * If deleting the top line on the screen, adjust Topchar
		 */
		if (Topchar->linep == Curschar->linep)
			Topchar->linep = q;

		free(Curschar->linep->s);
		free(Curschar->linep);

		Curschar->linep = q;
		Curschar->index = 0;		/* is this right? */
		CHANGED;

		/* If we delete the last line in the file, back up */
		if ( Curschar->linep == Fileend->linep) {
			Curschar->linep = Curschar->linep->prev;
			/* and don't try to delete any more lines */
			break;
		}
	}
}
