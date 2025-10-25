/* ELLE - Copyright 1987 by Ken Harrenstien, SRI International
 *	This software is quasi-public; it may be used freely with
 *	like software, but may NOT be sold or made part of licensed
 *	products without permission of the author.
 */
/* EEFICO - Functions for ICONOGRAPHICS configuration.
 *	These were written by Chris D. Tavares, 1982, who also wrote
 * a few other functions scattered throughout ELLE; these are either
 * conditionalized or marked as coming from the ICONOGRAPHICS configuration.
 *
 * KLH: These have been updated, in some cases extensively, as ELLE
 * changed, but are not guaranteed to still work!
 */

#include "elle.h"

#if !(ICONOGRAPHICS)	/* Entire file depends on this conditional! */

#if FX_ICOXCMD
f_icoxcmd() {}
#endif
#if FX_ICOTYPFNS
f_icotypfns () {}
#endif
#if FX_ICOSPIFNSS
f_icospifns () {}
#endif

#else			/* Everything else is ICONOGRAPHICS-only */


/* e_xcmds; extended commands for ELLE; C. D. Tavares, 8/18/82 */

extern int para_mode;	/* Defined in e_just.c */
extern int fill_mode;	/* Defined in e_fill.c */

static help_xcmds();
static char *cmd_list [] =
       {"?",
	"fillon",
	"filloff",
	"type-mode",
	"fundamental-mode",
	"blockpara",
	"linepara",
	0};

#define HELP		0
#define FILLON		1
#define FILLOFF		2
#define TYPEMODE	3
#define FUNMODE		4
#define BPARA		5
#define LPARA		6
#define N_COMMANDS	6

/* EFUN: "ICO Extend Command" (not EMACS) */
f_icoxcmd()
  {
	char *cmd;
	register int i;

	if ((cmd = ask ("Command: ")) == 0)
		return;				/* aborted */

	for (i = 0 ; cmd_list [i] ; i++)
	    if (strcmp (cmd, cmd_list [i]) == 0) break;

	switch (i)
	   {
	    case HELP:
		help_xcmds ();
		break;

	    case FILLON:
		if(!fill_mode) f_fillmode();
		break;

	    case FILLOFF:
		if(fill_mode) f_fillmode();
		break;

	    case TYPEMODE:
		Typemode ();
		break;

	    case FUNMODE:
		Typeoff ();
		break;

	    case BPARA:
		para_mode = PARABLOCK;
		break;

	    case LPARA:
		para_mode = PARALINE;
		break;

	    default:
		ring_bell ();
		sayntoo ("???");
		break;
	   }

	chkfree (cmd);
  }

static
help_xcmds ()
{
	register int i;
	register struct buffer *b;
	struct buffer *savbuf, *make_buf();

	savbuf = cur_buf;
	chg_buf(b = make_buf(" **HELP**"));

	e_sputz("Valid extended commands:\n\n");

	for (i = 1 ; cmd_list [i] ; i++)
	   {
	    e_sputz (cmd_list [i]);
	    e_putc (LF);
	   }

	e_sputz ("\n-- * * * * * * * * --\n");

	mk_showin(b);	/* Show this buffer in a temp window */
	chg_buf(savbuf);
	kill_buf(b);
}

/***********************************************************************/
/*                                                                     */
/*      Typesetting mode functions; 05/25/81, C. D. Tavares.           */
/*                                                                     */
/*      These functions are only useful on an Ann Arbor Ambassador.    */
/*      Among other things, they load the programmable keys with       */
/*      special typesetting command "macros" so that things like       */
/*      font changes, modes, and special characters are single         */
/*      keystrokes.  On other terminals, they're pretty moot.          */
/*                                                                     */
/***********************************************************************/


extern int fill_mode;
extern char fill_prefix[];

#define TCTLZ 90
#define TSPECIALCHARS 91
#define STDOUT 1
#define NULL 0
#define FIND_OFFS (long)-1

#define Tattn_msg ding
#define numarg exp                      /* I like "numarg" better */
#define is_numarg exp_p                 /* ditto */
#define BACKWARDS 1
#define FORWARDS 0

char *itoa ();

/* Constants for special keys.  They happen to be the key address as well */


