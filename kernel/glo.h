/* EXTERN is defined as extern except in table.c */
#ifdef _TABLE
#undef EXTERN
#define EXTERN
#endif

/* Global variables used in the kernel. */

/* Signals. */
EXTERN int sig_procs;		/* number of procs with p_pending != 0 */

/* Machine type. */
EXTERN int pc_at;		/* PC-AT compatible hardware interface */
EXTERN int ps;			/* PS/2 */
EXTERN int ps_mca;              /* PS/2 with Micro Channel */
EXTERN int port_65;		/* saved contents of Planar Control Register */
EXTERN unsigned processor;	/* 86, 186, 286, 386, ... */
EXTERN int protected_mode;	/* nonzero if running in Intel protected mode*/
extern int using_bios;		/* nonzero to force real mode (for bios_wini)*/

/* Video cards and keyboard types. */
EXTERN int color;		/* nonzero if console is color, 0 if mono */
EXTERN int ega;			/* nonzero if console is EGA */
EXTERN int scan_code;		/* scan code of key pressed to start minix */
EXTERN int snow;		/* nonzero if screen needs snow removal */

/* Low level interrupt communications. */
EXTERN struct proc *held_head;	/* head of queue of held-up interrupts */
EXTERN struct proc *held_tail;	/* tail of queue of held-up interrupts */
EXTERN unsigned char k_reenter;	/* kernel reentry count (entry count less 1)*/

/* Kernel memory. */
EXTERN phys_bytes code_base;	/* base of kernel code */
EXTERN phys_bytes data_base;	/* base of kernel data */

/* Memory sizes. */
EXTERN unsigned ext_memsize;	/* initialized by assembler startup code */
EXTERN unsigned low_memsize;
EXTERN phys_clicks mem_base[NR_MEMS];	/* bases of chunks of memory */
EXTERN phys_clicks mem_size[NR_MEMS];	/* sizes of chunks of memory */
EXTERN unsigned char mem_type[NR_MEMS];	/* types of chunks of memory */

/* Process table.  Here to stop too many things having to include proc.h. */
EXTERN struct proc *proc_ptr;	/* pointer to currently running process */

/* Miscellaneous. */
EXTERN u16_t Ax, Bx, Cx, Dx, Es;	/* to hold registers for BIOS calls */
EXTERN struct farptr_s break_vector;	/* debugger breakpoint hook */
EXTERN int db_enabled;		/* nonzero if external debugger is enabled */
EXTERN int db_exists;		/* nonzero if external debugger exists */
extern struct segdesc_s gdt[];	/* global descriptor table for protected mode*/
extern u16_t sizes[8];		/* table filled in by build */
EXTERN struct farptr_s sstep_vector;	/* debugger single-step hook */
extern struct tasktab tasktab[];	/* see table.c */
extern char t_stack[];		/* see table.c */
EXTERN u16_t vec_table[VECTOR_BYTES / sizeof(u16_t)]; /* copy of BIOS vectors*/

/* Functions. */

/* amoeba.c */
void amint_task();
void amoeba_task();

/* at_wini.c, bios_wini.c, ps_wini.c, xt_wini.c, too_many_wini.c */
void winchester_task();

/* clock.c */
void clock_handler();
void clock_task();
void milli_delay();
unsigned read_counter();

/* console.c */
void console();
void flush();
void out_char();
void putc();
void scr_init();
void toggle_scroll();

/* ctstart.c */
void cstart();

/* dmp.c */
void map_dmp();
void p_dmp();
void set_name();

/* exception.c */
void exception();

/* floppy.c */
void floppy_task();

/* i8259.c */
void enable_irq();
void init_8259();

/* keyboard.c */
int func_key();
void kb_init();
int kb_read();
void keyboard();
int letter_code();
int make_break();
void reboot();
void wreboot();

/* klib*.x */
void bios13();
void build_sig();
phys_bytes check_mem();
void cim_at_wini();
void cim_floppy();
void cim_printer();
void cim_xt_wini();
void cp_mess();
unsigned in_byte();
void klib_1hook();
void klib_2hook();
void lock();
void mpx_1hook();
void mpx_2hook();
void out_byte();
void phys_copy();
void port_read();
void port_write();
void reset();
void scr_down();
void scr_up();
void sim_printer();
unsigned tasim_printer();
int test_and_set();
void unlock();
void vid_copy();
void wait_retrace();

/* main.c */
void dp8390_int();
void eth_stp();
void main();
void panic();

/* memory.c */
void mem_task();

/* misc.c */
int do_vrdwt();
void mem_init();

/* mpx*.x */
void idle_task();
void restart();
void int00(), divide_error();	/* exception handlers, in numerical order */
void int01(), single_step_exception();
void int02(), nmi();
void int03(), breakpoint_exception();
void int04(), overflow();
void int05(), bounds_check();
void int06(), inval_opcode();
void int07(), copr_not_available();
void int08(), double_fault();
void int09(), copr_seg_overrun();
void int10(), inval_tss();
void int11(), segment_not_present();
void int12(), stack_exception();
void int13(), general_protection();
void int14(), page_fault();
void int15();
void int16(), copr_error();	/* end of exception handlers */
void clock_int();		/* hardware interrupt handlers, in order */
void tty_int();
void secondary_int(), psecondary_int(), eth_int();
void rs232_int(), prs232_int();
void disk_int();
void lpr_int();
void wini_int();		/* end of hardware interrupt handlers */
void trp();			/* software interrupt handlers, in order */
void s_call(), p_s_call();	/* end of software interrupt handlers */

/* printer.c */
void pr_char();
void pr_restart();
void printer_task();

/* proc.c */
void interrupt();
int lock_mini_send();
void lock_pick_proc();
void lock_ready();
void lock_sched();
void lock_unready();
int sys_call();
void proc_init();
void unhold();

/* protect.c */
void prot_init();
void init_codeseg();
void init_dataseg();
void ldt_init();

/* rs232.c */
void rs232_1handler();
void rs232_2handler();
void rs_inhibit();
int rs_init();
int rs_ioctl();
int rs_read();
void rs_istart();
void rs_istop();
void rs_ocancel();
void rs_setc();
void rs_write();

/* start.x */
void db();
u16_t get_chrome();
u16_t get_ega();
u16_t get_ext_memsize();
u16_t get_low_memsize();
u16_t get_processor();
u16_t get_word();
void put_word();

/* system.c */
void alloc_segments();
void cause_sig();
void inform();
phys_bytes numap();
void sys_task();
phys_bytes umap();

/* tty.c */
void finish();
void sigchar();
void tty_task();
void tty_wakeup();

/* library */
int memcpy();
void printk();
int receive();
int send();
int sendrec();
