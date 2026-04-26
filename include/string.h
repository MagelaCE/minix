/* The <string.h> header contains prototypes for the string handling 
 * functions.
 */

#ifndef _STRING_H
#define _STRING_H

#ifndef NULL
#define NULL    ((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;	/* type returned by sizeof */
#endif

/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( void *memcpy, (void *__s1, const void *__s2, size_t __n)        );
_PROTOTYPE( void *memmove, (void *__s1, const void *__s2, size_t __n)       );
_PROTOTYPE( char *strcpy, (char *__s1, const char *__s2)		     );
_PROTOTYPE( char *strncpy, (char *__s1, const char *__s2, size_t __n)	     );
_PROTOTYPE( char *strcat, (char *__s1, const char *__s2)		     );
_PROTOTYPE( char *strncat, (char *__s1, const char *__s2, size_t __n)	     );
_PROTOTYPE( int memcmp, (const void *__s1, const void *__s2, size_t __n)     );
_PROTOTYPE( int strcmp, (const char *__s1, const char *__s2)		     );
_PROTOTYPE( int strcoll, (const char *__s1, const char *__s2)		     );
_PROTOTYPE( int strncmp, (const char *__s1, const char *__s2, size_t __n)    );
_PROTOTYPE( size_t strxfrm, (char *__s1, const char *__s2, size_t __n)	     );
_PROTOTYPE( void *memchr, (const void *__s, int __c, size_t __n)	     );
_PROTOTYPE( char *strchr, (const char *__s, int __c)			     );
_PROTOTYPE( size_t strcspn, (const char *__s1, const char *__s2)	     );
_PROTOTYPE( char *strpbrk, (const char *__s1, const char *__s2)	     	     );
_PROTOTYPE( char *strrchr, (const char *__s, int __c)			     );
_PROTOTYPE( size_t strspn, (const char *__s1, const char *__s2)	     	     );
_PROTOTYPE( char *strstr, (const char *__s1, const char *__s2)	     	     );
_PROTOTYPE( char *strtok, (char *__s1, const char *__s2)		     );
_PROTOTYPE( void *memset, (void *__s, int __c, size_t __n)		     );
_PROTOTYPE( char *strerror, ( int __errnum)				     );
_PROTOTYPE( size_t strlen, (const char *__s)				     );

#endif /* _STRING_H */