#define KReset          '&'
#define KSetup          '\''
#define KBreak          '('
#define KSBreak         ')'
#define KPause          '*'
#define KReturn         '+'
#define KMoveUp         ','
#define KSMoveUp        '-'
#define KMoveDown       '.'
#define KSMoveDown      '/'
#define KS0             '0'
#define KS1             '1'
#define KS2             '2'
#define KS3             '3'
#define KS4             '4'
#define KS5             '5'
#define KS6             '6'
#define KS7             '7'
#define KS8             '8'
#define KS9             '9'
#define KPeriod         ':'
#define KTab            ';'
#define KEnter          '<'
#define KSTab           '='
#define KErase          '>'
#define KSErase         '?'
#define KEdit           '@'
#define KDelete         'A'
#define KSDelete        'B'
#define KInsert         'C'
#define KSInsert        'D'
#define KPrint          'E'
#define KSPrint         'F'
#define KCTLS7          'G'
#define KPF1            'H'
#define KPF2            'I'
#define KPF3            'J'
#define KPF4            'K'
#define KPF5            'L'
#define KPF6            'M'
#define KPF7            'N'
#define KPF8            'O'
#define KPF9            'P'
#define KPF10           'Q'
#define KPF11           'R'
#define KPF12           'S'
#define KSPF1           'T'
#define KSPF2           'U'
#define KSPF3           'V'
#define KSPF4           'W'
#define KSPF5           'X'
#define KSPF6           'Y'
#define KSPF7           'Z'
#define KSPF8           '['
#define KSPF9           '\\'
#define KSPF10          ']'
#define KSPF11          '^'
#define KSPF12          '_'



static int Tkeys_loaded = 0;

static char Tinit_string [] = "{ap}{a50}\n";

static char Keyloads [] =
       {0,              0,              0,              0,          /*  !"# */
        KEnter,         KS8,            KS9,            0,          /* $%&' */
        0,              0,              0,              KS7,        /* ()*+ */
        0,              0,              KSTab,          KS6,        /* ,-./ */
        KSPF5,          KPF1,           KPF2,           KPF3,       /* 0123 */
        KPF4,           KSPF1,          KSPF2,          KSPF3,      /* 4567 */
        KSPF4,          KPF5,           0,              0,          /* 89:; */
        0,              KS0,            0,              0,          /* <=>? */
        0,              KPF6,           KSPF6,          KPF7,       /* @ABC */
        KSPF7,          KPF8,           0,              0,          /* DEFG */
        KSPF8,          KPF9,           KSPF9,          KPF10,      /* HIJK */
        KSPF10,         KPF11,          KSPF11,         KPF12,      /* LMNO */
        KSPF12,         KErase,         KSErase,        KEdit,      /* PQRS */
        KDelete,        KSDelete,       KInsert,        KSInsert,   /* TUVW */
        KPrint,         KSPrint,        KPeriod,        0};         /* XYZ[ */

int Tcents(), Tfeet(), Ttimes(), Tbullet(), Tdegrees(), Tinches(), Tsymbol(),
    Tdhyphen (),
    TF_R(), TF_I(), TF_B(), TF_BI(), TF_EB(), TF_EBI(), TF_Bk(), TF_BkI(),
    TMtext(), TMcaption(), TMparah(), TM18pth(), TM24pth(), TM36pth(),
    Tcolwidth(), Tpicawidth(),
    Tdotleader(), Tbaseleader(),
    Tpointsize(), Tleading(), Texllines(), Texlpoints(), Texlpicas(),
    Tql(), Tql1nl(), Tqr(), Tqc(),
    Treport(), Tfraction();

static int (*CTLZfns []) () =
       {0,              0,              0,              0,          /*  !"# */
        0,              Tcents,         Tfeet,          0,          /* $%&' */
        0,              0,              0,              Ttimes,     /* ()*+ */
        0,              0,              Tbullet,        Tdegrees,   /* ,-./ */
        0,              TF_R,           TF_I,           TF_B,       /* 0123 */
        TF_BI,          TF_EB,          TF_EBI,         TF_Bk,      /* 4567 */
        TF_BkI,         0,              0,              0,          /* 89:; */
        0,              Tinches,        0,              0,          /* <=>? */
        0,              Tsymbol,        Tdhyphen,       TMtext,     /* @ABC */
        TMcaption,      TMparah,        0,              0,          /* DEFG */
        TM18pth,        TM24pth,        TM36pth,        Tcolwidth,  /* HIJK */
        Tpicawidth,     Tdotleader,     Tbaseleader,    Tpointsize, /* LMNO */
        Tleading,       Tql,            Tql1nl,         Tqr,        /* PQRS */
        Tqc,            Texllines,      Texlpoints,     Texlpicas,  /* TUVW */
        Treport,        0,              Tfraction,      0};         /* XYZ[ */

extern char charmap [], metatab [];

static struct
       {char    spf_char;
        int     old_fn_holder;}
    Special_fn_chars [] =
               {'\t',   0,
                '\r',   0,
                '/',    0,
                  0,    0};

static char *Trelfont_names [] =
       {" Roman",       " Italic",              " Bold",     " Bold Italic",
        " Extrabold",   " Extrabold Italic",    " Black",    " Black Italic"};

