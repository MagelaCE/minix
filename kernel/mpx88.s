| This file is part of the lowest layer of the MINIX kernel.  All processing
| switching and message handling is done here and in file "proc.c".  This file
| is entered on every transition to the kernel, both for sending/receiving
| messages and for all interrupts.  In all cases, the trap or interrupt
| routine first calls save() to store the machine state in the proc table.
| Then the stack is switched to k_stack.  Finally, the real trap or interrupt
| handler (in C) is called.  When it returns, the interrupt routine jumps to
| restart, to run the process or task whose number is in 'cur_proc'.
|
| The external entry points into this file are:
|   s_call:	process or task wants to send or receive a message
|   tty_int:	interrupt routine for each key depression and release
|   rs232_int:	interrupt routine for each rs232 interrupt on port 1
|   secondary:	interrupt routine for each rs232 interrupt on port 2
|   lpr_int:	interrupt routine for each line printer interrupt
|   disk_int:	disk interrupt routine
|   wini_int:	winchester interrupt routine
|   clock_int:	clock interrupt routine (HZ times per second)
|   eth_int:	ethernet interrupt routine
|   int00-int15:handlers for unused interrupt vectors < 16
|   trp:	all traps with vector >= 16 are vectored here
|   restart:	start running a task or process

| These symbols MUST agree with the values in ../h/com.h to avoid disaster.
K_STACK_BYTES	=  256
WINI		=   -6
FLOPPY		=   -5
CLOCK		=   -3
IDLE		= -999
DISKINT		=    1
CLOCK_TICK	=    2

| The following procedures are defined in this file and called from outside it.
.globl _tty_int, _rs232_int, _lpr_int, _clock_int, _disk_int, _wini_int
.globl _eth_int, _s_call, _trp, _restart, _secondary_int
.globl _int00, _int01, _int02, _int03, _int04, _int05, _int06, _int07
.globl _int08, _int09, _int10, _int11, _int12, _int13, _int14, _int15

| The following external procedures are called in this file.
.globl _main, _sys_call, _interrupt, _keyboard, _panic, _unexpected_int, _trap
.globl _pr_char, _rs232

| Variables, data structures and miscellaneous.
.globl _cur_proc, _proc_ptr, _scan_code, _int_mess, _k_stack, splimit
.globl _sizes, begtext, begdata, begbss

| The following constants are offsets into the proc table.
esreg = 14
dsreg = 16
csreg = 18
ssreg = 20
SP    = 22
PC    = 24
PSW   = 28
SPLIM = 50
OFF   = 18
ROFF  = 12

.text
begtext:
|*===========================================================================*
|*				MINIX					     *
|*===========================================================================*
MINIX:				| this is the entry point for the MINIX kernel.
	jmp M.0			| skip over the next few bytes
	.word 0,0		| build puts DS at kernel text address 4
M.0:	cli                     | disable interrupts
	mov ax,cs		| set up segment registers
	mov ds,ax		| set up ds
	mov ax,4		| build has loaded this word with ds value
	mov ds,ax		| ds now contains proper value
	mov ss,ax		| ss now contains proper value
	mov _scan_code,bx	| save scan code from bootstrap
  	mov sp,#_k_stack	| set sp to point to the top of the
	add sp,#K_STACK_BYTES	| 	kernel stack

	call _main		| start the main program of MINIX
M.1:	jmp M.1			| this should never be executed


|*===========================================================================*
|*				s_call					     *
|*===========================================================================*
_s_call:			| System calls are vectored here.
	call save		| save the machine state
	mov bp,_proc_ptr	| use bp to access sys call parameters
	push 2(bp)		| push(pointer to user message) (was bx)
	push (bp)		| push(src/dest) (was ax)
	push _cur_proc		| push caller
	push 4(bp)		| push(SEND/RECEIVE/BOTH) (was cx)
	call _sys_call		| sys_call(function, caller, src_dest, m_ptr)
	jmp _restart		| jump to code to restart proc/task running


|*===========================================================================*
|*				tty_int					     *
|*===========================================================================*
_tty_int:			| Interrupt routine for terminal input.
	call save		| save the machine state
	call _keyboard		| process a keyboard interrupt
	jmp _restart		| continue execution


|*============================================================================
|*				rs232_int				     *
|*============================================================================
_rs232_int:			| Interrupt routine for rs232 I/O.
	call save		| save the machine state
	mov ax,#1		| which unit caused the interrupt
	push ax			| pass it as a parameter
	call _rs232		| process a rs232 interrupt
	jmp _restart		| continue execution


