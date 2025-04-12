/* 286 software constants. */

/* Table sizes. */
#define GDT_SIZE (FIRST_LDT_INDEX + NR_TASKS + NR_PROCS) /* spec. and LDT's */
#define IDT_SIZE (IRQ8_VECTOR + 8)	/* only up to the highest vector */
#define LDT_SIZE         2	/* contains CS and DS only */

/* Fixed global descriptors; 1 to 7 are prescribed by the BIOS. */
#define GDT_INDEX        1	/* temp spot to store pointer to GDT */
#define IDT_INDEX        2	/* temp spot to store pointer to GDT */
#define DS_INDEX         3	/* kernel DS */
#define ES_INDEX         4	/* kernel ES */
#define SS_INDEX         5	/* kernel SS */
#define CS_INDEX         6	/* kernel CS */
#define BIOS_CS_INDEX    7	/* temp for BIOS */
#define TSS_INDEX        8	/* kernel TSS */
#define DB_TSS_INDEX     9	/* debugger TSS */
#define DS_286_INDEX    10	/* scratch source segment for klib286 */
#define ES_286_INDEX    11	/* scratch destination segment for klib286 */
#define COLOR_INDEX     12	/* color screen segment */
#define MONO_INDEX      13	/* mono screen segment */
				/* the next 2 only work for 386's but are
				 * reserved for debugging 286 code on 386's
				 */
#define REAL_CS_INDEX   14	/* kernel CS suitable for real mode switch */
#define REAL_DS_INDEX   15	/* kernel DS suitable for real mode switch */
#define FIRST_LDT_INDEX 16

#define GDT_SELECTOR      0x08	/* (GDT_INDEX * DESC_SIZE) bad for asld */
#define IDT_SELECTOR      0x10	/* (IDT_INDEX * DESC_SIZE) */
#define DS_SELECTOR       0x18	/* (DS_INDEX * DESC_SIZE) */
#define CS_SELECTOR       0x30	/* (CS_INDEX * DESC_SIZE) */
#define BIOS_CS_SELECTOR  0x38	/* (BIOS_CS_INDEX * DESC_SIZE) */
#define TSS_SELECTOR      0x40	/* (TSS_INDEX * DESC_SIZE) */
#define DB_TSS_SELECTOR   0x48	/* (DB_TSS_INDEX * DESC_SIZE) */
#define DS_286_SELECTOR   0x51	/* (DS_286_INDEX * DESC_SIZE + 1) */
#define ES_286_SELECTOR   0x59	/* (DS_286_INDEX * DESC_SIZE + 1) */
#define COLOR_SELECTOR    0x61	/* (COLOR_INDEX * DESC_SIZE + 1) */
#define MONO_SELECTOR     0x69	/* (MONO_INDEX * DESC_SIZE + 1) */
#define REAL_CS_SELECTOR  0x70	/* (REAL_CS_INDEX * DESC_SIZE) */
#define REAL_DS_SELECTOR  0x78	/* (REAL_DS_INDEX * DESC_SIZE) */

/* Fixed local descriptors. */
#define CS_LDT_INDEX     0	/* process CS */
#define DS_LDT_INDEX     1	/* process DS=ES=FS=GS=SS */

/* Privileges. */
#define INTR_PRIVILEGE   0	/* kernel and interrupt handlers */
#define TASK_PRIVILEGE   1
#define USER_PRIVILEGE   3

/* 286 hardware constants. */

/* Exception vector numbers. */
#define BOUNDS_VECTOR       5	/* bounds check failed */
#define INVAL_OP_VECTOR     6	/* invalid opcode */
#define COPROC_NOT_VECTOR   7	/* coprocessor not available */
#define DOUBLE_FAULT_VECTOR 8
#define COPROC_SEG_VECTOR   9	/* coprocessor segment overrun */
#define INVAL_TSS_VECTOR   10	/* invalid TSS */
#define SEG_NOT_VECTOR     11	/* segment not present */
#define STACK_FAULT_VECTOR 12	/* stack exception */
#define PROTECTION_VECTOR  13	/* general protection */

/* Selector bits. */
#define TI            0x04	/* table indicator */
#define RPL           0x03	/* requester privilege level */

/* Descriptor structure offsets. */
#define DESC_BASE        2	/* to base_low */
#define DESC_BASE_MIDDLE 4	/* to base_middle */
#define DESC_ACCESS	 5	/* to access byte */
#define DESC_SIZE        8	/* sizeof (struct segdesc_s) */

/* Segment sizes. */
#define MAX_286_SEG_SIZE 0x10000L
#define REAL_SEG_SIZE    0x10000L

/* Base and limit sizes and shifts. */
#define BASE_MIDDLE_SHIFT   16	/* shift for base --> base_middle */

/* Access-byte and type-byte bits. */
#define PRESENT       0x80	/* set for descriptor present */
#define DPL           0x60	/* descriptor privilege level mask */
#define DPL_SHIFT        5
#define SEGMENT       0x10	/* set for segment-type descriptors */

/* Access-byte bits. */
#define EXECUTABLE    0x08	/* set for executable segment */
#define CONFORMING    0x04	/* set for conforming segment if executable */
#define EXPAND_DOWN   0x04	/* set for expand-down segment if !executab */
#define READABLE      0x02	/* set for readable segment if executable */
#define WRITEABLE     0x02	/* set for writeable segment if !executable */
#define TSS_BUSY      0x02	/* set if TSS descriptor is busy */
#define ACCESSED      0x01	/* set if segment accessed */

/* Special descriptor types. */
#define AVL_286_TSS      1	/* available 286 TSS */
#define LDT              2	/* local descriptor table */
#define BUSY_286_TSS     3	/* set transparently to the software */
#define CALL_286_GATE    4	/* not used */
#define TASK_GATE        5	/* only used by debugger */
#define INT_286_GATE     6	/* interrupt gate, used for all vectors */
#define TRAP_286_GATE    7	/* not used */
