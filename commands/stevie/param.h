/*
 * $Header: param.h,v 1.5 88/11/10 08:57:56 tony Exp $
 *
 * Settable parameters
 *
 * $Log:	param.h,v $
 * Revision 1.5  88/11/10  08:57:56  tony
 * Added a new parameter, modelines ("ml"), to support the modeline
 * feature.
 * 
 * Revision 1.4  88/05/01  20:08:29  tony
 * Added the new boolean parameter "number" to turn on line numbering.
 * 
 * Revision 1.3  88/04/30  20:00:42  tony
 * Added the parameter "autoindent".
 * 
 * Revision 1.2  88/04/28  08:06:48  tony
 * Added a new parameter "ignorecase" which can be set to ignore case
 * distinctions in string searches.
 * 
 * Revision 1.1  88/03/20  21:04:50  tony
 * Initial revision
 * 
 *
 */

struct	param {
	char	*fullname;	/* full parameter name */
	char	*shortname;	/* permissible abbreviation */
	int	value;		/* parameter value */
	int	flags;
};

extern	struct	param	params[];

/*
 * Flags
 */
#define	P_BOOL		0x01	/* the parameter is boolean */
#define	P_NUM		0x02	/* the parameter is numeric */
#define	P_CHANGED	0x04	/* the parameter has been changed */

/*
 * The following are the indices in the params array for each parameter
 */

/*
 * Numeric parameters
 */
#define	P_TS		0	/* tab size */
#define	P_SS		1	/* scroll size */
#define	P_RP		2	/* report */
#define	P_LI		3	/* lines */

/*
 * Boolean parameters
 */
#define	P_VB		4	/* visual bell */
#define	P_SM		5	/* showmatch */
#define	P_WS		6	/* wrap scan */
#define	P_EB		7	/* error bells */
#define	P_MO		8	/* show mode */
#define	P_BK		9	/* make backups when writing out files */
#define	P_CR		10	/* use cr-lf to terminate lines on writes */
#define	P_LS		11	/* show tabs and newlines graphically */
#define	P_IC		12	/* ignore case in searches */
#define	P_AI		13	/* auto-indent */
#define	P_NU		14	/* number lines on the screen */
#define	P_ML		15	/* enables mode-lines processing */

/*
 * Macro to get the value of a parameter
 */
#define	P(n)	(params[n].value)
