/* fdisk - partition a hard disk	Author: Jakob Schripsema */

/* First run the DOS utilities FDISK and FORMAT.
 * FDISK puts the boot code in sector 0.
 * Then run this fdisk:
 *
 *	fdisk [-hheads] [-ssectors] [device]
 *
 * e.g.,
 *
 *	fdisk -h4 -s17 /dev/hd0		(MINIX default)
 *	fdisk -h4 -s17 c:		(DOS default)
 *	fdisk -h6 -s25 /dev/hd5		(second drive, probably RLL)
 *	fdisk junkfile			(to expermiment safely)
 *
 * Compiling
 *
 *	cc -o fdisk -DUNIX fdisk.c	(MINIX)
 *	cl -DDOS fdisk.c		(DOS with MS C compiler)
 */

#include <minix/partition.h>
#include <stdio.h>
#include <string.h>
#define UNIX			/* for MINIX */

#ifdef DOS
#include <dos.h>
#define DEFAULT_DEV	"c:"
#define LOAD_OPEN_MODE	0x8000
#define SAVE_CREAT_MODE	0644
#else
#define DEFAULT_DEV	"/dev/hd0"
#define LOAD_OPEN_MODE	0
#define SAVE_CREAT_MODE	0644
#endif

/* Constants */

#define	DEFAULT_NHEAD	4	/* # heads		 */
#define	DEFAULT_NSEC	17	/* sectors / track	 */
#define SECSIZE		512	/* sector size		 */
#define	OK		0
#define	ERR		1

#define CYL_MASK	0xc0	/* mask to extract cyl bits from sec field */
#define CYL_SHIFT	2	/* shift to extract cyl bits from sec field */
#define SEC_MASK	0x3f	/* mask to extract sec bits from sec field */
#define NOT_AN_INT	-32767

/* Globals  */
char secbuf[SECSIZE];
char *devname;
int nhead;
int nsec;
int readonly;

static void adj_base();
static void adj_size();
static struct part_entry *ask_partition();
static int get_an_int();
static void list_part_types();
static void mark_npartition();
static void mygets();
static char *systype();
static void toggle_active();
static void usage();

main(argc, argv)
int argc;
char *argv[];
{
  int argn;
  char *argp;
  int ch;

  /* Init */

  nhead = DEFAULT_NHEAD;
  nsec = DEFAULT_NSEC;
  for (argn = 1; argn < argc && (argp = argv[argn])[0] == '-'; ++argn) {
	if (argp[1] == 'h')
		nhead = atoi(argp + 2);
	else
		if (argp[1] == 's') nsec = atoi(argp + 2);
	else
		usage();
  }

  if (argn == argc)
	devname = DEFAULT_DEV;
  else if (argn == argc - 1)
	devname = argv[argn];
  else
	usage();

  getboot(secbuf);
  chk_table();

  do {
	putchar('\n');
	dpl_partitions(0);
	printf("\n(Enter 'h' for help) ");
	ch = get_a_char();
	putchar('\n');
	switch (ch) {
	    case 'h':	print_menu();			break;
	    case 'c':	change_partition(ask_partition());	break;
	    case 'a':	toggle_active(ask_partition());	break;
	    case 'm':	mark_partition(ask_partition());	break;
	    case 'n':	mark_npartition(ask_partition());	break;
	    case 'v':	chk_table();			break;
	    case 'B':	adj_base(ask_partition()); 	break;
	    case 'S':	adj_size(ask_partition()); 	break;
	    case 'w':
		if (readonly)
			printf("Write disabled\n");
		else if(chk_table() == OK) {
			putboot(secbuf);
			printf(
	"Partition table has been updated and the file system synced.\n");
			printf("Please reboot now.\n");
			exit(0);
		} else
			printf("Not written\n");
		break;
	    case 'l':	load_from_file();	  	break;
	    case 's':	save_to_file();	  		break;
	    case 'p':	dpl_partitions(1);  		break;
	    case 't':	list_part_types();	 	break;
	    case 'q':	exit(0);
	    default:	printf(" %c ????\n", ch);
  	}
  }
  while (1);
}


#ifdef UNIX

static int devfd;