static struct
       {int     f_nos [8];
        char    *f_name;}
    Tfont_table [] =
       {641, 642, 643, 644,   0,   0,   0,   0, "Times",
         13,   0,  15,   0,   0,   0,   0,   0, "American Typewriter",
        211,   0, 213,   0,   0,   0,   0,   0, "Fritz Quadrata",
        383, 384, 385, 386, 387, 388, 389, 390, "Helvetica",
        393, 394, 395, 396,   0,   0, 397, 398, "Helvetica Condensed",
        399,   0,   0,   0,   0,   0,   0,   0, "Helvetica Outline Bold",
        191,   0, 193,   0,   0,   0,   0,   0, "Eurostile",
        195,   0, 197,   0,   0,   0,   0,   0, "Eurostile Extended",
        421, 422, 423, 424, 425, 426, 427, 428, "Korinna",
        223,   0, 225,   0, 227,   0, 229,   0, "Futura",
        175,   0, 177,   0, 179,   0, 181,   0, "Eras",
        413,   0, 415,   0, 417,   0, 419,   0, "Kabel (OFF!)",
        471,   0, 473,   0, 475,   0, 477,   0, "Memphis (OFF!)",
          1,   2,   3,   0,   5,   0,   0,   0, "Americana",
         21,   0,  23,   0,  25,   0,  27,   0, "Antique Olive (OFF!)",
         43,   0,  45,   0,  47,   0,  49,   0, "Avant Garde Gothic (OFF!)",
        553,   0, 555,   0, 557,   0, 561,   0, "Serif Gothic",
        583, 584, 585, 586, 587, 588,   0,   0, "Souvenir",
        623,   0, 625,   0, 627,   0,   0,   0, "Tiffany",
         71,  72,  73,  74,  75,  76,   0,   0, "Benguiat (OFF!)",
        251, 252, 253,   0, 255,   0,   0,   0, "Goudy",
  /* Artsy fonts */
        257,   0,   0,   0,   0,   0,   0,   0, "Goudy Handtooled",
        923,   0,   0,   0,   0,   0,   0,   0, "Printout Bold",
        101,   0,   0,   0,   0,   0,   0,   0, "Broadway",
         51,   0,   0,   0,   0,   0,   0,   0, "Bauhaus Heavy",
        111,   0,   0,   0,   0,   0,   0,   0, "Busorama Light",
        131,   0,   0,   0,   0,   0,   0,   0, "Cascade Script",
        571,   0,   0,   0,   0,   0,   0,   0, "Snell Roundhand Script",
        521,   0,   0,   0,   0,   0,   0,   0, "Park Avenue Script",
         85,   0,   0,   0,   0,   0,   0,   0, "Poster Bodoni",
        441,   0,   0,   0,   0,   0,   0,   0, "Linotext Gothic",
          0,   0,   0,   0,   0,   0,   0,   0,  0};

static struct
       {char    *d_key;
        char    *d_fullstr;}
    Tmode_defaults [] =
       {"he1",  "{dhe1/f015/p24/l26/m45/xh/rr/xk/xt/xi}{note NR 24P H}",
        "he2",  "{dhe2/f015/p36/l38/m45/xh/rr/ak3/xt/xi}{note LG 36P H}",
        "su1",  "{dsu1/f0643/p12/l14/m14/xh/rr/xk/xt/xi}{note PARA H}",
        "su2",  "{dsu2/f015/p18/l19/m45/xh/rr/xk/xt/xi}{note SM 18P H}",
        "te1",  "{dte1/f0641/p9/l10/m14/ah/xr/xk/xt/xi}{note TEXT}",
        "te2",  "{dte2/f0642/p8/l9/m14/ah/rr/xk/xt/xi}{note CAPTION}",
        "te3",  "{dte3/f0642/p9/l10/m14/xh/rr/xk/xt/xi}{note BYLINE}",
           0 ,   0};

static char *Tknownsubmodes [] =
       {"he1", "he2",
        "su1", "su2",
        "te1", "te2", "te3", "te4", "te5", "te6", "te7", "te8",
        0};

typedef struct
       {char    *nt_short;
        char    *nt_long;} Nametable;

static Nametable Tkern_names [] =
       {"xk",  "",
        "xe",  "",
          0 ,  0};

static Nametable Ttilt_names [] =
       {"xt",  "",
        "at",  ", tilted",
          0 ,  0};

static Nametable Trag_names [] =
       {"rr", ", rag right",
        "rl", ", rag left",
        "rc", ", rag center",
        "xr", "",
          0 ,  0};

static Nametable Thyph_names [] =
       {"ah", " on",
        "xh", " off",
          0 ,  0};

static char *Tcolumn_widths [] = {"14", "29.6", "45"};

char *TFfontname(), *TFpointsize(), *TFkerning(), *TFleading(),
     *TFpagewidth(), *TFrag(), *TFhyph(), *TFfont(), *TFtilt()
;
char *Textract_field();

