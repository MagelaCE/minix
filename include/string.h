/*
# String library.		Author: Henry Spencer

# Configuration settings:  how should "size_t", "void *", "const" be written?
# "size_t" is what's needed to hold the result of sizeof; beware of problems
# with compatibility here, because X3J11 uses this for e.g. the third
# argument of strncpy() as well.  You may need to make it "int" even if
# this is a lie.  "void *" is the generic pointer type, "char *" in most
# existing implementations.  "const" is the keyword marking read-only
# variables and parameters, unimplemented in most existing implementations.
# These things need to be defined this way because they must be fitted into
# both the .h files and the .c files; see the make instructions for string.h
# in the Makefile.
*/
#define SIZET		int
#define VOIDSTAR 	char *
#define LVOIDSTAR 	char*
#define CONST  


/*
 * String functions.
 */

VOIDSTAR memcpy(/*VOIDSTAR dst, const VOIDSTAR src, SIZET size*/);
VOIDSTAR memccpy(/*VOIDSTAR dst, const VOIDSTAR src, int ucharstop, SIZET size*/);
char *strcpy(/*char *dst, const char *src*/);
char *strncpy(/*char *dst, const char *src, SIZET size*/);
char *strcat(/*char *dst, const char *src*/);
char *strncat(/*char *dst, const char *src, SIZET size*/);
int memcmp(/*const VOIDSTAR s1, const VOIDSTAR s2, SIZET size*/);
int strcmp(/*const char *s1, const char *s2*/);
int strncmp(/*const char *s1, const char *s2, SIZET size*/);
VOIDSTAR memchr(/*const VOIDSTAR s, int ucharwanted, SIZET size*/);
char *strchr(/*const char *s, int charwanted*/);
SIZET strcspn(/*const char *s, const char *reject*/);
char *strpbrk(/*const char *s, const char *breakat*/);
char *strrchr(/*const char *s, int charwanted*/);
SIZET strspn(/*const char *s, const char *accept*/);
char *strstr(/*const char *s, const char *wanted*/);
char *strtok(/*char *s, const char *delim*/);
VOIDSTAR memset(/*VOIDSTAR s, int ucharfill, SIZET size*/);
SIZET strlen(/*const char *s*/);

/*
 * V7 and Berklix compatibility.
 */
char *index(/*const char *s, int charwanted*/);
char *rindex(/*const char *s, int charwanted*/);
int bcopy(/*const char *src, char *dst, int length*/);
int bcmp(/*const char *s1, const char *s2, int length*/);
int bzero(/*char *dst, int length*/);

/*
 * Putting this in here is really silly, but who am I to argue with X3J11?
 */
char *strerror(/*int errnum*/);
