.define .ior
.text


.ior:
pop bx 
mov dx,di
mov di,sp
add di,cx
sar cx,#1
1:
pop ax
or ax,(di)
stow
loop 1b
mov di,dx
jmp (bx)
