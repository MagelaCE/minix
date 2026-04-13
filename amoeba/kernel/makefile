# The kernel dir contains xt_wini.c and at_wini.c.  Before running make
# you must copy one of these to wini.c, depending on which controller you
# have.  If you do not have a hard disk, you MUST choose one of them at random.
# On a PC, cpp and cem are in /lib and will be removed to make space while
# linking the kernel.  On an AT, they are in /usr/lib are are not removed.
# This is because they have to be in /lib on a PC; the diskette is too small
# for them to be in /usr/lib.  You can change this by editing commands/cc.c.
#
# Normally, MINIX scrolls the screen using the 6845's registers.  However,
# on some EGA cards (those that are not IBM compatible), the 6845 is not
# properly emulated.  On these machines, it is necessary to scroll in
# software by copying. This is much slower, but it works. The CFLAGS flags are:
#
#  -Di8088	- required on an 8088/80286/80386 CPU; forbidden on a 68000
#  -F		- run cpp and cem sequentially (used when memory is tight)
#  -T.		- put temporaries in working directory (when RAM disk is small)
#  -DAM_KERNEL	- includes code for Amoeba transactions
#  -DNONET	- single machine, no Ethernet available (requires -DAM_KERNEL)
#  -DSTATISTICS	- include code to collect Ethernet statistics
#
# Machines wishing to use Amoeba transactions should add -DAM_KERNEL to CFLAGS.
# If there is no ethernet connection then also add -DNONET.  The ethernet
# driver included in this distribution is for the Western Digital Ethercard
# Plus.  If you want ethernet statistics then add -DSTATISTICS to CFLAGS.
#
# When making a kernel on a machine with limited RAM disk, it may be 
# necessary to use -T. or even remove /lib/cem and /lib/cpp during asld.
#
CFLAGS	= -DAM_KERNEL -Di8088   -F -I../../kernel -I. 
A	= ../h
H	= ../../h
K	= ../../kernel
LIB	= /usr/lib

OBJ =	../../kernel/mpx88.s main.s tty.s floppy.s wini.s system.s proc.s \
	console.s rs232.s clock.s memory.s printer.s amoeba.s conf.s dp8390.s \
	util.s etherplus.s portcache.s trans.s table.s ../../kernel/klib88.s \
	dmp.s


COBJS =	main.s tty.s floppy.s wini.s system.s proc.s clock.s memory.s \
	amoeba.s conf.s dp8390.s util.s etherplus.s portcache.s trans.s \
	console.s rs232.s printer.s table.s dmp.s


kernel:	makefile $(OBJ) $(LIB)/libc.a
	@echo "Start linking Kernel."
	@asld -o kernel $(OBJ) $(LIB)/libc.a $(LIB)/end.s
	@echo "Kernel done.  "

clean:
	rm -f $(COBJS) kernel

clock.s:	$K/const.h $K/type.h $H/const.h $H/type.h
clock.s:	$H/callnr.h
clock.s:	$H/com.h
clock.s:	$H/error.h
clock.s:	$H/signal.h
clock.s:	$K/glo.h
clock.s:	$K/proc.h
clock.s:	$K/clock.c
	$(CC) $(CFLAGS) -c $K/clock.c

console.s:	$K/const.h $K/type.h $H/const.h $H/type.h
console.s:	$H/callnr.h
console.s:	$H/com.h
console.s:	$H/error.h
console.s:	$H/sgtty.h
console.s:	$H/signal.h
console.s:	$K/glo.h
console.s:	$K/proc.h
console.s:	$K/tty.h
console.s:	$K/tty.c
	$(CC) $(CFLAGS) -c $K/console.c

dmp.s:		$K/const.h $K/type.h $H/const.h $H/type.h
dmp.s:		$H/callnr.h
dmp.s:		$H/com.h
dmp.s:		$H/error.h
dmp.s:		$K/glo.h
dmp.s:		$K/proc.h
dmp.s:		$K/dmp.c
	$(CC) $(CFLAGS) -c $K/dmp.c

floppy.s:	$K/const.h $K/type.h $H/const.h $H/type.h
floppy.s:	$H/callnr.h
floppy.s:	$H/com.h
floppy.s:	$H/error.h
floppy.s:	$K/glo.h
floppy.s:	$K/proc.h
floppy.s:	$K/floppy.c
	$(CC) $(CFLAGS) -c $K/floppy.c

main.s:		$K/const.h $K/type.h $H/const.h $H/type.h
main.s:		$H/callnr.h
main.s:		$H/com.h
main.s:		$H/error.h
main.s:		$K/glo.h
main.s:		$K/proc.h
main.s:		$K/main.c
	$(CC) $(CFLAGS) -c $K/main.c

memory.s:	$K/const.h $K/type.h $H/const.h $H/type.h
memory.s:	$H/callnr.h
memory.s:	$H/com.h
memory.s:	$H/error.h
memory.s:	$K/proc.h
memory.s:	$K/memory.c
	$(CC) $(CFLAGS) -c $K/memory.c

printer.s:	$K/const.h $K/type.h $H/const.h $H/type.h
printer.s:	$H/callnr.h
printer.s:	$H/com.h
printer.s:	$H/error.h
printer.s:	$K/proc.h
printer.s:	$K/glo.h
printer.s:	$K/printer.c
	$(CC) $(CFLAGS) -c $K/printer.c

