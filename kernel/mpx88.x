| This file is part of the lowest layer of the MINIX kernel.  The other parts
| are "proc.c" and protected mode version(s) of this file.  The lowest layer
| does process switching and message handling.

| Every transition to the kernel goes through this file (or a protected mode
| version).  Transitions are caused by sending/receiving messages and by most
| interrupts.  (RS232 interrupts may be handled in the file "rs2.x" and then
| they rarely enter the kernel.)

| Transitions to the kernel may be nested.  The initial entry may be with a
| system call, exception or hardware interrupt; reentries may only be made
| by hardware interrupts.  The count of reentries is kept in 'k_reenter'.
| It is important for deciding whether to switch to the kernel stack and
| for protecting the message passing code in "proc.c".

| For the message passing trap, most of the machine state is saved in the
| proc table.  (Some of the registers need not be saved.)  Then the stack is
| switched to 'k_stack', and interrupts are reenabled.  Finally, the system
| call handler (in C) is called.  When it returns, interrupts are disabled
| again and the code falls into the restart routine, to finish off held-up
| interrupts and run the process or task whose pointer is in 'proc_ptr'.

| Hardware interrupt handlers do the same, except  (1) The entire state must
| be saved.  (2) There are too many handlers to do this inline, so the save
| routine is called.  A few cycles are saved by pushing the address of the
| appropiate restart routine for a return later.  (3) A stack switch is
| avoided when the stack is already switched.  (4) The (master) 8259 interrupt
| controller is reenabled centrally in save().  (5) Each interrupt handler
| masks its interrupt line using the 8259 before enabling (other unmasked)
| interrupts, and unmasks it after servicing the interrupt.  This limits the
| nest level to the number of lines and protects the handler from itself.

| The external entry points into this file are:
|   s_call:	process or task wants to send or receive a message
|   tty_int:	interrupt routine for each key depression and release
|   rs232_int:	interrupt routine for each rs232 interrupt on port 1
|   secondary_int:	interrupt routine for each rs232 interrupt on port 2
|   lpr_int:	interrupt routine for each line printer interrupt
|   disk_int:	disk interrupt routine
|   wini_int:	winchester interrupt routine
|   clock_int:	clock interrupt routine (HZ times per second)
|   eth_int:	ethernet interrupt routine
|   int00-int16:handlers for unused interrupt vectors < 17
|   trp:	all traps with vector >= 16 are vectored here
|   restart:	start running a task or process
|   idle_task:	executed when there is no work

| and for protected mode to patch
|   save:	save the machine state in the proc table

| and for protected mode with C RS232 handlers, some duplicate labels to
| avoid #if's elsewhere:
|   prs232_int:	duplicate rs232_int
|   psecondary_int:	duplicate secondary_int

#include "../h/const.h"
#include "../h/com.h"
#include "const.h"
#include "sconst.h"
#define MPX88 .define
#include "sglo.h"

	.text

begtext:
|*===========================================================================*
|*				MINIX					     *
|*===========================================================================*
MINIX:				| this is the entry point for the MINIX kernel.
	j	over_kernel_ds	| skip over the next few bytes
	.word	CLICK_SHIFT	| for build, later used by db for syms offset
kernel_ds:
	.word	0		| build puts kernel DS here at fixed address 4
over_kernel_ds:
	cli                     | disable interrupts
	cld			| C compiler needs es = ds and direction = up

| this cli is redundant, fsck1.s already did it
| NB stack is invalid here - fsck1.s did the wrong things

| copy boot parameters to kernel data, if any

	seg	cs
	mov	es,kernel_ds
	test	ax,ax		| 0 for new boot, nonzero (scan code) for old
	jnz	over_cp_param	| skip copying parameters if old boot
	seg	es
	mov	dx,_sizeof_bparam
	cmp	cx,dx
	jle	over_adjust_param_count
	mov	cx,dx
over_adjust_param_count:
	mov	ds,di		| ds:si = parameters source
	mov	di,#_boot_parameters	| es:di = parameters target
	rep
	movb
over_cp_param:
	mov	ax,es
	mov	ds,ax		| ds now contains proper value
	mov	ss,ax		| ss now contains proper value
	mov	_scan_code,bx	| save scan code from bootstrap
	mov	sp,#k_stktop	| set sp to point to the top of kernel stack
	jmp	_main		| start the main program of MINIX
				| this should return by calling restart()


