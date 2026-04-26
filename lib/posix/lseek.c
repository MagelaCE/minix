#include <lib.h>
#include <sys/types.h>

PUBLIC off_t lseek(fd, offset, whence)
int fd;
off_t offset;
int whence;
{
  int k;
  M.m2_i1 = fd;
  M.m2_l1 = offset;
  M.m2_i2 = whence;
  k = callx(FS, LSEEK);
  if (k != 0) return((off_t) k);/* send itself failed */
  return((off_t)M.m2_l1);
}
