/* This file contains a driver for the IBM-PS/2 winchester controller.
 * It was written by Wim van Leersum.
 *
 * The driver supports the following operations (using message format m2):
 *
 *    m_type      DEVICE    PROC_NR     COUNT    POSITION  ADRRESS
 * ----------------------------------------------------------------
 * |  DISK_READ | device  | proc nr |  bytes  |  offset | buf ptr |
 * |------------+---------+---------+---------+---------+---------|
 * | DISK_WRITE | device  | proc nr |  bytes  |  offset | buf ptr |
 * ----------------------------------------------------------------
 * |SCATTERED_IO| device  | proc nr | requests|         | iov ptr |
 * ----------------------------------------------------------------
 *
 * The file contains one entry point:
 *
 *   winchester_task:	main entry when system is brought up
 *
 */

#include "kernel.h"
#include <minix/callnr.h>
#include <minix/com.h>
#include <minix/partition.h>

/* I/O Ports used by winchester disk controller. */
#define DATA		0x320	/* data register */
#define ASR		0x322	/* Attachment Status Register */
#define ATT_REG		0x324	/* Attention register */
#define ISR		0x324	/* Interrupt status register */
#define ACR		0x322	/* Attachment control register */

/* Winchester disk controller status bits. */
#define BUSY		0x04	/* controler busy? */
#define DATA_REQUEST	0x10	/* controler asking for data */
#define IR		0x02	/* Interrupt Request */

/* Winchester disk controller command bytes. */
#define CSB0		0x03	/* Command Specify Block byte 0 */
#define CSB	        0x40	/* Get controlers attention for a CSB */
#define DR	        0x10	/* Get controlers attention for data transfer */
#define CCB	        0x80	/* same for command control block */
#define WIN_READ  (char)0x15	/* command for the drive to read */
#define WIN_WRITE (char)0x95	/* command for the drive to write */

/* Miscellaneous. */
#define SECTOR_SIZE      512	/* physical sector size in bytes */
#define ERR		  -1	/* general error */
#define MAX_ERRORS         4	/* how often to try rd/wt before quitting */
#define MAX_DRIVES         1	/* only one supported (hd0 - hd4) */
#define NR_DEVICES      (MAX_DRIVES * DEV_PER_DRIVE)
#define MAX_WIN_RETRY  10000	/* max # times to try to input from WIN */
#define DEV_PER_DRIVE   (1 + NR_PARTITIONS)	/* whole drive & each partn */

#define DMA_READ	0x47	/* DMA read opcode */
#define DMA_WRITE	0x4B	/* DMA write opcode */
#define DMA_ADDR       0x006	/* port for low 16 bits of DMA address */
#define DMA_TOP	       0x082	/* port for top 4 bits of 20-bit DMA addr */
#define DMA_COUNT      0x007	/* port for DMA count (count =	bytes - 1) */
#define DMA_M2	       0x00C	/* DMA status port */
#define DMA_M1	       0x00B	/* DMA status port */
#define DMA_INIT       0x00A	/* DMA init port */

/* Variables. */
PUBLIC int using_bios = FALSE;	/* this disk driver does not use the BIOS */

PRIVATE struct wini {		/* main drive struct, one entry per drive */
  int wn_heads;			/* maximum number of heads */
  int wn_maxsec;		/* maximum number of sectors per track */
  long wn_low;			/* lowest cylinder of partition */
  long wn_size;			/* size of partition in blocks */
} wini[NR_DEVICES];

PRIVATE int w_need_reset = FALSE;	 /* set to 1 when controller must be reset */
PRIVATE int nr_drives;		 /* Number of drives */

PRIVATE message w_mess;		/* message buffer for in and out */

PRIVATE char command[14];		/* Common command block */

PRIVATE unsigned char buf[BLOCK_SIZE] = { 1 };
				/* Buffer used by the startup routine */
				/* Initialized to try to avoid DMA wrap. */

FORWARD void ch_select();
FORWARD void ch_unselect();
FORWARD int com_out();
FORWARD void copy_params();
FORWARD void copy_prt();
FORWARD void init_params();
FORWARD void set_command();
FORWARD void sort();
FORWARD int status();
FORWARD int w_do_rdwt();
FORWARD void w_dma_setup();
FORWARD int w_reset();
FORWARD int w_transfer();
FORWARD int win_init();
FORWARD int win_results();

