#include <stdio.h>
#include <stdarg.h>

int vprintf(format, argp)
char *format;
va_list argp;
{
 return vfprintf(stdout, format, argp);
}
