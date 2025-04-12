.define .mlf8
| 
| floating point multiply routine
| author: Peter S. Housel 4/1/89
|

U	=	14		| offset of multiplier
V	=	6		| offset of multiplicand
BIAS8	=	0x3FF - 1

	.text
	.globl	.mlf8
.mlf8:
	push	si
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
	jz	retz		| multiplying by zero

	mov	ax,6+V(si)	| remove exponent from v.mantissa
	and	ax,#0x0F
	test	cx,cx		| check for zero exponent - no leading "1"
	jz	0f
	or	ax,#0x10	| restore implied leading "1"
0:	mov	6+V(si),ax
	or	ax,4+V(si)
	or	ax,2+V(si)
	or	ax,0+V(si)
	jz	retz2		| multiplying by zero

	xorb	dh,dl		| xor sign bits for resultant sign
	add	bx,cx		| add exponents,
	sub	bx,#BIAS8+5	| remove excess bias, account for
				| repositioning
	
	xor	ax,ax		| initialize 128-bit product to zero
	push	ax
	push	ax
	push	ax
	push	ax
	push	ax
	push	ax
	push	ax
	push	ax
	mov	di,sp		| di => result
	push	dx		| save sign
	push	bx		| save exponent

| see Knuth, Seminumerical Algorithms, section 4.3. algorithm M

big0:	mov	ax,0+U(si)	| bigit from multiplier
	test	ax,ax
	jz	big1		| don't multiply by zero
	mov	cx,ax		| hold in cx to reduce memory reads
	mul	0+V(si)
	mov	0(di),ax	| store into result
	mov	bx,dx		| save "carry" bigit
	
	mov	ax,cx		| restore multiplier bigit
	mul	2+V(si)
|	add	ax,2(di)	| add in bigit so far
|	adc	dx,#0
	add	ax,bx		| add in carry bigit
	adc	dx,#0
	mov	2(di),ax	| store into result
	mov	bx,dx

	mov	ax,cx		| restore multiplier bigit
	mul	4+V(si)
|	add	ax,4(di)	| add in bigit so far
|	adc	dx,#0
	add	ax,bx		| add in carry bigit
	adc	dx,#0
	mov	4(di),ax	| store into result
	mov	bx,dx

	mov	ax,cx		| restore multiplier bigit
	mul	6+V(si)
|	add	ax,6(di)	| add in bigit so far
|	adc	dx,#0
	add	ax,bx		| add in carry bigit
	adc	dx,#0
	mov	6(di),ax	| store into result
	mov	8(di),dx	| store carry
|--
big1:	mov	ax,2+U(si)	| bigit from multiplier
	test	ax,ax
	jz	big2		| don't multiply by zero
	mov	cx,ax		| hold in cx to reduce memory reads
	mul	0+V(si)
	add	ax,2(di)	| add in bigit so far	
	adc	dx,#0
	mov	2(di),ax	| store into result
	mov	bx,dx		| save "carry" bigit
	
	mov	ax,cx		| restore multiplier bigit
	mul	2+V(si)
	add	ax,4(di)	| add in bigit so far
	adc	dx,#0
	add	ax,bx		| add in carry bigit
	adc	dx,#0
	mov	4(di),ax	| store into result
	mov	bx,dx

	mov	ax,cx		| restore multiplier bigit
	mul	4+V(si)
	add	ax,6(di)	| add in bigit so far
	adc	dx,#0
	add	ax,bx		| add in carry bigit
	adc	dx,#0
	mov	6(di),ax	| store into result
	mov	bx,dx

	mov	ax,cx		| restore multiplier bigit
	mul	6+V(si)
	add	ax,8(di)	| add in bigit so far
	adc	dx,#0
	add	ax,bx		| add in carry bigit
	adc	dx,#0
	mov	8(di),ax	| store into result
	mov	10(di),dx	| store carry
