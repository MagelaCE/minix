#include <time.h>

static int days_per_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
static char *months[] = { 
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static char *days[] = {
	"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

/* To compute which date falls on which day of the week, we need to know the
 * day of the week corresponding to Jan 1 of each year 1970 - 1999. The table
 * newyear[] gives this, with 0 = Mon, 1 = Tue, 2 = Wed, ..., 6 = Sun.  After
 * Jan 1, 2000 this program will screw up.  It probably won't be the only one.
 */
static int newyear[] = {
	3, 4, 5, 0, 1, 2, 3, 5, 6, 0, 	/* 1970 - 1979 */
	1, 3, 4, 5, 6, 1, 2, 3, 4, 6,	/* 1980 - 1989 */
	0, 1, 2, 4, 5, 6, 0, 2, 3, 4	/* 1990 - 1999 */
};

#define	MIN	60L		/* # seconds in a minute */
#define	HOUR	(60 * MIN)	/* # seconds in an hour */
#define	DAY	(24 * HOUR)	/* # seconds in a day */
#define	YEAR	(365 * DAY)	/* # seconds in a year */

static struct tm tm;
static char buf[26];

char *ctime(pt)
long *pt;
{
	register long t = *pt;

	tm.tm_year = 0;
	tm.tm_mon = 0;
	tm.tm_mday = 1;
	tm.tm_hour = 0;
	tm.tm_min = 0;
	tm.tm_sec = 0;

	/* t is elapsed time in seconds since Jan 1, 1970. */
	while (t >= YEAR) {	/* how many years can we subtract from t? */
		if ((tm.tm_year % 4) == 2)
			t -= DAY;
		tm.tm_year += 1;
		t -= YEAR;
	}
	tm.tm_year += 1970;

	/* t is now the offset into the current year, in seconds. */
	tm.tm_yday = (t/DAY) + 1;	/* day # of the year, Jan 1 = 1 */
	tm.tm_wday = ((newyear[tm.tm_year - 1970] + tm.tm_yday - 1) % 7) + 1;

	days_per_month[1] = 28;
	if ((tm.tm_year % 4) == 0)	/* check for leap year */
		days_per_month[1]++;

	/* Compute month. */
	while (t >= (days_per_month[tm.tm_mon] * DAY))
		t -= days_per_month[tm.tm_mon++] * DAY;
	tm.tm_mon++;			/* Jan is month 1, not month 0 */

	/* Month established, now compute day of the month */
	while (t >= DAY) {
		t -= DAY;
		tm.tm_mday++;
	}

	/* Day established, now do hour. */
	while (t >= HOUR) {
		t -= HOUR;
		tm.tm_hour++;
	}

	/* Hour established, now do minute. */
	while (t >= MIN) {
		t -= MIN;
		tm.tm_min++;
	}

	/* Residual time is # seconds. */
	tm.tm_sec = (int) t;

	/* Generate output in ASCII in buf. */
	sprintf(buf, "%s %s %2d %02d:%02d:%02d %d\n",
		days[tm.tm_wday - 1], months[tm.tm_mon-1],
		tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_year); 
	return buf;
}

struct tm *localtime(pt)
long *pt;
{
  ctime(pt);
  return &tm;
}

struct tm *gmtime(pt)
long *pt;
{
  ctime(pt);
  return &tm;
}
