/* last - display log-in history	Author: Terrence W. Holm */


#include <utmp.h>
#include <stdio.h>
#include <signal.h>

#ifndef  WTMP
#define  WTMP   "/usr/adm/wtmp"
#endif

#define  FALSE	0
#define  TRUE	1

#define  BUFFER_SIZE     4096	/* Room for wtmp records	 */
#define  MAX_WTMP_COUNT  ( BUFFER_SIZE / sizeof(struct utmp) )

#define  min( a, b )     ( (a < b) ? a : b )
#define  max( a, b )	 ( (a > b) ? a : b )


extern long time();
extern char *malloc();
extern char *ctime();
int Sigint();
int Sigquit();


typedef struct logout {		/* A logout time record	 */
  char line[8];			/* The terminal name		 */
  long time;			/* The logout time		 */
  struct logout *next;		/* Next in linked list		 */
} logout;



/****************************************************************/
/*								*/
/*	last [-r] [-count] [-f file] [name] [tty] ...		*/
/*								*/
/*								*/
/*	Last(1) searches backwards through the file of log-in 	*/
/*	records (/usr/adm/wtmp), displaying the length of 	*/
/*	log-in sessions as requested by the options:		*/
/*								*/
/*								*/
/*	-r	Search backwards only until the last reboot	*/
/*		record.						*/
/*								*/
/*	-count	Only print out <count> records. Last(1) stops	*/
/*		when either -r or -count is satisfied, or at	*/
/*		the end of the file if neither is given.	*/
/*								*/
/*	-f file	Use "file" instead of "/usr/adm/wtmp".		*/
/*								*/
/*	name	Print records for the user "name".		*/
/*								*/
/*	tty	Print records for the terminal "tty". Actually,	*/
/*		a list of names may be given and all records	*/
/*		that match either the user or tty name are	*/
/*		printed. If no names are given then all records	*/
/*		are displayed.					*/
/*								*/
/*								*/
/*	A sigquit (^\) causes last(1) to display how far it	*/
/*	has gone back in the log-in record file, it then 	*/
/*	continues. This is used to check on the progress of	*/
/*	long running searches. A sigint will stop last(1).	*/
/*								*/
/****************************************************************/



/****************************************************************/
/*		Command-line option flags			*/
/****************************************************************/


char boot_limit = FALSE;	/* Stop on latest reboot	 */
char count_limit = FALSE;	/* Stop after print_count	 */
int print_count;
int arg_count;			/* Used to select specific	 */
char **args;			/* users and ttys		 */



/****************************************************************/
/*		Global variables				*/
/****************************************************************/


long boot_time = 0;		/* Zero means no reboot yet	 */
char *boot_down;		/* "crash" or "down " flag	 */
logout *first_link = NULL;	/* List of logout times	 */
int interrupt = FALSE;		/* If sigint or sigquit occurs	 */




/****************************************************************/
/*								*/
/*	main()							*/
/*								*/
/****************************************************************/


main(argc, argv)
int argc;
char *argv[];