|*===========================================================================*
|*				tty_int					     *
|*===========================================================================*
_tty_int:			| Interrupt routine for terminal input.
	call	save		| save the machine state
	in	INT_CTLMASK	| mask out further keyboard interrupts
	orb	al,#KEYBOARD_MASK
	out	INT_CTLMASK
	sti			| allow unmasked, non-keyboard, interrupts
	call	_keyboard	| process a keyboard interrupt
	cli
	in	INT_CTLMASK	| unmask keyboard interrupt
	andb	al,#notop(KEYBOARD_MASK)
	out	INT_CTLMASK
	ret			| return to appropiate restart built by save()


#ifdef C_RS232_INT_HANDLERS
|*===========================================================================*
|*				rs232_int				     *
|*===========================================================================*
_rs232_int:			| Interrupt routine for rs232 I/O.
#ifdef i80286
_prs232_int:
#endif
	call	save
	in	INT_CTLMASK
	orb	al,#RS232_MASK
	out	INT_CTLMASK

| Don't enable interrupts, the handlers are not designed for it.
| The mask was set as usual so the handler can reenable interrupts as desired.

	call	_rs232_1handler	| process a serial line 1 interrupt
	in	INT_CTLMASK
	andb	al,#notop(RS232_MASK)
	out	INT_CTLMASK
	ret


|*===========================================================================*
|*				secondary_int				     *
|*===========================================================================*
_secondary_int:			| Interrupt routine for rs232 port 2
#ifdef i80286
_psecondary_int:
#endif
	call	save
	in	INT_CTLMASK
	orb	al,#SECONDARY_MASK
	out	INT_CTLMASK
	call	_rs232_2handler	| process a serial line 2 interrupt
	in	INT_CTLMASK
	andb	al,#notop(SECONDARY_MASK)
	out	INT_CTLMASK
	ret
#endif /* C_RS232_INT_HANDLERS */


|*===========================================================================*
|*				lpr_int					     *
|*===========================================================================*
_lpr_int:			| Interrupt routine for printer output.
	call	save
	in	INT_CTLMASK
	orb	al,#PRINTER_MASK
	out	INT_CTLMASK
	sti
	call	_pr_char	| process a line printer interrupt
	cli
	in	INT_CTLMASK
#ifdef ASLD
	andb	al,#notop(PRINTER_MASK)
#else
	andb	al,#notop(PRINTER_MASK) & 0xFF
#endif
	out	INT_CTLMASK
	ret


|*===========================================================================*
|*				disk_int				     *
|*===========================================================================*
_disk_int:			| Interrupt routine for the floppy disk.
	call	save
	cmp	_ps,#0		| check for 2nd int controller on ps (?)
	je	over_ps_disk
	movb	al,#ENABLE
	out	0x3C
over_ps_disk:

| The last doesn't make sense as an 8259 command, since the floppy vector
| seems to be the same on PS's so the usual 8259 must be controlling it.
| This used to be done at the start of the interrupt handler, in proc.c.
| Find out where it really goes, and if there are any mask bits in port
| 0x3D which have to be fiddled (here and in fdc_results()).

	in	INT_CTLMASK
	orb	al,#FLOPPY_MASK
	out	INT_CTLMASK
	sti
	mov	ax,#FLOPPY
	push	ax
	call	_interrupt	| interrupt(FLOPPY)
	add	sp,#2
	cli
	ret


|*===========================================================================*
|*				wini_int				     *
|*===========================================================================*
_wini_int:			| Interrupt routine for the winchester disk.
	call	save
	cmp	_pc_at,#0	| check for 2nd int controller on AT
	jnz	at_wini_int

xt_wini_int:
	in	INT_CTLMASK
	orb	al,#XT_WINI_MASK
	out	INT_CTLMASK
	sti
	mov	ax,#WINCHESTER
	push	ax
	call	_interrupt	| interrupt(WINCHESTER)
	add	sp,#2
	cli
	ret

at_wini_int:
	in	INT2_MASK
	orb	al,#AT_WINI_MASK
	out	INT2_MASK
	sti
	movb	al,#ENABLE	| reenable slave 8259
	out	INT2_CTL	| the master one was done in save() as usual
	mov	ax,#WINCHESTER
	push	ax
	call	_interrupt	| interrupt(WINCHESTER)
	add	sp,#2
	cli
	ret


