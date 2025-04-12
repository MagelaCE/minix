#include <stdio.h>
#include <stdarg.h>

int fprintf(file, format /* , ... */)
FILE *file;
char *format;
{
 register va_list argp;
 int count;

 va_start(argp, format);
 count = vfprintf(file, format, argp);
 va_end(argp);

#ifdef PERPRINTF			/* for Minix */
 if(testflag(file, PERPRINTF))
    fflush(file);
#endif

 return count;
}
 
