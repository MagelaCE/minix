/* Data structures for IOCTL. */

struct sgttyb {
  char sg_ispeed;		/* input speed (not used at present) */
  char sg_ospeed;		/* output speed (not used at present) */
  char sg_erase;		/* erase character */
  char sg_kill;			/* kill character */
  int  sg_flags;		/* mode flags */
};

struct tchars {
  char t_intrc;			/* character that generates SIGINT */
  char t_quitc;			/* character that generates SIGQUIT */
  char t_startc;		/* start output (initially CTRL-Q) */
  char t_stopc;			/* stop output	(initially CTRL-S) */
  char t_eofc;			/* end-of-file  (initially CTRL-D) */
  char t_brkc;			/* input delimiter (like nl) */
};

/* Fields in t_flags. */
#define XTABS	     0006000	/* set to cause tab expansion */
#define RAW	     0000040	/* set to enable raw mode */
#define CRMOD	     0000020	/* set to map If to cr + If */
#define ECHO	     0000010	/* set to enable echoing of typed input */
#define CBREAK	     0000002	/* set to enable cbreak mode */
#define COOKED       0000000	/* neither CBREAK nor RAW */

#define TIOCGETP (('t'<<8) | 8)
#define TIOCSETP (('t'<<8) | 9)
#define TIOCGETC (('t'<<8) | 18)
#define TIOCSETC (('t'<<8) | 17)
