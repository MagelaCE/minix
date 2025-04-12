| This file contains a number of assembly code utility routines needed by the
| kernel.  They are:
|
|   cp_mess:	copies messages from source to destination
|   lock:	disable interrupts
|   restore:	restore interrupts (enable/disabled) as they were before lock()
|   build_sig:	build 4 word structure pushed onto stack for signals
|   csv:	procedure prolog to save the registers
|   cret:	procedure epilog to restore the registers
|   get_chrome:	returns 0 if display is monochrome, 1 if it is color
|   get_ega:	returns 1 if display is EGA, 0 otherwise
|   vid_copy:	copy data to video ram (on color display during retrace only)
|   scr_up:	scroll screen a line up (in software, by copying)
|   scr_down:	scroll screen a line down (in software, by copying)
|   reboot:	reboot for CTRL-ALT-DEL
|   wreboot:	wait for character then reboot 
|   wait_retrace: waits for retrace interval, and returns int disabled
|   ack_char:	acknowledge character from keyboard
|   save_tty_vec: save tty interrupt vector 0x71 for PS/2

| These routines are new or totally rewritten and/or renamed compatibility
| with protected mode.

|   check_mem:	check a block of memory, return the valid size
|   cim_at_wini:	clear the AT winchester interrupt mask
|   cim_floppy:		clear the floppy interrupt mask
|   cim_printer:	clear the printer interrupt mask
|   cim_xt_wini:	clear the XT winchester interrupt mask
|   codeseg:	return the current code segment
|   dataseg:	return the current data segment
|   get_extmemsize:	ask the BIOS how much extended memory there is
|   get_memsize:	ask the BIOS how much normal memory there is
|   get_phys_byte:	read a byte from memory and return it
|   get_processor:	return the processor type
|   inportb:	read a byte from a port and return it
|   phys_copy:	copy data from anywhere to anywhere in memory
|   porti_out:	set a port-index pair, for hardware like 6845's
|   port_read:	transfer data from (hard disk controller) port to memory
|   port_write:	transfer data from memory to (hard disk controller) port
|   sim_printer:	set the printer interrupt mask
|   tasim_printer:	test and set the printer interrupt mask
|   test_and_set:	test and set locking primitive on a word in memory
|   unlock:	enable interrupts

| Phys_copy was rewritten because the old one was contorted and slow to start.
| Get_phys_byte replaces get_byte, with a new interface.
| Inportb is in addition to port_in, with a new interface.
| Port_read/write replace dma_read/write, with a new interface.

#include "../h/const.h"
#include "const.h"
#include "sconst.h"
#define KLIB88 .define
#include "sglo.h"

.text
|*===========================================================================*
|*				cp_mess					     *
|*===========================================================================*
| This routine makes a fast copy of a message from anywhere in the address
| space to anywhere else.  It also copies the source address provided as a
| parameter to the call into the first word of the destination message.
| It is called by:
|    cp_mess(src, src_clicks, src_offset, dst_clicks, dst_offset)
| where all 5 parameters are shorts (16-bits).
|
| Note that the message size, 'Msize' is in WORDS (not bytes) and must be set
| correctly.  Changing the definition of message in the type file and not
| changing it here will lead to total disaster.
| This routine only preserves the registers the 'C' compiler
| expects to be preserved (es, ds, si, di, sp, bp).

_cp_mess:
	push es			| save es
	push ds			| save ds
	mov bx,sp		| index off bx because machine can't use sp
	push si			| save si
	push di			| save di

	mov	ax,12(bx)	| destination click
#if HCLICK_SHIFT > CLICK_SHIFT
#error /* Small click sizes are not supported (right shift will lose bits). */
#endif
#if HCLICK_SHIFT < CLICK_SHIFT
	movb	cl,#CLICK_SHIFT-HCLICK_SHIFT
	shl	ax,cl		| destination segment
#endif
	mov	es,ax
	mov	di,14(bx)	| offset of destination message

| Be careful not to destroy ds before we're finished with the bx pointer.
| We're using bx and not the more natural bp to save pushing bp.

	mov	ax,6(bx)	| process number of sender
	mov	si,10(bx)	| offset of source message
	mov	bx,8(bx)	| source click (finished with bx as a pointer)
#if HCLICK_SHIFT < CLICK_SHIFT
	shl	bx,cl		| source segment
#endif
	mov	ds,bx

	stow			| copy sender's process number to dest message
	add si,*2		| don't copy first word
	mov cx,*Msize-1		| remember, first word doesn't count
	rep			| iterate cx times to copy 11 words
	movw			| copy the message
	pop di			| restore di
	pop si			| restore si
	pop ds			| restore ds
	pop es			| restore es	
	ret			| that's all folks!


|*===========================================================================*
|*				lock					     *
|*===========================================================================*
| Disable CPU interrupts.  Return old psw as function value.
_lock:
	pushf			| save flags on stack
	cli			| disable interrupts
	pop ax	 		| return flags for restoration later
	ret			| return to caller


