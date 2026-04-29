/* cron - clock daemon			Author: S.R. Sampson */

/* Cron- 	clock daemon
 *
 * Usage:	Cron is the clock daemon.
 *		It is typically started up from the /etc/rc file by the line:
 *			/usr/bin/cron
 *		Cron automatically puts itself in the background,
 *		so no & is needed. If cron is used, it runs all day,
 *		spending most of its time asleep. Once a minute it wakes
 *		up and examines /etc/crontab to see if there are any
 *		commands to be executed.  The format of this table is
 *		the same as in UNIX, except that % is not allowed to
 *		indicate 'new line.'
 *
 *		Each crontab entry has six fields:
 *	   	 minute hour day-of-the-month month day-of-the-week command
 *		Each entry is checked in turn, and any entry matching
 *		the current time is executed.  The entry * matches anything.
 *
 * Example:
 *	min hr dat mo day   command
 *	*   *   *  *   *    /usr/bin/date >/dev/tty0  #print date every minute
 *	0   *   *  *   *    /usr/bin/date >/dev/tty0  #print date on the hour
 *	30  4   *  *  1-5   /bin/backup /dev/fd1      #backup Mon-Fri at 0430
 *	30  19  *  *  1,3,5 /etc/backup /dev/fd1      #Mon, Wed, Fri at 1930
 *	0   9  25 12   *    /usr/bin/sing >/dev/tty0  #Xmas at 0900 only
 *
 * Authors:	S.R. Sampson
 * 		Simmule Turner        |Arpa: simmy@nu.cs.fsu.edu
 * 		Florida State Univ    |Uucp: gatech!nu.cs.fsu.edu!simmy
 * 		444 OSB               | Cis: 70651,67       Genie:simmy
 * 		Tallahassee, FL 32306 | Tel: +1 904 644 1573
 *
 * History:
 *		1.5d  Ralf Wenk last update:	Tue Apr  3 22:50:48 1990
 *			fixed the stdin/out/err problem
 *
 *		1.5c  Blayne Puklich (puklich@plains.nodak.edu)  89/12/07
 *			Changed so parent exits, like in 1.3 version.
 *
 *		1.5b  FvK  89/09/28
 *			Edited for MINIX Style Sheet
 *
 *		1.5a  Ralf Wenk	last update:	Sat Aug 26 15:40:59 1989
 *			Little fixes.
 *
 * 		1.5  SrT  89/05/08
 *      		Changed sleep code.
 *
 * 		1.4  SrT  89/03/17
 *			Fixed a pointer problem, when reloading crontab.
 *
 * 		1.3  SrT  89/03/16
 *			Loads crontab, into memory and only rereads the disk
 *			version if it changes.  (Free up those CPU cycles!)
 *
 * 		Fixed 03/10/89, by Simmule Turner, simmy@nu.cs.fsu.edu
 *			Now correctly cleans up zombie processes
 *			Logs actions to /usr/adm/cronlog
 *			Syncs with clock after each minute
 *			Comments allowed in crontab
 *			Fixed bug that prevented month, from matching
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <setjmp.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>


#define CRONTAB "/etc/crontab"
#define LOGFILE "/usr/adm/cronlog"

#define NULLDEV "/dev/null"
#define SEPARATOR " \t"
#define CRONSIZE  2048
#define CRONSTRUCT struct cron_entry

#define	TRUE	1
#define	FALSE	0

struct cron_entry {
  char *mn;
  char *hr;
  char *day;
  char *mon;
  char *wkd;
  char *cmd;
  struct cron_entry *next;
} *head, *p;


char crontab[CRONSIZE];
FILE *cronlog;

void wakeup(), nothing();

long previous_time = 0L;
extern int errno;
extern char *malloc();


main()
{
  int pid;
  time_t clock;

  pid = fork();
  if (pid == -1) {
	fprintf(stderr, "Can't fork cron\n");
	exit(1);
  } else if (pid > 0)
	exit(0);	/* parent exits */
  signal(SIGINT, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);

  /* Release stdin. Stdout and stderr are redirected to LOGFILE or NULLDEV.
   * So the output of crontab commands could be found in LOGFILE if the file
   * is accessible.
   */
  fclose(stdin);

  /* Release current directory to avoid locking current device. */
  chdir("/");

  open(NULLDEV, O_RDONLY);
  if ((cronlog = freopen(LOGFILE,"a", stdout)) == (FILE *) NULL) {
	cronlog = freopen(NULLDEV,"w", stdout);
	freopen(NULLDEV,"w", stderr);
  } else {
	freopen(LOGFILE,"a", stderr);
	setbuf(cronlog, (char *) NULL);
  }

  p = (CRONSTRUCT *) malloc(sizeof(CRONSTRUCT));
  p->next = (CRONSTRUCT *) NULL;
  head = p;

  while (TRUE) {
	signal(SIGALRM, wakeup);
	time(&clock);
	alarm((unsigned) (60 - clock % 60));
	pause();

	signal(SIGALRM, nothing);
	alarm(1);
	while (wait((int *) NULL) != -1);
  }
}


