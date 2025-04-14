CFLAGS	= -Di8088 -T. -F
DEOBJ   = de.s de_stdin.s de_stdout.s de_diskio.s de_recover.s

de:	$(DEOBJ)
	cc -i -T. $(DEOBJ) -o de
	chmem =20000 de
	chmod  4755  de
	chown  root  de
	chgrp  bin   de

$(DEOBJ): de.h
