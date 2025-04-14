#include <lib.h>

#define	PTRSIZE	sizeof(char *)

PUBLIC int execn(name)
char *name;			/* pointer to file to be exec'd */
{
/* Special version used when there are no args and no environment.  This call
 * is principally used by INIT, to avoid having to allocate ARG_MAX.
 */

  PRIVATE char stack[3 * PTRSIZE];

  return(callm1(MM_PROC_NR, EXEC, len(name), sizeof(stack), 0, name, stack, NIL_PTR));
}