void nothing()
{
}


void wakeup()
{
  register struct tm *tm;
  time_t cur_time;
  extern struct tm *localtime();
  CRONSTRUCT *this_entry = head;

  load_crontab();

  time(&cur_time);
  tm = localtime(&cur_time);

  while (this_entry->next && this_entry->mn) {
	if (match(this_entry->mn, tm->tm_min) &&
	    match(this_entry->hr, tm->tm_hour) &&
	    match(this_entry->day, tm->tm_mday) &&
	    match(this_entry->mon, tm->tm_mon + 1) &&
	    match(this_entry->wkd, tm->tm_wday)) {
		fprintf(cronlog, "%02d/%02d-%02d:%02d  %s\n",
			tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
			tm->tm_min, this_entry->cmd);

		if (fork() == 0) {
			execl("/bin/sh", "/bin/sh", "-c", this_entry->cmd,
			      (char *) 0);
			exit(1);
		}
	}
	this_entry = this_entry->next;
  }
}


/*
 *	This routine will match the left string with the right number.
 *
 *	The string can contain the following syntax:
 *
 *	*		This will return TRUE for any number
 *	x,y [,z, ...]	This will return TRUE for any number given.
 *	x-y		This will return TRUE for any number within
 *			the range of x thru y.
 */

match(left, right)
register char *left;
register int right;
{
  register int n;
  register char c;

  n = 0;
  if (!strcmp(left, "*")) return(TRUE);

  while ((c = *left++) && (c >= '0') && (c <= '9')) n = (n * 10) + c - '0';

  switch (c) {
      case '\0':
	return(right == n);
      case ',':
	if (right == n) return(TRUE);
	do {
		n = 0;
		while ((c = *left++) && (c >= '0') && (c <= '9'))
			n = (n * 10) + c - '0';

		if (right == n) return(TRUE);
	} while (c == ',');
	return(FALSE);
      case '-':
	if (right < n) return(FALSE);
	n = 0;
	while ((c = *left++) && (c >= '0') && (c <= '9'))
		n = (n * 10) + c - '0';
	return(right <= n);
  }
}


load_crontab()
{
  int pos = 0;
  FILE *cfp;
  struct stat buf;

  if (stat(CRONTAB, &buf)) {
	if (previous_time == 0L) fprintf(cronlog, "Can't stat crontab\n");
	previous_time = 0L;
	return;
  }
  if (buf.st_mtime <= previous_time) return;

  if ((cfp = fopen(CRONTAB, "r")) == (FILE *) NULL) {
	if (previous_time == 0L) fprintf(cronlog, "Can't open crontab\n");
	previous_time = 0L;
	return;
  }
  previous_time = buf.st_mtime;

  p = head;
  while (fgets(&crontab[pos], CRONSIZE - pos, cfp) != (char *) NULL) {
	int len;

	if (crontab[pos] == '#') continue;
	len = strlen(&crontab[pos]);
	if (crontab[pos + len - 1] == '\n') {
		len--;
		crontab[pos + len] = '\0';
	}
	assign(p, &crontab[pos]);
	if (p->next == (CRONSTRUCT *) NULL) {
		p->next = (CRONSTRUCT *) malloc(sizeof(CRONSTRUCT));
		p->next->next = (CRONSTRUCT *) NULL;
	}
	p = p->next;
	pos += ++len;
	if (pos >= CRONSIZE) break;
  }
  fclose(cfp);

  while (p) {
	p->mn = (char *) NULL;
	p = p->next;
  }
}


assign(entry, line)
CRONSTRUCT *entry;
char *line;
{
  static char buf[256];
  int where;

  strncpy(buf, line, sizeof buf - 1);

  entry->mn = strtok(line, SEPARATOR);
  entry->hr = strtok((char *) NULL, SEPARATOR);
  entry->day = strtok((char *) NULL, SEPARATOR);
  entry->mon = strtok((char *) NULL, SEPARATOR);
  entry->wkd = strtok((char *) NULL, SEPARATOR);
  entry->cmd = strtok((char *) NULL, SEPARATOR);

  where = entry->cmd - line;
  strcpy(&line[where], &buf[where]);
}
