.define .cfi
| 
| floating point => integer conversion routines
| author: Peter S. Housel 6/12/89,6/14/89
|

BIAS4	=	0x7F - 1
BIAS8	=	0x3FF - 1

	.text
	.globl	.cfi
|
| on entry dx=source size, cx=dest. size
|
.cfi:
	push	si		| save register variable
	mov	si,sp

	cmp	dx,#4
	jne	cfi8
cfi4:
	mov	bx,4+2(si)	| extract exp
	movb	dh,bh		| extract sign
	shl	bx,*1		| shift up one, then down 8
	movb	bl,bh
	xorb	bh,bh

	mov	ax,4+2(si)	| remove exponent from mantissa
	and	ax,#0x7F
	test	bx,bx		| check for zero exponent - no leading "1"
	jz	0f		| for denormalized numbers
	or	ax,#0x80	| restore implied leading "1"
0:	mov	4+2(si),ax

	cmp	bx,#BIAS4	| check exponent
	jl	zer4		| strictly fractional, no integer part?
	cmp	bx,#BIAS4+32	| is it too big to fit in a 32-bit integer?
	jg	toobig

1:	cmp	bx,#BIAS4+24	| shifted all the way down yet?
	jge	2f
	shr	4+2(si),*1	| shift down to align radix point;
	rcr	4+0(si),*1	| extra bits fall off the end (no rounding)
	inc	bx
	jmp	1b

2:	cmp	bx,#BIAS4+24	| do we have to shift up?
	jle	3f
	shl	4+0(si),*1	| shift up to align radix point
	rcl	4+2(si),*1
	dec	bx
	jmp	2b
zer4:
	xor	ax,ax		| make the whole thing zero
	mov	4+0(si),ax
	mov	4+2(si),ax

3:	jmp	cfi8b		| amazingly, we can share the rest of the code

cfi8:
	cmp	dx,#8
	jne	ill		| illegal EM op

	mov	bx,4+6(si)	| extract exp
	movb	dh,bh		| extract sign
	shr	bx,*1
	shr	bx,*1
	shr	bx,*1
	shr	bx,*1
	and	bx,#2047	| kill sign bit

	mov	ax,4+6(si)	| remove exponent from mantissa
	and	ax,#0x0F
	test	bx,bx		| check for zero exponent - no leading "1"
	jz	0f		| for denormalized numbers
	or	ax,#0x10	| restore implied leading "1"
0:	mov	4+6(si),ax

	cmp	bx,#BIAS8	| check exponent
	jl	zer8		| strictly fractional, no integer part?
	cmp	bx,#BIAS8+32	| is it too big to fit in a 32-bit integer?
	jg	toobig

1:	cmp	bx,#BIAS8+53	| shifted all the way down yet?
	jge	cfi8b
	shr	4+6(si),*1	| shift down to align radix point;
	rcr	4+4(si),*1	| extra bits fall off the end (no rounding)
	rcr	4+2(si),*1
	rcr	4+0(si),*1
	inc	bx
	jmp	1b
zer8:
	xor	ax,ax		| make the whole thing zero
	mov	4+0(si),ax
	mov	4+2(si),ax
	mov	4+4(si),ax
	mov	4+6(si),ax

cfi8b:	cmp	cx,#2
	jne	cfi8_4
cfi8_2:				| 8-byte float to 2-byte integer
	mov	ax,4+2(si)
	or	ax,ax
	jnz	toobig		| too big to fit into a 16-bit integer?
	mov	ax,4+0(si)
	cmp	ax,#0x8000	| -32768 is a nasty evil special case
	jnz	0f
	testb	dh,#128		| this had better be -32768 and not 32768
	jz	toobig
	jmp	3f
0:	test	ax,ax		| otherwise, sign bit set? (i.e. too big?)
	js	toobig
	testb	dh,dh		| need to negate?
	jns	3f
	neg	ax
3:	pop	si		| restore si
	pop	bx		| save program counter
	xorb	dh,dh		| dl still contains source size
	add	sp,dx		| get rid of source value
	push	ax		| put return value on stack
	push	bx		| put back return address
	ret
cfi8_4:				| 8-byte float to 4-byte integer
	cmp	cx,#4
	jne	ill
	mov	ax,4+0(si)	| put integer into registers
	mov	cx,4+2(si)
	cmp	cx,#0x8000	| -2147483648 is a nasty evil special case
	jnz	6f
	test	ax,ax
	jnz	toobig
	testb	dh,#128		| this had better be -2^31 and not 2^31
	jz	toobig
	jmp	8f
6:	test	cx,cx		| sign bit set? (i.e. too big)
	js	toobig
	testb	dh,dh		| is it negative?
	jns	8f
	neg	ax		| negate
	neg	cx
	sbb	cx,#0
8:
	pop	si		| restore si
	pop	bx		| save program counter
	xorb	dh,dh		| dl still contains source size
	add	sp,dx		| get rid of source value
	push	cx		| put return value on stack
	push	ax
	push	bx		| put back return address
	ret
ill:
	pop	bx 
	jmp	.trpilin

ECONV	=	10
	.globl	.fat
toobig:
	mov	ax,#ECONV
	push	ax
	jmp	.fat
