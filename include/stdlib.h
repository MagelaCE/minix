/* The <stdlib.h> header defines certain common macros, types, and functions.*/

#ifndef _STDLIB_H
#define _STDLIB_H

/* The macros are NULL, EXIT_FAILURE, EXIT_SUCCESS, RAND_MAX, and MB_CUR_MAX.*/
#ifndef NULL
#define NULL    ((void *) 0)
#endif

#define EXIT_FAILURE       1	/* standard error return using exit() */
#define EXIT_SUCCESS       0	/* successful return using exit() */
#define RAND_MAX       32767	/* largest value generated by rand() */
#define MB_CUR_MAX         1	/* max value of multibyte character in MINIX */


/* The types are size_t, wchar_t, div_t, and ldiv_t. */
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;	/* type returned by sizeof */
#endif

#ifndef _WCHAR_T
#define _WCHAR_T
typedef char wchar_t;		/* type expanded character set */
#endif

typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;

/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( void abort, (void)						     );
_PROTOTYPE( int abs, (int __j)						     );
_PROTOTYPE( int atexit, (void (*func)(void))				     );
_PROTOTYPE( double atof, (const char *__nptr)				     );
_PROTOTYPE( int atoi, (const char *__nptr)				     );
_PROTOTYPE( long atol, (const char *__nptr)				     );
_PROTOTYPE( void *calloc, (size_t __nmemb, size_t __size)		     );
_PROTOTYPE( div_t div, (int __numer, int __denom)			     );
_PROTOTYPE( void exit, (int __status)					     );
_PROTOTYPE( void free, (void *__ptr)					     );
_PROTOTYPE( char *getenv, (const char *__name)				     );
_PROTOTYPE( long labs, (long __j)					     );
_PROTOTYPE( ldiv_t ldiv, (long __numer, long __denom)			     );
_PROTOTYPE( void *malloc, (size_t __size)				     );
_PROTOTYPE( int mblen, (const char *__s, size_t __n)			     );
_PROTOTYPE( size_t mbstowcs, (wchar_t *__pwcs, const char *__s, size_t __n)  );
_PROTOTYPE( int mbtowc, (wchar_t *__pwc, const char *__s, size_t __n)	     );
_PROTOTYPE( int rand, (void)						     );
_PROTOTYPE( void *realloc, (void *__ptr, size_t __size)			     );
_PROTOTYPE( void srand, (unsigned int __seed)				     );
_PROTOTYPE( double strtod, (const char *__nptr, char **__endptr)	     );
_PROTOTYPE( long strtol, (const char *__nptr, char **__endptr, int __base)   );
_PROTOTYPE( int system, (const char *__string)				     );
_PROTOTYPE( size_t wcstombs, (char *__s, const wchar_t *__pwcs, size_t __n)  );
_PROTOTYPE( int wctomb, (char *__s, int __wchar)			     );
_PROTOTYPE( void *bsearch, \
	(const void *__key, const void *__base, size_t __nmemb, \
	size_t __size, int (*__compar) (const void *, const void *))	     );
_PROTOTYPE( void qsort, (void *__base, size_t __nmemb, size_t __size, \
	int (*__compar) (const void *, const void *))			     );
_PROTOTYPE( unsigned long int strtoul,
			(const char *__nptr, char **__endptr, int __base)    );

#endif /* STDLIB_H */
