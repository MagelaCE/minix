#include <stdio.h>
#include <varargs.h>

char *vsprintf(buf,format,argp)
char *buf, *format;
va_list argp;
{
	FILE _tempfile;

	_tempfile._fd    = -1;
	_tempfile._flags = WRITEMODE + STRINGS;
	_tempfile._buf   = buf;
	_tempfile._ptr   = buf;

	vfprintf(&_tempfile,format,argp);
	putc('\0',&_tempfile);

	return buf;
}
