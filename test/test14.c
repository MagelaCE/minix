/* Test 14. unlinking an open file. */

#define TRIALS 100

char name[20] = {"TMP14."};

main()
{
  int fd0, i, pid;

  printf("Test 14 ");
  pid = getpid();
  name[6] = (pid & 037) + 33;
  name[7] = ((pid * pid) & 037) + 33;
  name[8] = 0;


  for (i = 0; i < TRIALS; i++) {
	fd0 = creat(name, 0777);
	write(fd0, name, 20);
	unlink(name);
	close(fd0);
  }


  fd0 = creat(name, 0777);
  write(fd0, name, 20);
  unlink(name);
  printf("ok\n");
}
