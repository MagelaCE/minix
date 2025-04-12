.define .ngf8

	.globl	.ngf8
	.text
.ngf8:				| floating point negate
	push	si
	mov	si,sp
	xorb	7+4(si),*0x80	| flip sign bit
	pop	si
	ret
