/*
 * strlen - length of string (not including NUL)
 */
int
strlen(s)
char *s;
{
	register char *scan;
	register int count;

	count = 0;
	scan = s;
	while (*scan++ != '\0')
		count++;
	return(count);
}
