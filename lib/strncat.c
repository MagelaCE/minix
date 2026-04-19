/*
 * strncat - append at most n characters of string src to dst
 */
char *				/* dst */
strncat(dst, src, n)
char *dst;
char *src;
int n;
{
	register char *dscan;
	register char *sscan;
	register int count;

	for (dscan = dst; *dscan != '\0'; dscan++)
		continue;
	sscan = src;
	count = n;
	while (*sscan != '\0' && --count >= 0)
		*dscan++ = *sscan++;
	*dscan++ = '\0';
	return(dst);
}
