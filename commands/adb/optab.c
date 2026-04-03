#include "adb.h"

int i_normal(), i_incdec(), i_ill(), i_jmp(), i_immed();
int i_mov(), i_movi(), i_movs();
int i_misc1(), i_misc2(), i_misc3();
int i_string(), i_xchg(), i_shift(), i_esc();
int i_ioi(), i_ior(), i_monad(), i_misc4();

extern long dot;
extern int dotoff;
extern struct file binary;
extern struct file core;

struct symbol *findsym();
long getn();
unsigned char getb();
unsigned int getw();

char *mon0[] = { "add",	"push\tes",	"pop\tes" };
char *mon1[] = { "or",	"push\tcs",	"???" };
char *mon2[] = { "adc",	"push\tss",	"pop\tss" };
char *mon3[] = { "sbb",	"push\tds",	"pop\tds" };
char *mon4[] = { "and",	"es:",		"daa" };
char *mon5[] = { "sub",	"cs:",		"das" };
char *mon6[] = { "xor",	"ss:",		"aaa" };
char *mon7[] = { "cmp",	"ds:",		"aas" };
char *mon8[] = {
	"add",	"or",	"adc",	"sbb",
	"and",	"sub",	"xor",	"cmp"
};
char *mon9[] = {
	"jo",	"jno",	"jb",	"jnb",
	"je",	"jne",	"jna",	"ja"
};
char *mon10[] = {
	"js",	"jns",	"jp",	"jnp",
	"jl",	"jnl",	"jng",	"jg"
};
char *mon11[] = {
	"cbw",	"cwd",	"call",	"wait",
	"pushf",	"popf",	"sahf",	"lahf"
	};
char *mon12[] = {
	"mov",	"mov",	"mov",	"mov",
	"movs",	"movs",	"cmps",	"cmps"
	};
char *mon13[] = {
	"test\tal",	"test\tax",	"stosb",	"stosw",
	"lodsb",	"lodsw",	"scasb",	"scasw"
};
char *mon14[] = {
	"rol",	"ror",	"rcl",	"rcr",
	"shl",	"shr",	"???",	"sar"
};
char *mon15[] = {
	"loopne\t",	"loope\t",	"loop\t",	"jcxz\t",
	"in\tal",	"in\tax",	"out\tal",	"out\tax"
};
char *mon16[] = {
	"lock",	"???",	"repne",	"rep",
	"hlt",	"cmd"
};
char *mon17[] = {
	"clc",	"stc",	"cli",	"sti",
	"cld",	"std"
};
char *mon18[] = {
	"test",	"???",	"not",	"neg",
	"mul",	"imul",	"div",	"idiv"
};
char *mon19[] = {
	"incw",	"decw",	"call",	"call",
	"jmp",	"jmpf",	"push"
};

char *reg8[] = {
	"al",	"cl",	"dl",	"bl",
	"ah",	"ch",	"dh",	"bh"
};

char *reg16[] = {
	"ax",	"cx",	"dx",	"bx",
	"sp",	"bp",	"si",	"di"
};

char *regsg[] = { "es",	"cs",	"ss",	"ds" };

char *regi[] = {
	"[bx+si]",	"[bx+di]",	"[bp+si]",	"[bp+di]",
	"[si]",		"[di]",		"[bp]",		"[bx]"
};

