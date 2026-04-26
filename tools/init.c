/* This process is the father (mother) of all Minix user processes.  When
 * Minix comes up, this is process number 2, and has a pid of 1.  It
 * executes the /etc/rc shell file, and then reads the /etc/ttys file to
 * determine which terminals need a login process.  The ttys file consists
 * of three-field lines as follows:
 *	abc
 * where
 *	a = 0 (line disabled = no shell), 1 (enabled = shell started)
 *	b = a-r defines UART paramers (baud, bits, parity), 0 for console
 *	c = line number or line name
 *
 * The letters a-r correspond to the 18 entries of the uart table below.
 * For example, 'a' is 110 baud, 8 bits, no parity; 'b' is 300 baud, 8
 * bits, no parity; 'j' is 2400 baud, 7 bits, even parity; etc.  If the
 * third field is a digit, then the terminal device will be /dev/tty{c},
 * otherwise it will be /dev/{c}.  Note that since login cheats in
 * determining the slot number, entries in /etc/ttys must always be in
 * minor device number order - the first line should be for tty0, the
 * second for tty1, and so on.
 *
 * If the files /usr/adm/wtmp and /etc/utmp exist and are writable, init
 * (with help from login) will maintain login accounting.  Sending a
 * signal 1 (SIGHUP) to init will cause it to reread /etc/ttys and start
 * up new shell processes if necessary.  It will not, however, kill off
 * login processes for lines that have been turned off; do this manually.
 */

#include <signal.h>
#include <sgtty.h>
#include <utmp.h>

#ifndef WTMP
#define WTMP		"/usr/adm/wtmp"	/* wtmp login accounting file */
#endif
#ifndef UTMP
#define UTMP		"/etc/utmp"	/* utmp login file */
#endif

#define CONSNAME	"/dev/tty0"	/* system console device */

#define PIDSLOTS	4		/* maximum number of ttys entries */
#define TTYSBUF		(8 * PIDSLOTS)	/* buffer for reading /etc/ttys */
#define STACKSIZE	(192 * sizeof(char *))	/* init's stack */

#define EXIT_TTYFAIL	253		/* child had problems with tty */
#define EXIT_EXECFAIL	254		/* child couldn't exec something */
#define EXIT_OPENFAIL	255		/* child couldn't open something */

struct uart {
  int baud;
  int flags;
} uart[] = {
  B110,   BITS8,		/* 'a':  110 baud, 8 bits, no parity */
  B300,   BITS8,		/* 'b':  300 baud, 8 bits, no parity */
  B1200,  BITS8,		/* 'c': 1200 baud, 8 bits, no parity */
  B2400,  BITS8,		/* 'd': 2400 baud, 8 bits, no parity */
  B4800,  BITS8,		/* 'e': 4800 baud, 8 bits, no parity */
  B9600,  BITS8,		/* 'f': 9600 baud, 8 bits, no parity */

  B110,   BITS7 | EVENP,	/* 'g':  110 baud, 7 bits, even parity */
  B300,   BITS7 | EVENP,	/* 'h':  300 baud, 7 bits, even parity */
  B1200,  BITS7 | EVENP,	/* 'i': 1200 baud, 7 bits, even parity */
  B2400,  BITS7 | EVENP,	/* 'j': 2400 baud, 7 bits, even parity */
  B4800,  BITS7 | EVENP,	/* 'k': 4800 baud, 7 bits, even parity */
  B9600,  BITS7 | EVENP,	/* 'l': 9600 baud, 7 bits, even parity */

  B110,   BITS7 | ODDP,		/* 'm':  110 baud, 7 bits, odd parity */
  B300,   BITS7 | ODDP,		/* 'n':  300 baud, 7 bits, odd parity */
  B1200,  BITS7 | ODDP,		/* 'o': 1200 baud, 7 bits, odd parity */
  B2400,  BITS7 | ODDP,		/* 'p': 2400 baud, 7 bits, odd parity */
  B4800,  BITS7 | ODDP,		/* 'q': 4800 baud, 7 bits, odd parity */
  B9600,  BITS7 | ODDP		/* 'r': 9600 baud, 7 bits, odd parity */
};

