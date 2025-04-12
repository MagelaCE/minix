/* file: fpp.c
 * EM floating-point-translator for i8088 Minix
 * (EXTREMELY machine and compiler-dependant!)
 * author: Peter S. Housel 6/12/89
 */

#include <stdio.h>
/* #include <stddef.h> */
/* #include <stdlib.h> */

union munge			/* type-punning */
      {
       double dpart;
       float fpart;
       int ipart[4];
      };

extern double atof(/* const char * */);

main(argc, argv)
int argc; char *argv[];
{
 if(++argv, --argc)
    if(strcmp(*argv, "-") != 0)
       if(freopen(*argv, "rb", stdin) == NULL)
	 {
          fprintf(stderr, "fpp: can't open %s for input\n", *argv);
	  exit(1);
	 }

 if(++argv, --argc)
    if(strcmp(*argv, "-") != 0)
       if(freopen(*argv, "wb", stdout) == NULL)
	 {
          fprintf(stderr, "fpp: can't open %s for output\n", *argv);
	  exit(1);
	 }

 if(argc > 1)
   {
    fprintf(stderr, "fpp: usage: fpp [infile] [outfile]\n");
    exit(1);
   }

 filein();

 exit(0);
}

struct instrtab
       {
	char *iname;		/* instruction name */
        char tag; 		/* argument description tag */
       };