|*===========================================================================*
|*				restore					     *
|*===========================================================================*
| restore enable/disable bit to the value it had before last lock.
_restore:
	push bp			| save it
	mov bp,sp		| set up base for indexing
	push 4(bp)		| bp is the psw to be restored
	popf			| restore flags
	pop bp			| restore bp
	ret			| return to caller


|*===========================================================================*
|*				build_sig				     *
|*===========================================================================*
|* Build a structure that is pushed onto the stack for signals.  It contains
|* pc, psw, etc., and is machine dependent. The format is the same as generated
|* by hardware interrupts, except that after the "interrupt", the signal number
|* is also pushed.  The signal processing routine within the user space first
|* pops the signal number, to see which function to call.  Then it calls the
|* function.  Finally, when the function returns to the low-level signal
|* handling routine, control is passed back to where it was prior to the signal
|* by executing a return-from-interrupt instruction, hence the need for using
|* the hardware generated interrupt format on the stack.  The call is:
|*     build_sig(sig_stuff, rp, sig)

_build_sig:
	push bp			| save bp
	mov bp,sp		| set bp to sp for accessing params
	push bx			| save bx
	push si			| save si
	mov bx,4(bp)		| bx points to sig_stuff
	mov si,6(bp)		| si points to proc table entry
	mov ax,8(bp)		| ax = signal number
	mov (bx),ax		| put signal number in sig_stuff
	mov ax,PCREG(si)	| ax = signalled process' PC
	mov 2(bx),ax		| put pc in sig_stuff
	mov ax,CSREG(si)	| ax = signalled process' cs
	mov 4(bx),ax		| put cs in sig_stuff
	mov ax,PSWREG(si)	| ax = signalled process' PSW
	mov 6(bx),ax		| put psw in sig_stuff
	pop si			| restore si
	pop bx			| restore bx
	pop bp			| restore bp
	ret			| return to caller


|*===========================================================================*
|*				csv & cret				     *
|*===========================================================================*
| This version of csv replaces the standard one.  It checks for stack overflow
| within the kernel in a simpler way than is usually done. cret is standard.
csv:
	pop bx			| bx = return address
	push bp			| stack old frame pointer
	mov bp,sp		| set new frame pointer to sp
	push di			| save di
	push si			| save si
	sub sp,ax		| ax = # bytes of local variables
	cmp sp,splimit		| has kernel stack grown too large
	jbe csv.1		| if sp is too low, panic
	jmpreg (bx)		| normal return: copy bx to program counter

csv.1:
	sub ax,ax		| zero
	mov splimit,ax		| prevent call to panic from aborting in csv
	mov bx,_proc_ptr	| update rp->p_splimit
	mov P_SPLIMIT(bx),ax	| rp->sp_limit = 0
	push P_NR(bx)		| task number
	mov ax,#stkoverrun	| stack overran the kernel stack area
	push ax			| push first parameter
	call _panic		| call is: panic(stkoverrun, task number)
	j csv.1			| this should not be necessary

cret:
	lea	sp,*-4(bp)	| set sp to point to saved si
	pop	si		| restore saved si
	pop	di		| restore saved di
	pop	bp		| restore bp
	ret			| end of procedure

|*===========================================================================*
|*				get_chrome				     *
|*===========================================================================*
| This routine calls the BIOS to find out if the display is monochrome or 
| color.  The drivers are different, as are the video ram addresses, so we
| need to know.
_get_chrome:
	int 0x11		| call the BIOS to get equipment type
	andb al,#0x30		| isolate color/mono field
	cmpb al,*0x30		| 0x30 is monochrome
	je getchr1		| if monochrome then go to getchr1
	mov ax,#1		| color = 1
	ret			| color return
getchr1: xor ax,ax		| mono = 0
	ret			| monochrome return

|*===========================================================================*
|*				get_ega  				     *
|*===========================================================================*
| This routine calls the BIOS to find out if the display is ega.  This
| is needed because scrolling is different.
_get_ega:
	movb bl,*0x10
	movb ah,*0x12
	int 0x10		| call the BIOS to get equipment type

	cmpb bl,*0x10		| if reg is unchanged, it failed
	je notega
	mov ax,#1		| color = 1
	ret			| color return
notega: xor ax,ax		| mono = 0
	ret			| monochrome return


|*===========================================================================*
|*				vid_copy				     *
|*===========================================================================*
| This routine takes a string of (character, attribute) pairs and writes them
| onto the screen.  For a color display, the writing only takes places during
| the vertical retrace interval, to avoid displaying garbage on the screen.
| The call is:
|     vid_copy(buffer, videobase, offset, words)
| where
|     'buffer'    is a pointer to the (character, attribute) pairs
|     'videobase' is 0xB800 for color and 0xB000 for monochrome displays
|     'offset'    tells where within video ram to copy the data
|     'words'     tells how many words to copy
| if buffer is zero, the fill char (blank_color) is used

_vid_copy:
	push bp			| we need bp to access the parameters
	mov bp,sp		| set bp to sp for indexing
	push si			| save the registers
	push di			| save di
	push bx			| save bx
	push cx			| save cx
	push dx			| save dx
	push es			| save es
