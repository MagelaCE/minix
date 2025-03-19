.SUFFIXES:	.c .s

CFLAGS = -F 

OBJS =	append.s catsub.s ckglob.s deflt.s del.s docmd.s doglob.s\
	doprnt.s doread.s dowrite.s ed.s egets.s find.s getfn.s getlst.s\
	getnum.s getone.s getptr.s getrhs.s gettxt.s ins.s maksub.s move.s\
	optpat.s set.s setbuf.s subst.s getpat.s matchs.s amatch.s\
	unmkpat.s omatch.s makepat.s bitmap.s dodash.s esc.s system.s

ed:	$(OBJS)
	cc -i -o ed $(OBJS)

