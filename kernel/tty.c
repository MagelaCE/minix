/* This file contains the terminal driver, both for the IBM console and regular
 * ASCII terminals.  It is split into two sections, a device-independent part
 * and a device-dependent part.  The device-independent part accepts
 * characters to be printed from programs and queues them in a standard way
 * for device-dependent output.  It also accepts input and queues it for
 * programs. This file contains 2 main entry points: tty_task() and keyboard().
 * When a key is struck on a terminal, an interrupt to an assembly language
 * routine is generated.  This routine saves the machine state and registers
 * and calls keyboard(), which enters the character in an internal table, and
 * then sends a message to the terminal task.  The main program of the terminal
 * task is tty_task(). It accepts not only messages about typed input, but
 * also requests to read and write from terminals, etc. 
 *
 * The device-dependent part interfaces with the IBM console and ASCII
 * terminals.  The IBM keyboard is unusual in that keystrokes yield key numbers
 * rather than ASCII codes, and furthermore, an interrupt is generated when a
 * key is depressed and again when it is released.  The IBM display is memory
 * mapped, so outputting characters such as line feed, backspace and bell are
 * tricky.
 *
 * The valid messages and their parameters are:
 *
 *   TTY_CHAR_INT: a character has been typed (character arrived interrupt)
 *   TTY_READ:     a process wants to read from a terminal
 *   TTY_WRITE:    a process wants to write on a terminal
 *   TTY_IOCTL:    a process wants to change a terminal's parameters
 *   TTY_SETPGRP:  indicate a change in a control terminal
 *   CANCEL:       terminate a previous incomplete system call immediately
 *
 *    m_type      TTY_LINE   PROC_NR    COUNT   TTY_SPEK  TTY_FLAGS  ADDRESS
 * ---------------------------------------------------------------------------
 * | TTY_CHAR_INT|         |         |         |         |         |array ptr|
 * |-------------+---------+---------+---------+---------+---------+---------|
 * | TTY_O_DONE  |minor dev|         |         |         |         |array ptr|
 * |-------------+---------+---------+---------+---------+---------+---------|
 * | TTY_READ    |minor dev| proc nr |  count  |         |         | buf ptr |
 * |-------------+---------+---------+---------+---------+---------+---------|
 * | TTY_WRITE   |minor dev| proc nr |  count  |         |         | buf ptr |
 * |-------------+---------+---------+---------+---------+---------+---------|
 * | TTY_IOCTL   |minor dev| proc nr |func code|erase etc|  flags  |         |
 * |-------------+---------+---------+---------+---------+---------+---------|
 * | TTY_SETPGRP |minor dev| proc nr |         |         |         |         |
 * |-------------+---------+---------+---------+---------+---------+---------
 * | CANCEL      |minor dev| proc nr |         |         |         |         |
 * ---------------------------------------------------------------------------
 */

#include "../h/const.h"
#include "../h/type.h"
#include "../h/callnr.h"
#include "../h/com.h"
#include "../h/error.h"
#include "../h/sgtty.h"
#include "../h/signal.h"
#include "const.h"
#include "type.h"
#include "glo.h"
#include "proc.h"

#define NR_CONS            1	/* how many consoles can system handle */
#define	NR_RS_LINES	   1	/* how many rs232 terminals can system handle*/
#define TTY_IN_BYTES     200	/* input queue size */
#define TTY_RAM_WORDS    320	/* ram buffer size */
#define TTY_BUF_SIZE     256	/* unit for copying to/from queues */
#define TAB_SIZE           8	/* distance between tabs */
#define TAB_MASK          07	/* mask for tty_column when tabbing */
#define MAX_OVERRUN      100	/* size of overrun input buffer */
#define MAX_ESC_PARMS      2	/* number of escape sequence params allowed */

#define ERASE_CHAR      '\b'	/* default erase character */
#define KILL_CHAR        '@'	/* default kill character */
#define INTR_CHAR (char)0177	/* default interrupt character */
#define QUIT_CHAR (char) 034	/* default quit character */
#define XOFF_CHAR (char) 023	/* default x-off character (CTRL-S) */
#define XON_CHAR  (char) 021	/* default x-on character (CTRL-Q) */
#define EOT_CHAR  (char) 004	/* CTRL-D */
#define MARKER    (char) 000	/* non-escaped CTRL-D stored as MARKER */
#define AT_SIGN         0220	/* code to yield for CTRL-@ */
#define SCODE1            71	/* scan code for Home on numeric pad */
#define SCODE2            81	/* scan code for PgDn on numeric pad */
#define DEL_CODE   (char) 83	/* DEL for use in CTRL-ALT-DEL reboot */
#define ESC       (char) 033	/* escape */
#define BRACKET          '['	/* Part of the ESC [ letter escape seq */

#define F1                59	/* scan code for function key F1 */
#define F2                60	/* scan code for function key F2 */
#define F3                61	/* scan code for function key F3 */
#define F4                62	/* scan code for function key F4 */
#define F5                63	/* scan code for function key F5 */
#define F9                67	/* scan code for function key F9 */
#define F10               68	/* scan code for function key F10 */
#define TOP_ROW           14	/* codes below this are shifted if CTRL */

PRIVATE struct tty_struct {
  /* Input queue.  Typed characters are stored here until read by a program. */
  char tty_inqueue[TTY_IN_BYTES];    /* array used to store the characters */
  char *tty_inhead;		/* pointer to place where next char goes */
  char *tty_intail;		/* pointer to next char to be given to prog */
  int tty_incount;		/* # chars in tty_inqueue */
  int tty_lfct;			/* # line feeds in tty_inqueue */

  /* Output section. */
  int tty_ramqueue[TTY_RAM_WORDS];	/* buffer for video RAM */
  int tty_rwords;		/* number of WORDS (not bytes) in outqueue */
  int tty_org;			/* location in RAM where 6845 base points */
  int tty_vid;			/* current position of cursor in video RAM */
  char tty_esc_state;		/* 0=normal, 1=ESC, 2=ESC[ */
  char tty_esc_intro;		/* Distinguishing character following ESC */
  int tty_esc_parmv[MAX_ESC_PARMS];	/* list of escape parameters */
  int *tty_esc_parmp;		/* pointer to current escape parameter */
  int tty_attribute;		/* current attribute byte << 8 */
  int (*tty_devstart)();	/* routine to start actual device output */

  /* Terminal parameters and status. */
  int tty_mode;			/* terminal mode set by IOCTL */
  int tty_speed;		/* low byte is ispeed; high byte is ospeed */
  int tty_column;		/* current column number (0-origin) */
  int tty_row;			/* current row (0 at top of screen) */
  char tty_busy;		/* 1 when output in progress, else 0 */
  char tty_escaped;		/* 1 when '\' just seen, else 0 */
  char tty_inhibited;		/* 1 when CTRL-S just seen (stops output) */
  char tty_makebreak;		/* 1 for terminals that interrupt twice/key */
  char tty_waiting;		/* 1 when output process waiting for reply */

  /* User settable characters: erase, kill, interrupt, quit, x-on; x-off. */
  char tty_erase;		/* char used to erase 1 char (init ^H) */
  char tty_kill;		/* char used to erase a line (init @) */
  char tty_intr;		/* char used to send SIGINT  (init DEL) */
  char tty_quit;		/* char used for core dump   (init CTRL-\) */
  char tty_xon;			/* char used to start output (init CTRL-Q)*/
  char tty_xoff;		/* char used to stop output  (init CTRL-S) */
  char tty_eof;			/* char used to stop output  (init CTRL-D) */

  /* Information about incomplete I/O requests is stored here. */
  char tty_incaller;		/* process that made the call (usually FS) */
  char tty_inproc;		/* process that wants to read from tty */
  char *tty_in_vir;		/* virtual address where data is to go */
  int tty_inleft;		/* how many chars are still needed */
  char tty_otcaller;		/* process that made the call (usually FS) */
  char tty_outproc;		/* process that wants to write to tty */
  char *tty_out_vir;		/* virtual address where data comes from */
  phys_bytes tty_phys;		/* physical address where data comes from */
  int tty_outleft;		/* # chars yet to be copied to tty_outqueue */
  int tty_cum;			/* # chars copied to tty_outqueue so far */
  int tty_pgrp;			/* slot number of controlling process */

  /* Miscellaneous. */
  int tty_ioport;		/* I/O port number for this terminal */
} tty_struct[NR_CONS+NR_RS_LINES];

/* Values for the fields. */
#define NOT_ESCAPED        0	/* previous character on this line not '\' */
#define ESCAPED            1	/* previous character on this line was '\' */
#define RUNNING            0	/* no CRTL-S has been typed to stop the tty */
#define STOPPED            1	/* CTRL-S has been typed to stop the tty */
#define INACTIVE           0	/* the tty is not printing */
#define BUSY               1	/* the tty is printing */
#define ONE_INT            0	/* regular terminals interrupt once per char */
#define TWO_INTS           1	/* IBM console interrupts two times per char */
#define NOT_WAITING        0	/* no output process is hanging */
#define WAITING            1	/* an output process is waiting for a reply */
#define COMPLETED          2	/* output done; send a completion message */

PRIVATE char tty_driver_buf[2*MAX_OVERRUN+2]; /* driver collects chars here */
PRIVATE char tty_copy_buf[2*MAX_OVERRUN];  /* copy buf used to avoid races */
PRIVATE char tty_buf[TTY_BUF_SIZE];	/* scratch buffer to/from user space */
PRIVATE int shift1, shift2, capslock, numlock;	/* keep track of shift keys */
PRIVATE int control, alt;	/* keep track of key statii */
PRIVATE int caps_off = 1;	/* 1 = normal position, 0 = depressed */
PRIVATE int num_off = 1;	/* 1 = normal position, 0 = depressed */
PRIVATE int softscroll = 0;	/* 1 = software scrolling, 0 = hardware */
PRIVATE int output_done;	/* number of RS232 output messages to be sent*/
PUBLIC int color;		/* 1 if console is color, 0 if it is mono */
PUBLIC int scan_code;		/* scan code for '=' saved by bootstrap */


/* Scan codes to ASCII for unshifted keys */
PRIVATE char unsh[] = {
 0,033,'1','2','3','4','5','6',        '7','8','9','0','-','=','\b','\t',
 'q','w','e','r','t','y','u','i',      'o','p','[',']',015,0202,'a','s',
 'd','f','g','h','j','k','l',';',      047,0140,0200,0134,'z','x','c','v',
 'b','n','m',',','.','/',0201,'*',     0203,' ',0204,0241,0242,0243,0244,0245,
 0246,0247,0250,0251,0252,0205,0210,0267,  0270,0271,0211,0264,0265,0266,0214
,0261,0262,0263,'0',0177
};

