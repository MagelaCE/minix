CFLAGS= -D_MINIX -D_POSIX_SOURCE

File:	$f.s
	@cc -o $f $f.s
	@chmem =8192 $f >/dev/null

clean:	
	@rm -f *.s *.bak

# The test11, test17, and test18 files must be setuid root, which means
# that you should be root when typing 'make all'.  Furthermore, cem needs
# extra memory when compiling test17.c and test18.c.
all:
	cc -o test0 test0.c
	cc -o test1 test1.c
	cc -o test2 test2.c
	cc -o test3 test3.c
	cc -o test4 test4.c
	cc -o test5 test5.c
	cc -o test6 test6.c
	cc -o test7 test7.c
	cc -o test8 test8.c
	cc -o test9 test9.c
	cc -o test10 test10.c
	cc -o test11 test11.c
	cc -o test12 test12.c
	cc -o test13 test13.c
	cc -o test14 test14.c
	cc -o test15 test15.c
	cc -o test16 test16.c
	cc -o test17 test17.c
	cc -o test18 test18.c
	cc -o test19 test19.c
	cc -o test20 test20.c
	cc -o test21 test21.c
	cc -o t10a t10a.c
	cc -o t11a t11a.c
	cc -o t11b t11b.c
	rm -f *.bak *.s
	chmem =20000 test20
	chown ast test*
	chown root test11 test17 test18
	chmod 4755 test11 test17 test18

