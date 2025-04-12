.define .norm4

	.globl	.norm4
|
| on entry to norm4:
|	bx=u.exp dh=u.sign dl=rounding bits
.norm4:
	push	si		| save register variables
	mov	si,sp		| si => u
	add	si,#4

	mov	ax,0(si)	| rounding and u.mant == 0?
	orb	al,dl
	or	ax,2(si)
	jz	retz
1:
	mov	ax,2(si)	| divide (shift) until
	and	ax,#0xff00	| no bits above 23
	jz	2f
	shr	2(si),*1
	rcr	0(si),*1
	rcrb	dl,*1		| shift into rounding bits
	jnc	0f		| make least sig bit "sticky"
	orb	dl,*1
0:	inc	bx		| increment exponent
	jmp	1b
2:
	mov	ax,2(si)	| multiply (shift) until
	and	ax,#0xff80	| one in "implied" position
	jnz	3f
	shlb	dl,*1		| some doubt about this one *
	rcl	0(si),*1
	rcl	2(si),*1
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
	jmp	1b		| go back and renormalize just in case
5:
	cmp	bx,#0		| check for exponent overflow or underflow
	jle	retz
	cmp	bx,#255
	jge	oflow
6:
	movb	cl,*7		| re-position exponent
	shl	bx,cl
	mov	ax,2(si)
	and	ax,#0x7f	| top mantissa bits
	or	ax,bx		| insert exponent
	andb	dh,*128		| sign bit
	orb	ah,dh		| insert sign bit
	mov	2(si),ax
	pop	si		| restore register variables
	ret

retz:
	xor	bx,bx		| exponent = 0,
	mov	dx,bx		|      sign = 0, rounding = 0,
	mov	0(si),bx	|      mantissa = 0
	mov	2(si),bx
	jmp	6b

	.globl	.fat
ECONV	=	10
	mov	ax,#ECONV
	push	ax
	jmp	.fat	

EFOVFL	=	4
oflow:
	mov	ax,#EFOVFL
	push	ax
	jmp	.fat

|EFUNFL	=	5
|uflow:	
|	mov	ax,#EFUNFL
|	push	ax
|	jmp	.fat
