#include <lib.h>
/* rename.c -- file renaming routine */

/* Rename(from, to) char *from, *to;
 */

#include <sys/types.h>
#include <unistd.h>

int rename(from, to)
_CONST register char *from, *to;
{
  (void) unlink(to);
  if (link(from, to) < 0) return(-1);

  (void) unlink(from);
  return(0);
}
