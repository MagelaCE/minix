#ifndef VERSION_H
#define VERSION_H

/*
 *	to get around no valid argv[0]...
 */
char	       *myname  = "nroff";

#ifdef ALCYON
char	       *version = "nroff (TOS) v0.90 ALPHA 11/12/89 wjr";
#else
char	       *version = "nroff (Minix) v0.90 ALPHA 11/12/89 wjr";
#endif


#endif /*VERSION_H*/
