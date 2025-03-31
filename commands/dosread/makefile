CFLAGS=
l=/usr/lib

obj = dosread.s
sh:	$(obj)
	@asld -i -o dosread $l/crtso.s $(obj) $l/libc.a $l/end.s
	@chmem =32000 dosread
