#include <lib.h>
#include <signal.h>

void (*vectab[_NSIG]) ();	/* array of funcs to catch signals */
