.define .ilar
.text

.ilar:
pop  cx
pop  dx
.globl .unknown
cmp  dx,#2
jne  .unknown
pop  bx 
pop  ax 
push  cx
.globl .lar2
jmp  .lar2
