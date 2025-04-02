static	char	RCSid[] =
"$Header: main.c,v 1.5 88/11/10 13:28:38 tony Exp $";

/*
 * The main routine and routines to deal with the input buffer.
 *
 * $Log:	main.c,v $
 * Revision 1.5  88/11/10  13:28:38  tony
 * Moved the call to do_mlines() to edit() so that it gets called after
 * the various position pointers have been initialized.
 * 
 * Revision 1.4  88/11/10  08:58:40  tony
 * Added code to scan for modelines in the file currently being edited.
 * 
 * Revision 1.3  88/11/01  21:32:29  tony
 * Changed the RBSIZE macro (and comment) since it is no longer tied to
 * the yank buffer size.
 * 
 * Revision 1.2  88/08/26  08:45:14  tony
 * Misc. changes to make lint happy.
 * 
 * Revision 1.1  88/03/20  21:08:18  tony
 * Initial revision
 * 
 *
 */

#include "stevie.h"

int Rows;		/* Number of Rows and Columns */
int Columns;		/* in the current window. */

char *Realscreen = NULL;	/* What's currently on the screen, a single */
				/* array of size Rows*Columns. */
char *Nextscreen = NULL;	/* What's to be put on the screen. */

char *Filename = NULL;	/* Current file name */

LPTR *Filemem;		/* Pointer to the first line of the file */

LPTR *Filetop;		/* Line 'above' the start of the file */

LPTR *Fileend;		/* Pointer to the end of the file in Filemem. */
			/* (It points to the byte AFTER the last byte.) */

LPTR *Topchar;		/* Pointer to the byte in Filemem which is */
			/* in the upper left corner of the screen. */

LPTR *Botchar;		/* Pointer to the byte in Filemem which is */
			/* just off the bottom of the screen. */

LPTR *Curschar;		/* Pointer to byte in Filemem at which the */
			/* cursor is currently placed. */

int Cursrow, Curscol;	/* Current position of cursor */

int Cursvcol;		/* Current virtual column, the column number of */
			/* the file's actual line, as opposed to the */
			/* column number we're at on the screen.  This */
			/* makes a difference on lines that span more */
			/* than one screen line. */

int Curswant = 0;	/* The column we'd like to be at. This is used */
			/* try to stay in the same column through up/down */
			/* cursor motions. */

bool_t set_want_col;	/* If set, then update Curswant the next time */
			/* through cursupdate() to the current virtual */
			/* column. */

int State = NORMAL;	/* This is the current state of the command */
			/* interpreter. */

int Prenum = 0;		/* The (optional) number before a command. */

LPTR *Insstart;		/* This is where the latest insert/append */
			/* mode started. */

bool_t Changed = 0;	/* Set to 1 if something in the file has been */
			/* changed and not written out. */

char Redobuff[1024];	/* Each command should stuff characters into this */
			/* buffer that will re-execute itself. */

char Insbuff[1024];	/* Each insertion gets stuffed into this buffer. */

int Ninsert = 0;	/* Number of characters in the current insertion. */
char *Insptr = NULL;

char **files;		/* list of input files */
int  numfiles;		/* number of input files */
int  curfile;		/* number of the current file */

static void
usage()
{
	fprintf(stderr, "usage: stevie [file ...]\n");
	fprintf(stderr, "       stevie -t tag\n");
	fprintf(stderr, "       stevie +[num] file\n");
	fprintf(stderr, "       stevie +/pat  file\n");
	exit(1);
}

