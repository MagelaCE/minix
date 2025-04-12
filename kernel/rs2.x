|*===========================================================================*
|*		rs232 interrupt handlers for real and protected modes	     *
|*===========================================================================*

#include "../h/const.h"
#include "const.h"
#include "sconst.h"
#define RS2 .define
#include "sglo.h"

#undef MINOR

#ifdef ASLD
#define add1_and_align(n)	[[[n]+1+1] / 2 * 2]	/* word alignment */
#else
#define add1_and_align(n)	(((n)+1+1) & !1)
#endif

| These constants are defined in tty.h. That has C stuff so can't be included.
EVENT_THRESHOLD		=	64
RS_IBUFSIZE		=	128

| These constants are defined in rs232.c.
IS_LINE_STATUS_CHANGE	=	6
IS_MODEM_STATUS_CHANGE	=	0
IS_NO_INTERRUPT		=	1
IS_RECEIVER_READY	=	4
IS_TRANSMITTER_READY	=	2
LS_OVERRUN_ERR		=	2
LS_PARITY_ERR		=	4
LS_FRAMING_ERR		=	8
LS_BREAK_INTERRUPT	=	0x10
LS_TRANSMITTER_READY	=	0x20
MC_DTR			=	1
MC_OUT2			=	8
MS_CTS			=	0x10
ODEVREADY		=	MS_CTS
ODONE			=	1
OQUEUED			=	0x20
ORAW			=	2
OSWREADY		=	0x40
OWAKEUP			=	4
RS_IHIGHWATER		=	3*RS_IBUFSIZE/4

| These port offsets are hard-coded in rs232.c.
XMIT_OFFSET		=	0
RECV_OFFSET		=	0
INT_ID_OFFSET		=	2
MODEM_CTL_OFFSET	=	4
LINE_STATUS_OFFSET	=	5
MODEM_STATUS_OFFSET	=	6

| Offsets in struct rs232_s. They must match rs232.c
MINOR			=	0
IDEVREADY		=	MINOR+2
ITTYREADY		=	IDEVREADY+1
IBUF			=	add1_and_align(ITTYREADY)
IBUFEND			=	IBUF+2
IHIGHWATER		=	IBUFEND+2
IPTR			=	IHIGHWATER+2
OSTATE			=	IPTR+2
OXOFF			=	OSTATE+1
OBUFEND			=	add1_and_align(OXOFF)
OPTR			=	OBUFEND+2
XMIT_PORT		=	OPTR+2
RECV_PORT		=	XMIT_PORT+2
DIV_LOW_PORT		=	RECV_PORT+2
DIV_HI_PORT		=	DIV_LOW_PORT+2
INT_ENAB_PORT		=	DIV_HI_PORT+2
INT_ID_PORT		=	INT_ENAB_PORT+2
LINE_CTL_PORT		=	INT_ID_PORT+2
MODEMCTL_PORT		=	LINE_CTL_PORT+2
LINESTATUS_PORT		=	MODEMCTL_PORT+2
MODEMSTATUS_PORT	=	LINESTATUS_PORT+2
LSTATUS			=	MODEMSTATUS_PORT+2
FRAMING_ERRORS		=	add1_and_align(LSTATUS)
OVERRUN_ERRORS		=	FRAMING_ERRORS+2
PARITY_ERRORS		=	OVERRUN_ERRORS+2
BREAK_INTERRUPTS	=	PARITY_ERRORS+2
IBUF1			=	BREAK_INTERRUPTS+2
IBUF2			=	IBUF1+RS_IBUFSIZE+1
SIZEOF_STRUCT_RS232_S	=	IBUF2+RS_IBUFSIZE+1

	.text

#ifdef i80286
| PUBLIC void interrupt _psecondary_int();

_psecondary_int:
	push	ds
	push	si
	mov	si,ss
	mov	ds,si
	mov	si,#_rs_lines+SIZEOF_STRUCT_RS232_S
	j	commonp
#endif

| PUBLIC void interrupt _secondary_int();

_secondary_int:
	push	ds
	push	si
	mov	si,#_rs_lines+SIZEOF_STRUCT_RS232_S
	j	common

#ifdef i80286
| PUBLIC void interrupt _prs232_int();

_prs232_int:
	push	ds
	push	si
	mov	si,ss
	mov	ds,si
	mov	si,#_rs_lines
	j	commonp
#endif

| input interrupt

inint:
	addb	dl,#RECV_OFFSET-INT_ID_OFFSET
	in
	mov	bx,IPTR(si)
	movb	(bx),al
	cmp	bx,IBUFEND(si)
	jge	checkxoff
	inc	_tty_events
	inc	bx
	mov	IPTR(si),bx
	cmp	bx,IHIGHWATER(si)
	jne	checkxoff
	addb	dl,#MODEM_CTL_OFFSET-RECV_OFFSET
	movb	al,#MC_OUT2+MC_DTR
	out
	movb	IDEVREADY(si),#FALSE