struct instrtab itab[] = {			/* EM instructions */
"NON", '-',	/* [Illegal instruction] */
"AAR", 'w',	/* Load address of array element */
"ADF", 'w',	/* Floating add */
"ADI", 'w',	/* Addition */
"ADP", 'f',	/* Add f to pointer on top of stack */
"ADS", 'w',	/* Add w-byte value and pointer */
"ADU", 'w',	/* Addition */
"AND", 'w',	/* Boolean and on two groups of w bytes */
"ASP", 'f',	/* Adjust the stack pointer by f */
"ASS", 'w',	/* Adjust the stack pointer by w-byte integer */
"BEQ", 'b',	/* Branch equal */
"BGE", 'b',	/* Branch greater or equal */
"BGT", 'b',	/* Branch greater */
"BLE", 'b',	/* Branch less or equal */
"BLM", 'z',	/* Block move z bytes; first pop dest. addr, then source */
"BLS", 'w',	/* Block move, size is in w-byte integer on top of stack */
"BLT", 'b',	/* Branch less (pop 2 words, branch if top > second) */
"BNE", 'b',	/* Branch not equal */
"BRA", 'b',	/* Branch unconditionally to label b */
"CAI", '-',	/* Call procedure (procedure identifier on stack) */
"CAL", 'p',	/* Call procedure (with identifier p) */
"CFF", '-',	/* Convert floating to floating */
"CFI", '-',	/* Convert floating to integer */
"CFU", '-',	/* Convert floating to unsigned */
"CIF", '-',	/* Convert integer to floating */
"CII", '-',	/* Convert integer to integer */
"CIU", '-',	/* Convert integer to unsigned */
"CMF", 'w',	/* Compare w byte reals */
"CMI", 'w',	/* Compare w byte int, push neg., zero, pos. for <, = or > */
"CMP", '-',	/* Compare pointers */
"CMS", 'w',	/* Compare w byte values, used only for bit equality test */
"CMU", 'w',	/* Compare w byte unsigneds */
"COM", 'w',	/* Complement (one's complement of top w bytes) */
"CSA", 'w',	/* Case jump; address of jump table at top of stack */
"CSB", 'w',	/* Table lookup jump; address of jump table at top of stack */
"CUF", '-',	/* Convert unsigned to floating */
"CUI", '-',	/* Convert unsigned to integer */
"CUU", '-',	/* Convert unsigned to unsigned */
"DCH", '-',	/* Follow dynamic chain, convert LB to LB of caller */
"DEC", '-',	/* Decrement word on top of stack by 1 */
"DEE", 'g',	/* Decrement external */
"DEL", 'l',	/* Decrement local or parameter */
"DUP", 's',	/* Duplicate top s bytes */
"DUS", 'w',	/* Duplicate top w bytes */
"DVF", 'w',	/* Floating divide */
"DVI", 'w',	/* Division */
"DVU", 'w',	/* Division */
"EXG", 'w',	/* Exchange top w bytes */
"FEF", 'w',	/* Split floating number in exponent and fraction part */
"FIF", 'w',	/* Floating multiply and split integer and fraction part */
"FIL", 'g',	/* File name (external 4 := g) */
"GTO", 'g',	/* Non-local goto, descriptor at g */
"INC", '-',	/* Increment word on top of stack by 1 */
"INE", 'g',	/* Increment external */
"INL", 'l',	/* Increment local or parameter */
"INN", 'w',	/* Bit test on w byte set (bit number on top of stack) */
"IOR", 'w',	/* Boolean inclusive or on two groups of w bytes */
"LAE", 'g',	/* Load address of external */
"LAL", 'l',	/* Load address of local or parameter */
"LAR", 'w',	/* Load array element, desc. contains integers of size w */
"LDC", 'd',	/* Load double constant ( push two words ) */
"LDE", 'g',	/* Load double external (two consec. externals are stacked) */
"LDF", 'f',	/* Load double offsetted (top of stack + f yield address) */
"LDL", 'l',	/* Load double local or parameter (2 consec. words stacked) */
"LFR", 's',	/* Load function result */
"LIL", 'l',	/* Load word pointed to by l-th local or parameter */
"LIM", '-',	/* Load 16 bit ignore mask */
"LIN", 'n',	/* Line number (external 0 := n) */
"LNI", '-',	/* Line number increment */
"LOC", 'c',	/* Load constant (i.e. push one word onto the stack) */
"LOE", 'g',	/* Load external word g */
"LOF", 'f',	/* Load offsetted (top of stack + f yield address) */
"LOI", 'o',	/* Load indirect o bytes (address is popped from the stack) */
"LOL", 'l',	/* Load word at l-th local (l<0) or parameter (l>=0) */
"LOR", 'r',	/* Load register (0=LB, 1=SP, 2=HP) */
"LOS", 'w',	/* Load indirect, w-byte integer on stack top gives size */
"LPB", '-',	/* Convert local base to argument base */
"LPI", 'p',	/* Load procedure identifier */
"LXA", 'n',	/* Load lexical (address of AB n static levels back) */
"LXL", 'n',	/* Load lexical (address of LB n static levels back) */
"MLF", 'w',	/* Floating multiply */
"MLI", 'w',	/* Multiplication */
"MLU", 'w',	/* Multiplication */
"MON", '-',	/* Monitor call */
"NGF", 'w',	/* Floating negate */
"NGI", 'w',	/* Negate (two's complement) */
"NOP", '-',	/* No operation */
"RCK", 'w',	/* Range check; trap on error */
"RET", 'z',	/* Return (function result consists of top z bytes) */
"RMI", 'w',	/* Remainder */
"RMU", 'w',	/* Remainder */
"ROL", 'w',	/* Rotate left a group of w bytes */
"ROR", 'w',	/* Rotate right a group of w bytes */
"RTT", '-',	/* Return from trap */
"SAR", 'w',	/* Store array element */
"SBF", 'w',	/* Floating subtract */
"SBI", 'w',	/* Subtraction */
"SBS", 'w',	/* Subtract pointers in same fragment and push diff as size w integer */
"SBU", 'w',	/* Subtraction */
"SDE", 'g',	/* Store double external */
"SDF", 'f',	/* Store double offsetted */
"SDL", 'l',	/* Store double local or parameter */
"SET", 'w',	/* Create singleton w byte set with bit n on (n is top of stack) */
"SIG", '-',	/* Trap errors to proc identifier on top of stack, -2 resets default */
"SIL", 'l',	/* Store into word pointed to by l-th local or parameter */
"SIM", '-',	/* Store 16 bit ignore mask */
"SLI", 'w',	/* Shift left */
"SLU", 'w',	/* Shift left */
"SRI", 'w',	/* Shift right */
"SRU", 'w',	/* Shift right */
"STE", 'g',	/* Store external */
"STF", 'f',	/* Store offsetted */
"STI", 'o',	/* Store indirect o bytes (pop address, then data) */
"STL", 'l',	/* Store local or parameter */
"STR", 'r',	/* Store register (0=LB, 1=SP, 2=HP) */
"STS", 'w',	/* Store indirect, w-byte integer on top of stack gives object size */
"TEQ", '-',	/* True if equal, i.e. iff top of stack = 0 */
"TGE", '-',	/* True if greater or equal, i.e. iff top of stack >= 0 */
"TGT", '-',	/* True if greater, i.e. iff top of stack > 0 */
"TLE", '-',	/* True if less or equal, i.e. iff top of stack <= 0 */
"TLT", '-',	/* True if less, i.e. iff top of stack < 0 */
"TNE", '-',	/* True if not equal, i.e. iff top of stack non zero */
"TRP", '-',	/* Cause trap to occur (Error number on stack) */
"XOR", 'w',	/* Boolean exclusive or on two groups of w bytes */
"ZEQ", 'b',	/* Branch equal zero */
"ZER", 'w',	/* Load w zero bytes */
"ZGE", 'b',	/* Branch greater or equal zero */
"ZGT", 'b',	/* Branch greater than zero */
"ZLE", 'b',	/* Branch less or equal to zero */
"ZLT", 'b',	/* Branch less than zero (pop 1 word, branch negative) */
"ZNE", 'b',	/* Branch not zero */
"ZRE", 'g',	/* Zero external */
"ZRF", 'w',	/* Load a floating zero of size w */
"ZRL", 'l'	/* Zero local or parameter */
};

