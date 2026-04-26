/* df - disk free block printout	Author: Andy Tanenbaum */

#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <minix/config.h>
#include <minix/const.h>
#include <minix/type.h>
#include "../fs/const.h"
#include "../fs/type.h"
#include "../fs/super.h"

#define block_nr long		/* Allow big devices even if FS doesn't */

extern int errno;
long bit_count();
char *mtab = "/etc/mtab";

#define REPORT 0
#define SILENT 1


main(argc, argv)
int argc;
char *argv[];
{
  register int i;

  sync();			/* have to make sure disk is up-to-date */
  fprintf(stdout, "\nDevice     Inodes  Inodes  Inodes       Blocks  Blocks  Blocks    ");
  if (argc == 1)
	fprintf(stdout, "Mounted on\n");
  else
	fprintf(stdout, "\n");

  fprintf(stdout, "           total   used    free         total   used    free\n");
  fprintf(stdout, "           -----   -----   -----        -----   -----   -----\n");

  if (argc == 1) defaults();

  for (i = 1; i < argc; i++) df(argv[i], "", SILENT);
  exit(0);
}


#define percent(num, tot)  ((int) ((100L * (num) + ((tot) - 1)) / (tot)))

df(name, mnton, silent)
char *name, *mnton;
int silent;
{
  register int fd;
  ino_t i_count;
  long z_count;
  block_nr totblocks, busyblocks;
  int i, j;
  char buf[BLOCK_SIZE], *s0;
  struct super_block super, *sp;

  if ((fd = open(name, O_RDONLY)) < 0) {
	if (!silent) {
		int e = errno;

		fprintf(stderr, "df: ");
		errno = e;
		perror(name);
	}
	return;
  }
  lseek(fd, (long) BLOCK_SIZE, SEEK_SET);	/* skip boot block */
  if (read(fd, &super, SUPER_SIZE) != SUPER_SIZE) {
	fprintf(stderr, "df: Can't read super block of %s\n", name);
	close(fd);
	return;
  }
  lseek(fd, (long) BLOCK_SIZE * 2L, SEEK_SET);	/* skip rest of super block */
  sp = &super;
  if (sp->s_magic != SUPER_MAGIC) {
	fprintf(stderr, "df: %s: Not a valid file system\n", name);
	close(fd);
	return;
  }
  i_count = (ino_t) bit_count(sp->s_imap_blocks, sp->s_ninodes + 1, fd);
  if (i_count == -1) {
	fprintf(stderr, "df: Can't find bit maps of %s\n", name);
	close(fd);
	return;
  }
  i_count--;			/* There is no inode 0. */

  z_count = bit_count(sp->s_zmap_blocks, sp->s_nzones, fd);
  if (z_count == -1) {
	fprintf(stderr, "df: Can't find bit maps of %s\n", name);
	close(fd);
	return;
  }
  totblocks = (block_nr) sp->s_nzones << sp->s_log_zone_size;
  busyblocks = (block_nr) z_count << sp->s_log_zone_size;

  /* Print results. */
  fprintf(stdout, "%s", name);
  j = 10 - strlen(name);
  while (j > 0) {
	fprintf(stdout, " ");
	j--;
  }

  fprintf(stdout, " %5u   %5u   %5u      %7ld %7ld %7ld     %s\n",
	sp->s_ninodes,		/* total inodes */
	i_count,		/* inodes used */
	sp->s_ninodes - i_count,/* inodes free */

	totblocks,		/* total blocks */
	busyblocks,		/* blocks used */
	totblocks - busyblocks,	/* blocsk free */

	strcmp(mnton, "device") == 0 ? "(root dev)" : mnton

	);
  close(fd);
}

long bit_count(blocks, bits, fd)
int blocks;
int bits;
int fd;
{
  register int i, b;
  long busy, count, w;
  int *wptr, *wlim;
  int buf[BLOCK_SIZE / sizeof(int)];

  /* Loop on blocks, reading one at a time and counting bits. */
  busy = 0;
  count = 0;
  for (i = 0; i < blocks; i++) {
	if (read(fd, buf, BLOCK_SIZE) != BLOCK_SIZE) return(-1);

	wptr = &buf[0];
	wlim = &buf[BLOCK_SIZE / sizeof(int)];

	/* Loop on the words of a block */
	while (wptr != wlim) {
		w = *wptr++;

		/* Loop on the bits of a word. */
		for (b = 0; b < 8 * sizeof(int); b++) {
			if ((w >> b) & 1) busy++;
			if (++count == bits) return (busy);
		}
	}
  }
  return(0);
}


char mtabbuf[1024];
int mtabcnt;

getname(d, m)
char **d, **m;
{
  int c;
  static char *mp = mtabbuf;

  *d = mp;
  *m = "";

  do {
	if (--mtabcnt < 0) exit(0);
	c = *mp++;
	if (c == ' ') {
		mp[-1] = 0;
		*m = mp;
	}
  } while (c != '\n');
  mp[-1] = 0;
}

    defaults()
{
/* Use the root file system and all mounted file systems. */

  char *dev, *dir;

  close(0);
  if (open(mtab, O_RDONLY) < 0 || (mtabcnt = read(0, mtabbuf, sizeof(mtabbuf))) <= 0) {
	fprintf(stderr, "df: cannot read %s\n", mtab);
	exit(1);
  }

  /* Read /etc/mtab and iterate on the lines. */
  while (1) {
	getname(&dev, &dir);	/* getname exits upon hitting EOF */
	df(dev, dir, REPORT);
  }

}


static int fd, w, left;
static char fill;

static void wr(a, n)
char *a;
int n;
{
  if (left && n > 0) write(fd, a, n);

  while (w > n) {
	write(fd, &fill, 1);
	--w;
  }

  if (!left && n > 0) write(fd, a, n);
}

static void pr_n(neg, n)
int neg;
unsigned long n;
{
  char a[3 * sizeof(n) + 1], *pa = a + sizeof(a);

  do
	*--pa = '0' + n % 10;
  while ((n /= 10) != 0);

  if (neg) *--pa = '-';

  wr(pa, a + sizeof(a) - pa);
}

static void pr_i(i)
long i;
{
  int neg = 0;

  if (i < 0) {
	neg = 1;
	i = -i;
  }
  pr_n(neg, (unsigned long) i);
}