getboot(buffer)
char *buffer;
{
  devfd = open(devname, 2);
  if (devfd < 0) {
	printf("No write permission on %s\n", devname);
	readonly = 1;
	devfd = open(devname, 0);
  }
  if (devfd < 0) {
	printf("Cannot open device %s\n", devname);
	exit(1);
  }
  if (read(devfd, buffer, SECSIZE) != SECSIZE) {
	printf("Cannot read boot sector\n");
	exit(2);
  }
}

putboot(buffer)
char *buffer;
{
  if (lseek(devfd, 0L, 0) < 0) {
	printf("Seek error during write\n");
	exit(1);
  }
  if (write(devfd, buffer, SECSIZE) != SECSIZE) {
	printf("Write error\n");
	exit(1);
  }
  sync();
}

#endif


load_from_file()
{
/* Load buffer from file  */

  char file[80];
  int fd;

  printf("Enter file name: ");
  mygets(file, sizeof file);
  fd = open(file, LOAD_OPEN_MODE);
  if (fd < 0) {
	fprintf(stderr, "Cannot load from %s\n", file);
	return;
  }
  if (read(fd, secbuf, SECSIZE) != SECSIZE) {
	fprintf(stderr, "Read error\n");
	exit(1);
  }
  close(fd);
  chk_table();
}


save_to_file()
{
/* Save to file  */

  char file[80];
  int fd;

  printf("Enter file name: ");
  mygets(file, sizeof file);
  if(chk_table() != OK) printf("Saving anyway\n");
  fd = creat(file, SAVE_CREAT_MODE);
#ifdef DOS
  if (fd < 0) {
	fprintf(stderr, "Cannot creat %s\n", file);
	return;
  }
  close(fd);
  fd = open(file, 0x8001);
#endif
  if (fd < 0)
	fprintf(stderr, "Cannot save to %s\n", file);
  else if (write(fd, secbuf, SECSIZE) != SECSIZE)
	fprintf(stderr, "Write error\n");
  close(fd);
}


dpl_partitions(rawflag)
int rawflag;
{
/* Display partition table */

  char active[5];
  int cyl_mask;
  char *format;
  int i;
  struct part_entry *pe;
  int sec_mask;
  char type[8];

  if (rawflag) {
	cyl_mask = 0;		/* no contribution of cyl to sec */
	sec_mask = 0xff;
        format =
	"%3d   %-7s   x%02x %3d  x%02x   x%02x %3d  x%02x   %4s %7ld %7ld%s\n";
  } else {
	cyl_mask = CYL_MASK;
	sec_mask = SEC_MASK;
	format = "%3d   %-7s  %4d %3d %3d   %4d %3d  %3d   %4s %7ld %7ld%s\n";
  }
  printf(
    "               ----first----  -----last----         ----sectors---\n");
  printf(
    "Part.  Type     Cyl Head Sec   Cyl Head Sec  Active   Base    Size\n");
  pe = (struct part_entry *) &secbuf[PART_TABLE_OFF];
  for (i = 1; i <= NR_PARTITIONS; i++, pe++) {
	if (rawflag) {
		sprintf(type, " 0x%02x", pe->sysind);
		sprintf(active, "0x%02x", pe->bootind);
	}
	printf(format,
		i,
		rawflag ? type : systype(pe->sysind),
		pe->start_cyl + ((pe->start_sec & cyl_mask) << CYL_SHIFT),
		pe->start_head,
		pe->start_sec & sec_mask,
		pe->last_cyl + ((pe->last_sec & cyl_mask) << CYL_SHIFT),
		pe->last_head,
		pe->last_sec & sec_mask,
		rawflag ? active : pe->bootind == ACTIVE_FLAG ? "A " : "  ",
		pe->lowsec,
		pe->size,
		(pe->sysind==OLD_MINIX_PART && pe->lowsec&1) ? "  odd base??" :
		(pe->sysind == MINIX_PART && pe->size & 1) ? "  odd size??" :
		"");
  }
}