struct instrtab ptab[] = {			/* EM pseudo-ops */
"BSS", '3',		/* Reserve storage */
"CON", '*',		/* Initialized constants */
"END", 'B',		/* End of procedure */
"EXA", '1',		/* External name */
"EXC", '2',		/* Exchange instruction blocks */
"EXP", '1',		/* External procedure identifier */
"HOL", '3',		/* Reserve global(?) storage */
"INA", '1',		/* Internal name */
"INP", '1',		/* Internal procedure name */
"MES", '*',		/* Back-end hints */
"PRO", 'A',		/* Start of procedure */
"ROM", '*'		/* Read-only constants */
};

filein()
{
 int ins;

 /* copy magic number */
 if(getchar() != 0255)
   {
    fprintf(stderr, "fpp: bad EM file magic number\n");
    exit(1);
   }
 putchar(0255);
 if(getchar() != 0)
   {
    fprintf(stderr, "fpp: bad EM file magic number\n");
    exit(1);
   }
 putchar(0);

 /* neutral state */
 while((ins = getchar()) != EOF)
      {
       if(0 <= ins && ins < sizeof itab / sizeof (struct instrtab))
         {       /* instruction */
	  putchar(ins);
	  if(itab[ins].tag != '-')
	     doargs(itab[ins].tag);
	 }
       else if(150 <= ins && ins < 150+(sizeof ptab)/sizeof(struct instrtab))
	 {	/* pseudo-op */
	  putchar(ins);
	  doargs(ptab[ins - 150].tag);
	 }
       else if(180 <= ins && ins < 240)
         {	/* instruction label */
	  putchar(ins);
	 }
       else if(240 <= ins && ins < 245)
	 {	/* instruction label */
	  doarg(ins);
	 }
       else
         {
	  putchar(ins);
	  fprintf(stderr, "illegal instruction byte %d\n", ins);
	 }
      } 
}

doargs(tag)
int tag;
{
 int argbyte;

 switch(tag)
       {
	case '3':			/* three args */
		doarg(getchar());
		/* FALLTHRU */
	case '2':			/* two args */
		doarg(getchar());
		/* FALLTHRU */
	case '1':			/* one arg, of one of several types */
	case 'c':
	case 'd':
	case 'l':
	case 'g':
	case 'f':
	case 'n':
	case 's':
	case 'z':
	case 'o':
	case 'p':
	case 'b':
	case 'r':
		doarg(getchar());
		break;

	case 'B':			/* one (optional) arg */
	case 'w':
		if((argbyte = getchar()) != 255)
		   doarg(argbyte);
		else
		   putchar(argbyte);
		break;

	case '*':			/* any number of args */
		while((argbyte = getchar()) != 255)
		      doarg(argbyte);
		putchar(255);
		break;

	case 'A':			/* one arg, one optional arg */
		doarg(getchar());
		if((argbyte = getchar()) != 255)
		  {
		   doarg(argbyte);
		  }
		else
		   putchar(255);
		break;
       }
}

