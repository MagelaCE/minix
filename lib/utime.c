/* utime(2) for POSIX		Authors: Terrence W. Holm & Edwin L. Froese */

#include "lib.h"

#define NULL  (long *) 0

extern long time();


PUBLIC int utime(name, timp)
char *name;
long timp[2];
{
  if (timp == NULL) {
      long current_time = time(NULL);
      M.m2_l1 = current_time;
      M.m2_l2 = current_time;
  } else {
      M.m2_l1 = timp[0];
      M.m2_l2 = timp[1];
  }

  M.m2_i1 = len(name);
  M.m2_p1 = name;
  return callx(FS, UTIME);
}