|*===========================================================================*
|*				clock_int				     *
|*===========================================================================*
_clock_int:			| Interrupt routine for the clock.
	call	save
	in	INT_CTLMASK
	orb	al,#CLOCK_MASK
	out	INT_CTLMASK
	sti
	call	_clock_handler	| process a clock interrupt
				| This calls interrupt() only if necessary.
				| Very rarely.
	cli
	in	INT_CTLMASK
	andb	al,#notop(CLOCK_MASK)
	out	INT_CTLMASK
	ret


|*===========================================================================*
|*				eth_int					     *
|*===========================================================================*
_eth_int:			| Interrupt routine for ethernet input
	call	save
	in	INT_CTLMASK
	orb	al,#ETHER_MASK
	out	INT_CTLMASK
	sti			| may not be able to handle this
				| but ethernet doesn't work in protected mode
				| yet, and tacitly assumes CLICK_SIZE == 16
	call	_dp8390_int	| call the handler
	cli
	in	INT_CTLMASK
	andb	al,#notop(ETHER_MASK)
	out	INT_CTLMASK
	ret


|*===========================================================================*
|*				int00-16				     *
|*===========================================================================*
_int00:				| interrupt through vector 0
	push	ax
	movb	al,#0
	j	exception

_int01:				| interrupt through vector 1, etc
	push	ax
	movb	al,#1
	j	exception

_int02:
	push	ax
	movb	al,#2
	j	exception

_int03:
	push	ax
	movb	al,#3
	j	exception

_int04:
	push	ax
	movb	al,#4
	j	exception

_int05:
	push	ax
	movb	al,#5
	j	exception

_int06:
	push	ax
	movb	al,#6
	j	exception

_int07:
	push	ax
	movb	al,#7
	j	exception

_int08:
	push	ax
	movb	al,#8
	j	exception

_int09:
	push	ax
	movb	al,#9
	j	exception

_int10:
	push	ax
	movb	al,#10
	j	exception

_int11:
	push	ax
	movb	al,#11
	j	exception

_int12:
	push	ax
	movb	al,#12
	j	exception

_int13:
	push	ax
	movb	al,#13
	j	exception

_int14:
	push	ax
	movb	al,#14
	j	exception

_int15:
	push	ax
	movb	al,#15
	j	exception

_int16:
	push	ax
	movb	al,#16
	j	exception

_trp:
	push	ax
	movb	al,#17		| any vector above 17 becomes 17

exception:
	seg	cs
	movb	ex_number,al	| it's cumbersome to get this into dseg
	pop	ax
	call	save
	seg	cs
	push	ex_number	| high byte is constant 0
	call	_exception	| do whatever's necessary (sti only if safe)
	add	sp,#2
	cli
	ret


|*===========================================================================*
|*				save					     *
|*===========================================================================*
save:				| save the machine state in the proc table.
	cld			| set direction flag to a known value
	push	ds
	push	si
	seg	cs
	mov	ds,kernel_ds
	incb	_k_reenter	| from -1 if not reentering
	jnz	push_current_stack	| stack is already kernel's

	mov	si,_proc_ptr
	mov	AXREG(si),ax
	mov	BXREG(si),bx
	mov	CXREG(si),cx
	mov	DXREG(si),dx
	pop	SIREG(si)
	mov	DIREG(si),di
	mov	BPREG(si),bp
	mov	ESREG(si),es
	pop	DSREG(si)
	pop	bx		| return adr
	pop	PCREG(si)
	pop	CSREG(si)
	pop	PSWREG(si)
	mov	SPREG(si),sp
	mov	SSREG(si),ss

	mov	dx,ds
	mov	ss,dx
	mov	sp,#k_stktop
#ifdef SPLIMITS
	mov	splimit,#k_stack+8
#endif
	mov	ax,#_restart	| build return address for interrupt handler
	push	ax

stack_switched:
	mov	es,dx
	movb	al,#ENABLE	| reenable int controller for everyone (early)
	out	INT_CTL
	seg	cs
	jmpreg	(bx)

