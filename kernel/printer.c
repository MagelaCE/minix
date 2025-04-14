/* This file contains the printer driver. It is a fairly simple driver,
 * supporting only one printer.  Characters that are written to the driver
 * are written to the printer without any changes at all.
 *
 * The valid messages and their parameters are:
 *
 *   HARD_INT:     interrupt handler has finished current chunk of output
 *   TTY_WRITE:    a process wants to write on a terminal
 *   CANCEL:       terminate a previous incomplete system call immediately
 *
 *    m_type      TTY_LINE   PROC_NR    COUNT    ADDRESS
 * -------------------------------------------------------
 * | HARD_INT    |         |         |         |         |
 * |-------------+---------+---------+---------+---------|
 * | TTY_WRITE   |minor dev| proc nr |  count  | buf ptr |
 * |-------------+---------+---------+---------+---------|
 * | CANCEL      |minor dev| proc nr |         |         |
 * -------------------------------------------------------
 * 
 * Note: since only 1 printer is supported, minor dev is not used at present.
 */

#include "kernel.h"
#include <minix/callnr.h>
#include <minix/com.h>

#define NORMAL_STATUS   0x90	/* printer gives this status when idle */
#define BUSY_STATUS     0x10	/* printer gives this status when busy */
#define ASSERT_STROBE   0x1D	/* strobe a character to the interface */
#define NEGATE_STROBE   0x1C	/* enable interrupt on interface */
#define SELECT          0x0C	/* select printer bit */
#define INIT_PRINTER    0x08	/* init printer bits */
#define NO_PAPER        0x20	/* status bit saying that paper is up */
#define ON_LINE         0x10	/* status bit saying that printer is online */
#define STATUS_MASK	0xB0	/* mask to filter out status bits */ 

PRIVATE int caller;		/* process to tell when printing done (FS) */
PRIVATE int done_status;	/* status of last output completion */
PRIVATE int oleft;		/* bytes of output left in obuf */
PRIVATE char obuf[128];		/* output buffer */
PRIVATE phys_bytes obuf_phys;	/* physical address of obuf */
PRIVATE int opending;		/* nonzero while expected printing not done */
PRIVATE char *optr;		/* ptr to next char in obuf to print */
PRIVATE int orig_count;		/* original byte count */
PRIVATE int port_base;		/* I/O port for printer */
PRIVATE int proc_nr;		/* user requesting the printing */
PRIVATE int user_left;		/* bytes of output left in user buf */
PRIVATE phys_bytes user_phys;	/* physical address of remainder of user buf */
PRIVATE int writing;		/* nonzero while write is in progress */

FORWARD void do_cancel();
FORWARD void do_done();
FORWARD void do_write();
FORWARD void pr_error();
FORWARD void pr_start();
FORWARD void print_init();
FORWARD void reply();

/*===========================================================================*
 *				printer_task				     *
 *===========================================================================*/
PUBLIC void printer_task()
{
/* Main routine of the printer task. */

  message print_mess;		/* buffer for all incoming messages */

  print_init();			/* initialize */

  while (TRUE) {
	receive(ANY, &print_mess);
	switch(print_mess.m_type) {
	    case TTY_WRITE:	do_write(&print_mess);	break;
	    case CANCEL   :	do_cancel(&print_mess);	break;
	    case HARD_INT :	do_done();		break;
	    default:		reply(TASK_REPLY, print_mess.m_source,
				      print_mess.PROC_NR, EINVAL);
							break;
	}
  }
}


/*===========================================================================*
 *				do_write				     *
 *===========================================================================*/
PRIVATE void do_write(m_ptr)
register message *m_ptr;	/* pointer to the newly arrived message */
{
/* The printer is used by sending TTY_WRITE messages to it. Process one. */

  register int r;

  r = SUSPEND;			/* if no errors, tell FS to suspend user */

  /* Reject command if last write is not finished or count is not positive. */
  if (writing) r = EIO;
  if (m_ptr->COUNT <= 0) r = EINVAL;

  if (r == SUSPEND) {
	/* Save information needed later. */
	caller = m_ptr->m_source;
	proc_nr = m_ptr->PROC_NR;
	user_left = m_ptr->COUNT;
	orig_count = m_ptr->COUNT;
	user_phys = numap(m_ptr->PROC_NR, (vir_bytes) m_ptr->ADDRESS,
			  (vir_bytes) m_ptr->COUNT);
	if (user_phys == 0)
		r = E_BAD_ADDR;
	else {
		pr_start();
		writing = TRUE;
	}
  }

  /* Reply to FS, no matter what happened. */
  reply(TASK_REPLY, m_ptr->m_source, m_ptr->PROC_NR, r);
}


/*===========================================================================*
 *				do_done					     *
 *===========================================================================*/
PRIVATE void do_done()
{
/* Previous chunk of printing is finished.  Continue if OK and more.
 * Otherwise, reply to caller (FS).
 */

  register int status;

  if (!writing) return;		/* interrupt while canceling */
  if (done_status != OK) {
	oleft = 0;		/* cancel output by interrupt handler */
	pr_error(done_status);
	status = EIO;
  } else if (user_left != 0) {
	pr_start();
	return;
  } else {
	status = orig_count;
  }
  reply(REVIVE, caller, proc_nr, status);
  writing = FALSE;
}


