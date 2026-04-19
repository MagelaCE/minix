/*
 - bzero - Berklix subset of memset
 */

bzero(dst, length)
char *dst;
int length;
{
	extern char * memset();

	(void) memset((char *)dst, 0, (int)length);
}
