.define .ret8

	.globl	.ret8
	.text
.ret8:
	push	bp
	mov	bp,sp
	mov	ax,4(bp)
	mov	dx,6(bp)
	mov	bx,8(bp)
	mov	cx,10(bp)
	pop	bp
	ret	8
