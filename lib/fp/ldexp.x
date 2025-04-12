.define _ldexp

	.globl _ldexp
	.text
_ldexp:
	push	bp
	mov	bp,sp
	lea	bx,4(bp)	| push value onto the stack
	mov	cx,#8
	call	.loi
	
	mov	bx,-2(bp)	| extract value.exp
	movb	dh,bh		| extract value.sign
	shr	bx,*1
	shr	bx,*1
	shr	bx,*1
	shr	bx,*1
	and	bx,#2047	| kill sign bit

	mov	ax,-2(bp)	| remove exponent from value.mantissa
	and	ax,#0x0F
	test	bx,bx		| check for zero exponent - no leading "1"
	jz	0f		| for denormalized numbers
	or	ax,#0x10	| restore implied leading "1"
0:	mov	-2(bp),ax

	add	bx,12(bp)	| add in exponent
	cmp	bx,#0
	jl	retz		| range error - underflow
	cmp	bx,#2047
	jge	rangerr		| range error - overflow

	xorb	dl,dl		| zero rounding bits

	call	.norm8		| normalize

1:	call	.ret8
	jmp	.cret

ERANGE	=	34		| from <errno.h>
	.globl	_errno

retz:
	mov	_errno,#ERANGE	| set errno
	add	sp,#8		| remove mantissa
	call	.zrf8		| return zero
	jmp	1b
rangerr:
	mov	_errno,#ERANGE	| set errno
	add	sp,#8		| remove mantissa
	mov	bx,#huge_val	| same as HUGE_VAL in <math.h>
	mov	cx,#8
	call	.loi
	andb	dh,*128		| get the sign bit of the argument,
	orb	-1(bp),dh	| set the sign of HUGE_VAL
	jmp	1b

	.data
huge_val:
	.word	0xffff,0xffff,0xffff,0x7fff
