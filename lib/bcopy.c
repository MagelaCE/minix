/*
 - bcopy - Berklix equivalent of memcpy
 */
#include <string.h>
bcopy(src, dst, length)
CONST char *src;
char *dst;
int length;
{
	extern VOIDSTAR memcpy();

	(void) memcpy((VOIDSTAR)dst, (CONST VOIDSTAR)src, (SIZET)length);
}