static struct
       {char    *im_name;
        char *  (*im_function) ();
        Nametable *im_table;
        char    im_prefix [8];}
    Tall_reports [] =
               {"font",         TFfontname,     0,              "",
                "point",        TFpointsize,    0,              ", ",
                "kern",         TFkerning,      Tkern_names,    "(",
                "leading",      TFleading,      0,              ")/",
                "measure",      TFpagewidth,    0,              ", col ",
                "rag",          TFrag,          Trag_names,     "",
                "hyph",         TFhyph,         Thyph_names,    ", hyph",
                "tilt",         TFtilt,         Ttilt_names,    "",
                    0 ,         0,              0,              0};

/* Code section.  Major mode and key-loading functions. */

static int ofillon, old_ctlz_fn, ofillcolumn;

static int TypeModeOn = 0;

struct majmode itypmode = { "Typesetting" };
Typemode ()
  {
        register int i;

        cur_mode = cur_buf->b_mode = &itypmode;
	redp(RD_MODE);

        ofillcolumn = ev_fcolumn;
        fill_prefix [0] = ' ';
        fill_prefix [1] = '\0';
        ev_fcolumn = 78;
        ofillon = fill_mode;
	if(!fill_mode) f_fillmode();

        Tloadbindkeys ();

        for (i = 0 ; Special_fn_chars [i].spf_char ; i++)
           {
            Special_fn_chars [i].old_fn_holder =
                                charmap [Special_fn_chars [i].spf_char];
            charmap [Special_fn_chars [i].spf_char] = TSPECIALS;
           }

        old_ctlz_fn = charmap [037 & 'Z'];
        charmap [037 & 'Z'] = TCTLZ;

        e_gobob ();

        if (!looking_at (Tinit_string))
           {
            if (e_blen () == 0)         /* buffer empty */
               {
                ed_sins (Tinit_string);
                f_bufnotmod();
               }
            else
               Tattn_msg
                 ("File does not start with standard initial string!");
           }

        TypeModeOn = 1;
  }


Typeoff ()
  {
        register int i;

        TypeModeOn = 0;

        cur_mode = cur_buf->b_mode = fun_mode;
	redp(RD_MODE);

        if (!ofillon) f_fillmode();
        fill_prefix [0] = '\0';
        ev_fcolumn = ofillcolumn;

        for (i = 0 ; Special_fn_chars [i].spf_char ; i++)
            charmap [Special_fn_chars [i].spf_char]
                                = Special_fn_chars [i].old_fn_holder;

        charmap [037 & 'Z'] = old_ctlz_fn;
  }

looking_at (str)
  char *str;
  {
        chroff origpos;
        int match;

        origpos = e_dot ();
        match = 1;

        while (*str)
           if (*(str++) != e_getc ())
               {
                match = 0;
                break;
               }

        e_go (origpos);
        return match;
  }


Tloadbindkeys ()
  {
        register int i;

        if (Tkeys_loaded) return;

        saynow ("Loading keys...");

        for (i = 0 ; i < sizeof (Keyloads) ; ++i)
            if (Keyloads [i])
                Tload_key (Keyloads [i], (char) (i+' '));

        Tkeys_loaded = 1;
        sayntoo (" Keys loaded.");
  }


Tload_key (key, ch)
  int key, ch;
  {
        static struct
               {char    xx_prefix [3];
                char    xx_key;
                char    xx_ctlz [2];
                char    xx_conts;
                char    xx_suffix [3];}
            outstring =
                {"\033P`", '?', "~Z", '?', "\033\\"};

        if (!key) return;

        outstring.xx_key = key;
        outstring.xx_conts = ch;

        writez (STDOUT, &outstring);
  }


Tundefkey ()
  {
        Tattn_msg ("Undefined key.");
  }


/* EFUN: "ICO Typeset Funs" (not EMACS) */
f_icotypfns ()
  {
        int (*fn) ();
        static char foochs [2] = {" "};

        if ((fn = CTLZfns [(foochs [0] = cmd_read()) - ' ']) == 0)
           {
            Tattn_msg ("UNKNOWN FUNCTION ^Z");
            sayntoo (foochs);
           }

        else fn ();
  }


/* EFUN: "ICO Spec Input Funs" (not EMACS) */
f_icospifns(ch)
  int ch;
  {
        char *e, errbuf [3];

        switch (ch)
           {
            case ' ':                   /* ESC-SPACE */
                Tenspace ();
                break;

            case '\t':                  /* TAB -> em space */
                Temspace ();
                break;

            case '/':                   /* warn if possible fraction */
                Tslashwarning ();
                break;

            case '\r':                  /* warn on EXPLICIT newline */
                Tnlwarning ();
                break;

            default:
                e = errbuf;
                if (ch < ' ')
                   {
                    *(e++) = '^';
                    ch |= 0100;
                   }
                *(e++) = ch;
                *(e++) = '\0';
                Tattn_msg (errbuf);
                sayntoo ("???");
                break;
           }
  }

