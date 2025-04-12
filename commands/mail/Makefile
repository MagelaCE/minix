#
#     Makefile for W-MAIL Local Mail Agent (MINIX)
#
#   Makefile @(#)1.3        MicroWalt W-MAIL V2.5
#
# Define UMAILER if your remote mailer understands the
# "-i datafile" option. Otherwise "< datafile" will be used.
#
# Define V7MAIL if you do not want the "To:"-line to be
# included in the message header.
#
PROG	=	/usr/lib/uucp
BIN	=	/usr/bin

OPTS	=	-DUMAILER
OPSYS   =	MINIX

CFLAGS	=	-D$(OPSYS) -DUWALT -S $(OPTS)
LDFLAGS =	-i

OBJS	=	wmmain.s wmread.s wmcreate.s wminteract.s \
		wmdeliver.s wmtime.s
SRCS	=	wmmain.c wmread.c wmcreate.c wminteract.c \
		wmdeliver.c wmtime.c
OTHERS	=	README wmail.doc


wmail:		$(OBJS)
		cc $(LDFLAGS) -o wmail $(OBJS)
		@chmem =20000 wmail >/dev/null

install:	wmail
		@echo 'Copying files...'
		@rm -f $(BIN)/wmail $(BIN)/lmail $(BIN)/Mail \
			$(BIN)/mailx $(BIN)/mail
		@cp wmail $(BIN)
		@echo 'Setting up links...'
		@ln $(BIN)/wmail $(BIN)/lmail
		@ln $(BIN)/wmail $(BIN)/mail
		@ln $(BIN)/wmail $(BIN)/Mail
		@ln $(BIN)/wmail $(BIN)/mailx
		@echo 'Setting up permissions...'
		chown root.root $(BIN)/wmail
		chmod 6555 $(BIN)/wmail

shar:		wmail
		@rm -f wmail.shar
		@shar -v -o wmail.shar $(OTHERS) wmail Makefile $(SRCS)

tar:		wmail
		@rm -f wmail.tar
		@tar c wmail.tar $(OTHERS) wmail Makefile $(SRCS)

wmmain.s:	wmail.h wmmain.c
		cc $(CFLAGS) wmmain.c

wmread.s:	wmail.h wmread.c
		cc $(CFLAGS) wmread.c

wmcreate.s:	wmail.h wmcreate.c
		cc $(CFLAGS) wmcreate.c

wminteract.s:	wmail.h wminteract.c
		cc $(CFLAGS) wminteract.c

wmdeliver.s:	wmail.h wmdeliver.c
		cc $(CFLAGS) wmdeliver.c

wmtime.s:	wmtime.c
		cc $(CFLAGS) wmtime.c