vid.0:	mov si,4(bp)		| si = pointer to data to be copied
	mov es,6(bp)		| load es NOW: int routines may NOT ruin it
	mov di,8(bp)		| di = offset within video ram
	and di,_vid_mask	| only 4K or 16K counts
	mov cx,10(bp)		| cx = word count for copy loop
	mov dx,#0x3DA		| prepare to see if color display is retracing

	mov bx,di		| see if copy will run off end of video ram
	add bx,cx		| compute where copy ends
	add bx,cx		| bx = last character copied + 1
	sub bx,_vid_mask	| bx = # characters beyond end of video ram
	sub bx,#1		| note: dec bx doesn't set flags properly
				| it DOES for jle!!
	jle vid.1		| jump if no overrun
	sar bx,#1		| bx = # words that don't fit in video ram
	sub cx,bx		| reduce count by overrun

vid.1:	push cx			| save actual count used for later
	testb _snow,#1		| skip vertical retrace test if no snow
	jz vid.4

|vid.2:	in			| with a color display, you can only copy to
|	test al,*010		| the video ram during vertical retrace, so
|	jnz vid.2		| wait for start of retrace period.  Bit 3 of
vid.3:	in			| 0x3DA is set during retrace.  First wait
	testb al,*010		| until it is off (no retrace), then wait
	jz vid.3		| until it comes on (start of retrace)

vid.4:	cmp si,#0		| si = 0 means blank the screen
	je vid.7		| jump for blanking
	lock			| this is a trick for the IBM PC simulator only
	inc vidlock		| 'lock' indicates a video ram access
	rep			| this is the copy loop
	movw			| ditto

vid.5:	pop si			| si = count of words copied
	cmp bx,#0		| if bx < 0, then no overrun and we are done
	jle vid.6		| jump if everything fit
	mov 10(bp),bx		| set up residual count
	mov 8(bp),#0		| start copying at base of video ram
	cmp 4(bp),#0		| NIL_PTR means store blanks
	je vid.0		| go do it
	add si,si		| si = count of bytes copied
	add 4(bp),si		| increment buffer pointer
	j vid.0			| go copy some more

vid.6:	pop es			| restore registers
	pop dx			| restore dx
	pop cx			| restore cx
	pop bx			| restore bx
	pop di			| restore di
	pop si			| restore si
	pop bp			| restore bp
	ret			| return to caller

vid.7:	mov ax,_blank_color	| ax = blanking character
	rep			| copy loop
	stow			| blank screen
	j vid.5			| done

|*===========================================================================*
|*			      wait_retrace				     *
|*===========================================================================*
| Wait until we're in the retrace interval.  Return locked (ints off).
| But enable them during the wait.

_wait_retrace: push dx
	pushf
	mov dx,_vid_port
	or dx,#0x0A
wtre.3:	sti
	nop
	nop
	cli	
	in			| 0x3DA bit 3 is set during retrace.
	testb al,*010		| Wait until it is on.
	jz wtre.3

	pop ax	 		| return flags for restoration later
	pop dx
	ret			| return to caller

|*===========================================================================*
|*				scr_up  				     *
|*===========================================================================*
| This routine scrolls the screen up one line
| 
| The call is:
|     scr_up(videoseg,source,dest,count)
| where
|     'videoseg'	is the segment of screen memory

_scr_up:
	push bp			| we need bp to access the parameters
	mov bp,sp		| set bp to sp for indexing
	push si			| save the registers
	push di			| save di
	push cx			| save cx
	push es			| save es
	push ds			| save ds
	mov si,6(bp)		| si = pointer to data to be copied
	mov di,8(bp)		| di = offset within video ram
	mov cx,10(bp)		| cx = word count for copy loop

	mov ax,4(bp)		| set source and target segments to videoseg
	mov ds,ax
	mov es,ax

	rep			| this is the copy loop
	movw			| ditto

	pop ds			| restore ds
	pop es			| restore es
	pop cx			| restore cx
	pop di			| restore di
	pop si			| restore si
	pop bp			| restore bp
	ret			| return to caller

|*===========================================================================*
|*				  scr_down				     *
|*===========================================================================*
| This routine scrolls the screen down one line
| 
| The call is:
|     scr_down(vidoeseg,source,dest,count)
| where
|     'videoseg'	is the segment of screen memory

_scr_down:
	push bp			| we need bp to access the parameters
	mov bp,sp		| set bp to sp for indexing
	push si			| save the registers
	push di			| save di
	push cx			| save cx
	push es			| save es
	push ds			| save ds
	mov si,6(bp)		| si = pointer to data to be copied
	mov di,8(bp)		| di = offset within video ram
	mov cx,10(bp)		| cx = word count for copy loop

	mov ax,4(bp)		| set source and target segments to videoseg
	mov ds,ax
	mov es,ax

	std			| reverse to avoid propagating 1st word
	rep			| this is the copy loop
	movw			| ditto

	cld			| restore direction flag to known state
	pop ds			| restore ds
	pop es			| restore es
	pop cx			| restore cx
	pop di			| restore di
	pop si			| restore si
	pop bp			| restore bp
	ret			| return to caller


