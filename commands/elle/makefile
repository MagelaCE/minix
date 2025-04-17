l=/usr/bin/lib
CFLAGS =  
CONFS = defprf.c eefdef.h eefidx.h deffun.e

OBJ= eebit.s  eebuff.s eecmds.s eedisp.s eeedit.s eeerr.s eef1.s eef2.s \
   eef3.s eefd.s eefed.s eefile.s eehelp.s eekmac.s eemain.s \
   eequer.s eeques.s eesite.s eesrch.s eeterm.s eevini.s sbbcpy.s \
   sberr.s sbm.s sbstr.s termcap.s eediag.s eefill.s  minix.s

elle:	$(OBJ) elle.h eesite.h
	asld -i -o elle $l/crtso.s $(OBJ) $l/libc.a $l/end.s

conf:
	cat deffun.e defprf.e | ellec -Pconf  > defprf.c
	cat deffun.e defprf.e | ellec -Fconf  > eefdef.h
	cat deffun.e defprf.e | ellec -FXconf > eefidx.h
	rm -f eecmds.s

ellec: ellec.c eesite.h defprf.e
	cc -o ellec ellec.c

eecmds.o: eecmds.c ellec $(CONFS)
	make conf
	cc -c eecmds.c
