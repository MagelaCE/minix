/*
 * strlen - length of string (not including NUL)
 */
#include <string.h>
SIZET
strlen(s)
CONST char *s;
{
	register CONST char *scan;
	register SIZET count;

	count = 0;
	scan = s;
	while (*scan++ != '\0')
		count++;
	return(count);
}
