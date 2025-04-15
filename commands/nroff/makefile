# Makefile for nroff
CFLAGS=-D_MINIX -D_POSIX_SOURCE
OBJS    = main.s command.s text.s io.s macros.s strings.s escape.s low.s

all:	$(OBJS) nroff.h
	cc -i -o nroff $(OBJS)

clean:	
	@rm -f *.bak *.s nroff
