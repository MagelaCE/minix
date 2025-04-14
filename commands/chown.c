/* chown  -  change owner		 Author: Bert Reuling */

/* This program is also chgrp.c */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

main(argc, argv)
int argc;
char *argv[];
{
  int i, uid, gid, owner, status = 0;
  struct passwd *pwd, *getpwuid(), *getpwnam();
  struct group *grp, *getgrgid(), *getgrnam();
  struct stat st;

  if (strcmp(argv[0], "chown") == 0)
	owner = TRUE;
  else if (strcmp(argv[0], "chgrp") == 0)
	owner = FALSE;
  else {
	fprintf(stderr, "%s: should be named \"chown\" or \"chgrp\"\n", argv[0]);
	exit(1);
  }

  if (argc < 3) {
	fprintf(stderr, "Usage: %s %s file ...\n", argv[0], (owner) ? "owner" : "group");
	exit(1);
  }
  if (isdigit(argv[1][0])) {
	if (owner)
		pwd = getpwuid(atoi(argv[1]));
	else
		grp = getgrgid(atoi(argv[1]));
  } else {
	if (owner)
		pwd = getpwnam(argv[1]);
	else
		grp = getgrnam(argv[1]);
  }

  if (owner && (pwd == NULL)) {
	fprintf(stderr, "%s: unknown user\n", argv[0]);
	exit(1);
  }
  if (!owner && (grp == NULL)) {
	fprintf(stderr, "%s: unknown group\n", argv[0]);
	exit(1);
  }
  for (i = 2; i < argc; i++) {
	if (stat(argv[i], &st) != -1) {
		if (owner) {
			uid = pwd->pw_uid;	/* new owner */
			gid = st.st_gid;	/* old group */
		} else {
			uid = st.st_uid;	/* old owner */
			gid = grp->gr_gid;	/* new group */
		}
		if (chown(argv[i], uid, gid) == -1) {
			fprintf(stderr, "%s: not changed\n", argv[i]);
			status++;
		}
	} else {
		perror(argv[i]);
		status++;
	}
  }
  exit(status);
}
