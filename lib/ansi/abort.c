#include <lib.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void abort()
{
  kill(getpid(), SIGABRT);
}
