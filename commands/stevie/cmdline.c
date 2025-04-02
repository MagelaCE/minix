static	char	RCSid[] =
"$Header: cmdline.c,v 1.9 88/11/10 08:58:04 tony Exp $";

/*
 * Routines to parse and execute "command line" commands, such as searches
 * or colon commands.
 *
 * further   modifications by:  Robert Regn	   rtregn@faui32.uucp
 *
 * $Log:	cmdline.c,v $
 * Revision 1.9  88/11/10  08:58:04  tony
 * Added support for modelines when a new file edit is started.
 * 
 * Revision 1.8  88/11/01  21:31:56  tony
 * Added a call to flushbuf() in msg() to make sure messages get out
 * when they should.
 * 
 * Revision 1.7  88/10/27  08:21:38  tony
 * Removed support for Megamax. Added support for ":sh" and ":!", and
 * removed doshell() since it needs to be system-dependent. Changed
 * ":p" to ":N" to avoid name conflict with the "print" command. The
 * address spec "%" is translated to "1,$". The "=" command now works
 * to print the line number of the given address.
 * 
 * Revision 1.6  88/08/26  08:44:48  tony
 * Misc. changes to make lint happy.
 * 
 * Revision 1.5  88/06/26  14:48:20  tony
 * Added a call to doglob() to handle the new ":g" command.
 * 
 * Revision 1.4  88/06/25  21:43:19  tony
 * Fixed a problem in the processing of colon commands that caused
 * substitutions of patterns containing white space to fail.
 * 
 * Revision 1.3  88/04/24  21:35:09  tony
 * Added a new colon command "s" (substitute), which is performed by the
 * routine dosub() in search.c.
 * 
 * Revision 1.2  88/03/23  21:25:10  tony
 * Fixed a bug involving backspacing out of a colon or search command.
 * The fix is a hack until that part of the code can be cleaned up more.
 * 
 * Revision 1.1  88/03/20  21:06:19  tony
 * Initial revision
 * 
 *
 */

#include "stevie.h"

static	char	*altfile = NULL;	/* alternate file */
static	int	altline;		/* line # in alternate file */

static	char	*nowrtmsg = "No write since last change (use ! to override)";
static	char	*nooutfile = "No output file";
static	char	*morefiles = "more files to edit";

extern	char	**files;		/* used for "n" and "rew" */
extern	int	numfiles, curfile;

/*
 * The next two variables contain the bounds of any range given in a
 * command. If no range was given, both contain null line pointers.
 * If only a single line was given, u_pos will contain a null line
 * pointer.
 */
static	LPTR	l_pos, u_pos;

static	bool_t	interactive;	/* TRUE if we're reading a real command line */

#define	CMDSZ	100		/* size of the command buffer */

static	bool_t	doecmd();
static	void	badcmd(), get_range();
static	LPTR	*get_line();

/*
 * readcmdline() - accept a command line starting with ':', '/', or '?'
 *
 * readcmdline() accepts and processes colon commands and searches. If
 * 'cmdline' is null, the command line is read here. Otherwise, cmdline
 * points to a complete command line that should be used. This is used
 * in main() to handle initialization commands in the environment variable
 * "EXINIT".
 */
