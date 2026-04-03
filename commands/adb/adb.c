#include <stdio.h>
#include "adb.h"

struct file binary;
struct file core;

long dot;
int dotoff;
int maxoff = 256;
int ibase = 16;

int lastc = 0;
char lbuf[128];
char *bufp = lbuf;
char sign = 0;

int mode = 0;
int buffer = 0;

main(argc, argv)
int argc;
char *argv[];
{
	char *cp;

	binary.symptr = 0;
	core.symptr = 0;
	while (argc > 1 && argv[1][0] == '-') {
		for (cp = &argv[1][1]; *cp; )
			switch (*cp++) {

			case 'b':
				buffer++;
				break;
			case 'w':
				mode = 2;
				break;
			case 'm':
				setsym(argv[2]);
				argc--;
				argv++;
				break;
			default:
				printf("%c:unknown flag\n", cp[-1]);
				break;

			}
		argc--;
		argv++;
	}
	if (argc > 1) {
		binary.name = argv[1];
		core.name = argv[1];
	}
	if (argc > 2)
		core.name = argv[2];
	if ((binary.fid = open(binary.name, mode)) < 0) {
		printf("%s:cannot open\n", binary.name);
		exit(1);
	}
	if ((core.fid = open(core.name, mode)) < 0) {
		printf("%s:cannot open\n", core.name);
		exit(2);
	}
	setobj(&binary);
	setcor(&core);
	adb();
}
setobj(fp)
struct file *fp;
{
	struct fheader hdr;

	fp->b1 = 0;
	fp->e1 = -1;
	fp->f1 = 0;
	fp->b2 = 0;
	fp->e2 = -1;
	fp->f2 = 0;
	fp->cblock = -1;
	lseek(fp->fid, (long)0, 0);
	if (read(fp->fid, &hdr, sizeof(hdr)) != sizeof(hdr))
		return;
	if (hdr.magic != MAGIC && hdr.magic != IDMAGIC)
		return;
	fp->b1 = 0;
	fp->e1 = hdr.tsize;
	fp->f1 = sizeof(hdr);
	fp->b2 = hdr.tsize;
	fp->e2 = hdr.tsize + hdr.dsize;
	fp->f2 = hdr.tsize + hdr.tsize;
	return;
}
setcor(fp)
struct file *fp;
{

	fp->b1 = 0;
	fp->e1 = -1;
	fp->f1 = 0;
	fp->b2 = 0;
	fp->e2 = 0;
	fp->f2 = 0;
	fp->cblock = -1;
	return;
}
adb()
{
	int cmddol(), cmdcol(), cmdprt(), cmdwrt(), null();
	long getn(), getdot();
	long expr();
	register int c, lc, count;
	int (*f)();
	long (*g)();
	char fmt[128];

	f = cmdprt;
	g = getdot;
	lc = '=';
	dot = 0;
	dotoff = 0;
	for (;;) {
		if ((c = peekc()) == '(' || c == '.' || (type(c) & ALPHANUM)) {
			dot = expr();
			dotoff = 0;
		}
		if (peekc() == ',') {
			nb();
			count = expr();
		} else
			count = 1;
		switch (c = nb()) {

		case EOF:
			return;
		case '$':
			f = cmddol;
			break;
		case ':':
			f = cmdcol;
			break;
		case '?':
		case '/':
			g = getn;
			switch (peekc()) {

			case 'w':
			case 'W':
				f = cmdwrt;
				break;
			default:
				f = cmdprt;
				getfmt(fmt);
			case '\n':
				break;

			}
			break;
		case '=':
			f = cmdprt;
			g = getdot;
			getfmt(fmt);
			break;
		case '\r':
		case '\n':
			c = lc;
			dot += dotoff;
			break;
		default:
			f = null;
			count = 1;
			break;

		}
		dotoff = 0;
		while (count--)
			(*f)(c, fmt, g);
		lc = c;
		while (getchr() != '\n')
			;
	}
}
getchr()
{
	register int c;

	if (lastc) {
		c = lastc;
		lastc = 0;
	} else
		c = getchar();
	return(c);
}
pushc(c)
int c;
{

	lastc = c;
	return(c);
}
peekc()
{

	return(pushc(nb()));
}
nb()
{
	register int c;

	while ((c = getchr()) == ' ' || c == '\t')
		;
	if (c == '\n')
		pushc(c);
	return(c);
}
type(c)
char c;
{

	if (c >= '0' && c <= '9')
		return(NUMERIC);
	if (c >= 'a' && c <= 'f')
		return(HEX);
	if (c >= 'A' && c <= 'F')
		return(HEX);
	if (c >= 'g' && c <= 'z')
		return(ALPHA);
	if (c >= 'G' && c <= 'Z')
		return(ALPHA);
	if (c == '_')
		return(ALPHA);
	return(SPEC);
}
	long