/*===========================================================================*
 *				winchester_task				     * 
 *===========================================================================*/
PUBLIC void winchester_task()
{
/* Main program of the winchester disk driver task. */

int r, caller, proc_nr;

  /* First initialize the controller */
  init_params();

  /* Here is the main loop of the disk task.  It waits for a message, carries
   * it out, and sends a reply.
   */

  while (TRUE) {
	/* First wait for a request to read or write a disk block. */
	receive(ANY, &w_mess);	/* get a request to do some work */
	if (w_mess.m_source < 0) {
		printf("winchester task got message from %d ", w_mess.m_source);
		continue;
	}
	caller = w_mess.m_source;
	proc_nr = w_mess.PROC_NR;

	/* Now carry out the work. */
	switch(w_mess.m_type) {
	    case DISK_READ:
	    case DISK_WRITE:	r = w_do_rdwt(&w_mess);	break;
	    case SCATTERED_IO:	r = do_vrdwt(&w_mess, w_do_rdwt); break;
	    default:		r = EINVAL;		break;
	}

	/* Finally, prepare and send the reply message. */
	w_mess.m_type = TASK_REPLY;	
	w_mess.REP_PROC_NR = proc_nr;

	w_mess.REP_STATUS = r;	/* # of bytes transferred or error code */
	send(caller, &w_mess);	/* send reply to caller */
  }
}


/*===========================================================================*
 *				w_do_rdwt					     * 
 *===========================================================================*/
PRIVATE int w_do_rdwt(m_ptr)
message *m_ptr;			/* pointer to read or write w_message */
{
/* Carry out a read or write request from the disk. */
register struct wini *wn;
int r, drive, device, errors = 0;
long sector;

  /* Decode the w_message parameters. */
  device = m_ptr->DEVICE;
  if (device < 0 || device >= NR_DEVICES)
	return(EIO);
  if (m_ptr->COUNT != BLOCK_SIZE)
	return(EINVAL);
  wn = &wini[device];		/* 'wn' points to entry for this drive */
  drive = device/DEV_PER_DRIVE;	/* save drive number */
  if (drive >= nr_drives)
	return(EIO);
  if (m_ptr->POSITION % BLOCK_SIZE != 0)
	return(EINVAL);
  sector = m_ptr->POSITION/SECTOR_SIZE;
  if ((sector+BLOCK_SIZE/SECTOR_SIZE) > wn->wn_size)
	return(0);

  ch_select();		/* Select fixed disk chip */

  /* This loop allows a failed operation to be repeated. */
  while (errors <= MAX_ERRORS) {
	r = OK;
	errors++;		/* increment count once per loop cycle */
	if (errors > MAX_ERRORS) {
		ch_unselect();
		return(EIO);
	}

	/* First check to see if a reset is needed. */
	if (w_need_reset) r = w_reset();

	if (r != OK) break;
	
	r = w_transfer(wn);	/* Perform the transfer. */
	if (r == OK) break;	/* if successful, exit loop */
  }

  ch_unselect();		/* Do not select fixed disk chip anymore */

  return(r == OK ? BLOCK_SIZE : EIO);
}


/*===========================================================================*
 *			ch_select					 				* 
 *==========================================================================*/
PRIVATE void ch_select() 
{
/* Select fixed disk chip. */

  out_byte(PCR, in_byte(PCR) | 1); /* bit 1 of Planar Control Reg selects it */
}

/*===========================================================================*
 *			ch_unselect					 				* 
 *==========================================================================*/
PRIVATE void ch_unselect() 
{
/* Unselect fixed disk chip. */

  out_byte(PCR, in_byte(PCR) & ~1); /* bit 1 of Planar Control Reg selects it*/
}

/*===========================================================================*
 *			w_dma_setup					 				* 
 *==========================================================================*/
PRIVATE void w_dma_setup()
{
/* The IBM PC can perform DMA operations by using the DMA chip.	 To use it,
 * the DMA (Direct Memory Access) chip is loaded with the 20-bit memory address
 * to by read from or written to, the byte count minus 1, and a read or write
 * opcode.	This routine sets up the DMA chip.	Note that the chip is not
 * capable of doing a DMA across a 64K boundary (e.g., you can't read a 
 * 512-byte block starting at physical address 65520).
 */

  int mode, low_addr, high_addr, top_addr, low_ct, high_ct, top_end;
  vir_bytes vir, ct;
  phys_bytes user_phys;

  mode = (w_mess.m_type == DISK_READ ? DMA_READ : DMA_WRITE);
  vir = (vir_bytes) w_mess.ADDRESS;
  ct = (vir_bytes) BLOCK_SIZE;
  user_phys = numap(w_mess.PROC_NR, vir, BLOCK_SIZE);

  low_addr	= (int) user_phys & BYTE;
  high_addr = (int) (user_phys >>  8) & BYTE;
  top_addr	= (int) (user_phys >> 16) & BYTE;
  low_ct  = (int) (ct - 1) & BYTE;
  high_ct = (int) ( (ct - 1) >> 8) & BYTE;

  /* Check to see if the transfer will require the DMA address counter to
   * go from one 64K segment to another.  If so, do not even start it, since
   * the hardware does not carry from bit 15 to bit 16 of the DMA address.
   * Also check for bad buffer address.	 These errors mean FS contains a bug.
   */
  if (user_phys == 0)
	  panic("FS gave winchester disk driver bad addr", (int) vir);
  top_end = (int) (((user_phys + ct - 1) >> 16) & BYTE);
  if (top_end != top_addr) panic("Trying to DMA across 64K boundary", top_addr);

  /* Now set up the DMA registers. */
  out_byte(DMA_M2, mode);	/* set the DMA mode */
  out_byte(DMA_M1, mode);	/* set it again */
  out_byte(DMA_ADDR, low_addr);	/* output low-order 8 bits */
  out_byte(DMA_ADDR, high_addr);/* output next 8 bits */
  out_byte(DMA_TOP, top_addr);	/* output highest 4 bits */
  out_byte(DMA_COUNT, low_ct);	/* output low 8 bits of count - 1 */
  out_byte(DMA_COUNT, high_ct);	/* output high 8 bits of count - 1 */
}

/*===========================================================================*
 *				w_transfer				     * 
 *===========================================================================*/
PRIVATE int w_transfer(wn)
struct wini *wn;	/* pointer to the drive struct */
{
register int i;
message dummy;

  if (w_mess.m_type == DISK_READ)
	set_command(WIN_READ, wn);		/* build command table */
  else
	set_command(WIN_WRITE, wn);

  if (com_out(6, CCB) != OK) return(ERR);	/* output command table */

  for (i = 0; i < MAX_WIN_RETRY; i++) {
	if (in_byte(ASR) & IR) break;		/* interrupt request */
	milli_delay(20);
  }

  if (i == MAX_WIN_RETRY) {
	w_need_reset = TRUE;
	return(ERR);
  }

 if (win_results() != OK) {
	w_need_reset = TRUE;
	return(ERR);
  }
 
  w_dma_setup();		/* set up dma controller */

  out_byte(ACR, 3);		/* enable interrupts and dma */
  out_byte(DMA_INIT, 3);	/* initialize DMA */

  if (com_out(0, DR) != OK) return(ERR);	/* ask for data transfer */

  cim_xt_wini(); 		/* ready for XT wini interrupts */
  receive(HARDWARE, &dummy);
  lock();
  out_byte(INT_CTLMASK, in_byte(INT_CTLMASK) | (1 << (XT_WINI_IRQ & 0x07)));
				/* disable XT wini interrupts again */	
  unlock();
  out_byte(ACR, 0);		/* disable interrupt and dma */

  if (win_results() != OK) {
	w_need_reset = TRUE;
	return(ERR);
  }

  return(OK);
}

/*===========================================================================*
 *				w_reset					     * 
 *===========================================================================*/
