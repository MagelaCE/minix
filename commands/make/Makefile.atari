# Makefile for make!  (Minix ST)

CFLAGS = -O -Dunix 

OBJS =	check.o input.o macro.o main.o \
	make.o reader.o rules.o

make :	$(OBJS)
	$(CC) -o make $(OBJS)
 
$(OBJS): h.h
