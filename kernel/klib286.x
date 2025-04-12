|*===========================================================================*
|*			klib support for 286 protected mode		     *
|*===========================================================================*

#include "../h/const.h"
#include "const.h"
#include "protect.h"
#include "sconst.h"
#define KLIB286 .define
#include "sglo.h"

	.text
|*===========================================================================*
|*				klib286_init				     *
|*===========================================================================*

| Patch the code segment for returning to real mode.
| Patch various functions in klib88 to jump to corresponding functions here.

_klib286_init:
	mov	si,#patch_table
	lodw			| original function
patch1:
	mov	bx,ax
	lodw			| new function - target of call or jump
	sub	ax,bx		| relative jump
	sub	ax,#3		| adjust by length of jump instruction
	seg	cs
	mov	1(bx),ax
	lodb			| opcode
	seg	cs
	movb	(bx),al
	lodw			| next original function
	test	ax,ax
	jnz	patch1
	ret


|*===========================================================================*
|*				p_check_mem				     *
|*===========================================================================*

PCM_DENSITY	=	256	| others are not supported

p_check_mem:
	pop	bx
	pop	_gdt+DS_286_OFFSET+DESC_BASE
	pop	ax		| pop base into base of source descriptor
	movb	_gdt+DS_286_OFFSET+DESC_BASE_MIDDLE,al
	pop	cx		| byte count in dx:cx
	pop	dx
	sub	sp,#4+4
	push	bx
	push	ds

	sub	ax,ax		| prepare for early exit
	test	dx,#0xFF00
	jnz	pcm_1exit	| can't handle bases above 16M
	movb	cl,ch		| divide size by 256 and discard high byte
	movb	ch,dl
	push	cx		| save divided size
	sub	bx,bx		| test bytes at bases of segments
pcm_loop:
	mov	ax,#DS_286_SELECTOR
	mov	ds,ax
	movb	dl,#TEST1PATTERN
	xchgb	dl,(bx)		| write test pattern, remember original value
	xchgb	dl,(bx)		| restore original value, read test pattern
	cmpb	dl,#TEST1PATTERN	| must agree if good real memory
	jnz	pcm_exit	| if different, memory is unusable
	movb	dl,#TEST2PATTERN
	xchgb	dl,(bx)
	xchgb	dl,(bx)
	cmpb	dl,#TEST2PATTERN
	jnz	pcm_exit
	seg	es		| next segement, test for wraparound at 16M
	add	_gdt+DS_286_OFFSET+DESC_BASE,#PCM_DENSITY
	seg	es		| assuming es == old ds
	adcb	_gdt+DS_286_OFFSET+DESC_BASE_MIDDLE,#0
	loopnz	pcm_loop

pcm_exit:
	pop	ax
	sub	ax,cx		| verified size in multiples of PCM_DENSITY
pcm_1exit:
	movb	dl,ah		| convert to phys_bytes in dx:ax
	subb	dh,dh
	movb	ah,al
	movb	al,dh
	pop	ds
	ret


|*===========================================================================*
|*				p_cp_mess				     *
|*===========================================================================*

| The 16 bit cp_mess() attempts to be efficient by passing raw segments but
| that just gets in the way here.

p_cp_mess:
	pop	dx
	pop	bx		| proc
	pop	cx		| source clicks
	pop	ax		| source offset
#if CLICK_SHIFT != HCLICK_SHIFT + 4
#error /* the only click size supported is 256, to avoid slow shifts here */
#endif
	addb	ah,cl		| calculate source offset
	adcb	ch,#0 		| and put in base of source descriptor
	mov	_gdt+DS_286_OFFSET+DESC_BASE,ax
	movb	_gdt+DS_286_OFFSET+DESC_BASE_MIDDLE,ch
	pop	cx		| destination clicks
	pop	ax		| destination offset
	addb	ah,cl		| calculate destination offset
	adcb	ch,#0 		| and put in base of destination descriptor
	mov	_gdt+ES_286_OFFSET+DESC_BASE,ax
	movb	_gdt+ES_286_OFFSET+DESC_BASE_MIDDLE,ch
	sub	sp,#2+2+2+2+2

	push	ds
	push	es
	mov	ax,#DS_286_SELECTOR
	mov	ds,ax
	mov	ax,#ES_286_SELECTOR
	mov	es,ax

	seg	es
	mov	0,bx		| sender's proc no. from arg, not msg
	mov	ax,si
	mov	bx,di
	mov	si,#2		| src offset is now 2 relative to start of seg
	mov	di,si		| and destination offset
	mov	cx,#Msize-1	| word count
	rep
	movw
	mov	di,bx
	mov	si,ax
	pop	es
	pop	ds
	jmpreg	(dx)


|*===========================================================================*
|*				p_get_phys_byte				     *
|*===========================================================================*

p_get_phys_byte:
	push	bp
	mov	bp,sp

