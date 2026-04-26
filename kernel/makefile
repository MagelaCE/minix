# Makefile for kernel using standard Minix directory layout and compiler.
# CPP may need changing to /usr/lib/cpp.
# The following nonstandard flags are used:
# -F:	run cpp and cem sequentially (used when memory is tight)
# -T.:	put temporaries in working directory (when RAM disk is small)
BIN		=.
CC		=cc
CFLAGS		=-F -T.
CPP		=/lib/cpp
CPPFLAGS	=-DASLD -P
END		=/lib/end.s
LD		=asld
LDFLAGS		=-i
LIBS		=/lib/libc.a
START		=start.s

a		=$i/errno.h $i/limits.h $s/types.h \
		 $h/config.h $h/const.h $h/type.h const.h type.h
h		=/usr/include/minix
i		=/usr/include
s		=/usr/include/sys

# Delete next line when C_RS232_INT_HANDLERS is defined in config.h.
RS2		=rs2.s

KEROBJS		=cstart.s protect.s protect1.s klib.s mpx.s wini.s \
	table.s main.s tty.s floppy.s system.s proc.s \
	clock.s memory.s console.s rs232.s $(RS2) printer.s \
	dmp.s exception.s i8259.s misc.s keyboard.s
SOBJS		=start.s klib.s mpx.s rs2.s

# Rules.
.SUFFIXES: .x		# .x files are .s files which need C-preprocessing
.x.s:
	$(CPP) $(CPPFLAGS) $< >$@

# What to make.
dummy: $(BIN)/kernel	# this line fixes a bug in RAL's latest make

$(BIN)/kernel: $(START) $(KEROBJS) $(LIBS) $(END)
	$(LD) $(LDFLAGS) -o $(BIN)/kernel \
	$(START) $(KEROBJS) \
	$(LIBS) $(END) -s > kernel.sym
	ast -X $(BIN)/kernel kernel.sym
	rm kernel.sym

clean:
	rm -f $(START) $(KEROBJS) $(SOBJS)

klib.s: $h/const.h const.h protect.h sconst.h klib.x
mpx.s: $h/const.h $h/com.h const.h protect.h sconst.h mpx.x
rs2.s: $h/const.h const.h sconst.h rs2.x
start.s: $h/const.h $h/com.h const.h protect.h sconst.h start.x

clock.s:	$i/signal.h
clock.s:	$a
clock.s:	$h/callnr.h
clock.s:	$h/com.h
clock.s:	glo.h
clock.s:	proc.h

console.s:	$i/sgtty.h
console.s:	$a
console.s:	$h/callnr.h
console.s:	$h/com.h
console.s:	glo.h
console.s:	protect.h
console.s:	tty.h

floppy.s:	$a
floppy.s:	$h/callnr.h
floppy.s:	$h/com.h
floppy.s:	glo.h

dmp.s:		$a
dmp.s:		$h/callnr.h
dmp.s:		$h/com.h
dmp.s:		glo.h
dmp.s:		proc.h

main.s:	$i/signal.h
main.s:	$a
main.s:	$h/callnr.h
main.s:	$h/com.h
main.s:	glo.h
main.s:	proc.h

memory.s:	$a
memory.s:	$h/callnr.h
memory.s:	$h/com.h
memory.s:	glo.h

printer.s:	$a
printer.s:	$h/callnr.h
printer.s:	$h/com.h
printer.s:	glo.h

proc.s:	$a
proc.s:	$h/callnr.h
proc.s:	$h/com.h
proc.s:	glo.h
proc.s:	proc.h

rs232.s:	$i/sgtty.h
rs232.s:	$a
rs232.s:	glo.h
rs232.s:	tty.h
rs232.s:	type.h

system.s:	$i/signal.h
system.s:	$a
system.s:	$h/boot.h
system.s:	$h/callnr.h
system.s:	$h/com.h
system.s:	glo.h
system.s:	proc.h
system.s:	protect.h

table.s:	$a
table.s:	$h/com.h
table.s:	glo.h
table.s:	proc.h
table.s:	tty.h

tty.s:		$i/sgtty.h
tty.s:		$i/signal.h
tty.s:		$a
tty.s:		$h/callnr.h
tty.s:		$h/com.h
tty.s:		glo.h
tty.s:		tty.h

wini.s:	$a
wini.s:	$h/callnr.h
wini.s:	$h/com.h
wini.s:	$h/partition.h
wini.s:	glo.h

cstart.s:	$a
cstart.s:	$h/boot.h
cstart.s:	glo.h
cstart.s:	cstart.c

exception.s:	$i/signal.h
exception.s:	$a
exception.s:	glo.h
exception.s:	proc.h

i8259.s:	$a
i8259.s:	glo.h

keyboard.s:	$i/sgtty.h
keyboard.s:	$i/signal.h
keyboard.s:	$a
keyboard.s:	$h/callnr.h
keyboard.s:	$h/com.h
keyboard.s:	glo.h
keyboard.s:	tty.h

misc.s:	$a
misc.s:	$h/com.h
misc.s:	glo.h
misc.s:	proc.h

protect.s:	$a
protect.s:	glo.h
protect.s:	protect.h
protect.s:	protect1.c

protect1.s:	$a
protect1.s:	glo.h
protect1.s:	proc.h
protect1.s:	protect.h
