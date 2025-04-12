/* General constants used by the kernel. */

#ifdef i8088

#define INIT_PSW      0x0200	/* initial psw */
#define INIT_TASK_PSW 0x1200	/* initial psw for tasks (with IOPL 1) */

/* Initial sp for mm, fs and init.
 *	2 bytes for short jump
 *	2 bytes unused
 *	3 words for init_org[] used by fs only
 *	3 words for real mode debugger trap
 *	3 words for save and restart temporaries
 *	3 words for interrupt
 * Leave no margin, to flush bugs early.
 */
#define INIT_SP (2 + 2 + 3 * 2 + 3 * 2 + 3 * 2 + 3 * 2)

#define HCLICK_SHIFT       4	/* log2 of HCLICK_SIZE */
#define HCLICK_SIZE       16	/* hardware segment conversion magic */

#define ALIGNMENT	   4	/* align large items to a multiple of this */
#define VECTOR_BYTES     284	/* bytes of interrupt vectors to save */

/* Interrupt vectors defined/reserved by processor */
#define DIVIDE_VECTOR      0	/* divide error */
#define DEBUG_VECTOR       1	/* single step (trace) */
#define NMI_VECTOR         2	/* non-maskable interrupt */
#define BREAKPOINT_VECTOR  3	/* software breakpoint */
#define OVERFLOW_VECTOR    4	/* from INTO */

/* Fixed system call vector (the only software interrupt). */
#define SYS_VECTOR        32	/* system calls are made with int SYSVEC */

/* Suitable irq bases for hardware interrupts.  Reprogram the 8259(s) from
 * the PC BIOS defaults since the BIOS doesn't respect all the processor's
 * reserved vectors (0 to 31).
 */
#define IRQ0_VECTOR     0x28	/* more or less arbitrary, but > 32 */
#define IRQ8_VECTOR     0x30 	/* together for simplicity */

/* Hardware interrupt numbers. */
#define CLOCK_IRQ          0
#define KEYBOARD_IRQ       1
#define CASCADE_IRQ        2	/* cascade enable for 2nd AT controller */
#define ETHER_IRQ          3	/* ethernet interrupt vector */
#define SECONDARY_IRQ      3	/* RS232 interrupt vector for port 2 */
#define RS232_IRQ          4	/* RS232 interrupt vector for port 1 */
#define XT_WINI_IRQ        5	/* xt winchester */
#define FLOPPY_IRQ         6	/* floppy disk */
#define PRINTER_IRQ        7
#define AT_WINI_IRQ       14	/* at winchester */
#define PS_KEYB_IRQ        9	/* keyboard interrupt vector for PS/2 */

/* Hardware vector numbers. */
#define CLOCK_VECTOR     ((CLOCK_IRQ & 0x07) + IRQ0_VECTOR)
#define KEYBOARD_VECTOR  ((KEYBOARD_IRQ & 0x07) + IRQ0_VECTOR)
#define ETHER_VECTOR     ((ETHER_IRQ & 0x07) + IRQ0_VECTOR)
#define SECONDARY_VECTOR ((SECONDARY_IRQ & 0x07) + IRQ0_VECTOR)
#define RS232_VECTOR     ((RS232_IRQ & 0x07) + IRQ0_VECTOR)
#define XT_WINI_VECTOR   ((XT_WINI_IRQ & 0x07) + IRQ0_VECTOR)
#define FLOPPY_VECTOR    ((FLOPPY_IRQ & 0x07) + IRQ0_VECTOR)
#define PRINTER_VECTOR   ((PRINTER_IRQ & 0x07) + IRQ0_VECTOR)
#define AT_WINI_VECTOR   ((AT_WINI_IRQ & 0x07) + IRQ8_VECTOR)
#define PS_KEYB_VECTOR   ((PS_KEYB_IRQ & 0x07) + IRQ8_VECTOR)

/* 8259A interrupt controller ports. */
#define INT_CTL         0x20	/* I/O port for interrupt controller */
#define INT_CTLMASK     0x21	/* setting bits in this port disables ints */
#define INT2_CTL        0xA0	/* I/O port for second interrupt controller */
#define INT2_MASK       0xA1	/* setting bits in this port disables ints */

/* Magic numbers for interrupt controller. */
#define ENABLE          0x20	/* code used to re-enable after an interrupt */

/* Sizes of memory tables. */
#define NR_MEMS            4	/* number of chunks of memory */

/* Magic memory locations and sizes. */
#define COLOR_BASE   0xB8000L	/* base of color video memory */
#define COLOR_SIZE    0x8000L	/* maximum usable color video memory */
#define MONO_BASE    0xB0000L	/* base of mono video memory */
#define MONO_SIZE     0x8000L	/* maximum usable mono video memory */

/* Misplaced stuff */
#define PCR		0x65	/* Planar Control Register */

#endif /* i8088 */

#define K_STACK_BYTES    512	/* how many bytes for the kernel stack */

/* The following items pertain to the 3 scheduling queues. */
#define NQ                 3	/* # of scheduling queues */
#define TASK_Q             0	/* ready tasks are scheduled via queue 0 */
#define SERVER_Q           1	/* ready servers are scheduled via queue 1 */
#define USER_Q             2	/* ready users are scheduled via queue 2 */

#define printf        printk	/* the kernel really uses printk, not printf */

#if CLICK_SIZE >= HCLICK_SIZE
#define click_to_hclick(n) ((n) << (CLICK_SHIFT - HCLICK_SHIFT))
#else
#define click_to_hclick(n) ((n) >> (HCLICK_SHIFT - CLICK_SHIFT))
#endif
