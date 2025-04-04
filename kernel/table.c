/* The object file of "table.c" contains all the data.  In the *.h files, 
 * declared variables appear with EXTERN in front of them, as in
 *
 *    EXTERN int x;
 *
 * Normally EXTERN is defined as extern, so when they are included in another
 * file, no storage is allocated.  If the EXTERN were not present, but just
 * say,
 *
 *    int x;
 *
 * then including this file in several source files would cause 'x' to be
 * declared several times.  While some linkers accept this, others do not,
 * so they are declared extern when included normally.  However, it must
 * be declared for real somewhere.  That is done here, by redefining
 * EXTERN as the null string, so the inclusion of all the *.h files in
 * table.c actually generates storage for them.  All the initialized
 * variables are also declared here, since
 *
 * extern int x = 4;
 *
 * is not allowed.  If such variables are shared, they must also be declared
 * in one of the *.h files without the initialization.
 */

#include "../h/const.h"
#include "../h/type.h"
#include "../h/com.h"
#include "const.h"
#include "type.h"
#undef   EXTERN
#define  EXTERN
#include "glo.h"
#include "proc.h"
#include "tty.h"

extern int sys_task(), clock_task(), mem_task(), floppy_task(),
           winchester_task(), tty_task(), printer_task();
#ifdef AM_KERNEL
extern int amoeba_task();
extern int amint_task();
#endif

/* The startup routine of each task is given below, from -NR_TASKS upwards.
 * The order of the names here MUST agree with the numerical values assigned to
 * the tasks in ../h/com.h.
 */
#define	SMALL_STACK	256

#define	TTY_STACK	SMALL_STACK
#define	PRINTER_STACK	SMALL_STACK
#define	WINCH_STACK	SMALL_STACK
#define	FLOP_STACK	SMALL_STACK
#define	MEM_STACK	SMALL_STACK
#define	CLOCK_STACK	SMALL_STACK
#define	SYS_STACK	SMALL_STACK



#ifdef AM_KERNEL
#	define	AMINT_STACK		SMALL_STACK
#	define	AMOEBA_STACK		1532
#	define	AMOEBA_STACK_SPACE	(AM_NTASKS*AMOEBA_STACK + AMINT_STACK)
#else
#	define	AMOEBA_STACK_SPACE	0
#endif

#define	TOT_STACK_SPACE		(TTY_STACK + AMOEBA_STACK_SPACE + \
				 PRINTER_STACK + WINCH_STACK + FLOP_STACK + \
				 MEM_STACK + CLOCK_STACK + SYS_STACK)

/*
** some notes about the following table:
**  1) The tty_task should always be first so that other tasks can use printf
**     if their initialisation has problems.
**  2) If you add a new kernel task, add it after the amoeba_tasks and before
**     the printer task.
**  3) The task name is used for process status (F1 key) and must be six (6)
**     characters in length.  Pad it with blanks if it is too short.
*/

PUBLIC struct tasktab tasktab[] = {
	tty_task,		TTY_STACK,	"TTY   ",
#ifdef AM_KERNEL
	amint_task,		AMINT_STACK,	"AMINT ",
	amoeba_task,		AMOEBA_STACK,	"AMTASK",
	amoeba_task,		AMOEBA_STACK,	"AMTASK",
	amoeba_task,		AMOEBA_STACK,	"AMTASK",
	amoeba_task,		AMOEBA_STACK,	"AMTASK",
#endif
	printer_task,		PRINTER_STACK,	"PRINTR",
	winchester_task,	WINCH_STACK,	"WINCHE",
	floppy_task,		FLOP_STACK,	"FLOPPY",
	mem_task,		MEM_STACK,	"RAMDSK",
	clock_task,		CLOCK_STACK,	"CLOCK ",
	sys_task,		SYS_STACK,	"SYS   ",
	0,			0,		"IDLE  ",
	0,			0,		"MM    ",
	0,			0,		"FS    ",
	0,			0,		"INIT  "
};

int t_stack[TOT_STACK_SPACE/sizeof (int)];

int k_stack[K_STACK_BYTES/sizeof (int)];	/* The kernel stack. */


/*
** The number of kernel tasks must be the same as NR_TASKS.
** If NR_TASKS is not correct then you will get the compile error:
**   multiple case entry for value 0
** The function ___dummy is never called.
*/

#define NKT (sizeof tasktab / sizeof (struct tasktab) - (INIT_PROC_NR + 1))
___dummy()
{
	switch(0)
	{
	case 0:
	case (NR_TASKS == NKT):
		;
	}
}