/* Scan codes to ASCII for shifted keys */
PRIVATE char sh[] = {
 0,033,'!','@','#','$','%','^',        '&','*','(',')','_','+','\b','\t',
 'Q','W','E','R','T','Y','U','I',      'O','P','{','}',015,0202,'A','S',
 'D','F','G','H','J','K','L',':',      042,'~',0200,'|','Z','X','C','V',
 'B','N','M','<','>','?',0201,'*',    0203,' ',0204,0221,0222,0223,0224,0225,
 0226,0227,0230,0231,0232,0204,0213,'7',  '8','9',0211,'4','5','6',0214,'1',
 '2','3','0','.'
};


/* Scan codes to ASCII for Olivetti M24 for unshifted keys. */
PRIVATE char unm24[] = {
 0,033,'1','2','3','4','5','6',        '7','8','9','0','-','^','\b','\t',
 'q','w','e','r','t','y','u','i',      'o','p','@','[','\r',0202,'a','s',
 'd','f','g','h','j','k','l',';',      ':',']',0200,'\\','z','x','c','v',
 'b','n','m',',','.','/',0201,'*',     0203,' ',0204,0241,0242,0243,0244,0245,
 0246,0247,0250,0251,0252,023,0210,0267,0270,0271,0211,0264,0265,0266,0214,0261,
0262,0263,'0','.',' ',014,0212,'\r',   0264,0262,0266,0270,032,0213,' ','/',
 0253,0254,0255,0256,0257,0215,0216,0217
};

/* Scan codes to ASCII for Olivetti M24 for shifted keys. */
PRIVATE char m24[] = {
 0,033,'!','"','#','$','%','&',        047,'(',')','_','=','~','\b','\t',
 'Q','W','E','R' ,'T','Y','U','I',     'O','P',0140,'{','\r',0202,'A','S',
 'D','F','G','H','J','K','L','+',      '*','}',0200,'|','Z','X','C','V',
 'B','N','M','<','>','?',0201,'*',     0203,' ',0204,0221,0222,0223,0224,0225,
 0226,0227,0230,0231,0232,0270,023,'7', '8','9',0211,'4','5','6',0214,'1',
 '2','3',0207,0177,0271,014,0272,'\r', '\b','\n','\f',036,032,0273,0274,'/',
 0233,0234,0235,0236,0237,0275,0276,0277
};

char scode_map[] = {'H', 'A', 'V', 'S', 'D', 'G', 'C', 'T', 'Y', 'B', 'U'};

/*===========================================================================*
 *				tty_task				     *
 *===========================================================================*/
PUBLIC tty_task()
{
/* Main routine of the terminal task. */

  message tty_mess;		/* buffer for all incoming messages */
  register struct tty_struct *tp;

  tty_init();			/* initialize */
  init_rs232();

  while (TRUE) {
	receive(ANY, &tty_mess);
	tp = &tty_struct[tty_mess.TTY_LINE];
	switch(tty_mess.m_type) {
	    case TTY_O_DONE:	/* same action as TTY_CHAR_INT */
	    case TTY_CHAR_INT:	do_int(&tty_mess);		break;
	    case TTY_READ:	do_read(tp, &tty_mess);		break;
	    case TTY_WRITE:	do_write(tp, &tty_mess);	break;
	    case TTY_IOCTL:	do_ioctl(tp, &tty_mess);	break;
	    case TTY_SETPGRP:   do_setpgrp(tp, &tty_mess);	break;
	    case CANCEL:	do_cancel(tp, &tty_mess);	break;
	    default:		tty_reply(TASK_REPLY, tty_mess.m_source, 
					tty_mess.PROC_NR, EINVAL, 0L, 0L);
	}
  }
}


/*===========================================================================*
 *				do_int					     *
 *===========================================================================*/
PRIVATE do_int(m_ptr)
message *m_ptr;
{
/* The TTY task can generate two kinds of interrupts:
 *	- a character has been typed on the console or an RS232 line
 *	- an RS232 line has completed a write request (on behalf of a user)
 * If either interrupt happens and the TTY task is idle, the task gets the
 * interrupt message immediately and processes it.  However, if the TTY
 * task is busy, a bit is set in 'busy_map' and the message pointer stored.
 * If multiple messages happen, the bit is only set once.  No input data is
 * lost even if this happens because all the input messages say is that there
 * is some input.  The actual input is in the tty_driver_buf array, so losing
 * a message just means that when the one interrupt-generated message is given
 * to the TTY task, it will find multiple characters in tty_driver_buf.
 *
 * The introduction of RS232 lines has complicated this situation somewhat. Now
 * a message can mean that some RS232 line has finished transmitting all the
 * output given to it.  If a character is typed at the instant an RS232 line
 * finishes, one of the two messages may be overwritten because MINIX only
 * provides single buffering for interrupt messages (in proc.c).To avoid losing
 * information, whenever an RS232 line finishes, the flag tty_waiting is set
 * to COMPLETED and kept that way until its completion is processed and a 
 * message sent to FS saying that output is done.  The number of RS232 lines in
 * COMPLETED state is kept in output_done, which is checked on each interrupt,
 * so that a lost TTY_O_DONE line completion interrupt will be quickly
 * recovered.
 *
 * In short, when this procedure is called, it can check for RS232 line done
 * by inspecting output_done and it can check for characters in the input
 * buffer by inspecting tty_driver_buf[0].  Thus losing a message to the TTY
 * task is not serious because the underlying conditions are explicitly checked
 * for on each interrupt.
 */

  /* First check to see if any RS232 lines have completed. */
  if (output_done > 0) {
	/* If a message is sent to FS for RS232 done, don't process any input
	 * characters now for fear of sending a second message to FS, which 
	 * would be lost.
	 */
	if (tty_o_done()) {
		return;
	}
  }
  charint(m_ptr);			/* check for input characters */
}


/*===========================================================================*
 *				charint					     *
 *===========================================================================*/
PRIVATE charint(m_ptr)
message *m_ptr;			/* message containing pointer to char(s) */
{
/* A character has been typed.  If a character is typed and the tty task is
 * not able to service it immediately, the character is accumulated within
 * the tty driver.  Thus multiple chars may be accumulated.  A single message
 * to the tty task may have to process several characters.
 */

  int m, n, count, replyee, caller, old_state;
  char *ptr, *copy_ptr, ch;
  struct tty_struct *tp;

  old_state = lock();
  ptr = m_ptr->ADDRESS;		/* pointer to accumulated char array */
  copy_ptr = tty_copy_buf;	/* ptr to shadow array where chars copied */
  n = *ptr;			/* how many chars have been accumulated */
  count = n;			/* save the character count */
  n = n + n;			/* each char occupies 2 bytes */
  ptr += 2;			/* skip count field at start of array */
  while (n-- > 0)
	*copy_ptr++ = *ptr++;	/* copy the array to safety */
  ptr = m_ptr->ADDRESS;
  *ptr = 0;			/* accumulation count set to 0 */
  restore(old_state);

  /* Loop on the accumulated characters, processing each in turn. */
  if (count == 0) return;	/* on TTY_O_DONE interrupt, count might be 0 */
  copy_ptr = tty_copy_buf;
  while (count-- > 0) {
	ch = *copy_ptr++;	/* get the character typed */
	n = *copy_ptr++;	/* get the line number it came in on */
	in_char(n, ch);		/* queue the char and echo it */

	/* See if a previously blocked reader can now be satisfied. */
	tp = &tty_struct[n];	/* pointer to struct for this character */
	if (tp->tty_inleft > 0 ) {	/* does anybody want input? */
		m = tp->tty_mode & (CBREAK | RAW);
		if (tp->tty_lfct > 0 || (m != 0 && tp->tty_incount > 0)) {
			m = rd_chars(tp);

			/* Tell hanging reader that chars have arrived. */
			replyee = (int) tp->tty_incaller;
			caller = (int) tp->tty_inproc;
			tty_reply(REVIVE, replyee, caller, m, 0L, 0L);
		}
	}
  }
}


/*===========================================================================*
 *				in_char					     *
 *===========================================================================*/
PRIVATE in_char(line, ch)
int line;			/* line number on which char arrived */
char ch;			/* scan code for character that arrived */
{
/* A character has just been typed in.  Process, save, and echo it. */

  register struct tty_struct *tp;
  int mode, sig, scode;
  char make_break();

  scode = ch;			/* save the scan code */
  tp = &tty_struct[line];	/* set 'tp' to point to proper struct */

  /* Function keys are temporarily being used for debug dumps. */
  if (line == 0 && ch >= F1 && ch <= F10) {	/* Check for function keys */
	func_key(ch);		/* process function key */
	return;
  }
  if (tp->tty_incount >= TTY_IN_BYTES) return;	/* no room, discard char */
  mode = tp->tty_mode & (RAW | CBREAK);
  if (tp->tty_makebreak == TWO_INTS)
	ch = make_break(ch);	/* console give 2 ints/ch */
  else
	if (mode != RAW) ch &= 0177;	/* 7-bit chars except in raw mode */
  if (ch == 0) return;

  /* Processing for COOKED and CBREAK mode contains special checks. */
  if (mode == COOKED || mode == CBREAK) {
	/* Handle erase, kill and escape processing. */
	if (mode == COOKED) {
		/* First erase processing (rub out of last character). */
		if (ch == tp->tty_erase && tp->tty_escaped == NOT_ESCAPED) {
			if (chuck(tp) != -1) {	/* remove last char entered */
				echo(tp, '\b');	/* remove it from the screen */
				echo(tp, ' ');
				echo(tp, '\b');
			}
			return;
		}

		/* Now do kill processing (remove current line). */
		if (ch == tp->tty_kill && tp->tty_escaped == NOT_ESCAPED) {
			while( chuck(tp) == OK) /* keep looping */ ;
			echo(tp, tp->tty_kill);
			echo (tp, '\n');
			return;
		}

		/* Handle EOT and the escape symbol (backslash). */
		if (tp->tty_escaped == NOT_ESCAPED) {
			/* Normal case: previous char was not backslash. */
			if (ch == '\\') {
				/* An escaped symbol has just been typed. */
				tp->tty_escaped = ESCAPED;
				echo(tp, ch);
				return;	/* do not store the '\' */
			}
			/* CTRL-D means end-of-file, unless it is escaped. It
			 * is stored in the text as MARKER, and counts as a
			 * line feed in terms of knowing whether a full line
			 * has been typed already.
			 */
			if (ch == tp->tty_eof) ch = MARKER;
		} else {
			/* Previous character was backslash. */
			tp->tty_escaped = NOT_ESCAPED;	/* turn escaping off */
			if (ch != tp->tty_erase && ch != tp->tty_kill &&
						   ch != tp->tty_eof) {
				/* Store the escape previously skipped over */
				*tp->tty_inhead++ = '\\';
				tp->tty_incount++;
				if (tp->tty_inhead ==
						&tp->tty_inqueue[TTY_IN_BYTES])
					tp->tty_inhead = tp->tty_inqueue;
			}
		}
	}
	/* Both COOKED and CBREAK modes come here; first map CR to LF. */
	if (ch == '\r' && (tp->tty_mode & CRMOD)) ch = '\n';

	/* Check for interrupt and quit characters. */
	if (ch == tp->tty_intr || ch == tp->tty_quit) {
		sig = (ch == tp->tty_intr ? SIGINT : SIGQUIT);
		sigchar(tp, sig);
		return;
	}

	/* Check for and process CTRL-S (terminal stop). */
	if (ch == tp->tty_xoff) {
		tp->tty_inhibited = STOPPED;
		return;
	}

	/* Check for and process CTRL-Q (terminal start). */
	if (tp->tty_inhibited == STOPPED) {
		tp->tty_inhibited = RUNNING;
		(*tp->tty_devstart)(tp);	/* resume output */
		return;
	}
  }

  /* All 3 modes come here. */
  if (ch == '\n' || ch == MARKER) tp->tty_lfct++;	/* count line feeds */

  /* The numeric pad generates ASCII escape sequences: ESC [ letter */
  if (line == 0 && scode >= SCODE1 && scode <= SCODE2 && 
		shift1 == 0 && shift2 == 0 && numlock == 0) {
	/* This key is to generate a three-character escape sequence. */
	*tp->tty_inhead++ = ESC; /* put ESC in the input queue */
	if (tp->tty_inhead == &tp->tty_inqueue[TTY_IN_BYTES])
		tp->tty_inhead = tp->tty_inqueue;      /* handle wraparound */
	tp->tty_incount++;
	echo(tp, 'E');
	*tp->tty_inhead++ = BRACKET; /* put ESC in the input queue */
	if (tp->tty_inhead == &tp->tty_inqueue[TTY_IN_BYTES])
		tp->tty_inhead = tp->tty_inqueue;      /* handle wraparound */
	tp->tty_incount++;
	echo(tp, BRACKET);
	ch = scode_map[scode-SCODE1];	/* generate the letter */
  }

  *tp->tty_inhead++ = ch;	/* save the character in the input queue */
  if (tp->tty_inhead == &tp->tty_inqueue[TTY_IN_BYTES])
	tp->tty_inhead = tp->tty_inqueue;	/* handle wraparound */
  tp->tty_incount++;
  echo(tp, ch);
}