|*============================================================================
|*				secondary_int				     *
|*============================================================================
_secondary_int:			| Interrupt routine for rs232 port 2
	call save		| save the machine state
	mov ax,#2		| which unit caused the interrupt
	push ax			| pass it as a parameter
	call _rs232		| process a rs232 interrupt
	jmp _restart		| continue execution


|*===========================================================================*
|*				lpr_int					     *
|*===========================================================================*
_lpr_int:			| Interrupt routine for terminal input.
	call save		| save the machine state
	call _pr_char		| process a line printer interrupt
	jmp _restart		| continue execution


|*===========================================================================*
|*				disk_int				     *
|*===========================================================================*
_disk_int:			| Interrupt routine for the floppy disk.
	call save		| save the machine state
	mov _int_mess+2,*DISKINT| build message for disk task
	mov ax,#_int_mess	| prepare to call interrupt(FLOPPY, &intmess)
	push ax			| push second parameter
	mov ax,*FLOPPY		| prepare to push first parameter
	push ax			| push first parameter
	call _interrupt		| this is the call
	jmp _restart		| continue execution


|*===========================================================================*
|*				wini_int				     *
|*===========================================================================*
_wini_int:			| Interrupt routine for the winchester disk.
	call save		| save the machine state
	mov _int_mess+2,*DISKINT| build message for winchester task
	mov ax,#_int_mess	| prepare to call interrupt(WINI, &intmess)
	push ax			| push second parameter
	mov ax,*WINI		| prepare to push first parameter
	push ax			| push first parameter
	call _interrupt		| this is the call
	jmp _restart		| continue execution


|*===========================================================================*
|*				clock_int				     *
|*===========================================================================*
_clock_int:			| Interrupt routine for the clock.
	call save		| save the machine state
	mov _int_mess+2,*CLOCK_TICK	| build message for clock task
	mov ax,#_int_mess	| prepare to call interrupt(CLOCK, &intmess)
	push ax			| push second parameter
	mov ax,*CLOCK		| prepare to push first parameter
	push ax			| push first parameter
	call _interrupt		| this is the call
	jmp _restart		| continue execution


|*===========================================================================*
|*				eth_int					     *
|*===========================================================================*
_eth_int:			| Interrupt routine for ethernet input
	call save		| save the machine state
	call _dp8390_int	| call the handler
	jmp _restart		| continue execution


|*===========================================================================*
|*				int00-15				     *
|*===========================================================================*
_int00:	call save		| interrupt through vector 0
	mov ax,#0		| save vector number in ax
	jmp vec_mess		| print message
	
_int01:	call save		| interrupt through vector 1
	mov ax,#1		| save vector number in ax
	jmp vec_mess		| print message
	
_int02:	call save		| interrupt through vector 1
	mov ax,#2		| save vector number in ax
	jmp vec_mess		| print message
	
_int03:	call save		| interrupt through vector 3
	mov ax,#3		| save vector number in ax
	jmp vec_mess		| print message
	
_int04:	call save		| interrupt through vector 4
	mov ax,#4		| save vector number in ax
	jmp vec_mess		| print message
	
_int05:	call save		| interrupt through vector 5
	mov ax,#5		| save vector number in ax
	jmp vec_mess		| print message
	
_int06:	call save		| interrupt through vector 6
	mov ax,#6		| save vector number in ax
	jmp vec_mess		| print message
	
_int07:	call save		| interrupt through vector 7
	mov ax,#7		| save vector number in ax
	jmp vec_mess		| print message
	
_int08:	call save		| interrupt through vector 8
	mov ax,#8		| save vector number in ax
	jmp vec_mess		| print message
	
_int09:	call save		| interrupt through vector 9
	mov ax,#9		| save vector number in ax
	jmp vec_mess		| print message
	
_int10:	call save		| interrupt through vector 10
	mov ax,#10		| save vector number in ax
	jmp vec_mess		| print message
	
_int11:	call save		| interrupt through vector 11
	mov ax,#11		| save vector number in ax
	jmp vec_mess		| print message
	
_int12:	call save		| interrupt through vector 12
	mov ax,#12		| save vector number in ax
	jmp vec_mess		| print message
	
_int13:	call save		| interrupt through vector 13
	mov ax,#13		| save vector number in ax
	jmp vec_mess		| print message
	
