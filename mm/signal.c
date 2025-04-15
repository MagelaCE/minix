/* This file handles signals, which are asynchronous events and are generally
 * a messy and unpleasant business.  Signals can be generated by the KILL
 * system call, or from the keyboard (SIGINT) or from the clock (SIGALRM).
 * In all cases control eventually passes to check_sig() to see which processes
 * can be signaled.  The actual signaling is done by sig_proc().
 *
 * The entry points into this file are:
 *   do_signal:	perform the SIGNAL system call
 *   do_kill:	perform the KILL system call
 *   do_ksig:	accept a signal originating in the kernel (e.g., SIGINT)
 *   sig_proc:	interrupt or terminate a signaled process
 *   do_alarm:	perform the ALARM system call by calling set_alarm()
 *   set_alarm:	tell the clock task to start or stop a timer
 *   do_pause:	perform the PAUSE system call
 *   unpause:	check to see if process is suspended on anything
 */


#include "mm.h"
#include <sys/stat.h>
#include <signal.h>
#include <minix/callnr.h>
#include <minix/com.h>
#include "mproc.h"
#include "param.h"

#define DUMP_SIZE	 256	/* buffer size for core dumps */
#define CORE_MODE	0777	/* mode to use on core image files */
#define DUMPED          0200	/* bit set in status when core dumped */

PRIVATE message m_sig;

#if AM_KERNEL
PRIVATE int Tfs;		/* if true then Tell file server to unpause */
#endif

FORWARD int check_sig();
FORWARD void dump_core();
FORWARD void unpause();

/*===========================================================================*
 *				do_signal				     *
 *===========================================================================*/
PUBLIC int do_signal()
{
/* Perform the signal(sig, func) call by setting bits to indicate that a signal
 * is to be caught or ignored.
 */

  int mask;
  int sigign = mp->mp_ignore;

  if (sig < 1 || sig > _NSIG) return(EINVAL);
  if (sig == SIGKILL) return(OK);	/* SIGKILL may not ignored/caught */
  mask = 1 << (sig - 1);	/* singleton set with 'sig' bit on */

  /* All this func does is set the bit maps for subsequent sig processing. */
  if (func == SIG_IGN) {
	mp->mp_ignore |= mask;
	mp->mp_catch &= ~mask;
  } else if (func == SIG_DFL) {
	mp->mp_ignore &= ~mask;
	mp->mp_catch &= ~mask;
  } else {
	mp->mp_ignore &= ~mask;
	mp->mp_catch |= mask;
	mp->mp_func = func;
  }

  if (sigign & mask) return(1);
  return(OK);
}


/*===========================================================================*
 *				do_kill					     *
 *===========================================================================*/
PUBLIC int do_kill()
{
/* Perform the kill(pid, kill_sig) system call. */

  return check_sig(pid, kill_sig, mp->mp_effuid);
}


/*===========================================================================*
 *				do_ksig					     *
 *===========================================================================*/
PUBLIC int do_ksig()
{
/* Certain signals, such as segmentation violations and DEL, originate in the
 * kernel.  When the kernel detects such signals, it sets bits in a bit map.
 * As soon as MM is awaiting new work, the kernel sends MM a message containing
 * the process slot and bit map.  That message comes here.  The File System
 * also uses this mechanism to signal writing on broken pipes (SIGPIPE).
 */

  register struct mproc *rmp;
  int i, j, proc_id, proc_nr, id;
  unshort sig_map;		/* bits 0 - 15 for sigs 1 - 16 */

  /* Only kernel may make this call. */
  if (who != HARDWARE) return(EPERM);
  dont_reply = TRUE;		/* don't reply to the kernel */

  proc_nr = mm_in.PROC1;
  rmp = &mproc[proc_nr];
  if ( (rmp->mp_flags & IN_USE) == 0 || (rmp->mp_flags & HANGING) ) return(OK);
  proc_id = rmp->mp_pid;
  sig_map = (unshort) mm_in.SIG_MAP;
  mp = &mproc[0];		/* pretend kernel signals are from MM */
  mp->mp_procgrp = rmp->mp_procgrp;	/* get process group right */

  /* Stack faults are passed from kernel to MM as pseudo-signal 16. */
  if (sig_map == 1 << (SIGSTKFLT - 1))
	stack_fault(proc_nr);

  /* Check each bit in turn to see if a signal is to be sent.  Unlike
   * kill(), the kernel may collect several unrelated signals for a process
   * and pass them to MM in one blow.  Thus loop on the bit map. For SIGINT
   * and SIGQUIT, use proc_id 0, since multiple processes may have to signaled.
   */
  for (i = 0, j = 1; i < _NSIG - 1; i++, j++) {
	id = (j == SIGINT || j == SIGQUIT) ? 0 : proc_id;
	if (j == SIGKILL) id = -1;	/* simulate kill -1 9 */
	if ( (sig_map >> i) & 1) {
		check_sig(id, j, SUPER_USER);
		sys_sig(proc_nr, -1, SIG_DFL);	/* tell kernel it's done */
	}
  }

  return(OK);
}