void
readcmdline(firstc, cmdline)
int	firstc;		/* either ':', '/', or '?' */
char	*cmdline;	/* optional command string */
{
	int c;
	char buff[CMDSZ];
	char cmdbuf[CMDSZ];
	char argbuf[CMDSZ];
	char *p, *q, *cmd, *arg;

	/*
	 * Clear the range variables.
	 */
	l_pos.linep = (struct line *) NULL;
	u_pos.linep = (struct line *) NULL;

	interactive = (cmdline == NULL);

	if (interactive)
		gotocmd(TRUE, firstc);
	p = buff;
	if ( firstc != ':' )
		*p++ = firstc;

	if (interactive) {
		/* collect the command string, handling '\b' and @ */
		for ( ; ; ) {
			c = vgetc();
			if ( c=='\n'||c=='\r'||c==EOF )
				break;
			if ( c=='\b' ) {
				if ( p > buff + (firstc != ':') ) {
					p--;
					/* this is gross, but it relies
					 * only on 'gotocmd'
					 */
					gotocmd(TRUE, firstc==':'?':':0);
					for ( q=buff; q<p; q++ )
						outchar(*q);
				} else {
					msg("");
					return;		/* back to cmd mode */
				}
				continue;
			}
			if ( c=='@' ) {
				p = buff;
				gotocmd(TRUE, firstc);
				continue;
			}
			outchar(c);
			*p++ = c;
		}
		*p = '\0';
	} else {
		if (strlen(cmdline) > CMDSZ-2)	/* should really do something */
			return;			/* better here... */
		strcpy(p, cmdline);
	}

	/* skip any initial white space */
	for ( cmd = buff; *cmd != NUL && isspace(*cmd); cmd++ )
		;

	/* search commands */
	c = *cmd;
	if ( c == '/' || c == '?' ) {
		cmd++;
		if ( *cmd == c || *cmd == NUL ) {
			/*
			 * The command was just '/' or '?' or possibly
			 * '//' or '??'. Search in the requested direction
			 * for the last search string used. The NULL
			 * parameter to dosearch() tells it to do this.
			 */
			dosearch((c == '/') ? FORWARD : BACKWARD, NULL);
			return;
		}
		/* If there is a matching '/' or '?' at the end, toss it */
		p = strchr(cmd, NUL);
		if ( *(p-1) == c && *(p-2) != '\\' )
			*(p-1) = NUL;
		dosearch((c == '/') ? FORWARD : BACKWARD, cmd);
		return;
	}

	if (*cmd == '%') {		/* change '%' to "1,$" */
		strcpy(cmdbuf, "1,$");	/* kind of gross... */
		strcat(cmdbuf, cmd+1);
		strcpy(cmd, cmdbuf);
	}

	while ( (p=strchr(cmd, '%')) != NULL && *(p-1) != '\\') {
					/* change '%' to Filename */
		if (Filename == NULL) {
			emsg("No filename");
			return;
		}
		*p= NUL;
		strcpy (cmdbuf, cmd);
		strcat (cmdbuf, Filename);
		strcat (cmdbuf, p+1);
		strcpy(cmd, cmdbuf);
		msg(cmd);			/*repeat */
	}

	while ( (p=strchr(cmd, '#')) != NULL && *(p-1) != '\\') {
					/* change '#' to Altname */
		if (altfile == NULL) {
			emsg("No alternate file");
			return;
		}
		*p= NUL;
		strcpy (cmdbuf, cmd);
		strcat (cmdbuf, altfile);
		strcat (cmdbuf, p+1);
		strcpy(cmd, cmdbuf);
		msg(cmd);			/*repeat */
	}

	/*
	 * Parse a range, if present (and update the cmd pointer).
	 */
	get_range(&cmd);

	if (l_pos.linep != NULL) {
		if (LINEOF(&l_pos) > LINEOF(&u_pos)) {
			emsg("Invalid range");
			return;
		}
	}

	strcpy(cmdbuf, cmd);	/* save the unmodified command */

	/* isolate the command and find any argument */
	for ( p=cmd; *p != NUL && ! isspace(*p); p++ )
		;
	if ( *p == NUL )
		arg = NULL;
	else {
		*p = NUL;
		for (p++; *p != NUL && isspace(*p) ;p++)
			;
		if (*p == NUL)
			arg = NULL;
		else {
			strcpy(argbuf, p);
			arg = argbuf;
		}
	}
	if ( strcmp(cmd,"q!")==0 )
		getout();
	if ( strcmp(cmd,"q")==0 ) {
		if ( Changed )
			emsg(nowrtmsg);
		else
		if ( (curfile + 1) < numfiles )
			emsg (morefiles);
		else	getout();
		return;
	}
	if ( strcmp(cmd,"w")==0 ) {
		if ( arg == NULL ) {
			if (Filename != NULL) {
				writeit(Filename, &l_pos, &u_pos);
			} else
				emsg(nooutfile);
		}
		else
			writeit(arg, &l_pos, &u_pos);
		return;
	}
	if ( strcmp(cmd,"wq")==0 ) {
		if (Filename != NULL) {
			if (writeit(Filename, (LPTR *)NULL, (LPTR *)NULL))
				getout();
		} else
			emsg(nooutfile);
		return;
	}
	if ( strcmp(cmd, "x") == 0 ) {
		doxit();
		return;
	}

	if ( strcmp(cmd,"f")==0 && arg==NULL ) {
		fileinfo();
		return;
	}
	if ( *cmd == 'n' ) {
		if ( (curfile + 1) < numfiles ) {
			/*
			 * stuff ":e[!] FILE\n"
			 */
			stuffin(":e");
			if (cmd[1] == '!')
				stuffin("!");
			stuffin(" ");
			stuffin(files[++curfile]);
			stuffin("\n");
		} else
			emsg("No more files!");
		return;
	}
	if ( *cmd == 'N' ) {
		if ( curfile > 0 ) {
			/*
			 * stuff ":e[!] FILE\n"
			 */
			stuffin(":e");
			if (cmd[1] == '!')
				stuffin("!");
			stuffin(" ");
			stuffin(files[--curfile]);
			stuffin("\n");
		} else
			emsg("No more files!");
		return;
	}
	if ( strncmp(cmd, "rew", 3) == 0) {
		if (numfiles <= 1)		/* nothing to rewind */
			return;
		curfile = 0;
		/*
		 * stuff ":e[!] FILE\n"
		 */
		stuffin(":e");
		if (cmd[3] == '!')
			stuffin("!");
		stuffin(" ");
		stuffin(files[0]);
		stuffin("\n");
		return;
	}
	if ( strcmp(cmd,"e") == 0 || strcmp(cmd,"e!") == 0 ) {
		doecmd(arg, cmd[1] == '!');
		return;
	}
	if ( strcmp(cmd,"f") == 0 ) {
		Filename = strsave(arg);
		filemess("");
		return;
	}
	if ( strcmp(cmd,"r") == 0 ) {
		if ( arg == NULL ) {
			badcmd();
			return;
		}
		if (readfile(arg, Curschar, 1)) {
			emsg("Can't open file");
			return;
		}
		updatescreen();
		CHANGED;
		return;
	}
	if ( strcmp(cmd,"=")==0 ) {
		smsg("%d", cntllines(Filemem, &l_pos));
		return;
	}
	if ( strncmp(cmd,"ta", 2) == 0 ) {
		dotag(arg, cmd[2] == '!');
		return;
	}
	if ( strncmp(cmd,"set", 2)==0 ) {
		doset(arg, interactive);
		return;
	}
	if ( strcmp(cmd,"help")==0 ) {
		if (help()) {
			screenclear();
			updatescreen();
		}
		return;
	}
	if ( strncmp(cmd, "ve", 2) == 0) {
		extern	char	*Version;

		msg(Version);
		return;
	}
	if ( strcmp(cmd, "sh") == 0) {
		doshell(NULL);
		return;
	}
	if ( *cmd == '!' ) {
		doshell(cmdbuf+1);
		return;
	}
	if ( strncmp(cmd, "s/", 2)==0 ) {
		dosub(&l_pos, &u_pos, cmdbuf+1);
		return;
	}
	if (strncmp(cmd, "g/", 2) == 0) {
		doglob(&l_pos, &u_pos, cmdbuf+1);
		return;
	}
	/*
	 * If we got a line, but no command, then go to the line.
	 */
	if (*cmd == NUL && l_pos.linep != NULL) {
		*Curschar = l_pos;
		cursupdate();
		return;
	}

	badcmd();
}


