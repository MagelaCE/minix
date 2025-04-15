/* memmove.c */
/* Moves a block of memory (safely). */
/* Calls memcpy(), so memcpy() had better be safe. */
/* Henry Spencer's routine is fine. */

#include <string.h>

void *memmove(s1, s2, n)
void *s1;
void *s2;
size_t n;
{
  return memcpy(s1, s2, n);
}
