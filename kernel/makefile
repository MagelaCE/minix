# Say 'make at' to make a kernel with the AT winchester driver.
# Say 'make xt' to make a kernel with the XT winchester driver.
# Say 'make' to make the default kernel.
# Use the at kernel if you don't have a hard disk.

# Uncomment one of these 4 groups of defines for special kernels.
# Protected mode may be defined for 8088 through 80386 processors, but only
# 286's and 386's take advantage of it.
# The posted version doesn't support DEBUGanything, PM3, or anything386.

# Protected mode.
 CPU		= -Di8088 -Di80286 # -Di80386
 SPECOBJS	= $(PM2OBJS) # $(PM3OBJS)

# Debugger and protected mode.
# DEBUG_EARLY	= -DDEBUG_EARLY
# DEBUGGER	= -DDEBUGGER
# CPU		= -Di8088 -Di80286 # -Di80386
# SPECOBJS	= $(DBOBJS) $(DB_PM2OBJS) $(PM2OBJS) #$(DB_PM3OBJS) $(PM3OBJS)

# Debugger.
# DEBUG_EARLY	= -DDEBUG_EARLY
# DEBUGGER	= -DDEBUGGER
# CPU		= i8088
# SPECOBJS	= $(DBOBJS)

# Ordinary kernel (1.3 with a few improvements).
# CPU		= -Di8088

# Futher, uncomment one of these 3 groups of defines to select the TTY driver.

# 1.3 TTY driver.
 C_RS232	= -DC_RS232_INT_HANDLERS
 OLD_TTY	= -DOLD_TTY

# Bruce's TTY driver.
# RS2 		= rs2.o

# Bruce's TTY driver, but use the C interrupt handlers.
# C_RS232	= -DC_RS232_INT_HANDLERS

# Further, uncomment one of these 2 groups of defines to select the
# environment.

# Standard Minix.
# CFLAGS include the following nonstandard ones.
# -F:	run cpp and cem sequentially (used when memory is tight)
# -T.:	put temporaries in working directory (when RAM disk is small)
 ASLD		= -DASLD
 BIN		= .
 CC		= cc
 CFLAGS		= $(CPPFLAGS) -F -T.
 END		= /usr/lib/end.s
 LD		= asld
 LIBS		= /usr/lib/libc.a
 o		= s

# Bruce's Minix.
# BIN		= /etc/system
# CC		= ecc
# CFLAGS		= $(CPPFLAGS)
# LD		= ld
# LIBS		= /local/lib/libc.a
# o		= o
# .s.o:
#	$(CC) -c $*.s
# VOID		= -Dvoid=int

# Other defines for C.
# AM_KERNEL	= -DAM_KERNEL	# Amoeba kernel (doesn't work)
 AUTO_BIOS	= -DAUTO_BIOS=0	# for xt_wini.c, make it =1 if wanted
# LINEWRAP	= -DLINEWRAP	# wrap lines at column 80
# NO_HANDSHAKE	= -DNO_HANDSHAKE	# no hardware handshake for RS232
# SPARE_VIDEO	= -DSPARE_VIDEO_MEMORY	# put mem from any 2nd vid card in map
 STUPID_WINI	= -DSTUPID_WINI_ADJUST	# skip odd 1st sector of partition

# Other defines for make.
CPP		= /lib/cpp
CPPFLAGS	= $(ASLD) $(AUTO_BIOS) $(AM_KERNEL) $(CPU) $(C_RS232) \
		  $(DEBUG_EARLY) $(DEBUGGER) $(LINEWRAP) $(NO_HANDSHAKE) \
		  $(OLD_TTY) $(SPARE_VIDEO) $(STUPID_WINI) $(VOID)
LDFLAGS		= -i
START		= mpx88.$o
h		= ../h

DB_PM2OBJS	= db286.$o
DB_PM3OBJS	= db386.$o
DBOBJS		= db/int88.$o db/db.$o db/getline.$o db/io.$o db/keyboard.$o \
		  db/lib88.$o db/pcio.$o db/sym.$o db/unasm.$o
KEROBJS		= main.$o tty.$o floppy.$o system.$o proc.$o clock.$o \
		  memory.$o console.$o rs232.$o $(RS2) printer.$o table.$o \
		  klib88.$o dmp.$o \
		  exception.$o i8259.$o misc.$o