|===========================================================================
|                		em_xfer
|===========================================================================
|
|  This file contains one routine which transfers words between user memory
|  and extended memory on an AT or clone.  A BIOS call (INT 15h, Func 87h)
|  is used to accomplish the transfer.  The BIOS call is "faked" by pushing
|  the processor flags on the stack and then doing a far call through the
|  saved vector table to the actual BIOS location.  An actual INT 15h would
|  get a MINIX complaint from an unexpected trap.

|  This particular BIOS routine runs with interrupts off since the 80286
|  must be placed in protected mode to access the memory above 1 Mbyte.
|  So there should be no problems using the BIOS call, except it may take
|  too long and cause interrupts to be lost.
|
	.text
gdt:				| Begin global descriptor table
					| Dummy descriptor
	.word 0		| segment length (limit)
	.word 0		| bits 15-0 of physical address
	.byte 0		| bits 23-16 of physical address
	.byte 0		| access rights byte
	.word 0		| reserved
					| descriptor for GDT itself
	.word 0		| segment length (limit)
	.word 0		| bits 15-0 of physical address
	.byte 0		| bits 23-16 of physical address
	.byte 0		| access rights byte
	.word 0		| reserved
src:					| source descriptor
srcsz:	.word 0		| segment length (limit)
srcl:	.word 0		| bits 15-0 of physical address
srch:	.byte 0		| bits 23-16 of physical address
	.byte 0x93	| access rights byte
	.word 0		| reserved
tgt:					| target descriptor
tgtsz:	.word 0		| segment length (limit)
tgtl:	.word 0		| bits 15-0 of physical address
tgth:	.byte 0		| bits 23-16 of physical address
	.byte 0x93	| access rights byte
	.word 0		| reserved
					| BIOS CS descriptor
	.word 0		| segment length (limit)
	.word 0		| bits 15-0 of physical address
	.byte 0		| bits 23-16 of physical address
	.byte 0		| access rights byte
	.word 0		| reserved
					| stack segment descriptor
	.word 0		| segment length (limit)
	.word 0		| bits 15-0 of physical address
	.byte 0		| bits 23-16 of physical address
	.byte 0		| access rights byte
	.word 0		| reserved

|
|
|  Execute a transfer between user memory and extended memory.
|
|  status = em_xfer(source, dest, count);
|
|    Where:
|       status => return code (0 => OK)
|       source => Physical source address (32-bit)
|       dest   => Physical destination address (32-bit)
|       count  => Number of words to transfer
|
|
|
_em_xfer:
	push	bp		| Save registers
	mov	bp,sp
	push	si
	push	es
	push	cx
|
|  Pick up source and destination addresses and update descriptor tables
|
	mov ax,4(bp)
	seg cs
	mov srcl,ax
	mov ax,6(bp)
	seg cs
	movb srch,al
	mov ax,8(bp)
	seg cs
	mov tgtl,ax
	mov ax,10(bp)
	seg cs
	movb tgth,al
|
|  Update descriptor table segment limits
|
	mov cx,12(bp)
	mov ax,cx
	add ax,ax
	seg cs
	mov tgtsz,ax
	seg cs
	mov srcsz,ax

| The BIOS call doesn't preserve the flags!!!
| Worse, it enables interrupts internally.
| This mainly hurts the call from db via p_dmp() and phys_copy(), which
| is only invoked for protected mode.
| Disable interrupts using the interrupt controller, and save the flags.
| On 386's, the extended flags and registers are destroyed.

	in	INT_CTLMASK
	push	ax
	movb	al,#0xFF	| mask everything
	out	INT_CTLMASK
	pushf

|
|  Now do actual DOS call
|
	push cs
	pop es
	mov si,#gdt
	movb ah,#EM_XFER_FUNC
	pushf			| fake interrupt
	callfarptr(_vec_table+4*EM_XFER_VEC)

| Restore flags and interrupt controller.

	popf
	pop	ax
	out	INT_CTLMASK

|
|  All done, return to caller.
|

	pop	cx		| restore registers
	pop	es
	pop	si
	mov	sp,bp
	pop	bp
	ret

|*===========================================================================
|*				ack_char
|*===========================================================================
| Acknowledge character from keyboard for PS/2

_ack_char:
	push dx
	mov dx,#0x69
	in
	xor ax,#0x10
	out
	xor ax,#0x10
	out

	mov dx,#0x66
	movb ah,#0x10
	in
	notb ah
	andb al,ah
	out
	jmp frw1
frw1:	notb ah
	orb al,ah
	out
	jmp frw2
frw2:	notb ah
	andb al,ah
	out
	
	pop dx
	ret


|*===========================================================================*
|*				save_tty_vec				     *
|*===========================================================================*
| Save the tty vector 0x71 (PS/2)
_save_tty_vec:
	push es
	xor ax,ax
	mov es,ax
	seg es
	mov ax,452
	mov tty_vec1,ax
	seg es
	mov ax,454
	mov tty_vec2,ax
	pop es
	ret


|*===========================================================================*
|*				reboot & wreboot			     *
|*===========================================================================*
| This code reboots the PC

