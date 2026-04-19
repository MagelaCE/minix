/*
 * memccpy - copy bytes up to a certain char
 *
 * CHARBITS should be defined only if the compiler lacks "unsigned char".
 * It should be a mask, e.g. 0377 for an 8-bit machine.
 */

#define	NULL	0

#ifndef CHARBITS
#	define	UNSCHAR(c)	((unsigned char)(c))
#else
#	define	UNSCHAR(c)	((c)&CHARBITS)
#endif

char *
memccpy(dst, src, ucharstop, size)
char * dst;
char * src;
int size;
{
	register char *d;
	register char *s;
	register int n;
	register int uc;

	if (size <= 0)
		return(NULL);

	s = src;
	d = dst;
	uc = UNSCHAR(ucharstop);
	for (n = size; n > 0; n--)
		if (UNSCHAR(*d++ = *s++) == uc)
			return(d);

	return(NULL);
}
