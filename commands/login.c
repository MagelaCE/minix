/* login - log into the system		Author: Patrick van Kleef */

#include <signal.h>
#include <sgtty.h>
#include <pwd.h>
#include <sys/stat.h>

#define NULL (char *) 0
#define WTMPSIZE 8
#define DIGIT 3

char user[32];
char home[64];
char ttyname[] = {"tty?"};
char *wtmpfile = "/usr/adm/wtmp";
char *env[] = {user, home, "TERM=minix", (char *)0 };

main() 
{
	char buf[30], buf1[30], *crypt(), *p, *q;
	int n, n1, bad, oldflags, ttynr;
	struct sgttyb args;
	struct stat statbuf;
	struct passwd *pwd, *getpwnam();

	/* Look up /dev/tty number. */
	fstat(0, &statbuf);
	ttynr = statbuf.st_rdev & 0377;
	ttyname[DIGIT] = '0' + ttynr;

	/* Reset some of the line parameters in case they have been mashed. */
	ioctl(0, TIOCGETP, &args);
	args.sg_kill = '@';
	args.sg_erase = '\b';
	oldflags = args.sg_flags & 01700;	/* save char len, parity */
	args.sg_flags = oldflags | XTABS | CRMOD | ECHO;
	ioctl(0, TIOCSETP, &args);

	/* Get login name and passwd. */
	for (;;) {
		bad = 0;
		do {
			write(1,"login: ",7);
			n = read (0, buf, 30);
		} while (n < 2);
		buf[n - 1] = 0;

		/* Look up login/passwd. */
		if ((pwd = getpwnam (buf)) == 0) bad++;

		/* If login name wrong or password exists, ask for pw. */
		if (bad || strlen (pwd->pw_passwd) != 0) {
			args.sg_flags &= ~ECHO;
			ioctl(0, TIOCSETP, &args);
			write(1,"Password: ",10);
			n1 = read (0, buf1, 30);
			buf1[n1 - 1] = 0;
			write(1,"\n",1);
			args.sg_flags |= ECHO;
			ioctl(0, TIOCSETP, &args);
			if (bad || strcmp (pwd->pw_passwd, 
						crypt(buf1, pwd->pw_passwd))) {
				write (1,"Login incorrect\n",16);
				continue;
			}
		}

		/* Successful login.   Set up environment. */
		setgid (pwd->pw_gid);
		setuid (pwd->pw_uid);
		chdir (pwd->pw_dir);
		strcpy(home, "HOME=");
		strcat(home, pwd->pw_dir);
		strcpy(user, "USER=");
		strcat(user, pwd->pw_name);

		/* Reset signals to default values. */
		for(n = 1; n <= NR_SIGS; ++n) signal(n, SIG_DFL);

		/* Creat wmtp entry. */
		wtmp(ttyname, pwd->pw_name);

		/* If shell has been specified, exec it, else use /bin/sh. */
		if (pwd->pw_shell) 
			execle(pwd->pw_shell, "-", NULL, env);
		execle("/bin/sh", "-", NULL, env);
		write(1,"login can't exec shell\n",23);
	}
}


wtmp(tty, name)
{
/* Make an entry in /usr/adm/wtmp. */

  int i, fd;
  long t, time();
  char ttybuff[WTMPSIZE], namebuff[WTMPSIZE];

  fd = open(wtmpfile, 2);
  if (fd < 0) return;		/* if wtmp does not exist, no accounting */
  lseek(fd, 0L, 2);		/* append to file */

  for (i = 0; i < WTMPSIZE; i++) {
	ttybuff[i] = 0;
	namebuff[i] = 0;
  }
  strncpy(ttybuff, tty, 8);
  strncpy(namebuff, name, 8);
  time(&t);
  write(fd, ttybuff, WTMPSIZE);
  write(fd, namebuff, WTMPSIZE);
  write(fd, &t, sizeof(t));
  close(fd);
}
