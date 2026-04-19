/* id - return uid and gid		Author: John J. Marco */


/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/* 		----- id.c -----					*/
/* id - get real and effective user id and group id			*/
/* Author: John J. Marco						*/
/*	   pa1343@sdcc15.ucsd.edu					*/
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
#include "stdio.h"
#include "pwd.h"
#include "grp.h"
main ()
{
	struct passwd *pwd;
	struct passwd *getpwuid ();
	struct group *grp;
	struct group *getgrgid ();
	int uid, gid, euid, egid;
	uid = getuid();
	gid = getgid();
	euid = geteuid();
	egid = getegid();
	if ((pwd = getpwuid(uid)) == 0) 
		printf("%s%d%s","uid=",uid," ");
	else printf("%s%d%s%s%s","uid=",uid,"(",pwd->pw_name,") ");
	if ((grp = getgrgid(gid)) == 0)
		printf("%s%d%s","gid=",gid," ");
	else printf("%s%d%s%s%s","gid=",gid,"(",grp->gr_name,") ");
	if (((pwd = getpwuid(euid)) != 0) && (uid != euid))
		printf("%s%d%s%s%s","euid=",euid,"(",pwd->pw_name,") ");
	else if (uid != euid) printf("%s%d%s","euid=",euid," ");
	if (((grp = getgrgid(egid)) != 0) && (gid != egid))
		printf("%s%d%s%s%s","egid=",egid,"(",grp->gr_name,") ");
	else if (gid != egid) printf("%s%d%s","egid=",egid," ");
	printf("\n");
}
