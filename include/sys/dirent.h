/*
	<sys/dirent.h> -- file system independent directory entry (SVR3)

	last edit:	27-Oct-1988	D A Gwyn

	prerequisite:	<sys/types.h>
*/

struct dirent				/* data from getdents()/readdir() */
	{
	long		d_ino;		/* inode number of entry */
	off_t		d_off;		/* offset of disk directory entry */
	unsigned short	d_reclen;	/* length of this record */
	char		d_name[1];	/* name of file */	/* non-ANSI */
	};

#ifdef BSD_SYSV				/* (e.g., when compiling getdents.c) */
extern struct dirent	__dirent;	/* (not actually used) */
/* The following is portable, although rather silly. */
#define	DIRENTBASESIZ		(__dirent.d_name - (char *)&__dirent.d_ino)

#else
/* The following nonportable ugliness could have been avoided by defining
   DIRENTSIZ and DIRENTBASESIZ to also have (struct dirent *) arguments.
   There shouldn't be any problem if you avoid using the DIRENTSIZ() macro. */

#define	DIRENTBASESIZ		(((struct dirent *)0)->d_name \
				- (char *)&((struct dirent *)0)->d_ino)
#endif

#define	DIRENTSIZ( namlen )	((DIRENTBASESIZ + sizeof(long) + (namlen)) \
				/ sizeof(long) * sizeof(long))

/* DAG -- the following was moved from <dirent.h>, which was the wrong place */
#define	MAXNAMLEN	512		/* maximum filename length */

#ifndef NAME_MAX
#define	NAME_MAX	(MAXNAMLEN - 1)	/* DAG -- added for POSIX */
#endif