/* Text-hacking functions */


int
Tlefthandchar ()
  {
        if (e_rgetc () == EOF) return NULL;
        return (e_getc ());
  }


Tinswnl (str)
  char  *str;
  {
        delwhitespace ();       /* delete white sides */
        e_sputz (str);
        Tnlnofill ();
  }


Tinsw2nl (str)
  char *str;
  {
        int c;

        delwhitespace ();
        e_sputz (str);
        /* if? */ fill_current_line ();
        Tnlnofill ();
        if ((c = e_peekc ()) == EOF)
            Tnlnofill ();               /* if at end of buffer */
        else if (c == '\n') e_getc ();
        e_setcur ();
  }

/* Font-, Mode-, Column-, and Rag-hacking functions */


TF_R ()         {Tset_font (0);}
TF_I ()         {Tset_font (1);}
TF_B ()         {Tset_font (2);}
TF_BI ()        {Tset_font (3);}
TF_EB ()        {Tset_font (4);}
TF_EBI ()       {Tset_font (5);}
TF_Bk ()        {Tset_font (6);}
TF_BkI ()       {Tset_font (7);}


Tset_font (relfont)
  int   relfont;
  {
        char *oldfontstring;
        int oldfontnum, newfontnum, fontinfamily, fontbasis;
        register int i, j, gotidx;

        oldfontstring = TFfont (FIND_OFFS, FIND_OFFS);
        oldfontnum = oldfontstring ? atoi (oldfontstring) : 0;
        fontinfamily = is_numarg ? numarg : oldfontnum;

        if (fontinfamily == 0)
           {
            Tattn_msg ("No current font!");
            return NULL;
           }

        for (i = gotidx = 0 ;
                 (!gotidx) && (fontbasis = Tfont_table [i].f_nos [0]) ; i++)
           {
            if (fontbasis == fontinfamily)
                gotidx = ++i;
            else
                for (j = 1 ; j < 8 ; j++)
                    if (Tfont_table [i].f_nos [j] == fontinfamily)
                       {
                        gotidx = ++i;
                        break;
                       }
           }

        if (!gotidx)
           {
            Tattn_msg ("No basic font for ");
            sayntoo (itoa (fontinfamily));
            return NULL;
           }

        newfontnum = Tfont_table [--gotidx].f_nos [relfont];
        if (is_numarg)
            if ((numarg != fontbasis) && (numarg != newfontnum))
                Tattn_msg ("Used correct font for given family");

        if (!newfontnum)
           {
            Tattn_msg ("No such font: ");
            sayntoo (Tfont_table [gotidx].f_name);
            sayntoo (Trelfont_names [relfont]);
            return NULL;
           }


        if (newfontnum == oldfontnum)
           {
            Tattn_msg ("Already in font ");
            sayntoo (Tfont_table [gotidx].f_name);
            sayntoo (Trelfont_names [relfont]);
            return NULL;
           }

        ed_sins ("{f0");
        ed_sins (itoa (newfontnum));
        ed_sins ("}");
        return 1;
  }


TMtext ()       {Tset_mode ("te1");}
TMcaption ()    {Tset_mode ("te2");}
TMparah ()      {Tset_mode ("su1");}
TM18pth ()      {Tset_mode ("su2");}
TM24pth ()      {Tset_mode ("he1");}
TM36pth ()      {Tset_mode ("he2");}


Tset_mode (typech)
  char *typech;
  {
        char newstring [8], defstring [8];
        chroff oldpos;
        int fullmode;
        register int i;
        register char *testkey;

        strcat (strcat (strcpy (newstring, "{"), typech), "}");
        strcat (strcpy (defstring, "{d"), typech);

        oldpos = e_dot ();
        if (e_search (defstring, BACKWARDS) == 0) fullmode = 1;
        else fullmode = 0;
        e_go (oldpos);

        if (oldpos != e_boldot ()) Tnlnofill ();
        if (fullmode)
            for (i = 0 ; testkey = Tmode_defaults [i].d_key ; i++)
                if (strcmp (testkey, typech) == 0)
                   {
                    ed_sins (Tmode_defaults [i].d_fullstr);
                    ed_crins ();
                    break;
                   }

        Tinsw2nl (newstring);
        e_setcur ();
  }

#if 0	/* Commented out */

char *
Tgetmodename (string)
  char *string;
  {
        register int i;
        register char *testfrag;

        for (i = 0 ; testfrag = Tmodenames [i].d_modefrag ; i++)
            if (strcmp (testfrag, string) == 0)
                return Tmodenames [i].d_explan;

        Tattn_msg ("No such mode: ");
        sayntoo (string);
        return NULL;
  }

#endif /*COMMENT*/


