OBJ=adb.s optab.s

CFLAGS=-c -T.

.c.s:
	cc $(CFLAGS) $*.c

adb:	$(OBJ)
	rm /lib/cpp /lib/cem
	cc -T. $(OBJ)