chk_table()
{
/* Check partition table */

  int active;
  unsigned char cylinder;
  unsigned char head;
  int i;
  long limit;
  struct part_entry *pe;
  int maxhead;
  int maxsec;
  unsigned char sector;
  int seenpart;
  int status;

  pe = (struct part_entry *) &secbuf[PART_TABLE_OFF];
  limit = 0;
  active = 0;
  maxhead = 0;
  maxsec = 0;
  seenpart = 0;
  status = OK;
  for (i = 1; i <= NR_PARTITIONS; i++, ++pe) {
	if (pe->bootind == ACTIVE_FLAG) active++;
	sec_to_hst(pe->lowsec, &head, &sector, &cylinder);
	if (head != pe->start_head || sector != pe->start_sec ||
	    cylinder != pe->start_cyl) {
		printf("Inconsistent base in partition %d.\n", i);
		printf("Suspect head and sector parameters.\n");
		status = ERR;
	}
	sec_to_hst(pe->lowsec + pe->size - 1, &head, &sector, &cylinder);
	if (head != pe->last_head || sector != pe->last_sec ||
	    cylinder != pe->last_cyl) {
		printf("Inconsistent size in partition %d.\n", i);
		printf("Suspect head and sector parameters.\n");
		status = ERR;
	}
	if (pe->size == 0) continue;
	seenpart = 1;
	if (pe->lowsec <= limit) {
		printf("Overlap between partitions %d and %d\n", i, i - 1);
		status = ERR;
	}
	limit = pe->lowsec + pe->size - 1;
	if (limit < pe->lowsec) {
		printf("Overflow from reposterous size in partition %d.\n", i);
		status = ERR;
	}
	if (maxhead < pe->start_head) maxhead = pe->start_head;
	if (maxhead < pe->last_head) maxhead = pe->last_head;
	if (maxsec < (pe->start_sec & SEC_MASK))
		maxsec = (pe->start_sec & SEC_MASK);
	if (maxsec < (pe->last_sec & SEC_MASK))
		maxsec = (pe->last_sec & SEC_MASK);
  }
  if (seenpart) {
	if (maxhead + 1 != nhead || maxsec != nsec) {
		printf(
	"Disk appears to have mis-specified number of heads or sectors.\n");
		printf("Try  fdisk -h%d -s%d %s  instead of\n",
			maxhead + 1, maxsec, devname);
		printf("     fdisk -h%d -s%d %s\n", nhead, nsec, devname);
		seenpart = 0;
	}
  } else {
	printf(
	"Empty table - skipping test on number of heads and sectors.\n");
	printf("Assuming %d heads and %d sectors.\n", nhead, nsec);
  }
  if (!seenpart) printf("Do not write the table if you are not sure!.\n");
  if (active > 1) {
	printf("%d active partitions\n", active);
	status = ERR;	
  }
  return(status);
}

sec_to_hst(logsec, hd, sec, cyl)
long logsec;
unsigned char *hd, *sec, *cyl;
{
/* Convert a logical sector number to  head / sector / cylinder */

  int bigcyl;

  bigcyl = logsec / (nhead * nsec);
  *sec = (logsec % nsec) + 1 + ((bigcyl >> CYL_SHIFT) & CYL_MASK);
  *cyl = bigcyl;
  *hd = (logsec % (nhead * nsec)) / nsec;
}

mark_partition(pe)
struct part_entry *pe;
{
/* Mark a partition as being of type MINIX. */

  if (pe != NULL) pe->sysind = MINIX_PART;
}

change_partition(entry)
struct part_entry *entry;
{
/* Get partition info : first & last cylinder */

  int first, last;
  long low, high;
  char ch;

  if (entry == NULL) return;
  printf("	Enter first cylinder: ");
  first = get_an_int();
  if (first < 0) return;
  printf("	Enter last cylinder: ");
  last = get_an_int();
  if (last < first) return;
  if (first == 0 && last == 0) {
	entry->bootind = 0;
	entry->start_head = 0;
	entry->start_sec = 0;
	entry->start_cyl = 0;
	entry->sysind = NO_PART;
	entry->last_head = 0;
	entry->last_sec = 0;
	entry->last_cyl = 0;
	entry->lowsec = 0;
	entry->size = 0;
	return;
  }
  low = first & 0xffff;
  low = low * nsec * nhead;
  if (low == 0) low = 1;	/* sec0 is master boot record */
  high = last & 0xffff;
  high = (high + 1) * nsec * nhead - 1;
  entry->lowsec = low;
  entry->size = high - low + 1;
  if (entry->size & 1) {
	/* Adjust size to even since Minix works with blocks of 2 sectors. */
	--high;
	--entry->size;
  }
  sec_to_hst(low, &entry->start_head, &entry->start_sec, &entry->start_cyl);
  sec_to_hst(high, &entry->last_head, &entry->last_sec, &entry->last_cyl);

