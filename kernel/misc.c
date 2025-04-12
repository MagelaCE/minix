/* This file contains a collection of miscellaneous procedures:
 *	mem_init:	initialize memory tables.  Some memory is reported
 *			by the BIOS, some is guesstimated and checked later
 */

#include "../h/const.h"
#include "../h/type.h"
#include "const.h"
#include "type.h"
#include "glo.h"

#ifdef i8088

#define EM_BASE     0x100000L	/* base of extended memory on AT's */
#define SHADOW_BASE 0xFA0000L	/* base of RAM shadowing ROM on some AT's */
#define SHADOW_MAX  0x060000L	/* maximum usable shadow memory (16M limit) */

extern unsigned get_extmemsize();
extern unsigned get_memsize();


/*=========================================================================*
 *				mem_init				   *
 *=========================================================================*/
PUBLIC mem_init()
{
/* Initialize the memory size tables.  This is complicated by fragmentation
 * and different access strategies for protected mode.  There must be a
 * chunk at 0 big enough to hold Minix proper.  For 286 and 386 processors,
 * there can be extended memory (memory above 1MB).  This usually starts at
 * 1MB, but there may be another chunk just below 16MB, reserved under DOS
 * for shadowing ROM, but available to Minix if the hardware can be re-mapped.
 * In protected mode, extended memory is accessible assuming CLICK_SIZE is
 * large enough, and is treated as ordinary momory.
 * The magic bits for memory types are:
 *	1: extended
 *	0x80: must be checked since BIOS doesn't and it may not be there.
 */

  /* Get the size of ordinary memory from the BIOS. */
  memsize[0] = k_to_click(get_memsize());	/* 0 base and type */

#ifdef SPARE_VIDEO_MEMORY
  /* Spare video memory.  Experimental, it's too slow for program memory
   * except maybe on PC's, and belongs low in a memory hierarchy.
   */
  if (color) {
	memsize[1] = MONO_SIZE >> CLICK_SHIFT;
	membase[1] = MONO_BASE >> CLICK_SHIFT;
  } else {
	memsize[1] = COLOR_SIZE >> CLICK_SHIFT;
	membase[1] = COLOR_BASE >> CLICK_SHIFT;
  }
  memtype[1] = 0x80;
#endif

  if (pc_at) {
	/* Get the size of extended memory from the BIOS.  This is special
	 * except in protected mode, but protected mode is now normal.
	 */
	memsize[2] = k_to_click(get_extmemsize());
	membase[2] = EM_BASE >> CLICK_SHIFT;

	/* Shadow ROM memory. */
	memsize[3] = SHADOW_MAX >> CLICK_SHIFT;
	membase[3] = SHADOW_BASE >> CLICK_SHIFT;
	memtype[3] = 0x80;
	if (processor < 286) {
		memtype[2] = 1;
		memtype[3] |= 1;
	}
  }
}
#endif /* i8088 */
