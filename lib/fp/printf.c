#include <stdio.h>
#include <stdarg.h>

int printf(format /* , ... */)
char *format;
{
 register va_list argp;
 int count;

 va_start(argp, format);
 count = vfprintf(stdout, format, argp);
 va_end(argp);

#ifdef PERPRINTF			/* for Minix */
 if(testflag(stdout, PERPRINTF))
    fflush(stdout);
#endif

 return count;
}
