/* This is the routine called by <assert.h>. */

#include <lib.h>
#include <stdlib.h>
#include <stdio.h>

void __assert(file, line)
char *file;
long line;
{
  fprintf(stderr, "Assertion error in file %s on line %u\n", file, line);
  abort();
}
