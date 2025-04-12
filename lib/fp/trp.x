.define .trp,.trpilin
|.define .trpdivz,.trpcase,.trprang,.trpset,.trpnofp,.trpheap

	.bss
.M:	.zerow	24/2

	.text
	.globl	.trp,.trpilin
|	.globl	.trpdivz,.trpcase,.trprang,.trpset,.trpnofp,.trpheap
|.trpdivz:
|	mov	ax,#6
|	mov	dx,#.Mdivz
|	jmp	.Trp
.trpilin:
	mov	ax,#18
	mov	dx,#.Milin
	jmp	.Trp
|.trpcase:
|	mov	ax,#20
|	mov	dx,#.Mcase
|	jmp	.Trp
|.trprang:
|	mov	ax,#1
|	mov	dx,#.Mrang
|	jmp	.Trp
|.trpset:
|	mov	ax,#2
|	mov	dx,#.Mset
|	jmp	.Trp
|.trpnofp:
|	mov	ax,#18
|	mov	dx,#.Milin
|	jmp	.Trp
|.trpheap:
|	mov	ax,#17
|	mov	dx,#.Mheap
||	jmp	.Trp
.Trp:
	xor	bx,bx
	.globl	.trppc
	xchg	bx,.trppc
	test	bx,bx
	jz	2f
	push	ax
	call	(bx)
	pop	ax
	ret
2:
	mov	bx,#22
	push	bx
	push	dx
	mov	ax,#2
	push	ax
	call	.Write
	call	_exit

.trp:
	mov	dx,ax
	cmp	dx,#21
	jae	1f
	sal	dx,#1
	mov	bx,#.Mtable
	add	bx,dx
	mov	bx,(bx)
	test	bx,bx
	jz	1f
	mov	dx,bx
	jmp	2f
1:
	mov	bx,#.Mtrp+14
	mov	cx,#6
	mov	dx,ax
1:
	and	dx,#7
	add	dx,#'0'
	movb	(bx),dl
	dec	bx
	sar	dx,#1
	sar	dx,#1
	sar	dx,#1
	loop	1b
	mov	dx,#.Mtrp
2:
	jmp	.Trp

.Write:
	push	bp
	mov	bp,sp
	mov	.M+2,#4
	mov	bx,4(bp)
	mov	.M+4,bx
	mov	bx,8(bp)
	mov	.M+6,bx
	mov	bx,6(bp)
	mov	.M+10,bx
	mov	ax,#.M
	push	ax
	mov	ax,#1
	push	ax

	mov	ax,#1
	mov	bx,#.M
	mov	cx,#3
	int	32
	mov	sp,bp
	pop	bp
	ret

	.data
.Mtable:
	.word	0, .Mrang, .Mset, 0, .Mfovf, .Mfunf, .Mdivz, .Mfdvz
	.word	0, 0, .Mconv, 0, 0, 0, 0, 0
	.word	0, .Mheap, .Milin, 0, .Mcase

.Mrang:	.asciz	"Variable out of range\n"	| ERANGE	=	1
.Mset:	.asciz	"Err in EM set instr  \n"	| ESET		=	2
.Mfovf:	.asciz	"Floating pt. overflow\n"	| EFOVFL	=	4
.Mfunf:	.asciz	"Floating pt underflow\n"	| EFUNFL	=	5
.Mdivz:	.asciz	"Integer division by 0\n"	| EIDIVZ	=	6
.Mfdvz:	.asciz	"Division by float 0.0\n"	| EFDIVZ	=	7
|.Mfunf:.asciz	"Bad floating pt value\n"	| EFUND		=	9
.Mconv:	.asciz	"Bad number conversion\n"	| ECONV		=	10
.Mheap:	.asciz	"Heap overflow        \n"	| EHEAP		=	17
.Milin:	.asciz	"Illegal EM instruct'n\n"	| EILLINS	=	18
.Mcase:	.asciz	"Err in EM case instr \n"	| ECASE		=	20

.Mtrp:	.asciz	"EM trap 0000000 octal\n"
