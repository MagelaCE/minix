/* The <regexp.h> header is used by the (V8-compatible) regexp(3) routines. */

#ifndef _REGEXP_H
#define _REGEXP_H

#define CHARBITS 0377
#define NSUBEXP  10
typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( regexp *regcomp, (char *__exp)				);
_PROTOTYPE( int regexec, (regexp *_prog, char *__string, int __bolflag) );
_PROTOTYPE( void regsub, (regexp *__prog, char *__source, char *__dest)	);
_PROTOTYPE( void regerror, (char *__message) 				);

#endif /* _REGEXP_H */
