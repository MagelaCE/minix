/* file: atof.c */

/* #include <stddef.h> */
/* #include <stdlib.h> */

#ifndef NULL
#define NULL 0
#endif

extern double strtod(/* const char *s, char **endp */);

double atof(s)
char *s;
{
 return strtod(s, (char **)NULL);
}
