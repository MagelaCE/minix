/*
 * strncpy - copy at most n characters of string src to dst
 */
char *				/* dst */
strncpy(dst, src, n)
char *dst;
char *src;
int n;
{
	register char *dscan;
	register char *sscan;
	register int count;

	dscan = dst;
	sscan = src;
	count = n;
	while (--count >= 0 && (*dscan++ = *sscan++) != '\0')
		continue;
	while (--count >= 0)
		*dscan++ = '\0';
	return(dst);
}
