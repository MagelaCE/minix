/*
 - bcopy - Berklix equivalent of memcpy
 */

bcopy(src, dst, length)
char *src;
char *dst;
int length;
{
	extern char * memcpy();

	(void) memcpy((char *)dst, (char *)src, (int)length);
}
