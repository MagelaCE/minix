.define .set
.text



.set:
pop  bx 
xor  dx,dx

sub sp,cx
push bx
push di
mov  bx,sp
xor di,di
sar cx,#1
1:
mov  4(bx_di),dx
inc di
inc di
loop 1b

mov  bx,#8
div bx
cmp  ax,di
jae 2f
mov di,dx
movb dl,bits(di)
mov  di,sp
add  di,ax
orb 4(di),dl
pop di
ret
2:
ESET = 2
.globl .error
pop di
mov  ax,#ESET
call .error
ret

.data
bits:
.byte 1,2,4,8,16,32,64,128
