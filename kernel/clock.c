/* This file contains the code and data for the clock task.  The clock task
 * has a single entry point, clock_task().  It accepts four message types:
 *
 *   HARD_INT:    a clock interrupt has occurred
 *   GET_TIME:    a process wants the real time
 *   SET_TIME:    a process wants to set the real time
 *   SET_ALARM:   a process wants to be alerted after a specified interval
 *
 * The input message is format m6.  The parameters are as follows:
 *
 *     m_type    CLOCK_PROC   FUNC    NEW_TIME
 * ---------------------------------------------
 * | SET_ALARM  | proc_nr  |f to call|  delta  |
 * |------------+----------+---------+---------|
 * | HARD_INT   |          |         |         |
 * |------------+----------+---------+---------|
 * | GET_TIME   |          |         |         |
 * |------------+----------+---------+---------|
 * | SET_TIME   |          |         | newtime |
 * ---------------------------------------------
 *
 * When an alarm goes off, if the caller is a user process, a SIGALRM signal
 * is sent to it.  If it is a task, a function specified by the caller will
 * be invoked.  This function may, for example, send a message, but only if
 * it is certain that the task will be blocked when the timer goes off.
 */

#include "../h/const.h"
#include "../h/type.h"
#include "../h/callnr.h"
#include "../h/com.h"
#include "../h/error.h"
#include "../h/signal.h"
#include "const.h"
#include "type.h"
#include "glo.h"
#include "proc.h"

/* Constant definitions. */
#define MILLISEC         100	/* how often to call the scheduler (msec) */
#define SCHED_RATE (MILLISEC*HZ/1000)	/* number of ticks per schedule */

/* Clock parameters. */
#define TIMER0          0x40	/* port address for timer channel 0 */
#define TIMER_MODE      0x43	/* port address for timer channel 3 */
#define IBM_FREQ    1193182L	/* IBM clock frequency for setting timer */
#define TIMER_COUNT (unsigned) (IBM_FREQ/HZ)/* count to load into timer */
#define SQUARE_WAVE     0x36	/* mode for generating square wave */

/* Clock task variables. */
PRIVATE real_time boot_time;	/* time in seconds of system boot */
PRIVATE real_time next_alarm;	/* probable time of next alarm */
PRIVATE real_time pending_ticks;	/* ticks seen by low level only */
PRIVATE real_time realtime;	/* real time clock */
PRIVATE int sched_ticks = SCHED_RATE;	/* counter: when 0, call scheduler */
PRIVATE struct proc *prev_ptr;	/* last user process run by clock task */
PRIVATE message mc;		/* message buffer for both input and output */
PRIVATE int (*watch_dog[NR_TASKS+1])();	/* watch_dog functions to call */

/*===========================================================================*
 *				clock_task				     *
 *===========================================================================*/
PUBLIC clock_task()
{
/* Main program of clock task.  It determines which of the 4 possible
 * calls this is by looking at 'mc.m_type'.   Then it dispatches.
 */
 
  int opcode;

  init_clock();			/* initialize clock tables */

  /* Main loop of the clock task.  Get work, process it, sometimes reply. */
  while (TRUE) {
     receive(ANY, &mc);		/* go get a message */
     opcode = mc.m_type;	/* extract the function code */

     lock();
     realtime += pending_ticks;	/* transfer ticks from low level handler */
     pending_ticks = 0;
     unlock();

     switch (opcode) {
	case SET_ALARM:	 do_setalarm(&mc);	break;
	case GET_TIME:	 do_get_time();		break;
	case SET_TIME:	 do_set_time(&mc);	break;
	case HARD_INT:   do_clocktick();	break;
	default: panic("clock task got bad message", mc.m_type);
     }

    /* Send reply, except for clock tick. */
    mc.m_type = OK;
    if (opcode != HARD_INT) send(mc.m_source, &mc);
  }
}


/*===========================================================================*
 *				do_setalarm				     *
 *===========================================================================*/
PRIVATE do_setalarm(m_ptr)
message *m_ptr;			/* pointer to request message */
{
/* A process wants an alarm signal or a task wants a given watch_dog function
 * called after a specified interval.  Record the request and check to see
 * it is the very next alarm needed.
 */

  register struct proc *rp;
  int proc_nr;			/* which process wants the alarm */
  long delta_ticks;		/* in how many clock ticks does he want it? */
  int (*function)();		/* function to call (tasks only) */

  /* Extract the parameters from the message. */
  proc_nr = m_ptr->CLOCK_PROC_NR;	/* process to interrupt later */
  delta_ticks = m_ptr->DELTA_TICKS;	/* how many ticks to wait */
  function = m_ptr->FUNC_TO_CALL;	/* function to call (tasks only) */
  rp = proc_addr(proc_nr);
  mc.SECONDS_LEFT = (rp->p_alarm == 0L ? 0 : (rp->p_alarm - realtime)/HZ );
  rp->p_alarm = (delta_ticks == 0L ? 0L : realtime + delta_ticks);
  if istaskp(rp) watch_dog[-proc_nr] = function;

  /* Which alarm is next? */
  next_alarm = MAX_P_LONG;
  for (rp = BEG_PROC_ADDR; rp < END_PROC_ADDR; rp++)
	if(rp->p_alarm != 0 && rp->p_alarm < next_alarm)next_alarm=rp->p_alarm;

}


/*===========================================================================*
 *				do_get_time				     *
 *===========================================================================*/
PRIVATE do_get_time()
{
/* Get and return the current clock time in ticks. */

  mc.m_type = REAL_TIME;	/* set message type for reply */
  mc.NEW_TIME = boot_time + realtime/HZ;	/* current real time */
}


