/* Peek and poke using /dev/mem. Callers now ought to check the return values.
 * Calling either of these functions consumes a file descriptor.
 */

#include <lib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define SEGSIZE 0x10

PRIVATE int memfd = -1;

int peek(segment, offset)
unsigned segment;
unsigned offset;
{
  unsigned char chvalue;

  if (memfd < 0) memfd = open("/dev/mem", O_RDONLY);
  if (memfd < 0 ||
    lseek(memfd, (unsigned long) segment * SEGSIZE + offset, 0) < 0 ||
      read(memfd, (char *) &chvalue, (size_t) 1) != 1)
	return(-1);
  return(chvalue);
}

int poke(segment, offset, value)
unsigned segment;
unsigned offset;
unsigned value;
{
  unsigned char chvalue;

  chvalue = value;
  if (memfd < 0) memfd = open("/dev/mem", O_WRONLY);
  if (memfd < 0 ||
    lseek(memfd, (unsigned long) segment * SEGSIZE + offset, 0) < 0 ||
      write(memfd, (char *) &chvalue, (size_t) 1) != 1)
	return(-1);
  return(chvalue);
}