  /* Accept the MINIX partition type.  Usually ignore foreign types, so this
   * fdisk can be used on foreign partitions.  Don't allow NO_PART, because
   * many DOS fdisks crash on it.
   */
  if (entry->sysind == NO_PART) {
	printf("	Changing partition type from None to MINIX\n");
	entry->sysind = MINIX_PART;
  } else if (entry->sysind == MINIX_PART) {
	printf("	Leaving partition type as MINIX\n");
  } else {
	printf("	Change from %s partition to MINIX partition?? (y/n) ",
		systype(entry->sysind));
	ch = get_a_char();
	if (ch == 'y' || ch == 'Y') entry->sysind = MINIX_PART;
  }

  printf("	Active partition? (y/n) ");
  ch = get_a_char();
  if (ch == 'y' || ch == 'Y')
	entry->bootind = ACTIVE_FLAG;
  else
	entry->bootind = 0;
}

get_a_char()
{
/* Read 1 character and discard rest of line */

  char buf[80];
  int ch;

  mygets(buf, sizeof buf);
  return(*buf & 0xFF);
}

print_menu()
{
  printf("Type a command letter, then a carriage return:\n");
  printf("   c - change a partition\n");
  printf("   B - adjust a base sector\n");
  printf("   S - adjust a size (by changing the last sector)\n");
  printf("   a - toggle an active flag\n");
  printf("   m - mark a partition as a MINIX partition\n");
  printf("   n - mark a partition as a non-MINIX partition (try 't' first)\n");
  printf("   v - verify partition table\n");
  printf("   l - load boot block (including partition table) from a file\n");
  printf("   s - save boot block (including partition table) on a file\n");
  printf("   p - print raw partition table\n");
  printf("   t - print known partition types\n");
 if (readonly)
  printf("   w - write (disabled)\n");
 else
  printf("   w - write changed partition table back to disk and exit\n");
  printf("   q - quit without making any changes\n");
}


/* Here are the DOS routines for reading and writing the boot sector. */

#ifdef DOS

union REGS regs;
struct SREGS sregs;
int drivenum;

getboot(buffer)
char *buffer;
{
/* Read boot sector  */

  segread(&sregs);		/* get ds */

  if (devname[1] != ':') {
	printf("Invalid drive %s\n", devname);
	exit(1);
  }
  if (*devname >= 'a') *devname += 'A' - 'a';
  drivenum = (*devname - 'C') & 0xff;
  if (drivenum < 0 || drivenum > 7) {
	printf("Funny drive number %d\n", drivenum);
	exit(1);
  }
  regs.x.ax = 0x201;		/* read 1 sectors	 */
  regs.h.ch = 0;		/* cylinder		 */
  regs.h.cl = 1;		/* first sector = 1	 */
  regs.h.dh = 0;		/* head = 0		 */
  regs.h.dl = 0x80 + drivenum;	/* drive = 0		 */
  sregs.es = sregs.ds;		/* buffer address	 */
  regs.x.bx = (int) buffer;

  int86x(0x13, &regs, &regs, &sregs);
  if (regs.x.cflag) {
	printf("Cannot read boot sector\n");
	exit(1);
  }
}


putboot(buffer)
char *buffer;
{
/* Write boot sector  */

  regs.x.ax = 0x301;		/* read 1 sectors	 */
  regs.h.ch = 0;		/* cylinder		 */
  regs.h.cl = 1;		/* first sector = 1	 */
  regs.h.dh = 0;		/* head = 0		 */
  regs.h.dl = 0x80 + drivenum;	/* drive = 0		 */
  sregs.es = sregs.ds;		/* buffer address	 */
  regs.x.bx = (int) buffer;

  int86x(0x13, &regs, &regs, &sregs);
  if (regs.x.cflag) {
	printf("Cannot write boot sector\n");
	exit(1);
  }
}

