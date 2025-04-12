.define .fat

	.text
	.globl	.fat
	.globl	.trp,.stop
.fat:
	call	.trp
	call	.stop