PM2OBJS		= mpx286.$o klib286.$o protect.$o 
PM3OBJS		= mpx386.$o klib386.$o
SOBJS		= db286.s db386.s klib286.s klib386.s klib88.s \
		  mpx286.s mpx386.s mpx88.s rs2.s

ATWINI		= at_wini.$o
XTWINI		= xt_wini.$o

# Main thing to make.  Swap the order of these if you prefer and XT kernel.

at: $(START) $(KEROBJS) $(SPECOBJS) $(ATWINI) $(LIBS) $(END)
	$(LD) $(LDFLAGS) -o $(BIN)/atkernel \
    $(START) $(SPECOBJS) $(KEROBJS) $(ATWINI) $(LIBS) $(END) \
#   -s > atkernel.sym
#	ast -X $(BIN)/atkernel atkernel.sym
	rm -f $(BIN)/kernel
	ln $(BIN)/atkernel $(BIN)/kernel

xt: $(START) $(KEROBJS) $(SPECOBJS) $(XTWINI) $(LIBS) $(END)
	$(LD) $(LDFLAGS) -o $(BIN)/xtkernel \
    $(START) $(KEROBJS) $(SPECOBJS) $(XTWINI) $(LIBS) $(END) \
#   -s > xtkernel.sym
#	ast -X $(BIN)/xtkernel xtkernel.sym
	rm -f $(BIN)/kernel
	ln $(BIN)/xtkernel $(BIN)/kernel

