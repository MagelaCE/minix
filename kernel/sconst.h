/* Miscellaneous constants used in assembler code. */
#define DS_286_OFFSET DS_286_INDEX*DESC_SIZE
#define EM_MASK		0xFFF0	/* extended memory mask for hi word */
#define ES_286_OFFSET ES_286_INDEX*DESC_SIZE
#define HCHIGH_MASK	0x0F	/* h/w click mask for low byte of hi word */
#define HCLOW_MASK	0xF0	/* h/w click mask for low byte of low word */
#define Msize		12	/* size of a message in 16-bit words */
#define OFF_MASK	0x000F	/* offset mask for long -> hclick:offset */
#define TEST1PATTERN	0x55	/* memory test pattern 1 */
#define TEST2PATTERN	0xAA	/* memory test pattern 2 */

/* Opcodes (used in patching). */
#define IRET_OPCODE	0xCF
#define JMP_OPCODE	0xE9
#define NOP_OPCODE	0x90

/* BIOS numbers. */
#define EM_XFER_VEC	0x15	/* copy (normal or extended) memory */
#define EM_XFER_FUNC	0x87
#define GET_EXTMEM_VEC	0x15	/* get extended memory size */
#define GET_EXTMEM_FUNC	0x88
#define GET_MEM_VEC	0x12	/* get memory size */
#define SET_PROTECT_VEC	0x15	/* set protected mode */
#define SET_PROTECT_FUNC 0x89

/* 8259 mask bits. */
#define AT_WINI_MASK	0x40	/* (1 << (AT_WINI_IRQ & 0x07)), etc */
#define CLOCK_MASK	0x01
#define ETHER_MASK	0x08
#define FLOPPY_MASK	0x40
#define KEYBOARD_MASK	0x02
#define PRINTER_MASK	0x80
#define RS232_MASK	0x10
#define SECONDARY_MASK	0x08
#define XT_WINI_MASK	0x20

/* Offsets in struct proc. They MUST match proc.h. Use '=' to define them
 * instead of '#define', so they can be built cumulatively (for easy changes)
 * without producing large macro expansions.
 */
#if 0 /* uurghh, the duplicated labels cause asld to run out of memory */
P_STACKBASE	=	0
ESREG		=	P_STACKBASE
DSREG		=	ESREG + 2
DIREG		=	DSREG + 2
SIREG		=	DIREG + 2
BPREG		=	SIREG + 2
STREG		=	BPREG + 2	/* hole for another SP */
BXREG		=	STREG + 2
DXREG		=	BXREG + 2
CXREG		=	DXREG + 2
AXREG		=	CXREG + 2
RETADR		=	AXREG + 2	/* return address for save() call */
PCREG		=	RETADR + 2
CSREG		=	PCREG + 2
PSWREG		=	CSREG + 2
SPREG		=	PSWREG + 2
SSREG		=	SPREG + 2
P_STACKTOP	=	SSREG + 2
P_NR		=	P_STACKTOP
P_LDT_SEL	=	P_NR + 2	/* assumes 16 bit ints */
P_LDT		=	P_LDT_SEL + 2
P_SPLIMIT	=	P_LDT_SEL + 16
#endif

#define P_STACKBASE	0
#define ESREG		0
#define DSREG		2
#define DIREG		4
#define SIREG		6
#define BPREG		8
#define STREG		10	/* hole for another SP */
#define BXREG		12
#define DXREG		14
#define CXREG		16
#define AXREG		18
#define RETADR		20	/* return address for save() call */
#define PCREG		22
#define CSREG		24
#define PSWREG		26
#define SPREG		28
#define SSREG		30
#define P_STACKTOP	32
#define P_NR		32
#define P_LDT_SEL	34	/* assumes 16 bit ints */
#define P_LDT		36
#define P_SPLIMIT	52

/* 286 tss offsets. */
#define TSS2_S_BACKLINK	0
#define TSS2_S_SP0	2
#define TSS2_S_SS0	4
#define TSS2_S_SP1	6
#define TSS2_S_SS1	8
#define TSS2_S_SP2	10
#define TSS2_S_SS2	12
#define TSS2_S_IP	14
#define TSS2_S_FLAGS	16
#define TSS2_S_AX	18
#define TSS2_S_CX	20
#define TSS2_S_DX	22
#define TSS2_S_BX	24
#define TSS2_S_SP	26
#define TSS2_S_BP	28
#define TSS2_S_SI	30
#define TSS2_S_DI	32
#define TSS2_S_ES	34
#define TSS2_S_CS	36
#define TSS2_S_SS	38
#define TSS2_S_DS	40
#define TSS2_S_LDT	42
#define SIZEOF_TSS2_S	44

/* Macros to get some 8086 instructions through asld. */
#ifdef ASLD
#define callfar(s,o)	calli	o,s
#define callfarptr(v)	calli	@v
#define jmpfar(s,o)	jmpi	o,s
#define jmpmem(adr)	jmp	@adr
#define jmpreg(reg)	jmp	(reg)
#define notop(n)	[-[n]-1]
#define orop		+	/* used only if same as add */
#else
#define callfar(s,o)	call	s:o
#define callfarptr(v)	call	far [v]
#define jmpfar(s,o)	jmp	s:o
#define jmpmem(adr)	jmp	adr
#define jmpreg(reg)	jmp	reg
#define notop(n)	(!(n))
#define orop		\	/* don't expose '\' to end of line */
#endif /* ASLD */

/* Macros to get some 286 instructions through asld.
 * Some are used without i80286.
 */
#ifdef ASLD
#define ESC2		.byte	0x0F	/* escape for some 286 instructions */
#define defsgdt(adr)	ESC2;	add	adr,ax	/* may not be general */
#define insw		.byte	0x6D
#define outsw		.byte	0x6F
#else
#define defsgdt(adr)	sgdt	adr
#endif

#ifdef i80286
#ifdef ASLD
#define deflgdt(adr)	ESC2;	add	adr,dx	/* may not be general */
#define deflidt(adr)	ESC2;	add	adr,bx	/* may not be general */
#define deflldt(adr)	ESC2;	addb	adr,dl	/* may not be general */
#define defltrax	ESC2;	.byte	0x00,0xD8
#define defsidt(adr)	ESC2;	add	adr,cx	/* may not be general */
#define defsldt(adr)	ESC2;	addb	adr,al	/* may not be general */
#define popa		.byte	0x61
#define pusha		.byte	0x60
#define pushi8(n)	.byte	0x6A;	.byte	n
#define pushi16(n)	.byte	0x68;	.word	n
#define use32
#else
#define deflgdt(adr)	lgdt	adr
#define deflidt(adr)	lidt	adr
#define deflldt(adr)	lldt	adr
#define defltrax	ltr	ax
#define defsidt(adr)	sidt	adr
#define defsldt(adr)	sldt	adr
#define pushi8(n)	push	#n
#define pushi16(n)	push	#n
#endif /* ASLD */
#endif /* i80286 */
