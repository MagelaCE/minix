#include <lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define  PMODE    0666

FILE *fopen(name, mode)
_CONST char *name, *mode;
{
  register int i;
  FILE *fp;
  int fd, flags = 0;

  for (i = 0; _io_table[i] != 0; i++)
	if (i >= NFILES) return((FILE *)NULL);

  switch (*mode) {

      case 'w':
	flags |= WRITEMODE;
	if ((fd = creat(name, PMODE)) < 0) return((FILE *)NULL);
	break;

      case 'a':
	flags |= WRITEMODE;
	if ((fd = open(name, 1)) < 0)
		if (errno != ENOENT || (fd = creat(name, PMODE)) < 0)
			return((FILE *)NULL);
	lseek(fd, 0L, 2);
	break;

      case 'r':
	flags |= READMODE;
	if ((fd = open(name, 0)) < 0) return((FILE *)NULL);
	break;

      default:	return((FILE *)NULL);
  }

  if ((fp = (FILE *)malloc(sizeof(FILE))) == (FILE *)NULL) return((FILE *)NULL);

  fp->_count = 0;
  fp->_fd = fd;
  fp->_flags = flags;
  fp->_buf = (char *)malloc(BUFSIZ);
  if (fp->_buf == (char *)NULL)
	fp->_flags |= UNBUFF;
  else
	fp->_flags |= IOMYBUF;

  fp->_ptr = fp->_buf;
  _io_table[i] = fp;
  return(fp);
}