proc.s:		$K/const.h $K/type.h $H/const.h $H/type.h
proc.s:		$H/callnr.h
proc.s:		$H/com.h
proc.s:		$H/error.h
proc.s:		$K/glo.h
proc.s:		$K/proc.h
proc.s:		$K/proc.c
	$(CC) $(CFLAGS) -c $K/proc.c

rs232.s:	$K/const.h $K/type.h $H/const.h $H/type.h
rs232.s:	$H/callnr.h
rs232.s:	$H/com.h
rs232.s:	$H/error.h
rs232.s:	$H/sgtty.h
rs232.s:	$H/signal.h
rs232.s:	$K/glo.h
rs232.s:	$K/proc.h
rs232.s:	$K/rs232.c
	$(CC) $(CFLAGS) -c $K/rs232.c

system.s:	$K/const.h $K/type.h $H/const.h $H/type.h
system.s:	$H/callnr.h
system.s:	$H/com.h
system.s:	$H/error.h
system.s:	$H/signal.h
system.s:	$K/glo.h
system.s:	$K/proc.h
system.s:	$K/system.c
	$(CC) $(CFLAGS) -c $K/system.c

table.s:	$K/const.h $K/type.h $H/const.h $H/type.h $H/com.h
table.s:	$K/glo.h
table.s:	$K/proc.h
table.s:	$K/table.c
	$(CC) $(CFLAGS) -c $K/table.c

tty.s:	$K/const.h $K/type.h $H/const.h $H/type.h
tty.s:	$H/callnr.h
tty.s:	$H/com.h
tty.s:	$H/error.h
tty.s:	$H/sgtty.h
tty.s:	$H/signal.h
tty.s:	$K/glo.h
tty.s:	$K/proc.h
tty.s:	$K/tty.c
tty.s:	$K/ttymaps.h
	$(CC) $(CFLAGS) -c $K/tty.c

wini.s:	$K/const.h $K/type.h $H/const.h $H/type.h
wini.s:	$H/callnr.h
wini.s:	$H/com.h
wini.s:	$H/error.h
wini.s:	$K/proc.h
wini.s:	$K/wini.c
	$(CC) $(CFLAGS) -c $K/wini.c

amoeba.s: $A/amoeba.h
amoeba.s: $A/host_os.h
amoeba.s: $A/amparam.h
amoeba.s: $H/signal.h
amoeba.s: $H/type.h
amoeba.s: ./assert.h
amoeba.s: ./byteorder.h
amoeba.s: $K/const.h
amoeba.s: ./dp8390info.h
amoeba.s: ./etherformat.h
amoeba.s: $K/glo.h
amoeba.s: ./global.h
amoeba.s: ./internet.h
amoeba.s: ./mpx.H
amoeba.s: ./portcache.H
amoeba.s: $K/proc.h
amoeba.s: ./task.h
amoeba.s: ./trans.H
amoeba.s: $K/type.h
amoeba.s: amoeba.c
conf.s: $A/amoeba.h
conf.s: $A/host_os.h
conf.s: $H/type.h
conf.s: ./conf.h
conf.s: ./global.h
conf.s: ./mpx.H
conf.s: ./portcache.H
conf.s: ./task.h
conf.s: ./trans.H
conf.s: conf.c
dp8390.s: $H/com.h
dp8390.s: $H/const.h
dp8390.s: $H/error.h
dp8390.s: $H/type.h
dp8390.s: ./assert.h
dp8390.s: $K/const.h
dp8390.s: ./dp8390.h
dp8390.s: ./dp8390info.h
dp8390.s: ./dp8390stat.h
dp8390.s: ./etherformat.h
dp8390.s: ./internet.h
dp8390.s: dp8390.c
etherplus.s: $H/com.h
etherplus.s: $H/const.h
etherplus.s: $H/type.h
etherplus.s: ./assert.h
etherplus.s: $K/const.h
etherplus.s: ./dp8390.h
etherplus.s: ./dp8390info.h
etherplus.s: ./eplinfo.h
etherplus.s: ./etherformat.h
etherplus.s: ./etherplus.h
etherplus.s: ./internet.h
etherplus.s: etherplus.c
portcache.s: $A/amoeba.h
portcache.s: $H/const.h
portcache.s: $A/host_os.h
portcache.s: $H/type.h
portcache.s: ./assert.h
portcache.s: ./conf.h
portcache.s: $K/const.h
portcache.s: ./global.h
portcache.s: ./internet.h
portcache.s: ./mpx.H
portcache.s: ./portcache.H
portcache.s: ./task.h
portcache.s: ./trans.H
portcache.s: portcache.c
trans.s: $A/amoeba.h
trans.s: $H/const.h
trans.s: $A/host_os.h
trans.s: $H/type.h
trans.s: ./amstat.h
trans.s: ./assert.h
trans.s: ./byteorder.h
trans.s: $K/const.h
trans.s: ./exception.h
trans.s: ./global.h
trans.s: ./internet.h
trans.s: ./mpx.H
trans.s: ./portcache.H
trans.s: ./task.h
trans.s: ./trans.H
trans.s: trans.c
util.s: $H/com.h
util.s: $H/const.h
util.s: $H/type.h
util.s: ./assert.h
util.s: $K/const.h
util.s: ./dp8390.h
util.s: ./dp8390info.h
util.s: ./eplinfo.h
util.s: $K/proc.h
util.s: $K/type.h
util.s: util.c

