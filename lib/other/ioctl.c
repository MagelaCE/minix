#include <lib.h>
#include <minix/com.h>
#include <sgtty.h>

PUBLIC int ioctl(fd, request, argp)
int fd;
int request;
struct sgttyb *argp;
{
  int n;
  long erase, kill, intr, quit, xon, xoff, eof, brk, speed;
  struct tchars *argt;

  M.TTY_REQUEST = request;
  M.TTY_LINE = fd;

  switch(request) {
     case TIOCSETP:
	erase = argp->sg_erase & BYTE;
	kill = argp->sg_kill & BYTE;
	speed = ((argp->sg_ospeed & BYTE) << 8) | (argp->sg_ispeed & BYTE);
	M.TTY_SPEK = (speed << 16) | (erase << 8) | kill;
	M.TTY_FLAGS = argp->sg_flags;
	n = callx(FS, IOCTL);
  	return(n);
 
     case TIOCSETC:
	argt = (struct tchars * /* kludge */) argp;
  	intr = argt->t_intrc & BYTE;
  	quit = argt->t_quitc & BYTE;
  	xon  = argt->t_startc & BYTE;
  	xoff = argt->t_stopc & BYTE;
  	eof  = argt->t_eofc & BYTE;
  	brk  = argt->t_brkc & BYTE;		/* not used at the moment */
  	M.TTY_SPEK = (intr<<24) | (quit<<16) | (xon<<8) | (xoff<<0);
  	M.TTY_FLAGS = (eof<<8) | (brk<<0);
  	n = callx(FS, IOCTL);
  	return(n);
  	
     case TIOCGETP:
  	n = callx(FS, IOCTL);
	argp->sg_erase = (M.TTY_SPEK >> 8) & BYTE;
	argp->sg_kill  = (M.TTY_SPEK >> 0) & BYTE;
  	argp->sg_flags = M.TTY_FLAGS & 0xFFFFL;
	speed = (M.TTY_SPEK >> 16) & 0xFFFFL;
	argp->sg_ispeed = speed & BYTE;
	argp->sg_ospeed = (speed >> 8) & BYTE;
  	return(n);

     case TIOCGETC:
  	n = callx(FS, IOCTL);
	argt = (struct tchars *) argp;
  	argt->t_intrc  = (M.TTY_SPEK >> 24) & BYTE;
  	argt->t_quitc  = (M.TTY_SPEK >> 16) & BYTE;
  	argt->t_startc = (M.TTY_SPEK >>  8) & BYTE;
  	argt->t_stopc  = (M.TTY_SPEK >>  0) & BYTE;
  	argt->t_eofc   = (M.TTY_FLAGS >> 8) & BYTE;
  	argt->t_brkc   = (M.TTY_FLAGS >> 8) & BYTE;
  	return(n);

/* This is silly, do we want to add 1001 cases and M.TTY_XYZ's here?
 * We should just pop argp into the message for low-level interpretation.
 */

     case TIOCFLUSH:
	M.TTY_FLAGS = (int /* kludge */) argp;
	return callx(FS, IOCTL);

     default:
	n = -1;
	errno = -(EINVAL);
	return(n);
  }
}
