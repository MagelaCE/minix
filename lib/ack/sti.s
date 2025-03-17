.define .sti
.text




.sti:
mov dx,di 
pop  ax 
mov di,bx
mov bx,ax
sar cx,#1
jnb 1f
pop  ax
stob
mov di,dx
jmp  (bx)
1:
mov ax,si
mov  si,sp
rep
mov
mov  sp,si
mov di,dx
mov si,ax
jmp  (bx)