/*===========================================================================*
 *				check_sig				     *
 *===========================================================================*/
PRIVATE int check_sig(proc_id, sig_nr, send_uid)
int proc_id;			/* pid of process to signal, or 0 or -1 */
int sig_nr;			/* which signal to send (1-16) */
uid_t send_uid;			/* identity of process sending the signal */
{
/* Check to see if it is possible to send a signal.  The signal may have to be
 * sent to a group of processes.  This routine is invoked by the KILL system
 * call, and also when the kernel catches a DEL or other signal. SIGALRM too.
 */

  register struct mproc *rmp;
  int count, send_sig;
  unshort mask;

  if (sig_nr < 1 || sig_nr > _NSIG) return(EINVAL);
  count = 0;			/* count # of signals sent */
  mask = 1 << (sig_nr - 1);

  /* Search the proc table for processes to signal.  Several tests are made:
   * 	- if proc's uid != sender's, and sender is not superuser, don't signal
   *	- if specific process requested (i.e., 'procpid' > 0), check for match
   *	- if a process has already exited, it can't receive signals
   *	- if 'proc_id' is 0 signal everyone in same process group except caller
   */
  for (rmp = &mproc[INIT_PROC_NR]; rmp < &mproc[NR_PROCS]; rmp++ ) {
	if ( (rmp->mp_flags & IN_USE) == 0) continue;
	send_sig = TRUE;	/* if it's FALSE at end of loop, don't signal*/
	if (send_uid != rmp->mp_effuid && send_uid != SUPER_USER)send_sig=FALSE;
	if (proc_id > 0 && proc_id != rmp->mp_pid) send_sig = FALSE;
	if (rmp->mp_flags & HANGING) send_sig = FALSE;   /*don't wake the dead*/
	if (proc_id == 0 && mp->mp_procgrp != rmp->mp_procgrp)send_sig = FALSE;
	if (send_uid == SUPER_USER && proc_id == -1) send_sig = TRUE;
	if (rmp->mp_pid == INIT_PID && proc_id == -1) send_sig = FALSE;
	if (rmp->mp_pid == INIT_PID && sig_nr == SIGKILL) send_sig = FALSE;

	/* SIGALARM is a little special.  When a process exits, a clock signal
	 * can arrive just as the timer is being turned off.  Also, turn off
	 * ALARM_ON bit when timer goes off to keep it accurate.
	 */
	if (sig_nr == SIGALRM) {
		if ( (rmp->mp_flags & ALARM_ON) == 0) continue;
		if (send_sig) rmp->mp_flags &= ~ALARM_ON;
	}

	if (send_sig == FALSE) continue;
	count++;
	if (rmp->mp_ignore & mask) continue;

#if AM_KERNEL
	/* see if an amoeba transaction should be signalled */
	Tfs = am_check_sig((int)(rmp - mproc), 0);
#endif

	/* Send the signal or kill the process, possibly with core dump. */
	sig_proc(rmp, sig_nr);

	/* If process is hanging on PAUSE, WAIT, tty, pipe, etc. release it. */
	unpause((int)(rmp - mproc));	/* check to see if process is paused */
	if (proc_id > 0) break;	/* only one process being signaled */
  }

  /* If the calling process has killed itself, don't reply. */
  if ((mp->mp_flags & IN_USE) == 0 || (mp->mp_flags & HANGING))dont_reply=TRUE;
  return(count > 0 ? OK : ESRCH);
}


