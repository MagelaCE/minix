.define .rmi4
.text

yl=6
yh=8
xl=10
xh=12

.rmi4:
push si
push di
mov  si,sp 
mov  bx,yl(si)
mov  ax,yh(si)
cwd
cmp  dx,ax
jne  7f
and  dx,dx
jge 1f
neg  bx
je 7f
1:
xor  dx,dx
mov  cx,xl(si)
mov  ax,xh(si)
and  ax,ax
jge 2f
neg  ax
neg  cx
sbb ax,dx
2:
div bx
xchg ax,cx
div bx 
xor  bx,bx
9:
cmp  xh(si),#0
jge 1f
neg  bx
neg  dx
sbb bx,#0
1:


mov ax,dx
mov dx,bx 
pop di
pop si
pop bx
add sp,#8
jmp (bx)

7:
mov  di,ax
xor  bx,bx
and  di,di
jge 1f
neg  di
neg  yl(si)
sbb di,bx
1:
mov  ax,xl(si)
mov  dx,xh(si)
and  dx,dx
jge 1f
neg  dx
neg  ax
sbb dx,bx
1:
mov  cx,#16
1:
shl ax,#1
rcl dx,#1
rcl bx,#1
cmp  di,bx
ja 3f
jb 2f
cmp  yl(si),dx
jbe 2f
3:
loop 1b

jmp  9b
2:
sub  dx,yl(si)
sbb bx,di
inc  ax
loop 1b
1:

jmp  9b