_int14:	call save		| interrupt through vector 14
	mov ax,#14		| save vector number in ax
	jmp vec_mess		| print message
	
_int15:	call save		| interrupt through vector 15
	mov ax,#15		| save vector number in ax
	jmp vec_mess		| print message
	
vec_mess:			| this is where unexpected interrupts come.
	push ax			| push the vector number
	call _unexpected_int	| go panic
	jmp _restart		| never executed


|*===========================================================================*
|*				trp					     *
|*===========================================================================*
_trp:				| this is where unexpected traps come.
	call save		| save the machine state
	call _trap		| print a message
	jmp _restart		| this error is not fatal


|*===========================================================================*
|*				save					     *
|*===========================================================================*
save:				| save the machine state in the proc table.  
	cld			| set direction flag to a known value
	push ds			| stack: psw/cs/pc/ret addr/ds
	push cs			| prepare to restore ds
	pop ds			| ds has now been set to cs
	mov ds,4		| word 4 in kernel text space contains ds value
	pop ds_save		| stack: psw/cs/pc/ret addr
	pop ret_save		| stack: psw/cs/pc
	mov bx_save,bx		| save bx for later ; we need a free register
	mov bx,_proc_ptr	| start save set up; make bx point to save area
	add bx,*OFF		| bx points to place to store cs
	pop PC-OFF(bx)		| store pc in proc table 
	pop csreg-OFF(bx)	| store cs in proc table
	pop PSW-OFF(bx)		| store psw 
	mov ssreg-OFF(bx),ss	| store ss
	mov SP-OFF(bx),sp	| sp as it was prior to interrupt
	mov sp,bx		| now use sp to point into proc table/task save
	mov bx,ds		| about to set ss
	mov ss,bx		| set ss
	push ds_save		| start saving all the registers, sp first
	push es			| save es between sp and bp
	mov es,bx		| es now references kernel memory too
	push bp			| save bp
	push di			| save di
	push si			| save si
	push dx			| save dx
	push cx			| save cx
	push bx_save		| save original bx
	push ax			| all registers now saved
	mov sp,#_k_stack	| temporary stack for interrupts
	add sp,#K_STACK_BYTES	| set sp to top of temporary stack
	mov splimit,#_k_stack	| limit for temporary stack
	add splimit,#8		| splimit checks for stack overflow
	cld
	mov ax,ret_save		| ax = address to return to
	jmp (ax)		| return to caller; Note: sp points to saved ax


|*===========================================================================*
|*				restart					     *
|*===========================================================================*
_restart:			| This routine sets up and runs a proc or task.
	cmp _cur_proc,#IDLE	| restart user; if cur_proc = IDLE, go idle
	je idle			| no user is runnable, jump to idle routine
	cli			| disable interrupts
	mov sp,_proc_ptr	| return to user, fetch regs from proc table
	pop ax			| start restoring registers
	pop bx			| restore bx
	pop cx			| restore cx
	pop dx			| restore dx
	pop si			| restore si
	pop di			| restore di
	mov lds_low,bx		| lds_low contains bx
	mov bx,sp		| bx points to saved bp register
	mov bp,SPLIM-ROFF(bx)	| splimit = p_splimit
	mov splimit,bp		| ditto
	mov bp,dsreg-ROFF(bx)	| bp = ds
	mov lds_low+2,bp	| lds_low+2 contains ds
	pop bp			| restore bp
	pop es			| restore es
	mov sp,SP-ROFF(bx)	| restore sp
	mov ss,ssreg-ROFF(bx)	| restore ss using the value of ds
	push PSW-ROFF(bx)	| push psw
	push csreg-ROFF(bx)	| push cs
	push PC-ROFF(bx)	| push pc
	lds bx,lds_low		| restore ds and bx in one fell swoop
	iret			| return to user or task


|*===========================================================================*
|*				idle					     *
|*===========================================================================*
idle:				| executed when there is no work 
	sti			| enable interrupts
L3:  	wait			| just idle while waiting for interrupt
	jmp L3			| loop until interrupt



|*===========================================================================*
|*				data					     *
|*===========================================================================*
.data
begdata:
_sizes:  .word 0x526F		| this must be the first data entry (magic #)
	 .zerow 7		| build table uses prev word and this space
bx_save: .word 0		| storage for bx
ds_save: .word 0		| storage for ds
ret_save:.word 0		| storage for return address
lds_low: .word 0,0		| storage used for restoring bx
ttyomess: .asciz "RS232 interrupt"
.bss
begbss:
