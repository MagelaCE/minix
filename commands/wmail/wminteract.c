/*
 * WMAIL -	MicroWalt Extended Mail Agent.
 *		This is the MicroWalt Mail Agent; which is derived
 *		from  the  "Mini-Mail" written by Peter S. Housel.
 *		This version  has a better  user-interface, and it
 *		also "knows" about things like forwarding, replies
 *		etc. Overall, it looks like the Mail (uppercase M)
 *		on our local DYNIX(tm) (==BSD) system...
 *		The paging-code (for paging letters on the screen)
 *		was taken from "more.c", by Brandon Allbery.
 *
 *		I N T E R A C T I O N    M O D U L E
 *
 * Author:	Fred van Kempen, MicroWalt Corporation
 *
 * To Do:
 *		- Builtin escapes (~i and friends)
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include "wmail.h"


/*
 * Send a reply to a message.
 */
static int do_reply(let)
LETTER *let;
{
  char repuser[512];
  char *who[2];

  strcpy(repuser, let->sender);
  who[0] = &repuser[0];
  who[1] = (char *)NULL;
  sprintf(subject, "Re: %s", let->subject);

  printf("\n*** From: %s %-24.24s\n", sender, xtime());
  printf("*** To: %s\n", who[0]);
  printf("*** Subject: %s\n\n", subject);

  deliver(1, who);
}


/*
 * Execute a shell (or a command only)
 */
static void do_shell(command)
char *command;
{
  int waitstat, pid;
  char *shell;

  if ((shell = getenv("SHELL")) == NIL) shell = SHELL;

  if ((pid = fork()) < 0) {
	fprintf(stderr, "%s: could not fork", progname);
       	return;
  } else if (pid != 0) {		/* parent */
		wait(&waitstat);
        	return;
    }

  /* child */
  setgid(old_gid);	/* UUCP or USER */
  setuid(old_uid);	/* UUCP or USER */
  umask(oldmask);

  execl(shell, shell, "-c", command, NIL);
  fprintf(stderr, "%s: cannot exec shell", progname);
  exit(127);
}


/*
 * Goto a specific letter.
 */
static LETTER *goto_letter(num)
int num;
{
  LETTER *let;

  let = firstlet;
  while (let != NIL_LET) {
	if (let->seqno == num) return(let);
	let = let->next;
  }
  return(NIL_LET);
}


/*
 * Skip the header of message 'let'.
 * Do this by just updating the 'boxfp' pointer...
 */
off_t skiphead(let)
LETTER *let;
{
  char xbuf[1024];
  off_t count;

  count = (off_t) 0;
  while (fgets(xbuf, sizeof(xbuf), boxfp) != NIL) {
	count += (off_t) strlen(xbuf);
      	if (xbuf[0] == '\n') break;  /* end of header */
  }
  return(count);
}


/*
 * Save the current letter to a disk-file.
 */
void savelet(let, savefile, withhead)
LETTER *let;
char *savefile;
int withhead;
{
  int c;
  off_t curr, limit, oldpos;
  register char *bp;
  FILE *savefp;

  bp = savefile;
  while (*bp && *bp!='\n') bp++;
  *bp = '\0';

  if ((savefp = fopen(savefile, "a")) == (FILE *)NULL) {
	fprintf(stderr, "%s: cannot append to savefile \"%s\"\n",
							progname, savefile);
	return;
  }

  oldpos = ftell(boxfp);
  fseek(boxfp, (curr = let->location), 0);
  limit = (let->next != NIL_LET) ? let->next->location : -1L;

  if (withhead == 0) curr += skiphead(let);	/* skip the message header */
  while(curr != limit && (c = fgetc(boxfp)) != EOF) {
	fputc(c, savefp);
      	++curr;
  }
  fflush(savefp);
  fseek(boxfp, oldpos, 0);

  if ((ferror(savefp) != 0) | (fclose(savefp) != 0)) {
	fprintf(stderr, "%s: savefile write error:", progname);
  }

  chown(savefile, old_uid, old_gid);
}


/*
 * Give a list of possible Interact Commands.
 */
