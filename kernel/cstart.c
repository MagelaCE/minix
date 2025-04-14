/* This file contains the C startup code for Minix on Intel processors.
 * It cooperates with start.x to set up a good environment for main().
 *
 * The code must run in 16-bit mode (and be compiled with a 16-bit
 * compiler!) even for a 32-bit kernel.
 * So care must be taken to avoid accessing data structures (such as the
 * process table) which depend on type sizes, and to avoid calling functions
 * except those in the companion files start.x and protect.c, and the stage
 * 1 assembler hooks.
 * Also, variables beyond 64K must not be accessed - this is guaranteed
 * by keeping the kernel small.
 * This is not so easy when the 32-bit compiler does not support separate
 * I&D, since the kernel is already larger than 64K.
 * The order of the objects in the makefile is chosen so all variables
 * accessed from here are early in the list, and the linker is relied on
 * not to alter the order.
 * It might be better to separately-compile start.x, cstart.c and protect.c,
 * and pass the results to main() in a big structure.
 */

#include "kernel.h"
#include <minix/boot.h>

/* Magic BIOS addresses. */
#define BIOS_CSEG		0xF000	/* segment of BIOS code */
#define BIOS_DSEG		0x0040	/* segment of BIOS data */
#	define BIOS_CURSOR	0x0060	/* offset to cursor type word */
#define MACHINE_TYPE_SEG	0xFFFF	/* segment of machine type word */
#	define MACHINE_TYPE_OFF	0x000E	/* offset of machine type word */
#	define PC_AT	0xFC	/* code in first byte for PC-AT */
#	define PS	0xFA	/* code in first byte for PS/2 Model 30 */
#	define PS_386	0xF8	/* code in first byte for PS/2 386 70 & 80 */
#	define PS_50	0x04	/* code in second byte for PS/2 Model 50 */
#	define PS_60	0x05	/* code in second byte for PS/2 Model 60 */

FORWARD void db_init();

/*==========================================================================*
 *				cstart					    *
 *==========================================================================*/
PUBLIC void cstart(ax, bx, cx, dx, si, di, cs, ds)
u16_t ax;			/* boot code (registers from boot loader) */
u16_t bx;			/* scan code */
u16_t cx;			/* amount of boot parameters in bytes */
u16_t dx;			/* not used */
u16_t si;			/* offset of boot parameters in loader */
u16_t di;			/* segment of boot parameters in loader  */
u16_t cs;			/* real mode kernel code segment */
u16_t ds;			/* real mode kernel data segment */
{
/* Perform initializations which must be done in real mode. */

  register u16_t *bootp;
  unsigned machine_magic;
  unsigned mach_submagic;

  /* Record where the kernel is. */
  code_base = hclick_to_physb(cs);
  data_base = hclick_to_physb(ds);

  /* Copy the boot parameters, if any, to kernel memory. */
  if (ax == 0) {
	/* New boot loader (ax == bx == scan code for old one). */
	if (cx > sizeof boot_parameters) cx = sizeof boot_parameters;
	cx /= 2;		/* word count */
	for (bootp = (u16_t *) &boot_parameters; cx != 0; --cx, si += 2)
		*bootp++ = get_word(di, si);
  }
  scan_code = bx;

  /* Get information from the BIOS. */
  color = get_chrome();
  ega = get_ega();
  ext_memsize = get_ext_memsize();
  low_memsize = get_low_memsize();

  /* Determine machine type. */
  processor = get_processor();	/* 86, 186, 286 or 386 */
  machine_magic = get_word(MACHINE_TYPE_SEG, MACHINE_TYPE_OFF);
  mach_submagic = (machine_magic >> 8) & BYTE;
  machine_magic &= BYTE;
  if (machine_magic == PC_AT) {
	pc_at = TRUE;
	/* could be a PS/2 Model 50 or 60 -- check submodel byte */
	if (mach_submagic == PS_50 || mach_submagic == PS_60) ps_mca = TRUE;
  } else if (machine_magic == PS_386)
	pc_at = ps_mca = TRUE;
  else if (machine_magic == PS)
	ps = TRUE;

  /* Decide if mode is protected. */
  if (processor >= 286 && boot_parameters.bp_processor >= 286 && !using_bios) {
	protected_mode = TRUE;
  }
  boot_parameters.bp_processor = protected_mode;	/* FS needs to know */

  /* Initialize debugger (requires 'protected_mode' to be initialized). */
  db_init();

  /* Call stage 1 assembler hooks to begin machine/mode specific inits. */
  mpx_1hook(); 
  klib_1hook(); 

  /* Call main() and never return if not protected mode. */
  if (!protected_mode) main();

  /* Initialize protected mode (requires 'break_vector' and other globals). */
  prot_init();

  /* Return to assembler code to switch modes. */
}


/*==========================================================================*
 *				db_init					    *
 *==========================================================================*/
PRIVATE void db_init()
{
/* Initialize vectors for external debugger. */

  break_vector.offset = get_word(VEC_TABLE_SEG, BREAKPOINT_VECTOR * 4);
  break_vector.selector = get_word(VEC_TABLE_SEG, BREAKPOINT_VECTOR * 4 + 2);
  sstep_vector.offset = get_word(VEC_TABLE_SEG, DEBUG_VECTOR * 4);
  sstep_vector.selector = get_word(VEC_TABLE_SEG, DEBUG_VECTOR * 4 + 2);

  /* No debugger if the breakpoint vector points into the BIOS. */
  if ((u16_t) break_vector.selector >= BIOS_CSEG) return;

  /* Debugger vectors for protected mode are by convention stored as 16-bit
   * offsets in the debugger code segment, just before the corresponding real
   * mode entry points.
   */
  if (protected_mode) {
	break_vector.offset = get_word((u16_t) break_vector.selector,
				       (u16_t) break_vector.offset - 2);
	sstep_vector.offset = get_word((u16_t) sstep_vector.selector,
				       (u16_t) sstep_vector.offset - 2);

	/* Different code segments are not supported. */
	if ((u16_t) break_vector.selector != (u16_t) sstep_vector.selector)
		return;
  }

  /* Enable debugger. */
  db_exists = TRUE;
  db_enabled = TRUE;

  /* Tell debugger about Minix's normal cursor shape via the BIOS.  It would
   * be nice to tell it the variable video parameters, but too hard.  At least
   * the others get refreshed by the console driver.  Blame the 6845's
   * read-only registers.
   */
  put_word(BIOS_DSEG, BIOS_CURSOR, CURSOR_SHAPE);
}
