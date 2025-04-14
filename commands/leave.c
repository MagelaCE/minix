/*  leave - tell the user when to go home	Author: Terrence W. Holm */

/*  Usage:  leave  [ [+] hhmm ] */


#include <signal.h>
#include <stdio.h>
#include <utmp.h>

#define  Min(a,b)  ((a<b) ? a : b)

#ifndef  WTMP
#define  WTMP   "/usr/adm/wtmp"
#endif

#define  BUFFER_SIZE     1024	/* Room for wtmp records  */
#define  MAX_WTMP_COUNT  ( BUFFER_SIZE / sizeof(struct utmp) )

struct utmp wtmp_buffer[MAX_WTMP_COUNT];

#define  STRING	   80		/* Lots of room for an argument  */
#define  MIN	   60L		/* Seconds per minute		  */
#define  HOUR      (60L*60L)	/* Seconds per hour		  */
#define  HALF_DAY  (12L*HOUR)	/* Seconds per half day	  */
#define  DAY	   (24L*HOUR)	/* Seconds per day		  */

/*  Set the following to your personal preferences for the	  */
/*  time and contents of warnings.				  */

#define  INTERVALS 13		/* Size of intervals[]		  */
#define  WARNINGS  4		/* Size of warnings[]		  */

int intervals[INTERVALS] = {-5 * MIN, -1 * MIN, 0, MIN, 2 * MIN, 3 * MIN,
      4 * MIN, 5 * MIN, 6 * MIN, 7 * MIN, 8 * MIN, 9 * MIN, 10 * MIN};

char *warnings[WARNINGS] = {
		    "You have to leave within 5 minutes",
		    "Just one more minute!",
		    "Time to leave!",
		    "You're going to be late!"	/* For all subsequent
						 * warnings */
};


#ifdef V7
extern long timezone;
#else
#ifdef BSD
#include <time.h>
long timezone;
#else
extern long timezone;
#endif
#endif


long time();
char *ttyname();
char *cuserid();
char *ctime();


main(argc, argv)
int argc;
char *argv[];

{
  char when[STRING];
  long now = time((long *)0);
  long leave;
  int hour, min;
  int pid;


  /* Get the argument string "when" either from stdin, or argv  */

  if (argc <= 1) {
	printf("When do you have to leave? ");
	if (fgets(when, STRING, stdin) == NULL || when[0] == '\n') exit(0);
  } else {
	strcpy(when, argv[1]);
	if (argc > 2) strcat(when, argv[2]);
  }

  /* Determine the leave time from the current time and "when"  */
  timezone = 0;
  if (when[0] == '+') {
	Get_Hour_Min(&when[1], &hour, &min);
	leave = now + hour * HOUR + min * MIN;
  } else {
	/* User entered an absolute time.  */

#ifdef BSD
	timezone = -localtime(&now)->tm_gmtoff;
#endif

	Get_Hour_Min(&when[0], &hour, &min);

	if (hour >= 1 && hour <= 12) {
		/* 12-hour format: relative to previous midnight or noon.  */

		leave = now - (now - timezone) % HALF_DAY + hour % 12 * HOUR + min * MIN;

		if (leave < now - HOUR)
			leave = leave + HALF_DAY;
		else if (leave < now) {
			printf("That time has already passed!\n");
			exit(1);
		}
	} else if (hour <= 24) {
		/* 24-hour format: relative to previous midnight.  */

		leave = now - (now - timezone) % DAY + hour * HOUR + min * MIN;

		if (leave < now - HOUR)
			leave = leave + DAY;
		else if (leave < now) {
			printf("That time has already passed!\n");
			exit(1);
		}
	} else
		Usage();
  }


  printf("Alarm set for %s", ctime(&leave));

  if ((pid = fork()) == -1) {
	fprintf(stderr, "leave: can not fork\n");
	exit(1);
  }
  if (pid != 0) exit(0);


  /* Only the child continues on  */

  {
	char *user = cuserid(NULL);
	char *tty = ttyname(0) + 5;
	long delta;
	int i;

	if (user == NULL || tty == NULL) {
		fprintf(stderr, "leave: Can not determine user and terminal name\n");
		exit(1);
	}
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);


	for (;;) {
		if (!Still_Logged_On(user, tty)) exit(0);

		/* How much longer until the leave time?  */

		delta = leave - time((long *)0);

		/* Which interval are we currently in?  */

		for (i = 0; i < INTERVALS; ++i)
			if (delta + intervals[i] > 0) break;

		/* If we are within intervals[0] then print a warning.  */
		/* If there are more intervals than messages, then use  */
		/* warnings[WARNINGS-1] for all subsequent messages.    */

		if (i > 0)
			printf("\007%s\n", warnings[i > WARNINGS ? WARNINGS - 1 : i - 1]);

		if (i == INTERVALS) {
			printf("That was the last time I'll tell you. Bye.\n");
			exit(0);
		}

		/* Sleep until the next interval. For long periods, wake	 */
		/* up every hour to check if the user is still on (also	 */
		/* required because 16 bit ints don't allow long
		 * waits).		 */

		sleep((int) Min(delta + intervals[i], HOUR));
	}
  }
}



Get_Hour_Min(when, hour, min)
char *when;
int *hour;
int *min;

{
  int hour_min;
  int just_min = 0;

  switch (sscanf(when, "%d:%d", &hour_min, &just_min)) {
      case 1:
	*hour = hour_min / 100;
	*min = hour_min % 100;
	break;

      case 2:
	*hour = hour_min;
	*min = just_min;
	break;

      default:	Usage();
  }


  if (hour_min < 0 || just_min < 0 || *min > 59) Usage();
}



Still_Logged_On(user, tty)
char *user;
char *tty;
{
  FILE *f;
  long size;			/* Number of wtmp records in the file	 */
  int wtmp_count;		/* How many to read into wtmp_buffer	 */


  if ((f = fopen(WTMP, "r")) == NULL) 	/* No login/logout records kept  */
	return(1);

  if (fseek(f, 0L, 2) != 0 || (size = ftell(f)) % sizeof(struct utmp) != 0) {
	fprintf(stderr, "leave: invalid wtmp file\n");
	exit(1);
  }
  size /= sizeof(struct utmp);	/* Number of records in wtmp	 */


  while (size > 0) {
	wtmp_count = (int) Min(size, MAX_WTMP_COUNT);

	size -= (long) wtmp_count;

	fseek(f, size * sizeof(struct utmp), 0);

	if (fread(&wtmp_buffer[0], sizeof(struct utmp), wtmp_count, f) != wtmp_count) {
		fprintf(stderr, "leave: read error on wtmp file\n");
		exit(1);
	}
	while (--wtmp_count >= 0) {
		if (strcmp(wtmp_buffer[wtmp_count].ut_line, "~") == 0)
			return(0);

		if (strncmp(wtmp_buffer[wtmp_count].ut_line, tty, 8) == 0)
			if (strncmp(wtmp_buffer[wtmp_count].ut_name, user, 8) == 0)
				return(1);
			else
				return(0);
	}

  }				/* end while( size > 0 ) */

  return(0);
}



Usage()
{
  fprintf(stderr, "Usage: leave [[+]hhmm]\n");
  exit(1);
}
