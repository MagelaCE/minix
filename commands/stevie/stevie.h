/*
 * $Header: stevie.h,v 1.15 88/11/10 08:59:10 tony Exp $
 *
 * Main header file included by all source files.
 *
 * $Log:	stevie.h,v $
 * Revision 1.15  88/11/10  08:59:10  tony
 * Added a declaration for the routine do_mlines() in main.c that checks
 * for mode lines in a file.
 * 
 * Revision 1.14  88/10/28  13:59:36  tony
 * Added missing endif.
 * 
 * Revision 1.13  88/10/27  08:23:47  tony
 *  Also added declarations for the new system-
 * dependent routines flushbuf() and doshell().
 * 
 * Revision 1.12  88/10/06  10:13:08  tony
 * Added a declaration for fixname().
 * 
 * Revision 1.11  88/08/30  20:37:10  tony
 * After much prodding from Mark, I finally added support for replace mode.
 * 
 * Revision 1.10  88/08/26  08:45:51  tony
 * Misc. changes to make lint happy.
 * 
 * Revision 1.9  88/07/09  20:38:47  tony
 * Added declarations for new routines in undo.c supporting the 'U' command.
 * 
 * Revision 1.8  88/06/26  14:52:33  tony
 * Added missing declaration for prt_line() in screen.c
 * 
 * Revision 1.7  88/06/26  14:49:21  tony
 * Added a declaration for the new routine, doglog(), in search.c.
 * 
 * Revision 1.6  88/06/20  14:52:13  tony
 * Merged in changes for BSD Unix sent in by Michael Lichter.
 * 
 * Revision 1.5  88/05/03  14:39:15  tony
 * Minor change for DOS support.
 * 
 * Revision 1.4  88/04/29  14:46:39  tony
 * Changed the declarations of repsearch() and dosearch() from void to bool_t.
 * 
 * Revision 1.3  88/04/24  21:35:56  tony
 * Added a declaration for the new routine dosub() in search.c.
 * 
 * Revision 1.2  88/03/21  16:46:57  tony
 * Moved the environmental defines out to a file named "env.h" which is
 * not maintained with RCS.
 * 
 * Revision 1.1  88/03/20  21:05:17  tony
 * Initial revision
 * 
 *
 */

#include "env.h"	/* defines to establish the compile-time environment */

#include <stdio.h>
#include <ctype.h>

#ifdef	BSD

#include <strings.h>
#define strchr index

#else

#ifdef	MINIX

extern	char	*strchr();
extern	char	*strrchr();
extern	char	*strcpy();
extern	char	*strcat();
extern	int	strlen();

#else
#include <string.h>
#endif

#endif

#include "ascii.h"
#include "keymap.h"
#include "param.h"
#include "term.h"

extern	char	*strchr();

#define NORMAL 0
#define CMDLINE 1
#define INSERT 2
#define REPLACE 3
#define FORWARD 4
#define BACKWARD 5

/*
 * Boolean type definition and constants
 */
typedef	short	bool_t;

#ifndef	TRUE
#define	FALSE	(0)
#define	TRUE	(1)
#endif

/*
 * SLOP is the amount of extra space we get for text on a line during
 * editing operations that need more space. This keeps us from calling
 * malloc every time we get a character during insert mode. No extra
 * space is allocated when the file is initially read.
 */
#define	SLOP	10

/*
 * LINEINC is the gap we leave between the artificial line numbers. This
 * helps to avoid renumbering all the lines every time a new line is
 * inserted.
 */
#define	LINEINC	10

#define CHANGED		Changed=TRUE
#define UNCHANGED	Changed=FALSE

struct	line {
	struct	line	*prev, *next;	/* previous and next lines */
	char	*s;			/* text for this line */
	int	size;			/* actual size of space at 's' */
	unsigned long	num;		/* line "number" */
};

#define	LINEOF(x)	((x)->linep->num)

struct	lptr {
	struct	line	*linep;		/* line we're referencing */
	int	index;			/* position within that line */
};

typedef	struct line	LINE;
typedef	struct lptr	LPTR;

struct charinfo {
	char ch_size;
	char *ch_str;
};

extern struct charinfo chars[];

extern int State;
extern int Rows;
extern int Columns;
extern char *Realscreen;
extern char *Nextscreen;
extern char *Filename;
extern LPTR *Filemem;
extern LPTR *Filetop;
extern LPTR *Fileend;
extern LPTR *Topchar;
extern LPTR *Botchar;
extern LPTR *Curschar;
extern LPTR *Insstart;
extern int Cursrow, Curscol, Cursvcol, Curswant;
extern bool_t set_want_col;
extern int Prenum;
extern bool_t Changed;
extern char Redobuff[], Insbuff[];
extern char *Insptr;
extern int Ninsert;

extern char *malloc(), *strcpy();

/*
 * alloc.c
 */
char	*alloc(), *strsave();
void	screenalloc(), filealloc(), freeall();
LINE	*newline();
bool_t	bufempty(), buf1line(), lineempty(), endofline(), canincrease();

/*
 * cmdline.c
 */
void	readcmdline(), dotag(), msg(), emsg(), smsg(), gotocmd(), wait_return();

/*
 * edit.c
 */
void	edit(), insertchar(), getout(), scrollup(), scrolldown(), beginline();
bool_t	oneright(), oneleft(), oneup(), onedown();

/*
 * fileio.c
 */
void	filemess(), renum();
bool_t	readfile(), writeit();

/*
 * help.c
 */
bool_t	help();

/*
 * linefunc.c
 */
LPTR	*nextline(), *prevline(), *coladvance();

/*
 * main.c
 */
void	stuffin(), stuffnum(), addtobuff();
void	do_mlines();
int	vgetc();
bool_t	anyinput();

/*
 * mark.c
 */
void	setpcmark(), clrall(), clrmark();
bool_t	setmark();
LPTR	*getmark();

/*
 * misccmds.c
 */
void	opencmd(), fileinfo(), inschar(), delline();
bool_t	delchar();
int	cntllines(), plines();
LPTR	*gotoline();

/*
 * normal.c
 */
void	normal();
char	*mkstr();

/*
 * param.c
 */
void	doset();

/*
 * ptrfunc.c
 */
int	inc(), dec();
int	gchar();
void	pchar(), pswap();
bool_t	lt(), equal(), ltoreq();
#if 0
/* not currently used */
bool_t	gtoreq(), gt();
#endif

/*
 * screen.c
 */
void	updatescreen(), updateline();
void	screenclear(), cursupdate();
void	s_ins(), s_del();
void	prt_line();

/*
 * search.c
 */
void	dosub(), doglob();
bool_t	searchc(), crepsearch(), findfunc(), dosearch(), repsearch();
LPTR	*showmatch();
LPTR	*fwd_word(), *bck_word(), *end_word();

/*
 * undo.c
 */
void	u_save(), u_saveline(), u_clear();
void	u_lcheck(), u_lundo();
void	u_undo();

/*
 * Machine-dependent routines.
 */
int	inchar();
void	flushbuf();
void	outchar(), outstr(), beep();
char	*fixname();
#ifndef	OS2
#ifndef	DOS
void	remove1(), rename1();
#endif
#endif
void	windinit(), windexit(), windgoto();
void	delay();
void	doshell();
