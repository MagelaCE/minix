/*
 - bcmp - Berklix equivalent of memcmp
 */

int				/* == 0 or != 0 for equality and inequality */
bcmp(s1, s2, length)
char *s1;
char *s2;	
int length;
{
	return(memcmp((char *)s1, (char *)s2, (int)length));
}
