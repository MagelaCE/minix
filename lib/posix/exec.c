#include <lib.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>

extern char **environ;		/* environment pointer */

#define	PTRSIZE	sizeof(char *)

PUBLIC int execl(name, arg0)
char *name;
char *arg0;
{
  return(execve(name, &arg0, environ));
}

PUBLIC int execle(name, argv)
char *name, *argv;
{
  char **p;
  p = (char **) &argv;
  while (*p++)			/* null statement */
	;
  return(execve(name, &argv, (char **) *p));
}

PUBLIC int execv(name, argv)
char *name, *argv[];
{
  return(execve(name, argv, environ));
}


PUBLIC int execve(name, argv, envp)
char *name;			/* pointer to name of file to be executed */
char *argv[];			/* pointer to argument array */
char *envp[];			/* pointer to environment */
{
  char stack[ARG_MAX];
  char **argorg, **envorg, *hp, **ap, *p;
  int i, nargs, nenvps, stackbytes, offset;

  /* Count the argument pointers and environment pointers. */
  nargs = 0;
  nenvps = 0;
  argorg = argv;
  envorg = envp;
  while (*argorg++ != NIL_PTR) nargs++;
  while (*envorg++ != NIL_PTR) nenvps++;

  /* Prepare to set up the initial stack. */
  hp = &stack[(nargs + nenvps + 3) * PTRSIZE];
  if (hp + nargs + nenvps >= &stack[ARG_MAX]) {
	errno = E2BIG;
	return(-1);
  }
  ap = (char **) stack;
  *ap++ = (char *) nargs;

  /* Prepare the argument pointers and strings. */
  for (i = 0; i < nargs; i++) {
	offset = hp - stack;
	*ap++ = (char *) offset;
	p = *argv++;
	while (*p) {
		*hp++ = *p++;
		if (hp >= &stack[ARG_MAX]) {
			errno = E2BIG;
			return(-1);
		}
	}
	*hp++ = (char) 0;
  }
  *ap++ = NIL_PTR;

  /* Prepare the environment pointers and strings. */
  for (i = 0; i < nenvps; i++) {
	offset = hp - stack;
	*ap++ = (char *) offset;
	p = *envp++;
	while (*p) {
		*hp++ = *p++;
		if (hp >= &stack[ARG_MAX]) {
			errno = E2BIG;
			return(-1);
		}
	}
	*hp++ = (char) 0;
  }
  *ap++ = NIL_PTR;
  stackbytes = (((int) (hp - stack) + PTRSIZE - 1) / PTRSIZE) * PTRSIZE;
  return(callm1(MM_PROC_NR, EXEC, len(name), stackbytes, 0, name, stack, NIL_PTR));
}
