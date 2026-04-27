/* The <time.h> header is used by the procedures that deal with time.
 * Handling time is surprisingly complicated, what with GMT, local time
 * and other factors.  Although the Bishop of Ussher (1581-1656) once
 * calculated that based on the Bible, the world began on 12 Oct. 4004 BC
 * at 9 o'clock in the morning, in the UNIX world time begins at midnight, 
 * 1 Jan. 1970 GMT.  Before that, all was NULL and (void).
 */

#ifndef _TIME_H
#define _TIME_H

#define NULL	((void *) 0)
#define CLOCKS_PER_SEC    60	/* MINIX always uses 60 Hz, even in Europe */

#ifdef _POSIX_SOURCE
#define CLK_TCK CLOCKS_PER_SEC	/* obsolete name for CLOCKS_PER_SEC */
#endif

#ifndef _TIME_T
#define _TIME_T
typedef long time_t;		/* time in sec since 1 Jan 1970 0000 GMT */
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef	long clock_t;		/* time in ticks since process started */
#endif

#ifndef _SIZE_T
#define _SIZE_T			/* may not be allowed by ANSI */
typedef unsigned size_t;	/* but we need size_t */
#endif

struct tm {
  int tm_sec;			/* seconds after the minute [0, 59] */
  int tm_min;			/* minutes after the hour [0, 59] */
  int tm_hour;			/* hours since midnight [0, 28] */
  int tm_mday;			/* day of the month [1, 31] */
  int tm_mon;			/* months since January [0, 11] */
  int tm_year;			/* years since 1900 */
  int tm_wday;			/* days since Sunday [0, 6] */
  int tm_yday;			/* days since January 1 [0, 365] */
  int tm_isdst;			/* Daylight Saving Time flag */
};

/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( clock_t clock, (void)					   );
_PROTOTYPE( double difftime, (time_t __time1, time_t __time0)		   );
_PROTOTYPE( time_t mktime, (const struct tm *__timeptr)			   );
_PROTOTYPE( time_t time, (time_t *__timeptr)				   );
_PROTOTYPE( char *asctime, (const struct tm *__timeptr)		   	   );
_PROTOTYPE( char *ctime, (const time_t *__timer)			   );
_PROTOTYPE( struct tm *gmtime, (const time_t *__timer)			   );
_PROTOTYPE( struct tm *localtime, (const time_t *__timer)		   );
_PROTOTYPE( time_t time, (time_t *__tloc)				   );
_PROTOTYPE( size_t strftime, \
	(char *__s, size_t __max, char *__fmt, const struct tm *__timep)   );

#ifdef _POSIX_SOURCE
_PROTOTYPE( void tzset, (void)						   );
#endif

#endif /* _TIME_H */
