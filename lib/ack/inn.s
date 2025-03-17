.define .inn
.text



.inn:
xor  dx,dx
mov  bx,#8
div bx
mov  bx,sp
add bx,#2
add  bx,ax
cmp  ax,cx
jae 1f
movb al,(bx)
mov bx,dx
testb al,bits(bx)
jz 1f
mov ax,#1
jmp 2f
1:
xor ax,ax
2:
pop bx
add  sp,cx

jmp  (bx)

.data
bits:
.byte 1,2,4,8,16,32,64,128
