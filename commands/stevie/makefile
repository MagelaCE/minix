#
# Makefile for PC Minix
#

LDFLAGS= -i -T/usr/tmp
CFLAGS=  -DMINIX  -O


MACH=   minix.s

OBJ=    main.s edit.s linefunc.s normal.s cmdline.s hexchars.s \
        misccmds.s help.s ptrfunc.s search.s alloc.s mark.s \
        regexp.s regsub.s \
        screen.s fileio.s param.s undo.s version.s



all : stevie

stevie : $(OBJ) $(MACH)
	$(CC) $(LDFLAGS) $(OBJ) $(MACH) -o stevie
	@echo  stevie fertig

ctags:	ctags.c
	$(CC) -o ctags ctags.c
	chmem =4096 ctags
clean :
	rm $(OBJ) $(MACH)
