: V6 runfile for compiling ELLE modules
if $1x = x goto done
: loop
echo Hacking $1.c ...
cc -c -O $1.c
if -r $1.o goto won
echo Compile of $1 failed - placeholder included
cat /dev/null > $1.o
ar rv elib.a $1.o
goto skipsiz

: won
size $1.o
: skipsiz
ar rv elib.a $1.o
rm -f $1.o
shift
if ! $1x = x goto loop
: done
