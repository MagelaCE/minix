/*
 * The defines in this file establish the environment we're compiling
 * in. Set these appropriately before compiling the editor.
 */

/*
 * One (and only 1) of the following defines should be uncommented.
 * Most of the code is pretty machine-independent. Machine dependent
 * code goes in a file like tos.c or unix.c. The only other place
 * where machine dependent code goes is term.h for escape sequences.
 */

/* #define	ATARI			/* For the Atari ST */
/* #define	UNIX			/* System V or BSD */
/* #define	OS2			/* Microsoft OS/2 1.1 */
/* #define	DOS			/* MSDOS 3.3 (on AT) */

/*
 * If UNIX is defined above, then BSD may be defined.
 */
#ifdef	UNIX
/* #define	BSD			/* Berkeley UNIX */
#endif

/*
 * If ATARI is defined, MINIX may be defined. Otherwise, the editor
 * is set up to compile using the Sozobon C compiler under TOS.
 */
#ifdef	ATARI
#define	MINIX			/* Minix for the Atari ST */
#endif

/*
 * If HELP is defined, the :help command shows a vi command summary.
 * Help costs on the PC minix about 900 Bytes text and 5000 Bytes data !!
 */
/* #define	HELP			/* enable help command */

/*
 * Use of termcap is optional in some environments. If available, it
 * is enabled by the following define. Check the appropriate system-
 * dependent source file to see if termcap is supported.
 */
/* #define	TERMCAP			/* enable termcap support */

/*
 * The yank buffer is still static, but its size can be specified
 * here to override the default of 4K.
 */
/* #define	YBSIZE	8192		/* yank buffer size */

/*
 * STRCSPN should be defined if the target system doesn't have the
 * routine strcspn() available. See regexp.c for details.
 */


#ifdef	MINIX
#define	STRCSPN
#endif

