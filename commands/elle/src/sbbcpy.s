/ AS version of bcopy routine for PDP-11, by KLH@SRI, 1985
/
/ bcopy(from,to,cnt)
/ char *from, *to; unsigned cnt;
.globl _bcopy

_bcopy:
	jsr	r5,csv
	mov	010(r5),r2	/ Count
	beq	9f		/ If count zero, return.
	mov	04(r5),r4	/ Source ptr
	mov	06(r5),r3	/ Dest ptr

	/ Check for ability to do word moves.
	bit	$01,r3		/ If dest ptr odd,
	beq	1f
	movb	(r4)+,(r3)+	/ try moving 1 byte.
	dec	r2
	beq	9f		/ Might be just that one.
1:	bit	$01,r4		/ Then see if source ptr even.
	beq	2f		/ If even, jump to use word moves.

	/ Do byte-by-byte copy
5:	movb	(r4)+,(r3)+
	sob	r2,5b
	br	9f

	/ Do word-by-word copy
2:	clr	r0		/ Clear carry bit
	ror	r2		/ Get # of words to move
	beq	7f		/ If only one byte, do just that one.
	bcs	6f		/ If carry set, we'll have an extra byte
3:	mov	(r4)+,(r3)+
	sob	r2,3b
9:	jmp	cret

6:	mov	(r4)+,(r3)+
	sob	r2,6b
7:	movb	(r4),(r3)	/ Move last (odd) byte
	br 9b
