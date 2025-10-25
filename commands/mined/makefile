CFLAGS=-F
l=/usr/lib

obj = mined1.s mined2.s
sh:	$(obj) mined.h
	@asld -i -o mined -T. $l/crtso.s $(obj) $l/libc.a $l/end.s
	@chmem =64000 mined
