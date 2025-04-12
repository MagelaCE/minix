/*
 * UMAIL -	MINIX Remote Domain-adressing Mail Router
 *
 *		This version of RMAIL handles message-headers in a much
 *		more "standard" way. It can handle bang-adresses, plus
 *		the new-style Internet adressing (user@host.domain).
 *		It is called by programs as "Mail" and "Uuxqt".
 *
 *          	C O N F I G U R A T I O N   F I L E   S C A N N E R
 *
 * Author:	F. van Kempen, Jul-Oct '89 (waltje@minixug.nluug.nl)
 */
#include <stdio.h>


typedef struct {
    char *name;
    int opcode;
} WORD;


/* operator opcodes */
#define O_ASSIGN        1       /* ":="             */
#define O_COMMA         2       /* ","              */
#define O_BEGIN         3       /* "BEGIN"          */
#define O_END           4       /* "END"            */
#define O_DATA          5       /* "DATA"           */
#define O_NAMES         6       /* "NAMES"          */
#define O_DOMAIN        7       /* "DOMAINS"        */
#define O_HOST          8       /* "HOST"           */


/* main scanner states */
#define S_LOOK          1       /* looking for a keyword */
#define S_BEGIN         2       /* got O_BEGIN */
#define S_END           3       /* got O_END */

/* block classes */
#define SC_DATA         1       /* DATA (variable) class */
#define SC_NAMES        2       /* NAMES (my domains) class */
#define SC_DOMAIN       3       /* DOMAIN TABLE class */
#define SC_HOST         4       /* HOSTS class */

#define SS_LOOK         1       /* looking for keyword or variable name */
#define SS_IDENT        2       /* looking for identifier */
#define SS_ASSIGN       3       /* got varname, looking for O_COMMA */
#define SS_COMMA        4       /* got varname, looking for O_ASSIGN */
#define SS_VALUE        5       /* got O_ASSIGN, looking for value */
#define SS_DOMAIN1      6       /* got domain, looking for hostname */
#define SS_DOMAIN2      7       /* got boolean, looking for domain descr. */
#define SS_HOST1        8       /* got host, looking for boolean */
#define SS_HOST2        9       /* got host, looking for host mailer name */
#define SS_HOST3       10       /* got host, looking for host mailer opts */


static FILE *infp;                      /* input file pointer */
static char scbuf[1024];                /* input line buffer */
static char *scptr = NULL;              /* input line pointer */
static char *errptr = NULL;		/* error pointer */
static char sctemp1[128];               /* current identifier */
static char sctemp2[128];               /* current identifier */
static int scbool;			/* temp. boolean value */
static int lineno = 1;                  /* current line number */
static int sclass = SC_DATA;            /* temporary state */
static int state = S_LOOK;              /* state of scanner */
static int sstate = SS_LOOK;            /* secondairy state */
static int tstate = SS_LOOK;		/* triple state */
static WORD table[] = {                 /* language table */
  { ":="    	,   O_ASSIGN    },
  { ","     	,   O_COMMA     },
  { "BEGIN" 	,   O_BEGIN     },
  { "END"   	,   O_END       },
  { "DATA"  	,   O_DATA      },
  { "NAMES" 	,   O_NAMES     },
  { "DOMAINS" 	,   O_DOMAIN    },
  { "HOSTS" 	,   O_HOST      },
};


extern int debug;


/*
 * Return next character of input file;
 * also do some bookkeeping for error-recovery.
 */
static int nextch(void)
{
  register int ch;

  ch = fgetc(infp);
  if (ch == '\n') {
	lineno++;
	*scptr = '\0';
	scptr = scbuf;
  } else *scptr++ = ch;
  return(ch);
}


/*
 * Handle a syntax error.
 * Also, perform some error recovery.
 */
static void syntax(s)
char *s;
{
  register char *bp, *ep;
  register int ch;

  ep = errptr;
  do {
      ch = nextch();			/* read up to end of line */
  } while (ch!='\n' && ch!=EOF);	/* and start over on next line */

  sstate = SS_LOOK;     		/* reset state machine #2 */

  fprintf(stderr, "%05.5d %s\n      ", lineno, scbuf);
  bp = scbuf;
  ep--;
  while (bp < ep) {
	fprintf(stderr, " ");
	bp++;
  }
  fprintf(stderr, "^ %s\n", s);
}


/*
 * Check the text of a keyword.
 */
static int crunch(text)
char *text;
{
  register WORD *wp;

  wp = &table[0];
  while (wp->opcode != 0) {
	if (!strcmp(wp->name, text)) return(wp->opcode);
	wp++;
  }
  return(0);
}


/*
 * Decode a word, and perform some action if necessary.
 * This routine holds all the syntax grammar.
 * It is far from perfect, but it works...
 */