checkxoff:
	testb	ah,#ORAW
	jne	rsnext
	cmpb	al,OXOFF(si)
	je	gotxoff
	testb	ah,#OSWREADY
	jne	rsnext
	orb	ah,#OSWREADY
	mov	dx,LINESTATUS_PORT(si)
	in
	testb	al,#LS_TRANSMITTER_READY
	je	rsnext
	addb	dl,#XMIT_OFFSET-LINE_STATUS_OFFSET
	j	outint1

gotxoff:
	andb	ah,#notop(OSWREADY)
	j	rsnext

| PUBLIC void interrupt rs232_int();

_rs232_int:
	push	ds
	push	si
	mov	si,#_rs_lines
common:
	seg	cs
	mov	ds,kernel_ds
	seg	cs
commonp:
	push	ax
	push	bx
	push	dx
	movb	ah,OSTATE(si)
	mov	dx,INT_ID_PORT(si)
	in
rsmore:
	cmpb	al,#IS_RECEIVER_READY
	je	inint
	cmpb	al,#IS_TRANSMITTER_READY
	je	outint
	cmpb	al,#IS_MODEM_STATUS_CHANGE
	je	modemint
	cmpb	al,#IS_LINE_STATUS_CHANGE
	jne	rsdone		| fishy

| line status change interrupt

	addb	dl,#LINE_STATUS_OFFSET-INT_ID_OFFSET
	in
	testb	al,#LS_FRAMING_ERR
	je	over_framing_error
	inc	FRAMING_ERRORS(si)	
over_framing_error:
	testb	al,#LS_OVERRUN_ERR
	je	over_overrun_error
	inc	OVERRUN_ERRORS(si)	
over_overrun_error:
	testb	al,#LS_PARITY_ERR
	je	over_parity_error
	inc	PARITY_ERRORS(si)
over_parity_error:
	testb	al,#LS_BREAK_INTERRUPT
	je	over_break_interrupt
	inc	BREAK_INTERRUPTS(si)
over_break_interrupt:

rsnext:
	mov	dx,INT_ID_PORT(si)
	in
	cmpb	al,#IS_NO_INTERRUPT
	jne	rsmore
rsdone:
	movb	al,#ENABLE
	out	INT_CTL
	testb	ah,#OWAKEUP
	jne	owakeup
	movb	OSTATE(si),ah
	pop	dx
	pop	bx
	pop	ax
	pop	si
	pop	ds
rs2_iret:
	iret			| changed to iretd for 386 protected mode
	nop			| space for longer opcode
	nop			| padding since convenient to change 3 bytes

| output interrupt

outint:
	addb	dl,#XMIT_OFFSET-INT_ID_OFFSET
outint1:
	cmpb	ah,#ODEVREADY+OQUEUED+OSWREADY
	jb	rsnext		| not all are set
	mov	bx,OPTR(si)
	movb	al,(bx)
	out
	inc	bx
	mov	OPTR(si),bx
	cmp	bx,OBUFEND(si)
	jb	rsnext
	add	_tty_events,#EVENT_THRESHOLD
	xorb	ah,#ODONE+OQUEUED+OWAKEUP	| OQUEUED off, others on
	j	rsnext		| direct exit might lose interrupt

| modem status change interrupt

modemint:
	addb	dl,#MODEM_STATUS_OFFSET-INT_ID_OFFSET
	in
	testb	al,#MS_CTS
	jne	m_devready
	andb	ah,#notop(ODEVREADY)
	j	rsnext

m_devready:
	testb	ah,#ODEVREADY
	jne	rsnext
	orb	ah,#ODEVREADY
	addb	dl,#LINE_STATUS_OFFSET-MODEM_STATUS_OFFSET
	in
	testb	al,#LS_TRANSMITTER_READY
	je	rsnext
	addb	dl,#XMIT_OFFSET-LINE_STATUS_OFFSET
	j	outint1

| special exit for output just completed

owakeup:
	andb	ah,#notop(OWAKEUP)
	movb	OSTATE(si),ah

| determine mask bit (it would be better to precalculate it in the struct)

	movb	ah,#SECONDARY_MASK
	cmp	si,#_rs_lines
	jne	got_rs_mask
	movb	ah,#RS232_MASK
got_rs_mask:
	mov	rs_mask,ax	| save ax to clear later
	in	INT_CTLMASK
	orb	al,ah
	out	INT_CTLMASK

| rearrange context to call tty_wakeup()

	pop	dx
	pop	bx
	pop	ax
	pop	si
	pop	ds
	call	save
	push	rs_mask		| save the mask again, reentrantly
	sti
	call	_tty_wakeup
	cli
	pop	ax
	notb	ah		| return this
	in	INT_CTLMASK
	andb	al,ah
	out	INT_CTLMASK
	ret


	.data
rs_mask:
	.space	2
	.space	2		| align
