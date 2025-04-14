#include <lib.h>
/* strncpy - copy at most n characters of string src to dst */

#include <string.h>

char *strncpy(dst, src, n)
char *dst;
_CONST char *src;
size_t n;
{
  register char *dscan;
  register _CONST char *sscan;
  register size_t count;

  dscan = dst;
  sscan = src;
  count = n;
  while (count > 0 && (*dscan++ = *sscan++) != '\0') count--;
  while (count > 0) {
    *dscan++ = '\0'; count--;
  }
  return(dst);
}
