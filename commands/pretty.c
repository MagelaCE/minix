/* pretty - MINIX prettyprinter		Author: Andy Tanenbaum */

/* This program is just a post processor for indent.  Unfortunately,
 * indent can't quite produce MINIX format output.  This program takes
 * the output from indent and fixes up a couple of items:
 *
 * 	- the first tab stop is at line_length 3 instead of 9
 *	- short 'if' and 'while' statements should be on a single line
 *	- placement of comments
 *	- return (value)  ==> return(value)
 *	- etc.
 *
 * To use pretty, install "indent" in /bin or /usr/bin, and put
 * a file named ".indent.pro" in your home directory.  This file must
 * contain a single line with the following contents:
 *
 * -bap -bbb -br -ncdb -cli0.5 -di1 -lp -npsl -nfc1 -nip
 *
 * Do not include the asterisk, i.e., -bap starts in column 1.
 * Do not modify .indent.pro as this program expects this version only.
 */

#include <stdio.h>

#define MAX_LINE           78
#define BUF_SIZE         1024
#define TAB                 8
#define COM_COL            33

#define skip_whitespace(p)  while (*p == ' ' || *p == '\t') p++;

char buf[BUF_SIZE];
char buf2[BUF_SIZE];
char buf3[BUF_SIZE];

main(argc, argv)
int argc;
char *argv[];
{

  int i;

  if (argc < 2) usage();
  for (i = 1; i < argc; i++) {
	indent(argv[i]);
	process(argv[i]);
  }
}

indent(s)
char *s;			/* name of file to prettyprint */
{
/* Call indent to indent the file.  Then rename it. */
  int n;
  char buf[BUF_SIZE];

  strcpy(buf, "indent ");
  strcat(buf, s);
  strcat(buf, "\n");
  n = system(buf);
  if (n < 0) {
	fprintf(stderr, "pretty: cannot indent %s\n", s);
	exit(1);
  }
  /* Rename the intermediate file. */
  strcpy(buf, "mv ");
  strcat(buf, s);
  strcat(buf, " +");
  strcat(buf, s);
  strcat(buf, "\n");
  n = system(buf);
  if (n < 0) {
	fprintf(stderr, "pretty: mv %s +%s failed\n", s, s);
	exit(1);
  }
}

process(s)
char *s;			/* name of file to prettyprint */
{
/* File is now indented.  Post process it. */

  int t, m, tabcol;
  FILE *in, *out;
  char *p;

  /* If the original file was called abc, the output from indent is now
   * +abc. */
  strcpy(buf, "+");
  strcat(buf, s);
  in = fopen(buf, "r");
  if (in == NULL) {
	fprintf(stderr, "pretty: cannot open %s\n", buf);
	exit(1);
  }
  /* Create the output file. */
  out = fopen(s, "w");
  if (out == NULL) {
	fprintf(stderr, "pretty: cannot open %s for writing\n", s);
	exit(1);
  }
  /* Process the file a line at a time. */
  fgets(buf, BUF_SIZE, in);
  fgets(buf2, BUF_SIZE, in);
  while (1) {
	if (fgets(buf3, BUF_SIZE, in) == NULL) {
		if (buf[0] == '\0') {
			fclose(in);
			fclose(out);
			return;
		} else {
			buf3[0] = '\0';
		}
	}
	do_return(buf3);

	/* Lines beginning with \t ==> "  "; \t\t ==> \t */
	if (buf[0] == '\t') {
		/* Check for any subsequent tabs; add 1 if found. */
		p = &buf[0];
		if (buf[1] == '\t') {
			/* Line starts with two tabs. */
			shift_left(buf);

			/* If there are more tabs, compensate for
			 * lost tab. */
			skip_whitespace(p);
			while (*p != '\t' && *p != '\n') p++;
			if (*p == '\t') {
				shift_right(p);
				*p = '\t';	/* insert new tab */
			}
		} else {
			/* Line starts with one tab. */
			shift_right(buf);
			buf[0] = ' ';
			buf[1] = ' ';
		}
	}

	ifwhile(in, out);
	do_return(buf);
	do_case(in);
	place_comment(buf);	/* align comment right */
	splice_comment(in);
	fputs(buf, out);
	strcpy(buf, buf2);
	strcpy(buf2, buf3);
  }
}


join()
{
  char *p, *q;
  int col, line_length();

  /* Don't join two if statements. */
  p = buf2;
  skip_whitespace(p);
  if (strncmp(p, "if ", 3) == 0) return (MAX_LINE + 100);

  /* Don't join if the third line is 'else'. */

  p = buf3;
  skip_whitespace(p);
  if (strncmp(p, "else", 4) == 0) return(MAX_LINE + 100);

  /* Determine how long the joined statements will be. */
  q = buf2;
  skip_whitespace(q);
  col = line_length(buf) + line_length(q) + 1;
  return (col);
}


shift_left(p)
register char *p;
{
/* Copy a string to the left 1 line_length. */

  do {
	*p = *(p + 1);
	p++;
  } while (*p != 0);
}

shift_right(p)
register char *p;
{
/* Copy a string to the right 1 line_length. */

  register char *q;

  q = p + strlen(p);		/* points to 0 byte */
  while (q >= p) {
	*(q + 1) = *q;		/* move character to the right */
	q--;
  }
}

int line_length(p)
char *p;
{
/* Determine the length of the line, handling tabs correctly. */

  int col = 1;

  while (*p != '\n' && *p != '\0') {
	if (*p != '\t') {
		col++;
	} else {
		col += TAB - ((col - 1) % TAB);
	}
	p++;
  }
  return (col - 1);
}