doxit()
{
	if (Changed) {
		if (Filename != NULL) {
			if (!writeit(Filename, (LPTR *)NULL, (LPTR *)NULL))
				return;
		} else {
			emsg(nooutfile);
			return;
		}
	}
	if ( (curfile + 1) < numfiles )
		emsg (morefiles);
	else	getout();
}

/*
 * get_range - parse a range specifier
 *
 * Ranges are of the form:
 *
 * addr[,addr]
 *
 * where 'addr' is:
 *
 * $  [+- NUM]
 * 'x [+- NUM]	(where x denotes a currently defined mark)
 * .  [+- NUM]
 * NUM
 *
 * The pointer *cp is updated to point to the first character following
 * the range spec. If an initial address is found, but no second, the
 * upper bound is equal to the lower.
 */
static void
get_range(cp)
char	**cp;
{
	LPTR	*l;
	char	*p;

	if ((l = get_line(cp)) == NULL)
		return;

	l_pos = *l;

	for (p = *cp; *p != NUL && isspace(*p) ;p++)
		;

	*cp = p;

	if (*p != ',') {		/* is there another line spec ? */
		u_pos = l_pos;
		return;
	}

	*cp = ++p;

	if ((l = get_line(cp)) == NULL) {
		u_pos = l_pos;
		return;
	}

	u_pos = *l;
}

static LPTR *
get_line(cp)
char	**cp;
{
	static	LPTR	pos;
	LPTR	*lp;
	char	*p, c;
	int	lnum;

	pos.index = 0;		/* shouldn't matter... check back later */

	p = *cp;
	/*
	 * Determine the basic form, if present.
	 */
	switch (c = *p++) {

	case '$':
		pos.linep = Fileend->linep->prev;
		break;

	case '.':
		pos.linep = Curschar->linep;
		break;

	case '\'':
		if ((lp = getmark(*p++)) == NULL) {
			emsg("Unknown mark");
			return (LPTR *) NULL;
		}
		pos = *lp;
		break;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		for (lnum = c - '0'; isdigit(*p) ;p++)
			lnum = (lnum * 10) + (*p - '0');

		pos = *gotoline(lnum);
		break;

	default:
		return (LPTR *) NULL;
	}

	while (*p != NUL && isspace(*p))
		p++;

	if (*p == '-' || *p == '+') {
		bool_t	neg = (*p++ == '-');

		for (lnum = 0; isdigit(*p) ;p++)
			lnum = (lnum * 10) + (*p - '0');

		if (neg)
			lnum = -lnum;

		pos = *gotoline( cntllines(Filemem, &pos) + lnum );
	}

	*cp = p;
	return &pos;
}

