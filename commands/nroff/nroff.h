#ifndef NRO_H
#define NRO_H

#ifdef alcyon
#define ALCYON				/* for gemdos version, alcyon C */
					/* otherwise minix/unix */
#endif

/*
 *	nroff.h - stuff for nroff
 *
 *	adapted for atariST/TOS by Bill Rosenkranz 10/89
 *	net:	rosenkra@hall.cray.com
 *	CIS:	71460,17
 *	GENIE:	W.ROSENKRANZ
 *
 *	things to look for here:
 *	1) TMAC definition for default macro package lib
 *	2) configuration sizes (see _STKSIZ below if alcyon/dri)
 *	3) libc should have getenv() and ctime()
 *	4) look in version.h for *printer file name (included below)
 *
 *	original author:
 *
 *	Stephen L. Browning
 *	5723 North Parker Avenue
 *	Indianapolis, Indiana 46220
 *
 *	history:
 *
 *	- Originally written in BDS C;
 *	- Adapted for standard C by W. N. Paul
 *	- Heavily hacked up to conform to "real" nroff by Bill Rosenkranz
 */

#include <ctype.h>

/*
 *	default prefix of macro files. files will be of the form "tmac.an"
 *	(for -man), "tmac.s" (for -ms), "tmac.e" (for -me), etc. first
 *	checks environment for TMACDIR which would be path (e.g. "c:\lib\tmac"
 *	or ".", no trailing slash char!).
 */
#ifdef ALCYON
#define TMACFULL	"c:\\lib\\tmac\\tmac."
#define TMACPRE		"\\tmac."
#else /*unix/minix???*/
#define TMACFULL	"/usr/lib/tmac/tmac."
#define TMACPRE		"/tmac."
#endif

/*
 *	command codes...
 */
#define MACRO		0	/* macro definition */
#define BP	 	1	/* begin page */
#define BR	 	2	/* break */
#define CE	 	3	/* center */
#define FI	 	4	/* fill	*/
#define FO	 	5	/* footer */
#define HE	 	6	/* header */
#define IN	 	7	/* indent */
#define LS	 	8	/* line spacing	*/
#define NF	 	9	/* no fill */
#define PL		10	/* page length */
#define RM		11	/* remove macro */
#define SP		12	/* line space */
#define TI		13	/* temp indent */
#define UL		14	/* underline */
#define JU		15	/* justify */
#define NJ		16	/* no justify */
#define M1		17	/* top margin */
#define M2		18	/* second top margin */
#define M3		19	/* first bottom margin */
#define M4		20	/* bottom-most margin */
#define BS		21	/* allow/disallow '\b' in output */
#define NE		22	/* need n lines */
#define PC		23	/* page number character (%) */
#define CC		24	/* control character (.) */
#define PO		25	/* page offset */
#define BO		26	/* bold face */
#define EH		27	/* header for even numbered pages */
#define OH		28	/* header for odd numbered pages */
#define EF		29	/* footer for even numbered pages */
#define OF		30	/* footer for odd numbered pages */
#define SO		31	/* source file */
#define CU		32	/* continuous underline	*/
#define DE		33	/* define macro	*/
#define EN		34	/* end macro definition	*/
#define NR		35	/* set number register */
#define EC		36	/* escape character (\) */
#define FT		37	/* font change (R,B,I,S,P) */
#define EO		38	/* turn escape parsing off */
#define LL		39	/* line length (same as RM) */
#define FL		40	/* flush output NOW */
#define PN		41	/* page number for next page */
#define RR		42	/* remove register */
#define C2		43	/* nobreak char */
/*	these 3 not done yet... */
#  define TR		44	/* translate character */
#  define LT		45	/* length of title */
#  define FC		46	/* field delimeter */
#define TL		47	/* like HE */
#define AF		48	/* assign format to nr */
#define AD		49	/* adjust line */
#define NA		50	/* no adjust */
#define DS		51	/* define string */
#define PM		52	/* print macro names */
#define IF		53	/* if */
#define IE		54	/* if/else */
#define EL		55	/* else */
/*	these aren't done yet either... */
#  define PS		56	/* point size */
#  define SS		57	/* space char size */
#  define CS		58	/* constant char space */
#  define BD		59	/* bold font */
#  define FP		60	/* font position */
#  define MK		61	/* mark vertical place */
#  define RT		62	/* return to marked vert place */
#  define VS		63	/* vertical baseline spacing */
#  define SV		64	/* save vertical distance */
#  define OS		65	/* output saved vertical distance */
#  define NS		66	/* no-space mode */
#  define RS		67	/* restore spacing mode */
#  define AM		68	/* append to macro */
#  define AS		69	/* append to string */
#  define RN		70	/* rename */
#  define DI		71	/* divert to macro */
#  define DA		72	/* divert/append to macro */
#  define WH		73	/* set location trap */
#  define CH		74	/* change trap location */
#  define DT		75	/* set diversion trap */
#  define IT		76	/* set input line trap */
#  define EM		77	/* end macro */
#  define TA		78	/* tab settings */
#  define TC		79	/* tab repetition char */
#  define LC		80	/* leader repetition char */
#  define LG		81	/* ligature mode */
#  define UF		82	/* underline font */
#  define NH		83	/* no hyphenation */
#  define HY		84	/* hyphenate */
#  define HC		85	/* hyphenation indication char */
#  define HW		86	/* hyphenation exception words */
#  define NM		87	/* number mode */
#  define NN		88	/* no number next lines */
#  define EV		89	/* environment switch */
#  define RD		90	/* read insertion */
#  define EX		91	/* exit */
#  define NX		92	/* next file */
#  define PI		93	/* pipe to program */
#  define MC		94	/* set margin char */
#  define TM		95	/* print to terminal */
#  define IG		96	/* ignore */


