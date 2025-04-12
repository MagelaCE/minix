/* Global variables used in the kernel. */

/* Signals. */
EXTERN int sig_procs;		/* number of procs with p_pending != 0 */

/* CPU type. */
EXTERN int pc_at;		/* PC-AT type diskette drives (360K/1.2M) ? */
EXTERN int ps;			/* are we dealing with a ps? */
EXTERN int port_65;		/* saved contents of Planar Control Register */
EXTERN unsigned processor;	/* 86, 186, 286, 386, ... */

/* Video cards and keyboard types. */
EXTERN int color;		/* 1 if console is color, 0 if it is mono */
EXTERN int ega;			/* 1 if console is EGA, 0 if not */
EXTERN int scan_code;		/* scan code of key pressed to start minix */
EXTERN int snow;		/* 1 if screen needs snow removal, 0 if not */

/* Low level interrupt communications. */
EXTERN struct proc *held_head;	/* head of queue of held-up interrupts */
EXTERN struct proc *held_tail;	/* tail of queue of held-up interrupts */
EXTERN unsigned char k_reenter;	/* kernel reentry count (entry count less 1)*/

/* Memory sizes. */
#ifdef i8088
EXTERN phys_clicks membase[NR_MEMS];	/* bases of chunks of memory */
EXTERN phys_clicks memsize[NR_MEMS];	/* sizes of chunks of memory */
EXTERN unsigned char memtype[NR_MEMS];	/* types of chunks of memory */
#endif

/* Miscellaneous (still all over the place in non-header files). */
extern unsigned sizes[8];		/* table filled in by build */
extern struct tasktab tasktab[];	/* see table.c */
extern char t_stack[];			/* see table.c */
extern int vec_table[VECTOR_BYTES/ sizeof (int)];  /* copy of BIOS vectors */

/* Non-integer functions (still all over the place in non-header files). */
extern phys_bytes umap();	/* map user address to physical address */
#ifdef i8088
extern unsigned codeseg();	/* current code segment */
extern unsigned dataseg();	/* current data segment */
#endif
