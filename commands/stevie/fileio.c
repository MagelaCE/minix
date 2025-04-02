static	char	RCSid[] =
"$Header: fileio.c,v 1.7 88/10/27 08:15:27 tony Exp $";

/*
 * Basic file I/O routines.
 *
 * further   modifications by:  Robert Regn	   rtregn@faui32.uucp
 *
 * $Log:	fileio.c,v $
 * Revision 1.7  88/10/27  08:15:27  tony
 * Removed support for Megamax, and added a call to flushbuf() for
 * versions that buffer output.
 * 
 * Revision 1.6  88/10/06  10:10:45  tony
 * File names passed to fopen() are now processed by fixname() to do
 * system-dependent hacks on the name (mainly for DOS & OS/2).
 * 
 * Revision 1.5  88/08/26  08:45:00  tony
 * Misc. changes to make lint happy.
 * 
 * Revision 1.4  88/06/10  13:43:19  tony
 * Fixed a bug involving writing out files with long pathnames. A small
 * fixed size buffer was being used. The space for the backup file name
 * is now allocated dynamically.
 * 
 * Revision 1.3  88/05/02  21:37:12  tony
 * Reworked the readfile() routine to better deal with null characters,
 * non-ascii characters, long lines, and incomplete last lines. The status
 * messages now match the real vi correctly for all cases.
 * 
 * Revision 1.2  88/03/21  16:46:01  tony
 * Changed the line counter in renum() from 'int' to 'long' to correspond
 * to the actual type of the pseudo line number in the LINE structure.
 * 
 * Revision 1.1  88/03/20  21:07:19  tony
 * Initial revision
 * 
 *
 */

#include "stevie.h"
#include <sys/stat.h>

void
filemess(s)
char *s;
{
	smsg("\"%s\" %s", (Filename == NULL) ? "" : Filename, s);
	flushbuf();
}

void
renum()
{
	LPTR	*p;
	unsigned long l = 0;

	for (p = Filemem; p != NULL ;p = nextline(p), l += LINEINC)
		p->linep->num = l;

	Fileend->linep->num = 0xffffffff;
}

#define	MAXLINE	256	/* maximum size of a line */

static char rdonly=0;

bool_t
readfile(fname,fromp,nochangename)
char	*fname;
LPTR	*fromp;
bool_t	nochangename;	/* if TRUE, don't change the Filename */
{
	FILE	*f, *fopen();
	LINE	*curr;
	char	buff[MAXLINE], buf2[80];
	int	i, c;
	long	nchars = 0;
	int	linecnt = 0;
	bool_t	wasempty = bufempty();
	int	nonascii = 0;		/* count garbage characters */
	int	nulls = 0;		/* count nulls */
	bool_t	incomplete = FALSE;	/* was the last line incomplete? */
	bool_t	toolong = FALSE;	/* a line was too long */

	curr = fromp->linep;

	if ( ! nochangename )
		Filename = strsave(fname);

	if ( (f=fopen(fixname(fname),"r")) == NULL )
		return TRUE;

	rdonly = (access(fname, 2) != 0);
	filemess( rdonly ? "[Read only]" : "");

	i = 0;
	do {
		c = getc(f);

		if (c == EOF) {
			if (i == 0)	/* normal loop termination */
				break;

			/*
			 * If we get EOF in the middle of a line, note the
			 * fact and complete the line ourselves.
			 */
			incomplete = TRUE;
			c = NL;
		}

		if (c >= 0x80) {
			c -= 0x80;
			nonascii++;
		}

		/*
		 * If we reached the end of the line, OR we ran out of
		 * space for it, then process the complete line.
		 */
		if (c == NL || i == (MAXLINE-1)) {
			int	len;
			LINE	*lp;

			if (c != NL)
				toolong = TRUE;

			buff[i] = '\0';
			len = strlen(buff) + 1;
			if ((lp = newline(len)) == NULL)
				{msg("Buffer overflow - File is uncomplete !!\n\r");
				break;
				}

			strcpy(lp->s, buff);

			curr->next->prev = lp;	/* new line to next one */
			lp->next = curr->next;

			curr->next = lp;	/* new line to prior one */
			lp->prev = curr;

			curr = lp;		/* new line becomes current */
			i = 0;
			linecnt++;

		} else if (c == NUL)
			nulls++;		/* count and ignore nulls */
		else {
			buff[i++] = c;		/* normal character */
		}

		nchars++;

	} while (!incomplete && !toolong);

	fclose(f);

	/*
	 * If the buffer was empty when we started, we have to go back
	 * and remove the "dummy" line at Filemem and patch up the ptrs.
	 */
	if (wasempty && nchars > 0 /* !!! */) {
		LINE	*dummy = Filemem->linep;	/* dummy line ptr */

		free(dummy->s);				/* free string space */
		Filemem->linep = Filemem->linep->next;
		free(dummy);				/* free LINE struct */
		Filemem->linep->prev = Filetop->linep;
		Filetop->linep->next = Filemem->linep;

		Curschar->linep = Filemem->linep;
		Topchar->linep  = Filemem->linep;
	}

	renum();

	if (toolong) {
		smsg("\"%s\" Line too long", fname);
		return FALSE;
	}

	sprintf(buff, "\"%s\" %s %s%d line%s, %ld character%s",
		fname,
		rdonly ? "[Read only]" : "",
		incomplete ? "[Incomplete last line] " : "",
		linecnt, (linecnt > 1) ? "s" : "",
		nchars, (nchars > 1) ? "s" : "");

	buf2[0] = NUL;

	if (nonascii || nulls) {
		if (nonascii) {
			if (nulls)
				sprintf(buf2, " (%d null, %d non-ASCII)",
					nulls, nonascii);
			else
				sprintf(buf2, " (%d non-ASCII)", nonascii);
		} else
			sprintf(buf2, " (%d null)", nulls);
	}
	strcat(buff, buf2);
	msg(buff);

	return FALSE;
}


