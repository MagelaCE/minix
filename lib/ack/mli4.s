.define .mli4
.text

yl=2
yh=4




.mli4:
mov bx,sp
push dx
mov cx,ax
mul yh(bx) 
pop dx
push ax
mov ax,dx
mul yl(bx) 
pop dx
add  dx,ax 
mov  ax,cx
mov cx,dx
mul yl(bx) 
add  dx,cx
pop bx
add sp,#4
jmp (bx)