expr()
{
	long term();
	long r;
	int c;

	r = term();
	for (;;)
		switch (c = nb()) {

		case '+':
			r += term();
			break;
		case '-':
			r -= term();
			break;
		case '*':
			r *= term();
			break;
		case '%':
			r /= term();
			break;
		case ')':
		default:
			pushc(c);
			return(r);

		}
}
	long
term()
{
	long n;
	register int c, base;
	char *cp, buf[80];
	struct symbol *sp, *findnam();

	if ((c = nb()) == '(') {
		n = expr();
		if (nb() != ')')
			putchr('?');
		return(n);
	} else if (c == '\'') {
		n = 0;
		while ((c = getchr()) != '\'')
			if (c == '\n') {
				pushc(c);
				break;
			} else
				n = (n << 8) | c;
		return(n);
	} else if (c == '.' || (type(c) & ALPHAONLY)) {
		cp = buf;
		*cp++ = c;
		if (c == '.')
			if (type(pushc(getchr())) == SPEC)
				return(dot);
		while (type(c = getchr()) & ALPHANUM)
			*cp++ = c;
		*cp = '\0';
		pushc(c);
		if (sp = findnam(buf, binary.symptr))
			return(sp->value);
		else if (sp = findnam(buf, core.symptr))
			return(sp->value);
		pushc('@');
		return(0);
	}
	n = 0;
	base = ibase;
	if (c == '0') {
		base = 8;
		switch (pushc(getchr())) {

		case 'x':
			base += 6;
		case 't':
			base += 2;
		case 'o':
			getchr();
			c = getchr();
			break;
		default:
			base = ibase;
			break;

		}
	}
	while (type(c) & HEXDIG) {
		if (c >= 'a' && c <= 'f')
			c -= 'a' - '9' - 1;
		if (c >= 'A' && c <= 'F')
			c -= 'A' - '9' - 1;
		n = (n * base) + (c - '0');
		c = getchr();
	}
	pushc(c);
	return(n);
}
null(c, fmt, get)
int c;
char *fmt;
long (*get)();
{

	printf("?\n");
	return(0);
}
cmdcol(c, fmt, get)
int c;
char *fmt;
long (*get)();
{

	printf("?\n");
	return(0);
}
cmddol(c, fmt, get)
int c;
char *fmt;
long (*get)();
{

	switch (c = nb()) {

	case 'm':
	case 'M':
		prtmap('?', &binary);
		prtmap('/', &core);
		break;
	case 'q':
		exit(0);
	case 'd':
		ibase = 10;
		break;
	case 'o':
		ibase = 8;
		break;
	case 'x':
		ibase = 16;
		break;
	case 's':
		maxoff = dot;
		break;
	default:
		pushc(c);
		putchr('?');
		putchr('\n');
		break;

	}
	return(0);
}
prtmap(c, fp)
char c;
struct file *fp;
{

