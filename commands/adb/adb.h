#define NUMERIC		3
#define HEX		7
#define ALPHA		5
#define SPEC		8

#define ALPHANUM	1
#define HEXDIG		2
#define ALPHAONLY	4

#define BSIZE		512
#define LOGBS		9
#define SYMSZ		10

struct symbol {
	struct symbol *	next;
	unsigned int	value;
	char			name[SYMSZ];
};

struct file {
	int	fid;
	char *	name;
	struct symbol *symptr;
	long	cblock;
	long	tmap[3];
	long	dmap[3];
	char	buf[BSIZE + BSIZE];
};

#define b1	tmap[0]
#define e1	tmap[1]
#define f1	tmap[2]
#define b2	dmap[0]
#define e2	dmap[1]
#define f2	dmap[2]

struct fheader {
	long	magic;
	long	flag;
	long	tsize;
	long	dsize;
	long	bsize;
	long	entry;
	long	size;
	long	fill;
};
#define MAGIC	0x04100301L
#define IDMAGIC	0x04200301L
