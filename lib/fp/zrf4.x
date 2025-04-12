.define .zrf4

	.globl	.zrf4
	.text
.zrf4:				| push (float) 0.0
	pop	bx		| save return address
	xor	ax,ax		| ax = 0
	push	ax		| push two zero words
	push	ax
	push	bx		| restore return address
	ret
