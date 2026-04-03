/* su - become super-user		Author: Patrick van Kleef */

/* Modified to set up HOME and SHELL in the environment, and pass */
/* the rest of the environment to the new shell. 1987-Oct-7 EFTH  */

#include "sgtty.h"
#include "stdio.h"
#include "pwd.h"

extern  char  **environ;

char *malloc();


main (argc, argv)
int   argc;
char *argv[];
{
	register char   *name;
	char   *crypt ();
	char   *shell = "/bin/sh";
	char   *_home, *_shell;
	int     nr;
	char    password[14];
	struct sgttyb   args;
	register struct passwd *pwd;
	struct passwd *getpwnam ();

	if (argc > 1)
		name = argv[1];
	else
		name = "root";

	if ((pwd = getpwnam (name)) == 0) {
		std_err("Unknown id: ");
		std_err(name);
		std_err("\n");
		exit (1);
	}

	if (pwd->pw_passwd[0] != '\0' && getuid()!= 0) {
		std_err("Password: ");
		ioctl (0, TIOCGETP, &args);	/* get parameters */
		args.sg_flags = args.sg_flags & (~ECHO);
		ioctl (0, TIOCSETP, &args); 
		nr = read (0, password, 14);
		password[nr - 1] = 0;
		putc('\n',stderr);
		args.sg_flags = args.sg_flags | ECHO;
		ioctl (0, TIOCSETP, &args); 
		if (strcmp (pwd->pw_passwd, crypt (password, pwd->pw_passwd))) {
			std_err("Sorry\n");
			exit (2);
		}
	}
	setgid (pwd->pw_gid);
	setuid (pwd->pw_uid);
	if (pwd->pw_shell[0])
		shell = pwd->pw_shell;

	/*  Set up HOME and SHELL in the environment  */

	_home = malloc( strlen(pwd->pw_dir) + 6 );
	strcpy( _home, "HOME=" );
	strcat( _home, pwd->pw_dir );
	putenv( _home );

	_shell = malloc( strlen(shell) + 7 );
	strcpy( _shell, "SHELL=" );
	strcat( _shell, shell );
	putenv( _shell );

	execle( shell, shell, (char *) 0, environ );
	std_err("No shell\n");
	exit (3);
}