#ifdef i8088
/*===========================================================================*
 *				make_break				     *
 *===========================================================================*/
PRIVATE char make_break(ch)
char ch;			/* scan code of key just struck or released */
{
/* This routine can handle keyboards that interrupt only on key depression,
 * as well as keyboards that interrupt on key depression and key release.
 * For efficiency, the interrupt routine filters out most key releases.
 */

  int c, make, code;


  c = ch & 0177;		/* high-order bit set on key release */
  make = (ch & 0200 ? 0 : 1);	/* 1 when key depressed, 0 when key released */
  if (olivetti == FALSE) {
	/* Standard IBM keyboard. */
	code = (shift1 || shift2 ? sh[c] : unsh[c]);
	if (control && c < TOP_ROW) code = sh[c];	/* CTRL-(top row) */
	if (c > 70 && numlock) 		/* numlock depressed */
		code = (shift1 || shift2 ? unsh[c] : sh[c]);
  } else {
	/* (Olivetti M24 or AT&T 6300) with Olivetti-style keyboard. */
	code = (shift1 || shift2 ? m24[c] : unm24[c]);
	if (control && c < TOP_ROW) code = sh[c];	/* CTRL-(top row) */
	if (c > 70 && numlock) 		/* numlock depressed */
		code = (shift1 || shift2 ? unm24[c] : m24[c]);
  }
  code &= BYTE;
  if (code < 0200 || code >= 0206) {
	/* Ordinary key, i.e. not shift, control, alt, etc. */
	if (capslock)
		if (code >= 'A' && code <= 'Z')
			code += 'a' - 'A';
		else if (code >= 'a' && code <= 'z')
			code -= 'a' - 'A';
	if (alt) code |= 0200;	/* alt key ORs 0200 into code */
	if (control) code &= 037;
	if (code == 0) code = AT_SIGN;	/* @ is 0100, so CTRL-@ = 0 */
	if (make == 0) code = 0;	/* key release */
	return(code);
  }

  /* Table entries 0200 - 0206 denote special actions. */
  switch(code - 0200) {
    case 0:	shift1 = make;		break;	/* shift key on left */
    case 1:	shift2 = make;		break;	/* shift key on right */
    case 2:	control = make;		break;	/* control */
    case 3:	alt = make;		break;	/* alt key */
    case 4:	if (make && caps_off) {
			capslock = 1 - capslock;
			set_leds();
		}
		caps_off = 1 - make;
		break;	/* caps lock */
    case 5:	if (make && num_off) {
			numlock  = 1 - numlock;
			set_leds();
		}
		num_off = 1 - make;
		break;	/* num lock */
  }
  return(0);
}
#endif


/*===========================================================================*
 *				echo					     *
 *===========================================================================*/
PRIVATE echo(tp, c)
register struct tty_struct *tp;	/* terminal on which to echo */
register char c;		/* character to echo */
{
/* Echo a character on the terminal. */

  if ( (tp->tty_mode & ECHO) == 0) return;	/* if no echoing, don't echo */
  if (c != MARKER) {
	if (tp - tty_struct < NR_CONS)
		out_char(tp, c);	/* echo to console */
	else
		rs_out_char(tp, c);	/* echo to RS232 line */
  }
  flush(tp);			/* force character out onto the screen */
}


/*===========================================================================*
 *				chuck					     *
 *===========================================================================*/
PRIVATE int chuck(tp)
register struct tty_struct *tp;	/* from which tty should chars be removed */
{
/* Delete one character from the input queue.  Used for erase and kill. */

  char *prev;

  /* If input queue is empty, don't delete anything. */
  if (tp->tty_incount == 0) return(-1);

  /* Don't delete '\n' or '\r'. */
  prev = (tp->tty_inhead != tp->tty_inqueue ? tp->tty_inhead - 1 :
					     &tp->tty_inqueue[TTY_IN_BYTES-1]);
  if (*prev == '\n' || *prev == '\r') return(-1);
  tp->tty_inhead = prev;
  tp->tty_incount--;
  return(OK);			/* char erasure was possible */
}


/*===========================================================================*
 *				do_read					     *
 *===========================================================================*/
PRIVATE do_read(tp, m_ptr)
register struct tty_struct *tp;	/* pointer to tty struct */
message *m_ptr;			/* pointer to message sent to the task */
{
/* A process wants to read from a terminal. */

  int code, caller;


  if (tp->tty_inleft > 0) {	/* if someone else is hanging, give up */
	tty_reply(TASK_REPLY, m_ptr->m_source, m_ptr->PROC_NR, EIO, 0L, 0L);
	return;
  }

  /* Copy information from the message to the tty struct. */
  tp->tty_incaller = m_ptr->m_source;
  tp->tty_inproc = m_ptr->PROC_NR;
  tp->tty_in_vir = m_ptr->ADDRESS;
  tp->tty_inleft = m_ptr->COUNT;

  /* Try to get chars.  This call either gets enough, or gets nothing. */
  code = rd_chars(tp);

  caller = (int) tp->tty_inproc;
  tty_reply(TASK_REPLY, m_ptr->m_source, caller, code, 0L, 0L);
}


/*===========================================================================*
 *				rd_chars				     *
 *===========================================================================*/
PRIVATE int rd_chars(tp)
register struct tty_struct *tp;	/* pointer to terminal to read from */
{
/* A process wants to read from a terminal.  First check if enough data is
 * available. If so, pass it to the user.  If not, send FS a message telling
 * it to suspend the user.  When enough data arrives later, the tty driver
 * copies it to the user space directly and notifies FS with a message.
 */

  int cooked, ct, user_ct, buf_ct, cum, enough, eot_seen;
  vir_bytes in_vir, left;
  phys_bytes user_phys, tty_phys;
  char ch, *tty_ptr;
  struct proc *rp;
  extern phys_bytes umap();

  cooked = ( (tp->tty_mode & (RAW | CBREAK)) ? 0 : 1);	/* 1 iff COOKED mode */
  if (tp->tty_incount == 0 || (cooked && tp->tty_lfct == 0)) return(SUSPEND);
  rp = proc_addr(tp->tty_inproc);
  in_vir = (vir_bytes) tp-> tty_in_vir;
  left = (vir_bytes) tp->tty_inleft;
  if ( (user_phys = umap(rp, D, in_vir, left)) == 0) return(E_BAD_ADDR);
  tty_phys = umap(proc_addr(TTY), D, (vir_bytes) tty_buf, TTY_BUF_SIZE);
  cum = 0;
  enough = 0;
  eot_seen = 0;

  /* The outer loop iterates on buffers, one buffer load per iteration. */
  while (tp->tty_inleft > 0) {
	buf_ct = MIN(tp->tty_inleft, tp->tty_incount);
	buf_ct = MIN(buf_ct, TTY_BUF_SIZE);
	ct = 0;
	tty_ptr = tty_buf;

	/* The inner loop fills one buffer. */
	while(buf_ct-- > 0) {
		ch = *tp->tty_intail++;
		if (tp->tty_intail == &tp->tty_inqueue[TTY_IN_BYTES])
			tp->tty_intail = tp->tty_inqueue;
		*tty_ptr++ = ch;
		ct++;
		if (ch == '\n' || ch == MARKER) {
			tp->tty_lfct--;
			if (cooked && ch == MARKER) eot_seen++;
			enough++;	/* exit loop */
			if (cooked) break;	/* only provide 1 line */
		}
	}

	/* Copy one buffer to user space.  Be careful about CTRL-D.  In cooked
	 * mode it is not transmitted to user programs, and is not counted as
	 * a character as far as the count goes, but it does occupy space in 
	 * the driver's tables and must be counted there.
	 */
	user_ct = (eot_seen ? ct - 1 : ct);	/* bytes to copy to user */
	phys_copy(tty_phys, user_phys, (phys_bytes) user_ct);
	user_phys += user_ct;
	cum += user_ct;
	tp->tty_inleft -= ct;
	tp->tty_incount -= ct;
	if (tp->tty_incount == 0 || enough) break;
  }

  tp->tty_inleft = 0;
  return(cum);
}


/*===========================================================================*
 *				finish					     *
 *===========================================================================*/
PRIVATE finish(tp, code)
register struct tty_struct *tp;	/* pointer to tty struct */
int code;			/* reply code */
{
/* A command has terminated (possibly due to DEL).  Tell caller. */

  int line, result, replyee, caller;

  tp->tty_rwords = 0;
  tp->tty_outleft = 0;
  if (tp->tty_waiting == NOT_WAITING) return;
  line = tp - tty_struct;
  result = (line < NR_CONS ? TASK_REPLY : REVIVE);
  replyee = (int) tp->tty_otcaller;
  caller = (int) tp->tty_outproc;
  tty_reply(result, replyee, caller, code, 0L, 0L);
  tp->tty_waiting = NOT_WAITING;
}


/*===========================================================================*
 *				do_write				     *
 *===========================================================================*/
PRIVATE do_write(tp, m_ptr)
register struct tty_struct *tp;	/* pointer to tty struct */
message *m_ptr;			/* pointer to message sent to the task */
{
/* A process wants to write on a terminal. */

  vir_bytes out_vir, out_left;
  struct proc *rp;
  extern phys_bytes umap();
  int caller,replyee;

  /* If the slot is already in use, better return an error than mess it up. */
  if (tp->tty_outleft > 0) {	/* if someone else is hanging, give up */
	tty_reply(TASK_REPLY, m_ptr->m_source, m_ptr->PROC_NR, EIO, 0L, 0L);
	return;
  }

  /* Copy message parameters to the tty structure. */
  tp->tty_otcaller = m_ptr->m_source;
  tp->tty_outproc = m_ptr->PROC_NR;
  tp->tty_out_vir = m_ptr->ADDRESS;
  tp->tty_outleft = m_ptr->COUNT;
  tp->tty_waiting = WAITING;
  tp->tty_cum = 0;

  /* Compute the physical address where the data is in user space. */
  rp = proc_addr(tp->tty_outproc);
  out_vir = (vir_bytes) tp->tty_out_vir;
  out_left = (vir_bytes) tp->tty_outleft;
  if ( (tp->tty_phys = umap(rp, D, out_vir, out_left)) == 0) {
	/* Buffer address provided by user is outside its address space. */
	tp->tty_cum = E_BAD_ADDR;
	tp->tty_outleft = 0;
  }

  /* Copy characters from the user process to the terminal. */
  (*tp->tty_devstart)(tp);	/* copy data to queue and start I/O */

  /* If output is for a bitmapped terminal as the IBM-PC console, the output-
   * routine will return at once so there is no need to suspend the caller,
   * on ascii terminals however, the call is suspended and later revived.
   */
  if (m_ptr->TTY_LINE != 0) {
	caller = (int) tp->tty_outproc;
	replyee = (int) tp->tty_otcaller;
	tty_reply(TASK_REPLY, replyee, caller, SUSPEND, 0L, 0L);
  }
}


/*===========================================================================*
 *				do_ioctl				     *
 *===========================================================================*/
PRIVATE do_ioctl(tp, m_ptr)
register struct tty_struct *tp;	/* pointer to tty_struct */
message *m_ptr;			/* pointer to message sent to task */
{
/* Perform IOCTL on this terminal. */

  long flags, erki, erase, kill, intr, quit, xon, xoff, eof;
  int r;

  r = OK;
  flags = 0;
  erki = 0;
  switch(m_ptr->TTY_REQUEST) {
     case TIOCSETP:
	/* Set erase, kill, and flags. */
	tp->tty_erase = (char) ((m_ptr->TTY_SPEK >> 8) & BYTE);	/* erase  */
	tp->tty_kill  = (char) ((m_ptr->TTY_SPEK >> 0) & BYTE);	/* kill  */
	tp->tty_mode  = (int) m_ptr->TTY_FLAGS;	/* mode word */
	if (m_ptr->TTY_SPEED != 0) tp->tty_speed = m_ptr->TTY_SPEED;
	if (tp-tty_struct >= NR_CONS)
		set_uart(tp - tty_struct, tp->tty_mode, tp->tty_speed);
	break;

     case TIOCSETC:
	/* Set intr, quit, xon, xoff, eof (brk not used). */
	tp->tty_intr = (char) ((m_ptr->TTY_SPEK >> 24) & BYTE);	/* interrupt */
	tp->tty_quit = (char) ((m_ptr->TTY_SPEK >> 16) & BYTE);	/* quit */
	tp->tty_xon  = (char) ((m_ptr->TTY_SPEK >>  8) & BYTE);	/* CTRL-S */
	tp->tty_xoff = (char) ((m_ptr->TTY_SPEK >>  0) & BYTE);	/* CTRL-Q */
	tp->tty_eof  = (char) ((m_ptr->TTY_FLAGS >> 8) & BYTE);	/* CTRL-D */
	break;

     case TIOCGETP:
	/* Get erase, kill, and flags. */
	erase = ((long) tp->tty_erase) & BYTE;
	kill  = ((long) tp->tty_kill) & BYTE;
	erki  = (erase << 8) | kill;
	flags = ( (long) tp->tty_speed << 16) | (long) tp->tty_mode;
	break;

     case TIOCGETC:
	/* Get intr, quit, xon, xoff, eof. */
	intr  = ((long) tp->tty_intr) & BYTE;
	quit  = ((long) tp->tty_quit) & BYTE;
	xon   = ((long) tp->tty_xon)  & BYTE;
	xoff  = ((long) tp->tty_xoff) & BYTE;
	eof   = ((long) tp->tty_eof)  & BYTE;
	erki  = (intr << 24) | (quit << 16) | (xon << 8) | (xoff << 0);
	flags = (eof <<8);
	break;

     default:
	r = EINVAL;
  }

  /* Send the reply. */
  tty_reply(TASK_REPLY, m_ptr->m_source, m_ptr->PROC_NR, r, flags, erki);
}


/*===========================================================================*
 *				do_setpgrp				     *
 *===========================================================================*/
PRIVATE do_setpgrp(tp, m_ptr)
register struct tty_struct *tp; /* pointer to tty struct */
message *m_ptr;			/* pointer to message sent to task */
{
/* A control process group has changed */

   tp->tty_pgrp = m_ptr->TTY_PGRP;
   tty_reply(TASK_REPLY, m_ptr->m_source, m_ptr->PROC_NR, OK, 0L, 0L);
}


/*===========================================================================*
 *				do_cancel				     *
 *===========================================================================*/
PRIVATE do_cancel(tp, m_ptr)
register struct tty_struct *tp;	/* pointer to tty_struct */
message *m_ptr;			/* pointer to message sent to task */
{
/* A signal has been sent to a process that is hanging trying to read or write.
 * The pending read or write must be finished off immediately.
 */

  int mode;

  /* First check to see if the process is indeed hanging.  If it is not, don't
   * reply (to avoid race conditions).
   */
  if (tp->tty_inleft == 0 && tp->tty_outleft == 0) return;

  /* Kill off input/output. */
  mode = m_ptr->COUNT;
  if (mode & R_BIT) {
	/* Process was reading when killed.  Clean up input. */
	tp->tty_inhead = tp->tty_inqueue;	/* discard all data */
	tp->tty_intail = tp->tty_inqueue;
	tp->tty_incount = 0;
	tp->tty_lfct = 0;
	tp->tty_inleft = 0;
	tp->tty_inhibited = RUNNING;
  }
  if (mode & W_BIT) {
	/* Process was writing when killed.  Clean up output. */
	tp->tty_outleft = 0;
	tp->tty_waiting = NOT_WAITING;	/* don't send reply */
  }
  tty_reply(TASK_REPLY, m_ptr->m_source, m_ptr->PROC_NR, EINTR, 0L, 0L);
}


/*===========================================================================*
 *				tty_reply				     *
 *===========================================================================*/
PRIVATE tty_reply(code, replyee, proc_nr, status, extra, other)
int code;			/* TASK_REPLY or REVIVE */
int replyee;			/* destination address for the reply */
int proc_nr;			/* to whom should the reply go? */
int status;			/* reply code */
long extra;			/* extra value */
long other;			/* used for IOCTL replies */
{
/* Send a reply to a process that wanted to read or write data. */

  message tty_mess;

  tty_mess.m_type = code;
  tty_mess.REP_PROC_NR = proc_nr;
  tty_mess.REP_STATUS = status;
  tty_mess.TTY_FLAGS = extra;	/* used by IOCTL for flags (mode) */
  tty_mess.TTY_SPEK = other;	/* used by IOCTL for erase and kill chars */
  send(replyee, &tty_mess);
}


/*===========================================================================*
 *				sigchar					     *
 *===========================================================================*/
PRIVATE sigchar(tp, sig)
register struct tty_struct *tp;	/* pointer to tty_struct */
int sig;			/* SIGINT, SIGQUIT, or SIGKILL */
{
/* Process a SIGINT, SIGQUIT or SIGKILL char from the keyboard */

  tp->tty_inhibited = RUNNING;	/* do implied CRTL-Q */
  finish(tp, EINTR);		/* send reply */
  tp->tty_inhead = tp->tty_inqueue;	/* discard input */
  tp->tty_intail = tp->tty_inqueue;
  tp->tty_incount = 0;
  tp->tty_lfct = 0;
  if (tp >= &tty_struct[NR_CONS]) rs_sig(tp);	/* RS232 only */
  if (tp->tty_pgrp) cause_sig(tp->tty_pgrp, sig);
}




/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#ifdef i8088
/* Now begins the code and data for the device-dependent tty drivers. */

/* Definitions used by the console driver. */
#define COLOR_BASE    0xB800	/* video ram paragraph for color display */
#define MONO_BASE     0xB000	/* video ram address for mono display */
#define C_VID_MASK    0x3FFF	/* mask for 16K video RAM */
#define M_VID_MASK    0x0FFF	/* mask for  4K video RAM */
#define C_RETRACE     0x0300	/* how many characters to display at once */
#define M_RETRACE     0x7000	/* how many characters to display at once */
#define WORD_MASK     0xFFFF	/* mask for 16 bits */
#define OFF_MASK      0x000F	/* mask for  4 bits */
#define BEEP_FREQ     0x0533	/* value to put into timer to set beep freq */
#define B_TIME        0x2000	/* how long to sound the CTRL-G beep tone */
#define BLANK         0x0700	/* determines  cursor color on blank screen */
#define LINE_WIDTH        80	/* # characters on a line */
#define SCR_LINES         25	/* # lines on the screen */
#define CTRL_S            31	/* scan code for letter S (for CRTL-S) */
#define MONOCHROME         1	/* value for tty_ioport tells color vs. mono */
#define CONSOLE            0	/* line number for console */
#define GO_FORWARD         0	/* scroll forward */
#define GO_BACKWARD        1	/* scroll backward */
#define TIMER2          0x42	/* I/O port for timer channel 2 */
#define TIMER3          0x43	/* I/O port for timer channel 3 */
#define KEYBD           0x60	/* I/O port for keyboard data */
#define PORT_B          0x61	/* I/O port for 8255 port B */
#define KBIT            0x80	/* bit used to ack characters to keyboard */
#define LED_CODE        0xED	/* command to keyboard to set LEDs */
#define LED_DELAY       0x80	/* device dependent delay needed */

/* Constants relating to the video RAM and 6845. */
#define M_6845         0x3B0	/* port for 6845 mono */
#define C_6845         0x3D0	/* port for 6845 color */
#define EGA            0x3C0	/* port for EGA card */
#define INDEX              4	/* 6845's index register */
#define DATA               5	/* 6845's data register */
#define CUR_SIZE          10	/* 6845's cursor size register */
#define VID_ORG           12	/* 6845's origin register */
#define CURSOR            14	/* 6845's cursor register */

/* Definitions used for determining if the keyboard is IBM or Olivetti type. */
#define KB_STATUS	0x64	/* Olivetti keyboard status port */
#define BYTE_AVAIL	0x01	/* there is something in KEYBD port */
#define KB_BUSY	        0x02	/* KEYBD port ready to accept a command */
#define DELUXE		0x01	/* this bit is set up iff deluxe keyboard */
#define GET_TYPE	   5	/* command to get keyboard type */
#define OLIVETTI_EQUAL    12	/* the '=' key is 12 on olivetti, 13 on IBM */

/* Global variables used by the console driver. */
PUBLIC  message keybd_mess;	/* message used for console input chars */
PRIVATE vid_retrace;		/* how many characters to display per burst */
PRIVATE unsigned vid_base;	/* base of video ram (0xB000 or 0xB800) */
PUBLIC int vid_mask;		/* 037777 for color (16K) or 07777 for mono */
PRIVATE int vid_port;		/* I/O port for accessing 6845 */


/*===========================================================================*
 *				keyboard				     *
 *===========================================================================*/
PUBLIC keyboard()
{
/* A keyboard interrupt has occurred.  Process it. */

  int val, code, k, raw_bit;
  char stopc;

  /* Fetch the character from the keyboard hardware and acknowledge it. */
  port_in(KEYBD, &code);	/* get the scan code for the key struck */
  port_in(PORT_B, &val);	/* strobe the keyboard to ack the char */
  port_out(PORT_B, val | KBIT);	/* strobe the bit high */
  port_out(PORT_B, val);	/* now strobe it low */

  /* The IBM keyboard interrupts twice per key, once when depressed, once when
   * released.  Filter out the latter, ignoring all but the shift-type keys.
   * The shift-type keys 29, 42, 54, 56, 58, and 69 must be processed normally.
   */
  k = code - 0200;		/* codes > 0200 mean key release */
  if (k > 0) {
	/* A key has been released. */
	if (k != 29 && k != 42 && k != 54 && k != 56 && k != 58 && k != 69) {
		port_out(INT_CTL, ENABLE);	/* re-enable interrupts */
	 	return;		/* don't call tty_task() */
	}
  } else {
	/* Check to see if character is CTRL-S, to stop output. Setting xoff
	 * to anything other than CTRL-S will not be detected here, but will
	 * be detected later, in the driver.  A general routine to detect any
	 * xoff character here would be complicated since we only have the
	 * scan code here, not the ASCII character.
	 */
	raw_bit = tty_struct[CONSOLE].tty_mode & RAW;
	stopc = tty_struct[CONSOLE].tty_xoff;
	if (raw_bit == 0 && control && code == CTRL_S && stopc == XOFF_CHAR) {
		tty_struct[CONSOLE].tty_inhibited = STOPPED;
		port_out(INT_CTL, ENABLE);
		return;
	}
  }

  /* Check for CTRL-ALT-DEL, and if found, reboot the computer. */
  if (control && alt && code == DEL_CODE) reboot();	/* CTRL-ALT-DEL */

  /* Store the character in memory so the task can get at it later.
   * tty_driver_buf[0] is the current count, and tty_driver_buf[1] is the
   * maximum allowed to be stored.
   */
  if ( (k = tty_driver_buf[0]) < tty_driver_buf[1]) {
	/* There is room to store this character; do it. */
	k = k + k;			/* each entry contains two bytes */
	tty_driver_buf[k+2] = code;	/* store the scan code */
	tty_driver_buf[k+3] = CONSOLE;	/* tell which line it came from */
	tty_driver_buf[0]++;		/* increment counter */

	/* Build and send the interrupt message. */
	keybd_mess.m_type = TTY_CHAR_INT;
	keybd_mess.ADDRESS = tty_driver_buf;
	interrupt(TTY, &keybd_mess);	/* send a message to the tty task */
  } else {
	/* Too many characters have been buffered.  Discard excess. */
	port_out(INT_CTL, ENABLE);	/* re-enable 8259A controller */
  }
}


/*===========================================================================*
 *				console					     *
 *===========================================================================*/
PRIVATE console(tp)
register struct tty_struct *tp;	/* tells which terminal is to be used */
{
/* Copy as much data as possible to the output queue, then start I/O.  On
 * memory-mapped terminals, such as the IBM console, the I/O will also be
 * finished, and the counts updated.  Keep repeating until all I/O done.
 */

  extern char get_byte();
  int count;
  char c;
  unsigned segment, offset, offset1;

  /* Loop over the user bytes one at a time, outputting each one. */
  segment = (tp->tty_phys >> 4) & WORD_MASK;
  offset = tp->tty_phys & OFF_MASK;
  offset1 = offset;
  count = 0;

  while (tp->tty_outleft > 0 && tp->tty_inhibited == RUNNING) {
	c = get_byte(segment, offset);	/* fetch 1 byte from user space */
	out_char(tp, c);	/* write 1 byte to terminal */
	offset++;		/* advance one character in user buffer */
	tp->tty_outleft--;	/* decrement count */
  }
  flush(tp);			/* clear out the pending characters */

  /* Update terminal data structure. */
  count = offset - offset1;	/* # characters printed */
  tp->tty_phys += count;	/* advance physical data pointer */
  tp->tty_cum += count;		/* number of characters printed */

  /* If all data has been copied to the terminal, send the reply. */
  if (tp->tty_outleft == 0) finish(tp, tp->tty_cum);
}


/*===========================================================================*
 *				out_char				     *
 *===========================================================================*/
PRIVATE out_char(tp, c)
register struct tty_struct *tp;	/* pointer to tty struct */
char c;				/* character to be output */
{
/* Output a character on the console.  Check for escape sequences first. */

  if (tp->tty_esc_state > 0) {
	parse_escape(tp, c);
	return;
  }

  switch(c) {
	case 007:		/* ring the bell */
		flush(tp);	/* print any chars queued for output */
		beep(BEEP_FREQ);/* BEEP_FREQ gives bell tone */
		return;

	case 013:		/* CTRL-K */
		move_to(tp, tp->tty_column, tp->tty_row - 1);
		return;

	case 014:		/* CTRL-L */
		move_to(tp, tp->tty_column + 1, tp->tty_row);
		return;

	case 016:		/* CTRL-N */
		move_to(tp, tp->tty_column + 1, tp->tty_row);
		return;

	case '\b':		/* backspace */
		move_to(tp, tp->tty_column - 1, tp->tty_row);
		return;

	case '\n':		/* line feed */
		if (tp->tty_mode & CRMOD) out_char(tp, '\r');
		if (tp->tty_row == SCR_LINES-1) 
			scroll_screen(tp, GO_FORWARD);
		else
			tp->tty_row++;
		move_to(tp, tp->tty_column, tp->tty_row);
		return;

	case '\r':		/* carriage return */
		move_to(tp, 0, tp->tty_row);
		return;

	case '\t':		/* tab */
		if ( (tp->tty_mode & XTABS) == XTABS) {
			do {
				out_char(tp, ' ');
			} while (tp->tty_column & TAB_MASK);
			return;
		}
		/* Ignore tab if XTABS is off--video RAM has no hardware tab */
		return;

	case 033:		/* ESC - start of an escape sequence */
		flush(tp);	/* print any chars queued for output */
		tp->tty_esc_state = 1;	/* mark ESC as seen */
		return;

	default:		/* printable chars are stored in ramqueue */
		if (tp->tty_column >= LINE_WIDTH) return;	/* long line */
		if (tp->tty_rwords == TTY_RAM_WORDS) flush(tp);
		tp->tty_ramqueue[tp->tty_rwords++]=tp->tty_attribute|(c & BYTE);
		tp->tty_column++;	/* next column */
		return;
  }
}

/*===========================================================================*
 *				scroll_screen				     *
 *===========================================================================*/
PRIVATE scroll_screen(tp, dir)
register struct tty_struct *tp;	/* pointer to tty struct */
int dir;			/* GO_FORWARD or GO_BACKWARD */
{
  int amount, offset, bytes;

  bytes = 2 * (SCR_LINES - 1) * LINE_WIDTH;	/* 2 * 24 * 80 bytes */

  /* Scrolling the screen is a real nuisance due to the various incompatible
   * video cards.  This driver supports hardware scrolling (mono and CGA cards)
   * and software scrolling (EGA cards).
   */
  if (softscroll) {
	/* Software scrolling for non-IBM compatible EGA cards. */
	if (dir == GO_FORWARD) {
		scr_up(vid_base);
		vid_copy(NIL_PTR, vid_base, tp->tty_org+bytes, LINE_WIDTH);
	} else {
		scr_down(vid_base);
		vid_copy(NIL_PTR, vid_base, tp->tty_org, LINE_WIDTH);
	}
  } else {
	/* Normal scrolling using the 6845 registers. */
	amount = (dir == GO_FORWARD ? 2 * LINE_WIDTH : -2 * LINE_WIDTH);
	tp->tty_org = (tp->tty_org + amount) & vid_mask;
	if (dir == GO_FORWARD)
		offset = (tp->tty_org + bytes) & vid_mask;
	else
		offset = tp->tty_org;

	/* Blank the new line at top or bottom. */
	vid_copy(NIL_PTR, vid_base, offset, LINE_WIDTH);
	set_6845(VID_ORG, tp->tty_org >> 1);	/* 6845 thinks in words */
  }
}


/*===========================================================================*
 *				flush					     *
 *===========================================================================*/
PRIVATE flush(tp)
register struct tty_struct *tp;	/* pointer to tty struct */
{
/* Have the characters in 'ramqueue' transferred to the screen. */

  if (tp->tty_rwords == 0) return;
  vid_copy((char *)tp->tty_ramqueue, vid_base, tp->tty_vid, tp->tty_rwords);

  /* Update the video parameters and cursor. */
  tp->tty_vid = (tp->tty_vid + 2 * tp->tty_rwords);
  set_6845(CURSOR, tp->tty_vid >> 1);	/* cursor counts in words */
  tp->tty_rwords = 0;
}


/*===========================================================================*
 *				move_to					     *
 *===========================================================================*/
PRIVATE move_to(tp, x, y)
struct tty_struct *tp;		/* pointer to tty struct */
int x;				/* column (0 <= x <= 79) */
int y;				/* row (0 <= y <= 24, 0 at top) */
{
/* Move the cursor to (x, y). */

  flush(tp);			/* flush any pending characters */
  if (x < 0 || x >= LINE_WIDTH || y < 0 || y >= SCR_LINES) return;
  tp->tty_column = x;		/* set x co-ordinate */
  tp->tty_row = y;		/* set y co-ordinate */
  tp->tty_vid = (tp->tty_org + 2*y*LINE_WIDTH + 2*x);
  set_6845(CURSOR, tp->tty_vid >> 1);	/* cursor counts in words */
}


/*===========================================================================*
 *				escape					     *
 *===========================================================================*/
PRIVATE parse_escape(tp, c)
register struct tty_struct *tp;	/* pointer to tty struct */
char c;				/* next character in escape sequence */
{
/* The following ANSI escape sequences are currently supported:
 *   ESC M         to reverse index the screen
 *   ESC [ y ; x H to move cursor to (x, y) [default (1,1)]
 *   ESC [ 0 J     to clear from cursor to end of screen
 *   ESC [ n m     to set the screen rendition
 *			n: 0 = normal [default]
 *			   7 = reverse
 */

  switch (tp->tty_esc_state) {
	case 1: 		/* ESC seen */
		tp->tty_esc_intro = '\0';
		tp->tty_esc_parmp = tp->tty_esc_parmv;
		tp->tty_esc_parmv[0] = tp->tty_esc_parmv[1] = 0;
		switch (c) {
		  case '[': 	/* Control Sequence Introducer */
			tp->tty_esc_intro = c;
			tp->tty_esc_state = 2; 
			break;
		  case 'M': 	/* Reverse Index */
			do_escape(tp, c);
			break;
		  default: 
			tp->tty_esc_state = 0; 
			break;
		}
		break;

	case 2: 		/* ESC [ seen */
		if (c >= '0' && c <= '9') {
			if (tp->tty_esc_parmp 
					< tp->tty_esc_parmv + MAX_ESC_PARMS)
				*tp->tty_esc_parmp =
				  *tp->tty_esc_parmp * 10 + (c - '0');
			break;
		}
		else if (c == ';') {
			if (++tp->tty_esc_parmp 
					< tp->tty_esc_parmv + MAX_ESC_PARMS)
				*tp->tty_esc_parmp = 0;
			break;
		}
		else {
			do_escape(tp, c);
		}
		break;
	default:		/* illegal state */
		tp->tty_esc_state = 0;
		break;
  }
}


/*===========================================================================*
 *				do_escape				     *
 *===========================================================================*/
PRIVATE do_escape(tp, c)
register struct tty_struct *tp;	/* pointer to tty struct */
char c;				/* next character in escape sequence */
{
  int n, ct, vx;

  /* Handle a sequence beginning with just ESC */
  if (tp->tty_esc_intro == '\0') {
    switch (c) {
	case 'M':		/* Reverse Index */
		if (tp->tty_row == 0)
			scroll_screen(tp, GO_BACKWARD);
		else
			tp->tty_row--;
		move_to(tp, tp->tty_column, tp->tty_row);
		break;
	default: break;
    }
  }
  else
  /* Handle a sequence beginning with ESC [ and parameters */
  if (tp->tty_esc_intro == '[') {
    switch (c) {
	case 'H':		/* Position cursor */
		move_to(tp, 
			MAX(1, MIN(LINE_WIDTH, tp->tty_esc_parmv[1])) - 1,
			MAX(1, MIN(SCR_LINES, tp->tty_esc_parmv[0])) - 1 );
		break;
	case 'J':		/* Clear from cursor to end of screen */
		if (tp->tty_esc_parmv[0] == 0) {
			n = 2 * ((SCR_LINES - (tp->tty_row + 1)) * LINE_WIDTH
				+ LINE_WIDTH - (tp->tty_column));
			vx = tp->tty_vid;
			while (n > 0) {
				ct = MIN(n, vid_retrace);
				vid_copy(NIL_PTR, vid_base, vx, ct/2);
				vx += ct;
				n -= ct;
			}
		}
		break;
	case 'm':		/* Set graphic rendition */
 		switch (tp->tty_esc_parmv[0]) {
 			case 1: /*  BOLD  (light green on black)  */
 				tp->tty_attribute = 0x0A << 8;
 				break;
 
 			case 4: /*  UNDERLINE  (blue on red)  */
 				tp->tty_attribute = 0x41 << 8;
 				break;
 
 			case 5: /*  BLINKING  (light grey on black)  */
				tp->tty_attribute = 0x87 << 8;
 				break;
 
 			case 7: /*  REVERSE  (black on light grey)  */
 				tp->tty_attribute = 0x70 << 8;
  				break;

 			default: tp->tty_attribute = 0007 << 8;
 				break;
 		}
		break;
	default:
		break;
    }
  }
  tp->tty_esc_state = 0;
}


/*===========================================================================*
 *				set_6845				     *
 *===========================================================================*/
PRIVATE set_6845(reg, val)
int reg;			/* which register pair to set */
int val;			/* 16-bit value to set it to */
{
/* Set a register pair inside the 6845.  
 * Registers 10-11 control the format of the cursor (how high it is, etc).
 * Registers 12-13 tell the 6845 where in video ram to start (in WORDS)
 * Registers 14-15 tell the 6845 where to put the cursor (in WORDS)
 *
 * Note that registers 12-15 work in words, i.e. 0x0000 is the top left
 * character, but 0x0001 (not 0x0002) is the next character.  This addressing
 * is different from the way the 8088 addresses the video ram, where 0x0002
 * is the address of the next character.
 */
  port_out(vid_port + INDEX, reg);	/* set the index register */
  port_out(vid_port + DATA, (val>>8) & BYTE);	/* output high byte */
  port_out(vid_port + INDEX, reg + 1);	/* again */
  port_out(vid_port + DATA, val&BYTE);	/* output low byte */
}


/*===========================================================================*
 *				beep					     *
 *===========================================================================*/
PRIVATE beep(f)
int f;				/* this value determines beep frequency */
{
/* Making a beeping sound on the speaker (output for CRTL-G).  The beep is
 * kept short, because interrupts must be disabled during beeping, and it
 * is undesirable to keep them off too long.  This routine works by turning
 * on the bits in port B of the 8255 chip that drive the speaker.
 */

  int x, k, s;

  s = lock();			/* disable interrupts */
  port_out(TIMER3,0xB6);	/* set up timer channel 2 mode */
  port_out(TIMER2, f&BYTE);	/* load low-order bits of frequency in timer */
  port_out(TIMER2,(f>>8)&BYTE);	/* now high-order bits of frequency in timer */
  port_in(PORT_B,&x);		/* acquire status of port B */
  port_out(PORT_B, x|3);	/* turn bits 0 and 1 on to beep */
  for (k = 0; k < B_TIME; k++);	/* delay loop while beeper sounding */
  port_out(PORT_B, x);		/* restore port B the way it was */
  restore(s);			/* re-enable interrupts to previous state */
}


/*===========================================================================*
 *				set_leds				     *
 *===========================================================================*/
PRIVATE set_leds()
{
/* Set the LEDs on the caps lock and num lock keys */

  int leds, dummy, i;

  if (pc_at == 0) return;	/* PC/XT doesn't have LEDs */
  leds = (numlock<<1) | (capslock<<2);	/* encode LED bits */
  port_out(KEYBD, LED_CODE);	/* prepare keyboard to accept LED values */
  port_in(KEYBD, &dummy);	/* keyboard sends ack; accept it */
  for (i = 0; i < LED_DELAY; i++) ;	/* delay needed */
  port_out(KEYBD, leds);	/* give keyboard LED values */
  port_in(KEYBD, &dummy);	/* keyboard sends ack; accept it */
}


/*===========================================================================*
 *				tty_init				     *
 *===========================================================================*/
PRIVATE tty_init()
{
/* Initialize the tty tables. */

  register struct tty_struct *tp;

  /* Tell the EGA card, if any, to simulate a 16K CGA card. */
  port_out(EGA + INDEX, 4);	/* register select */
  port_out(EGA + DATA, 1);	/* no extended memory to be used */

  for (tp = &tty_struct[0]; tp < &tty_struct[NR_CONS]; tp++) {
	tp->tty_inhead = tp->tty_inqueue;
	tp->tty_intail = tp->tty_inqueue;
	tp->tty_mode = CRMOD | XTABS | ECHO;
	tp->tty_devstart = console;
	tp->tty_makebreak = TWO_INTS;
	tp->tty_attribute = BLANK;
	tp->tty_erase = ERASE_CHAR;
	tp->tty_kill  = KILL_CHAR;
	tp->tty_intr  = INTR_CHAR;
	tp->tty_quit  = QUIT_CHAR;
	tp->tty_xon   = XON_CHAR;
	tp->tty_xoff  = XOFF_CHAR;
	tp->tty_eof   = EOT_CHAR;
  }

  if (color) {
	vid_base = COLOR_BASE;
	vid_mask = C_VID_MASK;
	vid_port = C_6845;
	vid_retrace = C_RETRACE;
  } else {
	vid_base = MONO_BASE;
	vid_mask = M_VID_MASK;
	vid_port = M_6845;
	vid_retrace = M_RETRACE;
  }
  tty_driver_buf[1] = MAX_OVERRUN;	/* set up limit on keyboard buffering*/
  set_6845(CUR_SIZE, 31);		/* set cursor shape */
  set_6845(VID_ORG, 0);			/* use page 0 of video ram */
  move_to(&tty_struct[0], 0, SCR_LINES-1); /* move cursor to lower left */

  /* Determine which keyboard type is attached.  The bootstrap program asks 
   * the user to type an '='.  The scan codes for '=' differ depending on the
   * keyboard in use.
   */
  if (scan_code == OLIVETTI_EQUAL) olivetti = TRUE;
}


/*===========================================================================*
 *				putc					     *
 *===========================================================================*/
PUBLIC putc(c)
char c;				/* character to print */
{
/* This procedure is used by the version of printf() that is linked with
 * the kernel itself.  The one in the library sends a message to FS, which is
 * not what is needed for printing within the kernel.  This version just queues
 * the character and starts the output.
 */

  out_char(&tty_struct[0], c);
}


/*===========================================================================*
 *				func_key				     *
 *===========================================================================*/