_reboot:
	cli			| disable interrupts
	mov ax,#0x20		| re-enable interrupt controller
	out 0x20
	call _eth_stp		| stop the ethernet chip

	cmp	_pc_at,#0
	jz	old_reboot
new_reboot:
	sub	bx,bx		| phys_copy(reboot_magic, 0x472L, 2L)
	mov	ax,#2		| to stop memory test
	push	bx		| can't load ds = 0x40 in protected mode
	push	ax		| push 2L
	mov	ax,#0x472
	push	bx		| push 0x472L
	push	ax
	seg	cs
	mov	ax,kernel_ds	| need real ds not necessarily current ds
	movb	cl,#HCLICK_SHIFT  | calculate bx:ax = ax * HCLICK_SHIFT
	rol	ax,cl
	movb	bl,al
	andb	bl,#HCHIGH_MASK
	andb	al,#HCLOW_MASK
	add	ax,#reboot_magic  | then bx:ax += &reboot_magic
	adc	bx,#0
	push	bx
	push	ax
	call	_phys_copy
	movb	al,#0xFE	| complemented 0x01 bit
	out	0x64		| tells keyboard controller to reset processor

| That should do it for AT's.  A solution for PS/2's remains to be found.

old_reboot:
	call resvec		| restore the vectors in low core
into_reboot:
	mov ax,#0x40
	push ds
	mov ds,ax
	mov ax,#0x1234
	mov 0x72,ax
	pop ds
	test _ps,#0xFFFF
	jnz r.1
	mov ax,#0xFFFF
	mov ds,ax
	mov ax,3
	push ax
	mov ax,1
	push ax
	reti
r.1:
	mov ax,_port_65		| restore port 0x65
	mov dx,#0x65
	out
	mov dx,#0x21		| restore interrupt mask port
	mov ax,#0xBC
	out
	sti			| enable interrupts
	int 0x19		| for PS/2 call bios to reboot

_wreboot:
	cli			| disable interrupts
	mov ax,#0x20		| re-enable interrupt controller
	out 0x20
	call _eth_stp		| stop the ethernet chip

	cmp	_pc_at,#0
	jz	old_wreboot
	call	_scan_keyboard	| ack any old input
waitkey:
	in	0x64		| test this keyboard status port
	testb	al,#0x01	| this bit is set when data is ready
	jz	waitkey
	j	new_reboot

old_wreboot:
	call resvec		| restore the vectors in low core
	mov	ax,#0x70	| restore the standard interrupt bases
	push	ax
	mov	ax,#8
	push	ax
	call	_init_8259
	pop	ax
	pop	ax
	movb	al,#notop(KEYBOARD_MASK)	| allow keyboard int (only)
	out	INT_CTLMASK	| after sti in int 0x16
	xor ax,ax		| wait for character before continuing
	int 0x16		| get char
	j into_reboot

| Restore the interrupt vectors in low core.
resvec:
	mov cx,#2*71
	mov si,#_vec_table
	xor di,di
	mov es,di
	rep
	movw

	mov ax,tty_vec1		| Restore keyboard interrupt vector for PS/2
	seg es
	mov 452,ax
	mov ax,tty_vec2
	seg es
	mov 454,ax

	ret

| Some library routines use exit, so this label is needed.
| Actual calls to exit cannot occur in the kernel.
| Same for .fat
| Same for .trp.

.fat:
.trp:
_exit:	sti
	j _exit

.data
vidlock:	.word 0		| dummy variable for use with lock prefix
splimit:	.word 0		| stack limit for current task (kernel only)
stkoverrun:	.ascii "Kernel stack overrun, task = "
		.byte 0
em_bad:		.ascii "Bad call to phys_copy with extended memory, error "
		.byte 0
		.space 3	| align
_vec_table:	.space VECTOR_BYTES	| storage for interrupt vectors
tty_vec1:	.word 0		| storage for vector 0x71 (offset)
tty_vec2:	.word 0		| storage for vector 0x71 (segment)
reboot_magic:	.word 0x1234	| to stop memory test
		.space 2	| align

	.text
|*===========================================================================*
|*				check_mem				     *
|*===========================================================================*

| PUBLIC phys_bytes check_mem(phys_bytes base, phys_bytes size);
| Check a block of memory, return the valid size.
| Only every 16th byte is checked.
| The memory and initial size must be <= 1M for non-protected mode.
| An initial size of 0 means everything.
| This really should do some alias checks.

_check_mem:
	push	bp
	mov	bp,sp
	push	ds
	mov	dx,4+2(bp)	| base in dx:ax
	sub	ax,ax		| prepare for early exit
	test	dx,#notop(HCHIGH_MASK)
	jnz	cm_1exit	| can't handle bases above 1M
	mov	ax,4(bp)	| ax = base segment = base / 16 % 0x10000
	andb	al,#HCLOW_MASK
	orb	al,dl
	movb	cl,#HCLICK_SHIFT
	ror	ax,cl
	mov	bx,4+4(bp)	| size in dx:bx
	mov	dx,4+4+2(bp)
	test	dx,#notop(HCHIGH_MASK)
	jz	over_cm_reduce
	movb	dl,#HCHIGH_MASK
	mov	bx,#0xFFFF
