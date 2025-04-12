.define .cif
| 
| floating point <=> integer conversion routines
| author: Peter S. Housel 3/28/89
|

BIAS4	=	0x7F - 1
BIAS8	=	0x3FF - 1

	.text
	.globl	.cif
|
| on entry dx=source size, cx=dest. size
|
.cif:
	pop	bx		| save return address
	cmp	dx,#2
	jne	cif4
cif2:
	pop	ax
	test	ax,ax		| check sign of ax
	jge	1f		| nonnegative
	neg	ax		| take absolute value
	movb	dh,*128		| set sign flag to negative
1:	push	ax
	cmp	cx,#4
	jne	cif2_8
cif2_4:				| 2-byte integer to 4-byte float
	xor	ax,ax
	push	ax		| pad rest of mantissa with zeroes
	push	bx		| restore return address
	mov	bx,#BIAS4+16-8	| radix point just after most sig. word
	xorb	dl,dl		| set rounding = 0
	jmp	.norm4
cif2_8:				| 2-byte integer to 8-byte double
	cmp	cx,#8
	jne	ill		| illegal EM op
	xor	ax,ax
	push	ax		| pad rest of mantissa with zeroes
	push	ax
	push	ax	
	push	bx		| restore return address
	mov	bx,#BIAS8+16-11	| radix point just after most sig. word
	xorb	dl,dl		| rounding = 0
	jmp	.norm8

cif4:
	cmp	dx,#4
	jne	ill		| illegal EM op
	pop	ax
	pop	dx
	test	dx,dx		| check sign of number
	jge	2f
	not	ax		| negate
	not	dx
	inc	ax
	adc	dx,#0
	push	dx
	movb	dh,*128
	jmp	3f
2:	push	dx
3:	push	ax
	cmp	cx,#4
	jne	cif4_8
cif4_4:				| 4-byte unsigned to 4-byte float
	push	bx		| restore return address
	mov	bx,#BIAS4+32-8	| radix point at end
	xorb	dl,dl		| rounding = 0 (if positive, dh already 0)
	jmp	.norm4
cif4_8:
	cmp	cx,#8
	jne	ill		| illegal EM op
	xor	ax,ax
	push	ax		| zero-fill rest of mantissa
	push	ax
	push	bx		| restore return address
	mov	bx,#BIAS8+32-11	| radix point just after first 32 bits
	xorb	dl,dl		| rounding = 0 (if positive, dh already 0)
	jmp	.norm8

ill:
	pop	bx 
	jmp	.trpilin
