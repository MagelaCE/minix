/* ELLE - Copyright 1982, 1984, 1987 by Ken Harrenstien, SRI International
 *	This software is quasi-public; it may be used freely with
 *	like software, but may NOT be sold or made part of licensed
 *	products without permission of the author.
 */
/*
 *  EETERM	ELLE Terminal Driver.
 *	Directly supports DM2500, H-19, Omron 8025AG, Coherent/IBM-PC, TVI925.
 *	Others also supported if using TX_TERMCAP.
 */

#include "elle.h"

extern char *tv_stype;	/* If set, specifies terminal type */
extern int tibfmsk;	/* Crock to mask off parity (meta) bit */
char *tvs_bs = "\010";	/* String to send for backspacing */
int tv_padc;		/* Pad character to use */
int tv_cspeed;		/* # msec per char (set from trm_ospeed) */
int tv_type;		/* Index of selected terminal type */

/* Define terminal indices (there may be holes but C preprocessor is too
 * stupid to let us close them).  Should be one TN_ definition for every
 * hardwired terminal type, even though whether or not it is actually
 * compiled depends on which TX_ switches are defined.
 */
#define TN_TERMCAP 0
#define TN_DM2500 1
#define TN_H19	  2
#define TN_OM8025 3
#define TN_COHIBM 4	/* Coherent IBM-PC console */
#define TN_TVI925 5
#define TN_SUNCON 6	/* SUN workstation console */

#if TX_COHIBM	/* If this is defined, */
#if !(TX_H19)
#define TX_H19		/* then make sure H19 is defined also! */
#endif /*-TX_H19*/
#endif /*TX_COHIBM*/

#ifndef TXS_DEFAULT		/* If no default is explicitly specified */
#define TXS_DEFAULT "H19"	/* Then settle for H-19 */
#endif /*TXS_DEFAULT*/


/* Character speed table, indexed by system output speed value (0-017).
 * Value in table is 100 * <# msec used per character>.
 */
int cspdtab[] =
{	/* Val    Idx Baud CPS  Time/char in msec */
	0,	/*  0 Hangup -	----		*/
	13333,	/*  1   50 7.5 133.33  (Baudot)	*/
	10000,	/*  2   75  10 100.0   (Baudot)	*/
	10000,	/*  3  110  10 100.0		*/
	 8200,	/*  4 134.5 12.2 82.0 (IBM2741)	*/
	 6666,	/*  5  150  15 	66.6666 	*/
	 5000,	/*  6  200  20	50.0		*/
	 3333,	/*  7  300  30	33.3333 	*/
	 1666,	/*  8  600  60  16.6666 	*/
	  833,	/*  9 1200 120   8.3333 	*/
	  555,	/* 10 1800 180   5.5555 	*/
	  416,	/* 11 2400 240   4.1666 	*/
	  208,	/* 12 4800 480   2.0833		*/
	  104,	/* 13 9600 960   1.04166	*/
	0,	/* 14 Ext A  ?	 ?		*/
	0	/* 15 Ext B  ?	 ?		*/
};

#if TX_TERMCAP
/* Declarations of termcap strings.  Only EETERM knows about them. */
/* Note that the IA and ID strings are not defined by the TERMCAP doc;
 * their usage here is derived from examining other TERMCAP-using programs.
 * Sigh!!!!
 */
#ifndef TCAPSLEN
#define TCAPSLEN 1024	/* Default size of buffer for TERMCAP strings */
#endif /*-TCAPSLEN*/

char	PC;		/* Pad char */
char	*IM,		/* Insert mode on */
	*DM,		/* Delete mode on */
	*IA,		/* Add line while in insert mode */
	*ID,		/* Delete line while in delete mode */
	*EI,		/* Insert mode off */
	*ED,		/* Delete mode off */
	*AL,		/* Add line */
	*DL,		/* Delete line */
	*CM,		/* Cursor motion */
	*CL,		/* Clear screen */
	*CE,		/* Erase to end of line (CLEOL) */
	*UP,		/* Cursor up */
	*IC,		/* Insert char */
	*IP,		/* Send this after inserting char (?) */
	*DC,		/* Delete char */
	*IS,		/* Terminal initialization string */
	*TE,		/* "string to end programs that use CM" */
	*TI,		/* "string to begin programs that use CM" */
	*BC,		/* Backspace */
	*SO, *SE;	/* Stand-out mode start, end */
#if TXC_VISBEL
char	*VB;		/* Visible bell */
#endif

int tc_km;		/* Set if meta key exists */
char *tc_mm;		/* String to set (turn on) meta-key mode */
char *tc_mo;		/* String to reset (turn off) meta-key mode */
int AM;			/* Set if has auto-wrap */
int tc_bs;		/* Set if can backspace */

int tgetent(), tgetnum(), tgetflag(), tputs();
char *tgetstr(), *tgoto();

int ospeed;		/* External var that termcap wants */

/*
 * There are many other things that must be taken into account.
 * The termcap code here will probably not work for many termcap entries,
 * but the only sure way to find out which ones they are is to try them.
 */
#endif /*TX_TERMCAP*/

