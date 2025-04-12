.define .cuf
| 
| floating point <=> unsigned conversion routines
| author: Peter S. Housel 3/28/89
|

BIAS4	=	0x7F - 1
BIAS8	=	0x3FF - 1

	.text
	.globl	.cuf
|
| on entry dx=source size, cx=dest. size
|
.cuf:
	pop	bx		| save return address
	cmp	dx,#2
	jne	cuf4
cuf2:
	cmp	cx,#4
	jne	cuf2_8
cuf2_4:				| 2-byte unsigned to 4-byte float
	xor	ax,ax
	push	ax		| pad rest of mantissa with zeroes
	push	bx		| restore return address
	mov	bx,#BIAS4+16-8	| radix point just after most sig. word
	xor	dx,dx		| sign = rounding = 0
	jmp	.norm4
cuf2_8:
	cmp	cx,#8
	jne	ill		| illegal EM op
	xor	ax,ax
	push	ax		| pad rest of mantissa with zeroes
	push	ax
	push	ax	
	push	bx		| restore return address
	mov	bx,#BIAS8+16-11	| radix point just after most sig. word
	xor	dx,dx		| sign = rounding = 0
	jmp	.norm8

cuf4:
	cmp	dx,#4
	jne	ill		| illegal EM op
	cmp	cx,#4
	jne	cuf4_8
cuf4_4:				| 4-byte unsigned to 4-byte float
	push	bx		| restore return address
	mov	bx,#BIAS4+32-8	| radix point at end
	xor	dx,dx		| sign = rounding = 0
	jmp	.norm4
cuf4_8:
	cmp	cx,#8
	jne	ill		| illegal EM op
	xor	ax,ax
	push	ax		| zero-fill rest of mantissa
	push	ax
	push	bx		| restore return address
	mov	bx,#BIAS8+32-11	| radix point just after first 32 bits
	xor	dx,dx		| sign = rounding = 0
	jmp	.norm8

ill:
	pop	bx 
	jmp	.trpilin