{
  char *wtmp_file = WTMP;
  FILE *f;
  long size;			/* Number of wtmp records in the file	 */
  int wtmp_count;		/* How many to read into wtmp_buffer	 */
  struct utmp wtmp_buffer[MAX_WTMP_COUNT];

  --argc;
  ++argv;

  while (argc > 0 && *argv[0] == '-') {
	if (strcmp(argv[0], "-r") == 0) boot_limit = TRUE;

	else if (argc > 1 && strcmp(argv[0], "-f") == 0) {
		wtmp_file = argv[1];
		--argc;
		++argv;
	} else if ((print_count = atoi(argv[0] + 1)) > 0)
		count_limit = TRUE;

	else {
		fprintf(stderr,
		     "Usage: last [-r] [-count] [-f file] [name] [tty] ...\n");
		exit(1);
	}

	--argc;
	++argv;
  }


  arg_count = argc;
  args = argv;



  if ((f = fopen(wtmp_file, "r")) == NULL) {
	perror(wtmp_file);
	exit(1);
  }
  if (fseek(f, 0L, 2) != 0 || (size = ftell(f)) % sizeof(struct utmp) != 0) {
	fprintf(stderr, "last: invalid wtmp file\n");
	exit(1);
  }
  if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
	signal(SIGINT, Sigint);
	signal(SIGQUIT, Sigquit);
  }
  size /= sizeof(struct utmp);	/* Number of records in wtmp	 */

  if (size == 0) {
	long now = time((long *) 0);

	printf("\nwtmp begins %.16s \n", ctime(&now));
	exit(0);
  }
  while (size > 0) {
	wtmp_count = (int) min(size, MAX_WTMP_COUNT);
	size -= (long) wtmp_count;
	fseek(f, size * sizeof(struct utmp), 0);

	if (fread(&wtmp_buffer[0], sizeof(struct utmp), wtmp_count, f) !=
	    wtmp_count) {
		fprintf(stderr, "last: read error on wtmp file\n");
		exit(1);
	}
	while (--wtmp_count >= 0) {
		Process(&wtmp_buffer[wtmp_count]);
		if (interrupt) {
			printf("\ninterrupted %.16s \n",
			   ctime(&wtmp_buffer[wtmp_count].ut_time));

			if (interrupt == SIGINT) exit(2);
			interrupt = FALSE;
			signal(SIGQUIT, Sigquit);
		}
	}

  }				/* end while( size > 0 ) */

  printf("\nwtmp begins %.16s \n", ctime(&wtmp_buffer[0].ut_time));
  exit(0);
}




/****************************************************************/
/*								*/
/*	A log-in record format file contains four types of	*/
/*	records.						*/
/*								*/
/*	  [1] generated on a system reboot:			*/
/*								*/
/*	   line="~", name="reboot", host="", time=date()	*/
/*								*/
/*								*/
/*	  [2] generated after a shutdown:			*/
/*								*/
/*	   line="~", name="shutdown", host="", time=date()	*/
/*								*/
/*								*/
/*	  [3] generated on a successful login(1)		*/
/*								*/
/*	   line=ttyname(), name=cuserid(), host=, time=date()	*/
/*								*/
/*								*/
/*	  [4] generated by init(8) on a logout			*/
/*								*/
/*	   line=ttyname(), name="", host="", time=date()	*/
/*								*/
/*								*/
/*	Note: This version of last(1) does not recognize the	*/
/*	'|' and '}' time change records.			*/
/*								*/
/****************************************************************/
/*								*/
/*	Last(1) pairs up line login's and logout's to generate	*/
/*	four types of output lines:				*/
/*								*/
/*	  [1] a system reboot or shutdown			*/
/*								*/
/*	   reboot    ~       Mon May 16 14:16 			*/
/*	   shutdown  ~       Mon May 16 14:15 			*/
/*								*/
/*								*/
/*	  [2] a login with a matching logout			*/
/*								*/
/*	   edwin     tty1    Thu May 26 20:05 - 20:32  (00:27)	*/
/*								*/
/*								*/
/*	  [3] a login followed by a reboot or shutdown		*/
/*								*/
/*	   root      tty0    Mon May 16 13:57 - crash  (00:19)	*/
/*	   root      tty1    Mon May 16 13:45 - down   (00:30)	*/
/*								*/
/*								*/
/*	  [4] a login not followed by a logout or reboot	*/
/*								*/
/*	   terry     tty0    Thu May 26 21:19   still logged in	*/
/*								*/
/****************************************************************/




/****************************************************************/
/*								*/
/*	Process( wtmp )						*/
/*								*/
/*		Interpret one record from the log-in record	*/
/*		file.						*/
/*								*/
/****************************************************************/


Process(wtmp)
struct utmp *wtmp;

{
  logout *link;
  logout *next_link;


  /* Suppress the job number on an "ftp" line  */

  if (strncmp(wtmp->ut_line, "ftp", 3) == 0)
	strncpy(wtmp->ut_line, "ftp", 8);


