.define _port_out, _port_in, _peek

| These routines are used by the kernel to write and read I/O ports.
| They can also be called by user programs

| port_out(port, value) writes 'value' on the I/O port 'port'.

_port_out:
	push bx			| save bx
	mov bx,sp		| index off bx
	push ax			| save ax
	push dx			| save dx
	mov dx,4(bx)		| dx = port
	mov ax,6(bx)		| ax = value
	out			| output 1 byte
	pop dx			| restore dx
	pop ax			| restore ax
	pop bx			| restore bx
	ret			| return to caller



| port_in(port, &value) reads from port 'port' and puts the result in 'value'.
_port_in:
	push bx			| save bx
	mov bx,sp		| index off bx
	push ax			| save ax
	push dx			| save dx
	mov dx,4(bx)		| dx = port
	in			| input 1 byte
	xorb ah,ah		| clear ah
	mov bx,6(bx)		| fetch address where byte is to go
	mov (bx),ax		| return byte to caller in param
	pop dx			| restore dx
	pop ax			| restore ax
	pop bx			| restore bx
	ret			| return to caller

| value = peek(segment, offset)
_peek:
	push bp			| save bp
	mov bp,sp		| we need to access parameters
	push es			| save es
	mov es,4(bp)		| load es with segment value
	mov bx,6(bp)		| load bx with offset from segment
	seg es			| go get the byte
	movb al,(bx)		| al = byte
	xorb ah,ah		| ax = byte
	pop es			| restore es
	pop bp			| restore bp
	ret			| return to caller
