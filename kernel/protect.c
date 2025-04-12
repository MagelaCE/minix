/* This file contains most of the static initialization for protected mode
 * (some is done in mpx286.x), and routines to initialize the code and data
 * segment descriptors for new processes.
 */

#include "../h/const.h"
#include "../h/type.h"
#include "const.h"
#include "type.h"
#include "glo.h"
#include "proc.h"
#include "protect.h"

struct desctableptr_s {
  u16_t limit;
  u32_t base;			/* really u24_t + pad for 286 */
};

struct gatedesc_s {
  u16_t offset_low;
  u16_t selector;
  u8_t pad;			/* |000|XXXXX| ig & trpg, |XXXXXXXX| task g */
  u8_t p_dpl_type;		/* |P|DL|0|TYPE| */
  u16_t reserved;
};

struct tss286_s {
  u16_t backlink;
  u16_t sp0;
  u16_t ss0;
  u16_t sp1;
  u16_t ss1;
  u16_t sp2;
  u16_t ss2;
  u16_t ip;
  u16_t flags;
  u16_t ax;
  u16_t cx;
  u16_t dx;
  u16_t bx;
  u16_t sp;
  u16_t bp;
  u16_t si;
  u16_t di;
  u16_t es;
  u16_t cs;
  u16_t ss;
  u16_t ds;
  u16_t ldt;
};

union tss_u {
  struct tss286_s t2;
};

/* PRIVATE int gdt_pad = 0;	/* attempt to align gdt on long boundary */
PUBLIC struct segdesc_s gdt[GDT_SIZE] = { 0, };
				/* temporary initialize it to get it in data
				 * segment for better control over alignment
				 */
PRIVATE struct gatedesc_s idt[IDT_SIZE];  /* zero-init so none present */
PUBLIC union tss_u tss;		/* zero init */

#ifdef DEBUGGER
#define IF 0x0200
PUBLIC union tss_u db_tss;	/* zero init */
extern char db_stacktop[];
#endif

extern void divide_error();
extern void debug2_exception();
extern void debug3_exception();
extern void nmi();
extern void breakpoint();
extern void overflow();
extern void bounds_check();
extern void inval_opcode();
extern void copr_not_available();
extern void double_fault();
extern void copr_seg_overrun();
extern void inval_tss();
extern void segment_not_present();
extern void stack_exception();
extern void general_protection();
extern void page_fault();
extern void copr_error();

extern void clock_int();
extern void tty_int();
extern void psecondary_int();
extern void prs232_int();
extern void disk_int();
extern void lpr_int();
extern void wini_int();

extern void p2_s_call();
extern void p3_s_call();

FORWARD void sdesc();
FORWARD void task_gate();

PRIVATE void int_gate( vec_nr, base, dpl_type )
unsigned vec_nr;
offset_t base;
unsigned dpl_type;
{
  /* Build descriptor for an interrupt gate. */

  register struct gatedesc_s *idp;

  idp = &idt[vec_nr];
  idp->offset_low = base;
  idp->selector = CS_SELECTOR;
  idp->p_dpl_type = dpl_type;
}

PUBLIC void init_codeseg( segdp, base, size, privilege )
register struct segdesc_s *segdp;
offset_t base;
offset_t size;
int privilege;
{
  /* Build descriptor for a code segment. */

  sdesc( segdp, base, size );
  segdp->access = (privilege << DPL_SHIFT)
		| (PRESENT | SEGMENT | EXECUTABLE | READABLE);
		/* CONFORMING = 0, ACCESSED = 0 */
}

PUBLIC void init_dataseg( segdp, base, size, privilege )
register struct segdesc_s *segdp;
offset_t base;
offset_t size;
int privilege;
{
  /* Build descriptor for a data segment. */

  sdesc( segdp, base, size );
  segdp->access = (privilege << DPL_SHIFT) | (PRESENT | SEGMENT | WRITEABLE);
		/* EXECUTABLE = 0, EXPAND_DOWN = 0, ACCESSED = 0 */
}

