/*  system.c
 *
 *  Changed to return() on fork failure, added signal()
 *  calls.          Terrence W. Holm      Oct. 1988
 */

#include <stdio.h>
#include <signal.h>

system(cmd)
char *cmd;
{
    int retstat, procid, waitstat;
    int (*sigint)(), (*sigquit)();

    if ( (procid = fork()) == 0) {
	/* Child does an exec of the command. */
        execl( "/bin/sh", "sh", "-c", cmd, 0 );
        exit( 127 );
    }

    /* Check to see if fork failed. */
    if (procid < 0) return( 127 << 8 );

    sigint  = signal( SIGINT,  SIG_IGN );
    sigquit = signal( SIGQUIT, SIG_IGN );

    while ( (waitstat = wait(&retstat)) != procid && waitstat != -1 ) ;
    if (waitstat == -1) retstat = -1;

    signal( SIGINT,  sigint );
    signal( SIGQUIT, sigquit );

    return(retstat);
}
