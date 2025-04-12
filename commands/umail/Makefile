#
#       Makefile for UMAIL Mail Router (MINIX)
#
# Author:   Fred van Kempen, MicroWalt Corporation
#
# Define WMAILER if the local mailer is W-MAIL.
# This mailer understands the "-i filename" option,
# and this works much faster than stdIO redirecting.
#
CFLAGS  =	-DMINIX -DWMAILER

BIN	=	/usr/bin

OBJS    =	ummain.s umtime.s umheader.s umconvert.s \
		umroute.s umsend.s umscanner.s
SRCS    =	ummain.c umtime.c umheader.c umconvert.c \
		umroute.c umsend.c umscanner.c
OTHERS	=	README umail.doc umail.cf umail Makefile uucp.h umail.h

umail:	   	Makefile $(OBJS)
	   	cc -i -o umail $(OBJS)
		@chmem =16000 umail >/dev/null

install:	umail
		@rm -f $(BIN)/umail $(BIN)/rmail
		@echo 'Copying files...'
		@cp umail $(BIN)/umail
		@echo 'Setting up links...'
		@ln $(BIN)/umail $(BIN)/rmail
		@echo 'Setting up permissions:'
		chown root.root $(BIN)/umail
		chmod 6555 $(BIN)/umail

shar:
		@shar -v -o umail.shar $(OTHERS) $(SRCS)

tar:
		@tar c umail.tar $(OTHERS) $(SRCS)

ummain.s:  	umail.h ummain.c
	   	cc $(CFLAGS) -S ummain.c

umtime.s:  	umail.h umtime.c
	   	cc $(CFLAGS) -S umtime.c

umheader.s:  	umail.h umheader.c
		cc $(CFLAGS) -S umheader.c

umconvert.s:    umail.h umconvert.c
	   	cc $(CFLAGS) -S umconvert.c

umroute.s:      umail.h umroute.c
	   	cc $(CFLAGS) -S umroute.c

umsend.s:    	umail.h uucp.h umsend.c
	   	cc $(CFLAGS) -S umsend.c

umscanner.s: 	umscanner.c
	   	cc $(CFLAGS) -S umscanner.c

