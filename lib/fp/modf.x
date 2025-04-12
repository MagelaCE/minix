.define	_modf

BIAS8	=	0x3ff - 1

	.globl	_modf
	.text
_modf:
	push	bp
	mov	bp,sp
	push	si
	mov	si,12(bp)	| si -> ipart

	mov	bx,4+6(bp)	| extract value.exp
	movb	dh,bh		| extract value.sign
	shr	bx,*1
	shr	bx,*1
	shr	bx,*1
	shr	bx,*1
	and	bx,#2047	| kill sign bit

	cmp	bx,#BIAS8
	jge	1f		| fabs(value) >= 1.0

	xor	ax,ax		| store zero as the integer part
	mov	0(si),ax
	mov	2(si),ax
	mov	4(si),ax
	mov	6(si),ax

	lea	bx,4(bp)	| return entire value as fractional part
	mov	cx,#8
	call	.loi
9:	call	.ret8
	jmp	.sret

1:
	cmp	bx,#BIAS8+53	| all integer, with no fractional part?
	jl	2f		| no, mixed

	lea	bx,4(bp)	| store entire value as the integer part
	mov	cx,#8
	call	.loi
	mov	bx,si
	mov	cx,#8
	call	.sti

	call	.zrf8
	jmp	9b		| return zero as fractional part

2:
	mov	ax,4+6(bp)	| remove exponent from value.mantissa
	and	ax,#0x0F
	or	ax,#0x10	| restore implied leading "1"
0:	mov	4+6(bp),ax

	xor	ax,ax		| push a zero fractional part
	push	ax
	push	ax
	push	ax
	push	ax

4:
	shr	4+6(bp),*1	| shift integer part
	rcr	4+4(bp),*1
	rcr	4+2(bp),*1
	rcr	4+0(bp),*1

	rcr	-10+6(bp),*1	| shift high bit into fractional part
	rcr	-10+4(bp),*1
	rcr	-10+2(bp),*1
	rcr	-10+0(bp),*1

	inc	cx		| increment frac part exponent
	inc	bx		| increment ipart exponent
	cmp	bx,#BIAS8+53	| done?
	jl	4b		| keep shifting

	movb	dl,*0		| clear rounding bits
	push	dx		| save sign and rounding bits

	push	4+6(bp)		| push the integer part
	push	4+4(bp)
	push	4+2(bp)
	push	4+0(bp)

	call	.norm8		| renormalize the integer part

	mov	bx,si		| store indirectly through the pointer
	mov	cx,#8
	call	.sti

	pop	dx		| dx = frac part sign and rounding
	mov	bx,#BIAS8-11	| bx = frac part exponent
	call	.norm8		| normalize frac part
	jmp	9b		| return frac part