#define NPARAMSETS (sizeof uart / sizeof(struct uart))

struct slotent {
  int onflag;			/* should this ttyslot be on? */
  int pid;			/* pid of login process for this tty line */
  char name[8];			/* name of this tty */
  int flags;			/* sg_flags field for this tty */
  int speed;			/* sg_ispeed for this tty */
};

struct slotent slots[PIDSLOTS];	/* init table of ttys and pids */

char stack[STACKSIZE];		/* init's stack */
char *stackpt = &stack[STACKSIZE];
char **environ;			/* declaration required by library routines */
extern int errno;

char CONSOLE[] = CONSNAME;	/* name of system console */
struct sgttyb args;		/* buffer for TIOCGETP */
int gothup = 0;			/* flag, showing signal 1 was recieved */
int pidct = 0;			/* count of running children */

main()
{
  int pid;			/* pid of child process */
  int fd;			/* fd of console for error messages */
  int i;			/* loop variable */
  int status;			/* return status from child process */
  struct slotent *slotp;	/* slots[] pointer */
  int onhup();			/* SIGHUP interrupt catch routine */

  sync();			/* force buffers out onto disk */

  /* Execute the /etc/rc file. */
  if(fork()) {
	/* Parent just waits. */
	wait(&status);
  } else {
	/* Child exec's the shell to do the work. */
	if(open("/etc/rc", 0) < 0) exit(EXIT_OPENFAIL);
	dup(open(CONSOLE, 1));	/* std output, error */
	execn("/bin/sh");
	exit(EXIT_EXECFAIL);	/* impossible, we hope */
  }

  wtmp("~", "reboot", -1);	/* log system reboot */

  /* Read the /etc/ttys file */
  readttys();
  
  /* Main loop. If login processes have already been started up, wait for one
   * to terminate, or for a HUP signal to arrive. Start up new login processes
   * for all ttys which don't have them. Note that wait() also returns when
   * somebody's orphan dies, in which case ignore it.
   * First set up the signals.
   */

  for (i = 1; i <= _NSIG; i++) signal(i, SIG_IGN);
  signal(SIGHUP, onhup);

  while(1) {
	sync();

	if( pidct && (pid = wait(&status)) > 0 ) {
		/* Search to see which line terminated. */
		for(slotp = slots; slotp < &slots[PIDSLOTS]; ++slotp) {
			if(slotp->pid == pid) {
			    --pidct;
			    slotp->pid = 0;	/* now no login process */
			    wtmp(slotp->name, "", slotp - slots);

			    if(((status >> 8) & 0xFF) == EXIT_TTYFAIL) {
				fd = open(CONSOLE, 1);
				write(fd, "init: problems with tty, shutting down ", 39);
				write(fd, slotp->name, sizeof slotp->name);
				write(fd, "\n", 1);

				close(fd);
				slotp->onflag = 0;
			    }
			    break;
			}
	      }
	}

	/* If a signal 1 (SIGHUP) is recieved, reread /etc/ttys */
	if(gothup) {
		readttys();
		gothup = 0;
	}

	/* See which lines need a login process started up */
	for(slotp = slots; slotp < &slots[PIDSLOTS]; ++slotp) {
		if(slotp->onflag && slotp->pid <= 0) startup(slotp - slots);
	}
  }
}

onhup()
{
  gothup = 1;
  signal(SIGHUP, onhup);
}

