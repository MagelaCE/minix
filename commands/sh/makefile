CFLAGS=-F
l=/usr/lib

shobj = sh1.s sh2.s sh3.s sh4.s sh5.s sh6.s
sh:	$(shobj) sh.h
	@asld -i -o sh -T. $l/crtso.s $(shobj) $l/libc.a $l/end.s
	@chmem =8000 sh