Tcolwidth ()
  {
        register int n_cols;
        char newstring [12];

        n_cols = numarg - 1;            /* will be 0 if no numarg */
        if (n_cols < 0) n_cols = 0;
        if (n_cols > 2) n_cols = 2;

        strcat (strcat (strcpy (newstring, "{m"),
                                         Tcolumn_widths [n_cols]), "}");

        Tgeneral_column_width (newstring);
  }


Tgeneral_column_width (newstring)
  char *newstring;
  {
        if (strcmp (newstring, TFpagewidth (FIND_OFFS, FIND_OFFS)) == 0)
           {
            Tattn_msg ("Already in columnwidth ");
            sayntoo (newstring);
            return 0;
           }

        ed_sins (newstring);
        return 1;
  }


Tpicawidth ()   {Tgenask ("Page width (picas): ", "{m");}
Tpointsize ()   {Tgenask ("Point size: ", "{p");}
Tleading ()     {Tgenask ("Leading: ", "{l");}
Tsymbol ()      {Tgenask ("Symbol: ", "{sy");}


Tgenask (question, prefix)
  char *question, *prefix;
  {
        char *item;

        if (is_numarg) item = itoa (numarg);
        else if ((item = ask (question)) == NULL) return;  /* user aborted */

        ed_sins (prefix);
        ed_sins (item);
        ed_sins ("}");

        if (!is_numarg) chkfree (item);
  }


Ttimes ()       {ed_sins ("{mult}");}
Tinches ()      {ed_sins ("{inch}");}
Tdegrees ()     {ed_sins ("{degree}");}
Tfeet ()        {ed_sins ("{foot}");}
Tcents ()       {ed_sins ("{cent}");}


/* Other random key functions */


Tdotleader ()   {ed_sins ("{jd}");}
Tbullet ()      {ed_sins ("{hb}");}
Tdhyphen ()     {ed_sins ("{-}");}

Tql ()  {Tinsw2nl ("{ql}");}
Tqc ()  {Tinswnl ("{qc}");}
Tqr ()  {Tinswnl ("{qr}");}

Tql1nl ()
  {
        if (is_numarg)
            ed_sins ("{ql}");
        else
            Tinswnl ("{ql}");
  }


Texlpoints ()   {Textralead (numarg);}  /* remember, numarg = 1 default */
Texlpicas ()    {Textralead (numarg * 12);}
Texllines ()    {Textralead (numarg * atoi (TFleading (FIND_OFFS, FIND_OFFS)));}

Textralead (n)
  int   n;
  {
        if ((e_dot () != e_boldot ()) && (Tlefthandchar () != '\n'))
            Tattn_msg ("No quad!");

        ed_sins ("{a");
        ed_sins (itoa (n));
        Tinsw2nl ("}");
  }

Temspace ()     {ed_sins ("{em}");}

Tenspace ()
  {
        if (TypeModeOn) ed_sins ("{en}");
        else ring_bell ();
  }

Tfraction ()
  {
        chroff startpoint, eolpoint, leftpoint;
        register int i;
        register int ch;

        startpoint = e_dot ();
        eolpoint = e_eoldot ();

        for (i = 0 ; ; i++)
            if (((ch = e_rgetc ()) < '0') || (ch > '9')) break;

        if ((i == 0) || (ch != '/'))
           {
            Tattn_msg ("Not fraction: bad denominator");
            return NULL;
           }

        for (i = 0 ; ; i++)
            if (((ch = e_rgetc ()) < '0') || (ch > '9')) break;

        if (i == 0)
           {
            Tattn_msg ("Not fraction: bad numerator");
            return NULL;
           }

        if ((ch == SP) || (ch == TAB))
           {
            e_gobwsp();
            ch = e_rgetc ();
            e_getc ();
            if ((ch >= '0') && (ch <= '9')) delwhitespace ();
            else e_gofwsp();
           }

        else e_getc ();                 /* space forward */

        leftpoint = e_dot ();

        e_sputz ("{fr");

        e_go (startpoint + (e_eoldot () - eolpoint));
                                        /* adj. for chars added or deleted */
        e_putc ('}');
        e_setcur ();

        buf_tmod (leftpoint - cur_dot); /* Mark buffer modified, etc */
        return 1;
  }


Tslashwarning ()
  {
        register int prevc;

        if (((prevc = Tlefthandchar ()) >= '0') && (prevc <= '9'))
            ring_bell ();
        ed_sins ("/");
 }


Tnlwarning ()
  {
        register int prevc;

        delwhitespace ();
        if (((prevc = Tlefthandchar ()) != '}') && (prevc != '\n'))
            ring_bell ();
        if (e_dot () == e_boldot ())  Tnlnofill ();
        else
           {
            /* if? */ fill_current_line ();
            Tnlnofill ();
           }
        e_setcur ();
  }

