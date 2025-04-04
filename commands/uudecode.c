/* uudecode - decode a uuencoded file */

/* call:   uudecode [input_file]	*/

/* hgm - June 9, 1987 - added code to handle short lines and tabs */

#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

extern FILE *fopen();
char *fgets();
#define NULLF (FILE *) 0
#define NULLP (char *) 0

/* single character decode */
#define DEC(c)	(((c) - ' ') & 077)

main(argc, argv)
char **argv;
{
	FILE *in, *out;
	struct stat sbuf;
	int mode;
	char dest[128];
	char buf[80];

	/* optional input arg */
	if (argc > 1) {
		if ((in = fopen(argv[1], "r")) == NULLF) {
			perror(argv[1]);
			exit(1);
		}
		argv++; argc--;
	} else
		in = stdin;

	if (argc != 1) {
		printf("Usage: uudecode [infile]\n");
		exit(2);
	}

	/* search for header line */
	for (;;) {
		if (fgets(buf, sizeof buf, in) == NULLP) {
			fprintf(stderr, "No begin line\n");
			exit(3);
		}
		if (strncmp(buf, "begin ", 6) == 0)
			break;
	}
	sscanf(buf, "begin %o %s", &mode, dest);

	/* handle ~user/file format */
	if (dest[0] == '~') {
		char *sl;
		struct passwd *getpwnam();
		char *uindex();
		struct passwd *user;
		char dnbuf[100];

		sl = uindex(dest, '/');
		if (sl == NULLP) {
			fprintf(stderr, "Illegal ~user\n");
			exit(3);
		}
		*sl++ = 0;
		user = getpwnam(dest+1);
		if (user == NULL) {
			fprintf(stderr, "No such user as %s\n", dest);
			exit(4);
		}
		strcpy(dnbuf, user->pw_dir);
		strcat(dnbuf, "/");
		strcat(dnbuf, sl);
		strcpy(dest, dnbuf);
	}

	/* create output file */
	out = fopen(dest, "w");
	if (out == NULLF) {
		perror(dest);
		exit(4);
	}
	chmod(dest, mode);

	decode(in, out);

	if (fgets(buf, sizeof buf, in) == NULLP || strcmp(buf, "end\n")) {
		fprintf(stderr, "No end line\n");
		exit(5);
	}
	exit(0);
}

/*
 * copy from in to out, decoding as you go along.
 */
decode(in, out)
FILE *in;
FILE *out;
{
	char tbuf[82];
	char buf[82];
	char *bp;
	char *p;
	int n;
	int col;

	for (;;) {
		/* for each input line */
		if (fgets(tbuf, sizeof tbuf, in) == NULLP) {
			printf("Short file\n");
			exit(10);
		}
		n = tbuf[0] - ' ';
		if (n <= 0)
			break;

		/* expand tabs */
		for (col = 0, p = tbuf; ((*p) && (*p != '\n') && (col < 78)); p++) {
			if (*p == '\t') {
				do {
					buf[col++] = ' ';
				} while (col % 8);
			} else {
				buf[col++] = *p;
			}
		}

		/* fill with trailing blanks */
		for (; ((col < (4*n + 1)) && (col < 78)); col++)
			buf[col] = ' ';
		buf[col++] = '\n';
		buf[col] = '\0';

		bp = &buf[1];
		while (n > 0) {
			outdec(bp, out, n);
			bp += 4;
			n -= 3;
		}
	}
}

/*
 * output a group of 3 bytes (4 input characters).
 * the input chars are pointed to by p, they are to
 * be output to file f.  n is used to tell us not to
 * output all of them at the end of the file.
 */
outdec(p, f, n)
char *p;
FILE *f;
{
	int c1, c2, c3;

	c1 = DEC(*p) << 2 | DEC(p[1]) >> 4;
	c2 = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
	c3 = DEC(p[2]) << 6 | DEC(p[3]);
	if (n >= 1)
		putc(c1, f);
	if (n >= 2)
		putc(c2, f);
	if (n >= 3)
		putc(c3, f);
}


/* fr: like read but stdio */
int
fr(fd, buf, cnt)
FILE *fd;
char *buf;
int cnt;
{
	int c, i;

	for (i=0; i<cnt; i++) {
		c = getc(fd);
		if (c == EOF)
			return(i);
		buf[i] = c;
	}
	return (cnt);
}

/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 */


char *
uindex(sp, c)
register char *sp, c;
{
	do {
		if (*sp == c)
			return(sp);
	} while (*sp++);
	return(NULL);
}
