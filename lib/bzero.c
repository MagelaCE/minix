/*
 - bzero - Berklix subset of memset
 */
#include <string.h>
bzero(dst, length)
char *dst;
int length;
{
	extern VOIDSTAR memset();

	(void) memset((VOIDSTAR)dst, 0, (SIZET)length);
}
