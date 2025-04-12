#include <stdio.h>
#include <stdarg.h>

int vsprintf(buf, format, argp)
char *buf;
char *format;
register va_list argp;
{
 int count;
 FILE stringfile;

 stringfile._fd    = -1;
 stringfile._flags = WRITEMODE|STRINGS;
 stringfile._buf   = buf;
 stringfile._ptr   = buf;

 count = vfprintf(&stringfile, format, argp);

 *(stringfile._ptr) = '\0';

 return count;
}
