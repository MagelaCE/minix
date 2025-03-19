l=/usr/lib
CFLAGS=-F

mined:	mined1.s mined2.s mined.h
	@asld -i -o mined $l/crtso.s mined1.s mined2.s $l/libc.a $l/end.s
	@chmem =64000 mined