/* T_INIT is called once only at program startup, to identify the
 *	terminal type and set up any one-time things.
 * T_ENTER is called after TS_ENTER to set the terminal parameters for
 *	editing (as opposed to normal typeout).  It may be called
 *	several times.
 * T_EXIT is called before TS_EXIT to restore normal typeout modes.
 *	It is called on exit from the program, and perhaps other times.
 */
t_init ()
{
	char *getenv();

	/* Set some default parameters */
	scr_ht = 24;
	scr_wid = 79;
	trm_flags = 0;
	tvc_cin = 1;		/* Assume 1 char per char I/D pos */
	tvc_cdn = 1;
	tvc_pos = 4;		/* Default abs-move cost is 4 chars */
	tvc_bs = 1;		/* Default backspace cost is 1 char */
	tv_cspeed = cspdtab[trm_ospeed];	/* Find # msec per char */

	/* First must determine terminal type, and check for terminals
	 * that are hardwired into ELLE. */
	if(!tv_stype		/* String set in command line args? */
#if !(V6)
	 && !(tv_stype = getenv("TERM"))	/* or given by TERM var? */
#endif /*-V6*/
		) tv_stype = TXS_DEFAULT;	/* No, try using default */
	if(0) ;			/* Sigh, stupid construct */
#if TX_H19
	else if(ustrcmp(tv_stype,"H19")) tv_type = TN_H19;
#endif /*TX_H19*/
#if TX_OM8025
	else if(ustrcmp(tv_stype,"OM8025")) tv_type = TN_OM8025;
#endif /*TX_OM8025*/
#if TX_DM2500
	else if(ustrcmp(tv_stype,"DM2500")) tv_type = TN_DM2500;
	else if(ustrcmp(tv_stype,"DM3025")) tv_type = TN_DM2500;
#endif /*TX_DM2500*/
#if TX_COHIBM
	else if(ustrcmp(tv_stype,"COHIBM")) tv_type = TN_COHIBM;
#endif /*TX_COHIBM*/
#if TX_TVI925
	else if(ustrcmp(tv_stype,"TVI925")) tv_type = TN_TVI925;
#endif /*TX_TVI925*/
#if TX_TERMCAP	/* This should be last thing */
	else if(getcap(tv_stype)) tv_type = TN_TERMCAP;
#endif /*TX_TERMCAP*/
	else
	  {	writez(1,tv_stype);
		writez(1,": unknown terminal type\n");
		exit(1);
	  }
#if TX_SUNCON
	/* Sun uses termcap except for line scrolling for now */
	if(ustrcmp(tv_stype,"SUN")) tv_type = TN_SUNCON;
#endif /*TX_SUNCON*/

	/* Terminal selected, now initialize parameters for it. */
	switch(tv_type)
	  {
#if TX_DM2500
		case TN_DM2500:
			tv_padc = 0177;		/* Use rubout for pad */
			tvc_pos = 3;		/* Only 3 chars for abs mov */
			tvc_ci = 2;
		/*	tvc_cin = 1; */		/* Default is OK */
			tvc_cd = 2;
		/*	tvc_cdn = 1; */		/* Default is OK */
			tvc_ld = 2;
			tvc_ldn = 1;
			tvc_li = 2;
			tvc_lin = 1;
			if(trm_ospeed == 13)	/* If 9600, */
			  {	tvc_cin = 5;		/* Sigh, high cost */
				tvc_cdn = 2;
				tvc_lin = 18;
				tvc_ldn = 2;
			  }
			trm_flags |= IDLIN|IDCHR|CLEOL|METAKEY;
			break;
#endif /*TX_DM2500*/
#if TX_H19
		case TN_H19:			
			trm_flags |= IDLIN|IDCHR|CLEOL;
			tvc_ci = 8;
		/*	tvc_cin = 1; */	/* default is ok */
			tvc_cd = 0;
			tvc_cdn = 2;
		/*	tvc_ld = 0; */	/* Default is OK */
			tvc_ldn = 1 << (trm_ospeed - 7);
		/*	tvc_li = 0; */	/* Default is OK */
			tvc_lin = tvc_ldn;
			break;
#endif /*TX_H19*/
#if TX_COHIBM
		case TN_COHIBM:
			trm_flags |= IDLIN|IDCHR|CLEOL|METAKEY|DIRVID;
			/* Always use lowest possible costs */
		/*	tvc_ci = 0;	/* Default */
			tvc_cin = 2;
		/*	tvc_cd = 0;	/* Default */
			tvc_cdn = 2;
		/*	tvc_ld = 0;	/* Default */
			tvc_ldn = 2;
		/*	tvc_li = 0;	/* Default */
			tvc_lin = 2;
			break;
#endif /*TX_COHIBM*/
#if TX_OM8025
		case TN_OM8025:
			trm_flags |= IDLIN|IDCHR|CLEOL;
			tvc_pos = 6;
		/*	tvc_ci = tvc_cd = 0;	/* Default */
			tvc_cin = 4;
			tvc_cdn = 2;
		/*	tvc_ld = tvc_li = 0	/* Default */
			tvc_ldn = 10;		/* Crude approx */
			tvc_lin = 10;
			if(trm_ospeed > 7)	/* If faster than 300 baud */
				trm_flags &= ~IDLIN;	/* Turn off LID */
			break;
#endif /*TX_OM8025*/
#if TX_TVI925
		case TN_TVI925:
			trm_flags |= IDLIN|IDCHR|CLEOL;
			tvc_ci = tvc_cd = tvc_cin = tvc_cdn
				= tvc_ldn = tvc_lin = 2;
			break;
#endif /*TX_TVI925*/
#if TX_SUNCON
		case TN_SUNCON:
			tvc_lin = tvc_ldn = 1; /* very crude approx. */
			break;
#endif
	  }
	if(tibfmsk < 0)		/* If mask is still default -1, set it. */
		tibfmsk = ((trm_flags&METAKEY) ? 0377 : 0177);
}

