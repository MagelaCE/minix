static	char	RCSid[] =
"$Header: hexchars.c,v 1.3 88/08/26 08:45:10 tony Exp $";

/*
 * Contains information concerning the representation of characters for
 * visual output by the editor.
 *
 * $Log:	hexchars.c,v $
 * Revision 1.3  88/08/26  08:45:10  tony
 * Misc. changes to make lint happy.
 * 
 * Revision 1.2  88/05/03  14:38:43  tony
 * Fixed the representation for the ascii character DELETE to be the
 * same as vi.
 * 
 * Revision 1.1  88/03/20  21:07:42  tony
 * Initial revision
 * 
 *
 */

#include "stevie.h"

/*
 * This file shows how to display characters on the screen. This is
 * approach is something of an overkill. It's a remnant from the
 * original code that isn't worth messing with for now. TABS are
 * special-cased depending on the value of the "list" parameter.
 */

struct charinfo chars[] = {
	/* 000 */	1, NULL,
	/* 001 */	2, "^A",
	/* 002 */	2, "^B",
	/* 003 */	2, "^C",
	/* 004 */	2, "^D",
	/* 005 */	2, "^E",
	/* 006 */	2, "^F",
	/* 007 */	2, "^G",
	/* 010 */	2, "^H",
	/* 011 */	2, "^I",
	/* 012 */	7, "[ERROR]",	/* shouldn't occur */
	/* 013 */	2, "^K",
	/* 014 */	2, "^L",
	/* 015 */	2, "^M",
	/* 016 */	2, "^N",
	/* 017 */	2, "^O",
	/* 020 */	2, "^P",
	/* 021 */	2, "^Q",
	/* 022 */	2, "^R",
	/* 023 */	2, "^S",
	/* 024 */	2, "^T",
	/* 025 */	2, "^U",
	/* 026 */	2, "^V",
	/* 027 */	2, "^W",
	/* 030 */	2, "^X",
	/* 031 */	2, "^Y",
	/* 032 */	2, "^Z",
	/* 033 */	2, "^[",
	/* 034 */	2, "^\\",
	/* 035 */	2, "^]",
	/* 036 */	2, "^^",
	/* 037 */	2, "^_",
	/* 040 */	1, NULL,
	/* 041 */	1, NULL,
	/* 042 */	1, NULL,
	/* 043 */	1, NULL,
	/* 044 */	1, NULL,
	/* 045 */	1, NULL,
	/* 046 */	1, NULL,
	/* 047 */	1, NULL,
	/* 050 */	1, NULL,
	/* 051 */	1, NULL,
	/* 052 */	1, NULL,
	/* 053 */	1, NULL,
	/* 054 */	1, NULL,
	/* 055 */	1, NULL,
	/* 056 */	1, NULL,
	/* 057 */	1, NULL,
	/* 060 */	1, NULL,
	/* 061 */	1, NULL,
	/* 062 */	1, NULL,
	/* 063 */	1, NULL,
	/* 064 */	1, NULL,
	/* 065 */	1, NULL,
	/* 066 */	1, NULL,
	/* 067 */	1, NULL,
	/* 070 */	1, NULL,
	/* 071 */	1, NULL,
	/* 072 */	1, NULL,
	/* 073 */	1, NULL,
	/* 074 */	1, NULL,
	/* 075 */	1, NULL,
	/* 076 */	1, NULL,
	/* 077 */	1, NULL,
	/* 100 */	1, NULL,
	/* 101 */	1, NULL,
	/* 102 */	1, NULL,
	/* 103 */	1, NULL,
	/* 104 */	1, NULL,
	/* 105 */	1, NULL,
	/* 106 */	1, NULL,
	/* 107 */	1, NULL,
	/* 110 */	1, NULL,
	/* 111 */	1, NULL,
	/* 112 */	1, NULL,
	/* 113 */	1, NULL,
	/* 114 */	1, NULL,
	/* 115 */	1, NULL,
	/* 116 */	1, NULL,
	/* 117 */	1, NULL,
	/* 120 */	1, NULL,
	/* 121 */	1, NULL,
	/* 122 */	1, NULL,
	/* 123 */	1, NULL,
	/* 124 */	1, NULL,
	/* 125 */	1, NULL,
	/* 126 */	1, NULL,
	/* 127 */	1, NULL,
	/* 130 */	1, NULL,
	/* 131 */	1, NULL,
	/* 132 */	1, NULL,
	/* 133 */	1, NULL,
	/* 134 */	1, NULL,
	/* 135 */	1, NULL,
	/* 136 */	1, NULL,
	/* 137 */	1, NULL,
	/* 140 */	1, NULL,
	/* 141 */	1, NULL,
	/* 142 */	1, NULL,
	/* 143 */	1, NULL,
	/* 144 */	1, NULL,
	/* 145 */	1, NULL,
	/* 146 */	1, NULL,
	/* 147 */	1, NULL,
	/* 150 */	1, NULL,
	/* 151 */	1, NULL,
	/* 152 */	1, NULL,
	/* 153 */	1, NULL,
	/* 154 */	1, NULL,
	/* 155 */	1, NULL,
	/* 156 */	1, NULL,
	/* 157 */	1, NULL,
	/* 160 */	1, NULL,
	/* 161 */	1, NULL,
	/* 162 */	1, NULL,
	/* 163 */	1, NULL,
	/* 164 */	1, NULL,
	/* 165 */	1, NULL,
	/* 166 */	1, NULL,
	/* 167 */	1, NULL,
	/* 170 */	1, NULL,
	/* 171 */	1, NULL,
	/* 172 */	1, NULL,
	/* 173 */	1, NULL,
	/* 174 */	1, NULL,
	/* 175 */	1, NULL,
	/* 176 */	1, NULL,
	/* 177 */	2, "^?",
};