#define COMMENT		1000	/* comment (.\") */

#define UNKNOWN		-1

/*
 *	MAXLINE is set to a value slightly larger than twice the longest
 *	expected input line. Because of the way underlining is handled, the
 *	input line which is to be underlined, can almost triple in length.
 *	Unlike normal underlining and boldfacing, continuous underlining
 *	affects all characters in the buffer, and represents the worst case
 *	condition.  If the distance between the left margin and the right
 *	margin is greater than about 65 characters, and continuous underlining
 *	is in effect, there is a high probability of buffer overflow.
 */
#define MAXLINE		200
#define PAGELEN	 	66
#define PAGEWIDTH 	80
#define HUGE		256
#define INFINITE	32000
#define LEFT		0	/* indecies into hdr margin lim arrays*/
#define RIGHT		1
#define Nfiles		4	/* nesting depth for input files */

/*
 *	The following parameters may be defined elsewhere so undef/def
 */
#undef min
#undef max
#undef YES
#define YES		1
#undef NO
#define NO		0
#undef ERR
#define ERR		-1
#define EOS 		'\0'
#undef FALSE
#define FALSE 		0
#undef TRUE
#define TRUE 		!FALSE
#undef OK
#define OK 		!ERR

#define ADJ_OFF		0
#define ADJ_LEFT	1
#define ADJ_RIGHT	2
#define ADJ_CENTER	3
#define ADJ_BOTH	4


/*
 *	The parameter values selected for macro definitions are somewhat
 *	arbitrary.  MACBUF is the storage area for both macro names and
 *	definitions.  Since macro processing is handled by pushing back
 *	the expansion into the input buffer, the longest possible expansion
 *	would be MAXLINE characters.  Allowing for argument expansion,
 *	MXMLEN was chosen slightly less than MAXLINE. It is assumed that
 *	most macro definitions will not exceed 20 characters, hence MXMDEF
 *	of 150.
 */

