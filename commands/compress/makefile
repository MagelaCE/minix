CFLAGS=
l=/usr/lib

obj = compress.s
sh:	$(obj)
	@asld -i -o compress $l/crtso.s $(obj) $l/libc.a $l/end.s
	@chmem =64000 compress
