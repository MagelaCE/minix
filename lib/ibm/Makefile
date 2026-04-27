CFLAGS= -c -F -LIB -D_MINIX -D_POSIX_SOURCE

$f.s:	$f.c
	@echo ------------------------------ $f ------------------------------
	@cc $(CFLAGS) $f.c


clean:	
	@rm -rf portio.s peekpoke.s *.bak


all:	
	@rm -f portio.s peekpoke.s
	@ls *.s >tmp.1
	@sed <tmp.1 >tmp.2 '/.*/s//& >\/tmp\/&/;/^/s//libpack </'
	@sh tmp.2
	@rm tmp.?
	@ls *.c >tmp.1
	@sed <tmp.1 >tmp.2 '/\.c/s///;/^/s//make f=/'
	@rm -rf tmp.1 *.s *.bak
	@sh tmp.2
	@rm tmp.?