/*===========================================================================*
 *				do_cancel				     *
 *===========================================================================*/
PRIVATE void do_cancel(m_ptr)
register message *m_ptr;	/* pointer to the newly arrived message */
{
/* Cancel a print request that has already started.  Usually this means that
 * the process doing the printing has been killed by a signal.  It is not
 * clear if there are race conditions.  Try not to cancel the wrong process,
 * but rely on FS to handle the EINTR reply and de-suspension properly.
 */

  if (writing && m_ptr->PROC_NR == proc_nr) {
	oleft = 0;		/* cancel output by interrupt handler */
	writing = FALSE;
  }
  reply(TASK_REPLY, m_ptr->m_source, m_ptr->PROC_NR, EINTR);
}


/*===========================================================================*
 *				reply					     *
 *===========================================================================*/
PRIVATE void reply(code, replyee, process, status)
int code;			/* TASK_REPLY or REVIVE */
int replyee;			/* destination for message (normally FS) */
int process;			/* which user requested the printing */
int status;			/* number of  chars printed or error code */
{
/* Send a reply telling FS that printing has started or stopped. */

  message pr_mess;

  pr_mess.m_type = code;	/* TASK_REPLY or REVIVE */
  pr_mess.REP_STATUS = status;	/* count or EIO */
  pr_mess.REP_PROC_NR = process;	/* which user does this pertain to */
  send(replyee, &pr_mess);	/* send the message */
}


/*===========================================================================*
 *				pr_error				     *
 *===========================================================================*/
PRIVATE void pr_error(status)
int status;			/* printer status byte */
{
/* The printer is not ready.  Display a message on the console telling why. */

  if ((status & ON_LINE) == 0)
	printf("Printer is not on line\n");
  else {
	if (status & NO_PAPER)
		printf("Printer is out of paper\n");
	else
		printf("Printer error, status is 0x%02x\n", status);
  }
}


/*===========================================================================*
 *				print_init				     *
 *===========================================================================*/
PRIVATE void print_init()
{
/* Set global variables.  Get the port base for the first printer from the
 * BIOS and initialize the printer.
 */

  obuf_phys = umap(proc_ptr, D, obuf, sizeof obuf);
  phys_copy(0x408L, umap(proc_ptr, D, &port_base, 2), 2L);
  out_byte(port_base + 2, INIT_PRINTER);
  milli_delay(2);
  out_byte(port_base + 2, SELECT);
  cim_printer();		/* ready for printer interrupts */
}


/*==========================================================================*
 *				pr_start				    *
 *==========================================================================*/
PRIVATE void pr_start()
{
/* Start next chunk of printer output. */

  register int chunk;

  if ( (chunk = user_left) > sizeof obuf) chunk = sizeof obuf;
  phys_copy(user_phys, obuf_phys, (phys_bytes) chunk);
  user_phys += chunk;
  user_left -= chunk;
  optr = obuf;
  opending = TRUE;
  oleft = chunk;		/* now interrupt handler is enabled */
}


/*===========================================================================*
 *				pr_char					     *
 *===========================================================================*/
PUBLIC void pr_char()
{
/* This is the interrupt handler.  When a character has been printed, an
 * interrupt occurs, and the assembly code routine trapped to calls pr_char().
 * One annoying problem is that the 8259A controller sometimes generates
 * spurious interrupts to vector 15, which is the printer vector.  Ignore them.
 */

  register int status;

  if (oleft == 0) return;	/* finished, canceled, or spurious interrupt */

  do {
	/* Loop to handle fast (buffered) printers.  It is important that
	 * processor interrupts are not disabled here, just printer interrupts.
	 */
	status = in_byte(port_base + 1);
	if ((status & STATUS_MASK) == BUSY_STATUS)
		return;		/* still busy with last char */
	else if ((status & STATUS_MASK) == NORMAL_STATUS) {
		/* Everything is all right.  Output another character. */
		out_byte(port_base, *optr++);	/* output character */
		lock();		/* some printers don't like long strobes */
		out_byte(port_base + 2, ASSERT_STROBE);
		out_byte(port_base + 2, NEGATE_STROBE);
		unlock();
		opending = FALSE;	/* show interrupt is working */
	} else {
		/* Error.  This would be better ignored (treat as busy). */
		done_status = status;
		interrupt(PRINTER);
		return;
	}
  }
  while (--oleft != 0);

  /* Finished printing chunk OK. */
  done_status = OK;
  interrupt(PRINTER);
}
/*==========================================================================*
 *				pr_restart				    *
 *==========================================================================*/
PUBLIC void pr_restart()
{
/* Check if printer is hung up, and if so, restart it.
 * Test-and-set done by tasim_printer() stops pr_char() being reentered.
 */

  if (oleft != 0) {
	if (opending && !tasim_printer()) {
		pr_char();
		cim_printer();	/* ready for printer interrupts again */
	}
	opending = TRUE;	/* expect some printing before next call */
  }
}
