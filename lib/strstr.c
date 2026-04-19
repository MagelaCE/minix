/*
 * strstr - find first occurrence of wanted in s
 */

#define	NULL	0

char *				/* found string, or NULL if none */
strstr(s, wanted)
char *s;
char *wanted;
{
	register char *scan;
	register int len;
	register char firstc;
	extern int strcmp();
	extern int strlen();

	/*
	 * The odd placement of the two tests is so "" is findable.
	 * Also, we inline the first char for speed.
	 * The ++ on scan has been moved down for optimization.
	 */
	firstc = *wanted;
	len = strlen(wanted);
	for (scan = s; *scan != firstc || strncmp(scan, wanted, len) != 0; )
		if (*scan++ == '\0')
			return(NULL);
	return(scan);
}