| Save the part of the DS_286 segment descriptor which is going to be
| modified, so badly behaved interrupt handlers (printer and old tty)
| can use this routine.
| This is cheaper than locking all the other descriptor-fiddling routines
| in this file.

	push	_gdt+DS_286_OFFSET+DESC_BASE
	push	_gdt+DS_286_OFFSET+DESC_BASE_MIDDLE

	mov	ax,4(bp)	| move source into base of source descriptor
	mov	_gdt+DS_286_OFFSET+DESC_BASE,ax
	movb	al,4+2(bp)
	movb	_gdt+DS_286_OFFSET+DESC_BASE_MIDDLE,al
	mov	cx,ds
	mov	ax,#DS_286_SELECTOR
	mov	ds,ax
	movb	al,0		| get the byte from the start of the segment
	subb	ah,ah
	mov	ds,cx

	pop	_gdt+DS_286_OFFSET+DESC_BASE_MIDDLE
	pop	_gdt+DS_286_OFFSET+DESC_BASE
	pop	bp
	ret


|*===========================================================================*
|*				p_phys_copy				     *
|*===========================================================================*

p_phys_copy:
	pop	dx
	pop	_gdt+DS_286_OFFSET+DESC_BASE
	pop	ax		| pop source into base of source descriptor
	movb	_gdt+DS_286_OFFSET+DESC_BASE_MIDDLE,al
	pop	_gdt+ES_286_OFFSET+DESC_BASE
	pop	ax		| pop destination into base of dst descriptor
	movb	_gdt+ES_286_OFFSET+DESC_BASE_MIDDLE,al
	pop	cx		| byte count in bx:cx
	pop	bx
	sub	sp,#4+4+4

	push	di
	push	si
	push	es
	push	ds
	sub	si,si		| src offset is now 0 relative to start of seg
	mov	di,si		| and destination offset
	j	ppc_next

| It is too much trouble to align the segment bases so word alignment is hard.
| Avoiding the book-keeping for alignment may be good anyway.

ppc_large:
	push	cx
	mov	cx,#0x8000	| copy a large chunk of this many words
	rep
	movw
	pop	cx
	dec	bx
	pop	ds		| update the descriptors
	incb	_gdt+DS_286_OFFSET+DESC_BASE_MIDDLE
	incb	_gdt+ES_286_OFFSET+DESC_BASE_MIDDLE
	push	ds
ppc_next:
	mov	ax,#DS_286_SELECTOR	| (re)load the selectors
	mov	ds,ax
	mov	ax,#ES_286_SELECTOR
	mov	es,ax
	test	bx,bx
	jnz	ppc_large

	shr	cx,#1		| word count
	rep
	movw			| move any leftover words
	rcl	cx,#1		| restore old bit 0
	rep
	movb			| move any leftover byte
	pop	ds
	pop	es
	pop	si
	pop	di
	jmpreg	(dx)


|*===========================================================================*
|*				p_port_read				     *
|*===========================================================================*

| This is not reentrant, but only the WINCHESTER task is allowed to call it.

p_port_read:
	pop	bx
	pop	dx		| port
	pop	_gdt+ES_286_OFFSET+DESC_BASE
	pop	ax		| pop destination into base of dst descriptor
	movb	_gdt+ES_286_OFFSET+DESC_BASE_MIDDLE,al
	pop	cx		| byte count
	sub	sp,#2+4+2
	push	es
	mov	ax,#ES_286_SELECTOR
	mov	es,ax
	mov	ax,di
	sub	di,di		| dst offset is now 0 relative to start of seg
	shr	cx,#1		| word count
	rep
	insw			| read everything
	mov	di,ax
	pop	es
	jmpreg	(bx)


|*===========================================================================*
|*				p_port_write				     *
|*===========================================================================*

p_port_write:
	pop	bx
	pop	dx		| port
	pop	_gdt+DS_286_OFFSET+DESC_BASE
	pop	ax		| pop offset into base of source descriptor
	movb	_gdt+DS_286_OFFSET+DESC_BASE_MIDDLE,al
	pop	cx		| byte count, discard high word
	sub	sp,#2+4+2
	push	ds
	mov	ax,#DS_286_SELECTOR
	mov	ds,ax
	mov	ax,si
	sub	si,si		| src offset is now 0 relative to start of seg
	shr	cx,#1		| word count
	rep
	outsw			| write everything
	mov	si,ax
	pop	ds
	jmpreg	(bx)


|*===========================================================================*
|*				data					     *
|*===========================================================================*

	.data

patch_table:			| triples (old function, new function, opcode)
	.word	_check_mem, p_check_mem
	.byte	JMP_OPCODE
	.word	_cp_mess, p_cp_mess
	.byte	JMP_OPCODE
	.word	_get_phys_byte, p_get_phys_byte
	.byte	JMP_OPCODE
	.word	_phys_copy, p_phys_copy
	.byte	JMP_OPCODE
	.word	_port_read, p_port_read
	.byte	JMP_OPCODE
	.word	_port_write, p_port_write
	.byte	JMP_OPCODE
	.word	_restart, p_restart
	.byte	JMP_OPCODE
	.word	save, p2_save
	.byte	JMP_OPCODE
	.word	0		| end of table