struct optable {
	char	**numon;
	int	(*proc)();
} optable[] = {
	mon0,	i_normal,				/* 00000xxx */
	mon1,	i_normal,				/* 00001xxx */
	mon2,	i_normal,				/* 00010xxx */
	mon3,	i_normal,				/* 00011xxx */
	mon4,	i_normal,				/* 00100xxx */
	mon5,	i_normal,				/* 00101xxx */
	mon6,	i_normal,				/* 00110xxx */
	mon7,	i_normal,				/* 00111xxx */
	(char **)"inc",	i_incdec,		/* 01000xxx */
	(char **)"dec",	i_incdec,		/* 01001xxx */
	(char **)"push",	i_incdec,	/* 01010xxx */
	(char **)"pop",	i_incdec,		/* 01011xxx */
	0,	i_ill,						/* 01100xxx */
	0,	i_ill,						/* 01101xxx */
	mon9,	i_jmp,					/* 01110xxx */
	mon10,	i_jmp,					/* 01111xxx */
	0,	i_immed,					/* 10000xxx */
	0,	i_mov,						/* 10001xxx */
	0,	i_xchg,						/* 10010xxx */
	mon11,	i_misc1,				/* 10011xxx */
	mon12,	i_movs,					/* 10100xxx */
	mon13,	i_string,				/* 10101xxx */
	reg8,	i_movi,					/* 10110xxx */
	reg16,	i_movi,					/* 10111xxx */
	0,	i_misc2,					/* 11000xxx */
	0,	i_misc3,					/* 11001xxx */
	0,	i_shift,					/* 11010xxx */
	0,	i_esc,						/* 11011xxx */
	0,	i_ioi,						/* 11100xxx */
	0,	i_ior,						/* 11101xxx */
	0,	i_monad,					/* 11110xxx */
	0,	i_misc4						/* 11111xxx */
};

puti(fp)
struct file *fp;
{
	int opcode;
	struct optable *op;