static void
badcmd()
{
	if (interactive)
		emsg("Unrecognized command");
}

#define	LSIZE	512	/* max. size of a line in the tags file */

/*
 * dotag(tag, force) - goto tag
 */
void
dotag(tag, force)
char	*tag;
bool_t	force;
{
	FILE	*tp, *fopen();
	char	lbuf[LSIZE];
	char	*fname, *str;

	if ((tp = fopen("tags", "r")) == NULL) {
		emsg("Can't open tags file");
		return;
	}

	while (fgets(lbuf, LSIZE, tp) != NULL) {
	
		if ((fname = strchr(lbuf, TAB)) == NULL) {
			emsg("Format error in tags file");
			return;
		}
		*fname++ = '\0';
		if ((str = strchr(fname, TAB)) == NULL) {
			emsg("Format error in tags file");
			return;
		}
		*str++ = '\0';

		if (strcmp(lbuf, tag) == 0) {
			if (doecmd(fname, force)) {
				stuffin(str);		/* str has \n at end */
				stuffin("\007");	/* CTRL('G') */
				fclose(tp);
				return;
			}
		}
	}
	emsg("tag not found");
	fclose(tp);
}

static	bool_t
doecmd(arg, force)
char	*arg;
bool_t	force;
{
	int	line = 1;		/* line # to go to in new file */

	if (!force && Changed) {
		emsg(nowrtmsg);
		if (altfile)
			free(altfile);
		altfile = strsave(arg);
		return FALSE;
	}
	if ( arg != NULL ) {
		/*
		 * First detect a ":e" on the current file. This is mainly
		 * for ":ta" commands where the destination is within the
		 * current file.
		 */
		if (Filename != NULL && strcmp(arg, Filename) == 0) {
			if (!Changed || (Changed && !force))
				return TRUE;
		}
#ifdef OLDSUB
		if (strcmp(arg, "#") == 0) {	/* alternate */
			char	*s = Filename;

			if (altfile == NULL) {
				emsg("No alternate file");
				return FALSE;
			}
			Filename = altfile;
			altfile  = s;
			line = altline;
			altline = cntllines(Filemem, Curschar);
		} else
		  if (strchr(arg, '%')) {
			char *s, *buf=alloc(strlen(arg)+strlen(Filename));
			if (Filename == NULL) {
				emsg("No filename");
				return FALSE;
			}
			s = strchr(arg, '%');
			*s = 0;
			strcpy (buf, arg);
			strcat (buf, Filename);
			strcat (buf, s+1);
			if (altfile)		/* I'm not shure if it is ok */
				free(altfile);
			altfile = Filename;
			altline = cntllines(Filemem, Curschar);
			Filename = buf;
		} else {
#endif
			if (altfile)		/* I'm not shure if it is ok */
				{
				if (strcmp (arg, altfile) == 0)
					line = altline;
				free(altfile);
				}
			altfile = Filename;
			altline = cntllines(Filemem, Curschar);
			Filename = strsave(arg);
#ifdef OLDSUB
		}
#endif
	}
	if (Filename == NULL) {
		emsg("No filename");
		return FALSE;
	}

	/* clear mem and read file */
	freeall();
	filealloc();
	UNCHANGED;

	if (readfile(Filename, Filemem, 0))
		filemess("[New File]");
	*Topchar = *Curschar;
	if (line != 1) {
		stuffnum(line);
		stuffin("G");
	}
	do_mlines();
	setpcmark();
	updatescreen();
	return TRUE;
}

void
gotocmd(clr, firstc)
bool_t  clr;
char	firstc;
{
	windgoto(Rows-1,0);
	if ( clr )
		outstr(T_EL);		/* clear the bottom line */
	if ( firstc )
		outchar(firstc);
}

/*
 * msg(s) - displays the string 's' on the status line
 */
void
msg(s)
char *s;
{
	gotocmd(TRUE, 0);
	outstr(s);
	flushbuf();
}

/*VARARGS1*/
void
smsg(s, a1, a2, a3, a4, a5, a6, a7, a8, a9)
char	*s;
int	a1, a2, a3, a4, a5, a6, a7, a8, a9;
{
	char	sbuf[80];

	sprintf(sbuf, s, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	msg(sbuf);
}

/*
 * emsg() - display an error message
 *
 * Rings the bell, if appropriate, and calls message() to do the real work
 */
void
emsg(s)
char	*s;
{
	if (P(P_EB))
		beep();
	msg(s);
}

void
wait_return()
{
	char	c;

	outstr("Press RETURN to continue");
	do {
		c = vgetc();
	} while (c != '\r' && c != '\n' && c != ' ' && c != ':');

	if ( c == ':') {				/* this can vi too  */
		outstr("\n");
		readcmdline(c, NULL);
	}
	else
		screenclear();
	updatescreen();
}
