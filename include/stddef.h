/* The <stddef.h> header defines certain commonly used macros. */

#ifndef _STDDEF_H
#define _STDDEF_H

#define NULL	((void *) 0)

/* The final exam will not cover the material present on the next two lines. */
#define offsetof(T,m)	((size_t) (((char *) &(((T *) 256)->m)) - \
						       ((char *) ((T *) 256))))

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef int ptrdiff_t;		/* result of subtracting two pointers */
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;	/* type returned by sizeof */
#endif

#ifndef _WCHAR_T
#define _WCHAR_T
typedef char wchar_t;		/* type expanded character set */
#endif

#endif /* _STDDEF_H */
