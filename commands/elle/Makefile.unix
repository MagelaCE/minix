# ELLE Makefile.
#	The main trickery to notice is that this Makefile invokes itself
# in order to build ELLE.  This allows it to generate sections of
# the makefile dynamically, namely:
#	makecf.rl  - defines the RANLIB macro to use "ranlib" if system has it
#	makecf.fun - defines the FUN_OFILES macro to specify which modules
#			are needed in order to furnish all of the desired
#			functions for a specific ELLE configuration.

# Basic definitions

CFLAGS = -c -O

CONFS = makecf.rl makecf.fun defprf.c eefdef.h eefidx.h

CORE_OFILES = eemain.o eecmds.o eesite.o eevini.o\
	eedisp.o eeterm.o eeerr.o eeques.o\
	eebuff.o eefile.o eefed.o eeedit.o eebit.o

.c.o:
	cc $(CFLAGS) $*.c
	ar rv elib.a $*.o

# ------------------------------------------------------------------
#	Standard ELLE configuration: "elle".
#	Default if no target given to make.
#	Note that "xelle" is created, not "elle".

elle: $(CONFS)
	make -f makecf.rl -f makecf.fun -f Makefile B-elle

B-elle: $(CORE_OFILES) $(FUN_OFILES) elle.h eesite.h libsb.a
	$(RANLIB) elib.a
	cc -i -o xelle -u _main elib.a libsb.a -ltermlib


# Don't flush these files if interrupted, dammit!

.PRECIOUS: ellec deffun.e defprf.e $(CONFS) libsb.a

# Configuration setup stuff, for files listed by CONFS.
#	If any of the files is missing or outdated, all must be
#	re-generated.  We assume the *.t files always exist.
#	A forced re-compilation of eecmds.c must also be done to
#	ensure that the latest .h files are included in ELLE.

makecf.fun defprf.c eefdef.h eefidx.h : ellec defprf.e deffun.e
	cat deffun.e defprf.e | ellec -CMconf > makecf.fun
	cat deffun.e defprf.e | ellec -Pconf  > defprf.c
	cat deffun.e defprf.e | ellec -Fconf  > eefdef.h
	cat deffun.e defprf.e | ellec -FXconf > eefidx.h
	rm -f eecmds.o

# The following modules make use of eefidx.h and thus must also
# be recompiled if the configuration is changed.

eebuff.o eeerr.o eehelp.o eejust.o eemain.o eeques.o eef1.o : eefidx.h
	cc $(CFLAGS) $*.c
	ar rv elib.a $*.o


# RANLIB definition stuff.  The idea here is to automatically check to see
#	whether a system has the "ranlib" program, and set up the RANLIB
#	macro appropriately (use ranlib if possible; else substitute a dummy
#	program, "echo" in this case).  The existence of "makecf.rl"
#	indicates that this setup code has been executed; it should never
#	need to be done again.
makecf.rl:
	echo "RANLIB = echo" > makecf.rl
	-sh -c "if ranlib; then (echo \"RANLIB = ranlib\" > makecf.rl;) fi"

# ELLE profile compiler.  Needed to generate makecf files!
#	Although eefdef.h and defprf.c are included by ELLEC, they
#	are not listed as dependencies in order to avoid loops (see
#	their target entries).  That is OK because their information is not
#	used when generating the makecf files; it only furnishes default
#	values needed when an ELLE user compiles a user profile.

ellec: ellec.c
	cc -o ellec -O ellec.c

# SB library, used by ELLE.
#	This target should not be directly invoked by the user since it
#	requires that RANLIB be defined; for direct invocation, better to
#	just do a MAKE of "sb" in the sbmake file.

libsb.a:
	make -f sbmake sbnoran
	$(RANLIB) libsb.a

# ------------------------------------------------------------------
#	ELLE Variants (system or configuration dependent)

# SUN workstation system/configuration.
#	Needs extra libraries to support window hacking; libsb.a comes last
#	so that "valloc" refs can be satisfied from SB library instead of
#	C library.
#	Plus special patch to binary, very installation-dependent!

sunelle: $(CONFS)
	make -f makecf.rl -f makecf.fun -f Makefile B-sunelle

B-sunelle: $(CORE_OFILES) $(FUN_OFILES) eesun.o elle.h eesite.h libsb.a
	$(RANLIB) elib.a
	cc  -o xelle -u _main elib.a -ltermlib\
		-lsuntool -lsunwindow -lpixrect\
		libsb.a
	echo "tool_select+148?W my_select" > esunpat.sh
	adb -w xelle < esunpat.sh

# APOLLO system.
#	Linker doesn't understand libraries as regular arguments.
#	May have troubles with duplication of CORE and FUN files.
#	System has no termlib - uses TERMCAP emulation that comes with AUX.

SB_OFILES = sbstr.o sbm.o sberr.o

apolloelle: $(CONFS)
	make -f makecf.rl -f makecf.fun -f Makefile B-apolloelle

B-apolloelle: $(CORE_OFILES) $(FUN_OFILES) elle.h eesite.h libsb.a
	cc -o xelle $(CORE_OFILES) $(FUN_OFILES) $(SB_OFILES) -ltermcap

# IBM PC/IX system.
#	Just needs -ltermcap instead of -ltermlib.
#	Does not have/need "ranlib" by the way.

pcixelle: $(CONFS)
	make -f makecf.rl -f makecf.fun -f Makefile B-pcixelle

B-pcixelle: $(CORE_OFILES) $(FUN_OFILES) elle.h eesite.h libsb.a
	cc -i -o xelle -u _main elib.a libsb.a -ltermcap

