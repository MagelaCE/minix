#include <lib.h>
/* This is the routine called by <assert.h>. */

#include <stdio.h>
#include <stdlib.h>

void __assert(file, line)
char *file;
long line;
{
  fprintf(stderr, "Assertion error in file %s on line %u\n", file, line);
  abort();
}