/*===========================================================================*
 *				do_set_time				     *
 *===========================================================================*/
PRIVATE do_set_time(m_ptr)
message *m_ptr;			/* pointer to request message */
{
/* Set the real time clock.  Only the superuser can use this call. */

  boot_time = m_ptr->NEW_TIME - realtime/HZ;
}


/*===========================================================================*
 *				do_clocktick				     *
 *===========================================================================*/
PRIVATE do_clocktick()
{
/* This routine called on clock ticks when a lot of work needs to be done. */

  register struct proc *rp;
  register int t, proc_nr;

  if (next_alarm <= realtime) {
	/* An alarm may have gone off, but proc may have exited, so check. */
	next_alarm = MAX_P_LONG;	/* start computing next alarm */
	for (rp = BEG_PROC_ADDR; rp < END_PROC_ADDR; rp++) {
		if (rp->p_alarm != (real_time) 0) {
			/* See if this alarm time has been reached. */
			if (rp->p_alarm <= realtime) {
				/* A timer has gone off.  If it is a user proc,
				 * send it a signal.  If it is a task, call the
				 * function previously specified by the task.
				 */
				if ( (proc_nr = proc_number(rp)) >= 0)
					cause_sig(proc_nr, SIGALRM);
				else
					(*watch_dog[-proc_nr])();
				rp->p_alarm = 0;
			}

			/* Work on determining which alarm is next. */
			if (rp->p_alarm != 0 && rp->p_alarm < next_alarm)
				next_alarm = rp->p_alarm;
		}
	}
  }

  /* If a user process has been running too long, pick another one. */
  if (--sched_ticks == 0) {
	if (bill_ptr == prev_ptr) locksched();	/* process has run too long */
	sched_ticks = SCHED_RATE;		/* reset quantum */
	prev_ptr = bill_ptr;			/* new previous process */
  }

}


/*===========================================================================*
 *				accounting				     *
 *===========================================================================*/
/* This is now done at the lowest level, in clock_handler(). */


#ifdef i8088
/*===========================================================================*
 *				init_clock				     *
 *===========================================================================*/
PRIVATE init_clock()
{
/* Initialize channel 2 of the 8253A timer to e.g. 60 Hz. */

  port_out(TIMER_MODE, SQUARE_WAVE);	/* set timer to run continuously */
  port_out(TIMER0, TIMER_COUNT);	/* load timer low byte */
  port_out(TIMER0, TIMER_COUNT >> 8);	/* load timer high byte */
}
#endif


/*===========================================================================*
 *				clock_handler				     *
 *===========================================================================*/
PUBLIC clock_handler()
{
/* Switch context to do_clocktick if an alarm has gone off.
 * Also switch there to reschedule if the reschedule will do something.
 * This happens when
 *	(1) quantum has expired
 *	(2) current process received full quantum (as clock sampled it!)
 *	(3) something else is ready to run.
 * Also call TTY and PRINTER and let them do whatever is necessary.
 *
 * Many global global and static variables are accessed here.  The safety
 * of this must be justified.  Most of them are not changed here:
 *	k_reenter:
 *		This safely tells if the clock interrupt is nested.
 *	proc_ptr, bill_ptr:
 *		These are used for accounting.  It does not matter if proc.c
 *		is changing them, provided they are always valid pointers,
 *		since at worst the previous process would be billed.
 *	next_alarm, realtime, sched_ticks, bill_ptr, prev_ptr,
 *	rdy_head[USER_Q]:
 *		These are tested to decide whether to call interrupt().  It
 *		does not matter if the test is sometimes (rarely) backwards
 *		due to a race, since this will only delay the high-level
 *		processing by one tick, or call the high level unnecessarily.
 * The variables which are changed require more care:
 *	rp->user_time, rp->sys_time:
 *		These are protected by explicit locks in system.c.  They are
 *		not properly protected in dmp.c (the increment here is not
 *		atomic) but that hardly matters.
 *	pending_ticks:
 *		This is protected by an explicit lock in clock.c.  Don't
 *		update realtime directly, since there are too many
 *		references to it to guard conveniently.
 *	sched_ticks, prev_ptr:
 *		Updating these competes with similar code in do_clocktick().
 *		No lock is necessary, because if bad things happen here
 *		(like sched_ticks going negative), the code in do_clocktick()
 *		will restore the variables to reasonable values, and an
 *		occasional missed or extra sched() is harmless.
 *
 * Are these complications worth the trouble?  Well, they make the system 15%
 * faster on a 5MHz 8088, and make task debugging much easier since there are
 * no task switches on an inactive system.
 */

  register struct proc *rp;

  /* Update user and system accounting times.
   * First charge the current process for user time.
   * If the current process is not the billable process (usually because it
   * is a task), charge the billable process for system time as well.
   * Thus the unbillable tasks' user time is the billable users' system time.
   */
  if (k_reenter != 0)
	rp = cproc_addr(HARDWARE);
  else
	rp = proc_ptr;
  ++rp->user_time;
  if (rp != bill_ptr)
	++bill_ptr->sys_time;

  ++pending_ticks;
  tty_wakeup();			/* possibly wake up TTY */
  pr_restart();			/* possibly restart printer */

  if (next_alarm <= realtime + pending_ticks ||
      sched_ticks == 1 &&
      bill_ptr == prev_ptr && rdy_head[USER_Q] != NIL_PROC) {
	interrupt(CLOCK);
	return;
  }

  if (--sched_ticks == 0) {
	/* If bill_ptr == prev_ptr, no ready users so don't need sched(). */
	sched_ticks = SCHED_RATE;	/* reset quantum */
	prev_ptr = bill_ptr;		/* new previous process */
  }
}
