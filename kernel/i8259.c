/* This file contains routines for initializing the 8259 interrupt controller:
 *	enable_irq:	enable an interrupt line.  The cim...() functions in
 *			klib88 are specialized versions of this
 *	init_8259:	initialize the 8259(s), since the BIOS does it wrong
 */

#include "../h/const.h"
#include "../h/type.h"
#include "../h/com.h"
#include "const.h"
#include "type.h"
#include "glo.h"

#define ICW1_AT         0x11	/* edge triggered, cascade, need ICW4 */
#define ICW1_PC         0x13	/* edge triggered, no cascade, need ICW4 */
#define ICW3_M          0x04	/* bit 2 for slave on channel 2 */
#define ICW3_S          0x02	/* slave identity is 2 */
#define ICW4_AT         0x01	/* not SFNM, not buffered, normal EOI, 8086 */
#define ICW4_PC         0x09	/* not SFNM, buffered, normal EOI, 8086 */


/*==========================================================================*
 *				enable_irq				    *
 *==========================================================================*/
PUBLIC enable_irq(irq_nr)
unsigned irq_nr;
{
/* Clear the corresponding 8259 register bit. */

  int old_state;

  old_state = lock();
  if (irq_nr < 8)
	port_out(INT_CTLMASK, inportb(INT_CTLMASK) & ~(1 << irq_nr));
  else 
	port_out(INT2_MASK, inportb(INT2_MASK) & ~(1 << (irq_nr - 8)));
  restore(old_state);
}


/*==========================================================================*
 *				init_8259				    *
 *==========================================================================*/
PUBLIC init_8259(master_base, slave_base)
unsigned master_base;
unsigned slave_base;
{
/* Initialize the 8259(s), finishing with all interrupts disabled. */

  if (pc_at) {
	port_out(INT_CTL, ICW1_AT);
	port_out(INT_CTLMASK, master_base);	/* ICW2 for master */
	port_out(INT_CTLMASK, ICW3_M);
	port_out(INT_CTLMASK, ICW4_AT);
	port_out(INT2_CTL, ICW1_AT);
	port_out(INT2_MASK, slave_base);	/* ICW2 for slave */
	port_out(INT2_MASK, ICW3_S);
	port_out(INT2_MASK, ICW4_AT);
	port_out(INT2_MASK, ~0);
  } else {
	port_out(INT_CTL, ICW1_PC);
	port_out(INT_CTLMASK, master_base);	/* no slave */
	port_out(INT_CTLMASK, ICW4_PC);
  }
  port_out(INT_CTLMASK, ~0);
}
