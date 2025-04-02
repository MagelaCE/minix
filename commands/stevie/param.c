static	char	RCSid[] =
"$Header: param.c,v 1.6 88/11/10 08:57:18 tony Exp $";

/*
 * Code to handle user-settable parameters. This is all pretty much table-
 * driven. To add a new parameter, put it in the params array, and add a
 * macro for it in param.h. If it's a numeric parameter, add any necessary
 * bounds checks to doset(). String parameters aren't currently supported.
 *
 * $Log:	param.c,v $
 * Revision 1.6  88/11/10  08:57:18  tony
 * Added a new parameter, modelines ("ml"), to support the modeline
 * feature.
 * 
 * Revision 1.5  88/08/26  08:45:34  tony
 * Misc. changes to make lint happy.
 * 
 * Revision 1.4  88/05/01  20:08:12  tony
 * Added the new boolean parameter "number" to turn on line numbering.
 * 
 * Revision 1.3  88/04/30  20:00:29  tony
 * Added the parameter "autoindent".
 * 
 * Revision 1.2  88/04/28  08:06:11  tony
 * Added a new parameter "ignorecase" which can be set to ignore case
 * distinctions in string searches.
 * 
 * Revision 1.1  88/03/20  21:09:40  tony
 * Initial revision
 * 
 *
 */

#include "stevie.h"

struct	param	params[] = {

	{ "tabstop",	"ts",		8,	P_NUM },
	{ "scroll",	"scroll",	12,	P_NUM },
	{ "report",	"report",	5,	P_NUM },
	{ "lines",	"lines",	25,	P_NUM },

	{ "vbell",	"vb",		TRUE,	P_BOOL },
	{ "showmatch",	"sm",		FALSE,	P_BOOL },
	{ "wrapscan",	"ws",		TRUE,	P_BOOL },
	{ "errorbells",	"eb",		FALSE,	P_BOOL },
	{ "showmode",	"mo",		FALSE,	P_BOOL },
	{ "backup",	"bk",		FALSE,	P_BOOL },
	{ "return",	"cr",		TRUE,	P_BOOL },
	{ "list",	"list",		FALSE,	P_BOOL },
	{ "ignorecase",	"ic",		FALSE,	P_BOOL },
	{ "autoindent",	"ai",		FALSE,	P_BOOL },
	{ "number",	"nu",		FALSE,	P_BOOL },
	{ "modelines",	"ml",		FALSE,	P_BOOL },
	{ "",		"",		0,	0, }		/* end marker */

};

static	void	showparms();

void
doset(arg, inter)
char	*arg;		/* parameter string */
bool_t	inter;		/* TRUE if called interactively */
{
	int	i;
	char	*s;
	bool_t	did_lines = FALSE;

	bool_t	state = TRUE;		/* new state of boolean parms. */

	if (arg == NULL) {
		showparms(FALSE);
		return;
	}
	if (strncmp(arg, "all", 3) == 0) {
		showparms(TRUE);
		return;
	}
	if (strncmp(arg, "no", 2) == 0) {
		state = FALSE;
		arg += 2;
	}

	for (i=0; params[i].fullname[0] != NUL ;i++) {
		s = params[i].fullname;
		if (strncmp(arg, s, strlen(s)) == 0)	/* matched full name */
			break;
		s = params[i].shortname;
		if (strncmp(arg, s, strlen(s)) == 0)	/* matched short name */
			break;
	}

	if (params[i].fullname[0] != NUL) {	/* found a match */
		if (params[i].flags & P_NUM) {
			did_lines = (i == P_LI);
			if (inter && (arg[strlen(s)] != '=' || state == FALSE))
				emsg("Invalid set of numeric parameter");
			else {
				params[i].value = atoi(arg+strlen(s)+1);
				params[i].flags |= P_CHANGED;
			}
		} else /* boolean */ {
			if (inter && (arg[strlen(s)] == '='))
				emsg("Invalid set of boolean parameter");
			else {
				params[i].value = state;
				params[i].flags |= P_CHANGED;
			}
		}
	} else {
		if (inter)
			emsg("Unrecognized 'set' option");
	}

	/*
	 * Update the screen in case we changed something like "tabstop"
	 * or "list" that will change its appearance.
	 */
	if (inter)
		updatescreen();

	if (did_lines) {
		Rows = P(P_LI);
		screenalloc();		/* allocate new screen buffers */
		screenclear();
		updatescreen();
	}
	/*
	 * Check the bounds for numeric parameters here
	 */
	if (P(P_TS) <= 0 || P(P_TS) > 32) {
		if (inter)
			emsg("Invalid tab size specified");
		P(P_TS) = 8;
		return;
	}

	if (P(P_SS) <= 0 || P(P_SS) > Rows) {
		if (inter)
			emsg("Invalid scroll size specified");
		P(P_SS) = 12;
		return;
	}

	/*
	 * Check for another argument, and call doset() recursively, if
	 * found. If any argument results in an error, no further
	 * parameters are processed.
	 */
	while (*arg != ' ' && *arg != '\t') {	/* skip to next white space */
		if (*arg == NUL)
			return;			/* end of parameter list */
		arg++;
	}
	while (*arg == ' ' || *arg == '\t')	/* skip to next non-white */
		arg++;

	if (*arg)
		doset(arg, inter);	/* recurse on next parameter */
}

static	void
showparms(all)
bool_t	all;	/* show ALL parameters */
{
	struct	param	*p;
	char	buf[64];

	gotocmd(TRUE, 0);
	outstr("Parameters:\r\n");

	for (p = &params[0]; p->fullname[0] != NUL ;p++) {
		if (!all && ((p->flags & P_CHANGED) == 0))
			continue;
		if (p->flags & P_BOOL)
			sprintf(buf, "\t%s%s\r\n",
				(p->value ? "" : "no"), p->fullname);
		else
			sprintf(buf, "\t%s=%d\r\n", p->fullname, p->value);

		outstr(buf);
	}
	wait_return();
}
