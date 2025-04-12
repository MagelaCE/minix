/* printroot - print root device on stdout	Author: Bruce Evans */

/* This program figures out what the root device is by doing a stat on it, and
 * then searching /dev until it finds an entry with the same device number.
 * A typical use (probably the only use) is in /etc/rc for initializing 
 * /etc/mtab, as follows:
 *
 *	/usr/bin/printroot >/etc/mtab
 *
 * 1 October 1989 - Minor changes by Andy Tanenbaum
 */

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>

#include <limits.h>
#include <string.h>

#define DEV_PATH	"/dev/"
#define MTAB		"/etc/mtab"
#define UNKNOWN_DEV	"/dev/unknown"
#define ROOT		"/"

char message[] = " is root device\n";

static void done();

int main(argc, argv)
int argc;
char **argv;
{
  struct direct dir;
  int fd;
  struct stat filestat, rootstat;
  static char namebuf[sizeof DEV_PATH + NAME_MAX] = DEV_PATH;

  if (stat(ROOT, &rootstat) == 0 && (fd = open(DEV_PATH, 0)) >= 0) {
	while (read(fd, &dir, sizeof dir) == sizeof dir) {
		if (dir.d_ino == 0) continue;
		strcpy(namebuf, DEV_PATH);
		strncat(namebuf + sizeof DEV_PATH - 1, dir.d_name, NAME_MAX);

		/* Last null in static buffer guarantees null termination. */
		if (stat(namebuf, &filestat) != 0) continue;
		if ((filestat.st_mode & S_IFMT) != S_IFBLK) continue;
		if (filestat.st_rdev != rootstat.st_dev) continue;
		done(namebuf, 0);
	}
  }
  done(UNKNOWN_DEV, 1);
}

void done(name, status)
char *name;
int status;
{
  write(1, name, strlen(name));
  write(1, message, sizeof message - 1);
  exit(status);
}
