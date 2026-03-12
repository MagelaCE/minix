l=../lib
i=../include
CFLAGS=-O -I../include

mined:	mined1.o mined2.o $i/mined.h
	@ld -s -i -o bin/mined $l/crtso.o mined1.o mined2.o $l/libc.a $l/end.o