#endif

static void adj_base(pe)
struct part_entry *pe;
{
/* Adjust base sector of partition, usually to make it even. */

  int adj;

  if (pe == NULL) return;
  printf("	Enter adjustment (+-): ");
  adj = get_an_int();
  if (adj == NOT_AN_INT) return;	/* negative may be OK */
  if (pe->lowsec + adj < 2 || pe->size - adj < 2) return;
  pe->lowsec += adj; 
  pe->size -= adj;
  sec_to_hst(pe->lowsec, &pe->start_head, &pe->start_sec, &pe->start_cyl);
}

static void adj_size(pe)
struct part_entry *pe;
{
/* Adjust size of partition by reducing high sector. */

  int adj;

  if (pe == NULL) return;
  printf("	Enter adjustment (+-): ");
  adj = get_an_int();
  if (adj == NOT_AN_INT) return;	/* negative may be OK */
  if (pe->size + adj < 2) return;
  pe->size += adj;
  sec_to_hst(pe->lowsec + pe->size - 1,
	     &pe->last_head, &pe->last_sec, &pe->last_cyl);
}

static struct part_entry *ask_partition()
{
/* Ask for a valid partition number and return its entry. */

  int num;

  printf("Which partition?  ");
  num = get_an_int();
  if (num < 1 || num > NR_PARTITIONS) {
	printf("\nNot valid.  Valid partitions are 1, 2, 3, and 4.\n");
	return(NULL);
  }
  return((struct part_entry *) &secbuf[PART_TABLE_OFF] + num - 1);
}

static int get_an_int()
{
/* Read an int from the start of line of stdin, discard rest of line. */

  char buf[80];
  int num;

  mygets(buf, sizeof buf);
  if ((sscanf(buf, "%d", &num)) != 1) return(NOT_AN_INT);
  return(num);
}

static void list_part_types()
{
/* Print all known partition types. */

  int type;

  for (type = 0; type < 0x100; ++type)
	if (strcmp(systype(type), "Unknown") != 0)
		printf("0x%02x: %-9s\n", type, systype(type));
}

static void mark_npartition(pe)
struct part_entry *pe;
{
/* Mark a partition with arbitrary type. */

  if (pe == NULL) return;
  printf("	Enter partition type: ");
  pe->sysind = get_an_int();
}

static void mygets(buf, length)
char *buf;
int length;			/* as for fgets(), but must be >= 2 */
{
/* Get a non-empty line of maximum length 'length'. */

  while (1) {
	fflush(stdout);
	if (fgets(buf, length, stdin) == NULL)
		strcpy(buf, "h");	/* this shouldn't happen */
	if (strrchr(buf, '\n') != NULL)
		*strrchr(buf, '\n') = 0;
	if (*buf != 0) return;
	printf("(Please type something before the newline) ");
  }
}

static char *systype(type)
int type;
{
/* Convert system indicator into system name. */

  switch(type) {
	case NO_PART: return("None");
	case 1: return("DOS-12");
	case 2: return("XENIX");
	case 4: return("DOS-16");
	case 5: return("DOS-EXT");
	case 6: return("DOS-BIG");
	case 0x64: return("NOVELL");
	case 0x75: return("PCIX");
	case MINIX_PART: return("MINIX");
	case OLD_MINIX_PART: return("MNX-OLD");
	case 0xDB: return("CPM");
	case 0xFF: return("BBT");
	default: return("Unknown");
  }
}

static void toggle_active(pe)
struct part_entry *pe;
{
/* Toggle active flag of a partition. */

  if (pe == NULL) return;
  pe->bootind = (pe->bootind == ACTIVE_FLAG) ? 0 : ACTIVE_FLAG;
}

static void usage()
{
/* Print usage message and exit. */

  printf("Usage: fdisk [-hheads] [-ssectors] [device]\n");
  exit(1);
}
