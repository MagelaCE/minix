/*
	closedir -- close a directory stream

	last edit:	11-Nov-1988	D A Gwyn
*/

#include	<errno.h>
#include	<sys/types.h>
#include	<dirent.h>

typedef char	*pointer;		/* (void *) if you have it */

extern void	free();
extern int	close();

extern int	errno;

#ifndef NULL
#define	NULL	0
#endif

int
closedir( dirp )
	register DIR	*dirp;		/* stream from opendir() */
	{
	register int	fd;

	if ( dirp == NULL || dirp->dd_buf == NULL )
		{
		errno = EFAULT;
		return -1;		/* invalid pointer */
		}

	fd = dirp->dd_fd;		/* bug fix thanks to R. Salz */
	free( (pointer)dirp->dd_buf );
	free( (pointer)dirp );
	return close( fd );
	}
