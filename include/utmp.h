/* The <utmp.h> header is used by login(1), init, and who(1)  */

#ifndef _UTMP_H
#define _UTMP_H

#define WTMP  "/usr/adm/wtmp"

struct  utmp
{
  char ut_line[8];		/* terminal name */
  char ut_name[8];		/* user name */
  long ut_time;			/* login/out time */
};

#endif /* _UTMP_H */