  if (strcmp(wtmp->ut_line, "~") == 0) {
	/* A reboot or shutdown record  */

	if (boot_limit) exit(0);

	if (Print_Record(wtmp)) putchar('\n');

	boot_time = wtmp->ut_time;

	if (strcmp(wtmp->ut_name, "reboot") == 0)
		boot_down = "crash";
	else
		boot_down = "down ";


	/* Remove any logout records  */

	for (link = first_link; link != NULL; link = next_link) {
		next_link = link->next;
		free(link);
	}

	first_link = NULL;
  } else if (wtmp->ut_name[0] == '\0') {
	/* A logout record  */
	Record_Logout_Time(wtmp);
  } else {
	/* A login record  */
	for (link = first_link; link != NULL; link = link->next)
		if (strncmp(link->line, wtmp->ut_line, 8) == 0) {
			/* Found corresponding logout record  */
			if (Print_Record(wtmp)) {
				printf("- %.5s ", ctime(&link->time) + 11);

				Print_Duration(wtmp->ut_time, link->time);
			}

			/* Record login time  */
			link->time = wtmp->ut_time;
			return;
		}

	/* Could not find a logout record for this login tty  */

	if (Print_Record(wtmp))
		if (boot_time == 0)	/* Still on  */
			printf("  still logged in\n");

		else {		/* System crashed while on  */
			printf("- %s ", boot_down);

			Print_Duration(wtmp->ut_time, boot_time);
		}

	Record_Logout_Time(wtmp);	/* Needed in case of 2
					 * consecutive logins  */
  }
}




/****************************************************************/
/*								*/
/*	Print_Record( wtmp )					*/
/*								*/
/*		If the record was requested, then print out	*/
/*		the user name, terminal, host and time.		*/
/*								*/
/****************************************************************/


Print_Record(wtmp)
struct utmp *wtmp;

{
  int i;
  char print_flag = FALSE;

  /* Check if we have already printed the requested number of records  */
  if (count_limit && print_count == 0) exit(0);
  for (i = 0; i < arg_count; ++i)
	if (strcmp(args[i], wtmp->ut_name) == 0 ||
	    strcmp(args[i], wtmp->ut_line) == 0)
		print_flag = TRUE;

  if (arg_count == 0 || print_flag) {
#ifdef RLOGIN
	printf("%-8.8s  %-8.8s %-16.16s %.16s ",
	       wtmp->ut_name, wtmp->ut_line, wtmp->ut_host, ctime(&wtmp->ut_time));
#else
	printf("%-8.8s  %-8.8s  %.16s ",
	       wtmp->ut_name, wtmp->ut_line, ctime(&wtmp->ut_time));
#endif

	--print_count;
	return(TRUE);
  }
  return(FALSE);
}




/****************************************************************/
/*								*/
/*	Print_Duration( from, to )				*/
/*								*/
/*		Calculate and print the days and hh:mm between	*/
/*		the log-in and the log-out.			*/
/*								*/
/****************************************************************/


Print_Duration(from, to)
long from;
long to;

{
  long delta, days, hours, minutes;

  delta = max(to - from, 0);
  days = delta / (24L * 60L * 60L);
  delta = delta % (24L * 60L * 60L);
  hours = delta / (60L * 60L);
  delta = delta % (60L * 60L);
  minutes = delta / 60L;

  if (days > 0)
	printf("(%ld+", days);
  else
	printf(" (");

  printf("%02D:%02D)\n", hours, minutes);
}




/****************************************************************/
/*								*/
/*	Record_Logout_Time( wtmp )				*/
/*								*/
/*		A linked list of "last logout time" is kept.	*/
/*		Each element of the list is for one terminal.	*/
/*								*/
/****************************************************************/


Record_Logout_Time(wtmp)
struct utmp *wtmp;

{
  logout *link;

  /* See if the terminal is already in the list  */

  for (link = first_link; link != NULL; link = link->next)
	if (strncmp(link->line, wtmp->ut_line, 8) == 0) {
		link->time = wtmp->ut_time;
		return;
	}

  /* Allocate a new logout record, for a tty not previously encountered  */

  link = (logout *) malloc(sizeof(logout));

  if (link == (logout *) NULL) {
	fprintf(stderr, "last: malloc failure\n");
	exit(1);
  }
  strncpy(link->line, wtmp->ut_line, 8);

  link->time = wtmp->ut_time;
  link->next = first_link;

  first_link = link;
}




/****************************************************************/
/*								*/
/*	Sigint()						*/
/*	Sigquit()						*/
/*								*/
/*		Flag occurrence of an interrupt.		*/
/*								*/
/****************************************************************/


Sigint()
{
  interrupt = SIGINT;
}



Sigquit()
{
  interrupt = SIGQUIT;
}
