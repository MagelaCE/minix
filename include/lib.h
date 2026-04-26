/* The <lib.h> header is the master header used by the library.
 * All the C files in the lib subdirectories include it.
 */

#ifndef _LIB_H
#define _LIB_H

/* First come the defines. */
#define _POSIX_SOURCE      1	/* tell headers to include POSIX stuff */
#define _MINIX             1	/* tell headers to include MINIX stuff */

/* The following are so basic, all the lib files get them automatically. */
#include <minix/config.h>	/* must be first */
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <ansi.h>

#include <minix/const.h>
#include <minix/type.h>
#include <minix/callnr.h>

extern message M;

#define MM                 0
#define FS                 1

_PROTOTYPE( int callm1, (int proc, int syscallnr, 
			 int int1, int int2, int int3,
			 char *ptr1, char *ptr2, char *ptr3)		);
_PROTOTYPE( int callm3, (int proc, int syscallnr, int int1,
			 const char *name)				);
_PROTOTYPE( int callx, (int proc, int syscallnr)			);
_PROTOTYPE( int len, (const char *s)					);
_PROTOTYPE( void panic, (const char *message, int errnum)		);
_PROTOTYPE( int sendrec, (int src_dest, message *m_ptr)			);
_PROTOTYPE( int syserr, (const char *name) /* used but not defined */	);
extern _PROTOTYPE( void (*begsig), (int dummy)				);

#endif /* _LIB_H */