	printf("%c\t%s\n", c, fp->name);
	putx(fp->b1, 10);
	putx(fp->e1, 10);
	putx(fp->f1, 10);
	putchr('\n');
	putx(fp->b2, 10);
	putx(fp->e2, 10);
	putx(fp->f2, 10);
	putchr('\n');
	return;
}
cmdwrt(c, fmt, get)
char c;
char *fmt;
long (*get)();
{
	long l;
	struct file *fp;

	if (mode != 2) {
		prt("not is write mode\n");
		return;
	}
	fp = (c == '?') ? &binary : &core;
	c = nb();
	l = expr();
	putn(l, dot + dotoff, c == 'w' ? 2 : 4, fp);
	return;
}
cmdprt(c, fmt, get)
int c;
char *fmt;
long (*get)();
{
	register int c1;
	long *ip;
	struct file *fp;
	struct symbol *sp, *findsym();

	fp = (c == '?') ? &binary : &core;
	while (c = *fmt++)
		switch (c) {

		case 'a':
		case 'p':
			if ((sp = findsym((int)(dot + dotoff), binary.symptr)) ||
				(sp = findsym((int)(dot + dotoff), core.symptr)))
					prtsym((int)(dot + dotoff), sp);
				else
					putx(dot + dotoff, 8);
			if (c == 'p')
				break;
			putchr(':');
			putchr('\t');
			break;
		case 'i':
			if (get == getdot) {
				putchr('?');
				break;
			}
			puti(fp);
			if (*fmt)
				putchr('\n');
			break;
		case 'm':
		case 'M':
			if (get == getdot) {
				putchr('?');
				break;
			}
			ip = c == 'm' ? fp->tmap : fp->dmap;
			for (c1 = 0; c1 < 3; c1++) {
				if (peekc() == '\n')
					break;
				*ip++ = expr();
			}
			return;
		case 'o':
			puto((*get)(dot + dotoff, 2, fp) & 0xffff, 6);
			break;
		case 'O':
			puto((*get)(dot + dotoff, 4, fp), 12);
			break;
		case 'd':
			putd((*get)(dot + dotoff, 2, fp), 6);
			break;
		case 'D':
			putd((*get)(dot + dotoff, 4, fp), 11);
			break;
		case 'x':
			putx((*get)(dot + dotoff, 2, fp) & 0xffff, 4);
			break;
		case 'X':
			putx((*get)(dot + dotoff, 4, fp), 8);
			break;
		case 'b':
			puto((*get)(dot + dotoff, 1, fp) & 0xff, 4);
			break;
		case 'c':
			putchr((char)(*get)(dot + dotoff, 1, fp));
			break;
		case 'S':
		case 's':
			while (c1 = (char)(*get)(dot + dotoff, 1, fp)) {
				if ((c1 < ' ' || c1 > 127) && (c == 'S'))
					c1 = '.';
				putchr(c1);
			}
			break;
		case '"':
			while ((c = *fmt++) != '"' && c)
				putchr(c);
			if (c != '"')
				fmt--;
			break;
		default:
			putchr(c);
			break;

		}
	putchr('\n');
	return;
}
getfmt(fmt)
char *fmt;
{
	char c;

	if ((c = peekc()) == 'm' || c == 'M' || c == '*') {
		nb();
		if (c == '*')
			if ((c = nb()) != 'm' && c != 'M') {
				pushc(c);
				c = '?';
			} else
				c = 'M';
		else
			c = 'm';
		*fmt++ = c;
		*fmt++ = '\0';
		return;
	}
	while ((*fmt = getchr()) != '\n')
		fmt++;
	*fmt = '\0';
	pushc('\n');
	return;
}
	long
getdot(d, n, fp)
long d;
int n;
struct file *fp;
{
	long l;

	l = dot;
	if (n == 2)
		l &= 0xffff;
	else if (n == 1)
		l &= 0xff;
	return(l);
}
	unsigned char
getb(fp)
struct file *fp;
{

	return(getn(dot + dotoff, 1, fp));
}
	unsigned int
getw(fp)
struct file *fp;
{

	return(getn(dot + dotoff, 2, fp));
}
putn(v, d, n, fp)
long v, d;
int n;
struct file *fp;
{
	long b, no;
	register int o;
	char *p;

	if (d >= fp->b1 && d < fp->e1)
		d += fp->f1 - fp->b1;
	else if (d >= fp->b2 && d < fp->e2)
		d += fp->f2 - fp->b2;
	b = d >> LOGBS;
	o = d & (BSIZE - 1);
	if (buffer) {
		if (fp->cblock != b) {
			fp->cblock = b;
			lseek(fp->fid, b << LOGBS, 0);
			read(fp->fid, fp->buf, sizeof(fp->buf));
		}
	} else {
		lseek(fp->fid, d, 0);
		read(fp->fid, &fp->buf[o], n);
	}
	p = &fp->buf[o];
	dotoff += n;
	switch (n) {

	case 2:
		putx(((long)*(int *)p) & 0xffff, 4);
		prt(" =");
		putx(v & 0xffff, 4);
		putchr('\n');
		*(int *)p = v;
		break;
	case 4:
		putx(*(long *)p, 8);
		prt(" =");
		putx(v, 8);
		putchr('\n');
		*(long *)p = v;
		break;

	}
	if (buffer) {
		lseek(fp->fid, b << LOGBS, 0);
		write(fp->fid, fp->buf, sizeof(fp->buf));
	} else {
		lseek(fp->fid, d, 0);
		write(fp->fid, &fp->buf[o], n);
	}
	return;
}
	long
getn(d, n, fp)
long d;
int n;
struct file *fp;
{
	long b, no;
	register int o;
	char *p;

