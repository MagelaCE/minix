: First ensure SB library set up
if -r libsb.a goto skipsb
sbv6mk.sh
: skipsb

: Then ensure ELLEC profile compiler is there
if -r ellec goto skipec
cc -o ellec -O ellec.c
: skipec

: Now re-do configuration and profile if necessary
if ! -r v6conf.sh goto doconf
if ! -r defprf.c  goto doconf
if ! -r eefdef.h  goto doconf
if ! -r eefidx.h  goto doconf
goto skipconf
: doconf
cat deffun.e defprf.e | ellec -CSconf v6.cc > v6conf.sh
cat deffun.e defprf.e | ellec -Pconf  > defprf.c
cat deffun.e defprf.e | ellec -Fconf  > eefdef.h
cat deffun.e defprf.e | ellec -FXconf > eefidx.h
rm -f eecmds.o
: skipconf

: Now hack ELLE - first do ELLE core files, which must always exist
: regardless of the configuration.
v6.cc eemain eecmds eesite eevini eedisp eeterm eeerr eeques eefed eeedit \
	eefile eebuff

: Then do ELLE function files - depends on configuration
sh < config.v6