/*===========================================================================*
 *				sig_proc				     *
 *===========================================================================*/
PUBLIC void sig_proc(rmp, sig_nr)
register struct mproc *rmp;	/* pointer to the process to be signaled */
int sig_nr;			/* signal to send to process (1-16) */
{
/* Send a signal to a process.  Check to see if the signal is to be caught.
 * If so, the pc, psw, and signal number are to be pushed onto the process'
 * stack.  If the stack cannot grow or the signal is not to be caught, kill
 * the process.
 */

  unshort mask;
  int core_file;
  vir_bytes new_sp;

  if ( (rmp->mp_flags & IN_USE) == 0) return;	/* if already dead forget it */
  if (rmp->mp_flags & TRACED && sig_nr != SIGKILL) {
	/* A traced process has special handling. */
	stop_proc(rmp, sig_nr); /* a signal causes it to stop */
	return;
  }
  mask = 1 << (sig_nr - 1);
  if (rmp->mp_catch & mask) {
	/* Signal should be caught. */
	rmp->mp_catch &= ~mask;		/* disable further signals */
	sys_getsp((int)(rmp - mproc), &new_sp);
	new_sp -= SIG_PUSH_BYTES;
	if (adjust(rmp, rmp->mp_seg[D].mem_len, new_sp) == OK) {
		sys_sig((int)(rmp - mproc), sig_nr, rmp->mp_func);
		return;		/* successful signal */
	}
  }

  /* Signal should not or cannot be caught.  Take default action. */
  core_file = ( core_bits >> (sig_nr - 1 )) & 1;
  rmp->mp_sigstatus = (char) sig_nr;
  if (core_file) dump_core(rmp); /* dump core */
  mm_exit(rmp, 0);		/* terminate process */
}


/*===========================================================================*
 *				do_alarm				     *
 *===========================================================================*/
PUBLIC int do_alarm()
{
/* Perform the alarm(seconds) system call. */

  register int r;
  unsigned sec;

  sec = (unsigned) seconds;
  r = set_alarm(who, sec);
  return(r);
}


/*===========================================================================*
 *				set_alarm				     *
 *===========================================================================*/
PUBLIC int set_alarm(proc_nr, sec)
int proc_nr;			/* process that wants the alarm */
unsigned sec;			/* how many seconds delay before the signal */
{
/* This routine is used by do_alarm() to set the alarm timer.  It is also used
 * to turn the timer off when a process exits with the timer still on.
 */

  int remaining;

  m_sig.m_type = SET_ALARM;
  m_sig.CLOCK_PROC_NR = proc_nr;
  m_sig.DELTA_TICKS = HZ * sec;
  if (sec != 0)
	mproc[proc_nr].mp_flags |= ALARM_ON;	/* turn ALARM_ON bit on */
  else
	mproc[proc_nr].mp_flags &= ~ALARM_ON;	/* turn ALARM_ON bit off */

  /* Tell the clock task to provide a signal message when the time comes. */
  if (sendrec(CLOCK, &m_sig) != OK) panic("alarm er", NO_NUM);
  remaining = (int) m_sig.SECONDS_LEFT;
  return(remaining);
}


/*===========================================================================*
 *				do_pause				     *
 *===========================================================================*/
PUBLIC int do_pause()
{
/* Perform the pause() system call. */

  mp->mp_flags |= PAUSED;	/* turn on PAUSE bit */
  dont_reply = TRUE;
  return(OK);
}


/*===========================================================================*
 *				unpause					     *
 *===========================================================================*/
