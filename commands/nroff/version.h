#ifndef VERSION_H
#define VERSION_H

/*
 *	to get around no valid argv[0] with some compilers...
 */
char	       *myname  = "nroff";

#ifdef GEMDOS
char	       *version = "nroff (TOS) v0.99 BETA 02/25/90 wjr";
#endif
#ifdef MINIX_ST
char	       *version = "nroff (Minix-ST) v0.99 BETA 02/25/90 wjr";
#endif
#ifdef MINIX_PC
char	       *version = "nroff (Minix-PC) v0.99 BETA 02/25/90 wjr";
#endif
#ifdef UNIX
char	       *version = "nroff (Unix) v0.99 BETA 02/25/90 wjr";
#endif

#endif /*VERSION_H*/
