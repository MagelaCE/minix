/* Test POSIX directory operations. */

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#define DIR_NULL (DIR *) NULL
#define ITERATIONS       15
#define MAX_FD          100	/* must be large enough to cause error */

int subtest, errct;
extern errno;

main()
{

  int i;

  sync();
  printf("Test 20 ");

  for (i = 0; i < ITERATIONS; i++) {
	test20a();		/* test for correct operation */
	test20b();		/* test general error handling */
	test20c();		/* test for EMFILE error */
  }
  if (errct == 0)
	printf("ok\n");
  else
	printf(" %d errors\n", errct);
  exit(0);
}

test20a()
{
/* Subtest 1. Correct operation */

  int f1, f2, f3, f4, f5;
  DIR *dirp;

  /* Remove any residue of previous tests. */
  subtest = 1;
  system("rm -rf foo");

  /* Create a directory foo with 5 files in it. */
  system("mkdir foo");
  if ( (f1 = creat("foo/f1", 0666)) < 0) e(1);
  if ( (f2 = creat("foo/f2", 0666)) < 0) e(2);
  if ( (f3 = creat("foo/f3", 0666)) < 0) e(3);
  if ( (f4 = creat("foo/f4", 0666)) < 0) e(4);
  if ( (f5 = creat("foo/f5", 0666)) < 0) e(5);

  /* Now remove 2 files to create holes in the directory. */
  if (unlink("foo/f2") < 0) e(6);
  if (unlink("foo/f4") < 0) e(7);

  /* Close the files. */
  close(f1);
  close(f2);
  close(f3);
  close(f4);
  close(f5);

  /* Open the directory. */
  dirp = opendir("./foo");
  if (dirp == DIR_NULL) e(6);

  /* Read the 5 files from it.
  checkdir(dirp, 2);

  /* Rewind dir and test again. */
  rewinddir(dirp);
  checkdir(dirp, 3);

  /* We're done.  Close the directory stream. */
  if (closedir(dirp) < 0) e(7);

  /* Remove dir for next time. */
  system("rm -rf foo");
}

checkdir(dirp, t)
DIR *dirp;			/* poinrter to directory stream */
int t;				/* subtest number to use */
{

  int i, f1, f2, f3, f4, f5, dot, dotdot, subt;
  struct dirent *d;
  char *s;

  /* Save subtest number */
  subt = subtest;
  subtest = t;

  /* Clear the counters. */
  f1 = 0;
  f2 = 0;
  f3 = 0;
  f4 = 0;
  f5 = 0;
  dot = 0;
  dotdot = 0;

  /* Read the directory.  It should contain 5 entries, ".", ".." and 3 files.*/
  for (i = 0; i < 5; i++) {
	d = readdir(dirp);
	if (d == (struct dirent *) NULL) {
		e(1);
		subtest = subt;	/* restore subtest number */
		return;
	}
	s = d->d_name;
	if (strcmp(s, "." ) == 0) dot++;
	if (strcmp(s, "..") == 0) dotdot++;
	if (strcmp(s, "f1") == 0) f1++;
	if (strcmp(s, "f2") == 0) f2++;
	if (strcmp(s, "f3") == 0) f3++;
	if (strcmp(s, "f4") == 0) f4++;
	if (strcmp(s, "f5") == 0) f5++;
  }

  /* Check results. */
  d = readdir(dirp);
  if (d != (struct dirent *) NULL) e(2);
  if (f1 != 1 || f3 != 1 || f5 != 1) e(3);
  if (f2 != 0 || f4 != 0) e(4);
  if (dot != 1 || dotdot != 1) e(5);
  subtest = subt;
  return;
}


test20b()
{
/* Subtest 4.  Test error handling. */

  int fd, fd2;
  DIR *dirp;

  subtest = 4;

  if (opendir("foo/xyz/---") != DIR_NULL) e(1);
  if (errno != ENOENT) e(2);
  if (system("mkdir foo") < 0) e(3);
  if (chmod("foo", 0) < 0) e(4);
  if (opendir("foo/xyz/--") != DIR_NULL) e(5);
  if (errno != EACCES) e(6);
  if (system("rmdir foo") < 0) e(7);
  if ( (fd = creat("abc", 0666)) < 0) e(8);
  if (close(fd) < 0) e(9);
  if (opendir("abc/xyz") != DIR_NULL) e(10);
  if (errno != ENOTDIR) e(11);
  if ( (dirp = opendir(".")) == DIR_NULL) e(12);
  if (closedir(dirp) != 0) e(13);
  if (closedir(dirp) >= 0) e(14);
  if (readdir(dirp) != (struct dirent *) NULL) e(15);
  if (errno != EBADF) e(16);
  if (readdir( (DIR *) -1) != (struct dirent *) NULL) e(17);
  if (errno != EBADF) e(18);
  
}


test20c()
{
/* Subtest 5.  See what happens if we open too many directory streams. */

  int i, j;
  DIR *dirp[MAX_FD];

  subtest = 5;

  for (i = 0; i < MAX_FD; i++) {
	dirp[i] = opendir(".");
	if (dirp[i] == NULL) {
		/* We have hit the limit. */
		if (errno != EMFILE) e(1);
		for (j = 0; j < i; j++) 
			if (closedir(dirp[j]) != 0) e(2);	/* close */
		return;
	}
  }

  /* Control should never come here.  This is an error. */
  e(3);
  for (i = 0; i < MAX_FD; i++) closedir(dirp[i]);	/* don't check */
}


e(n)
int n;
{
  printf("\n\tSubtest %d,  error %d,  errno=%d  ", subtest, n, errno);
  perror("");
  errct++;
}

