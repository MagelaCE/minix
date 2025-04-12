struct tasktab {
	int	(*initial_pc)();
	int	stksize;
	char	name[8];
};

#ifdef i8088

/* The u.._t types and their derivatives are used (only) when the precise
 * size must be specified for alignment of machine-dependent structures.
 * The offset_t type is the promotion of the smallest unsigned type large
 * enough to hold all machine offsets.  The phys_bytes type is not right
 * since it is signed, so will break on future 386 systems which have real
 * or virtual memory in the top half of the address space. Also, signed
 * remainders and divisions by powers of 2 cannot be done as efficiently.
 */
typedef unsigned long offset_t;	/* machine offset */
typedef unsigned char u8_t;	/* unsigned 8 bits */
typedef unsigned short u16_t;	/* unsigned 16 bits */
typedef unsigned long u32_t;	/* unsigned 32 bits */

/* The stack frame layout is determined by the software, but for efficiency
 * it is laid out so the assembly code to use it is as simple as possible.
 * 80286 protected mode and all real modes use the same frame, built with
 * 16-bit registers.  Real mode lacks an automatic stack switch, so little
 * is lost by using the 286 frame for it.
 */
union stackframe_u {
  struct {
	u16_t es;
	u16_t ds;
	u16_t di;		/* di through cx are not accessed in C */
	u16_t si;		/* order is to match pusha/popa */
	u16_t bp;
	u16_t st;		/* hole for another copy of sp */
	u16_t bx;
	u16_t dx;
	u16_t cx;
	u16_t retreg;		/* ax */
	u16_t retadr;		/* return address for assembly code save() */
	u16_t pc;		/* interrupt pushes rest of frame */
	u16_t cs;
	u16_t psw;
	u16_t sp;
	u16_t ss;
  } r16;
};

#ifdef i80286
struct segdesc_s {		/* segment descriptor */
  u16_t limit_low;
  u16_t base_low;
  u8_t base_middle;
  u8_t access;			/* |P|DL|1|X|E|R|A| */
  u16_t reserved;
};
#endif /* i80286 */
#endif /* i8088 */