PRIVATE int w_reset()
{
/* Issue a reset to the controller.  This is done after any catastrophe,
 * like the controller refusing to respond.
 */

  int i;

  out_byte(ACR, 0x80);	/* Strobe reset bit high. */
  out_byte(ACR, 0);	/* Strobe reset bit low. */

  for (i = 0; i < MAX_WIN_RETRY; i++) {
	if((status() & IR) == IR) break;
		milli_delay(20);
  }
  if (i == MAX_WIN_RETRY) {
	printf("Winchester won't reset\n");
	return(ERR);
  }

  /* Reset succeeded.  Tell WIN drive parameters. */
  if (win_init() != OK) {		/* Initialize the controler */
	printf("Winchester wouldn't accept parameters\n");
	return(ERR);
  }
  
  w_need_reset = FALSE;
  return(OK);
}

/*===========================================================================*
 *				win_init				     * 
 *===========================================================================*/
PRIVATE int win_init()
{
/* Routine to initialize the drive parameters after boot or reset */

register int i;

  command[0] = CSB0;		/* set command bytes */
  for (i = 1; i < 14; i++)
	command[i] = 0;

  if (com_out(14, CSB) != OK) {	/* Output command block */
	printf("Can't output command block to winchester controler\n");
	return(ERR);
  }

  out_byte(ACR, 0);	/* no interrupts and no dma */
  return(OK);
}

/*============================================================================*
 *				win_results				      *
 *============================================================================*/
PRIVATE int win_results()
{
/* Routine to check if controller has done the operation succesfully */
  
  if ((in_byte(ISR) & 0xFD) != 0) return(ERR);
  return(OK);
}
  
/*============================================================================*
 *				set_command					      *
 *============================================================================*/
PRIVATE void set_command(r_w, wn)
char r_w;
register struct wini *wn;
{
/* Set command block to read or write */
long sector;
unsigned rw_sector, cylinder, head;

  sector = w_mess.POSITION/SECTOR_SIZE;
  sector += wn->wn_low;
  cylinder = sector / (wn->wn_heads * wn->wn_maxsec);
  rw_sector =  (sector % wn->wn_maxsec) + 1;
  head = (sector % (wn->wn_heads * wn->wn_maxsec) )/wn->wn_maxsec;

  command[0] = r_w;		/* WIN_READ or WIN_WRITE */
  command[1] = ((head << 4) & 0xF0) | ((cylinder >> 8) & 0x03);
  command[2] = cylinder;
  command[3] = rw_sector;
  command[4] = 2;
  command[5] = BLOCK_SIZE/SECTOR_SIZE;		/* Number of sectors */
}

/*============================================================================*
 *				com_out					      *
 *============================================================================*/
PRIVATE int com_out(nr_bytes, attention) 
int nr_bytes;
int attention;
{

/* Output the command block to the winchester controller and return status */

register int i, j;

  out_byte(ATT_REG, attention);		/* get controler's attention */

  if (nr_bytes == 0) return(OK);

  for (i = 0; i < nr_bytes; i++) {	/* output command block */
	for (j = 0; j < MAX_WIN_RETRY; j++)	/* wait for data request */
		if (status() & DATA_REQUEST) break;

	if (j == MAX_WIN_RETRY) {
		w_need_reset = TRUE;
		return(ERR);
	}
	out_byte(DATA, (int) command[i]);
  }

  for (i = 0; i < MAX_WIN_RETRY; i++) {
	if ((status() & BUSY) != BUSY) break;
	milli_delay(20);
  }
  if (i == MAX_WIN_RETRY) {
	w_need_reset = TRUE;
	return(ERR);
  }

  return(OK);
}

/*============================================================================*
 *				status					      *
 *============================================================================*/
PRIVATE int status()
{
/* Get status of the controler */

  return in_byte(ASR);
}

/*============================================================================*
 *				init_params				      *
 *============================================================================*/
