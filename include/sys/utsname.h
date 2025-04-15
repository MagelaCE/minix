/* The <sys/utsname.h> header gives the system name. */

#ifndef _UTSNAME_H
#define _UTSNAME_H

#define _UTSLEN  9
#define _SNLEN  15

struct utsname {
  char sysname[_UTSLEN];
  char nodename[_UTSLEN];
  char release[_UTSLEN];
  char version[_UTSLEN];
  char machine[_UTSLEN];
  char idnumber[_SNLEN];
};

extern struct utsname utsname;

/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( int uname, (struct utsname *__name)				);

#endif /* _UTSNAME_H */