readttys()
{
  /* (Re)read /etc/ttys. */

  char ttys[TTYSBUF];			/* buffer for reading /etc/ttys */
  register char *p;			/* current pos. within ttys */
  char *endp;				/* pointer to end of ttys buffer */
  int fd;				/* file descriptor for /etc/ttys */
  struct slotent *slotp = slots;	/* entry in slots[] */
  char *q;				/* pointer for copying ttyname */

  if((fd = open("/etc/ttys", 0)) < 0) {
	write(open(CONSOLE, 1), "init: can't open /etc/ttys\n", 27);
	while (1) ;		/* just hang -- system cannot be started */
  }

  /* Read /etc/ttys file. */
  endp = (p = ttys) + read(fd, ttys, TTYSBUF);
  *endp = '\n';

  while(p < endp) {
	if(*p != '0' && *p != '1') goto endline;
	slotp->onflag = ('1' == *p++);			/* 'line on' flag */
	slotp->flags = CRMOD | XTABS | ECHO;		/* sg_flags setting */
	if('a' <= *p && *p <= 'a' + NPARAMSETS)	{	/* a serial line? */
		slotp->flags |= uart[*p - 'a'].flags;
		slotp->speed = uart[*p - 'a'].baud;
	} else if(*p != '0')				/* non-serial line? */
		goto endline;
        ++p;

	if('0' <= *p && *p <= '9') {			/* ttyname = digit? */
		strncpy(slotp->name, "tty?", sizeof (slotp->name));
		slotp->name[3] = *p;			/* fill in '?' */
	} else {					/* full name - copy */
		for(q = slotp->name; *p != '\n';) {
			*q++ = *p++;
		}
		*q = '\0';
	}

	++slotp;
endline:
	while(*p++ != '\n') ;
  }

  close(fd);
}

startup(linenr)
int linenr;
{
  /* Fork off a process for the indicated line. */

  register struct slotent *slotp;	/* pointer to ttyslot */
  int pid;				/* new pid */
  char line[30];			/* tty device name */

  slotp = &slots[linenr];

  if( (pid = fork()) != 0 ) {
	/* Parent */
	slotp->pid = pid;
	if( pid > 0 ) ++pidct;
  } else {
	/* Child */
	close(0);				/* just in case */
	strcpy(line, "/dev/");			/* part of device name */
	strncat(line, slotp->name, sizeof (slotp->name)); /* rest of name */

	if( open(line, 2) != 0 ) exit(EXIT_TTYFAIL);	/* standard input */
	if(	   dup(0) != 1 ) exit(EXIT_TTYFAIL);	/* standard output */
	if( 	   dup(1) != 2 ) exit(EXIT_TTYFAIL);	/* standard error */

	/* Set line parameters. */

	if(ioctl(0, TIOCGETP, &args) < 0) exit(EXIT_TTYFAIL);
	args.sg_ispeed = args.sg_ospeed = slotp->speed;
	args.sg_flags = slotp->flags;
	if(ioctl(0, TIOCSETP, &args) < 0) exit(EXIT_TTYFAIL);

	/* Try to exec login, or in an emergency, exec the shell. */
	execn("/usr/bin/login");
	execn("/bin/login");
	execn("/bin/sh");
	execn("/usr/bin/sh");
	write(open(CONSOLE, 1), "init: couldn't exec login\n", 26);
	exit(EXIT_EXECFAIL);
  }
}

wtmp(line, user, lineno)
char *line, *user; int lineno;
{
  struct utmp entry;
  register int fd;
  extern long time();

  (void) strncpy( entry.ut_line, line, sizeof(entry.ut_line) );
  (void) strncpy( entry.ut_name, user, sizeof(entry.ut_name) );

  entry.ut_time = time((long *)0);

  if( (fd = open( WTMP, 1 )) < 0 ) return;
  if( lseek( fd, 0L, 2 ) >= 0L ) {
	write( fd, (char *) &entry, sizeof(struct utmp) );
  }

  close( fd );

  if( lineno >= 0 ) {		/* remove entry from utmp */
	if( (fd = open( UTMP, 1 )) < 0 ) return;
	    if( lseek( fd, (long) (lineno * sizeof(struct utmp)), 0 ) >= 0 ) {
		write( fd, (char *) &entry, sizeof(struct utmp) );
	    }

  	close( fd );
  }
}
