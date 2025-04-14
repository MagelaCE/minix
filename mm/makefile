i=/usr/include
s=/usr/include/sys
h=/usr/include/minix
l=/usr/lib
a=const.h $h/const.h $h/type.h $h/config.h $i/errno.h $i/limits.h $s/types.h
CFLAGS = -F

obj =	main.s forkexit.s break.s exec.s signal.s getset.s \
	alloc.s utility.s table.s putc.s trace.s

mm:	makefile    $l/head.s $(obj) $l/libc.a $l/end.s
	asld -o mm $l/head.s $(obj) $l/libc.a $l/end.s

clean:
	rm -f $(obj)

alloc.s:	$a
alloc.s:	glo.h

break.s:	$a
break.s:	$i/signal.h
break.s:	glo.h
break.s:	mproc.h
break.s:	param.h

exec.s:		$a
exec.s:		$h/callnr.h
exec.s:		$s/stat.h
exec.s:		glo.h
exec.s:		mproc.h
exec.s:		param.h

forkexit.s:	$a
forkexit.s:	$h/callnr.h
forkexit.s:	glo.h
forkexit.s:	mproc.h
forkexit.s:	param.h

getset.s:	$a
getset.s:	$h/callnr.h
getset.s:	glo.h
getset.s:	mproc.h
getset.s:	param.h

main.s:		$a
main.s:		$h/callnr.h
main.s:		$h/com.h
main.s:		glo.h
main.s:		mproc.h
main.s:		param.h

putc.s:		$a
putc.s:		$h/com.h
putc.s:		glo.h

signal.s:	$a
signal.s:	$h/callnr.h
signal.s:	$h/com.h
signal.s:	$i/signal.h
signal.s:	glo.h
signal.s:	mproc.h
signal.s:	param.h

table.s:	$a
table.s:	$h/callnr.h
table.s:	glo.h
table.s:	mproc.h
table.s:	param.h

trace.s:	$a
trace.s:	$i/signal.h
trace.s:	glo.h
trace.s:	mproc.h
trace.s:	param.h

utility.s:	$a
utility.s:	$h/callnr.h
utility.s:	$h/com.h
utility.s:	$s/stat.h
utility.s:	glo.h
utility.s:	mproc.h
