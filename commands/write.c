/*  write(1) - write to a logged-in user	Author: Nick Andrew */

/*  Author: Nick Andrew  (nick@nswitgould.oz)  - Public Domain
 *  Minix version: 1.4a, 30 March 1989
 *
 *  Usage:  write  [flags] user [tty]
 *  flags:	-c	Read & write one character at a time (cbreak mode)
 *		-v	Verbose
 *
 * NOTES:
 *	Write requires 1.4a (or higher) libraries, for getopt(), strchr().
 *
 * BUGS:
 *	Shell escape is not supported when in cbreak mode.
 *	Verbose mode is ineffectual.
 */

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <pwd.h>
#include <sgtty.h>
#include <signal.h>

extern long time();
extern char *ttyname();
extern struct passwd *getpwnam();
extern char *getenv();
extern char *ctime();
extern int optind;

long now;

int otty, i, wtmpfd;

char *user = NULL, *tty = NULL, wtmptty[8], optstring[] = "cv";
char *ourtty, othertty[16], line[80], ourname[9];
char *wtmpfile = "/usr/adm/wtmp";

int c, cbreak = 0, verbose = 0, writing = 0;

struct passwd *userptr;
struct sgttyb ttyold, ttynew;

struct wtmprec {
  char wt_line[8];
  char wt_name[8];
  long wt_time;
} wtmp;

#define show(var,true,false) \
  fprintf(stderr,"var:\t%s\n", var ? true : false)

main(argc, argv)
int argc;
char *argv[];
{
  setbuf(stdout, NULL);

  /* Parse options */
  while ((c = getopt(argc, argv, optstring)) != EOF) switch (c) {
	    case 'c':	cbreak = 1;	break;
	    case 'v':	verbose = 1;	break;
	    default:	usage();
	}

  /* Parse user and tty arguments */
  if (optind < argc) {
	user = argv[optind++];
	if (strlen(user) > 8)
		err("username must be 1 to 8 characters long\n");
	if (optind < argc) {
		tty = argv[optind++];
		if (optind < argc) usage();
	}
  } else {
	usage();
  }

  finduser();			/* find which tty to write onto */
  settty();			/* setup our terminal */
  sayhello();			/* print the initial message */
  writetty();			/* the write loop */

  stty(0, &ttyold);
  exit(0);
}

usage()
{
  fprintf(stderr, "usage: write [flags] user [tty]\n");
  fprintf(stderr, "flags: -c == cbreak mode, -v == verbose\n");
  exit(255);
}


finduser()
{
/* Search the accumulated who file for the user we want */
  ourtty = ttyname();
  if (ourtty == NULL) ourtty = "/dev/tty0";

  if (user == NULL) exit(1);
  if ((userptr = getpwnam(user)) == NULL) {
	fprintf(stderr, "No such user: %s\n", user);
	exit(1);
  }
  fprintf(stderr, "Trying to write to %s\n", userptr->pw_gecos);

  if ((wtmpfd = open(wtmpfile, O_RDONLY)) < 0)
	err("Cannot open wtmp file\n");

  wtmptty[0] = '\0';
  while (read(wtmpfd, &wtmp, sizeof(wtmp)) == sizeof(wtmp)) {

	/* We want to find if steve is logged on, and return in
	 * wtmptty[] steve's terminal, and if steve is logged onto
	 * the tty the user specified, return that tty name */

	/* Reboot, nobody's logged on */
	if (!strcmp(wtmp.wt_line, "~")) {
		wtmptty[0] = '\0';
		continue;
	}

	/* We found a tty that steve used, but this is a logoff */
	if (!strcmp(wtmp.wt_line, wtmptty) && wtmp.wt_name[0] == 0) {
		wtmptty[0] = '\0';
		continue;
	}

	/* Is this steve logging on? */
	if (strcmp(wtmp.wt_name, user)) continue;

	/* Is he on the terminal we want to write to? */
	if (tty == NULL || strcmp(wtmptty, tty))	/* not yet apparently */
		strcpy(wtmptty, wtmp.wt_line);	/* on somewhere */
  }

  if (wtmptty[0] == 0) {
	fprintf(stderr, "%s is not logged on\n", user);
	exit(1);
  }
  if ((tty != NULL) && strcmp(wtmptty, tty)) {
	fprintf(stderr, "%s is logged onto %s not %s\n",
		user, wtmptty, tty);
	exit(1);
  }
  fprintf(stderr, "Writing to %s on %s\n", user, wtmptty);
}

err(s)
char *s;
{
  fputs(s, stderr);
  exit(255);
}

intr()
{
/* The interrupt key has been hit. exit cleanly */
  signal(SIGINT, SIG_IGN);
  fprintf(stderr, "\nInterrupt. Exiting write\n");
  stty(0, &ttyold);
  if (writing) write(otty, "\nEOT\n", 5);
  exit(0);
}


settty()
{
/* Open other person's terminal and setup our own terminal */
  sprintf(othertty, "/dev/%s", wtmptty);
  if ((otty = open(othertty, O_WRONLY)) < 0) {
	fprintf(stderr, "Cannot open %s to write to %s\n", wtmptty, user);
	fprintf(stderr, "It may have write permission turned off\n");
	exit(1);
  }
  gtty(0, &ttyold);
  ttynew = ttyold;
  ttynew.sg_flags |= CBREAK;
  signal(SIGINT, intr);
  if (cbreak) stty(0, &ttynew);
}

sayhello()
{
  now = time((long *) 0);
  printf("Message from %s on %s at %s\n",
         getenv("USER"), ourtty, ctime(&now));
}


writetty()
{
/* The write loop */
  int n;

  writing = 1;
  while ((n = read(0, line, 79)) > 0) {
	if (line[0] == '!' && !cbreak)
		escape();
	else
		write(otty, line, n);
  }
  write(1, "\nEOT\n", 5);
  write(otty, "\nEOT\n", 5);
}


escape()
{
/* Shell escape */

  char *x;

  write(1, "!\n", 2);
  for (x = line; *x; ++x)
	if (*x == '\n') *x = 0;
  system(&line[1]);
  write(1, "!\n", 2);
}