over_cm_reduce:
	andb	bl,#HCLOW_MASK	| cx = size in hclicks = size / 16 % 0x10000
	orb	bl,dl
	ror	bx,cl
	mov	cx,bx
	push	cx		| save size in clicks
	mov	bx,4(bp)	| bx = base offset = base % 16
	and	bx,#OFF_MASK
cm_loop:
	mov	ds,ax
	movb	dl,#TEST1PATTERN
	xchgb	dl,(bx)		| write test pattern, remember original value
	xchgb	dl,(bx)		| restore original value, read test pattern
	cmpb	dl,#TEST1PATTERN	| must agree if good real memory
	jnz	cm_exit		| if different, memory is unusable
	movb	dl,#TEST2PATTERN
	xchgb	dl,(bx)
	xchgb	dl,(bx)
	cmpb	dl,#TEST2PATTERN
	jnz	cm_exit
	inc	ax		| next segment, test for wraparound at 1M
	loopnz	cm_loop
cm_exit:
	pop	ax
	sub	ax,cx		| verified size in phys_clicks
cm_1exit:
	movb	dl,ah		| convert to phys_bytes in dx:ax
	movb	cl,#HCLICK_SHIFT
	shl	ax,cl
	shr	dx,cl
	and	dx,#HCHIGH_MASK
	pop	ds
	pop	bp
	ret


|*===========================================================================*
|*				cim_at_wini				     *
|*				cim_floppy				     *
|*				cim_printer				     *
|*				cim_xt_wini				     *
|*===========================================================================*

| All these routines are meant to be called from the task level where
| interrupts should not be globally disabled, so they return with interrupts
| enabled.

| PUBLIC void cim_at_wini();
| Clear the AT winchester interrupt mask.

_cim_at_wini:
	cli
	in	INT2_MASK
	andb	al,#notop(AT_WINI_MASK)
	out	INT2_MASK
	sti
	ret

| PUBLIC void cim_floppy();
| Clear the AT winchester interrupt mask.

.define _cim_floppy
_cim_floppy:
	cli
	in	INT_CTLMASK
	andb	al,#notop(FLOPPY_MASK)
	out	INT_CTLMASK
	sti
	ret

| PUBLIC void cim_printer();
| Clear the printer interrupt mask.

_cim_printer:
	cli
	in	INT_CTLMASK
#ifdef ASLD
	andb	al,#notop(PRINTER_MASK)
#else
	andb	al,#notop(PRINTER_MASK) & 0xFF
#endif
	out	INT_CTLMASK
	sti
	ret

| PUBLIC void cim_xt_wini();
| Clear the xt_wini interrupt mask.

_cim_xt_wini:
	cli
	in	INT_CTLMASK
	andb	al,#notop(XT_WINI_MASK)
	out	INT_CTLMASK
	sti
	ret


|*===========================================================================*
|*				get_extmemsize				     *
|*===========================================================================*

| PUBLIC void get_extmemsize();
| Ask the BIOS how much extended memory there is.

_get_extmemsize:
	movb	ah,#GET_EXTMEM_FUNC
	int	GET_EXTMEM_VEC	| returns size (in K) in ax for AT's
	ret


|*===========================================================================*
|*				get_phys_byte				     *
|*===========================================================================*

| PUBLIC int get_phys_byte(phys_bytes offset);
| Fetch a byte from anywhere in memory.
| Get_byte() was inadequate above 1MB and the segment arithmetic to set up
| its arguments used the wrong CLICK_SIZE.
| This and its protected mode version provide a uniform interface.

_get_phys_byte:
	pop	dx		| return addr
	pop	ax		| source addr in cx:ax
	pop	cx
	sub	sp,#4		| adjust for parameter popped
	mov	bx,ax		| bx = source offset = address % 16
	and	bx,#OFF_MASK
	andb	cl,#HCHIGH_MASK	| ds = source segment = address / 16 % 0x10000
	andb	al,#HCLOW_MASK
	orb	al,cl
	movb	cl,#HCLICK_SHIFT
	ror	ax,cl
	mov	cx,ds		| save ds
	mov	ds,ax
	movb	al,(bx)		| fetch the byte
	subb	ah,ah		| zero-extend to int
	mov	ds,cx
	jmpreg	(dx)


|*===========================================================================*
|*				get_memsize				     *
|*===========================================================================*

| PUBLIC void get_memsize();
| Ask the BIOS how much normal memory there is.

_get_memsize:
	int	GET_MEM_VEC	| this returns the size (in K) in ax
	ret


|*===========================================================================*
|*				phys_copy				     *
|*===========================================================================*

| PUBlIC void phys_copy( long source, long destination, long bytecount);
| Copy a block of physical memory.

DESTLO	=	8
DESTHI	=	10
SRCLO	=	4
SRCHI	=	6
COUNTLO	=	12
COUNTHI	=	14

_phys_copy:
	push	bp		| save only registers required by C
	mov	bp,sp		| set bp to point to source arg less 4

