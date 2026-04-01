#makefile for the don kneller's shareware ndmake

OBJS=   sedcomp.s sedexec.s

sed: 	$(OBJS) sed.h debug.h
        cc -o sed -i  $(OBJS)

sedcomp.s: debug.h sed.h sedcomp.c 
sedexec.s: debug.h sed.h sedexec.c 
