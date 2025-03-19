# To make  'cp', type:   make f=cp
# To make  'ls', type:   make f=ls
# To make  'cat', type:   make f=cat
# Get the idea?

l=/usr/lib
i=/usr/include
CFLAGS= -Di8088 -I/usr/include -F

file:	$l/libc.a $f.s
	@cc -o bin/$f $f.s
	@chmem =2048 bin/$f >/dev/null
	@echo "$f done ."
