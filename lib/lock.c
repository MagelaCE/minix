#include <errno.h>

typedef enum { FALSE, TRUE } BOOLEAN;

#define LOCKDIR "/tmp/"     /* or /usr/tmp/ as the case may be */
#define MAXTRIES 3
#define NAPTIME 5

BOOLEAN lock(name)          /* acquire lock */
char *name;
{
    char *path, *lockpath();
    int fd, tries;
    extern int errno;

    path = lockpath(name);
    tries = 0;
    while ((fd = creat(path, 0)) == -1 && errno == EACCES)
    {
        if (++tries >= MAXTRIES)
            return(FALSE);
        sleep(NAPTIME);
    }
    if (fd == -1 || close(fd) == -1)
        syserr("lock");
    return(TRUE);
}

void unlock(name)           /* free lock */
char *name;
{
    char *lockpath();

    if (unlink(lockpath(name)) == -1)
        syserr("unlock");
}

static char *lockpath(name) /* generate lock file path */
char *name;
{
    static char path[20];
    char *strcat();

    strcpy(path, LOCKDIR);
    return(strcat(path, name));
}

