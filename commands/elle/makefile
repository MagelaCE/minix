Path: tut!enea!mcvax!botter!ast
From: ast@cs.vu.nl (Andy Tanenbaum)
Newsgroups: comp.os.minix
Subject: Makefile for ELLE that I used on PC-IX
Message-ID: <1799@botter.cs.vu.nl>
Date: 20 Dec 87 14:34:03 GMT
Reply-To: ast@cs.vu.nl (Andy Tanenbaum)
Organization: VU Informatica, Amsterdam
Lines: 37


For those of you who want to fetch the ELLE sources and work on them, be
warned that the MINIX make program can't handle the way Ken did things.
In particular, when you invoke make, the makefile makes another makefile
that is then executed.  The MINIX make first parses the entire makefile,
and dies because it encounters macros that will only be defined when the
other makefile is executed.  Since the semantics of makefiles are defined
nowhere, it is hard to tell if this is a bug, a feature or what.  In any
case, I just made up my own little makefile for use on PC-IX.  Here it is.

Andy Tanenbaum (ast@cs.vu.nl)

---------------------- ELLE Makefile for PC-IX ----------------------
l=/usr/ast/minix/lib
CFLAGS = -I/usr/ast/minix/include 
CONFS = defprf.c eefdef.h eefidx.h deffun.e

OBJ= eebit.o  eebuff.o eecmds.o eedisp.o eeedit.o eeerr.o eef1.o eef2.o \
   eef3.o eefd.o eefed.o eefile.o eehelp.o eekmac.o eemain.o \
   eequer.o eeques.o eesite.o eesrch.o eeterm.o eevini.o sbbcpy.o \
   sberr.o sbm.o sbstr.o termcap.o eediag.o eefill.o  minix.o

elle:	$(OBJ) elle.h eesite.h
	ld -i -s -o xelle $l/crtso.o $(OBJ) $l/libc.a $l/end.o

conf:
	cat deffun.e defprf.e | ellec -Pconf  > defprf.c
	cat deffun.e defprf.e | ellec -Fconf  > eefdef.h
	cat deffun.e defprf.e | ellec -FXconf > eefidx.h
	rm -f eecmds.o

ellec: ellec.c eesite.h defprf.e
	cc -o ellec ellec.c

eecmds.o: eecmds.c ellec $(CONFS)
	make conf
	cc -c eecmds.c
