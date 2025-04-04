static	char	RCSid[] =
"$Header: ptrfunc.c,v 1.4 88/10/27 08:14:11 tony Exp $";

/*
 * The routines in this file attempt to imitate many of the operations
 * that used to be performed on simple character pointers and are now
 * performed on LPTR's. This makes it easier to modify other sections
 * of the code. Think of an LPTR as representing a position in the file.
 * Positions can be incremented, decremented, compared, etc. through
 * the functions implemented here.
 *
 * $Log:	ptrfunc.c,v $
 * Revision 1.4  88/10/27  08:14:11  tony
 * Made some of the pointer operations less sensitive to the possibility
 * of null pointers.
 * 
 * Revision 1.3  88/08/26  08:45:40  tony
 * Misc. changes to make lint happy.
 * 
 * Revision 1.2  88/04/23  20:36:45  tony
 * Fixed a bug in dec() that caused problems when decrementing at the start
 * of the file. dec() now sticks at the start of the file. This caused
 * problems when reverse searching from the first character of the file.
 * 
 * Revision 1.1  88/03/20  21:09:55  tony
 * Initial revision
 * 
 *
 */

#include "stevie.h"

/*
 * inc(p)
 *
 * Increment the line pointer 'p' crossing line boundaries as necessary.
 * Return 1 when crossing a line, -1 when at end of file, 0 otherwise.
 */
int
inc(lp)
register LPTR	*lp;
{
	register char *p;

	if (lp && lp->linep)
		p = &(lp->linep->s[lp->index]);
	else
		return -1;

	if (*p != NUL) {			/* still within line */
		lp->index++;
		return ((p[1] != NUL) ? 0 : 1);
	}

	if (lp->linep->next != Fileend->linep) {  /* there is a next line */
		lp->index = 0;
		lp->linep = lp->linep->next;
		return 1;
	}

	return -1;
}

/*
 * dec(p)
 *
 * Decrement the line pointer 'p' crossing line boundaries as necessary.
 * Return 1 when crossing a line, -1 when at start of file, 0 otherwise.
 */
int
dec(lp)
register LPTR	*lp;
{
	if (lp->index > 0) {			/* still within line */
		lp->index--;
		return 0;
	}

	if (lp->linep->prev != Filetop->linep) { /* there is a prior line */
		lp->linep = lp->linep->prev;
		lp->index = strlen(lp->linep->s);
		return 1;
	}

	lp->index = 0;				/* stick at first char */
	return -1;				/* at start of file */
}

/*
 * gchar(lp) - get the character at position "lp"
 */
int
gchar(lp)
register LPTR	*lp;
{
	if (lp && lp->linep)
		return (lp->linep->s[lp->index]);
	else
		return 0;
}

/*
 * pchar(lp, c) - put character 'c' at position 'lp'
 */
void
pchar(lp, c)
register LPTR	*lp;
char	c;
{
	lp->linep->s[lp->index] = c;
}

/*
 * pswap(a, b) - swap two position pointers
 */
void
pswap(a, b)
register LPTR	*a, *b;
{
	LPTR	tmp;

	tmp = *a;
	*a  = *b;
	*b  = tmp;
}

/*
 * Position comparisons
 */

bool_t
lt(a, b)
register LPTR	*a, *b;
{
	register int an, bn;

	an = LINEOF(a);
	bn = LINEOF(b);

	if (an != bn)
		return (an < bn);
	else
		return (a->index < b->index);
}

#if 0
bool_t
gt(a, b)
LPTR	*a, *b;
{
	register int an, bn;

	an = LINEOF(a);
	bn = LINEOF(b);

	if (an != bn)
		return (an > bn);
	else
		return (a->index > b->index);
}
#endif

bool_t
equal(a, b)
register LPTR	*a, *b;
{
	return (a->linep == b->linep && a->index == b->index);
}

bool_t
ltoreq(a, b)
register LPTR	*a, *b;
{
	return (lt(a, b) || equal(a, b));
}

#if 0
bool_t
gtoreq(a, b)
LPTR	*a, *b;
{
	return (gt(a, b) || equal(a, b));
}
#endif
