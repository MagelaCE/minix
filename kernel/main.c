/* This file contains the main program of MINIX.  The routine main()
 * initializes the system and starts the ball rolling by setting up the proc
 * table, interrupt vectors, and scheduling each task to run to initialize
 * itself.
 * 
 * The entries into this file are:
 *   main:		MINIX main program
 *   panic:		abort MINIX due to a fatal error
 */

#include "../h/const.h"
#include "../h/type.h"
#include "../h/boot.h"
#include "../h/callnr.h"
#include "../h/com.h"
#include "../h/error.h"
#include "../h/signal.h"
#include "const.h"
#include "type.h"
#include "glo.h"
#include "proc.h"

#define CLOCK_MASK	(1 << (CLOCK_IRQ & 0x07))
#define CASCADE_MASK	(1 << (CASCADE_IRQ & 0x07))
#define XT_WINI_MASK	(1 << (XT_WINI_IRQ & 0x07))
#define FLOPPY_MASK	(1 << (FLOPPY_IRQ & 0x07))
#define PRINTER_MASK	(1 << (PRINTER_IRQ & 0x07))
#define AT_WINI_MASK	(1 << (AT_WINI_IRQ & 0x07))

#define SAFETY            32	/* safety margin for stack overflow (bytes) */
#define BASE            1536	/* address where MINIX starts in memory */
#define SIZES              8	/* sizes array has 8 entries */
#define CPU_TY1       0xFFFF	/* BIOS segment that tells CPU type */
#define CPU_TY2       0x000E	/* BIOS offset that tells CPU type */
#define PC_AT           0xFC	/* IBM code for PC-AT (in BIOS at 0xFFFFE) */
#define PS              0xFA	/* IBM code for PS/2  (in BIOS at 0xFFFFE) */
#define CMASK4          0x9E	/* Planar Control Register */
#define HIGH_INT          17	/* limit of the interrupt vectors */

extern int int00(), int01(), int02(), int03(), int04(), int05(), int06(), 
           int07(), int08(), int09(), int10(), int11(), int12(), int13(), 
           int14(), int15(), int16();
int (*int_vec[HIGH_INT])() = {int00, int01, int02, int03, int04, int05, int06,
    int07, int08, int09, int10, int11, int12, int13, int14, int15, int16};


/*===========================================================================*
 *                                   main                                    * 
 *===========================================================================*/