doarg(byte)
int byte;
{
 int b1, b2, b3, b4;
 int length;
 int i;

 if(byte == EOF)
   {
    fprintf(stderr, "Unexpected eof\n");
    exit(1);
   }

 if(byte != 253)		/* special case floating point constant */
    putchar(byte);

 if(0 <= byte && byte < 240)
   {
    /* nothing */
   }
 else
    switch(byte)
	  {
	   case 240:			/* 1-byte instruction label */
		putchar(getchar());
		break;
	   case 241:			/* 2-byte instruction label */
		b1 = getchar(); putchar(b1);
		b2 = getchar(); putchar(b2);
		break;
	   case 242:			/* 1-byte data label */
		putchar(getchar());
		break;
	   case 243:			/* 2-byte data label */
		b1 = getchar(); putchar(b1);
		b2 = getchar(); putchar(b2);
		break;
	   case 244:			/* global symbol */
		length = getcst();
		putcst(length);
		while(length--)
		      putchar(getchar());
		break;
	   case 245:			/* 2-byte integer constant */
		b1 = getchar(); putchar(b1);
		b2 = getchar(); putchar(b2);
		break;
	   case 246:			/* 4-byte integer constant */
		b1 = getchar(); putchar(b1);
		b2 = getchar(); putchar(b2);
		b3 = getchar(); putchar(b3);
		b4 = getchar(); putchar(b4);
		break;
	   case 247:			/* 8-byte integer constant */
		length = 8;
		while(length--)
		      putchar(getchar());
		break;
	   case 248:			/* global label + constant */
		doarg(getchar());	
		putcst(getcst());
		break;
	   case 249:			/* procedure name */
		length = getcst();
		putcst(length);
		while(length--)
		      putchar(getchar());
		break;
	   case 250:			/* string */
		length = getcst();
		putcst(length);
		while(length--)
		      putchar(getchar());
		break;
	   case 251:			/* integer constant */
		putcst(getcst());
		length = getcst();
		putcst(length);
		while(length--)
		      putchar(getchar());
		break;
	   case 252:			/* unsigned constant */
		putcst(getcst());
		length = getcst();
		putcst(length);
		while(length--)
		      putchar(getchar());
		break;
	   case 253:			/* floating constant */
		if((i = getcst()) == 8 || i == 4)
		  {
		   char buf[512];	/* for reading ASCII constant */
		   char *bufp;
		   union munge number;

		   length = getcst();
		   bufp = &buf[0];
		   while(length-- && (bufp - buf) < (sizeof buf) - 1)
		         *bufp++ = getchar();
		   *bufp = '\0';

		   if(i == 8)
		     {
		      number.dpart = atof(buf);	/* convert to a double, */
		      putcst(number.ipart[0]);	/* then write out each */
		      putcst(number.ipart[1]);	/* of the words that makes */
		      putcst(number.ipart[2]);	/* it up as a 16-bit */
		      putcst(number.ipart[3]);	/* integer constant */
		     }
		   else
		     {
		      number.fpart = (float) atof(buf);
		      putcst(number.ipart[0]);
		      putcst(number.ipart[1]);
		     }
		  }
		else
		   fprintf(stderr, "Illegal floating-point constant size\n");
		break;
	   case 254:
	   case 255:
		fprintf(stderr, "Illegal argument type %d\n", byte);
	  }
}

int getcst()
{
 int b1, b2;
 int type;

 type = getchar();
 if(0 <= type && type < 240)	/* short constant */
    return type - 120;

 if(type != 245)
   {
    fprintf(stderr, "Illegal constant argument type %d\n", type);
   }

 b1 = getchar();		/* 16-bit integer constant */
 b2 = getchar();
 return 256 * b2 + b1; 
}

putcst(number)
int number;
{
 if(-120 <= number && number < 120)
   {
    putchar(number + 120);	/* short constant */
    return;
   }

 putchar(245);			/* 16-bit integer constant */
 putchar(number & 255);
 putchar((number >> 8) & 255);
}
