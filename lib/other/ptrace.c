#include <lib.h>

PUBLIC long ptrace(req, pid, addr, data)
int req, pid;
long addr, data;
{
  M.m2_i1 = pid;
  M.m2_i2 = req;
  M.m2_l1 = addr;
  M.m2_l2 = data;
  if (callx(MM, PTRACE) == -1) return(-1L);
  if (M.m2_l2 == -1) {
	errno = 0;
	return(-1L);
  }
  return(M.m2_l2);
}
