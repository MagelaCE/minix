/*
 * strcat - append string src to dst
 */
char *				/* dst */
strcat(dst, src)
char *dst;
char *src;
{
	register char *dscan;
	register char *sscan;

	for (dscan = dst; *dscan != '\0'; dscan++)
		continue;
	sscan = src;
	while ((*dscan++ = *sscan++) != '\0')
		continue;
	return(dst);
}