/* T_ENTER is called after TS_ENTER to set the terminal parameters for
 *	editing (as opposed to normal typeout).
 *	Standout mode must initially be off.
 */

t_enter()
{	switch(tv_type)
	  {
#if TX_SUNCON
		case TN_SUNCON:
#endif
#if TX_TERMCAP
		case TN_TERMCAP:
#if 0		/* There seems to be some question whether this is needed. */
		/* Imagen claims not. */
			putpad(IS);
#endif /*COMMENT*/
                        putpad(TI);
			if(tc_km) putpad(tc_mm);	/* Use meta if poss */
#if FX_SOWIND
			t_standout(0);		/* Ensure standout mode off */
#endif
			break;
#endif /*TX_TERMCAP*/
#if TX_DM2500
		case TN_DM2500:
			tput(030);	/* Just in case, flush stray modes */
			break;
#endif /*TX_DM2500*/
#if TX_COHIBM
		case TN_COHIBM:		/* Note TN_H19 will exist too */
#endif /*TX_COHIBM*/
#if TX_H19
		case TN_H19:
			/* Enter ZDS (Heath) mode, then
			 * Exit graphics mode (G) Exit ins-char mode (O)
			 * exit rev video mode (q) exit hold-screen mode (\)
			 * set cursor on (y5)
			 */
			tputz("\033[?2h\033G\033O\033q\033\\\033y5");
			/* Set Discard-at-EOL (w)
			 * Set no auto-CR (y9)
			 * Enable 25th line (x1)
			 */
			tputz("\033w\033y9\033x1");
			break;
#endif /*TX_H19*/
	  }
}

/* T_EXIT - Leave editing modes.  This function should restore
**	the terminal's modes to what they were before ELLE was started.
**	Standout mode is turned off.
*/

t_exit()
{
	switch(tv_type)
	  {
#if TX_SUNCON
		case TN_SUNCON:
#endif
#if TX_TERMCAP
		case TN_TERMCAP:
			if(tc_km) putpad(tc_mo);	/* Turn off meta */
			putpad(TE);
			break;
#endif /*TX_TERMCAP*/
#if TX_DM2500
		case TN_DM2500:
			tput(035);	/* Turn on roll mode */
			break;
#endif /*TX_DM2500*/
#if TX_COHIBM
		case TN_COHIBM:		/* If this exists, TN_H19 will too */
#endif /*TX_COHIBM*/
#if TX_H19
		case TN_H19:
			tputz("\033v");		/* Turn EOL-wrap back on */
#if DNTTY
			tputz("\033<");		/* Return to ANSI mode */
#endif /*DNTTY*/
			break;
#endif /*TX_H19*/
	  }
}

/* T_CLEAR() - Clears the screen and homes the cursor.
 *	Always valid - ELLE refuses to support terminals without this.
 */

t_clear ()
{	switch(tv_type)
	  {
#if TX_SUNCON
		case TN_SUNCON:
#endif
#if TX_TERMCAP
		case TN_TERMCAP:
			putnpad(CL,scr_ht);
			break;
#endif /*TX_TERMCAP*/
#if TX_DM2500
		case TN_DM2500:
			tputz("\036\036");	/* Double Master Clear */
			break;
#endif /*TX_DM2500*/
#if TX_COHIBM
		case TN_COHIBM:		/* Note TN_H19 will exist too */
#endif /*TX_COHIBM*/
#if TX_H19
		case TN_H19:
			tputz("\033E");
		/*	tputn(zpadstr,9);	*/
			break;
#endif /*TX_H19*/
#if TX_OM8025
		case TN_OM8025:
			tputz("\033H\033J");	/* Home then CLEOS */
			tpad(1000);		/* One second!!!! */
			break;
#endif /*TX_OM8025*/
#if TX_TVI925
		case TN_TVI925:
			tput(032);	/* ^Z */
			break;
#endif /*TX_TVI925*/
	  }
	curs_lin = curs_col = 0;
}

/* T_CURPOS(y, x) - Absolute move.  Place cursor in given position
 *	regardless of where it currently is.
 *	Updates curs_lin, curs_col.
 *	Always valid -- ELLE refuses to support terminals without this.
 */

