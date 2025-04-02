/*
 * Definitions of various common control characters
 *
 * $Header: ascii.h,v 1.1 88/03/20 21:03:24 tony Exp $
 *
 * $Log:	ascii.h,v $
 * Revision 1.1  88/03/20  21:03:24  tony
 * Initial revision
 * 
 *
 */

#define	NUL	'\0'
#define	BS	'\010'
#define	TAB	'\011'
#define	NL	'\012'
#define	CR	'\015'
#define	ESC	'\033'

#define	CTRL(x)	((x) & 0x1f)
