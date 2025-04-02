static	char	RCSid[] =
"$Header: version.c,v 3.45 88/11/10 09:00:06 tony Exp $";

/*
 * Contains the declaration of the global version number variable.
 *
 * changes by Robert Regn:
 * Tue Nov 29 12:12:10 MET 1988
 * Better handling of Read/only files
 * preserving link structure and modes
 * recognizes % in cmdline - better algorithm for # also
 * edit an empty file is now possible
 * restore tty if the file to edit is too big
 * if writeit in :w fails, :q is not further possible
 *
 * $Log:	version.c,v $
 * Revision 3.45  88/11/10  09:00:06  tony
 * Added support for mode lines. Strings like "vi:stuff:" or "ex:stuff:"
 * occurring in the first or last 5 lines of a file cause the editor to
 * pretend that "stuff" was types as a colon command. This examination
 * is done only if the parameter "modelines" (or "ml") is set. This is
 * not enabled, by default, because of the security implications involved.
 * 
 * Revision 3.44  88/11/01  21:34:11  tony
 * Fixed a couple of minor points for Minix, and improved the speed of
 * the 'put' command dramatically.
 * 
 * Revision 3.43  88/10/31  13:11:33  tony
 * Added optional support for termcap. Initialization is done in term.c
 * and also affects the system-dependent files. To enable termcap in those
 * environments that support it, define the symbol "TERMCAP" in env.h
 * 
 * Revision 3.42  88/10/27  18:30:19  tony
 * Removed support for Megamax. Added '%' as an alias for '1,$'. Made the
 * 'r' command more robust. Now prints the string on repeated searches.
 * The ':=" command now works. Some pointer operations are now safer.
 * The ":!" and ":sh" now work correctly. Re-organized the help screens
 * a little.
 * 
 * Revision 3.41  88/10/06  10:15:00  tony
 * Fixed a bug involving ^Y that occurs when the cursor is on the last
 * line, and the line above the screen is long. Also hacked up fileio.c
 * to pass pathnames off to fixname() for system-dependent processing.
 * Used under DOS & OS/2 to trim parts of the name appropriately.
 * 
 * Revision 3.40  88/09/16  08:37:36  tony
 * No longer beeps when repeated searches fail.
 * 
 * Revision 3.39  88/09/06  06:51:07  tony
 * Fixed a bug with shifts that was introduced when replace mode was added.
 * 
 * Revision 3.38  88/08/31  20:48:28  tony
 * Made another fix in search.c related to repeated searches.
 * 
 * Revision 3.37  88/08/30  20:37:16  tony
 * After much prodding from Mark, I finally added support for replace mode.
 * 
 * Revision 3.36  88/08/26  13:46:34  tony
 * Added support for the '!' (filter) operator.
 * 
 * Revision 3.35  88/08/26  08:46:01  tony
 * Misc. changes to make lint happy.
 * 
 * Revision 3.34  88/08/25  15:13:36  tony
 * Fixed a bug where the cursor didn't land on the right place after
 * "beginning-of-word" searches if the word was preceded by the start
 * of the line and a single character.
 * 
 * Revision 3.33  88/08/23  12:53:08  tony
 * Fixed a bug in ssearch() where repeated searches ('n' or 'N') resulted
 * in dynamic memory being referenced after it was freed.
 * 
 * Revision 3.32  88/08/17  07:37:07  tony
 * Fixed a general problem in u_save() by checking both parameters for
 * null values. The specific symptom was that a join on the last line of
 * the file would crash the editor.
 * 
 * Revision 3.31  88/07/09  20:39:38  tony
 * Implemented the "line undo" command (i.e. 'U').
 * 
 * Revision 3.30  88/06/28  07:54:22  tony
 * Fixed a bug involving redo's of the '~' command. The redo would just
 * repeat the replacement last performed instead of switching the case of
 * the current character.
 * 
 * Revision 3.29  88/06/26  14:53:19  tony
 * Added support for a simple form of the "global" command. It supports
 * commands of the form "g/pat/d" or "g/pat/p", to delete or print lines
 * that match the given pattern. A range spec may be used to limit the
 * lines to be searched.
 * 
 * Revision 3.28  88/06/25  21:44:22  tony
 * Fixed a problem in the processing of colon commands that caused
 * substitutions of patterns containing white space to fail.
 * 
 * Revision 3.27  88/06/20  14:52:21  tony
 * Merged in changes for BSD Unix sent in by Michael Lichter.
 * 
 * Revision 3.26  88/06/10  13:44:06  tony
 * Fixed a bug involving writing out files with long pathnames. A small
 * fixed size buffer was being used. The space for the backup file name
 * is now allocated dynamically.
 * 
 * Revision 3.25  88/05/04  08:29:02  tony
 * Fixed a minor incompatibility with vi involving the 'G' command. Also
 * changed the RCS version number of version.c to match the actual version
 * of the editor.
 * 
 * Revision 1.12  88/05/03  14:39:52  tony
 * Changed the screen representation of the ascii character DELETE to be
 * compatible with vi. Also merged in support for DOS.
 * 
 * Revision 1.11  88/05/02  21:38:21  tony
 * The code that reads files now handles boundary/error conditions much
 * better, and generates status/error messages that are compatible with
 * the real vi. Also fixed a bug in repeated reverse searches that got
 * inserted in the recent changes to search.c.
 * 
 * Revision 1.10  88/05/02  07:35:41  tony
 * Fixed a bug in the routine plines() that was introduced during changes
 * made for the last version.
 * 
 * Revision 1.9  88/05/01  20:10:19  tony
 * Fixed some problems with auto-indent, and added support for the "number"
 * parameter.
 * 
 * Revision 1.8  88/04/30  20:00:49  tony
 * Added support for the auto-indent feature.
 * 
 * Revision 1.7  88/04/29  14:50:11  tony
 * Fixed a class of bugs involving commands like "ct)" where the cursor
 * motion part of the operator can fail. If the motion failed, the operator
 * was continued, with the cursor position unchanged. Cases like this were
 * modified to abort the operation if the motion fails.
 * 
 * Revision 1.6  88/04/28  08:19:35  tony
 * Modified Henry Spencer's regular expression library to support new
 * features that couldn't be done easily with the existing interface.
 * This code is now a direct part of the editor source code. The editor
 * now supports the "ignorecase" parameter, and multiple substitutions
 * per line, as in "1,$s/foo/bar/g".
 * 
 * Revision 1.5  88/04/24  21:38:00  tony
 * Added preliminary support for the substitute command. Full range specs.
 * are supported, but only a single substitution is allowed on each line.
 * 
 * Revision 1.4  88/04/23  20:41:01  tony
 * Worked around a problem with adding lines to the end of the buffer when
 * the cursor is at the bottom of the screen (in misccmds.c). Also fixed a
 * bug that caused reverse searches from the start of the file to bomb.
 * 
 * Revision 1.3  88/03/24  08:57:00  tony
 * Fixed a bug in cmdline() that had to do with backspacing out of colon
 * commands or searches. Searches were okay, but colon commands backed out
 * one backspace too early.
 * 
 * Revision 1.2  88/03/21  16:47:55  tony
 * Fixed a bug in renum() causing problems with large files (>6400 lines).
 * Also moved system-specific defines out of stevie.h and into a new file
 * named env.h. This keeps volatile information outside the scope of RCS.
 * 
 * Revision 1.1  88/03/20  21:00:39  tony
 * Initial revision
 * 
 */

char	*Version = "STEVIE - Version 3.45";
