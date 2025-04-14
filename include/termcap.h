#ifndef _TERMCAP_H
#define _TERMCAP_H

#include <ansi.h>

_PROTOTYPE( int tgetent, (char *bp, char *name)				);
_PROTOTYPE( int tgetflag, (char *id)					);
_PROTOTYPE( int tgetnum, (char *id)					);
_PROTOTYPE( char *tgetstr, (char *id, char **area)			);
_PROTOTYPE( char *tgoto, (char *cm, int destcol, int destline)		);
_PROTOTYPE( int tputs, (char *cp, int affcnt, void (*outc)(int))	);

#endif /* _TERMCAP_H */
