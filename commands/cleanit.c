/* cleanit - clean up mail message	Author: Andy Tanenbaum */

/* When a program comes in over USENET, it has a header on the front
 * and (usually) a signature at the end.  This program strips off this
 * junk to get at the program (or shar file or whatever).  It uses the
 * concept of a "cut line", which is a line beginning with >= 20 hyphens,
 * like this:
------------------------- cut here -------------------------
 * Cleanit assumes that the program being sent starts with a cut line and
 * ends with a cut line.  It strips off everything up to and including
 * the first cut line and everything from the second cut line to the end.
 * What is left is the program being transmitted.  The original file is
 * left intact on a file whose name is the original name with # attached.
 *
 * Examples:
 *	cleanit x y z		# three files are cleaned up
 * 	cat x | cleanit > y	# clean x and write it on y
 */

#include <stdio.h>

#define MAX_NAME   14
#define BUF_SIZE 1024
#define CUT_LINE "--------------------"

main(argc, argv)
int argc;
char *argv[];
{

  int i;
  FILE *in, *out;
  char buf[BUF_SIZE], *p, *q;

  if (argc == 1) {
	clean(stdin, stdout);
	exit(0);
  } else {
	for (i = 1; i < argc; i++) {
		/* For each file, open it, rename it, and copy it. */
		p = argv[i];
		in = fopen(p, "r");
		if (in == NULL) {
			fprintf(stderr, "cleanit: cannot open %s\n", p);
			continue;
		}
		/* Build string to pass to system to rename file. */
		strcpy(buf, "mv ");
		strcat(buf, p);
		strcat(buf, " ");
		q = buf + strlen(buf);
		strcat(buf, p);
		if (strlen(argv[i]) >= MAX_NAME) {
			*(q + MAX_NAME - 1) = 0;
		}
		strcat(buf, "#");
		system(buf);		/* rename the file */
		if ( (out = fopen(p, "a")) == NULL) {
			fprintf(stderr, "cleanit: cannot create %s\n", p);
			continue;
		}
		clean(in, out, p);
		fclose(in);
		fflush(out);
		fclose(out);
	}
  }
  exit(0);
}

clean(in, out, file)
FILE *in, *out;
char *file;
{
/* Copy the file. */

  char buffer[BUF_SIZE];
  int size;

  size = strlen(CUT_LINE);
  while (1) {
	if (fgets(buffer, BUF_SIZE, in) == NULL) return;
	if (buffer[0] != '-') continue;
	if (strncmp(buffer, CUT_LINE, size) == 0) break;
  }

  /* First cut line has been found. Start copying with next line. */
  while (1) {
	if (fgets(buffer, BUF_SIZE, in) == NULL) return;
	if (strncmp(buffer, CUT_LINE, size) == 0) return;
	if (fputs(buffer, out) == NULL) {
		fprintf("cleanit: write error on %s\n", file);
		return;
	}	
  }
}
