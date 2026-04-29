CFLAGS= -F -D_MINIX -D_POSIX_SOURCE

$f:	$f.s
	@cc -o $f $f.s
	@chmem =8192 $f >/dev/null

clean:	
	@rm -f *.s *.bak

# The test11, test17, and test18 files must be setuid root, which means
# that you should be root when typing 'make all'.  Furthermore, cem needs
# extra memory when compiling test17.c and test18.c.
all:
	cc $(CFLAGS) -o test0 test0.c
	cc $(CFLAGS) -o test1 test1.c
	cc $(CFLAGS) -o test2 test2.c
	cc $(CFLAGS) -o test3 test3.c
	cc $(CFLAGS) -o test4 test4.c
	cc $(CFLAGS) -o test5 test5.c
	cc $(CFLAGS) -o test6 test6.c
	cc $(CFLAGS) -o test7 test7.c
	cc $(CFLAGS) -o test8 test8.c
	cc $(CFLAGS) -o test9 test9.c
	cc $(CFLAGS) -o test10 test10.c
	cc $(CFLAGS) -o test11 test11.c
	cc $(CFLAGS) -o test12 test12.c
	cc $(CFLAGS) -o test13 test13.c
	cc $(CFLAGS) -o test14 test14.c
	cc $(CFLAGS) -o test15 test15.c
	cc $(CFLAGS) -o test16 test16.c
	cc $(CFLAGS) -o test17 test17.c
	cc $(CFLAGS) -o test18 test18.c
	cc $(CFLAGS) -o test19 test19.c
	cc $(CFLAGS) -o test20 test20.c
	cc $(CFLAGS) -o test21 test21.c
	cc $(CFLAGS) -o t10a t10a.c
	cc $(CFLAGS) -o t11a t11a.c
	cc $(CFLAGS) -o t11b t11b.c
	rm -f *.bak *.s
	chmem =20000 test14
	chmem =65000 test20
	chown bin test*
	chown root test11 test17 test18
	chmod 4755 test11 test17 test18
	chmod 777 .

