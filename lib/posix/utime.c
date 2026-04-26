/* utime(2) for POSIX		Authors: Terrence W. Holm & Edwin L. Froese */

#include <lib.h>
#include <stddef.h>
#include <time.h>
#include <utime.h>

PUBLIC int utime(name, timp)
char *name;
struct utimbuf *timp;
{
  long current_time;

  if (timp == (struct utimbuf *)NULL) {
	current_time = time((long *)NULL);
	M.m2_l1 = current_time;
	M.m2_l2 = current_time;
  } else {
	M.m2_l1 = timp->actime;
	M.m2_l2 = timp->modtime;
  }

  M.m2_i1 = len(name);
  M.m2_p1 = name;
  return callx(FS, UTIME);
}