t_curpos (lin, col)
register int lin, col;
{
	if(col > scr_wid)		/* Easiest to catch here */
		col = scr_wid;

	/* Do absolute positioning */
	switch(tv_type)
	  {
#if TX_SUNCON
		case TN_SUNCON:
#endif
#if TX_TERMCAP
		case TN_TERMCAP:
			putpad(tgoto(CM, col, lin));
			break;
#endif /*TX_TERMCAP*/
#if TX_DM2500
		case TN_DM2500:
			tput(014);
			tput(col^0140);
			tput(lin^0140);
			break;
#endif /*TX_DM2500*/
#if TX_COHIBM
		case TN_COHIBM:		/* If this exists, TN_H19 will too */
#endif /*TX_COHIBM*/
#if TX_H19
		case TN_H19:
			tputz("\033Y");
			tput(lin+040);
			tput(col+040);
			break;
#endif /*TX_H19*/
#if TX_OM8025
		case TN_OM8025:
	/*		tputz("\033\0175"); */
	/* Losing C compiler produces the string 033 017 065 !!! */
			tput(033); tput(0175);
			tput(0100+((lin+1)>>4));
			tput(0100+((lin+1)&017));
			tput(0100+((col+1)>>4));
			tput(0100+((col+1)&017));
			break;
#endif /*TX_OM8025*/
#if TX_TVI925
		case TN_TVI925:
			tputz("\033=");
			tput(lin+040);
			tput(col+040);
			break;
#endif /*TX_TVI925*/
	  }
	curs_lin = lin;
	curs_col = col;
}

/* T_BACKSPACE() - Back up 1 character position.
 *	Updates curs_col.
 *	Only valid if tvc_bs has a "reasonable" value ( < 1000)
 */

t_backspace()
{	tputz(tvs_bs);
	--curs_col;
}

/* T_BELL() - Ring terminal's bell (or flash something, or whatever).
 *	Forces out all output thus far, to ensure immediate attention.
 *	This used to be an unbuffered feep, but was changed to use normal
 *	output path in order to avoid messing up terminal escape sequences.
 */
t_bell()
{
#if TXC_VISBEL
#if TX_TERMCAP
	if(VB)
	        tputz(VB);		/* Do visible bell if possible */
	else
#endif /*TX_TERMCAP*/
#endif /*TXC_VISBEL*/
        tput(BELL);
        tbufls();       /* Force it out */
}

/* T_CLEOL() - Clear to End Of Line.
 *	Only valid if trm_flags has CLEOL set.
 */

t_cleol ()
{
	switch(tv_type)
	  {
#if TX_SUNCON
		case TN_SUNCON:
#endif
#if TX_TERMCAP
		case TN_TERMCAP:
			putpad(CE);
			break;
#endif /*TX_TERMCAP*/
#if TX_DM2500
		case TN_DM2500:
			tput(027);
			break;
#endif /*TX_DM2500*/
#if TX_COHIBM
		case TN_COHIBM:		/* If this exists, TN_H19 will too */
#endif /*TX_COHIBM*/
#if TX_H19
		case TN_H19:
			tputz("\033K");
			break;
#endif /*TX_H19*/
#if TX_OM8025
		case TN_OM8025:
			tputz("\033K");
			tpad(41);	/* 1/25 sec padding */
			break;
#endif /*TX_OM8025*/
#if TX_TVI925
		case TN_TVI925:
			tputz("\033T");
			break;
#endif /*TX_TVI925*/
	  }
}

/* NOTE: H19 supposedly requires 19 ms for each line during line I/D
 * operations.
 * In actual practice, at 9600 baud 25 pads are necessary (24 wont work!)
 * for both I and D.  Plus esc-E needs 9 pads.
 */

/* T_INSLIN(n, bot) - Insert lines in window.
 *	n   - # blank lines to insert.
 *	bot - # of last line of current window
 *
 *		The current line is moved down and N blank lines inserted.
 *	Lines which are moved past bot are lost.
 *	May leave cursor in random place.
 *	Only valid if trm_flags has IDLIN set.
 */

