l=/usr/lib
i=/usr/include
CFLAGS=-F

mined:	mined1.s mined2.s $i/mined.h
	@asld -i -o mined $l/crtso.s mined1.s mined2.s $l/libc.a $l/end.s
