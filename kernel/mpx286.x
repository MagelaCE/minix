|*===========================================================================*
|*			mpx support for 286 protected mode		     *
|*===========================================================================*

#include "const.h"
#include "protect.h"
#include "sconst.h"
#define MPX286 .define
#include "sglo.h"

	.text
|*===========================================================================*
|*				exception handlers			     *
|*===========================================================================*

_divide_error:
	pushi8	(DIVIDE_VECTOR)
	j	pexception

_nmi:
	pushi8	(NMI_VECTOR)
	j	pexception

_overflow:
	pushi8	(OVERFLOW_VECTOR)
	j	pexception

_bounds_check:
	pushi8	(BOUNDS_VECTOR)
	j	pexception

_inval_opcode:
	pushi8	(INVAL_OP_VECTOR)
	j	pexception

_copr_not_available:
	pushi8	(COPROC_NOT_VECTOR)
	j	pexception

_double_fault:
	pushi8	(DOUBLE_FAULT_VECTOR)
	j	perrexception

_copr_seg_overrun:
	pushi8	(COPROC_SEG_VECTOR)
	j	pexception

_inval_tss:
	pushi8	(INVAL_TSS_VECTOR)
	j	perrexception

_segment_not_present:
	pushi8	(SEG_NOT_VECTOR)
	j	perrexception

_stack_exception:
	pushi8	(STACK_FAULT_VECTOR)
	j	perrexception

_general_protection:
	pushi8	(PROTECTION_VECTOR)
	j	perrexception


|*===========================================================================*
|*				pexception				     *
|*===========================================================================*

| This is called for all exceptions which don't push an error code.

pexception:
	seg	ss
	pop	ds_ex_number
	call	p2_save
	j	p1exception


|*===========================================================================*
|*				perrexception				     *
|*===========================================================================*

| This is called for all exceptions which push an error code.

p2_errexception:
perrexception:
	seg	ss
	pop	ds_ex_number
	seg	ss
	pop	trap_errno
	call	p2_save
p1exception:			| Common for all exceptions.
	push	ds_ex_number
	call	_exception
	add	sp,#2
	cli
	ret


|*===========================================================================*
|*				p2_save					     *
|*===========================================================================*

| Save for 286 protected mode.
| This is much simpler than for 8086 mode, because the stack already points
| into process table, or has already been switched to the kernel stack.

p2_save:
	cld			| set direction flag to a known value
	pusha			| save "general" registers
	push	ds		| save ds
	push	es		| save es
	mov	dx,ss		| ss is kernel data segment
	mov	ds,dx		| load rest of kernel segments
	mov	es,dx
	movb	al,#ENABLE	| reenable int controller
	out	INT_CTL
	mov	bp,sp		| prepare to return
	incb	_k_reenter	| from -1 if not reentering
	jnz	set_p1restart	| stack is already kernel's
	mov	sp,#k_stktop
#ifdef SPLIMITS
	mov	splimit,#k_stack+8
#endif
	pushi16	(prestart)	| build return address for interrupt handler
	jmpmem	(RETADR-P_STACKBASE(bp))

set_p1restart:
	pushi16	(p1restart)
	jmpmem	(RETADR-P_STACKBASE(bp))


|*===========================================================================*
|*				p2_s_call				     *
|*===========================================================================*

_p2_s_call:
	cld			| set direction flag to a known value
	sub	sp,#6*2		| skip RETADR, ax, cx, dx, bx, st
	push	bp		| stack already points into process table
	push	si
	push	di
	push	ds
	push	es
	mov	dx,ss
	mov	ds,dx
	mov	es,dx
	incb	_k_reenter
	mov	si,sp		| assumes P_STACKBASE == 0
	mov	sp,#k_stktop
#ifdef SPLIMTS
	mov	splimit,#k_stack+8
#endif
				| end of inline save
	sti			| allow SWITCHER to be interrupted
				| now set up parameters for C routine sys_call
	push	bx		| pointer to user message
	push	ax		| src/dest
	push	cx		| SEND/RECEIVE/BOTH
	call	_sys_call	| sys_call(function, src_dest, m_ptr)
				| caller is now explicitly in proc_ptr
	mov	AXREG(si),ax	| sys_call MUST PRESERVE si
	cli

| Fall into code to restart proc/task running.

prestart:

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
	deflldt	(P_LDT_SEL(si))		| enable task's segment descriptors
	defsldt	(_tss+TSS2_S_LDT)
	lea	ax,P_STACKTOP(si)	| arrange for next interrupt
	mov	_tss+TSS2_S_SP0,ax	| to save state in process table
	mov	sp,si		| assumes P_STACKBASE == 0
p1restart:
	decb	_k_reenter
	pop	es
	pop	ds
	popa
	add	sp,#2		| skip RETADR
	iret			| continue process


|*===========================================================================*
|*				p_restart				     *
|*===========================================================================*

| This now just starts the 1st task.

p_restart:

| Call the BIOS to switch to protected mode.
| This is just to do any cleanup necessary, typically to disable a hardware
| kludge which holds the A20 address line low.

| The call requires the gdt as we set it up:
|	gdt pointer in gdt[1]
|	ldt pointer in gdt[2]
|	new ds in gdt[3]
|	new es in gdt[4]
|	new ss in gdt[5]
|	new cs in gdt[6]
|	nothing in gdt[7] (overwritten with BIOS cs)
|	ICW2 for master 8259 in bh
|	ICW2 for slave 8259 in bl
| Interrupts are enabled at the start but at the finish they are disabled in
| both the processor flags and the interrupt controllers. Most registers are
| destroyed. The 8259's are reinitialised.

	in	INT_CTLMASK	| save interrupt masks
	push	ax
	in	INT2_MASK
	push	ax
	movb	al,#0xFF	| protect against sti in BIOS
	out	INT_CTLMASK
	mov	si,#_gdt
	mov	bx,#IRQ0_VECTOR * 256 orop IRQ8_VECTOR
	movb	ah,#SET_PROTECT_FUNC
	pushf
	callfarptr(_vec_table+4*SET_PROTECT_VEC)
	pushi8	(0)		| set kernel flags to known state, especially
	popf			| clear nested task and interrupt enable
	pop	ax		| restore interrupt masks
	out	INT2_MASK
	pop	ax
	out	INT_CTLMASK

	deflidt	(_gdt+IDT_SELECTOR)	| loaded by BIOS, but in wrong mode!
	mov	ax,#TSS_SELECTOR	| no other TSS is used except by db
	defltrax
	sub	ax,ax		| zero, for no PRESENT bit
	movb	_gdt+GDT_SELECTOR+DESC_ACCESS,al	| zap invalid desc.
	movb	_gdt+IDT_SELECTOR+DESC_ACCESS,al
	movb	_gdt+BIOS_CS_SELECTOR+DESC_ACCESS,al

p2_resdone:
	jmp	prestart


|*===========================================================================*
|*				data					     *
|*===========================================================================*

	.bss

ds_ex_number:
	.space	2
	.space	2		| align
trap_errno:
	.space	4		| large enough for mpx386 too