PRIVATE func_key(ch)
char ch;			/* scan code for a function key */
{
/* This procedure traps function keys for debugging purposes.  When MINIX is
 * fully debugged, it should be removed.
 */

  if (ch == F1) p_dmp();	/* print process table */
  if (ch == F2) map_dmp();	/* print memory map */
  if (ch == F3) {		/* hardware vs. software scrolling */
	softscroll = 1 - softscroll;	/* toggle scroll mode */
	tty_struct[0].tty_org = 0;
	move_to(&tty_struct[0], 0, SCR_LINES-1); /* cursor to lower left */
	set_6845(VID_ORG, 0);
	if (softscroll)
		printf("\033[H\033[JSoftware scrolling enabled.\n");
	else
		printf("\033[H\033[JHardware scrolling enabled.\n");
  }

#ifdef AM_KERNEL
#ifndef NONET
  if (ch == F4) net_init();	/* re-initialise the ethernet card */
#endif NONET
#endif AM_KERNEL
  if (ch == F9) sigchar(&tty_struct[0], SIGKILL);	/* issue SIGKILL */
}
#endif

/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************/


#ifdef i8088
/* Now begins the code and data for the device RS232 drivers. */

/* Definitions used by the RS232 driver. */
#define	RS_BUF_SIZE		 256	/* output buffer per serial line */
#define PRIMARY                0x3F8	/* I/O port of primary RS232 */
#define SECONDARY              0x2F8	/* I/O port of secondary RS232 */
#define SPARE                     16	/* leave room in buffer for echoes */
#define THRESHOLD                 20	/* # chars to accumulate before msg */

/* Constants relating to the 8250. */
#define	RS232_RATE_DIVISOR	   0	/* address of baud rate divisor reg */
#define	RS232_TRANSMIT_HOLDING	   0	/* address of transmitter holding reg*/
#define	RS232_RECEIVER_DATA_REG	   0	/* address of receiver data register */
#define	RS232_INTERRUPTS	   1	/* address of interrupt enable reg */
#define	RS232_INTERRUPT_ID_REG	   2	/* address of interrupt id register */
#define	RS232_LINE_CONTROL	   3	/* address of line control register */
#define	RS232_MODEM_CONTROL	   4	/* address of modem control register */
#define	RS232_LINE_STATUS	   5	/* address of line status register */
#define	RS232_MODEM_STATUS	   6	/* address of modem status register */
#define	LINE_CONTROLS		0x0B	/* odd parity,1 stop bit,8 data bits */
#define	MODEM_CONTROLS		0x0B	/* RTS & DTR */
#define	ADDRESS_DIVISOR		0x80	/* value to address divisor */
#define HOLDING_REG_EMPTY       0x20	/* transmitter holding reg empty */
#define	RS232_INTERRUPT_CLASSES	0x03	/* receiver Data Ready & xmt empty */
#define	UART_FREQ  	     115200L	/* UART timer frequency */
#define DEF_BAUD                1200	/* default baud rate */

/* Line control setting related constants. */
#define ODD			   0
#define	EVEN			   1
#define NONE			  -1
#define	PARITY_ON_OFF		0x08	/* position of parity bit in line reg*/
#define	PARITY_TYPE_SHIFT	   4	/* shift count for parity_type bit */
#define	STOP_BITS_SHIFT		   2	/* shift count for # stop_bits */
#define DATA_LEN                   8	/* how much to shift sg_mode for len */

/* RS232 interrupt types. */
#define MODEM_STATUS            0x00	/* UART modem status change */
#define TRANSMITTER_READY	0x02	/* transmitter ready to accept data */
#define RECEIVER_READY		0x04	/* data received interrupt */
#define LINE_STATUS             0x06	/* UART line status change */
#define INT_TYPE_MASK		0x06	/* mask to mask out interrupt type */
#define	INT_PENDING		0x01	/* position of interrupt-pending bit */

/* Status register values. */
#define DATA_REGISTER_EMPTY	0x20	/* mask to see if data reg is empty */
#define DATA_RECEIVED		0x01	/* mask to see if data has arrived */

/* Global variables used by the RS232 driver. */
PUBLIC message rs232_rd_mess;		/* used when chars arrive from tty */
PUBLIC message rs232_wt_mess;		/* used when output to tty done */
PUBLIC int flush_flag;			/* indicates chars in tty_driver_buf */
PRIVATE int first_rs_write_int_seen = FALSE;

PRIVATE struct rs_struct{
  int rs_base;			/* 0x3F8 for primary, 0x2F8 secondary*/
  int rs_busy;			/* line is idle or not */
  int rs_left;			/* # chars left in buffer to output */
  char *rs_next;		/* pointer to next char to output */
  char rs_buf[RS_BUF_SIZE];	/* output buffer */
} rs_struct[NR_RS_LINES];


/*===========================================================================*
 *				rs232				 	     *
 *===========================================================================*/
PUBLIC rs232(unit)
int unit;				/* which unit caused the interrupt */
{
/* When an RS232 interrupt occurs, mpx88.s catches it and calls rs232().
 * Because more than one interrupt condition can occur at the same
 * time, the conditions are presented in the interrupt-identification
 * register in priority order, we have to keep scanning until the 
 * interrupt-pending bit goes down.  Only one communications port is really
 * supported here because the other vector is used by the Ethernet.
 */

  int interrupt_type, t, old_state, val;
  struct rs_struct *rs;

  old_state = lock();
  rs = &rs_struct[unit - NR_CONS];
  while (TRUE) {
	port_in(rs->rs_base + RS232_INTERRUPT_ID_REG, &interrupt_type); 
	if ((interrupt_type & INT_PENDING) == 1) break;	/* 1 = no interrupt */
	t = interrupt_type & INT_TYPE_MASK;
	switch(t) {
	    case RECEIVER_READY:	/* a character has arrived */
		rs_read_int(unit);
		break;

	    case TRANSMITTER_READY:	/* a character has been output */
		rs_write_int(unit);
		break;

	    case LINE_STATUS:		/* line status event, (disabled) */
		port_in(rs_struct[unit-1].rs_base + RS232_LINE_STATUS, &val);
		printf("RS 232 line status event %x\n", val);
		break;

	    case MODEM_STATUS:		/* modem status event, (disabled) */
		port_in(rs_struct[unit-1].rs_base + RS232_MODEM_STATUS, &val);
		printf("RS 232 modem status event %x\n", val);
		break;
	}
  }
  restore(old_state);
}


/*===========================================================================*
 *				rs_read_int			 	     *
 *===========================================================================*/
PRIVATE	rs_read_int(line)
int line;
{
  int val, k, base;

  base = rs_struct[line - NR_CONS].rs_base;

  /* Fetch the character from the RS232 hardware. */
  port_in(base + RS232_RECEIVER_DATA_REG, &val);

  /* Store the character in memory so the task can get at it later */
  if ((k = tty_driver_buf[0]) < tty_driver_buf[1]) {
	/* There is room to store this character, do it */
	k = k + k;			/* each entry contains two bytes */
	tty_driver_buf[k + 2] = val;	/* store the ascii code */
	tty_driver_buf[k + 3] = line;	/* tell wich line it came from */ 
	tty_driver_buf[0]++;		/* increment counter */

	if (tty_driver_buf[0] < THRESHOLD) {
		/* Don't send message.  Just accumulate.  Let clock do it. */
		port_out(INT_CTL, ENABLE);
		flush_flag++;
		return;
	}
	rs_flush();			/* send TTY task a message */
  } else {
	/* Too many character have been buffered. Discard excess */
	port_out(INT_CTL, ENABLE);	/* re-enable 8259A controller */
  }
}


/*===========================================================================*
 *				rs_flush	  		 	     *
 *===========================================================================*/
PUBLIC rs_flush()
{
/* Flush the tty_driver_buf by sending a message to TTY.  This procedure can
 * be triggered locally, when a character arrives, or by the clock task.
 */

  /* Build and send the interrupt message */ 
  flush_flag = 0;
  if (tty_driver_buf[0] == 0) return;	/* nothing to flush */
  rs232_rd_mess.m_type = TTY_CHAR_INT;
  rs232_rd_mess.ADDRESS = tty_driver_buf;
  interrupt(TTY, &rs232_rd_mess);	/* send a message to the tty task */
}


/*===========================================================================*
 *				rs_write_int	  		 	     *
 *===========================================================================*/
PRIVATE	rs_write_int(line)
int line;
{
/* An output ready interrupt has occurred, or a write has been done to an idle
 * line.  In both cases, start the output.
 */

  int val;
  struct tty_struct *tp;
  struct rs_struct *rs;

  if (first_rs_write_int_seen == FALSE) {
	first_rs_write_int_seen = TRUE;
	port_out(INT_CTL, ENABLE);	/* re-enable 8259A controller */
	return;
  }

  /* On the XT and clones, check for spurious write-completed interrupts. */
  rs = &rs_struct[line - NR_CONS];
  port_in(rs->rs_base + RS232_LINE_STATUS, &val);
  if ( (val & HOLDING_REG_EMPTY) == 0) {
	port_out(INT_CTL, ENABLE);	/* re-enable 8259A controller */
	return;
  }

  /* If there are more characters in rs_buf, output the next one. */
  if (rs->rs_left > 0) {
	rs_feed(rs);			/* output the next char in rs_buf */
	port_out(INT_CTL, ENABLE);	/* re-enable 8259A controller */
	return;
  }

  /* The current rs_buf is finished.  See if the complete user buf is done. */
  tp = &tty_struct[line];
  rs->rs_busy = FALSE;
  rs->rs_next = &rs->rs_buf[0];
  if (tp->tty_outleft > 0) {
	serial_out(tp);			/* copy the next chunk to rs_buf */
	port_out(INT_CTL, ENABLE);	/* re-enable 8259A controller */
	return;
  }

  /* The current output buffer is finished.  Send a message to the tty task. */
  if (tp->tty_waiting == WAITING) {
	rs232_wt_mess.m_type = TTY_O_DONE;	/* build the message */
	rs232_wt_mess.ADDRESS = tty_driver_buf;	/* pro forma */
	rs232_wt_mess.TTY_LINE = line;	/* which line is finished */
	tp->tty_waiting = COMPLETED;	/* mark this line as done */
	output_done++;			/* # of RS232 lines now completed */
	interrupt(TTY, &rs232_wt_mess);	/* send the message to the tty task */
  } else {
	port_out(INT_CTL, ENABLE);	/* re-enable 8259A controller */
  }
}


/*===========================================================================*
 *				rs_feed		  		 	     *
 *===========================================================================*/
PRIVATE rs_feed(rs)
struct rs_struct *rs;			/* which line */
{
/* If there is more output queued, output the next character. */

  char byte;

  if (rs->rs_left > 0) {
	byte = *rs->rs_next;
	port_out(rs->rs_base + RS232_TRANSMIT_HOLDING, (int) byte);
	rs->rs_next++;
	rs->rs_left--;			/* one char done */
  }
}


/*===========================================================================*
 *				start_rs232				     * 
 *===========================================================================*/
PRIVATE	start_rs232(tp)
struct tty_struct *tp;			/* which tty */
{
  int old_state;

  old_state = lock();
  serial_out(tp);
  restore(old_state);
}	


/*===========================================================================*
 *				serial_out				     * 
 *===========================================================================*/
PRIVATE	serial_out(tp)
register struct tty_struct *tp;	/* tells which terminal is to be used */
{
/* Copy as much data as possible to the output queue, then start I/O if
 * necessary.
 */
  int bytes, line;
  char c, *limit;
  unsigned short segment, offset, offset1;
  register struct rs_struct *rs;
  extern char get_byte();

  if (tp->tty_inhibited != RUNNING) return;

  line = tp - &tty_struct[0];		/* line is index into tty_struct */
  rs = &rs_struct[line - NR_CONS];	/* 0 to NR_CONS - 1 are consoles */
	
  /* Copy bytes from user space to rs_buf. */
  limit = &rs->rs_buf[RS_BUF_SIZE - SPARE];
  segment = (tp->tty_phys >> 4) & WORD_MASK;
  offset = tp->tty_phys & OFF_MASK;
  offset1 = offset;

  /* While there is still data to output and there is still room in buf, copy*/
  while (tp->tty_outleft > 0 && rs->rs_next + rs->rs_left < limit) {
	c = get_byte(segment, offset);	/* fetch 1 byte */
	offset++;
	tp->tty_outleft--;
	if (c < ' ') {
		rs_expand(tp, rs, c);	/* insert the char in rs_buf */
	} else {
		*(rs->rs_next + rs->rs_left) = c;	/* avoid proc call */
		rs->rs_left++;
		tp->tty_column++;
	}
  }

  bytes = offset - offset1;		/* does not include '\r' or tab exp */
  tp->tty_cum += bytes;			/* update cumulative total */
  tp->tty_phys += bytes;		/* next time, take different bytes */

  if (!rs->rs_busy) {			/* is the line idle? */
	rs->rs_busy = TRUE;		/* if so, mark it as busy */
	rs_feed(rs);			/* and start it going */
  }
}


/*===========================================================================*
 *				rs_out_char				     *
 *===========================================================================*/
PRIVATE rs_out_char(tp, c)
register struct tty_struct *tp;	/* pointer to tty struct */
char c;				/* character to be output */
{
/* Output a character on an RS232 line. */

  int line, old_state;
  register struct rs_struct *rs;

  /* See if there is room to store a character, and if so, do it. */
  old_state = lock();
  line = tp - tty_struct;
  rs = &rs_struct[line - NR_CONS];
  if (rs->rs_next + rs->rs_left == &rs->rs_buf[RS_BUF_SIZE]) return;  /*full */
  rs_expand(tp, rs, c);

  if (!rs->rs_busy) {		/* if terminal line is idle, start it */
	rs->rs_busy = TRUE;
	rs_feed(rs);
  }
  restore(old_state);
}


/*===========================================================================*
 *				rs_expand				     *
 *===========================================================================*/
PRIVATE rs_expand(tp, rs, c)
register struct tty_struct *tp;	/* pointer to tty struct */
register struct rs_struct *rs;	/* pointer to rs struct */
char c;				/* character to be output */
{
/* Some characters output to RS-232 lines need special processing, such as
 * tab expansion and LF to CR+CF mapping.  These things are done here.
 */

  int mode, count, count1;
  char *deposit;		/* where to deposit the next character */

  mode = tp->tty_mode;
  deposit = rs->rs_next + rs->rs_left;

  switch(c) {
	case '\b':
		tp->tty_column -= 2;	/* it is incremented later */
		break;

	case '\r':
		tp->tty_column = -1;	/* it is incremented below */
		break;

	case '\n':
		/* Check to see if LF has to be mapped to CR + LF. */
		if (mode & CRMOD) {
			*deposit++ = '\r';
			rs->rs_left++;
			tp->tty_column = -1;
		}
		break;

	case '\t':
	 	count = 8 - (tp->tty_column % 8);	/* # spaces */
		count1 = count;
		if ((mode & XTABS) == XTABS) {
			/* Tabs must be expanded. */
			while (count1--) *deposit++ = ' ';
			rs->rs_left += count;
			tp->tty_column += count;
			return;
		} else {
			/* Tabs are sent to the terminal hardware. */
			tp->tty_column += count - 1;
		}
  }

  /* Output character and update counters. */
  *deposit = c;			/* copy character to rs_buf */
  rs->rs_left++;		/* there is one more character to print */
  tp->tty_column++;		/* update column */
}


/*===========================================================================*
 *				tty_o_done				     *
 *===========================================================================*/
PRIVATE int tty_o_done()
{
/* A write request on an RS232 line has completed.  Send FS a message. */

  int replyee, caller, old_state;
  struct tty_struct *tp;

  /* See if any of the RS232 lines are complete.  Send at most one message. */
  old_state = lock();
  for (tp = &tty_struct[NR_CONS]; tp < &tty_struct[NR_CONS+NR_RS_LINES]; tp++){
	if (tp->tty_waiting == COMPLETED) {
		replyee = (int) tp->tty_otcaller;
		caller = (int) tp->tty_outproc;
		tty_reply(REVIVE, replyee, caller, tp->tty_cum, 0L, 0L);
		tp->tty_waiting = NOT_WAITING;
		output_done--;
		restore(old_state);
		return(1);
	}
  }
  restore(old_state);
  return(0);
}

/*===========================================================================*
 *				rs_sig					     * 
 *===========================================================================*/
PRIVATE rs_sig(tp)
struct tty_struct *tp;
{
/* Called when a DEL character is typed.  It resets the output. */

  int line;
  struct rs_struct *rs;

  line = tp - tty_struct;
  rs = &rs_struct[line - NR_CONS];
  rs->rs_left = 0;
  rs->rs_busy = 0;
  rs->rs_next = &rs->rs_buf[0];
}


/*===========================================================================*
 *				init_rs232				     * 
 *===========================================================================*/
PRIVATE	init_rs232()
{
  register struct tty_struct *tp;
  register struct rs_struct *rs;
  int line;

  for (tp = &tty_struct[NR_CONS]; tp < &tty_struct[NR_CONS+NR_RS_LINES]; tp++){
	tp->tty_inhead = tp->tty_inqueue;
	tp->tty_intail = tp->tty_inqueue;
	tp->tty_mode = CRMOD | XTABS | ECHO;
	tp->tty_devstart = start_rs232;
	tp->tty_erase	= ERASE_CHAR;
	tp->tty_kill	= KILL_CHAR;
	tp->tty_intr	= INTR_CHAR;
	tp->tty_quit	= QUIT_CHAR;
	tp->tty_xon	= XON_CHAR;
	tp->tty_xoff	= XOFF_CHAR;
	tp->tty_eof	= EOT_CHAR;
	tp->tty_makebreak = ONE_INT;	/* RS232 only interrupts once/char */
  }

  rs_struct[0].rs_base = PRIMARY;
  rs_struct[1].rs_base = SECONDARY;

  for (rs = &rs_struct[0]; rs < &rs_struct[NR_RS_LINES]; rs++) {
	line = rs - rs_struct + NR_CONS;
	rs->rs_next = & rs->rs_buf[0];
	rs->rs_left = 0;
	rs->rs_busy = FALSE;
	config_rs232(line, DEF_BAUD, DEF_BAUD, NONE, 1, 8);    /* set params */
	port_out(rs->rs_base + RS232_MODEM_CONTROL, MODEM_CONTROLS);
	port_out(rs->rs_base + RS232_INTERRUPTS, RS232_INTERRUPT_CLASSES);
  }
}


/*===========================================================================*
 *				set_uart			 	     *
 *===========================================================================*/
PRIVATE set_uart(line, mode, speeds)
int line;			/* which line number (>= NR_CONS) */
int mode;			/* sgtty.h sg_mode word */
int speeds;			/* low byte is input speed, next is output */
{
/* Set the UART parameters. */
  int in_baud, out_baud, parity, stop_bits, data_bits;

  in_baud = 100 * (speeds & BYTE);
  if (in_baud == 100) in_baud = 110;
  out_baud = 100 * ((speeds >> 8) & BYTE);
  if (out_baud == 100) out_baud = 110;
  parity = NONE;
  if (mode & ODDP) parity = ODD;
  if (mode & EVENP) parity = EVEN;
  stop_bits = (in_baud == 110 ? 2 : 1);		/* not quite cricket */
  data_bits = 5 + ((mode >> DATA_LEN) & 03);
  config_rs232(line, in_baud, out_baud, parity, stop_bits, data_bits);
}


/*===========================================================================*
 *				config_rs232			 	     *
 *===========================================================================*/
PRIVATE	config_rs232(line, in_baud, out_baud, parity, stop_bits, data_bits)
int line;			/* which tty */
int in_baud;			/* input speed: 110, 300, 1200, etc. */
int out_baud;			/* output speed: 110, 300, 1200, etc. */
int parity;			/* EVEN, ODD, or NONE */
int stop_bits;			/* 2 (110 baud) or 1 (other speeds) */
int data_bits;			/* 5, 6, 7, or 8 */
{
/* Set various line control parameters for RS232 I/O.
 * If DataBits == 5 and StopBits == 2, UART will generate 1.5 stop bits
 * The 8250 can't handle split speed, but we have propagated both speeds
 * anyway for the benefit of future UART chips.
 */

  int line_controls = 0, base, freq;

  base = rs_struct[line - NR_CONS].rs_base;

  /* First tell line control register to address baud rate divisor */
  port_out(base + RS232_LINE_CONTROL, ADDRESS_DIVISOR);

  /* Now set the baud rate. */
  if (in_baud < 50) in_baud = DEF_BAUD;		/* prevent divide overflow */
  if (out_baud < 50) out_baud = DEF_BAUD;	/* prevent divide overflow */
  freq = (int) (UART_FREQ / in_baud);		/* UART can't hack 2 speeds  */
  port_out(base + RS232_RATE_DIVISOR, freq & BYTE);
  port_out(base + RS232_RATE_DIVISOR+1, (freq >> 8) & BYTE);
  tty_struct[line].tty_speed = ((out_baud/100) << 8) | (in_baud/100);

  /* Put parity_type bits in line_controls */
  if (parity != NONE) {
	line_controls |= PARITY_ON_OFF;
	line_controls |= (parity << PARITY_TYPE_SHIFT);
  }

  /* Put #stop_bits bits in line_controls */
  if (stop_bits == 1 || stop_bits == 2)
	line_controls |= (stop_bits - 1) << STOP_BITS_SHIFT;

  /* Put #data_bits bits in line_controls */
  if (data_bits >=5 && data_bits <= 8)
	line_controls |= (data_bits - 5);

  port_out(base + RS232_LINE_CONTROL, line_controls);
}

#endif
