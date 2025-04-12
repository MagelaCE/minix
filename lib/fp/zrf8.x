.define .zrf8

	.globl	.zrf8
	.text
.zrf8:				| push (double) +0.0
	pop	bx		| save return address
	xor	ax,ax		| ax = 0
	push	ax		| push four zero words
	push	ax
	push	ax
	push	ax
	push	bx		| restore return address
	ret
