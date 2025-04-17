cc -c -O -DV6 sbstr.c sbm.c sberr.c bcopy.s
ar rv libsb.a sbstr.o sbm.o sberr.o bcopy.o
rm sbstr.o sbm.o sberr.o bcopy.o
