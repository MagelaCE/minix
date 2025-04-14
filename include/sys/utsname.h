/* The <sys/utsname.h> header gives the system name. */

#ifndef _UTSNAME_H
#define _UTSNAME_H

struct utsname {
  char *sysname;
  char *nodename;
  char *release;
  char *version;
  char *machine;
} utsname;


/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( int uname, (struct utsname *__name)				);

#endif /* _UTSNAME_H */