	opcode = (int)getb(fp) & 0xff;
	op = &optable[opcode >> 3];
	(*(op->proc))(opcode, op->numon, fp);
	return;
}
i_ill(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{

	putchr('?');
	putchr('?');
	putchr('?');
	return;
}

i_incdec(opc, mon, fp)
int opc;
char *mon;
struct file *fp;
{

	prt(mon);
	putchr('\t');
	prt(reg16[opc & 7]);
	return;
}
i_normal(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	int mod;

	switch (opc & 7) {

	case 0:
	case 1:
	case 2:
	case 3:
		mod = getb(fp);
		prt(mon[0]);
		putchr('\t');
		putad(opc, mod, fp);
		break;
	case 4:
		prt(mon[0]);
		prt("\tal,");
		putx((long)getb(fp) & 0xff, 0);
		break;
	case 5:
		prt(mon[0]);
		prt("\tax,");
		putx((long)getw(fp), 0);
		break;
	case 6:
	case 7:
		prt(mon[(opc & 7) - 5]);
		break;

	}
	return;
}
putad(opc, mod, fp)
int opc, mod;
struct file *fp;
{

	switch (opc & 3) {

	case 0:
		putea(mod, reg8, fp);
		putchr(',');
		prt(reg8[(mod >> 3) & 7]);
		break;
	case 1:
		putea(mod, reg16, fp);
		putchr(',');
		prt(reg16[(mod >> 3) & 7]);
		break;
	case 2:
		prt(reg8[(mod >> 3) & 7]);
		putchr(',');
		putea(mod, reg8, fp);
		break;
	case 3:
		prt(reg16[(mod >> 3) & 7]);
		putchr(',');
		putea(mod, reg16, fp);
		break;

	}
	return;
}
putea(mod, rp, fp)
int mod;
char *rp[];
struct file *fp;
{
	int type, reg;
	long l;
	struct symbol *sp;

	type = mod & 0xc0;
	reg = mod & 7;
	if (type == 0xc0) {
		prt(rp[reg]);
		return;
	}
	if (type == 0x00 && reg == 6) {
		l = getw(fp);
		l &= 0xffff;
		if (sp = findsym((int)l, core.symptr))
			prtsym((int)l, sp);
		else {
			putchr('[');
			putx(l, 0);
			putchr(']');
		}
		return;
	}
	if (type == 0x40)
		putx((long)getb(fp) & 0xff, 0);
	else if (type == 0x80)
		putx((long)getw(fp), 0);
	prt(regi[reg]);
	return;
}
i_immed(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	int mod, reg;

	mod = getb(fp);
	reg = (mod >> 3) & 7;
	mon = opc & 1 ? reg16 : reg8;
	if (opc & 4) {
		prt(opc & 2 ? "xchg\t" : "test\t");
		putad(opc, mod, fp);
		return;
	}
	prt(mon8[reg]);
	putchr('\t');
	putea(mod, mon, fp);
	putchr(',');
	if ((opc & 3) == 1)
		putx((long)getw(fp), 0);
	else
		putx((long)getb(fp) & 0xff, 0);
	return;
}
i_jmp(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	unsigned int pc;
	struct symbol *sp;

	pc = getb(fp);
	pc += dot + dotoff;
	prt(mon[opc & 7]);
	putchr('\t');
	if (sp = findsym(pc, binary.symptr))
		prtsym(pc, sp);
	else
		putx((long)(pc), 0);
	return;
}
i_mov(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	int mod;

	mod = getb(fp);
	mon = opc & 1 ? reg16 : reg8;
	switch (opc & 7) {

	case 0:
	case 1:
	case 2:
	case 3:
		prt("mov\t");
		putad(opc, mod, fp);
		break;
	case 4:
		if (mod & 0x20) {
			prt("???");
			break;
		}
		prt("mov\t");
		putea(mod, reg16, fp);
		putchr(',');
		prt(regsg[(mod >> 3) & 3]);
		break;
	case 5:
		prt("lea\t");
		putad(opc, mod, fp);
		break;
	case 6:
		if (mod & 0x20) {
			prt("???");
			break;
		}
		prt("mov\t");
		prt(regsg[(mod >> 3) & 3]);
		putchr(',');
		putea(mod, reg16, fp);
		break;
	case 7:
		if (mod & 0x38) {
			prt("???");
			break;
		}
		prt("pop\t");
		putea(mod, reg16, fp);
		break;

	}
	return;
}
i_xchg(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{

	if ((opc & 7) == 0)
		prt("nop");
	else {
		prt("xchg\tax,");
		prt(reg16[opc & 7]);
	}
	return;
}
i_misc1(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	long pc;
	struct symbol *sp;

	prt(mon[opc & 7]);
	if ((opc & 7) == 2) {
		putchr('\t');
		pc = getw(fp);
		putx((long)getw(fp), 0);
		putchr(':');
		if (sp = findsym((int)pc, binary.symptr))
			prtsym((int)pc, sp);
		else
			putx(pc, 0);
	}
	return;
}
i_misc2(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	int mod, reg;

	if ((opc & 7) >= 4) {
		mod = getb(fp);
		reg = (mod >> 3) & 7;
	}
	switch (opc & 7) {

	case 2:
		prt("ret\t");
		putx((long)getw(fp), 0);
		break;
	case 3:
		prt("ret");
		break;
	case 4:
		prt("lea\t");
		prt(reg16[reg]);
		putchr(',');
		putea(mod, reg16, fp);
		break;
	case 5:
		prt("lds\t");
		prt(reg16[reg]);
		putchr(',');
		putea(mod, reg16, fp);
		break;
	case 6:
		prt("movb\t");
		putea(mod, reg8, fp);
		putchr(',');
		putx((long)getb(fp) & 0xff, 0);
		break;
	case 7:
		prt("movw\t");
		putea(mod, reg16, fp);
		putchr(',');
		putx((long)getw(fp), 0);
		break;
	default:
		prt("???");
		break;

	}
	return;
}
i_misc3(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	int sub;
	long l;

	sub = opc & 7;
	if (sub == 2)
		l = getw(fp);
	else if (sub == 5)
		l = (long)getb(fp) & 0xff;
	switch (sub) {

	case 2:
		prt("ret\t");
		putx(l, 0);
		break;
	case 3:
		prt("ret");
		break;
	case 4:
		prt("int\t3");
		break;
	case 5:
		prt("int\t");
		putx(l, 0);
		break;
	case 6:
		prt("into");
		break;
	case 7:
		prt("iret");
		break;
	default:
		prt("???");
		break;

	}
	return;
}
i_movs(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	int sub;
	long l;

	sub = opc & 7;
	prt(mon[sub]);
	putchr('\t');
	if (sub < 5)
		l = getw(fp);
	switch (sub) {

	case 0:
		prt("al,");
		putx(l, 0);
		break;
	case 1:
		prt("ax,");
		putx(l, 0);
		break;
	case 2:
		putx(l, 0);
		prt(",al");
		break;
	case 3:
		putx(l, 0);
		prt(",ax");
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		break;

	}
	return;
}
i_string(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	int sub;

	sub = opc & 7;
	prt(mon[sub]);
	if (sub == 0)
		putx((long)getb(fp) & 0xff, 0);
	else
		putx((long)getw(fp), 0);
	return;
}
i_movi(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	long l;

	if (opc & 8)
		l = getw(fp);
	else
		l = getb(fp) & 0xff;
	prt("mov\t");
	prt(mon[opc & 7]);
	putchr(',');
	putx(l, 0);
	return;
}
i_shift(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	int mod, reg;
	char *count;

	switch (opc & 7) {

	case 4:
		mod = getb(fp);
		prt(mod == 0x0a ? "aam" : "???");
		return;
	case 5:
		mod = getb(fp);
		prt(mod == 0x0a ? "aad" : "???");
		return;
	case 6:
		prt("???");
		return;
	case 7:
		prt("xlat");
		return;

	}
	mod = getb(fp);
	reg = (mod >> 3) & 7;
	mon = opc & 1 ? reg16 : reg8;
	count = opc & 2 ? ",cl" : ",1";
	if (reg == 6) {
		prt("???");
		return;
	}
	prt(mon14[reg]);
	putchr('\t');
	putea(mod, mon, fp);
	prt(count);
	return;
}
i_esc(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	int mod, code;

	mod = getb(fp);
	code = ((opc & 7) << 3) | ((mod >> 3) & 7);
	prt("esc\t");
	putx((long)code, 0);
	putchr(',');
	putea(mod, reg8, fp);
	return;
}
i_ioi(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	int sub;
	long pc;
	struct symbol *sp;

	pc = getb(fp);
	sub = opc & 7;
	prt(mon15[sub]);
	putchr(',');
	if (sub < 4)
		pc += dot + dotoff;
	else
		pc &= 0xff;
	if (sp = findsym((int)pc, binary.symptr))
		prtsym((int)pc, sp);
	else
		putx(pc, 0);
	return;
}
i_ior(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	long pc;
	struct symbol *sp;

	switch (opc & 7) {

	case 0:
		prt("call\t");
		pc = getn(dot + dotoff, 2, fp);
		pc += dot + dotoff;
		break;
	case 1:
		prt("jmp\t");
		pc = getn(dot + dotoff, 2, fp);
		pc += dot + dotoff;
		break;
	case 2:
		prt("jmp\t");
		pc = getw(fp);
		putx((long)getw(fp), 0);
		putchr(':');
		break;
	case 3:
		prt("jmp\t");
		pc = getn(dot + dotoff, 1, fp);
		pc += dot + dotoff;
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		prt(mon15[opc & 7]);
		prt(",dx");
		return;

	}
	if (sp = findsym((int)pc, binary.symptr))
		prtsym((int)pc, sp);
	else
		putx(pc, 0);
	return;
}
i_monad(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	int mod, sub;

	sub = opc & 7;
	switch (sub) {

	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		prt(mon16[sub]);
		break;
	case 6:
	case 7:
		mon = sub == 7 ? reg16 : reg8;
		mod = getb(fp);
		sub = (mod >> 3) & 7;
		if (sub == 1) {
			prt("???");
			break;
		}
		prt(mon18[sub]);
		putchr('\t');
		putea(mod, mon, fp);
		if (sub == 0) {
			putchr(',');
			if (opc & 1)
				putx((long)getw(fp), 0);
			else
				putx((long)getb(fp) & 0xff, 0);
		}
		break;

	}
	return;
}
i_misc4(opc, mon, fp)
int opc;
char *mon[];
struct file *fp;
{
	int mod, sub;

	sub = opc & 7;
	switch (sub) {

	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		prt(mon17[sub]);
		break;
	case 6:
		mod = getb(fp);
		sub = (mod >> 3) & 7;
		if (sub == 0) {
			prt("incb\t");
			putea(mod, reg8, fp);
		} else if (sub == 1) {
			prt("decb\t");
			putea(mod, reg8, fp);
		} else
			prt("???");
		break;
	case 7:
		mod = getb(fp);
		sub = (mod >> 3) & 7;
		if (sub == 7)
			prt("???");
		else {
			prt(mon19[sub]);
			putchr('\t');
			putea(mod, reg16, fp);
		}
		break;

	}
	return;
}


