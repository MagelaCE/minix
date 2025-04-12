.define _frexp

BIAS8	=	0x3ff - 1

	.globl	_frexp
	.text
_frexp:
	push	bp
	mov	bp,sp
	lea	bx,4(bp)
	mov	cx,#8		| copy value onto stack
	call	.loi

	mov	cx,-2(bp)	| extract value.exp
	shr	cx,*1
	shr	cx,*1
	shr	cx,*1
	shr	cx,*1
	and	cx,#2047	| kill sign bit

	mov	ax,-2(bp)	| remove exponent from value.mantissa
	and	ax,#0x0F
	test	cx,cx		| check for zero exponent - no leading "1"
	jz	0f		| for denormalized numbers
	or	ax,#0x10	| restore implied leading "1"
0:	mov	-2(bp),ax

	sub	cx,#BIAS8	| remove bias

	or	ax,-4(bp)	| check for zero
	or	ax,-6(bp)
	or	ax,-8(bp)
	jnz	1f
	xor	cx,cx		| if zero, exponent is zero
1:
	mov	bx,12(bp)	| store exponent
	mov	(bx),cx

	mov	bx,#BIAS8	| set bias for return value
	xor	dx,dx		| sign = 0, rounding = 0
	call	.norm8
	call	.ret8
	jmp	.cret