PUBLIC void prot_init()
{
  /* Set up tables for protected mode, even if they will not be used.
   * All GDT slots are allocated at compile time.
   */

  offset_t codebase;
  offset_t code_bytes;
  offset_t database;
  offset_t data_bytes;
  int gate_nr;
  u8_t type3_bit;
  unsigned ldt_selector;
  register struct proc *rp;

  static struct
  {
    unsigned vec_nr;
    void (*gate)();
  }
  gate_table[] =
  {
    DIVIDE_VECTOR, divide_error,
    NMI_VECTOR, nmi,
    OVERFLOW_VECTOR, overflow,
    BOUNDS_VECTOR, bounds_check,
    INVAL_OP_VECTOR, inval_opcode,
    COPROC_NOT_VECTOR, copr_not_available,
    DOUBLE_FAULT_VECTOR, double_fault,
    COPROC_SEG_VECTOR, copr_seg_overrun,
    INVAL_TSS_VECTOR, inval_tss,
    SEG_NOT_VECTOR, segment_not_present,
    STACK_FAULT_VECTOR, stack_exception,
    PROTECTION_VECTOR, general_protection,
    IRQ0_VECTOR + CLOCK_IRQ, clock_int,
    IRQ0_VECTOR + KEYBOARD_IRQ, tty_int,
    IRQ0_VECTOR + SECONDARY_IRQ, psecondary_int,
    IRQ0_VECTOR + RS232_IRQ, prs232_int,
    IRQ0_VECTOR + FLOPPY_IRQ, disk_int,
    IRQ0_VECTOR + PRINTER_IRQ, lpr_int,
    IRQ8_VECTOR + AT_WINI_IRQ - 8, wini_int,
  };

  /* This is called early and can't use tables set up by main(). */
  codebase = (offset_t) codeseg() << HCLICK_SHIFT;
  database = (offset_t) dataseg() << HCLICK_SHIFT;
  data_bytes = (offset_t) sizes[1] << CLICK_SHIFT;
  if (sizes[0] == 0)
    code_bytes = data_bytes;	/* common I&D */
  else
    code_bytes = (offset_t) sizes[0] << CLICK_SHIFT;

  /* Build temporary gdt and idt pointers in GDT where BIOS needs them. */
  ((struct desctableptr_s *) &gdt[GDT_INDEX])->limit = sizeof gdt - 1;
  ((struct desctableptr_s *) &gdt[GDT_INDEX])->base =
					       database + (offset_t) gdt;
  ((struct desctableptr_s *) &gdt[IDT_INDEX])->limit = sizeof idt - 1;
  ((struct desctableptr_s *) &gdt[IDT_INDEX])->base =
					       database + (offset_t) idt;

  /* Build segment descriptors for tasks and interrupt handlers. */
  init_dataseg( &gdt[DS_INDEX], database, data_bytes, INTR_PRIVILEGE );
  init_dataseg( &gdt[ES_INDEX], database, data_bytes, INTR_PRIVILEGE );
  init_dataseg( &gdt[SS_INDEX], database, data_bytes, INTR_PRIVILEGE );
  init_codeseg( &gdt[CS_INDEX], codebase, code_bytes, INTR_PRIVILEGE );

  /* Build scratch descriptors for functions in klib286. */
  init_dataseg( &gdt[DS_286_INDEX], (offset_t) 0, (offset_t) MAX_286_SEG_SIZE,
		TASK_PRIVILEGE );
  init_dataseg( &gdt[ES_286_INDEX], (offset_t) 0, (offset_t) MAX_286_SEG_SIZE,
		TASK_PRIVILEGE );

  /* Build descriptors for video segments. */
  init_dataseg( &gdt[COLOR_INDEX], (offset_t) COLOR_BASE,
		(offset_t) COLOR_SIZE, TASK_PRIVILEGE );
  init_dataseg( &gdt[MONO_INDEX], (offset_t) MONO_BASE,
		(offset_t) MONO_SIZE, TASK_PRIVILEGE );

  /* Build segment descriptors for switching to real mode.
   * They are only used by the debugger, which only works on 386's.
   */
  init_codeseg( &gdt[REAL_CS_INDEX], codebase, (offset_t) REAL_SEG_SIZE,
		INTR_PRIVILEGE );
  init_dataseg( &gdt[REAL_DS_INDEX], database, (offset_t) REAL_SEG_SIZE,
		INTR_PRIVILEGE );

  /* Build main TSS.  This is used only to record the stack pointer to
   * be used after an interrupt.  This pointer is set up so that an
   * interrupt automatically saves the current process' registers
   * eip:cs:ef:esp:ss in the correct slots in the process table.
   */
  tss.t2.ss0 = DS_SELECTOR;
  type3_bit = 0;
  init_dataseg( &gdt[TSS_INDEX], database + (offset_t) &tss,
		(offset_t) sizeof tss, INTR_PRIVILEGE );
  gdt[TSS_INDEX].access = PRESENT | (INTR_PRIVILEGE << DPL_SHIFT)
			  | AVL_286_TSS | type3_bit;

#ifdef DEBUGGER
  /* Build TSS and task gates for debugger. */
  init_dataseg( &gdt[DB_TSS_INDEX], database + (offset_t) &db_tss,
		(offset_t) sizeof db_tss, INTR_PRIVILEGE );
  gdt[DB_TSS_INDEX].access = PRESENT | (INTR_PRIVILEGE << DPL_SHIFT)
			     | AVL_286_TSS | type3_bit;
  task_gate( &idt[DEBUG_VECTOR], DB_TSS_SELECTOR, USER_PRIVILEGE );
  task_gate( &idt[BREAKPOINT_VECTOR], DB_TSS_SELECTOR, USER_PRIVILEGE );
  db_tss.t2.cs = CS_SELECTOR;
  db_tss.t2.ip = (u16_t) debug2_exception;
  db_tss.t2.flags = INIT_PSW & ~IF;
  db_tss.t2.ds =
  db_tss.t2.es =
  db_tss.t2.ss = DS_SELECTOR;
  db_tss.t2.sp = (u16_t) db_stacktop;
#endif

  /* Build local descriptors in GDT for LDT's in process table.
   * The LDT's are allocated at compile time in the process table, and
   * initialized whenever a process' map is initialized or changed.
   */
  for ( rp = BEG_PROC_ADDR, ldt_selector = FIRST_LDT_INDEX * DESC_SIZE;
	rp < END_PROC_ADDR; ++rp, ldt_selector += DESC_SIZE )
  {
    init_dataseg( &gdt[ldt_selector / DESC_SIZE],
		  database + (offset_t) rp->p_ldt,
		  (offset_t) sizeof rp->p_ldt, INTR_PRIVILEGE );
    gdt[ldt_selector / DESC_SIZE].access = PRESENT | LDT;
    rp->p_ldt_sel = ldt_selector;
  }

  /* Build descriptors for interrupt gates in IDT. */
  for ( gate_nr = 0; gate_nr < sizeof gate_table / sizeof gate_table[0];
	++gate_nr )
  {
    int_gate( gate_table[gate_nr].vec_nr, (offset_t) gate_table[gate_nr].gate,
	      PRESENT | (INTR_PRIVILEGE << DPL_SHIFT) |
	      INT_286_GATE | type3_bit );
  }
  int_gate( SYS_VECTOR, (offset_t)
	    p2_s_call,
	    PRESENT | (USER_PRIVILEGE << DPL_SHIFT) |
	    INT_286_GATE | type3_bit );
}

PRIVATE void sdesc( segdp, base, size )
register struct segdesc_s *segdp;
offset_t base;
offset_t size;
{
  /* Fill in size fields (base and limit) of a descriptor. */

  segdp->base_low = base;
  segdp->base_middle = base >> BASE_MIDDLE_SHIFT;
  segdp->limit_low = size - 1;
}

PRIVATE void task_gate( gatedp, selector, privilege )
register struct gatedesc_s *gatedp;
unsigned selector;
int privilege;
{
  /* Build descriptor for a task gate. */

  gatedp->selector = selector;
  gatedp->p_dpl_type = (privilege << DPL_SHIFT) | PRESENT | TASK_GATE;
		     /* SEGMENT = 0 */
}
