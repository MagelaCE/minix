/* who - tell who is currently logged in	Author: Andy Tanenbaum
 * Modified:
 *	allow usage: who [filename]  to print entire file's contents
 *	fix bug with 8-character login names	(nick@nswitgould.oz)
 */

/* Who reads the file /usr/adm/wtmp and prints a list of who is curently
 * logged in.  The format of this file is a sequence of 20-character records,
 * as defined by struct wtmprec below.  There is an implicit assumption that
 * all terminal names are of the form ttyn, where n is a single decimal digit.
 */

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

#define SLOTS   10
#define WTMPSIZE 8
#define DIGIT    3

char *fn, *wtmpfile = "/usr/adm/wtmp";
int fromfile = 0;
char name[WTMPSIZE + 1], line[WTMPSIZE + 1];

struct wtmprec {
  char wt_line[WTMPSIZE];	/* tty name */
  char wt_name[WTMPSIZE];	/* user id */
  long wt_time;			/* time */
} wtmp;

struct wtmprec user[SLOTS];
extern char *ctime();

main(argc, argv)
int argc;
char *argv[];
{
  int fd;

#ifdef noperprintf
  noperprintf(stdout);		/* yuk */
#endif
  name[WTMPSIZE] = line[WTMPSIZE] = 0;

  if (argc == 2) {
	fromfile = 1;
	fn = argv[1];
  }
  fd = open(fromfile ? fn : wtmpfile, O_RDONLY);
  if (fd < 0) {
	printf("The file %s cannot be opened.\n", fromfile ? fn : wtmpfile);
	if (!fromfile)
		printf("To enable login accounting (required by who),");
	if (!fromfile) printf("create an empty file with this name.\n");
	exit(1);
  }
  if (fromfile)
	readprint(fd);
  else {
	readwtmp(fd);
	printwtmp();
  }
}


readwtmp(fd)
int fd;
{
/* Read the /usr/adm/wtmp file and build up a log of current users. */

  int i, ttynr;

  while (read(fd, &wtmp, sizeof(wtmp)) == sizeof(wtmp)) {
	if (strcmp(wtmp.wt_line, "~") == 0) {
		/* This line means that the system was rebooted. */
		for (i = 0; i < SLOTS; i++) user[i].wt_line[0] = 0;
		continue;
	}
	ttynr = wtmp.wt_line[DIGIT] - '0';
	if (ttynr < 0 || ttynr >= SLOTS) continue;
	if (wtmp.wt_name[0] == 0) {
		user[ttynr].wt_line[0] = 0;
		continue;
	}
	user[ttynr] = wtmp;
  }
}

printwtmp()
{
  struct wtmprec *w;
  char *p;

  for (w = &user[0]; w < &user[SLOTS]; w++) {
	if (w->wt_line[0] == 0) continue;
	padnprint(w);
  }
}

readprint(fd)
int fd;
{
  /* Read the who-format file and print all entries. */

  while (read(fd, &wtmp, sizeof(wtmp)) == sizeof(wtmp)) {
	padnprint(&wtmp);
  }
}

padnprint(w)
struct wtmprec *w;
{
  char *p;
  int i;

  p = w->wt_name;
  for (i = 0; i < 8 && *p;) name[i++] = *(p++);
  while (i < 8) name[i++] = ' ';
  p = w->wt_line;
  for (i = 0; i < 8 && *p;) line[i++] = *(p++);
  while (i < 8) line[i++] = ' ';
  printf("%s %s ", name, line);
  p = ctime(&w->wt_time);
  *(p + 16) = 0;
  printf("%s\n", p);
}
