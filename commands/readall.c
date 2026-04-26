
/* readall - read a whole device fast		Author: Andy Tanenbaum */

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <blocksize.h>

#define CHUNK    25		/* max number of blocks read at once */
#define RESUME  200		/* # good reads before going back to CHUNK */
#define DIVISOR	 50		/* how often to print statistics */

int chunk = CHUNK;		/* current number of blocks being read */
long goodies;			/* incremented on good reads */
long errors;			/* number of errors so far */
char a[CHUNK * BLOCK_SIZE];

main(argc, argv)
int argc;
char *argv[];
{
  int fd, s;
  long b = 0;
  if (argc != 2) {
	printf("Usage: readall file\n");
	exit(1);
  }
  fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
	printf("%s is not readable\n", argv[1]);
	exit(1);
  }

  /* Read the entire file. Try it in large chunks, but if an error occurs,
   * go to single reads for a while.
   */
  while (1) {
	lseek(fd, BLOCK_SIZE * b, SEEK_SET);
	s = read(fd, a, BLOCK_SIZE * chunk);
	if (s == BLOCK_SIZE * chunk) {
		/* Normal read, no errors. */
		b += chunk;
		goodies++;
		if (chunk == 1) {
			if(goodies >= RESUME && b % DIVISOR == 0)chunk = CHUNK;
		}
	} else if (s < 0) {
		/* I/O error. */
		if (chunk != 1) {
			chunk = 1;	/* regress to single block mode */
			continue;
		}
		b += chunk;
		errors++;
	} else {
		/* End of file. */
		b += s/BLOCK_SIZE;
		output(b);
		printf("\n");
		exit(0);
	}
	if (b % DIVISOR == 0) output(b);
  }
}

output(b)
long b;
{
  printf("%8ld blocks read, %5ld errors\r", b, errors);
}
