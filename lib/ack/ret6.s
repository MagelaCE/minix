.define .ret6
.text
.globl .retarea

.ret6:
pop bx
pop .retarea
pop .retarea+2
pop .retarea+4
jmp (bx)
