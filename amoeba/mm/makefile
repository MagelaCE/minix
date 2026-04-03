# This makefile is used for both the PC and AT.  If you run out of space
# while compiling or linking, you may have to use the -T. flag, or even
# rm /lib/cem and /lib/cpp while running asld.  See the makefiles in
# kernel/IBM_PC.
#
# If you do not intend to use the amoeba networking then remove the -DAM_KERNEL
# from the CFLAGS. Do NOT change anything else in the makefile.  It is
# unnecessary.

H=../../h
M=../../mm
A=../h
L=/usr/lib
CFLAGS = -DAM_KERNEL -Di8088 -w -F -I../../mm

OBJ =	main.s forkexit.s break.s exec.s signal.s getset.s  \
	alloc.s utility.s table.s putc.s amoeba.s

mm:	makefile    $L/head.s $(OBJ) $L/libc.a $L/end.s
	@echo "Start linking MM."
	@asld -o mm $L/head.s $(OBJ) $L/libc.a $L/end.s
	@echo "MM done. "

clean:
	rm -f $(OBJ)

alloc.s:	$M/const.h $H/const.h $H/type.h
	$(CC) $(CFLAGS) -c $M/alloc.c

amoeba.s:	$H/const.h $H/com.h $H/type.h $H/error.h \
		$A/amoeba.h $A/host_os.h \
		$M/const.h $M/glo.h
	$(CC) $(CFLAGS) -c amoeba.c

break.s:	$M/const.h $H/const.h $H/type.h
break.s:	$H/error.h
break.s:	$H/signal.h
break.s:	$M/glo.h
break.s:	$M/mproc.h
break.s:	$M/param.h
	$(CC) $(CFLAGS) -c $M/break.c

exec.s:		$M/const.h $H/const.h $H/type.h
exec.s:		$H/callnr.h
exec.s:		$H/error.h
exec.s:		$H/stat.h
exec.s:		$M/glo.h
exec.s:		$M/mproc.h
exec.s:		$M/param.h
	$(CC) $(CFLAGS) -c $M/exec.c

forkexit.s:	$M/const.h $H/const.h $H/type.h
forkexit.s:	$H/callnr.h
forkexit.s:	$H/error.h
forkexit.s:	$M/glo.h
forkexit.s:	$M/mproc.h
forkexit.s:	$M/param.h
	$(CC) $(CFLAGS) -c $M/forkexit.c

getset.s:	$M/const.h $H/const.h $H/type.h
getset.s:	$H/callnr.h
getset.s:	$H/error.h
getset.s:	$M/glo.h
getset.s:	$M/mproc.h
getset.s:	$M/param.h
	$(CC) $(CFLAGS) -c $M/getset.c

main.s:		$M/const.h $H/const.h $H/type.h
main.s:		$H/callnr.h
main.s:		$H/com.h
main.s:		$H/error.h
main.s:		$M/glo.h
main.s:		$M/mproc.h
main.s:		$M/param.h
	$(CC) $(CFLAGS) -c $M/main.c

putc.s:		$H/const.h $H/type.h
putc.s:		$H/com.h
	$(CC) $(CFLAGS) -c $M/putc.c

signal.s:	$M/const.h $H/const.h $H/type.h
signal.s:	$H/callnr.h
signal.s:	$H/com.h
signal.s:	$H/error.h
signal.s:	$H/signal.h
signal.s:	$H/stat.h
signal.s:	$M/glo.h
signal.s:	$M/mproc.h
signal.s:	$M/param.h
	$(CC) $(CFLAGS) -c $M/signal.c

table.s:	$M/const.h $H/const.h $H/type.h
table.s:	$H/callnr.h
table.s:	$M/glo.h
table.s:	$M/mproc.h
table.s:	$M/param.h
	$(CC) $(CFLAGS) -c $M/table.c

utility.s:	$M/const.h $H/const.h $H/type.h
utility.s:	$H/callnr.h
utility.s:	$H/com.h
utility.s:	$H/error.h
utility.s:	$H/stat.h
utility.s:	$M/glo.h
utility.s:	$M/mproc.h
	$(CC) $(CFLAGS) -c $M/utility.c
