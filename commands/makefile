# To make  'cp', type:   make f=cp
# To make  'ls', type:   make f=ls
# To make  'cat', type:   make f=cat
# Get the idea?

# To make everything, use:
#  mkdir bin
#  for i in *.c
#  do make f=`basename $i .c`
#  done
#
# Furthermore, many commands need a different memory allocation, so chmem must
# be run to change it.  See the shell script 'changemem' in tools.

l=/usr/lib
CFLAGS=  -F -D_MINIX -D_POSIX_SOURCE

File:	$l/libc.a $f.s
	@cc -i -o bin/$f $f.s
	@chmem =8192 bin/$f >/dev/null
	@echo "$f done ."



