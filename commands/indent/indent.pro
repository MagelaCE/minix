-bap -bbb -br -ncdb -cli0.5 -di1 -nei -lp -npsl -nfc1 -nip/
echo x - makefile
sed '/^X/s///' > makefile << '/'
# makefile for the 'indent' command

CFLAGS = -F -T. -Dlint

OBJS = indent.s io.s lexi.s parse.s comment.s args.s

indent: ${OBJS}
	$(CC) -i -T. -o indent ${OBJS}

$(OBJS): globs.h codes.h