PRIVATE void init_params()
{
/* This routine is called at startup to initialize the partition table,
 * the number of drives and the controller
*/
unsigned int i, segment, offset;
phys_bytes address;

  /* Copy the parameter vector from the saved vector table */
  offset = vec_table[2 * 0x41];
  segment = vec_table[2 * 0x41 + 1];

  /* Calculate the address off the parameters and copy them to buf */
  address = hclick_to_physb(segment) + offset;
  phys_copy(address, umap(proc_ptr, D, (vir_bytes)buf, 16), 16L);

  /* Copy the parameters to the structures */
  copy_params(buf, &wini[0]);

  /* Copy the parameter vector from the saved vector table */
  offset = vec_table[2 * 0x46];
  segment = vec_table[2 * 0x46 + 1];

  /* Calculate the address off the parameters and copy them to buf */
  address = hclick_to_physb(segment) + offset;
  phys_copy(address, umap(proc_ptr, D, (vir_bytes)buf, 16), 16L);

  /* Copy the parameters to the structures */
  copy_params(buf, &wini[5]);

  /* Get the nummer of drives from the bios */
  phys_copy(0x475L, umap(proc_ptr, D, (vir_bytes)buf, 1), 1L);
  nr_drives = (int) *buf;
  if (nr_drives > MAX_DRIVES) nr_drives = MAX_DRIVES;

  /* Set the parameters in the drive structure */
  wini[0].wn_low = wini[5].wn_low = 0L;

  ch_select();	/* select fixed disk chip */
  win_init();	/* output parameters to controler */
  ch_unselect();

  /* Read the partition table for each drive and save them */
  for (i = 0; i < nr_drives; i++) {
	w_mess.DEVICE = i * 5;
	w_mess.POSITION = 0L;
	w_mess.COUNT = BLOCK_SIZE;
	w_mess.ADDRESS = (char *) buf;
	w_mess.PROC_NR = WINCHESTER;
	w_mess.m_type = DISK_READ;
	if (w_do_rdwt(&w_mess) != BLOCK_SIZE) {
		printf("Can't read partition table on winchester %d\n",i);
		milli_delay(20000);
		continue;
	}
	if (buf[510] != 0x55 || buf[511] != 0xAA) {
		printf("Invalid partition table on winchester %d\n",i);
		milli_delay(20000);
		continue;
	}
	copy_prt((int)i*5);
  }
}

/*============================================================================*
 *				copy_params				      *
 *============================================================================*/
PRIVATE void copy_params(src, dest)
register unsigned char *src;
register struct wini *dest;
{
/* This routine copies the parameters from src to dest
 * and sets the parameters for partition 0 and 5
*/
  register int i;
  long cyl, heads, sectors;

  for (i=0; i<5; i++) {
	dest[i].wn_heads = (int)src[2];
	dest[i].wn_maxsec = (int)src[14];
  }
  cyl = (long)(*(u16_t *)src);
  heads = (long)dest[0].wn_heads;
  sectors = (long)dest[0].wn_maxsec;
  dest[0].wn_size = cyl * heads * sectors;
}

/*===========================================================================*
 *				copy_prt				     *
 *===========================================================================*/
PRIVATE void copy_prt(base_dev)
int base_dev;			/* base device for drive */
{
/* This routine copies the partition table for the selected drive to
 * the variables wn_low and wn_size
 */

  register struct part_entry *pe;
  register struct wini *wn;

  for (pe = (struct part_entry *) &buf[PART_TABLE_OFF],
       wn = &wini[base_dev + 1];
       pe < ((struct part_entry *) &buf[PART_TABLE_OFF]) + NR_PARTITIONS;
       ++pe, ++wn) {
	wn->wn_low = pe->lowsec;
	wn->wn_size = pe->size;

	/* Adjust low sector to a multiple of (BLOCK_SIZE/SECTOR_SIZE) for old
	 * Minix partitions only.  We can assume the ratio is 2 and round to
	 * even, which is slightly simpler.
	 */
	if (pe->sysind == OLD_MINIX_PART && wn->wn_low & 1) {
		++wn->wn_low;
		--wn->wn_size;
	}
  }
  sort(&wini[base_dev + 1]);
}

/*===========================================================================*
 *					sort				     *
 *===========================================================================*/
PRIVATE void sort(wn)
register struct wini wn[];
{
  register int i,j;
  struct wini tmp;

  for (i = 0; i < NR_PARTITIONS; i++)
	for (j = 0; j < NR_PARTITIONS-1; j++)
		if ((wn[j].wn_low == 0 && wn[j+1].wn_low != 0) ||
		    (wn[j].wn_low > wn[j+1].wn_low && wn[j+1].wn_low != 0)) {
			tmp = wn[j];
			wn[j] = wn[j+1];
			wn[j+1] = tmp;
		}
}

