.define .isar
.text

.isar:
pop  cx
pop  ax
cmp  ax,#2
.globl .unknown
jne  .unknown
pop  bx 
pop  ax 
push  cx
.globl .sar2
jmp  .sar2