#define MXMDEF		150	/* max no. of macro definitions */
#define MACBUF		32000	/* macro definition buffer size */
#define MXMLEN		250	/* max length of each macro def */
#define MNLEN		10	/* max length of macro name */
#define MAXREGS		100	/* max number of registers (2-char) */

struct macros
{
	char   *mnames[MXMDEF];	/* table of ptrs to macro names */
	int	lastp;		/* index to last mname	*/
	char   *emb;		/* next char avail in macro defn buf */
	char 	mb[MACBUF];	/* table of macro definitions */
	char   *ppb;		/* pointer into push back buffer */
	char 	pbb[MAXLINE];	/* push back buffer */
};


#define RF_READ		0x0001	/* register flags */
#define RF_WRITE	0x0002

struct regs
{
	char	rname[4];	/* 2-char register name */
	int	rauto;		/* autoincrement value */
	int	rval;		/* current value of the register */
	int	rflag;		/* register flags */
	char	rfmt;		/* register format (1,a,A,i,I,...) */
};



/*
 *	control parameters for nro
 */
struct docctl
{
	int	fill;		/* fill if YES, init = YES */
	int	dofnt;		/* handle font change, init = YES */
	int	lsval;		/* current line spacing, init = 1 */
	int	inval;		/* current indent, >= 0, init = 0 */
	int	rmval;		/* current right margin, init = 60 */
	int	llval;		/* current line length, init = 60 */
	int	ltval;		/* current title length, init = 60 */
	int	tival;		/* current temp indent, init = 0 */
	int	ceval;		/* number of lines to center, init = 0 */
	int	ulval;		/* number of lines to underline, init = 0 */
	int	cuval;		/* no lines to continuously uline, init = 0 */
	int	juval;		/* justify if YES, init = YES */
	int	adjval;		/* adjust type, init = ADJ_BOTH */
	int	boval;		/* number of lines to bold face, init = 0 */
	int	bsflg;		/* can output contain '\b', init = FALSE */
	int	prflg;		/* print on or off, init = TRUE */
	int	sprdir;		/* direction for spread(), init = 0 */
	int	flevel;		/* nesting depth for source cmd, init = 0 */
	int	lastfnt;	/* previous used font */
	int	thisfnt;	/* current font, init = 1 (1=R,2=I,3=B,4=S) */
	int	escon;		/* whether esc parsing is on, init = YES */
	int	nr[26];		/* number registers */
	int	nrauto[26];	/* number registers auto increment */
	char	nrfmt[26];	/* number registers formats, init = '1' */
				/* input code how printed */
				/* 1     '1'  1,2,3,... */
				/* a     'a'  a,b,c,...,aa,bb,cc,... */
				/* A     'A'  A,B,C,...,AA,BB,CC,... */
				/* i     'i'  i,ii,iii,iv,v... */
				/* I     'I'  I,II,III,IV,V... */
				/* 01     2   01,02,03,... */
				/* 001    3   001,002,003,... */
				/* 0..1   8   00000001,00000002,... */
	char	pgchr;		/* page number character, init = '%' */
	char	cmdchr;		/* command character, init = '.' */
	char	escchr;		/* escape char, init = '\' */
	char	nobrchr;	/* nobreak char, init = '\'' */
};


/*
 *	output buffer control parameters
 */
struct cout
{
	int	outp;		/* next avail char pos in outbuf, init = 0 */
	int	outw;		/* width of text currently in buffer */
	int	outwds;		/* number of words in buffer, init = 0 */
	int	lpr;		/* output to printer, init = FALSE */
	int	outesc;		/* number of escape char on this line */
	char	outbuf[MAXLINE];/* output of filled text */
};


/*
 *	page control parameters for nroff
 */
