#include <ctype.h>
/*
 * Bawk constants and variable declarations.
 */

#ifdef BDS_C
#define EXTERN
#else

#ifdef MAIN
#define EXTERN
#else
#define EXTERN extern
#endif

#endif


#ifdef DEBUG
EXTERN char Debug;              /* debug print flag */
#endif

/*
 * Table and buffer sizes
 */
#define MAXLINELEN      128
#define MAXWORDS        (MAXLINELEN/2)
#define MAXWORKBUFLEN   4096
#define MAXVARTABSZ     50
#define MAXVARLEN       10
#define MAXSTACKSZ      40


/**********************************************************
 * Current Input File variables                           *
 **********************************************************/
/*
 * Current Input File pointer:
 */
#ifdef BDS_C
EXTERN char *Fileptr, Curfbuf[BUFSIZ];
#else
EXTERN FILE *Fileptr;
#endif
EXTERN char *Filename;          /* current input file name */
EXTERN int Linecount;           /* current input line number */
EXTERN int Recordcount;         /* record count */
/*
 * Working buffers.
 */
EXTERN char Linebuf[MAXLINELEN];/* current input line buffer */
EXTERN char *Fields[MAXWORDS];  /* pointers to the words in Linebuf */
EXTERN int Fieldcount;          /* and the # of words */
EXTERN char Workbuf[MAXWORKBUFLEN];     /* work area for C action and */
 /* regular expression parsers */

/**********************************************************
 * Regular Expression Parser variables                    *
 **********************************************************/
/*
 * Tokens:
 */
#define CHAR    1
#define BOL     2
#define EOL     3
#define ANY     4
#define CLASS   5
#define NCLASS  6
#define STAR    7
#define PLUS    8
#define MINUS   9
#define ALPHA   10
#define DIGIT   11
#define NALPHA  12
#define PUNCT   13
#define RANGE   14
#define ENDPAT  15


/**********************************************************
 * C Actions Interpreter variables                        *
 **********************************************************/
/*
 * Tokens:
 */
#define T_STRING        'S'
#define T_DOLLAR        '$'
#define T_REGEXP        'r'
#define T_CONSTANT      'C'
#define T_VARIABLE      'V'
#define T_FUNCTION      'F'
#define T_SEMICOLON     ';'
#define T_EOF           'Z'
#define T_LBRACE        '{'
#define T_RBRACE        '}'
#define T_LPAREN        '('
#define T_RPAREN        ')'
#define T_LBRACKET      '['
#define T_RBRACKET      ']'
#define T_COMMA         ','
#define T_ASSIGN        '='
#define T_MUL           '*'
#define T_DIV           '/'
#define T_MOD           '%'
#define T_ADD           '+'
#define T_SUB           '-'
#define T_SHL           'L'
#define T_SHR           'R'
#define T_LT            '<'
#define T_LE            'l'
#define T_GT            '>'
#define T_GE            'g'
#define T_EQ            'q'
#define T_NE            'n'
#define T_NOT           '~'
#define T_AND           '&'
#define T_XOR           '^'
#define T_IOR           '|'
#define T_LNOT          '!'
#define T_LAND          'a'
#define T_LIOR          'o'
#define T_INCR          'p'
#define T_DECR          'm'
#define T_IF            'i'
#define T_ELSE          'e'
#define T_WHILE         'w'
#define T_BREAK         'b'
#define T_CHAR          'c'
#define T_INT           't'
#define T_BEGIN         'B'
#define T_END           'E'
#define T_NF            'f'
#define T_NR            '#'
#define T_FS            ' '
#define T_RS            '\n'
#define T_FILENAME      'z'

#define PATTERN 'P'
#define ACTION  'A'

/*
 * Symbol Table values
 */
#define ACTUAL          0
#define LVALUE          1
#define BYTE            1
#define WORD            2
/*
 * Symbol table
 */
struct variable {
        char vname[MAXVARLEN];
        char vclass;
        char vsize;
        int vlen;
        char *vptr;
};
#define VARIABLE struct variable
EXTERN VARIABLE Vartab[MAXVARTABSZ], *Nextvar;
/*
 * Value stack
 */
union datum {
        int ival;
        char *dptr;
        char **ptrptr;
};
#define DATUM union datum
struct item {
        char class;
        char lvalue;
        char size;
        DATUM value;
};
#define ITEM struct item
EXTERN ITEM Stackbtm[MAXSTACKSZ], *Stackptr, *Stacktop;
/*
 * Miscellaneous
 */
EXTERN char *Actptr;            /* pointer into Workbuf during compilation */
EXTERN char Token;              /* current input token */
EXTERN DATUM Value;             /* and its value */
EXTERN char Saw_break;          /* set when break stmt seen */
EXTERN char Where;              /* indicates whether C stmt is a PATTERN or
                                 * ACTION */
EXTERN char Fieldsep[3];        /* field seperator */
EXTERN char Recordsep[3];       /* record seperator */
EXTERN char *Beginact;          /* BEGINning of input actions */
EXTERN char *Endact;            /* END of input actions */

/**********************************************************
 * Rules structure                                        *
 **********************************************************/
struct rule {
        struct {
                char *start;    /* C statements that match pattern start */
                char *stop;     /* C statements that match pattern end */
                char startseen; /* set if both a start and stop pattern */
                /* given and if an input line matched the */
                /* start pattern */
        } pattern;
        char *action;           /* contains quasi-C statements of actions */
        struct rule *nextrule;  /* pointer to next rule */
};
#define RULE struct rule
EXTERN RULE *Rules,             /* rule structures linked list head */
*Rulep;                         /* working pointer */


/**********************************************************
 * Miscellaneous                                          *
 **********************************************************/
/*
 * Error exit values (returned to command shell)
 */
#define USAGE_ERROR     1
#define FILE_ERROR      2
#define RE_ERROR        3
#define ACT_ERROR       4
#define MEM_ERROR       5
/*
 * Functions that return something special:
 */
char *
str_compile(), *getmem(), *cclass(), *pmatch(), *fetchptr();
char *storeptr();
VARIABLE *
findvar(), *addvar(), *decl();