main(argc,argv)
int	argc;
char	*argv[];
{
	char	*initstr, *getenv();	/* init string from the environment */
	char	*tag = NULL;		/* tag from command line */
	char	*pat = NULL;		/* pattern from command line */
	int	line = -1;		/* line number from command line */

	/*
	 * Process the command line arguments.
	 */
	if (argc > 1) {
		switch (argv[1][0]) {
		
		case '-':			/* -t tag */
			if (argv[1][1] != 't')
				usage();

			if (argv[2] == NULL)
				usage();

			Filename = NULL;
			tag = argv[2];
			numfiles = 1;
			break;

		case '+':			/* +n or +/pat */
			if (argv[1][1] == '/') {
				if (argv[2] == NULL)
					usage();
				Filename = strsave(argv[2]);
				pat = &(argv[1][1]);
				numfiles = 1;

			} else if (isdigit(argv[1][1]) || argv[1][1] == NUL) {
				if (argv[2] == NULL)
					usage();
				Filename = strsave(argv[2]);
				numfiles = 1;

				line = (isdigit(argv[1][1])) ?
					atoi(&(argv[1][1])) : 0;
			} else
				usage();

			break;

		default:			/* must be a file name */
			Filename = strsave(argv[1]);
			files = &(argv[1]);
			numfiles = argc - 1;
			break;
		}
	} else {
		Filename = NULL;
		numfiles = 1;
	}
	curfile = 0;

	if (numfiles > 1)
		fprintf(stderr, "%d files to edit\n", numfiles);

	windinit();

	/*
	 * Allocate LPTR structures for all the various position pointers
	 */
	if ((Filemem = (LPTR *) malloc(sizeof(LPTR))) == NULL ||
	    (Filetop = (LPTR *) malloc(sizeof(LPTR))) == NULL ||
	    (Fileend = (LPTR *) malloc(sizeof(LPTR))) == NULL ||
	    (Topchar = (LPTR *) malloc(sizeof(LPTR))) == NULL ||
	    (Botchar = (LPTR *) malloc(sizeof(LPTR))) == NULL ||
	    (Curschar = (LPTR *) malloc(sizeof(LPTR))) == NULL ||
	    (Insstart = (LPTR *) malloc(sizeof(LPTR))) == NULL ) {
		fprintf(stderr, "Can't allocate data structures\n");
		windexit(0);
	}

	screenalloc();
	filealloc();		/* Initialize Filemem, Filetop, and Fileend */

	screenclear();

	if ((initstr = getenv("EXINIT")) != NULL) {
		char *lp, buf[128];

		if ((lp = getenv("LINES")) != NULL) {
			sprintf(buf, "%s lines=%s", initstr, lp);
			readcmdline(':', buf);
		} else
			readcmdline(':', initstr);
	}

	if (Filename != NULL) {
		if (readfile(Filename, Filemem, FALSE))
			filemess("[New File]");
	} else
		msg("Empty Buffer");

	setpcmark();

	updatescreen();
	
	if (tag) {
		stuffin(":ta ");
		stuffin(tag);
		stuffin("\n");

	} else if (pat) {
		stuffin(pat);
		stuffin("\n");

	} else if (line >= 0) {
		if (line > 0)
			stuffnum(line);
		stuffin("G");
	}

	edit();

	windexit(0);

	return 1;		/* shouldn't be reached */
}

#define	RBSIZE	1024
static char getcbuff[RBSIZE];
static char *getcnext = NULL;

void
stuffin(s)
char *s;
{
	if ( getcnext == NULL ) {
		strcpy(getcbuff,s);
		getcnext = getcbuff;
	} else
		strcat(getcbuff,s);
}

void
stuffnum(n)
int	n;
{
	char	buf[32];

	sprintf(buf, "%d", n);
	stuffin(buf);
}

/*VARARGS1*/
void
addtobuff(s,c1,c2,c3,c4,c5,c6)
char *s;
char c1, c2, c3, c4, c5, c6;
{
	char *p = s;
	if ( (*p++ = c1) == NUL )
		return;
	if ( (*p++ = c2) == NUL )
		return;
	if ( (*p++ = c3) == NUL )
		return;
	if ( (*p++ = c4) == NUL )
		return;
	if ( (*p++ = c5) == NUL )
		return;
	if ( (*p++ = c6) == NUL )
		return;
}

int
vgetc()
{
	int	c;

	/*
	 * inchar() may map special keys by using stuffin(). If it does
	 * so, it returns -1 so we know to loop here to get a real char.
	 */
	do {
		if ( getcnext != NULL ) {
			int nextc = *getcnext++;
			if ( *getcnext == NUL ) {
				*getcbuff = NUL;
				getcnext = NULL;
			}
			return(nextc);
		}
		c = inchar();
	} while (c == -1);

	return c;
}

#if 0
int
vpeekc()
{
	if ( getcnext != NULL )
		return(*getcnext);
	return(-1);
}
#endif

/*
 * anyinput
 *
 * Return non-zero if input is pending.
 */

bool_t
anyinput()
{
	return (getcnext != NULL);
}

/*
 * do_mlines() - process mode lines for the current file
 *
 * Returns immediately if the "ml" parameter isn't set.
 */
#define	NMLINES	5	/* no. of lines at start/end to check for modelines */

void
do_mlines()
{
	void	chk_mline();
	int	i;
	LPTR	*p;

	if (!P(P_ML))
		return;

	p = Filemem;
	for (i=0; i < NMLINES ;i++) {
		chk_mline(p->linep->s);
		if ((p = nextline(p)) == NULL)
			break;
	}

	if ((p = prevline(Fileend)) == NULL)
		return;

	for (i=0; i < NMLINES ;i++) {
		chk_mline(p->linep->s);
		if ((p = prevline(p)) == NULL)
			break;
	}
}

/*
 * chk_mline() - check a single line for a mode string
 */
static void
chk_mline(s)
register char	*s;
{
	register char	*cs;		/* local copy of any modeline found */
	register char	*e;

	for (; *s != NUL ;s++) {
		if (strncmp(s, "vi:", 3) == 0 || strncmp(s, "ex:", 3) == 0) {
			cs = strsave(s+3);
			if ((e = strchr(cs, ':')) != NULL) {
				*e = NUL;
				readcmdline(':', cs);
			}
			free(cs);
		}
	}
}
