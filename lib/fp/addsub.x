.define .sbf8,.adf8
| 
| floating point add/subtract routines
| author: Peter S. Housel 9/21/88,01/17/89,03/19/89,5/24/89
|

V	=	6		| stack offset of v
U	=	V + 8		| stack offset of u

	.text
	.globl	.sbf8,.adf8,.norm8
.sbf8:
	push	si		| save register variables
	push	di
	mov	di,sp		| di => v
	add	di,#V
	xorb	7(di),*128	| negate second argument
				| fall through to adf8
	jmp	0f
.adf8:
	push	si		| save register variables
	push	di
	mov	di,sp
	add	di,#V		| di => v
0:	mov	si,sp
	add	si,#U		| si => u

	mov	bx,6(si)	| extract u.exp
	movb	dh,bh		| extract u.sign
	shr	bx,*1
	shr	bx,*1
	shr	bx,*1
	shr	bx,*1
	and	bx,#2047	| kill sign bit

	mov	cx,6(di)	| extract v.exp
	movb	dl,ch		| extract v.sign
	shr	cx,*1
	shr	cx,*1
	shr	cx,*1
	shr	cx,*1
	and	cx,#2047	| kill sign bit

	mov	ax,6(si)	| remove exponent from u.mantissa
	and	ax,#0x0F
	test	bx,bx		| check for zero exponent - no leading "1"
	jz	0f		| for denormalized numbers
	or	ax,#0x10	| restore implied leading "1"
0:	mov	6(si),ax

	mov	ax,6(di)	| remove exponent from v.mantissa
	and	ax,#0x0F
	test	cx,cx		| check for zero exponent - no leading "1"
	jz	0f		| for denormalized numbers
	or	ax,#0x10	| restore implied leading "1"
0:	mov	6(di),ax

	mov	ax,bx
	sub	ax,cx		| ax = u.exp - v.exp
	movb	cl,*0		| (put initial zero rounding bits in cl)
	je	4f		| exponents are equal - no shifting necessary
	jg	1f		| not equal but no exchange necessary
	xchg	si,di		| exchange u and v
	xchgb	dh,dl
	sub	bx,ax		| bx = u.exp - (u.exp - v.exp) = v.exp
	neg	ax
1:
	cmp	ax,#53		| is u so much bigger that v is not
	jge	7f		| significant?

	movb	ch,#10		| shift u left up to 10 bits to minimize loss
2:
	shl	0(si),*1
	rcl	2(si),*1
	rcl	4(si),*1
	rcl	6(si),*1
	dec	bx		| decrement exponent
	dec	ax
	jz	4f		| done shifting altogether
	decb	ch
	jnz	2b		| still can shift u.mant more
3:
	shr	6(di),*1	| shift v.mant right the rest of the way
	rcr	4(di),*1	| to line it up with u.mant
	rcr	2(di),*1
	rcr	0(di),*1
	rcrb	cl,*1		| shift into rounding bits
	jnc	0f
	orb	cl,*1		| make least sig bit "sticky"
0:	dec	ax		| loop
	jne	3b
4:
	movb	al,dh		| are the signs equal?
	xorb	al,dl
	andb	al,*128
	je	6f		| yes, no negate necessary

	notb	cl		| negate rounding bits and v.mant
	addb	cl,*1
	not	0(di)
	adc	0(di),#0
	not	2(di)
	adc	2(di),#0
	not	4(di)
	adc	4(di),#0
	not	6(di)
	adc	6(di),#0
6:
	push	ax		| save signs flag
	mov	ax,0(di)	| u.mant = u.mant + v.mant
	add	0(si),ax
	mov	ax,2(di)
	adc	2(si),ax
	mov	ax,4(di)
	adc	4(si),ax
	mov	ax,6(di)
	adc	6(si),ax

	pop	ax		| restore u.sign ^ v.sign
	jc	7f		| needn't negate
	testb	al,al		| opposite signs?
	je	7f		| don't need to negate result

	notb	cl		| negate rounding bits and u.mant
	addb	cl,*1
	not	0(si)
	adc	0(si),#0
	not	2(si)
	adc	2(si),#0
	not	4(si)
	adc	4(si),#0
	not	6(si)
	adc	6(si),#0
	xorb	dh,*128		| switch sign
7:
	movb	dl,cl		| put rounding bits in dl for .norm8
	mov	di,sp		| set destination = u
	add	di,#U
	mov			| if we swapped, move v onto u;
	mov			| if we didn't swap, move u onto itself
	mov
	mov
	pop	di		| restore register variables
	pop	si
	pop	cx		| cx = return address
	add	sp,#8		| remove v from the stack
	push	cx		| put the return address back
	jmp	.norm8
