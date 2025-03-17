.define .iaar
.text

.iaar:
pop  cx
pop  dx
cmp  dx,#2
.globl .unknown
jne  .unknown
pop  bx 
pop  ax 
pop  dx 
sub  ax,(bx)
mul 4(bx)
mov bx,dx
add  bx,ax
push cx
ret
