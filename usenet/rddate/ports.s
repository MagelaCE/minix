|
|	routines to read/write i/o ports
|	Similar to those in klib88.s
|

	.globl	_port_in, _port_out

|
|	i = port_in(port)
|
_port_in:
	push	bx
	mov	bx,sp
	push	dx
	mov	dx,4(bx)
	in
	xorb	ah,ah
	pop	dx
	pop	bx
	ret

|
|	port_out(port, data)
|
_port_out:
	push	bx
	mov	bx,sp
	push	ax
	push	dx
	mov	dx,4(bx)
	mov	ax,6(bx)
	out
	pop	dx
	pop	ax
	pop	bx
	ret
