#define	DIRBLKSIZ	512		/* size of directory block */

#ifndef DIRSIZ
#define	DIRSIZ	14
#endif

struct direct {
	ino_t	d_ino;
	char	d_name[DIRSIZ];
};
