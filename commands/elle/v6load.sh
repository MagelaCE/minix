: V6 with termcap
cc   -o xelle $1 $2 $3 -u _main elib.a libsb.a -lS -ltermlib

: V6 PWB may want this (for logdir function)
: cc -o xelle $1 $2 $3 -u _main elib.a libsb.a -lPW -lS string.o
