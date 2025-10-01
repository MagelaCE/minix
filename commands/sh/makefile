l=../../lib
CFLAGS=-O -I../../include

shobj = sh1.o sh2.o sh3.o sh4.o sh5.o
sh:	$(shobj) sh.h
	@ld -s -i -o ../bin/sh $l/crtso.o $(shobj) $l/libc.a $l/end.o
	@chmem =8000 ../bin/sh