static void do_help(void)
{
  printf("\n   ** W-MAIL Commands **\n\n");
  printf("?\tThis help\n");
  printf("!\tShell Command Escape\n");
  printf("-\tPrevious letter\n");
  printf("+\tNext letter\n");
  printf("<ENTER>\tNext letter\n");
  printf("d\tDelete current letter\n");
  printf("i\tDisplay a summary of letters\n");
  printf("p\tPrint a letter again\n");
  printf("q\tQuit, update mailbox\n");
  printf("r\tReply to the current letter\n");
  printf("s\tSave current letter\n");
  printf("t\tType a letter, no paging\n");
  printf("w\tSave letter without header\n");
  printf("x\tExit, do not update mailbox\n");
  printf("\n");
}


/*
 * Give a summary of letters./
 */
static void summary(void)
{
  register LETTER *let;
  register int i;
  register char *sp;

  let = firstlet;
  printf(" No.  Sender           Date            Subject\n");
  printf(" ----------------------------------------------");
  printf("--------------------------------\n");

  for (i=0; i<numlet; i++) {
	sp = let->date;
	if (strchr(sp, ',')) sp += 5;	/* new-style ctime() */
	  else sp += 4;
	printf("%c%c %-3.3d  %-8.8s  %-15.15s  \"%.40s\"\n",
		(nextlet == let->seqno) ? '>': ' ',
		(let->status == DELETED) ? '*': ' ',
 		let->seqno, basepath(let->sender), sp, let->subject);
        let = let->next;
  }

  printf("\n");
}


/*
 * Interactively read the mail-box.
 */
void interact(void)
{
  char input[512];	/* user input line */
  char *p;
  LETTER *let, dummy;	/* current and next letter */
  LETTER *templet;
  int interrupted = 0;	/* SIGINT hit during letter print */
  char *savefile;		/* filename to save into */
  int i, temp;

  if (firstlet == NIL_LET) {
	printf("No mail for %s.\n", sender);
      	return;
  }

  printf("W-MAIL %s.  Type ? for Help.\n", Version);
  printf("\"%s\": %d message(s)\n\n", mailbox, numlet);

  nextlet = 1;
  dummy.seqno = nextlet;
  dummy.next = firstlet;
  let = &dummy;

  summary();

  while(1) {
	nextlet = let->seqno;

       	if (!quitmode) {
		interrupted = setjmp(printjump);
		signal(SIGINT, onint);
       	}

      	if (interrupted) printf("\n");
      	printf(PROMPT, let->seqno);
      	fflush(stdout);

      	if (fgets(input, sizeof(input), stdin) == NIL) break;

      	if (!quitmode) signal(SIGINT, SIG_IGN);

      	switch(input[0]) {
		case '!':
			do_shell(input + 1);
			continue;
		case '?':	
			do_help();
			continue;
		case '-':
			if (let->prev != NIL_LET) let = let->prev;
			  else printf("Top of mailbox\n");
			continue;
		case '+':
		case '\n':
			if (let->next != NIL_LET) {
				let = let->next;
				if (!interrupted) {
		            		if (let->status != DELETED)
							let->status = READ;
			    		printlet(let, stdout);
		           	}
			} else printf("At EOF\n");
			continue;
		case 'd':
			let->status = DELETED;
			if (let->next != NIL_LET) let = let->next;
			needupdate = 1;
			continue;
		case 'i':
			summary();
			continue;
		case 'p':
			if (!interrupted) printlet(let, stdout);
			continue;
		case 'q':
			return;
		case 'r':
			do_reply(let);
			break;
		case 's':
			savefile = &input[1];
			if (*savefile != '\0') {
			  	while (*savefile==' ' || *savefile=='\t')
								savefile++; 
			} else savefile = SAVEFILE;
			savelet(let, savefile, 1);
			continue;
		case 't':
			temp = printmode;
			printmode = 1;
			if (!interrupted) printlet(let, stdout);
			printmode = temp;
			continue;
		case 'w':
			savefile = &input[1];
			if (*savefile != '\0') {
			  	while (*savefile==' ' || *savefile=='\t')
								savefile++; 
			} else savefile = SAVEFILE;
			savelet(let, savefile, 0);
			continue;
		case 'x':
			exit(0);
		default:
			if (isdigit(input[0])) {
				templet = goto_letter(atoi(input));
				if (templet != NIL_LET) {
					let = templet;
					printlet(let, stdout);
			   	} else printf("Illegal message-number\n");
			} else printf("Illegal command\n");
			continue;
  	}
  }   
}

