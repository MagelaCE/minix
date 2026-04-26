#include <lib.h>
#include <ctype.h>
#include <stdlib.h>

int atoi(s)
register _CONST char *s;
{
  register int total = 0;
  register unsigned digit;
  register minus = 0;

  while (isspace(*s)) s++;
  if (*s == '-') {
	s++;
	minus = 1;
  }
  while ((digit = *s++ - '0') < 10) {
	total *= 10;
	total += digit;
  }
  return(minus ? -total : total);
}
