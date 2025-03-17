.define .lar2
.text

.lar2:


pop cx
pop dx 
push cx
push si
mov si,dx
sub  ax,(bx)
mov  cx,4(bx)
imul cx
add  si,ax
sar cx,#1
jnb 1f
xorb  ah,ah
lodb
pop si
pop bx
push  ax
jmp  (bx)
1:
pop dx 
mov ax,4(bx)
pop bx 
sub  sp,ax
mov ax,di 
mov  di,sp
rep
mov
mov di,ax
mov si,dx
jmp  (bx)
