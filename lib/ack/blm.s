.define .blm
.text


.blm:
mov bx,sp
mov ax,si
mov dx,di
mov di,2(bx)
mov si,4(bx)
rep
mov
mov si,ax
mov di,dx
pop bx
add sp,#4
jmp (bx)

