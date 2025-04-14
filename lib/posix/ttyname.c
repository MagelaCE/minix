/*  ttyname(3)
 */

#include <lib.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define  DEV	"/dev/"

PRIVATE char file_name[40];

char *ttyname(tty_file_desc)
int tty_file_desc;
{
  int dev_dir_desc;
  struct direct dir_entry;
  struct stat tty_stat;
  struct stat dev_stat;

  /* Make sure the supplied file descriptor is for a terminal  */
  if (fstat(tty_file_desc, &tty_stat) < 0) return((char *)NULL);
  if ((tty_stat.st_mode & S_IFMT) != S_IFCHR) return((char *)NULL);

  /* Open /dev for reading  */
  if ((dev_dir_desc = open(DEV, O_RDONLY)) < 0) return((char *)NULL);
  while (read(dev_dir_desc, (char *) &dir_entry, sizeof(struct direct)) > 0) {
	/* Find an entry in /dev with the same inode number  */
	if (tty_stat.st_ino != dir_entry.d_ino) continue;

	/* Put the name of the device in PRIVATE storage  */
	strcpy(file_name, DEV);
	strncat(file_name, dir_entry.d_name, sizeof(dir_entry.d_name));

	/* Be absolutely certain the inodes match  */
	if (stat(file_name, &dev_stat) < 0) continue;
	if ((dev_stat.st_mode & S_IFMT) != S_IFCHR) continue;
	if (tty_stat.st_ino == dev_stat.st_ino &&
	    tty_stat.st_dev == dev_stat.st_dev &&
	    tty_stat.st_rdev == dev_stat.st_rdev) {
		close(dev_dir_desc);
		return(file_name);
	}
  }

  close(dev_dir_desc);
  return((char *)NULL);
}
