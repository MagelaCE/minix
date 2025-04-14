#include <lib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int fflush(iop)
FILE *iop;
{
  int count;

  if (testflag(iop, UNBUFF) || !testflag(iop, WRITEMODE)) return(0);

  if (iop->_count <= 0) return(0);

  count = write(iop->_fd, iop->_buf, iop->_count);

  if (count == iop->_count) {
	iop->_count = 0;
	iop->_ptr = iop->_buf;
	return(count);
  }
  iop->_flags |= _ERR;
  return(EOF);
}