clean:
	rm -f $(START) $(DB_PM2OBJS) $(DB_PM3OBJS) $(KEROBJS) \
	      $(PM2OBJS) $(PM3OBJS) $(SOBJS) \
	      $(ATWINI) $(XTWINI

db286.o: db286.s
	$(CC) -c $*.s
db286.s: $h/const.h const.h protect.h sconst.h sglo.h db286.x
	$(CPP) $(CPPFLAGS) $*.x | grep -v '^\#' > $*.s

db386.o: db386.s
	$(CC) -c $*.s
db386.s: $h/const.h const.h protect.h sconst.h sglo.h db386.x
	$(CPP) $(CPPFLAGS) $*.x | grep -v '^\#' > $*.s

klib286.o: klib286.s
	$(CC) -c $*.s
klib286.s: $h/const.h const.h protect.h sconst.h sglo.h klib286.x
	$(CPP) $(CPPFLAGS) $*.x | grep -v '^\#' > $*.s

klib386.o: klib386.s
	$(CC) -c $*.s
klib386.s: $h/const.h const.h protect.h sconst.h sglo.h klib386.x
	$(CPP) $(CPPFLAGS) $*.x | grep -v '^\#' > $*.s

klib88.o: klib88.s
	$(CC) -c $*.s
klib88.s: $h/const.h const.h sconst.h sglo.h klib88.x
	$(CPP) $(CPPFLAGS) $*.x | grep -v '^\#' > $*.s

mpx286.o: mpx286.s
	$(CC) -c $*.s
mpx286.s: const.h protect.h sconst.h sglo.h mpx286.x
	$(CPP) $(CPPFLAGS) $*.x | grep -v '^\#' > $*.s

mpx386.o: mpx386.s
	$(CC) -c $*.s
mpx386.s: const.h protect.h sconst.h sglo.h mpx386.x
	$(CPP) $(CPPFLAGS) $*.x | grep -v '^\#' > $*.s

mpx88.o: mpx88.s
	$(CC) -c $*.s
mpx88.s: $h/const.h $h/com.h const.h sconst.h sglo.h mpx88.x
	$(CPP) $(CPPFLAGS) $*.x | grep -v '^\#' > $*.s

rs2.o: rs2.s
	$(CC) -c $*.s
rs2.s: $h/const.h const.h sconst.h sglo.h rs2.x
	$(CPP) $(CPPFLAGS) $*.x | grep -v '^\#' > $*.s

clock.$o:	const.h type.h $h/const.h $h/type.h
clock.$o:	$h/callnr.h
clock.$o:	$h/com.h
clock.$o:	$h/error.h
clock.$o:	$h/signal.h
clock.$o:	glo.h
clock.$o:	proc.h

console.$o:	const.h type.h $h/const.h $h/type.h
console.$o:	$h/callnr.h
console.$o:	$h/com.h
console.$o:	$h/error.h
console.$o:	$h/sgtty.h
console.$o:	$h/signal.h
console.$o:	glo.h
console.$o:	proc.h
console.$o:	protect.h
console.$o:	tty.h

floppy.$o:	const.h type.h $h/const.h $h/type.h
floppy.$o:	$h/callnr.h
floppy.$o:	$h/com.h
floppy.$o:	$h/error.h
floppy.$o:	glo.h
floppy.$o:	proc.h

dmp.$o:		const.h type.h $h/const.h $h/type.h
dmp.$o:		$h/callnr.h
dmp.$o:		$h/com.h
dmp.$o:		$h/error.h
dmp.$o:		glo.h
dmp.$o:		proc.h

main.$o:	const.h type.h $h/const.h $h/type.h
main.$o:	$h/boot.h
main.$o:	$h/callnr.h
main.$o:	$h/com.h
main.$o:	$h/error.h
main.$o:	$h/signal.h
main.$o:	glo.h
main.$o:	proc.h

memory.$o:	const.h type.h $h/const.h $h/type.h
memory.$o:	$h/callnr.h
memory.$o:	$h/com.h
memory.$o:	$h/error.h
memory.$o:	glo.h
memory.$o:	proc.h

printer.$o:	const.h type.h $h/const.h $h/type.h
printer.$o:	$h/callnr.h
printer.$o:	$h/com.h
printer.$o:	$h/error.h
printer.$o:	proc.h
printer.$o:	glo.h

proc.$o:	const.h type.h $h/const.h $h/type.h
proc.$o:	$h/callnr.h
proc.$o:	$h/com.h
proc.$o:	$h/error.h
proc.$o:	glo.h
proc.$o:	proc.h

rs232.$o:	const.h $h/const.h $h/type.h
rs232.$o:	$h/sgtty.h
rs232.$o:	tty.h

system.$o:	const.h type.h $h/const.h $h/type.h
system.$o:	$h/boot.h
system.$o:	$h/callnr.h
system.$o:	$h/com.h
system.$o:	$h/error.h
system.$o:	$h/signal.h
system.$o:	glo.h
system.$o:	proc.h
system.$o:	protect.h

table.$o:	const.h type.h $h/const.h $h/type.h $h/com.h
table.$o:	glo.h
table.$o:	proc.h
table.$o:	tty.h

tty.$o:		const.h type.h $h/const.h $h/type.h
tty.$o:		$h/callnr.h
tty.$o:		$h/com.h
tty.$o:		$h/error.h
tty.$o:		$h/sgtty.h
tty.$o:		$h/signal.h
tty.$o:		glo.h
tty.$o:		proc.h
tty.$o:		tty.h
tty.$o:		ttymaps.h

at_wini.$o:	const.h type.h $h/const.h $h/type.h
at_wini.$o:	$h/callnr.h
at_wini.$o:	$h/com.h
at_wini.$o:	$h/error.h
at_wini.$o:	proc.h

xt_wini.$o:	const.h type.h $h/const.h $h/type.h
xt_wini.$o:	$h/callnr.h
xt_wini.$o:	$h/com.h
xt_wini.$o:	$h/error.h
xt_wini.$o:	proc.h

exception.$o:	$h/const.h
exception.$o:	$h/type.h
exception.$o:	$h/signal.h
exception.$o:	const.h
exception.$o:	type.h
exception.$o:	glo.h
exception.$o:	proc.h

i8259.$o:	$h/const.h
i8259.$o:	$h/type.h
i8259.$o:	$h/com.h
i8259.$o:	const.h
i8259.$o:	type.h
i8259.$o:	glo.h
i8259.$o:	proc.h

misc.$o:	$h/const.h
misc.$o:	$h/type.h
misc.$o:	const.h
misc.$o:	type.h
misc.$o:	glo.h

protect.$o:	$h/const.h
protect.$o:	$h/type.h
protect.$o:	const.h
protect.$o:	type.h
protect.$o:	glo.h
protect.$o:	proc.h
protect.$o:	protect.h
