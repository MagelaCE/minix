.define __mul10add,__adjust

| 
| auxiliary routines for strtod()
| author: Peter S. Housel 6/3/89
|

BIAS8	=	0x3FF - 1

	.globl __mul10add
	.text
__mul10add:
	push	bp
	mov	bp,sp
	push	si
	mov	si,4(bp)

	mov	ax,#1			| return 1 if overflow
	cmp	6(si),#3276		| would this digit cause overflow?
	jg	1f

	shl	0(si),*1		| multiply accumulator by 10 - first
	rcl	2(si),*1		|  we multiply by two
	rcl	4(si),*1
	rcl	6(si),*1

	push	6(si)			| save 2x value
	push	4(si)
	push	2(si)
	push	0(si)

	shl	0(si),*1		| multiply by four, to make this
	rcl	2(si),*1		|  8x the original accumuator
	rcl	4(si),*1
	rcl	6(si),*1
	shl	0(si),*1
	rcl	2(si),*1
	rcl	4(si),*1
	rcl	6(si),*1

	pop	ax			| add 2x value back in
	add	0(si),ax
	pop	ax
	adc	2(si),ax
	pop	ax
	adc	4(si),ax
	pop	ax
	adc	6(si),ax

	mov	ax,6(bp)		| get digit value
	add	0(si),ax		| add it in
	adc	2(si),#0
	adc	4(si),#0
	adc	6(si),#0

	xor	ax,ax			| return zero overflow flag
1:	jmp	.sret


	.globl	__adjust
__adjust:
	push	bp
	mov	bp,sp
	push	si			| si => 64-bit accumulator
	mov	si,4(bp)
	push	di			| di = decimal exponent
	mov	di,6(bp)

	mov	bx,#0			| bx = binary scale factor
	test	di,di			| check decimal exponent
	jz	9f			| if zero, no scaling necessary
	js	4f			| if negative, do division loop

1:	cmp	6(si),#6553		| compare with 2^15 / 5
	jb	2f			| no danger of overflow?
	shr	6(si),*1		| could overflow; divide by two
	rcr	4(si),*1		| to prevent it
	rcr	2(si),*1
	rcr	0(si),*1
	inc	bx			| increment scale factor to compensate
	jmp	1b			| try again to see if we're ok now
2:					| now we multiply by 5:
	push	6(si)			| save 1x value
	push	4(si)
	push	2(si)
	push	0(si)

	shl	0(si),*1		| multiply by four, 
	rcl	2(si),*1
	rcl	4(si),*1
	rcl	6(si),*1

	shl	0(si),*1
	rcl	2(si),*1
	rcl	4(si),*1
	rcl	6(si),*1

	pop	ax			| add 1x value back in
	add	0(si),ax
	pop	ax
	adc	2(si),ax
	pop	ax
	adc	4(si),ax
	pop	ax
	adc	6(si),ax

	inc	bx			| increment scale factor to make this
					| a multiplication by 10
	sub	di,#1			| decrement decimal exponent
	jnz	1b			| keep scaling if not done

	jmp	9f			| done with multiplication loop

4:
	test	6(si),#0xC000		| make sure upper bits set to preserve
	jnz	5f			| as much precision as possible
	shl	0(si),*1		| if not, multiply by 2
	rcl	2(si),*1
	rcl	4(si),*1
	rcl	6(si),*1
	dec	bx			| decrement scale factor to compensate
	jmp	4b			| and check again
5:
	mov	cx,#5		 	| division by 5
	xor	dx,dx			| start off with zero remainder
	mov	ax,6(si)		| get word
	div	cx			| divide dx/ax by 5
	mov	6(si),ax		| store result; dx contains remainder
	mov	ax,4(si)
	div	cx
	mov	4(si),ax
	mov	ax,2(si)
	div	cx
	mov	2(si),ax
	mov	ax,0(si)
	div	cx
	mov	0(si),ax

	dec	bx			| increment scale factor to make this
					| a division by 10
	add	di,#1			| increment decimal exponent
	jnz	4b			| keep looping if not done

9:	push	bx			| scale factor; multiplier for ldexp()

	push	6(si)			| copy value onto stack
	push	4(si)
	push	2(si)
	push	0(si)

	xor	dx,dx			| sign = 0, rounding bits = 0
	mov	ax,8(bp)		| check sign flag argument
	test	ax,ax
	jz	0f
	orb	dh,*128			| set sign bit in dh if negative
0:	mov	bx,#BIAS8+53		| first normalize the number as
	call	.norm8			| an integer,

	call	_ldexp			| then re-scale it using ldexp()
					| to check for over/underflow
	add	sp,#10			| remove args

|	call	.lfr8			| these cancel each other out;
|	call	.ret8			| code generator should special-case
	jmp	.dsret