	if (d >= fp->b1 && d < fp->e1)
		d += fp->f1 - fp->b1;
	else if (d >= fp->b2 && d < fp->e2)
		d += fp->f2 - fp->b2;
	b = d >> LOGBS;
	o = d & (BSIZE - 1);
	if (buffer) {
		if (fp->cblock != b) {
			fp->cblock = b;
			lseek(fp->fid, b << LOGBS, 0);
			read(fp->fid, fp->buf, sizeof(fp->buf));
		}
	} else {
		lseek(fp->fid, d, 0);
		read(fp->fid, &fp->buf[o], n);
	}
	p = &fp->buf[o];
	dotoff += n;
	switch (n) {

	case 1:
		no = *p;
		break;
	case 2:
		no = *(int *)p;
		break;
	case 4:
		no = *(long *)p;
		break;

	}
	return(no);
}
puto(n, s)
unsigned long n;
int s;
{

	if (n > 0)
		puto((n >> 3) & 0x1fffffff, --s);
	else
		while (s-- > 0)
			putchr(' ');
	putchr((char)((n & 7) + '0'));
	return;
}
putd(n, s)
long n;
int s;
{

	if (n < 0) {
		s--;
		n = -n;
		if (n < 0) {
			while (s-- > 0)
				putchr(' ');
			putchr('?');
			return;
		} else
			sign = '-';
	}
	if (n > 9)
		putd(n / 10, --s);
	else
		while (s-- > 0)
			putchr(' ');
	if (sign) {
		putchr(sign);
		sign = 0;
	}
	putchr((char)((n % 10) + '0'));
	return;
}
putx(n, s)
unsigned long n;
int s;
{

	if (n > 0xf)
		putx((n >> 4) & 0xfffffff, --s);
	else
		while (s-- > 0)
			putchr(' ');
	putchr("0123456789abcdef"[n & 0xf]);
	return;
}
prt(s)
char *s;
{

	while (*s)
		putchr(*s++);
	return;
}
putchr(c)
char c;
{

	*bufp++ = c;
	if (c == '\n' || bufp > &lbuf[70]) {
		write(1, lbuf, bufp - lbuf);
		bufp = lbuf;
	}
	return(c);
}
setsym(mn)
char *mn;
{
	FILE *fd;
	char buf[80];

	if ((fd = fopen(mn, "r")) == NULL) {
		printf("%s:cannot open\n", mn);
		return;
	}
	while (fgets(buf, sizeof(buf), fd))
		switch(buf[0]) {

		case 'T':
			addsym(&binary.symptr, buf);
			break;
		case 'A':
		case 'D':
		case 'B':
			addsym(&core.symptr, buf);
			break;

		}
	return;
}
addsym(sp, cp)
struct symbol *sp;
char *cp;
{
	unsigned int v;
	char *cptr;
	struct symbol *p;

	v = htoi(cp + 2);
	while (sp->next)
		if (sp->next->value > v)
			break;
		else
			sp = sp->next;
	p = (struct symbol *)malloc(sizeof(*p));
	p->next = sp->next;
	sp->next = p;
	p->value = v;
	cp += 7;
	cptr = p->name;
	while ((type(*cp) & ALPHANUM) || *cp == '.')
		*cptr++ = *cp++;
	*cptr = '\0';
	return;
}
	struct symbol *
findnam(cp, sp)
char *cp;
struct symbol *sp;
{

	while (sp)
		if (strcmp(cp, sp->name) == 0)
			return(sp);
		else
			sp = sp->next;
	return(0);
}
	struct symbol *
findsym(v, sp)
unsigned int v;
struct symbol *sp;
{
	struct symbol *lp;

	lp = sp;
	while (sp)
		if (sp->value > v)
			break;
		else {
			lp = sp;
			sp = sp->next;
		}
	if (lp && v >= lp->value && v < lp->value + maxoff)
		return(lp);
	return(0);
}
htoi(cp)
char *cp;
{
	int n;
	char c;

	n = 0;
	while (type(c = *cp++) & HEXDIG) {
		if (c >= 'a' && c <= 'f')
			c -= 'a' - '9' - 1;
		else if (c >= 'A' && c <= 'F')
			c -= 'A' - '9' - 1;
		n = (n << 4) + (c - '0');
	}
	return(n);
}
prtsym(v, sp)
unsigned int v;
struct symbol *sp;
{

	prt(sp->name);
	if (v != sp->value) {
		putchr('+');
		putx((long)(v - sp->value), 0);
	}
	return;
}
