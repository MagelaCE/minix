/*
 *	termcap.c	1.1	20/7/87		agc	Joypace Ltd
 *
 *	Copyright Joypace Ltd, London, UK, 1987. All rights reserved.
 *	This file may be freely distributed provided that this notice
 *	remains attached.
 *
 *	A public domain implementation of the termcap(3) routines.
 *
 *	improvements and optimalisation by Klamer Schutte 21/11/88
 */
#include <stdio.h>

#define CAPABLEN	2

#define ISSPACE(c)	((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')
#define ISDIGIT(x)	((x) >= '0' && (x) <= '9')

extern short	ospeed;		/* output speed */
extern char	PC;		/* padding character */
extern char	*BC;		/* back cursor movement */
extern char	*UP;		/* up cursor movement */

char		*capab = NULL;	/* the capability itself */
int		incr;		/* set by %i flag */

extern char	*getenv();	/* new, improved getenv */
extern FILE	*fopen();	/* old fopen */

/*
 *	tgetent - get the termcap entry for terminal name, and put it
 *	in bp (which must be an array of 1024 chars). Returns 1 if
 *	termcap entry found, 0 if not found, and -1 if file not found.
 */
int
tgetent(bp, name)
char	*bp;
char	*name;
{
	FILE	*fp;
	char	*file;
	char	*cp;
	short	len = strlen(name);
	int	get_entry();
	int	cnt;

	capab = bp;
	if ((file = getenv("TERMCAP")) != (char *) NULL) {
		if (*file != '/' &&
		    (cp = getenv("TERM")) != NULL && strcmp(name, cp) == 0) {
			(void) strcpy(bp, file);
			return(1);
		}
	} else
		file = "/etc/termcap";
	if ((fp = fopen(file, "r")) == (FILE *) NULL)
	{	capab = NULL;			/* no valid termcap	*/
		return(-1); 
	}

	while( get_entry( bp, fp ) )
	{ 	cp = bp;
		do
		{	for(cnt=0; name[cnt] != 0; cnt++, cp++)
				if (name[cnt] != *cp)
					break;
			if ((name[cnt] != 0) || ((*cp != '|') && (*cp != ':')))
				goto next_name;
			fclose(fp);
			return 1;
next_name: 		while( (*cp) && (*cp != '|') && (*cp != ':'))
				cp++;
		} while( *cp++ == '|');
	}	
	fclose(fp);
	capab = NULL;				/* no valid termcap	*/
	return(0);
	
}

/* copy entry in termcap file to buf */
static get_entry( buf, file )
register char	*buf;
register FILE	*file;
{
	register int	c, last = 0;
	
	while( (c = fgetc( file )) != EOF)
		if ((*buf++ = c) == '\n')
			if (last == '\\')
				buf -= 2;
			else
			{	*buf = 0;
				return 1;
			}
		else
			last = c;
	return 0;
}
	
/*
 *	tgetnum - get the numeric terminal capability corresponding
 *	to id. Returns the value, -1 if invalid.
 */
int
tgetnum(id)
char	*id;	
{
	register char	*cp;

	if ((cp = capab) == NULL || id == NULL)
		return(-1);
	while (*cp++ != ':')			/* skip terminal name	*/
		;
	for (; *cp ; cp++) {
		if ((*id == *cp++) && (id[1] == *cp)){
			if (cp[1] != '#')
				return(-1);
			return atoi( cp + 2 );
		}
		while (*cp && *cp != ':')
			cp++;
	}
	return(-1);
}

/*
 *	tgetflag - get the boolean flag corresponding to id. Returns -1
 *	if invalid, 0 if the flag is not in termcap entry, or 1 if it is
 *	present.
 */
int
tgetflag(id)
char	*id;
{
	register char	*cp;

	if ((cp = capab) == NULL || id == NULL)
		return(-1);
	while (*cp++ != ':')			/* skip terminal name	*/
		;
	for ( ; *cp ; cp++) {
		if ((*id == *cp++) && (id[1] == *cp))
			return(1);
		while (*cp && *cp != ':')
			cp++;
	}
	return(0);
}

/*
 *	tgetstr - get the string capability corresponding to id and place
 *	it in area (advancing area at same time). Expand escape sequences
 *	etc. Returns the string, or NULL if it can't do it.
 */
char *
tgetstr(id, area)
char	*id;
char	**area;
{
	register char	*cp, *wsp;		/* workspace == *area	*/
	char	*ret;
	int	i;

	if ((cp = capab) == NULL || id == NULL)
		return(NULL);
	while (*cp++ != ':')			/* skip terminal name	*/
		;
	for ( ; *cp ; cp++) {
		if ((*id == *cp++) && (id[1] == *cp)){
			if (cp[1] != '=')
				return(NULL);
			cp += 2;
			for (ret = wsp = *area; *cp && *cp != ':' ; wsp++, cp++)
				switch(*cp) {
				case '^' :
					*wsp = *++cp - 'A';
					break;
				case '\\' :
					switch(*++cp) {
					case 'E' :
						*wsp = '\033';
						break;
					case 'n' :
						*wsp = '\n';
						break;
					case 'r' :
						*wsp = '\r';
						break;
					case 't' :
						*wsp = '\t';
						break;
					case 'b' :
						*wsp = '\b';
						break;
					case 'f' :
						*wsp = '\f';
						break;
					case '0' :
					case '1' :
					case '2' :
					case '3' :
						for (i=0 ; *cp && ISDIGIT(*cp) ; cp++)
							i = i * 8 + *cp - '0';
						*wsp = i;
						cp--;
						break;
					case '^' :
					case '\\' :
						*wsp = *cp;
						break;
					}
					break;
				default :
					*wsp = *cp;
				}
			*area = wsp;
			*(*area)++ = '\0';
			return(ret);
		}
		while (*cp && *cp != ':')
			cp++;
	}
	return(NULL);
}

/*
 *	tgoto - given the cursor motion string cm, make up the string
 *	for the cursor to go to (destcol, destline), and return the string.
 *	Returns "OOPS" if something's gone wrong, or the string otherwise.
 */
char *
tgoto(cm, destcol, destline)
char	*cm;
int	destcol;
int	destline;
{
	register char	*rp;
	static char	ret[24];
	int		*dp = &destcol;
	int 		argno = 0, numval;

	for (rp = ret ; *cm ; cm++) {
		switch(*cm) {
		case '%' :
			switch(*++cm) {
			case '+' :
				if (dp == NULL)
					return("OOPS");
				*rp++ = *dp + *++cm;
				dp = (dp == &destcol) ? &destline : NULL;
				break;

			case '%' :
				*rp++ = '%';
				break;

			case 'i' :

				incr = 1;
				break;

			case 'd' :
				numval = (argno == 0 ? destline : destcol);
				numval += incr;
				argno++;
				*rp++ = '0' + (numval/10);
				*rp++ = '0' + (numval%10);
				break;
			}

			break;
		default :
			*rp++ = *cm;
		}
	}
	*rp = '\0';
	return(ret);
}

/*
 *	tputs - put the string cp out onto the terminal, using the function
 *	outc. This should do padding for the terminal, but I can't find a
 *	terminal that needs padding at the moment...
 */
int
tputs(cp, affcnt, outc)
register char	*cp;
int		affcnt;
int		(*outc)();
{
	if (cp == NULL)
		return(1);
	/* do any padding interpretation - left null for MINIX just now */
	while (*cp)
		(*outc)(*cp++);
	return(1);
}