t_inslin (n, bot)
int   n;			/* number of lines */
int   bot;			/* line number of last line in window */
{	register  i, j;
	int savc,savl;

	if((i = n) <= 0) return;
	if(bot < (scr_ht-1))
	  {	savc = curs_col;
		savl = curs_lin;
		t_curpos(bot-i, 0);
		t_dellin(i, scr_ht);
		t_curpos(savl, savc);
	  }
	switch(tv_type)
	  {
#if TX_TERMCAP
		case TN_TERMCAP:
			if(IA)
			  {	putpad(IM);
				do { putpad(IA);
				  } while(--i);
				putpad(EI);
			  }
			else
				do { putnpad(AL,scr_ht - curs_lin);
				  } while(--i);
			curs_lin = -1;		/* Don't know where we are */
			break;
#endif /*TX_TERMCAP*/
#if TX_SUNCON
		case TN_SUNCON:
			 { 
			char sunstr[ 10];
			sprintf( sunstr, "[%dL", i);
			putnpad( sunstr, scr_ht - curs_lin); 
			curs_lin = -1;		/* Don't know where we are */
			break;
			}
#endif /*TX_SUNCON*/
#if TX_DM2500
		case TN_DM2500:
			tput(020);		/* Enter I/D mode */
			do {	tput(012);		/* Insert line */
			  	switch(trm_ospeed)
				  {	case 13: j = 17; break;	/* 9600 */
					case 12: j = 8; break;	/* 4800 */
					case 11: j = 4; break;	/* 2400 */
					case 9:  j = 2; break;	/* 1200 */
					default: j = 0; break;
				  }
				tpadn(j);
			  } while(--i);
			tput(030);			/* Exit I/D mode */
			break;
#endif /*TX_DM2500*/
#if TX_H19
		case TN_H19:
			do {	tputz("\033L");
				switch(trm_ospeed)
				  {	case 13: j = 25; break;
					case 9:	j = 4; break;
					case 7: j = 1; break;
					default: j = 0; break;
				  }
				tpadn(j);
			  } while(--i);
			break;
#endif /*TX_H19*/
#if TX_COHIBM
		case TN_COHIBM:
			do {	tputz("\033L");  /* no padding required */
		  	  } while(--i);
			break;
#endif /*TX_COHIBM*/
#if TX_OM8025
		case TN_OM8025:
			do {	tputz("\033L");
				tpad(100*(scr_ht - curs_lin));	/* .1 per moved line*/
			  } while(--i);
			break;
#endif /*TX_OM8025*/
#if TX_TVI925
		case TN_TVI925:
			do tputz("\033E");
			while(--i);
			break;
#endif /*TX_TVI925*/
	  }
}

/* T_DELLIN(n, bot) - Delete lines from window.
 *	n   - # lines to delete.
 *	bot - # of last line of current window.
 *		The current line, and N-1 following lines, are deleted.
 *	Blank lines are inserted past bot.
 *	Cursor should be left at original position.
 *	Only valid if trm_flags has IDLIN set.
 */
t_dellin (n, bot)
int   n;			/* number of lines */
int   bot;			/* line number of last line in window */
{	register  i, j;
	int savl, savc;

	if((i = n) <= 0) return;
	switch(tv_type)
	  {
#if TX_TERMCAP
		case TN_TERMCAP:
			if(ID)
			  {	putpad(DM);
				do putpad(ID);
				while(--i);
				putpad(ED);
			  }
			else
				do { putnpad(DL,scr_ht - curs_lin);
				  } while(--i);

			curs_lin = -1;	/* Don't know location now */
			break;
#endif /*TX_TERMCAP*/
#if TX_SUNCON
		case TN_SUNCON:
			 { 
			char sunstr[ 10];
			sprintf( sunstr, "[%dM", i);
			putnpad( sunstr, scr_ht - curs_lin); 
			curs_lin = -1;		/* Don't know where we are */
			break;
			}
#endif /*TX_SUNCON*/
#if TX_DM2500
		case TN_DM2500:
			tput(020);
			do {	tput(032);
			  	if(trm_ospeed >= 13)	/* 9600 */
					tput(0177);
			  } while(--i);
			tput(030);
			break;
#endif /*TX_DM2500*/
#if TX_H19
		case TN_H19:
			do {	tputz("\033M");
				switch(trm_ospeed){
					case 13: j = 25; break;
					case 9:	j = 4; break;
					case 7: j = 1; break;
					default: j = 0; break;
					}
				tpadn(j);
			  } while(--i);
			break;
#endif /*TX_H19*/
#if TX_COHIBM
		case TN_COHIBM:
			do {	tputz("\033M");	  /* no padding required */
			  } while(--i);
			break;
#endif /*TX_COHIBM*/
#if TX_OM8025
		case TN_OM8025:
			do {	tputz("\033M");
				tpad(100*(scr_ht - curs_lin));
			  } while(--i);
			break;
#endif /*TX_OM8025*/
#if TX_TVI925
		case TN_TVI925:
			do {	tputz("\033R");
			  } while(--i);
			break;
#endif /*TX_TVI925*/
	  }
	if(bot < (scr_ht-1))
	  {	savl = curs_lin;
		savc = curs_col;
		t_curpos(bot-n,0);
		t_inslin(n,scr_ht);
		t_curpos(savl,savc);
	  }
}

/* T_INSCHR(n, str) - Insert n chars in current line
 *	n   - # characters to insert
 *	str - Pointer to char string.  If 0, insert spaces.
 *
 *	Insert N characters from string str at current position.
 *	The cursor may move but curs_col must be updated.
 *	Only valid if trm_flags has IDCHR set.
 */