static void do_word(name)
char *name;
{
  int op;               /* decoded keyword code */
  char *s = "expected END";

  op = crunch(name);
  if (sstate == SS_LOOK) switch(op) {
	case O_DATA:
        	if (state==S_LOOK || state==S_END) sclass = SC_DATA;
		  else syntax(s);
		break;
        case O_NAMES:
		if (state==S_LOOK || state==S_END) sclass = SC_NAMES;
		  else syntax(s);
		break;
        case O_DOMAIN:
		if (state==S_LOOK || state==S_END) sclass = SC_DOMAIN;
		  else syntax(s);
		break;
        case O_HOST:
            	if (state==S_LOOK || state==S_END) sclass = SC_HOST;
              	  else syntax(s);
		break;
        case O_BEGIN:
            	switch(sclass) {
                	case SC_DATA:
                	case SC_DOMAIN:
                	case SC_HOST:
			case SC_NAMES:
                		sstate = SS_LOOK;
                		state = S_BEGIN;
                		break;
                	default:
                    		syntax("expected class prefix");
                    		break;
            	}
            	break;
        case O_END:
            	if (state == S_BEGIN) {
			state = S_LOOK;
			sclass = 0;
	    	} else syntax("expected BEGIN");
            	break;
        default:
            	sstate = SS_IDENT;
            	break;
  }
  switch(sstate) {
    	case SS_LOOK:		/* propagated from the above switch() */
        	break;
	case SS_IDENT:      	/* looking for identifier */
		switch(sclass) {
			case SC_DATA:
			case SC_HOST:
			case SC_DOMAIN:
    				strcpy(sctemp1, name);
				sstate = SS_ASSIGN;
		        	break;
			case SC_NAMES:
				add_name(name);
				sstate = SS_LOOK;
				break;
			default:
				syntax("expected BEGIN or CLASS");
		}
		break;
    	case SS_ASSIGN:     	/* looking for O_ASSIGN */
		op = crunch(name);
        	if (op == O_ASSIGN) switch(sclass) {     /* found O_ASSIGN */
            		case SC_DATA:
                		sstate = SS_VALUE;
                		break;
            		case SC_DOMAIN:
                		sstate = SS_DOMAIN1;
                		break;
            		case SC_HOST:
                		sstate = SS_HOST1;
                		break;
            		default:
                		break;
       		 } else syntax("expected ASSIGN");
        	break;
	case SS_COMMA:		/* field separator */
		op = crunch(name);
		if (op == O_COMMA) switch(sclass) {
			case SC_DOMAIN:
				sstate = SS_DOMAIN2;
				break;
			case SC_HOST:
				switch(tstate) {
					case SS_HOST1:
						sstate = SS_HOST2;
						break;
					case SS_HOST2:
						sstate = SS_HOST3;
						break;
					default:
						syntax("no comma here");
						break;
				}
				break;
			default:
				syntax("no comma here");
				break;
		} else syntax("expected COMMA");
		break;
    	case SS_VALUE:      	/* looking for value */
        	add_var(sctemp1, name);
        	sstate = SS_LOOK;
        	break;
	case SS_DOMAIN1:		/* looking for route hostname */
		strcpy(sctemp2, name);
		sstate = SS_COMMA;
		break;
   	case SS_DOMAIN2:		/* looking for host route */
        	add_route(sctemp1, sctemp2, name);
        	sstate = SS_LOOK;
        	break;
    	case SS_HOST1:		/* looking for host 'smart' boolean */
        	scbool = boolean(name);
		sstate = SS_COMMA;
		tstate = SS_HOST1;
        	break;
    	case SS_HOST2:		/* looking for host mailer name */
		strcpy(sctemp2, name);	/* mailer name */
        	sstate = SS_COMMA;	/* look for mailer opts */
		tstate = SS_HOST2;
        	break;
    	case SS_HOST3:		/* looking for host mailer opts */
		add_host(sctemp1, scbool, sctemp2, name);
		sstate = SS_LOOK;
        	break;
    	default:		/* this can't be for real! */
        	syntax("Huh? You must be joking!");
        	break;
  }
}


/*
 * This is the Configuration File Scanner.
 * It has *some* level of intelligence, but is must
 * be improved a lot.
 */
int scanner(fname)
char *fname;
{
  char wordbuf[512];
  register char *bp = wordbuf;
  register int ch;
  register int quote = 0;
  register int stopped = 0;

  infp = fopen(fname, "r");
  if (infp == (FILE *)NULL) return(-1);

  scptr = scbuf;
  ch = nextch();
  do {
      switch(ch) {
 	case '#':   /* comment, skip rest of line */
        	do {
                    ch = nextch();
            	   } while (ch != '\n');
            	break;
        case ' ':   /* SPACE */
        case '\t':
            	if (quote == 0) {
			errptr = scptr;
                	do {    /* skip rest of leading space */
                    	    ch = nextch();
                	   } while (ch==' ' || ch=='\t');
                	*bp = '\0';
                	if (bp != wordbuf) {
                    		do_word(wordbuf);
                    		bp = wordbuf;
                	}
            	} else {
                    	*bp++ = ch;
                    	ch = nextch();
              	  }
            	break;
        case EOF:   /* EOF, done! */
            	stopped = 1;
        case '\n':  /* end-of-word marker, handle word */
            	*bp = '\0';
            	if (wordbuf[0]) {
                	do_word(wordbuf);
                	bp = wordbuf;
            	}
            	if (ch != EOF) ch = nextch();
            	break;
        case '"':   /* quote, set/reset quoting flag */
            	quote = 1 - quote;
            	ch = nextch();
            	break;
        default:    /* anything else: text */
            	*bp++ = ch;
            	ch = nextch();
            	break;
      }
  } while (!stopped);

  fclose(infp);

  return(0);
}
