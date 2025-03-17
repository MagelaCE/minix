.define .error





.text
.error:
push bp
push si
push di
push dx
push cx
push bx
push ax
mov  cx,ax
mov  bx,#1
sal bx,cl
.globl .ignmask
.globl .trp
test bx,.ignmask
jne  2f
call  .trp
2:
pop  ax
pop  bx
pop  cx
pop  dx
pop  di
pop  si
pop  bp
ret
