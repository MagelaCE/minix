.define .ciu
.define .cui
.define .cuu

.text
.ciu:
.cui:
.cuu:
pop  bx 



cmp  dx,cx
je 8f
cmp  dx,#2
je 1f
cmp  dx,#4
jne  9f
cmp  cx,#2
jne  9f
pop  dx
8:
jmp  (bx)
1:
cmp  cx,#4
jne  9f
xor  dx,dx
push  dx
jmp  (bx)
9:
push  ax 
EILLINS = 18
.globl .fat
mov  ax,#EILLINS
push  ax
jmp  .fat