t_inschr(n, str)
char *str;
{	register int i;
	register char *cp;

	if((i = n) <= 0) return;
	cp = str;
	switch(tv_type)
	  {
#if TX_SUNCON
		case TN_SUNCON:
#endif
#if TX_TERMCAP
		case TN_TERMCAP:
			putpad(IM);		/* Go into insert mode */
			do {
				if(IC) putpad(IC);
				if(cp) tput(*cp++);
				else tput(SP);
				if(IP) putpad(IP);
			  } while(--i);
			putpad(EI);		/* Exit insert mode */
			curs_col += n;
			break;
#endif /*TX_TERMCAP*/
#if TX_COHIBM
		case TN_COHIBM:		/* If this exists, TN_H19 will too */
#endif /*TX_COHIBM*/
#if TX_H19
		case TN_H19:
			tputz("\033@");		/* Enter ins char mode */
			do {	if(cp) tput(*cp++);
				else tput(SP);
			  } while(--i);
			tputz("\033O");		/* Exit ins char mode */
			curs_col += n;
			break;
#endif /*TX_H19*/
#if TX_DM2500
		case TN_DM2500:
			tput(020);		/* Enter I/D mode */
			if(trm_ospeed == 13)	/* 9600 baud lossage */
			  {	do {
					tputz(" \177");	/* SP and DEL */
				  } while(--i);
				tput(030);
				i = n;
				if(i < 3)	/* If close enough, */
					tputn("\010\010", i);	/* use BSes */
				else t_curpos(curs_lin, curs_col);
			  }
			else			/* Not 9600, can win */
			  {	do { tput(034);
				  } while(--i);
				tput(030);
				if(cp == 0) return;
				i = n;
			  }

			do {	if(cp) tput(*cp++);
				else tput(SP);
			  } while(--i);
			curs_col += n;
			break;
#endif /*TX_DM2500*/
#if TX_OM8025
		case TN_OM8025:
			do {
				tputz("\033@");
				if(cp) tput(*cp++);
				else tput(SP);
			  } while(--i);
			curs_col += n;
			break;
#endif /*TX_OM8025*/
#if TX_TVI925
		case TN_TVI925:
			do {	tputz("\033Q");
			  } while(--i);
			if(cp)
			  {	tputn(cp, n);
				curs_col += n;
			  }
			break;
#endif /*TX_TVI925*/
	  }
}

/* T_DELCHR(n) - Delete N chars in current line.
 *	Deletes the N characters to the right of the cursor.  Remaining
 *	chars are shifted left.  The cursor should not move.
 *	Only valid if trm_flags has IDCHR set.
 */
t_delchr(n)		/* Delete N chars at current loc */
{	register int i;

	if((i = n) <= 0) return;
	switch(tv_type)
	  {
#if TX_SUNCON
		case TN_SUNCON:
#endif
#if TX_TERMCAP
		case TN_TERMCAP:
			putpad(DM);	/* Enter delete mode */
			do putpad(DC);	/* Delete char while in del mode */
			while(--i);
			putpad(ED);	/* Exit delete mode */
			break;
#endif /*TX_TERMCAP*/
#if TX_COHIBM
		case TN_COHIBM:		/* If this exists, TN_H19 will too */
#endif /*TX_COHIBM*/
#if TX_H19
		case TN_H19:
			do tputz("\033N");
			while(--i);
			break;
#endif /*TX_H19*/
#if TX_DM2500
		case TN_DM2500:
			tput(020);		/* Enter I/D mode */
			do if(trm_ospeed == 13)	/* 9600? */
				tputz("\010\177");	/* BS and DEL */
			  else tput(010);
			while(--i);
			tput(030);		/* Exit I/D mode */
			break;
#endif /*TX_DM2500*/
#if TX_OM8025
		case TN_OM8025:
			do tputz("\033P");
			while (--i);
			break;
#endif /*TX_OM8025*/
#if TX_TVI925
		case TN_TVI925:
			do {	tputz("\033W");
			  } while(--i);
#endif /*TX_TVI925*/
	  }
}

#if FX_SOWIND

/* T_STANDOUT(n) - Enter or leave standout mode.
 *	n   - 0 to return to normal display mode,
 *	      1 to enter standout display mode.
 *		This is usually reverse video but may be something else.
 *
 *	Only valid if trm_flags has TC_SO set.
 */

t_standout(on)
{
	switch(tv_type)
	  {
#if TX_SUNCON
		case TN_SUNCON:
#endif
#if TX_TERMCAP
		case TN_TERMCAP:
			putpad(on ? SO : SE);
			break;
#endif /*TX_TERMCAP*/

#if 0
#if TX_DM2500
		case TN_DM2500:
			break;
#endif /*TX_DM2500*/
#if TX_COHIBM
		case TN_COHIBM:		/* Note TN_H19 will exist too */
#endif /*TX_COHIBM*/
#if TX_H19
		case TN_H19:
			break;
#endif /*TX_H19*/
#if TX_OM8025
		case TN_OM8025:
			break;
#endif /*TX_OM8025*/
#if TX_TVI925
		case TN_TVI925:
			break;
#endif /*TX_TVI925*/
#endif /* COMMENT */
	  }
}
#endif /*FX_SOWIND*/


/* TPADN(n) - Output N pad chars.
 */
tpadn(n)
{	register int i, pad;
	if((i = n) > 0)
	  {	pad = tv_padc;
		do { tput(pad);
		  } while(--i);
	  }
}

/* TPAD(msec) - Output padding for given # of milliseconds.
 */
