#include <stdio.h>
#include <stdarg.h>

int sprintf(buf, format /* , ... */)
char *buf;
char *format;
{
 register va_list argp;
 int count;

 va_start(argp, format);
 count = vsprintf(buf, format, argp);
 va_end(argp);

 return count;
}
