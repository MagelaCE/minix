.define .loi
.text




.loi:
pop  ax
mov dx,si
mov si,bx
mov bx,ax
mov  ax,cx
sar cx,#1
jnb 1f
xorb  ah,ah
lodb
mov si,dx
push  ax
jmp  (bx)
1:
sub  sp,ax
mov ax,di
mov  di,sp
rep
mov
mov si,dx
mov di,ax
jmp  (bx)