tpad(n)
{	register int i, i2;

	i = n;
	while(i > 0)
	  {	if((i2 = 320) < i)	/* So can use integers */
			i2 = i;
		i -= i2;
		i2 *= 100;
		while((i2 -= tv_cspeed) > 0)
			tput(tv_padc);
	  }
}
#if TX_TERMCAP
/*
 * Print the string str, interpreting padding.
 */
int tput();	/* Our output function */
putpad(str)	{ tputs(str, 1, tput); }	/* Invoke TERMCAP function */
putnpad(str,n)	{ tputs(str, n, tput); }

/*
 * Read in the stuff from termcap upon startup.
 */

char *_tcbptr;	/* Current deposit pointer into termcap buffer */
char *
tgetx(str)	/* TGETX - trade off speed for instruction space */
char *str;
{	return(tgetstr(str, &_tcbptr));
}

getcap(stype)
char *stype;
{	register char *tcbuf, *t;
	register int buflen;
	char tmpbuf[TCAPSLEN];		/* Allocate from stack */
	char *malloc();

	if((tgetent(tmpbuf, stype)) != 1)
		return(0);
	ospeed = trm_ospeed;		/* Sigh, set external for termcap */

	if(!(tcbuf = malloc(TCAPSLEN)))	/* Get permanent buffer */
	  {	writez(1,"Cannot allocate termcap buffer\n");
		exit(1);
	  }
	_tcbptr = tcbuf;		/* Set global ptr for tgetx hack */

	if(t = tgetx("pc"))		/* pad char */
		PC = *t;
	IM = tgetx("im");		/* insert mode on */
	DM = tgetx("dm");		/* delete mode on */
	IA = tgetx("ia");		/* char to add line in insert mode */
	ID = tgetx("id");		/* char to del line in delete mode */
	EI = tgetx("ei");		/* insert mode off */
	ED = tgetx("ed");		/* delete mode off */
	IS = tgetx("is");		/* tty initialization string */
	TE = tgetx("te");		/* str to end programs that use cm */
	TI = tgetx("ti");		/* str to begin progs that use cm */
	AL = tgetx("al");		/* add line */
	DL = tgetx("dl");		/* delete line */
	CM = tgetx("cm");		/* cursor motion */
	CL = tgetx("cl");		/* clear */
	CE = tgetx("ce");		/* erase to end of line */
	UP = tgetx("up");		/* cursor up */
	IC = tgetx("ic");		/* insert char */
	IP = tgetx("ip");		/* send after inserting char */
	DC = tgetx("dc");		/* delete char */
	AM = tgetflag("am");		/* auto wrap */
	tc_bs = tgetflag("bs");		/* tty can backspace */
	if(!tc_bs) tvs_bs = tgetx("bc");	/* Backspace str (if no BS) */

#if FX_SOWIND
	if(tgetnum("sg") <= 0)		/* If no magic cookie problems */
	  {	if ((SO = tgetx("so"))	/*  get standout fellows */
		  && (SE = tgetx("se")))
			trm_flags |= TC_SO;	/* Say has standout cap */
	  }
#endif
	tc_km = (tgetflag("km")		/* TTY has meta key */
		|| tgetflag("MT"));	/* More modern version of "km" */
	/* Any terminal with prefix aaa- or name ambassador has a meta-key */
	if(strncmp(stype, "aaa-", 4) == 0 || strcmp(stype, "ambassador") == 0)
		tc_km = 1;
	tc_mm = tgetx("mm");		/* Turn on meta mode */
	tc_mo = tgetx("mo");		/* Turn off meta mode */

	scr_ht = tgetnum("li");
	scr_wid = tgetnum("co");

#if TXC_VISBEL
        VB = tgetx("vb");
#endif

	buflen = _tcbptr - tcbuf;
	if(buflen >= TCAPSLEN)
	  {	writez(1,"Terminal description too big!\n");
		exit(1);
	  }
	realloc(tcbuf, buflen);		/* Free up unused part of buffer */

	/* Basic data extracted, now mull over it and set the remaining
	 * ELLE variables
	 */
	if (!(CM && CL))
	  {	writez(1,stype);
		writez(1,": terminal lacks one of: cursor addressing, clear screen.\n");
		exit(1);
	  }
	tvc_pos = tstrlen(CM);			/* Find cost of abs move */
	if(!tc_bs)				/* Find cost of backspace */
		tvc_bs = tvs_bs ? tstrlen(tvs_bs) : 1000;
	if ((IM||IC) && (DM||DC))
	  {	trm_flags |= IDCHR;
		tvc_ci  = tstrlen(IM)+tstrlen(EI);
		tvc_cin = tstrlen(IC)+1+tstrlen(IP);
		tvc_cd  = tstrlen(DM)+tstrlen(ED);
		tvc_cdn = tstrlen(DC);
	  }
	if ((IA || AL) && (ID || DL))
	  {	trm_flags |= IDLIN;
		if(IA)
		  {	tvc_li = tstrlen(IM)+tstrlen(EI);
			tvc_lin = tstrlen(IA);
		  }
		else
		  {	tvc_li = 0;
			tvc_lin = tstrlen(AL);
		  }
		if(ID)
		  {	tvc_ld = tstrlen(DM)+tstrlen(ED);
			tvc_ldn = tstrlen(ID);
		  }
		else
		  {	tvc_ld = 0;
			tvc_ldn = tstrlen(DL);
		  }
	  }
	if (AM)
	  {	scr_wid--;		/* For now, avoid invoking wrap. */
	/*	trm_flags |= AUTOWRAP;	/* */
	  }
	if (CE) trm_flags |= CLEOL;
	if(tc_km)			/* If term has meta key */
		trm_flags |= METAKEY;		/* Look for 8th bit */
	return(1);
}

