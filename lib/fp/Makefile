#
# Makefile for fpp and floating point library
#
CFLAGS = -f -LIB -DFLOATLIB

FPPOBJS	= fpp.s atof.s strtod.s strtod_aux.s ldexp.s cff.s cmf8.s \
	  zrf8.s norm8.s norm4.s

fpp: $(FPPOBJS)
	cc -o fpp $(FPPOBJS)
fpp.s:
	cc -c fpp.c

CSRCS	= fabs.c pow.c exp.c sqrt.c log10.c log.c _poly.c _mult.c \
	  ceil.c floor.c atof.c strtod.c printf.c fprintf.c sprintf.c \
	  vprintf.c vsprintf.c vfprintf.c
COBJS	= fabs.s pow.s exp.s sqrt.s log10.s log.s _poly.s _mult.s \
	  ceil.s floor.s atof.s strtod.s printf.s fprintf.s sprintf.s \
	  vprintf.s vsprintf.s vfprintf.s
GENSRCS	= frexp.x modf.x strtod_aux.x ldexp.x
GENOBJS	= frexp.s modf.s strtod_aux.s ldexp.s
AUXSRCS	= cff.x cfu.x cuf.x cif.x addsub.x cfi.x fat.x trp.x dvf8.x \
	  ngf8.x mlf8.x cmf8.x zrf4.x zrf8.x norm4.x norm8.x
AUXOBJS	= cff.s cfu.s cuf.s cif.s addsub.s cfi.s fat.s trp.s dvf8.s \
	  ngf8.s mlf8.s cmf8.s zrf4.s zrf8.s norm4.s norm8.s

MINIX1_1SRCS	= ret8.x return.x
# MINIX1_1OBJS	= ret8.s return.s

LIBOBJS	= ${COBJS} ${GENOBJS} ${AUXOBJS} ${MINIX1_1OBJS}

libfp.a: ${LIBOBJS}
	ar cr libfp.a `lorder ${LIBOBJS} | tsort`

.SUFFIXES: .x
.x.s:
	-libpack < $*.x | sed '/^$$/d' > $*.s

clean:
	rm -f libfp.a fpp $(LIBOBJS) core

depend:
	mkdep fpp.c $(CSRCS)

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

fpp.s: /usr/include/stdio.h
fpp.s: fpp.c
fabs.s: /usr/include/math.h
fabs.s: fabs.c
pow.s: /usr/include/errno.h
pow.s: /usr/include/limits.h
pow.s: /usr/include/math.h
pow.s: pow.c
exp.s: /usr/include/errno.h
exp.s: /usr/include/limits.h
exp.s: /usr/include/math.h
exp.s: exp.c
sqrt.s: /usr/include/errno.h
sqrt.s: /usr/include/math.h
sqrt.s: sqrt.c
log10.s: /usr/include/math.h
log10.s: log10.c
log.s: /usr/include/errno.h
log.s: /usr/include/math.h
log.s: log.c
_poly.s: /usr/include/math.h
_poly.s: _poly.c
_mult.s: /usr/include/math.h
_mult.s: _mult.c
ceil.s: /usr/include/math.h
ceil.s: ceil.c
floor.s: /usr/include/math.h
floor.s: floor.c
atof.s: atof.c
strtod.s: /usr/include/ctype.h
strtod.s: strtod.c
printf.s: /usr/include/stdarg.h
printf.s: /usr/include/stdio.h
printf.s: printf.c
fprintf.s: /usr/include/stdarg.h
fprintf.s: /usr/include/stdio.h
fprintf.s: fprintf.c
sprintf.s: /usr/include/stdarg.h
sprintf.s: /usr/include/stdio.h
sprintf.s: sprintf.c
vprintf.s: /usr/include/stdarg.h
vprintf.s: /usr/include/stdio.h
vprintf.s: vprintf.c
vsprintf.s: /usr/include/stdarg.h
vsprintf.s: /usr/include/stdio.h
vsprintf.s: vsprintf.c
vfprintf.s: /usr/include/ctype.h
vfprintf.s: /usr/include/stdarg.h
vfprintf.s: /usr/include/stdio.h
vfprintf.s: /usr/include/sys/types.h
vfprintf.s: vfprintf.c

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
