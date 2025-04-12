.define .cmf8
| 
| floating point compare routine
| author: Peter S. Housel 6/3/89
|

V	=	4		| offset of second operand
U	=	V + 8		| offset of first operand

	.text
	.globl	.cmf8
.cmf8:
	push	si
	mov	si,sp

	mov	ax,6+U(si)
	test	ax,ax		| check sign bit
	jns	1f
	not	0+U(si)		| negate
	add	0+U(si),#1
	not	2+U(si)
	adc	2+U(si),#1
	not	4+U(si)
	adc	4+U(si),#1
	not	6+U(si)
	adc	6+U(si),#1
	xorb	7+U(si),*128	| toggle sign bit
1:
	mov	ax,6+V(si)
	test	ax,ax		| check sign bit
	jns	2f
	not	0+V(si)		| negate
	add	0+V(si),#1
	not	2+V(si)
	adc	2+V(si),#1
	not	4+V(si)
	adc	4+V(si),#1
	not	6+V(si)
	adc	6+V(si),#1
	xorb	7+V(si),*128	| toggle sign bit
2:
	mov	ax,6+U(si)
	cmp	ax,6+V(si)
	jg	gt
	jl	lt
	mov	ax,4+U(si)
	cmp	ax,4+V(si)
	ja	gt
	jb	lt
	mov	ax,2+U(si)
	cmp	ax,2+V(si)
	ja	gt
	jb	lt
	mov	ax,0+U(si)
	cmp	ax,0+V(si)
	ja	gt
	jb	lt
eq:
	xor	ax,ax
	jmp	9f
lt:
	mov	ax,#-1
	jmp	9f
gt:
	mov	ax,#1

9:	pop	si		| restore register variable
	pop	bx		| bx = return address
	add	sp,#16		| remove arguments
	push	ax		| put flag on stack
	push	bx		| restore return address
	ret