| check for extended memory

	mov	ax,SRCHI(bp)
	or	ax,DESTHI(bp)
	test	ax,#EM_MASK
	jnz	to_em_xfer

	push	si		| save si
	push	di		| save di
	push	ds		| save ds
	push	es		| save es

	mov	ax,SRCLO(bp)	| dx:ax = source address (dx is NOT segment)
	mov	dx,SRCHI(bp)
	mov	si,ax		| si = source offset = address % 16
	and	si,#OFF_MASK
|	andb	dl,#HCHIGH_MASK	| ds = source segment = address / 16 % 0x10000
				| mask is unnecessary because of EM_MASK test
	andb	al,#HCLOW_MASK
	orb	al,dl		| now bottom 4 bits of dx are in ax
	movb	cl,#HCLICK_SHIFT | rotate them to the top 4
	ror	ax,cl
	mov	ds,ax

	mov	ax,DESTLO(bp)	| dx:ax = destination addr (dx is NOT segment)
	mov	dx,DESTHI(bp)
	mov	di,ax		| di = dest offset = address % 16
	and	di,#OFF_MASK
|	andb	dl,#HCHIGH_MASK	| es = dest segment = address / 16 % 0x10000
	andb	al,#HCLOW_MASK
	orb	al,dl
	ror	ax,cl
	mov	es,ax

	mov	ax,COUNTLO(bp)	| dx:ax = remaining count
	mov	dx,COUNTHI(bp)

