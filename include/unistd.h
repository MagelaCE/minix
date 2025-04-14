/* The <unistd.h> header contains a few miscellaneous manifest constants. */

#ifndef _UNISTD_H
#define _UNISTD_H

/* Values used by access().  POSIX Table 2-6. */
#define F_OK               0	/* test if file exists */
#define X_OK               1	/* test if file is executable */
#define W_OK               2	/* test if file is writable */
#define R_OK               4	/* test if file is readable */

/* Values used for whence in lseek(fd, offset, whence).  POSIX Table 2-7. */
#define SEEK_SET           0	/* offset is absolute  */
#define SEEK_CUR           1	/* offset is relative to current position */
#define SEEK_END           2	/* offset is relative to end of file */

/* This value is required by POSIX Table 2-8. */
#define _POSIX_VERSION 198808L	/* which standard is being conformed to */

/* These three definitions are required by POSIX Sec. 8.2.1.2. */
#define STDIN_FILENO       0	/* file descriptor for stdin */
#define STDOUT_FILENO      1	/* file descriptor for stdout */
#define STDERR_FILENO      2	/* file descriptor for stderr */

/* NULL must be defined in <unistd.h> according to POSIX Sec. 2.8.1. */
#ifndef NULL
#define NULL    ((void *) 0)
#endif

/* The following relate to configurable system variables. POSIX Table 4-2. */
#define _SC_ARG_MAX        1
#define _SC_CHILD_MAX      2
#define _SC_CLOCKS_PER_SEC 3
#define _SC_NGROUPS_MAX    4
#define _SC_OPEN_MAX       5
#define _SC_JOB_CONTROL    6
#define _SC_SAVED_IDS      7
#define _SC_VERSIONS       8

/* The following relate to configurable pathname variables. POSIX Table 5-2. */
#define _PC_LINK_MAX    1	/* link count */
#define _PC_MAX_CANON   2	/* size of the canonical input queue */
#define _PC_MAX_INPUT   3	/* type-ahead buffer size */
#define _PC_NAME_MAX    4	/* file name size */
#define _PC_PATH_MAX    5	/* pathname size */
#define _PC_PIPE_BUF    6	/* pipe size */
#define _PC_NO_TRUNC    7	/* treatment of long name components */
#define _PC_VDISABLE    8	/* tty disable */
#define _PC_CHOWN_RESTRICTED  9	/* chown restricted or not */

/* POSIX defines several options that may be implemented or not, at the
 * implementer's whim.  This implementer has made the following choices:
 *
 * _POSIX_JOB_CONTROL	    not defined:	no job control
 * _POSIX_SAVED_IDS 	    not defined:	no saved uid/gid
 * _POSIX_NO_TRUNC	    not defined:	long path names are truncated
 * _POSIX_CHOWN_RESTRICTED  defined:		you can't give away files
 * _POSIX_VDISABLE	    defined:		tty functions can be disabled
 */
#define _POSIX_CHOWN_RESTRICTED
#define _POSIX_VDISABLE '\t'	/* can't set any control char to tab */


/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( void _exit, (int status)					);
_PROTOTYPE( int access, (char *path, int amode)				);
_PROTOTYPE( int chdir, (char *path)					);
_PROTOTYPE( int chown, (char *path, int owner, int group)		);
_PROTOTYPE( int close, (int fd)						);
_PROTOTYPE( char *ctermid, (char *s)					);
_PROTOTYPE( char *cuserid, (char *s)					);
_PROTOTYPE( int dup, (int fd)						);
_PROTOTYPE( int dup2, (int fd, int fd2)					);
_PROTOTYPE( int execl, (char *path, ...)				);
_PROTOTYPE( int execle, (char *path, ...)				);
_PROTOTYPE( int execlp, (char *file, ...)				);
_PROTOTYPE( int execv, (char *path, char *argv[])			);
_PROTOTYPE( int execve, (char *path, char *argv[], char *envp[])	);
_PROTOTYPE( int execvp, (char *file, char *argv[])			);
_PROTOTYPE( pid_t fork, (void)						);
_PROTOTYPE( long fpathconf, (int fd, int name)				);
_PROTOTYPE( char *getcwd, (char *buf, int size)				);
_PROTOTYPE( gid_t getegid, (void)					);
_PROTOTYPE( uid_t geteuid, (void)					);
_PROTOTYPE( gid_t getgid, (void)					);
_PROTOTYPE( int getgroups, (int gidsetsize, gid_t grouplist[])		);
_PROTOTYPE( char *getlogin, (void)					);
_PROTOTYPE( pid_t getpgrp, (void)					);
_PROTOTYPE( pid_t getpid, (void)					);
_PROTOTYPE( pid_t getppid, (void)					);
_PROTOTYPE( uid_t getuid, (void)					);
_PROTOTYPE( unsigned int alarm, (unsigned int seconds)			);
_PROTOTYPE( unsigned int sleep, (unsigned int seconds)			);
_PROTOTYPE( int isatty, (int fd)					);
_PROTOTYPE( int link, (const char *path1, const char *path2)		);
_PROTOTYPE( off_t lseek, (int fd, off_t offset, int whence)		);
_PROTOTYPE( long pathconf, (char *path, int name)			);
_PROTOTYPE( int pause, (void)						);
_PROTOTYPE( int pipe, (int fildes[2])					);
_PROTOTYPE( int read, (int fd, char *buf, unsigned int n)		);
_PROTOTYPE( int rmdir, (char *path)					);
_PROTOTYPE( int setgid, (int gid)					);
_PROTOTYPE( int setpgid, (pid_t pid, pid_t pgid)			);
_PROTOTYPE( pid_t setsid, (void)					);
_PROTOTYPE( int setuid, (int uid)					);
_PROTOTYPE( long sysconf, (int name)					);
_PROTOTYPE( pid_t tcgetpgrp, (int fd)					);
_PROTOTYPE( int tcsetpgrp, (int fd, pid_t pgrp_id)			);
_PROTOTYPE( char *ttyname, (int fd)					);
_PROTOTYPE( int unlink, (const char *path)				);
_PROTOTYPE( int write, (int fd, char *buf, unsigned int n)		);

#endif /* _UNISTD_H */
