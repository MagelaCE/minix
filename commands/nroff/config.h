#ifndef CONFIG_H
#define CONFIG_H

/*
 *	for diffent os, define tos, unix, or minix. for gemdos, pick
 *	a compiler (alcyon, mwc, etc). see makefile for VERSFLAGS.
 *
 *	for atari TOS, do:	cc -Dtos -Dalcyon ...
 *
 *	for minix, do:		cc -Dminix -DatariST ...	(ST minix)
 *				cc -Dminix ...			(PC minix)
 *
 *	for unix, do:		cc -Dunix ...			(generic)
 *				cc -Dunix -DBSD...		(BSD)
 *
 *	nroff uses index/rindex. you may need -Dindex=strchr -Drindex=strrchr
 *	as well. this file is included in "nroff.h" which gets included in all
 *	sources so any definitions you need should be added here.
 *
 *	all os-dependent code is #ifdef'ed with GEMDOS, MINIX_ST, MINIX_PC,
 *	MINIX, or UNIX. most of the differences deal with i/o only.
 */
#ifdef ALCYON
# ifndef tos
#  define tos
# endif
#endif

#ifdef tos
# define GEMDOS
# undef minix
# undef unix
# undef _MINIX
# undef MINIX_ST
# undef MINIX_PC
# undef UNIX
#define register
#endif

#ifdef alcyon
# ifndef ALCYON
#  define ALCYON			/* for gemdos version, alcyon C */
# endif
# ifndef GEMDOS
#  define GEMDOS
# endif
#endif

#ifdef minix
# define register
# define _MINIX
# undef tos
# undef unix
# undef GEMDOS
# undef MINIX_ST
# undef MINIX_PC
# undef UNIX
# ifdef atariST
#  define MINIX_ST
# else
#  define MINIX_PC
# endif
#endif

#ifdef unix
# define register
# undef tos
# undef minix
# undef GEMDOS
# undef MINIX_ST
# undef MINIX_PC
# ifndef UNIX
#  define UNIX
# endif
#endif

#endif /*CONFIG_H*/

