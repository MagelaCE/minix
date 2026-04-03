###############################################################################
##									     ##
##	File:     Makefile						     ##
##	Altered-by:   Frank Wortner					     ##
##	Created:  Fri May 27 09:47:49 1988				     ##
##	Contents: A Makefile for Patch Under Minix			     ##
##									     ##
##	$Log$								     ##
###############################################################################
CC = cc
bin = /usr/bin
mansrc = /usr/doc
manext = l
CFLAGS =  -DMINIX
LDFLAGS = -i


public = patch
libs = 
private = 
defs =
manpages = patch.man
util = Makefile

c = patch.c pch.c inp.c version.c util.c

obj = patch.s pch.s inp.s util.s version.s


all: $(public) $(private) $(util)
	touch all

patch: $(obj)
	$(CC) $(LDFLAGS) $(obj) $(libs) -o patch

# won't work with csh
install: patch
	export PATH || exit 1
	- mv $(bin)/patch $(bin)/patch.old
	- if test `pwd` != $(bin); then cp $(public) $(bin); fi
	cd $(bin); chmod 755 $(public)
	- if test `pwd` != $(mansrc); then \
for page in $(manpages); do \
cp $$page $(mansrc)/`basename $$page .man`.$(manext); \
done; \
fi

clean:
	rm -f *.s *.orig core

patch.s: config.h common.h patch.c inp.h pch.h util.h version.h
pch.s: config.h common.h pch.c pch.h util.h
inp.s: config.h common.h inp.c inp.h util.h
util.s: config.h common.h util.c util.h
version.s: config.h common.h version.c version.h patchlevel.h util.h