Tbaseleader ()
  {
        if (is_numarg)
            ed_insn ('_', numarg);
        else ed_sins ("{jr}");
  }



/* This function scans the daylights out of the file to tell you your
   typestyle, size, and other miscellaneous modes in effect at the point. */


Treport ()
  {
        register int i, j;
        char *prev_str, *tempent;
        register Nametable *tempptr;
        chroff defoffs, modeoffs;

        TFmodedef (&modeoffs, &defoffs);

        for (i = 0 ; Tall_reports [i].im_name ; i++)
           {
            prev_str = Tall_reports [i].im_function (modeoffs, defoffs);

            if (i == 0) saynow (Tall_reports [i].im_prefix);
            else sayntoo (Tall_reports [i].im_prefix);

            if (prev_str == NULL)
                sayntoo ("(???)");

            else
               {
                tempent = NULL;

                if ((tempptr = Tall_reports [i].im_table))
                    for (j = 0 ; tempent = (tempptr+j) -> nt_short ; j++)
                        if (strcmp (tempent, prev_str) == 0) break;

                if (tempent) sayntoo ((tempptr+j) -> nt_long);
                else sayntoo (prev_str);
               }
           }
  }

/* Utility backscanning functions */

char *Tget_font_name ();

char *
TFfontname ()   {Tget_font_name (TFfont (FIND_OFFS, FIND_OFFS));}


char *
Tget_font_name (fontnumch)
  char *fontnumch;
  {
        register int i, j, fnum;
        static char buff [80];

        fnum = atoi (fontnumch);

        for (i = 0 ; Tfont_table [i].f_nos [0] ; i++)
            for (j = 0 ; j < 8 ; j++)
                if (Tfont_table [i].f_nos [j] == fnum)
                   {
                    strcpy (buff, Tfont_table [i].f_name);
                    strcat (buff, Trelfont_names [j]);
                    return buff;
                   }

        Tattn_msg ("Font not known by name");
        strcpy (buff, "Font #");
        strcat (buff, fontnumch);
        return buff;
  }


TFmodedef (modeoffsp, defoffsp)
  chroff *modeoffsp, *defoffsp;
  {
        char fragment [8], *f;
        chroff startpos;
        register int i;

        startpos = e_dot ();
        *modeoffsp = *defoffsp = 0;

        if (Tcontext_search (Tknownsubmodes, modeoffsp) == 0)
            return;

        e_go (*modeoffsp);

        f = fragment;                   /* find its definition */
        *f++ = '{';
        *f++ = 'd';
        for (i = 0 ; i < 3 ; i++) *f++ = e_getc ();
        *f = '\0';

        if (!e_search (fragment, BACKWARD))
           {
            Tattn_msg ("Can't find ");
            sayntoo (fragment);
            Twait_LF ();
           }

        else *defoffsp = e_dot ();
        e_go (startpos);
        return;
  }


static char *Fontflags  [] = {"f0", 0};
static char *Pointflags [] = {"p", 0};
static char *Kernflags  [] = {"ak", "xk", "ae", "xe", 0};
static char *Leadflags  [] = {"l", 0};
static char *Widthflags [] = {"m", 0};
static char *Ragflags   [] = {"xr", "rr", "rl", "rc", 0};
static char *Hyphflags  [] = {"xh", "ah", 0};
static char *Tiltflags  [] = {"xt", "at", 0};

char *TFsubmode ();

char *TFfont (modeoffs, defoffs)
  chroff modeoffs, defoffs;
        {return (TFsubmode (modeoffs, defoffs, Fontflags));}

char *TFpointsize (modeoffs, defoffs)
  chroff modeoffs, defoffs;
        {return (TFsubmode (modeoffs, defoffs, Pointflags));}

char *TFkerning (modeoffs, defoffs)
  chroff modeoffs, defoffs;
        {return (TFsubmode (modeoffs, defoffs, Kernflags));}

char *TFleading (modeoffs, defoffs)
  chroff modeoffs, defoffs;
        {return (TFsubmode (modeoffs, defoffs, Leadflags));}

char *TFpagewidth (modeoffs, defoffs)
  chroff modeoffs, defoffs;
        {return (TFsubmode (modeoffs, defoffs, Widthflags));}

char *TFrag (modeoffs, defoffs)
  chroff modeoffs, defoffs;
        {return (TFsubmode (modeoffs, defoffs, Ragflags));}

char *TFhyph (modeoffs, defoffs)
  chroff modeoffs, defoffs;
        {return (TFsubmode (modeoffs, defoffs, Hyphflags));}

char *TFtilt (modeoffs, defoffs)
  chroff modeoffs, defoffs;
        {return (TFsubmode (modeoffs, defoffs, Tiltflags));}

