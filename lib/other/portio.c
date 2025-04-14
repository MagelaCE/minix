/* Port i/o functions using /dev/port.
 * Callers now ought to check the return values.
 * Calling either of these functions consumes a file descriptor.
 */

#include <lib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

PRIVATE int portfd = -1;

int port_in(port, valuep)
unsigned port;
unsigned *valuep;
{
  unsigned char chvalue;

  if (portfd < 0) portfd = open("/dev/port", O_RDONLY);
  if (portfd < 0 ||
      lseek(portfd, (long) port, 0) < 0 ||
      read(portfd, (char *) &chvalue, (size_t) 1) != 1)
	return(*valuep = -1);
  return(*valuep = chvalue);
}

int port_out(port, value)
unsigned port;
unsigned value;
{
  unsigned char chvalue;

  chvalue = value;
  if (portfd < 0) portfd = open("/dev/port", O_WRONLY);
  if (portfd < 0 || lseek(portfd, (long) port, 0) < 0 ||
      write(portfd, (char *) &chvalue, (size_t) 1) != 1)
	return(-1);
  return(chvalue);
}
