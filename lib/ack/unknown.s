.define .unknown
.text
.globl .fat
EILLINS = 18

.unknown:
mov  ax,#EILLINS
push ax
jmp  .fat