PUBLIC main()
{
/* Start the ball rolling. */

  register struct proc *rp;
  register int t;
  vir_clicks size;
  phys_clicks base_click, mm_base, previous_base;
  phys_bytes phys_b;
  int	stack_size;
  offset_t ktsb;		/* kernel task stack base */
  extern unsigned sizes[8];	/* table filled in by build */
  extern int port_65, ega, color, vec_table[], get_chrome();
  extern int s_call(), disk_int(), tty_int(), clock_int(), disk_int();
  extern int wini_int(), lpr_int(), trp(), rs232_int(), secondary_int();
  extern phys_bytes umap();
  extern struct tasktab tasktab[];	/* see table.c */
  extern char t_stack[];		/* see table.c */

#ifdef AM_KERNEL
#ifndef NONET
  extern int eth_int();
#endif
#endif

  /* Set up proc table entry for user processes.  Be very careful about
   * sp, since the 3 words prior to it will be clobbered when the kernel pushes
   * pc, cs, and psw onto the USER's stack when starting the user the first
   * time.  And if an interrupt happens before the user loads a better stack
   * pointer, these 3 words will be used to save the state, and the interrupt
   * handler will use another 3, and a debugger trap may use another 3. This
   * means that with INIT_SP == 0x1C, user programs must leave the memory
   * between 0x000A and 0x001B free.
   */

  /* Interrupts are disabled here (by fsck.s and then redundantly by mpx88.x),
   * so it is unnecessary to call lock().  They are reenabled when INIT_PSW is
   * loaded by the first restart().
   */

#ifdef DEBUGGER
#ifdef DEBUG_EARLY
  db();
#endif
#endif

  /* Mask all interrupts.  BIOS calls will reenable interrupts, which hurts
   * if someone has hooked the vectors and Minix is copied over the handlers.
   */
  port_out(INT_CTLMASK, ~0);

  proc_init(); 
  processor = get_processor();
  if (processor > boot_parameters.bp_processor)
	processor = boot_parameters.bp_processor;
#ifdef i80286
  prot_init();
#endif

  /* Align stack base suitably. */
  ktsb = ((offset_t) t_stack + (ALIGNMENT - 1)) & ~((offset_t) ALIGNMENT - 1);

  base_click = BASE >> CLICK_SHIFT;
  size = sizes[0] + sizes[1];	/* kernel text + data size in clicks */
  mm_base = base_click + size;	/* place where MM starts (in clicks) */

  for (rp = BEG_PROC_ADDR; rp <= BEG_USER_ADDR; ++rp, ++t)
  {
	t = proc_number(rp);

	if (t < 0) {
		stack_size = tasktab[t+NR_TASKS].stksize;
		rp->p_splimit = ktsb + SAFETY;
		ktsb += stack_size;
		rp->p_reg.r16.sp = ktsb;
	} else {
		rp->p_reg.r16.sp = INIT_SP;
		rp->p_splimit = rp->p_reg.r16.sp;
	}

	rp->p_reg.r16.pc = (u16_t) tasktab[t + NR_TASKS].initial_pc;
	if (!isidlehardware(t)) lockready(rp);	/* IDLE, HARDWARE neveready */
	rp->p_reg.r16.psw = istaskp(rp) ? INIT_TASK_PSW : INIT_PSW;
	rp->p_flags = 0;

	/* Set up memory map for tasks and MM, FS, INIT. */
	if (t < 0) {
		/* I/O tasks. */
		rp->p_map[T].mem_len  = sizes[0];
		rp->p_map[T].mem_phys = base_click;
		rp->p_map[D].mem_len  = sizes[1];
		rp->p_map[D].mem_phys = base_click + sizes[0];
		rp->p_map[S].mem_phys = base_click + sizes[0] + sizes[1];
		rp->p_map[S].mem_vir = sizes[1];
	} else {
		/* MM, FS, and INIT. */
		previous_base = proc_addr(t - 1)->p_map[S].mem_phys;
		rp->p_map[T].mem_len  = sizes[2*t + 2];
		rp->p_map[T].mem_phys = (t == 0 ? mm_base : previous_base);
		rp->p_map[D].mem_len  = sizes[2*t + 3];
		rp->p_map[D].mem_phys = rp->p_map[T].mem_phys + sizes[2*t + 2];
		rp->p_map[S].mem_vir  = sizes[2*t + 3];
		rp->p_map[S].mem_phys = rp->p_map[D].mem_phys + sizes[2*t + 3];
	}


#ifdef i8088
	alloc_segments(rp);
#endif
  }

  /* Determine if display is color or monochrome and CPU type (from BIOS). */
  color = get_chrome();		/* 0 = mono, 1 = color */
  ega = get_ega();
  t = get_phys_byte(((phys_bytes) CPU_TY1 << HCLICK_SHIFT) + CPU_TY2);
  if (t == PC_AT) pc_at = TRUE;
  else if (t == PS) ps = TRUE;

  /* Get memory sizes from the BIOS. */
  mem_init();

  /* Save the old interrupt vectors. */
  phys_b = umap(proc_addr(SYSTASK), D, (vir_bytes) vec_table, VECTOR_BYTES);
  phys_copy(0L, phys_b, (long) VECTOR_BYTES);	/* save all the vectors */
  if (ps) save_tty_vec();	/* save tty vector 0x71 for reboot() */

  /* Set up the new interrupt vectors. */
  for (t = 0; t < HIGH_INT; t++) set_vec(t, int_vec[t], base_click);
  for (t = HIGH_INT; t < 256; t++) set_vec(t, trp, base_click);

  set_vec(SYS_VECTOR, s_call, base_click);
  set_vec(CLOCK_VECTOR, clock_int, base_click);
  set_vec(KEYBOARD_VECTOR, tty_int, base_click);
  set_vec(SECONDARY_VECTOR, secondary_int, base_click);
  set_vec(RS232_VECTOR, rs232_int, base_click);
  set_vec(FLOPPY_VECTOR, disk_int, base_click);
  set_vec(PRINTER_VECTOR, lpr_int, base_click);
#ifdef AM_KERNEL
#ifndef NONET
  set_vec(ETHER_VECTOR, eth_int, base_click);	/* overwrites RS232 port 2 */
#endif
#endif
  if (pc_at) {
	set_vec(AT_WINI_VECTOR, wini_int, base_click);
  } else
	set_vec(XT_WINI_VECTOR, wini_int, base_click);

  if (ps)		/* PS/2 */
	set_vec(PS_KEYB_VECTOR, tty_int, base_click);

  /* Put a ptr to proc table in a known place so it can be found in /dev/mem */
  set_vec( (BASE - 4)/4, proc, (phys_clicks) 0);
  
  bill_ptr = cproc_addr(IDLE);  /* it has to point somewhere */
  lockpick_proc();

#ifdef DEBUGGER
  dbinit();
#endif

  /* Patch assembler modules. */
#ifdef i80286
  if (processor >= 286)
	klib286_init();
#endif

  /* Finish initializing 8259 (needs pc_at). */
  init_8259(IRQ0_VECTOR, IRQ8_VECTOR);

  /* Unmask interrupts for clock, floppy, printer, hard disk.  The other
   * devices are (correctly) unmasked _after_ they initialize themselves.
   * The ethernet driver will need to unmask itself when it is fixed to
   * work with this kernel.
   */
  if (ps) {
	port_in(PCR, &port_65);		/* save Planar Control Register */
	port_out(0x65, CMASK4);		/* set Planar Control Register */
	port_out(INT_CTLMASK,
		 ~(CLOCK_MASK | XT_WINI_MASK | FLOPPY_MASK | PRINTER_MASK));
  } else if (pc_at) {
	port_out(INT_CTLMASK,
		 ~(CLOCK_MASK | CASCADE_MASK | FLOPPY_MASK | PRINTER_MASK));
	port_out(INT2_MASK, ~AT_WINI_MASK);
  } else
	port_out(INT_CTLMASK,
		 ~(CLOCK_MASK | XT_WINI_MASK | FLOPPY_MASK | PRINTER_MASK));

  /* Now go to the assembly code to start running the current process. */
  restart();
}


