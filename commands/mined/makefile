l=../../lib
CFLAGS=-O -I../../include

mined:	mined1.o mined2.o mined.h
	@ld -s -i -o ../bin/mined $l/crtso.o mined1.o mined2.o $l/libc.a $l/end.o
