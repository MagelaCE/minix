static	char	RCSid[] =
"$Header: term.c,v 1.2 88/10/31 13:10:32 tony Exp $";

/*
 * Termcap initialization (optional).
 *
 * $Log:	term.c,v $
 * Revision 1.2  88/10/31  13:10:32  tony
 * Removed text after "#endif" that some compilers complain about.
 * 
 * Revision 1.1  88/10/29  14:05:27  tony
 * Initial revision
 * 
 */

#include <stdio.h>
#include "stevie.h"

#ifdef	TERMCAP

static	char	buf[1024];	/* termcap entry read here */
static	char	cap[256];	/* capability strings go in here */

char	*T_EL;		/* erase the entire current line */
char	*T_IL;		/* insert one line */
char	*T_DL;		/* delete one line */
char	*T_SC;		/* save the cursor position */
char	*T_ED;		/* erase display (may optionally home cursor) */
char	*T_RC;		/* restore the cursor position */
char	*T_CI;		/* invisible cursor (very optional) */
char	*T_CV;		/* visible cursor (very optional) */

char	*T_CM;		/* cursor motion string */

extern	int	tgetent(), tgetnum();
extern	char	*tgetstr();
extern	char	*getenv();

int
t_init()
{
	char	*term;
	int	n;
	char	*cp = cap;

	if ((term = getenv("TERM")) == NULL)
		return 0;

	if (tgetent(buf, term) != 1)
		return 0;

	if ((n = tgetnum("li")) == -1)
		return 0;
	else
		P(P_LI) = Rows = n;

	if ((n = tgetnum("co")) == -1)
		return 0;
	else
		Columns = n;

	/*
	 * Get mandatory capability strings.
	 */
	if ((T_CM = tgetstr("cm", &cp)) == NULL)
		return 0;

	if ((T_EL = tgetstr("ce", &cp)) == NULL)
		return 0;

	if ((T_IL = tgetstr("al", &cp)) == NULL)
		return 0;

	if ((T_DL = tgetstr("dl", &cp)) == NULL)
		return 0;

	if ((T_ED = tgetstr("cl", &cp)) == NULL)
		return 0;

	if ((T_SC = tgetstr("sc", &cp)) == NULL)
		return 0;

	if ((T_RC = tgetstr("rc", &cp)) == NULL)
		return 0;

	/*
	 * Optional capabilities.
	 */
	if ((T_CI = tgetstr("vi", &cp)) == NULL)
		T_CI = "";

	if ((T_CV = tgetstr("ve", &cp)) == NULL)
		T_CV = "";

	return 1;
}

#endif