/* Pair of routines which conspire in order to find # chars actually output
 * by a particular termcap string.
 */
int _tslen;		/* Stored count */
_tslinc(ch) { _tslen++; }
tstrlen(str)
char *str;
{	_tslen = 0;
	if(str && str[0])
		tputs(str, 1, _tslinc);	/* Mult padding by just 1 */
	return(_tslen);
}

#endif /*TX_TERMCAP*/

/* Direct-Video terminal output routine
 *	Currently only COHERENT has this capability.
 */

#if COHERENT
#include <sgtty.h>

struct vidctl {
	int	v_position;		/* Position in video memory */
	int	v_count;		/* Number of characters to transfer */
	char	*v_buffer;		/* Character buffer to read/write */
};
/*
 * Attribute masks for TIOVPUTB - attributes occupy odd addresses
 * in video memory.
 */
#define	VNORM	0x07			/* Ordinary Video */
#define	VINTE	0x08			/* Intense video */
#define	VBLIN	0x80			/* Blinking video */
#define	VREVE	0x70			/* Reverse video */
#define	VUNDE	0x01			/* Underline video (mono board) */

/* T_DIRECT(line, col, string, len) - Do direct-video output of string.
 *	Puts the string ("len" chars in length) on the screen starting at
 *	the X,Y character position given by col, line.
 *	This routine is only called if terminal has the "DIRVID" flag set.
 */
t_direct(lin, col, str, len)
register char *str;
register int len;
{	register char *cp;
	char vbuf[MAXLINE*2];
	struct vidctl v;

	if(len <= 0) return;
	tbufls();		/* Ensure normal output is forced out */
	v.v_position = (lin*80 + col)*2;
	v.v_count = len*2;
	v.v_buffer = cp = vbuf;
	do {
		*cp++ = *str++;
		*cp++ = VNORM;
	  } while(--len);
	ioctl(1, TIOVPUTB, &v);
}
#endif /*COHERENT*/

/*
 * Terminal Output buffering routines
 */

static char tbuf[TOBFSIZ];	/* Output buffer */
static int tbufcnt = 0;		/* # chars of room left in buffer */
static char *tbufp = 0;		/* Pointer to deposit in buffer */

tput(ch)
{	if(--tbufcnt < 0)
		tbufls();
	*tbufp++ = ch;
}

tputz(str)
char *str;
{	register int c;
	register char *cp, *tp;
	cp = str;
	tp = tbufp;
	while(c = *cp++)
	  {	if(--tbufcnt < 0)
		  {	tbufp = tp;
			tbufls();
			tp = tbufp;
		  }
		*tp++ = c;
	  }
	tbufp = tp;
}
tputn(str,cnt)
char *str;
{	register int c;
	register char *cp, *tp;
	cp = str;
	tp = tbufp;
	if((c = cnt) > 0)
	do {
		if(--tbufcnt < 0)
		  {
			tbufp = tp;
			tbufls();
			tp = tbufp;
		  }
		*tp++ = *cp++;
	  } while(--c);
	tbufp = tp;
}

tbufls()
{	register int cnt;

	if(tbufp
	  && (cnt = tbufp - tbuf) > 0)		/* # chars written */
		write(1, tbuf, cnt);		/* Out they go */
	tbufp = tbuf;
	tbufcnt = TOBFSIZ-1;	/* Allow for usual expected decrement */
}

/*
 * Terminal Input buffering routines
 */

int tibfmsk = -1;		/* Mask AND'ed with input chars (external) */
static char tibuf[TIBFSIZ];	/* TTY input buffer */
static char *tibfp;		/* Pointer to read from buffer */
static int tibfcnt = 0;		/* # chars left to be read from buffer */

#if SUN
tgete()		/* Probably not used any more, but just in case */
{	sun_rdevf = 1;
	return(tgetc());
}
#endif /*SUN*/

tgetc()
{
#if SUN
	register int c;
	if(sun_winfd)
	  {	if(!sun_rdevf)
			return(sun_input(1)&tibfmsk);
		sun_rdevf = 0;		/* Check mouse too, but only once! */
		c = sun_input(0);
		if(c != -1) c &= tibfmsk;
		return(c);
	  }
#endif /*SUN*/
	while(--tibfcnt < 0)
		tibfcnt = read(0,(tibfp = tibuf),TIBFSIZ);
	return((*tibfp++)&tibfmsk);
}

tinwait()
{	return(tibfcnt > 0 || ts_inp());
}