char *
TFsubmode (modeoffs, defoffs, mode_str_list)
  chroff modeoffs, defoffs;
  char *mode_str_list [];
  {
        chroff origpos, localmark;
        int stripit, i, found;
        char fragment [8], tempstr [8], *resultstr;

        origpos = e_dot ();

        stripit = (mode_str_list [1] == 0);
                        /* if only one mode, must be form "XXnnn", strip XX */

  /* Find the "most recent" (closest to the rear) occurrence of either
     a submode callout control or a specific mode control for these modes */

        if (modeoffs == FIND_OFFS)
            TFmodedef (&modeoffs, &defoffs);

        localmark = modeoffs;

        if ((Tcontext_search (mode_str_list, &localmark) == 0)
                                                        && (modeoffs == 0))
           {
            Tattn_msg ("No previous mode setting for any of: ");
            for (i = 0 ; mode_str_list [i] ; i++)
               {
                sayntoo (mode_str_list [i]);
                sayntoo (" ");
               }
            e_go (origpos);
            return NULL;
           }

        e_go (localmark);                       /* goto most recent control */

        if (localmark == modeoffs)              /* if it is a submode */
           {
            e_go (defoffs);
            found = 0;                          /* then find mode in def */

            for (i = 0 ; mode_str_list [i] ; i++)
               {
                strcpy (tempstr, "/");
                strcat (tempstr, mode_str_list [i]);
                if (Tlinesearch (tempstr))
                   {
                    found = 1;
                    e_igoff (1 - strlen (tempstr));
                    break;
                   }
                e_go (defoffs);
               }

            if (!found)
               {
                Tattn_msg ("Can't find ");
                for (i = 0 ; mode_str_list [i] ; i++)
                   {
                    sayntoo (mode_str_list [i]);
                    sayntoo (" ");
                   }

                sayntoo ("in ");
                sayntoo (fragment);
                Twait_LF ();
                e_go (origpos);
                return NULL;
               }
           }

        resultstr = Textract_field ("/", "}", stripit, mode_str_list [0]);
        if (!resultstr)
           {
            Tattn_msg ("Bad format in ");
            sayntoo (fragment);
           }

        e_go (origpos);
        return resultstr;
  }


Tlinesearch (str)
  char *str;
  {
        chroff startdot, eoldot;

        startdot = e_dot ();
        eoldot = e_eoldot ();
        if (e_search (str, FORWARDS))
            if (e_dot () <= eoldot) return 1;
        e_go (startdot);
        return 0;
  }


Tcontext_search (search_list, mostrecentp)
  char *search_list [];
  chroff *mostrecentp;
  {
        int i, found;
        chroff origdot, dot;
        char tempstr [8];

        origdot = e_dot ();

        for (found = i = 0 ; search_list [i] ; i++)
           {
            strcpy (tempstr, "{");
            strcat (tempstr, search_list [i]);
            if (e_search (tempstr, BACKWARDS))
               {
                found = 1;
                if ((dot = e_dot ()) > *mostrecentp)
                    *mostrecentp = dot + 1;
               }
            e_go (origdot);
           }

        return found;
  }


char *
Textract_field (ch, altch, stripit, prefix)
  char *ch, *altch, *prefix;
  int stripit;
  {
        chroff startpos, firstchpos, lastchpos;
        int success;
        register int i;
        register char *f;
        static char field [12];

        if (stripit) e_igoff (strlen (prefix)); /* skip XX from XXnnnn */

        startpos = e_dot ();

        if (success = Tlinesearch (ch)) /* find end of mode definition */
           {
            firstchpos = e_dot ();
            e_go (startpos);

            if (Tlinesearch (altch))    /* search to first of ch or altch */
                if (e_dot () > firstchpos)
                    e_go (firstchpos);
                else ;
            else e_go (firstchpos);
           }

        else success = Tlinesearch (altch);

        if (success)
           {
            lastchpos = e_dot () - 1;
            e_go (startpos);

            for (f = field, i = startpos ; i < lastchpos ; i++)
                *(f++) = e_getc ();     /* extract the mode */
            *f = '\0';

            f = field;
           }
        else f = NULL;

        e_go (startpos);

        if (stripit)                    /* otherwise, punt */
            e_igoff (- strlen (prefix));

        return f;
  }


Tnlnofill ()
  {
        delwhitespace ();
        e_setcur ();
        ed_crins ();
  }


delwhitespace ()
  {
        e_setcur();
        f_delspc();
  }

char *
itoa (num)
  register int num;
  {
        static char buf [8];
        register char *b;

        buf [7] = '\0';
        b = &(buf [7]);

        do
            *--b = '0' + (num % 10);
          while (num /= 10);

        return b;
  }


Twait_LF ()
  {
        int crap;

        sayntoo (" (HIT LF)");
/*      do crap = getchar (); while (crap != LF); */
        return;
  }

#endif /*ICONOGRAPHICS*/