place_comment(buf)
char *buf;
{
/* See if there is a comment that has to be readjusted to get it right. */

  int col, col2, comcol, last, quotes;
  char tmp[BUF_SIZE];
  char *p, *q, *rb, *rt;

  col = 1;
  p = buf;

  /* Skip any initial white space in buf. */
  while (*p == ' ' || *p == '\t') {
	if (*p == ' ') col++;
	else
		col += TAB - (col - 1) % TAB;
	p++;
  }
  if (*p == '\n') return;		/* empty line */
  if (*p == '/' && *(p + 1) == '*')
	return;			/* line starts with comment */

  /* Scan forward looking for a comment. */
  quotes = 0;
  while (1) {
	if (*p == '\n') return;
	if (*p == '"')  quotes++;	/* careful about strings */
	if (*p == '/' && *(p + 1) == '*') break;
	if (*p != '\t') col++;
	else
		col += TAB - (col - 1) % TAB;
	if (*p != ' ' && *p != '\t') last = col;
	p++;
  }

  /* We found a comment.  Where should it go? */
  if (quotes & 1) return;	/* if this is inside a string, return */
  if (last <= COM_COL) {
	comcol = COM_COL;
  } else {
	comcol = ((last - 1) / TAB) * TAB + TAB + 1;
  }
  if (comcol == col) return;		/* it is ok as is */

  if (comcol > col) {
	/* Move comment to the right. */
	shift_right(p);
	*p = '\t';
	return;
  }
  /* Move comment to the left. */
  q = p - 1;			/* q points to char before comment */
  while (*q == ' ' || *q == '\t') q--;
  rb = buf;
  rt = tmp;
  col2 = 1;
  while (rb <= q) {
	if (*rb != '\t') col2++;
	else
		col2 += TAB - (col2 - 1) % TAB;
	*rt++ = *rb++;		/* copy one character */
  }
  while (col2 < comcol) {
	*rt++ = '\t';
	col2 += TAB - (col2 - 1) % TAB;
  }
  *rt = 0;
  strcat(rt, p);
  strcpy(buf, tmp);
}


ifwhile(in, out)
FILE *in, *out;
{
/* Check for 'if' or 'while' split over two lines. */

  int is_if, is_while;
  char *first1, *first2, *second1;

	/* Check for 'if' or 'while' statements split over two lines. */
	first1 = &buf[0];
	while (*first1 == ' ' || *first1 == '\t') first1++;
	first2 = first1 + strlen(first1) - 2;		/* last char */
	while (first2 > &buf[0] && (*first2 == ' ' || *first2 == '\t'))
		first2--;
	
	/* first1 and first2 now point to first/last nonblank chars. */
	is_if = (strncmp(first1, "if ", 3) == 0 ? 1 : 0);
	is_while = (strncmp(first1, "while ", 6) == 0 ? 1 : 0);

	if ( (is_if || is_while) && *first2 == ')') {
		/* This is an 'if' or 'while' statement ending with ')'. */
		if (join() < MAX_LINE) {
			*(first2 + 1) = ' ';
			*(first2 + 2) = 0;
			second1 = &buf2[0];
			while (*second1 == ' ' || *second1 == '\t')
				second1++;
			strcat(first2, second1);
			strcpy(buf2, buf3);
			if (fgets(buf3, BUF_SIZE, in) == NULL) buf3[0] = '\0';
		}
	}
}

do_return(b)
char *b;
{
/* Remove the space after return, i.e., return (0) ==> return(0). */

  char *p;

  p = b;
  skip_whitespace(p);
  if (strncmp(p, "return (", 8) != 0) return;
  shift_left(p+6);
}


splice_comment(in)
FILE *in;
{
/* Indent has the problem that it sometimes breaks one line comments over two
 * lines, even though the original comment fir quite well on one line.  Fix it.
 */

  char *p, *q;

  p = buf2;			/* affect lines are followed by  * something */
  skip_whitespace(p);
  if (*p != '*' || *(p+1) != ' ') return;

  /* Second line starts with * something.  See if first one contains comment.*/
  q = buf;
  while (1) {
	while (*q != '/' && *q != '\n') q++;
	if (*q == '\n') return;
	if (*(q+1) == '*') break;
	q++;
  }

  /* We found a comment here.  See if the two lines can be merged. */
  if (line_length(buf) + line_length(p+1) >= MAX_LINE) return;
  q = buf + strlen(buf) - 1;	/* q points to '\n' */
  *q = 0;
  strcat(buf, p+1);
  strcpy(buf2, buf3);
  if (fgets(buf3, BUF_SIZE, in) == NULL) buf3[0] = '\0';
}


usage()
{
  fprintf(stderr, "Usage: pretty file ...\n");
}

do_case(in)
FILE *in;
{
/* Try to get short cases all on 1 line. */

  char *p, *q, *r;

  p = buf;
  skip_whitespace(p);
  if (strncmp(p, "case ", 5) != 0) return;

  q = buf2;
  skip_whitespace(q);

  r = buf3;
  skip_whitespace(r);
  if (strncmp(r, "break;", 6) != 0) return;

  /* This is a case statement and the case is only 1 line long (+ break). */
  if (line_length(buf) + 3*TAB + line_length(q) + line_length(r) > MAX_LINE) 
	return;
  shift_right(p);  *p++ = ' ';
  shift_right(p);  *p++ = ' ';
  shift_right(p);  *p++ = ' ';
  shift_right(p);  *p++ = ' ';
  p = buf + strlen(buf) - 1;
  *p = 0;
  strcat(buf, "\t");
  strcat(buf, q);
  p = buf + strlen(buf) - 1;
  *p = 0;
  strcat(buf, "\t");
  strcat(buf, r);
  if (fgets(buf2, BUF_SIZE, in) == NULL) buf2[0] = '\0';
  if (fgets(buf3, BUF_SIZE, in) == NULL) buf3[0] = '\0';
}
