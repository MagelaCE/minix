.define .cff
| 
| floating point <=> floating point conversion routines
| author: Peter S. Housel 06/03/89
|

BIAS4	=	0x7F - 1
BIAS8	=	0x3FF - 1

	.text
	.globl	.cff
|
| on entry dx=source size, cx=dest. size
|
.cff:
	cmp	dx,#4
	jne	cff8
cff4:
	cmp	cx,#4
	jne	cff4_8
cff4_4:				| redundant 4-byte float to 4-byte float
	ret
cff4_8:				| 4-byte float to 8-byte double
	cmp	cx,#8
	jne	ill		| illegal EM op

	pop	cx		| cx = return address
	xor	ax,ax		| append 32 bits of zeroes
	push	ax
	push	ax
	push	cx		| put return address back
	push	si
	mov	si,sp

	mov	bx,4+6(si)	| extract exponent
	movb	dh,bh		| extract sign
	movb	cl,*7
	shr	bx,cl
	xorb	bh,bh		| kill sign bit (exponent is 8 bits)

	mov	ax,4+6(si)	| remove exponent from mantissa
	and	ax,#0x7F
	test	bx,bx		| check for zero exponent - no leading "1"
	jz	0f		| for denormalized numbers
	or	ax,#0x80	| restore implied leading "1"
0:	mov	4+6(si),ax

	add	bx,#BIAS8-BIAS4-3	| adjust bias, account for shift
	xorb	dl,dl

	pop	si		| restore si
	jmp	.norm8		| normalize and return
cff8:
	cmp	dx,#8
	jne	ill		| illegal EM op

	cmp	cx,#4
	jne	cff8_8
cff8_4:				| 8-byte double to 4-byte float
	push	si
	mov	si,sp

	mov	bx,4+6(si)	| extract exp
	movb	dh,bh		| extract sign
	shr	bx,*1
	shr	bx,*1
	shr	bx,*1
	shr	bx,*1
	and	bx,#2047	| kill sign bit

	mov	ax,4+6(si)	| remove exponent from v.mantissa
	and	ax,#0x0F
	test	bx,bx		| check for zero exponent - no leading "1"
	jz	0f		| for denormalized numbers
	or	ax,#0x10	| restore implied leading "1"
0:	mov	4+6(si),ax

	add	bx,#BIAS4-BIAS8	| adjust bias

	mov	cx,#3		| shift up to realign mantissa for floats
1:	shl	4+0(si),*1
	rcl	4+2(si),*1
	rcl	4+4(si),*1
	rcl	4+6(si),*1
	loop	1b

	movb	dl,4+3(si)	| set rounding bits
	mov	ax,4+0(si)	| check to see if sticky bit should be set
	orb	al,4+2(si)
	or	ax,ax
	jz	2f
	orb	dl,*1		| set sticky bit

2:	pop	si		| restore si
	pop	cx		| cx = return address
	add	sp,#4		| remove lower 32 bits
	push	cx		| put return address back
	jmp	.norm4
cff8_8:
	cmp	cx,#8
	jne	ill		| illegal EM op
	ret			| redundant

ill:
	pop	bx 
	jmp	.trpilin
