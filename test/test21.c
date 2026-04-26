/* POSIX test program (20).			Author: Andy Tanenbaum */

/* The following POSIX calls are tested:
 *
 *	opendir()
 *	readdir()
 *	rewinddir()
 *	closedir()
 *	chdir()
 *	getcwd()
 */

#define _POSIX_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define DIR_NULL (DIR*) NULL
#define ITERATIONS        15
#define MAX_FD           100	/* must be large enough to cause error */
#define BUF_SIZE PATH_MAX+20
#define ERR_CODE          -1	/* error return */
#define RD_BUF           200

char str[] = {"The time has come the walrus said to talk of many things.\n"};
char str2[] = {"Of ships and shoes and sealing wax, of cabbages and kings.\n"};
char str3[] = {"Of why the sea is boiling hot and whether pigs have wings\n"};

int subtest, errct;
extern errno;

main()
{

  int i;

  sync();
  printf("Test 20 ");

  for (i = 0; i < ITERATIONS; i++) {

	test20h();		/* test access() */
	test20i();		/* test chmod() */
  }
  if (errct == 0)
	printf("ok\n");
  else
	printf(" %d errors\n", errct);
  exit(0);
}

test20h()
{
/* Test access. */

  int fd;

  subtest = 10;
  system("rm -rf A1");
  if ( (fd = creat("A1", 0777)) < 0) e(1);
  if (close(fd) != 0) e(2);
  if (access("A1", R_OK) != 0) e(3);
  if (access("A1", W_OK) != 0) e(4);
  if (access("A1", X_OK) != 0) e(5);
  if (access("A1", (R_OK|W_OK|X_OK)) != 0) e(6);
  
  if (chmod("A1", 0400) != 0) e(7);
  if (access("A1", R_OK) != 0) e(8);
  if (access("A1", W_OK) != -1) e(9);
  if (access("A1", X_OK) != -1) e(10);
  if (access("A1", (R_OK|W_OK|X_OK)) != -1) e(11);
  
  if (chmod("A1", 0077) != 0) e(12);
  if (access("A1", R_OK) != -1) e(13);
  if (access("A1", W_OK) != -1) e(14);
  if (access("A1", X_OK) != -1) e(15);
  if (access("A1", (R_OK|W_OK|X_OK)) != -1) e(16);
  if (errno != EACCES) e(17);

  if (access("", R_OK) != -1) e(18);
  if (errno != ENOENT) e(19);
  if (access("./A1/x", R_OK) != -1) e(20);
  if (errno != ENOTDIR) e(21);

  if (unlink("A1") != 0) e(22);
}

test20i()
{
/* Test chmod. */

  int fd, i;
  struct stat stbuf;

  subtest = 11;
  unlink("A1");
  if ( (fd = creat("A1", 0777)) < 0) e(1);

  for (i = 0; i < 511; i++) {
	if (chmod("A1", i) != 0) e(100+i);
	if (fstat(fd, &stbuf) != 0) e(200+i);
	if ( (stbuf.st_mode&(S_IRWXU|S_IRWXG|S_IRWXO)) != i) e(300+i);
  }
  if (close(fd) != 0) e(2);

  if (chmod("A1/x", 0777) != -1) e(3);
  if (errno != ENOTDIR) e(4);
  if (chmod("Axxx", 0777) != -1) e(5);
  if (errno != ENOENT) e(6);
  errno = 0;
  if (chmod ("", 0777) != -1) e(7);
  if (errno != ENOENT) e(8);

  if (unlink("A1") != 0) e(9);
}


e(n)
int n;
{
  if (errct == 0) printf("\n");
  printf("\tSubtest %d,  error %d,  errno=%d  ", subtest, n, errno);
  perror("");
  errct++;
}