struct page
{
	int	curpag;		/* current output page number, init =0 */
	int	newpag;		/* next output page number, init = 1 */
	int	lineno;		/* next line to be printed, init = 0 */
	int	plval;		/* page length in lines, init = 66 */
	int	m1val;		/* margin before and including header */
	int	m2val;		/* margin after header */
	int	m3val;		/* margin after last text line */
	int	m4val;		/* bottom margin, including footer */
	int	bottom;		/* last live line on page
					= plval - m3val - m4val	*/
	int	offset;		/* page offset from left, init = 0 */
	int	frstpg;		/* first page to print, init = 0 */
	int	lastpg;		/* last page to print, init = 30000 */
	int	ehlim[2];	/* left/right margins for headers/footers */
	int	ohlim[2];	/* init = 0 and PAGEWIDTH */
	int	eflim[2];
	int	oflim[2];
	char	ehead[MAXLINE];	/* top of page title, init = '\n' */
	char	ohead[MAXLINE];
	char	efoot[MAXLINE];	/* bottom of page title, init = '\n' */
	char	ofoot[MAXLINE];
};



/*
 *	forward refs from libc
 */
char   *getenv ();
char   *ctime ();		/* NOTE: my ctime is slightly different */

/*
 *	forward refs from nroff
 */
char   *getmac ();
char   *getstr ();
char   *skipwd ();
char   *skipbl ();
char   *getfield ();

int	comand ();
int	comtyp ();
int	gettl ();
int	getval ();
int	set ();
int	expesc ();
int	specialchar ();
int	fontchange ();
int	getlin ();
int	ngetc ();
int	pbstr ();
int	putbak ();
int	prchar ();
int	put ();
int	putlin ();
int	atod ();
int	robrk ();
int	ctod ();
int	space ();
int	getwrd ();
int	countesc ();
int	itoda ();
int	itoROMAN ();
int	itoroman ();
int	itoLETTER ();
int	itoletter ();
int	min ();
int	max ();
int	defmac ();
int	colmac ();
int	putmac ();
int	maceval ();
int	main ();
int	usage ();
int	init ();
int	pswitch ();
int	profile ();
int	text ();
int	bold ();
int	center ();
int	expand ();
int	justcntr ();
int	justleft ();
int	justrite ();
int	leadbl ();
int	pfoot ();
int	phead ();
int	puttl ();
int	putwrd ();
int	skip ();
int	spread ();
int	strkovr ();
int	underl ();
int	width ();

int	findreg ();
int	set_ireg ();



/*
 *	globals. define NRO_MAIN in main.c to define globals there. else
 *	you get extern.
 */

#ifdef NRO_MAIN

struct docctl		dc;
struct page		pg;
struct cout		co;
struct macros		mac;
struct regs		rg[MAXREGS];
FILE		       *pout;
FILE		       *err_stream;
FILE		       *dbg_stream;
FILE		       *sofile[Nfiles+1];
int			hold_screen;
int			debugging;
char		       *dbgfile = "nroff.dbg";
#ifdef ALCYON
char		       *printer = "prn:";	/* this WON'T work!!! */
#else
char		       *printer = "/dev/lp";	/* this probably won't */
#endif

#include "version.h"			/* for myname and version */

#ifdef ALCYON
/*
 *	this SHOULD be big enough for most needs. only used by startup
 *	code (gemstart.o or crt0.o)
 */
long			_STKSIZ = 0x00055555L;	/* about 350000 */
#endif


#else /*NRO_MAIN*/

extern struct docctl	dc;
extern struct page	pg;
extern struct cout	co;
extern struct macros	mac;
extern struct regs	rg[MAXREGS];
extern FILE	       *pout;
extern FILE	       *err_stream;
extern FILE	       *dbg_stream;
extern FILE	       *sofile[Nfiles+1];
extern int		hold_screen;
extern int		debugging;
extern char	       *dbgfile;
extern char	       *printer;
extern char	       *myname;
extern char	       *version;

#endif /*NRO_MAIN*/
 
#endif /*NRO_H*/

