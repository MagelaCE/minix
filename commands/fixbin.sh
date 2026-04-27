#This shell script installs new binaries, changing sizes and modes
#Call: $0 src_dir dst_dir
#Example: $0 /usr/src/commands/bin /bin

#Check for args
case $# in
2) ;;
*) echo Usage: $0 src_dir dst_dir
   exit 1
   ;;
esac

src=$1
dst=$2
if test -d $src; then : ; else echo $src is not a directory; exit 1; fi
if test -d $dst; then : ; else echo $dst is not a directory; exit 1; fi

#Is dst dir writable?
date >$dst/$$
if test -r $dst/$$
   then :
   else echo $0: $dst is not writable
        exit 1
fi

#Is this script running as superuser?
if chown bin $dst/$$ >/dev/null 2>&1
   then :
   else echo You must be superuser to run $0
	rm $dst/$$
	exit 1
fi
	
#Copy all the files into position if needed
if test $src = $dst
   then :			# e.g., $0 /bin /bin
   else cd $src
	cp * $dst
fi
	
#Set owner
cd $dst
chown bin *

#The following files are setuid root
chown root $dst/badblocks
chown root $dst/chgrp
chown root $dst/de
chown root $dst/df
chown root $dst/fsck
chown root $dst/login
chown root $dst/mv
chown root $dst/passwd
chown root $dst/readall
chown root $dst/readclock
chown root $dst/su

chmod 4755 $dst/badblocks
chmod 4755 $dst/chgrp
chmod 4755 $dst/de
chmod 4755 $dst/df
chmod 4755 $dst/fsck
chmod 4755 $dst/login
chmod 4755 $dst/mv
chmod 4755 $dst/passwd
chmod 4755 $dst/readall
chmod 4755 $dst/readclock
chmod 4755 $dst/su

#Increase default stack size from 8K to more for selected programs
chmem =50000 $dst/ar
chmem =16000 $dst/ast
chmem =60000 $dst/asld
chmem =16000 $dst/cp
chmem =55000 $dst/cdiff
chmem =64000 $dst/compress
chmem =64000 $dst/cpdir
chmem =20000 $dst/de
chmem =40000 $dst/dd
chmem =32000 $dst/du
chmem =55000 $dst/diff
chmem =32000 $dst/dosread
chmem =60000 $dst/ed
chmem =50000 $dst/file
chmem =50000 $dst/find
chmem =60000 $dst/fix
chmem =64000 $dst/libpack
chmem =64000 $dst/libupack
chmem =20000 $dst/make
chmem =64000 $dst/mined
chmem =40000 $dst/mkfs
chmem =16000 $dst/nm
chmem =65000 $dst/nroff
chmem =32000 $dst/pr
chmem =50000 $dst/readfs
chmem =16000 $dst/roff
chmem =16000 $dst/strings
chmem =14000 $dst/sh
chmem =60000 $dst/sort
chmem =16000 $dst/strip
chmem =60000 $dst/treecmp