|--
big2:	mov	ax,4+U(si)	| bigit from multiplier
	test	ax,ax
	jz	big3		| don't multiply by zero
	mov	cx,ax		| hold in cx to reduce memory reads
	mul	0+V(si)
	add	ax,4(di)	| add in bigit so far	
	adc	dx,#0
	mov	4(di),ax	| store into result
	mov	bx,dx		| save "carry" bigit
	
	mov	ax,cx		| restore multiplier bigit
	mul	2+V(si)
	add	ax,6(di)	| add in bigit so far
	adc	dx,#0
	add	ax,bx		| add in carry bigit
	adc	dx,#0
	mov	6(di),ax	| store into result
	mov	bx,dx

	mov	ax,cx		| restore multiplier bigit
	mul	4+V(si)
	add	ax,8(di)	| add in bigit so far
	adc	dx,#0
	add	ax,bx		| add in carry bigit
	adc	dx,#0
	mov	8(di),ax	| store into result
	mov	bx,dx

	mov	ax,cx		| restore multiplier bigit
	mul	6+V(si)
	add	ax,10(di)	| add in bigit so far
	adc	dx,#0
	add	ax,bx		| add in carry bigit
	adc	dx,#0
	mov	10(di),ax	| store into result
	mov	12(di),dx	| store carry
|--
big3:	mov	ax,6+U(si)	| bigit from multiplier
	test	ax,ax
	jz	mdone		| don't multiply by zero
	mov	cx,ax		| hold in cx to reduce memory reads
	mul	0+V(si)
	add	ax,6(di)	| add in bigit so far	
	adc	dx,#0
	mov	6(di),ax	| store into result
	mov	bx,dx		| save "carry" bigit
	
	mov	ax,cx		| restore multiplier bigit
	mul	2+V(si)
	add	ax,8(di)	| add in bigit so far
	adc	dx,#0
	add	ax,bx		| add in carry bigit
	adc	dx,#0
	mov	8(di),ax	| store into result
	mov	bx,dx

	mov	ax,cx		| restore multiplier bigit
	mul	4+V(si)
	add	ax,10(di)	| add in bigit so far
	adc	dx,#0
	add	ax,bx		| add in carry bigit
	adc	dx,#0
	mov	10(di),ax	| store into result
	mov	bx,dx

	mov	ax,cx		| restore multiplier bigit
	mul	6+V(si)
	add	ax,12(di)	| add in bigit so far
	adc	dx,#0
	add	ax,bx		| add in carry bigit
	adc	dx,#0
	mov	12(di),ax	| store into result
	mov	14(di),dx	| store carry - SHOULD BE ZERO
|----
mdone:	pop	bx		| restore exponent
	pop	dx		| restore sign

2:
	mov	ax,12(di)	| multiply (shift) until
	and	ax,#0xfff0	| one in "implied" position
	jnz	3f
	shl	0(di),*1
	rcl	2(di),*1
	rcl	4(di),*1
	rcl	6(di),*1
	rcl	8(di),*1
	rcl	10(di),*1
	rcl	12(di),*1
	dec	bx		| decrement exponent
	jmp	2b
3:
	movb	dl,5(di)	| get rounding bits
	mov	ax,0(di)
	orb	al,4(di)
	or	ax,2(di)
	jz	4f
	orb	dl,*1		| set "sticky bit" if any low-order set
4:
	add	si,#U		| si => u
	add	di,#6		| di => most sig words of result
	xchg	si,di		| reverse for move into u's location
	mov
	mov
	mov
	mov
	add	sp,#16		| remove result
9:	pop	di		| restore register variables
	pop	si
	pop	cx		| save return address
	add	sp,#8		| remove v
	push	cx		| restore return address
	jmp	.norm8

retz2:
	xor	ax,ax
	mov	0+U(si),ax
	mov	2+U(si),ax
	mov	4+U(si),ax
	mov	6+U(si),ax

retz:	xor	bx,bx		| exponent = 0
	xor	dx,dx		| sign = 0, round bits = 0
	jmp	9b