/*===========================================================================*
 *                                   panic                                   * 
 *===========================================================================*/
PUBLIC panic(s,n)
char *s;
int n; 
{
/* The system has run aground of a fatal error.  Terminate execution.
 * If the panic originated in MM or FS, the string will be empty and the
 * file system already syncked.  If the panic originates in the kernel, we are
 * kind of stuck. 
 */

  if (*s != 0) {
	printf("\r\nKernel panic: %s",s); 
	if (n != NO_NUM) printf(" %d", n);
	printf("\r\n");
  }
  printf("Type any key to reboot\r\n");
#ifdef DEBUGGER
  db();
#endif
  wreboot();
}

#ifdef i8088
/*===========================================================================*
 *                                   set_vec                                 * 
 *===========================================================================*/
PRIVATE set_vec(vec_nr, addr, base_click)
int vec_nr;			/* which vector */
int (*addr)();			/* where to start */
phys_clicks base_click;		/* click where kernel sits in memory */
{
/* Set up an interrupt vector. */

  unsigned vec[2];
  phys_bytes phys_b;

  /* Build the vector in the array 'vec'. */
  vec[0] = (unsigned) addr;
  vec[1] = (unsigned) click_to_hclick(base_click);

  /* Copy the vector into place. */
  phys_b = umap(cproc_addr(SYSTASK), D, (vir_bytes) vec, 4);
  phys_copy(phys_b, (phys_bytes) vec_nr*4, (phys_bytes) 4);
}
#endif

/*===========================================================================*
 *                                   networking                              * 
 *===========================================================================*/
#ifndef AM_KERNEL
/* These routines are dummies.  They are only needed when networking is
 * disabled.  They are called in mpx88.s and klib88.s.
 */
PUBLIC eth_stp() {}			/* stop the ethernet upon reboot */
PUBLIC dp8390_int(){}			/* Ethernet interrupt */
#endif
