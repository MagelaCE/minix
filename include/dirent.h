/* The <dirent.h> header file is for POSIX.  It is used by the opendir(), 
 * readdir(), writedir(), and related procedure calls.
 */

#ifndef _DIRENT_H
#define _DIRENT_H

/* DIRBUF must in general be larger than the filesystem buffer size. */
#define	_DIRBUF		2048	/* buffer size for fs-indep. dirs */
#define _DIR_MAGIC     19997	/* arbitrary number for marking DIRs */

/* The DIR structure is used for all the directory operations. */
typedef struct {
 int dd_fd;			/* file descriptor */
 int dd_loc;			/* offset in block */
 int dd_size;			/* amount of valid data */
 int dd_magic;			/* magic number to recognize DIRs */
 char *dd_buf;			/* -> directory block */
} DIR;				/* stream data from opendir() */


struct dirent {			/* data from getdents()/readdir() */
  long d_ino;			/* inode number of entry */
  off_t d_off;			/* offset of disk directory entry */
  unsigned short d_reclen;	/* length of this record */
  char d_name[1];		/* name of file plus a 0 byte */
};

/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( int closedir, (DIR *__dirp)					);
_PROTOTYPE( int getdents, (int fildes, char *buf, unsigned nbyte)	);
_PROTOTYPE( DIR *opendir, (char *__dirname)				);
_PROTOTYPE( struct dirent *readdir, (DIR *__dirp)			);
_PROTOTYPE( void rewinddir, (DIR *__dirp)				);

#endif /* _DIRENT_H */