PRIVATE void unpause(pro)
int pro;			/* which process number */
{
/* A signal is to be sent to a process.  If that process is hanging on a
 * system call, the system call must be terminated with EINTR.  Possible
 * calls are PAUSE, WAIT, READ and WRITE, the latter two for pipes and ttys.
 * First check if the process is hanging on PAUSE or WAIT.  If not, tell FS,
 * so it can check for READs and WRITEs from pipes, ttys and the like.
 */

  register struct mproc *rmp;

  rmp = &mproc[pro];

  /* Check to see if process is hanging on a PAUSE call. */
  if ( (rmp->mp_flags & PAUSED) && (rmp->mp_flags & HANGING) == 0) {
	rmp->mp_flags &= ~PAUSED;	/* turn off PAUSED bit */
	reply(pro, EINTR, 0, NIL_PTR);
	return;
  }

  /* Check to see if process is hanging on a WAIT call. */
  if ( (rmp->mp_flags & WAITING) && (rmp->mp_flags & HANGING) == 0) {
	rmp->mp_flags &= ~WAITING;	/* turn off WAITING bit */
	reply(pro, EINTR, 0, NIL_PTR);
	return;
  }

#if AM_KERNEL
  /* if it was an amoeba transaction, it is already tidied up by now. */
  if (Tfs)
#endif
  /* Process is not hanging on an MM call.  Ask FS to take a look. */
	tell_fs(UNPAUSE, pro, 0, 0);

  return;
}


/*===========================================================================*
 *				dump_core				     *
 *===========================================================================*/
PRIVATE void dump_core(rmp)
register struct mproc *rmp;	/* whose core is to be dumped */
{
/* Make a core dump on the file "core", if possible. */

  struct stat s_buf, d_buf;
  char buf[DUMP_SIZE];
  int i, r, s, er1, er2, slot;
  vir_bytes v_buf;
  long a, c, ct, dest;
  struct mproc *xmp;

  /* Change to working directory of dumpee. */
  slot = (int)(rmp - mproc);
  tell_fs(CHDIR, slot, 0, 0);

  /* Can core file be written? */
  if (rmp->mp_realuid != rmp->mp_effuid) {
	tell_fs(CHDIR, 0, 1, 0);	/* go back to MM's directory */
	return;
  }
  xmp = mp;			/* allowed() looks at 'mp' */
  mp = rmp;
  r = allowed(core_name, &s_buf, W_BIT);	/* is core_file writable */
  s = allowed(".", &d_buf, W_BIT);	/* is directory writable? */
  mp = xmp;
  if (r >= 0) close(r);
  if (s >= 0) close(s);
  if (rmp->mp_effuid == SUPER_USER) r = 0;	/* su can always dump core */

  if (s >= 0 && (r >= 0 || r == ENOENT)) {
	/* Either file is writable or it doesn't exist & dir is writable */
	r = creat(core_name, CORE_MODE);
	tell_fs(CHDIR, 0, 1, 0);	/* go back to MM's own dir */
	if (r < 0) return;
	rmp->mp_sigstatus |= DUMPED;

	/* First write the memory map of all segments on core file. */
	if (write(r, (char *) rmp->mp_seg, (int)sizeof(rmp->mp_seg)) < 0) {
		close(r);
		return;
	}

	/* Now loop through segments and write the segments themselves out. */
	v_buf = (vir_bytes) buf;
	dest = (long) v_buf;
	for (i = 0; i < NR_SEGS; i++) {
		a = (phys_bytes) rmp->mp_seg[i].mem_vir << CLICK_SHIFT;
		c = (phys_bytes) rmp->mp_seg[i].mem_len << CLICK_SHIFT;

		/* Loop through a segment, dumping it. */
		while (c > 0) {
			ct = MIN(c, DUMP_SIZE);
			er1 = mem_copy(slot, i, a, MM_PROC_NR, D, dest, ct);
			er2 = write(r, buf, (int) ct);
			if (er1 < 0 || er2 < 0) {
				close(r);
				return;
			}
			a += ct;
			c -= ct;
		}
	}
  } else {
	tell_fs(CHDIR, 0, 1, 0);	/* go back to MM's own dir */
	close(r);
	return;
  }

  close(r);
}