| copy upwards (can't handle overlapped copy)

pc_loop:
	mov	cx,ax		| provisional count for this iteration
	test	ax,ax		| if count >= 0x8000, only do 0x8000 per iter
	js	pc_bigcount	| low byte already >= 0x8000
	test	dx,dx
	jz	pc_upcount	| less than 0x8000
pc_bigcount:
	mov	cx,#0x8000	| use maximum count per iteration
pc_upcount:
	sub	ax,cx		| update count
	sbb	dx,#0		| can't underflow, so carry clear now for rcr
	rcr	cx,#1		| count in words, carry remembers if byte
	jnc	pc_even		| no odd byte
	movb			| copy odd byte
pc_even:
	rep			| copy 1 word at a time
	movw			| word copy

	mov	cx,ax		| test if remaining count is 0
	or	cx,dx
	jnz	pc_more		| more to do

	pop	es		| restore es
	pop	ds		| restore ds
	pop	di		| restore di
	pop	si		| restore si
	pop	bp		| restore bp
	ret			| return to caller

pc_more:
	sub	si,#0x8000	| adjust pointers so the offset doesn't
	mov	cx,ds		| overflow in the next 0x8000 bytes
	add	cx,#0x800	| pointers end up same physical location
	mov	ds,cx		| the current offsets are known >= 0x8000
	sub	di,#0x8000	| since we just copied that many
	mov	cx,es
	add	cx,#0x800
	mov	es,cx
	j	pc_loop		| start next iteration

to_em_xfer:			| source or target is above 1M, join em_xfer
	sub	bx,bx		| build error code here
	mov	ax,COUNTHI(bp)	| convert count to words
	rcr	ax,#1		| carry is clear from  previous test instruct
	rcr	COUNTLO(bp),#1
	pop	bp		| stack frame now agrees with em_xfer's
	jc	pc_panic	| count was odd
	inc	bx
	test	ax,ax
	jnz	pc_panic	| count is too big
	inc	bx
	cmp	_processor,#286
	jb	pc_panic	| not 286 or 386	
	jmp	_em_xfer

pc_panic:
	push	bx		| error code
	mov	ax,#em_bad	| string to print
	push	ax
	call	_panic
pc_1panic:
	j	pc_1panic	| this should not be necessary


|*===========================================================================*
|*				porti_out				     *
|*===========================================================================*

| PUBLIC void porti_out(int portpair, int indexpair, int datapair);
| Set a port-index pair. For hardware like 6845's.

_porti_out:
	pop	cx		| return adr
	pop	dx		| portpair
	pop	ax		| indexpair
	pop	bx		| datapair
	sub	sp,#6		| adjust for 3 parameters popped
	xchgb	ah,bl		| low byte of data in ah, high index in bl
	pushf			| don't let interrupt separate the halves
	cli			
	outw			| low index and data at once
				| (may depend on 8 bit bus & speed?)
	xchg	ax,bx		| high index in al, high byte of data in ah
	outw			| high index and data at once
	popf
	jmpreg	(cx)


|*===========================================================================*
|*				port_read				     *
|*===========================================================================*

| PUBLIC void port_read(port_t port, long destination, unsigned bytcount);
| Transfer data from (hard disk controller) port to memory.

_port_read:
	push	bp
	mov	bp,sp
	push	cx
	push	dx
	push	di
	push	es
	mov	ax,4+2(bp)	| destination addr in dx:ax
	mov	dx,4+2+2(bp)
	mov	di,ax		| di = dest offset = address % 16
	and	di,#OFF_MASK
	andb	dl,#HCHIGH_MASK	| es = dest segment = address / 16 % 0x10000
	andb	al,#HCLOW_MASK
	orb	al,dl
	movb	cl,#HCLICK_SHIFT
	ror	ax,cl
	mov	es,ax

	mov	cx,4+2+4(bp)	| count in bytes
	shr	cx,#1		| count in words
	mov	dx,4(bp)	| port to read from
	rep
	insw
	pop	es
	pop	di
	pop	dx
	pop	cx
	mov	sp,bp
	pop	bp
	ret


|*===========================================================================*
|*				port_write				     *
|*===========================================================================*

| PUBLIC void port_write(port_t port, long source, unsigned bytcount);
| Transfer data from memory to (hard disk controller) port.

_port_write:
	push	bp
	mov	bp,sp
	push	cx
	push	dx
	push	si
	push	ds
	mov	ax,4+2(bp)	| source addr in dx:ax
	mov	dx,4+2+2(bp)
	mov	si,ax		| di = source offset = address % 16
	and	si,#OFF_MASK
	andb	dl,#HCHIGH_MASK	| ds = source segment = address / 16 % 0x10000
	andb	al,#HCLOW_MASK
	orb	al,dl
	movb	cl,#HCLICK_SHIFT
	ror	ax,cl
	mov	ds,ax
	mov	cx,4+2+4(bp)	| count in bytes
	shr	cx,#1		| count in words
	mov	dx,4(bp)	| port to read from
	rep
	outsw
	pop	ds
	pop	si
	pop	dx
	pop	cx
	mov	sp,bp
	pop	bp
	ret


|*===========================================================================*
|*				sim_printer				     *
|*===========================================================================*

| PUBLIC void sim_printer();
| Set the printer interrupt mask.
| This is meant to be called from the task level, so it returns with
| interrupts enabled, like cim_printer().

_sim_printer:
	cli
	in	INT_CTLMASK
	orb	al,#PRINTER_MASK
	out	INT_CTLMASK
	sti
	ret


|*===========================================================================*
|*				tasim_printer				     *
|*===========================================================================*

| PUBLIC int tasim_printer();
| Set the printer interrupt mask, indivisibly with getting its old value.
| Return old value.
| Although this is meant to be called from the clock interrupt handler via
| a call to pr_restart(), it returns with interrupts enabled since the
| clock handler has them enabled.
| This might not work for multiple processors, unlike test_and_set().

_tasim_printer:
	cli
	in	INT_CTLMASK
	movb	ah,al
	orb	al,#PRINTER_MASK
	out	INT_CTLMASK
	sti
	movb	al,ah
	and	ax,#PRINTER_MASK
	ret


|*===========================================================================*
|*				test_and_set				     *
|*===========================================================================*

| PUBLIC int test_and_set( int *flag );
| Set the flag to TRUE, indivisibly with getting its old value.
| Return old flag.

_test_and_set:
	pop	dx
	pop	bx
	sub	sp,#2
	mov	ax,#1
	xchg	ax,(bx)
	jmpreg	(dx)


|*===========================================================================*
|*				unlock					     *
|*===========================================================================*

| PUBLIC void unlock();
| Enable CPU interrupts.

_unlock:
	sti			| enable interrupts
	ret			| return to caller


#ifndef DEBUGGER

|*===========================================================================*
|*				codeseg					     *
|*===========================================================================*

| PUBLIC phys_clicks codeseg();
| Return the current code segment.

_codeseg:
	mov	ax,cs
	ret


|*===========================================================================*
|*				dataseg					     *
|*===========================================================================*

| PUBLIC phys_clicks dataseg();
| Return the current data segment.

_dataseg:
	mov	ax,ds
	ret


|*===========================================================================*
|*				get_processor				     *
|*===========================================================================*

| PUBLIC unsigned get_processor();
| Decide processor type among 8088=8086, 80188=80186, 80286, 80386.
| Return 86, 186, 286 or 386.
| Preserves all registers except the flags and the return register ax.

| Method:
| 8088=8086 and 80188=80186 push sp as new sp, 80286 and 80386 as old sp.
| All but 8088=8086 do shifts mod 32 or 16.
| 386 stores 0 for the upper 8 bits of the GDT pointer in 16 bit mode,
| while 286 stores 0xFF.

_get_processor:
	push	sp
	pop	ax
	cmp	ax,sp
	jz	new_processor
	push	cx
	mov	cx,#0x0120
	shlb	ch,cl		| zero tells if 86
	pop	cx
	mov	ax,#86
	jz	got_processor
	mov	ax,#186
	ret

new_processor:
	push	bp
	mov	bp,sp
	sub	sp,#6		| space for GDT ptr
	defsgdt	(-6(bp))	| save 3 word GDT ptr
	add	sp,#4		| discard 2 words of GDT ptr
	pop	ax		| top word of GDT ptr
	pop	bp
	cmpb	ah,#0		| zero only for 386
	mov	ax,#286
	jnz	got_processor
	mov	ax,#386
got_processor:
	ret


|*===========================================================================*
|*				inportb					     *
|*===========================================================================*

| PUBLIC unsigned inportb( port_t port );
| Read an (unsigned) byte from the i/o port  port  and return it.

_inportb:
	pop	bx
	pop	dx
	dec	sp
	dec	sp
	in
	subb	ah,ah
	jmpreg	(bx)

#endif /* DEBUGGER */
