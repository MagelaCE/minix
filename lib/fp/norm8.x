.define .norm8

	.globl	.norm8
|
| on entry to norm8:
|	bx=u.exp dh=u.sign dl=rounding bits
.norm8:
	push	si		| save register variables
	mov	si,sp		| si => u
	add	si,#4

	mov	ax,0(si)	| rounding and u.mant == 0?
	orb	al,dl
	or	ax,2(si)
	or	ax,4(si)
	or	ax,6(si)
	jz	retz
1:
	mov	ax,6(si)	| divide (shift) until
	and	ax,#0xffe0	| no bits above 53
	jz	2f
	shr	6(si),*1
	rcr	4(si),*1
	rcr	2(si),*1
	rcr	0(si),*1
	rcrb	dl,*1		| shift into rounding bits
	jnc	0f		| make least sig bit "sticky"
	orb	dl,*1
0:	inc	bx		| increment exponent
	jmp	1b
2:
	mov	ax,6(si)	| multiply (shift) until
	and	ax,#0xfff0	| one in "implied" position
	jnz	3f
	shlb	dl,*1		| some doubt about this one *
	rcl	0(si),*1
	rcl	2(si),*1
	rcl	4(si),*1
	rcl	6(si),*1
	dec	bx		| decrement exponent
	jmp	2b
3:
	testb	dl,dl		| check rounding bits
	jge	5f		| round down - no action necessary
	negb	dl
	jge	4f		| round up
	and	0(si),#0xfffe	| tie case - round to even
	jmp	5f
4:
	xorb	dl,dl		| zero rounding bits
	add	0(si),#1	| round up
	adc	2(si),#0
	adc	4(si),#0
	jnc	5f		| no chance of "rounding overflow"
	adc	6(si),#0	| some chance of rounding overflow - go
	jmp	1b		|  back and renormalize just in case
5:
	cmp	bx,#0		| check for exponent underflow
	jle	retz
	cmp	bx,#2047	| check for exponent overflow
	jge	oflow
6:
	shl	bx,*1		| re-position exponent
	shl	bx,*1
	shl	bx,*1
	shl	bx,*1
	mov	ax,6(si)
	and	ax,#0xf		| top mantissa bits
	or	ax,bx		| insert exponent
	andb	dh,*128		| sign bit
	orb	ah,dh		| insert sign bit
	mov	6(si),ax
	pop	si
	ret

retz:
	xor	bx,bx		| exponent = 0,
	mov	dx,bx		|      sign = 0, rounding = 0
	mov	0(si),bx
	mov	2(si),bx
	mov	4(si),bx
	mov	6(si),bx
	jmp	6b

EFOVFL	=	4
	.globl	.fat
oflow:
	mov	ax,#EFOVFL
	push	ax
	jmp	.fat

|EFUNFL	=	5
|uflow:	
|	mov	ax,#EFUNFL
|	push	ax
|	jmp	.fat