/*
 * writeit - write to file 'fname' lines 'start' through 'end'
 *
 * If either 'start' or 'end' contain null line pointers, the default
 * is to use the start or end of the file respectively.
 */
bool_t
writeit(fname, start, end)
char	*fname;
LPTR	*start, *end;
{
	FILE	*f, *fopen();
	FILE	*fopenb();		/* open in binary mode, where needed */
	char	*backup, *s;
	long	nchars;
	int	lines;
	LPTR	*p;
	struct stat sbuf;
	char newfile=0;
  
	if (stat(fname, &sbuf)!= 0)	/* save mode of file for creating */
		newfile = 1;
  
  	/*

	smsg("\"%s\"", fname);

	/*
	 * Form the backup file name - change foo.* to foo.bak
	 */
	backup = alloc((unsigned) (strlen(fname) + 5));
	strcpy(backup, fname);
	for (s = backup; *s && *s != '.' ;s++)
		;
	*s = NUL;
	strcat(backup, ".bak");

	/*
	 * Delete any existing backup and move the current version
	 * to the backup. For safety, we don't remove the backup
	 * until the write has finished successfully. And if the
	 * 'backup' option is set, leave it around.
	 */
	/* skipping rename avoids overwriting  R/O files by creating a new */
	/* also save link structure 			R. Regn*/

	if (!rdonly && sbuf.st_nlink == 1)
		rename(fname, backup);


	f = P(P_CR) ? fopen(fixname(fname), "w") : fopenb(fixname(fname), "w");

	if ( f == NULL ) {
		if (rdonly)
			emsg("File is Read only");
		else 	emsg("Permission denied");
		free(backup);
		return FALSE;
	}

	if (!rdonly && !newfile)	/* restore mode (and owner) properly */
		{chmod (fname, sbuf.st_mode&04777);
		 chown (fname, sbuf.st_uid, sbuf.st_gid);
		}

	/*
	 * If we were given a bound, start there. Otherwise just
	 * start at the beginning of the file.
	 */
	if (start == NULL || start->linep == NULL)
		p = Filemem;
	else
		p = start;

	lines = nchars = 0;
	do {
		fprintf(f, "%s\n", p->linep->s);
		nchars += strlen(p->linep->s) + 1;
		lines++;

		/*
		 * If we were given an upper bound, and we just did that
		 * line, then bag it now.
		 */
		if (end != NULL && end->linep != NULL) {
			if (end->linep == p->linep)
				break;
		}

	} while ((p = nextline(p)) != NULL);

	fclose(f);
	smsg("\"%s\" %s %d line%s, %ld character%s", fname,
		newfile ? "[New File]" : "",
		lines, (lines > 1) ? "s" : "",
		nchars, (nchars > 1) ? "s" : "");

	UNCHANGED;

	/*
	 * Remove the backup unless they want it left around
	 */
	if (!P(P_BK))
		remove(backup);

	free(backup);

	return TRUE;
}
