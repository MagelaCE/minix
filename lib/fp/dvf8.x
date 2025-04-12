.define .dvf8
| 
| floating point divide routine
| author: Peter S. Housel 4/8/89,6/2/89,6/13/89
|

U	=	14		| offset of dividend
V	=	6		| offset of divisor
BIAS8	=	0x3FF - 1

	.text
	.globl	.dvf8
.dvf8:
	push	si		| save register variables
	push	di

	mov	si,sp		| point to arguments

	mov	bx,6+U(si)	| extract u.exp
	movb	dh,bh		| extract u.sign
	shr	bx,*1
	shr	bx,*1
	shr	bx,*1
	shr	bx,*1
	and	bx,#2047	| kill sign bit

	mov	cx,6+V(si)	| extract v.exp
	movb	dl,ch		| extract v.sign
	shr	cx,*1
	shr	cx,*1
	shr	cx,*1
	shr	cx,*1
	and	cx,#2047	| kill sign bit

	mov	ax,6+U(si)	| remove exponent from u.mantissa
	and	ax,#0x0F
	test	bx,bx		| check for zero exponent - no leading "1"
	jz	0f
	or	ax,#0x10	| restore implied leading "1"
0:	mov	6+U(si),ax
	or	ax,4+U(si)
	or	ax,2+U(si)
	or	ax,0+U(si)
	jz	retz		| dividing zero

	mov	ax,6+V(si)	| remove exponent from v.mantissa
	and	ax,#0x0F
	test	cx,cx		| check for zero exponent - no leading "1"
	jz	0f
	or	ax,#0x10	| restore implied leading "1"
0:	mov	6+V(si),ax
	or	ax,4+V(si)
	or	ax,2+V(si)
	or	ax,0+V(si)
	jz	divz		| dividing by zero

	xorb	dh,dl		| xor sign bits for resultant sign
	sub	bx,cx		| subtract exponents,
	add	bx,#BIAS8-11+1	| add bias back in, account for shift

	xor	ax,ax		| zero the quotient
	push	ax
	push	ax
	push	ax
	push	ax
	mov	di,sp		| di => quotient

	mov	ax,0+V(si)	| initial subtraction
	sub	0+U(si),ax
	mov	ax,2+V(si)
	sbb	2+U(si),ax
	mov	ax,4+V(si)
	sbb	4+U(si),ax
	mov	ax,6+V(si)
	sbb	6+U(si),ax

	mov	cx,#64		| loop on all bits
	jmp	3f		| skip first shift of quotient
2:
	shl	0(di),*1	| shift quotient
	rcl	2(di),*1
	rcl	4(di),*1
	rcl	6(di),*1
3:
	shl	0+U(si),*1	| shift dividend
	rcl	2+U(si),*1
	rcl	4+U(si),*1
	rcl	6+U(si),*1

	jc	4f		| if carry set, add

	mov	ax,0+V(si)	| else subtract
	sub	0+U(si),ax
	mov	ax,2+V(si)
	sbb	2+U(si),ax
	mov	ax,4+V(si)
	sbb	4+U(si),ax
	mov	ax,6+V(si)
	sbb	6+U(si),ax

	orb	0(di),*1	| set bit zero of quotient

	loop	2b
	jmp	5f
4:
	mov	ax,0+V(si)	| add (restore)
	add	0+U(si),ax
	mov	ax,2+V(si)
	adc	2+U(si),ax
	mov	ax,4+V(si)
	adc	4+U(si),ax
	mov	ax,6+V(si)
	adc	6+U(si),ax

	loop	2b		| loop
5:
	add	si,#U		| si => u, di => quotient
	xchg	si,di		| reverse for move into u's location
	mov
	mov
	mov
	mov
	add	sp,#8		| remove result
9:	pop	di		| restore register variables
	pop	si
	pop	cx		| save return address
	add	sp,#8		| remove v
	push	cx		| restore return address
	xorb	dl,dl		| zero rounding bits
	jmp	.norm8

retz:	xor	bx,bx
	xor	dx,dx
	jmp	9b

	.globl	.fat
EFDIVZ	=	7
divz:
	mov	ax,#EFDIVZ
	push	ax
	call	.fat