push_current_stack:
	push	es
	push	bp
	push	di
	push	dx
	push	cx
	push	bx
	push	ax
	mov	bp,sp
	mov	bx,18(bp)	| get the return adr; it becomes junk on stack
	mov	ax,#restart1
	push	ax
	mov	dx,ss
	mov	ds,dx
	j	stack_switched


|*===========================================================================*
|*				s_call					     *
|*===========================================================================*
_s_call:			| System calls are vectored here.
				| Do save routine inline for speed,
				| but don't save ax, bx, cx, dx,
				| since C doesn't require preservation,
				| and ax returns the result code anyway.
				| Regs bp, si, di get saved by sys_call as
				| well, but it is impractical not to preserve
				| them here, in case context gets switched.
				| Some special-case code in pick_proc()
				| could avoid this.
	cld			| set direction flag to a known value
	push	ds
	push	si
	seg	cs
	mov	ds,kernel_ds
	incb	_k_reenter
	mov	si,_proc_ptr
	pop	SIREG(si)
	mov	DIREG(si),di
	mov	BPREG(si),bp
	mov	ESREG(si),es
	pop	DSREG(si)
	pop	PCREG(si)
	pop	CSREG(si)
	pop	PSWREG(si)
	mov	SPREG(si),sp
	mov	SSREG(si),ss
	mov	dx,ds
	mov	es,dx
	mov	ss,dx		| interrupt handlers may not make system calls
	mov	sp,#k_stktop	| so stack is not already switched
#ifdef SPLIMITS
	mov	splimit,#k_stack+8
#endif
				| end of inline save
				| now set up parameters for C routine sys_call
	push	bx		| pointer to user message
	push	ax		| src/dest
	push	cx		| SEND/RECEIVE/BOTH
	sti			| allow SWITCHER to be interrupted
	call	_sys_call	| sys_call(function, src_dest, m_ptr)
				| caller is now explicitly in proc_ptr
	mov	AXREG(si),ax	| sys_call MUST PRESERVE si
	cli

| Fall into code to restart proc/task running.


|*===========================================================================*
|*				restart					     *
|*===========================================================================*
_restart:

| Flush any held-up interrupts.
| This reenables interrupts, so the current interrupt handler may reenter.
| This doesn't matter, because the current handler is about to exit and no
| other handlers can reenter since flushing is only done when k_reenter == 0.

	cmp	_held_head,#0	| do fast test to usually avoid function call
	jz	over_call_unhold
	call	_unhold		| this is rare so overhead is acceptable
over_call_unhold:

	mov	si,_proc_ptr
#ifdef SPLIMITS
	mov	ax,P_SPLIMIT(si)	| splimit = p_splimit
	mov	splimit,ax
#endif
	decb	_k_reenter
	mov	ax,AXREG(si)	| start restoring registers from proc table
				| could make AXREG == 0 to use lodw here
	mov	bx,BXREG(si)
	mov	cx,CXREG(si)
	mov	dx,DXREG(si)
	mov	di,DIREG(si)
	mov	bp,BPREG(si)
	mov	es,ESREG(si)
	mov	ss,SSREG(si)
	mov	sp,SPREG(si)
	push	PSWREG(si)	| fake interrupt stack frame
	push	CSREG(si)
	push	PCREG(si)
				| could put si:ds together to use
				| lds si,SIREG(si)
	push	DSREG(si)
	mov	si,SIREG(si)
	pop	ds
#ifdef DEBUGGER
	nop			| for db emul. of iret - last pop will skip
#endif
	iret			| return to user or task

restart1:
	decb	_k_reenter
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	pop	di
	pop	bp
	pop	es
	pop	si
	pop	ds
	add	sp,#2		| skip return adr
	iret


|*===========================================================================*
|*				idle					     *
|*===========================================================================*
_idle_task:			| executed when there is no work 
	j	_idle_task	| a "hlt" before this fails in protected mode


|*===========================================================================*
|*				data					     *
|*===========================================================================*
ex_number:			| exception number (stored in code segment)
	.space	2

	.data
begdata:
_sizes:				| sizes of kernel, mm, fs filled in by build
	.word	0x526F		| this must be the first data entry (magic #)
	.zerow	7		| build table uses prev word and this space
k_stack:			| kernel stack
	.space	K_STACK_BYTES
k_stktop:			| top of kernel stack

	.bss
begbss:
