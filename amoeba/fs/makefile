# This makefile is used for both the PC and AT.  If you run out of space
# while compiling or linking, you may have to use the -T. flag, or even
# rm /lib/cem and /lib/cpp while running asld.  See the makefiles in
# kernel/IBM_PC.

CFLAGS= -DAM_KERNEL -Di8088 -w -F -I../../fs
H=../../h
F=../../fs
L=/usr/lib

OBJ =	main.s open.s read.s write.s pipe.s device.s \
	path.s mount.s link.s super.s inode.s cache.s filedes.s \
	stadir.s protect.s time.s misc.s utility.s table.s putc.s

fs:	makefile   $L/head.s $(OBJ) $L/libc.a $L/end.s
	@echo "Start linking FS.  "
	@asld  -o fs $L/head.s $(OBJ) $L/libc.a $L/end.s
	@echo "FS done.  "

clean:
	rm -f $(OBJ)

cache.s:	$F/const.h $F/type.h $H/const.h $H/type.h
cache.s:	$H/error.h
cache.s:	$F/buf.h
cache.s:	$F/file.h
cache.s:	$F/fproc.h
cache.s:	$F/glo.h
cache.s:	$F/inode.h
cache.s:	$F/super.h
	$(CC) $(CFLAGS) -c $F/cache.c

device.s:	$F/const.h $F/type.h $H/const.h $H/type.h
device.s:	$H/com.h
device.s:	$H/error.h
device.s:	$F/dev.h
device.s:	$F/file.h
device.s:	$F/fproc.h
device.s:	$F/glo.h
device.s:	$F/inode.h
device.s:	$F/param.h
	$(CC) $(CFLAGS) -c $F/device.c

filedes.s:	$F/const.h $F/type.h $H/const.h $H/type.h
filedes.s:	$H/error.h
filedes.s:	$F/file.h
filedes.s:	$F/fproc.h
filedes.s:	$F/glo.h
filedes.s:	$F/inode.h
	$(CC) $(CFLAGS) -c $F/filedes.c

inode.s:	$F/const.h $F/type.h $H/const.h $H/type.h
inode.s:	$H/error.h
inode.s:	$F/buf.h
inode.s:	$F/file.h
inode.s:	$F/fproc.h
inode.s:	$F/glo.h
inode.s:	$F/inode.h
inode.s:	$F/super.h
	$(CC) $(CFLAGS) -c $F/inode.c

link.s:		$F/const.h $F/type.h $H/const.h $H/type.h
link.s:		$H/error.h
link.s:		$F/buf.h
link.s:		$F/file.h
link.s:		$F/fproc.h
link.s:		$F/glo.h
link.s:		$F/inode.h
link.s:		$F/param.h
	$(CC) $(CFLAGS) -c $F/link.c

main.s:		$F/const.h $F/type.h $H/const.h $H/type.h
main.s:		$H/callnr.h
main.s:		$H/com.h
main.s:		$H/error.h
main.s:		$F/buf.h
main.s:		$F/file.h
main.s:		$F/fproc.h
main.s:		$F/glo.h
main.s:		$F/inode.h
main.s:		$F/param.h
main.s:		$F/super.h
	$(CC) $(CFLAGS) -c $F/main.c

misc.s:		$F/const.h $F/type.h $H/const.h $H/type.h
misc.s:		$H/callnr.h
misc.s:		$H/com.h
misc.s:		$H/error.h
misc.s:		$F/buf.h
misc.s:		$F/file.h
misc.s:		$F/fproc.h
misc.s:		$F/glo.h
misc.s:		$F/inode.h
misc.s:		$F/param.h
misc.s:		$F/super.h
	$(CC) $(CFLAGS) -c $F/misc.c

mount.s:	$F/const.h $F/type.h $H/const.h $H/type.h
mount.s:	$H/error.h
mount.s:	$F/buf.h
mount.s:	$F/file.h
mount.s:	$F/fproc.h
mount.s:	$F/glo.h
mount.s:	$F/inode.h
mount.s:	$F/param.h
mount.s:	$F/super.h
	$(CC) $(CFLAGS) -c $F/mount.c

open.s:		$F/const.h $F/type.h $H/const.h $H/type.h
open.s:		$H/callnr.h
open.s:		$H/error.h
open.s:		$F/buf.h
open.s:		$F/file.h
open.s:		$F/fproc.h
open.s:		$F/glo.h
open.s:		$F/inode.h
open.s:		$F/param.h
	$(CC) $(CFLAGS) -c $F/open.c

path.s:		$F/const.h $F/type.h $H/const.h $H/type.h
path.s:		$H/error.h
path.s:		$F/buf.h
path.s:		$F/file.h
path.s:		$F/fproc.h
path.s:		$F/glo.h
path.s:		$F/inode.h
path.s:		$F/super.h
	$(CC) $(CFLAGS) -c $F/path.c

pipe.s:		$F/const.h $F/type.h $H/const.h $H/type.h
pipe.s:		$H/callnr.h
pipe.s:		$H/com.h
pipe.s:		$H/error.h
pipe.s:		$H/signal.h
pipe.s:		$F/file.h
pipe.s:		$F/fproc.h
pipe.s:		$F/glo.h
pipe.s:		$F/inode.h
pipe.s:		$F/param.h
	$(CC) $(CFLAGS) -c $F/pipe.c

protect.s:	$F/const.h $F/type.h $H/const.h $H/type.h
protect.s:	$H/error.h
protect.s:	$F/buf.h
protect.s:	$F/file.h
protect.s:	$F/fproc.h
protect.s:	$F/glo.h
protect.s:	$F/inode.h
protect.s:	$F/param.h
protect.s:	$F/super.h
	$(CC) $(CFLAGS) -c $F/protect.c

putc.s:		$F/const.h $F/type.h $H/const.h $H/type.h
putc.s:		$H/com.h
	$(CC) $(CFLAGS) -c $F/putc.c

read.s:		$F/const.h $F/type.h $H/const.h $H/type.h
read.s:		$H/com.h
read.s:		$H/error.h
read.s:		$F/buf.h
read.s:		$F/file.h
read.s:		$F/fproc.h
read.s:		$F/glo.h
read.s:		$F/inode.h
read.s:		$F/param.h
read.s:		$F/super.h
	$(CC) $(CFLAGS) -c $F/read.c

stadir.s:	$F/const.h $F/type.h $H/const.h $H/type.h
stadir.s:	$H/error.h
stadir.s:	$H/stat.h
stadir.s:	$F/file.h
stadir.s:	$F/fproc.h
stadir.s:	$F/glo.h
stadir.s:	$F/inode.h
stadir.s:	$F/param.h
	$(CC) $(CFLAGS) -c $F/stadir.c

super.s:	$F/const.h $F/type.h $H/const.h $H/type.h
super.s:	$H/error.h
super.s:	$F/buf.h
super.s:	$F/inode.h
super.s:	$F/super.h
	$(CC) $(CFLAGS) -c $F/super.c

table.s:	$F/const.h $F/type.h $H/const.h $H/type.h
table.s:	$H/com.h
table.s:	$H/callnr.h
table.s:	$H/error.h
table.s:	$H/stat.h
table.s:	$F/buf.h
table.s:	$F/dev.h
table.s:	$F/file.h
table.s:	$F/fproc.h
table.s:	$F/glo.h
table.s:	$F/inode.h
table.s:	$F/super.h
	$(CC) $(CFLAGS) -c $F/table.c

time.s:		$F/const.h $F/type.h $H/const.h $H/type.h
time.s:		$H/callnr.h
time.s:		$H/com.h
time.s:		$H/error.h
time.s:		$F/file.h
time.s:		$F/fproc.h
time.s:		$F/glo.h
time.s:		$F/inode.h
time.s:		$F/param.h
	$(CC) $(CFLAGS) -c $F/time.c

utility.s:	$F/const.h $F/type.h $H/const.h $H/type.h
utility.s:	$H/com.h
utility.s:	$H/error.h
utility.s:	$F/buf.h
utility.s:	$F/file.h
utility.s:	$F/fproc.h
utility.s:	$F/glo.h
utility.s:	$F/inode.h
utility.s:	$F/param.h
utility.s:	$F/super.h
	$(CC) $(CFLAGS) -c $F/utility.c

write.s:	$F/const.h $F/type.h $H/const.h $H/type.h
write.s:	$H/error.h
write.s:	$F/buf.h
write.s:	$F/file.h
write.s:	$F/fproc.h
write.s:	$F/glo.h
write.s:	$F/inode.h
write.s:	$F/super.h
	$(CC) $(CFLAGS) -c $F/write.c
