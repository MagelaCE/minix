/* The <math.h> header contains prototypes for mathematical functions. */

#ifndef _MATH_H
#define _MATH_H

#define HUGE_VAL	9.9e+999	/* though it will generate a warning */

/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif
	
_PROTOTYPE( double acos,  (double __x)					);
_PROTOTYPE( double asin,  (double __x)					);
_PROTOTYPE( double atan,  (double __x)					);
_PROTOTYPE( double atan2, (double __y, double __x)			);
_PROTOTYPE( double ceil,  (double __x)					);
_PROTOTYPE( double cos,   (double __x)					);
_PROTOTYPE( double cosh,  (double __x)					);
_PROTOTYPE( double exp,   (double __x)					);
_PROTOTYPE( double fabs,  (double __x)					);
_PROTOTYPE( double floor, (double __x)					);
_PROTOTYPE( double fmod,  (double __x, double __y)			);
_PROTOTYPE( double frexp, (double __x, int *__exp)			);
_PROTOTYPE( double ldexp, (double __x, int __exp)			);
_PROTOTYPE( double log,   (double __x)					);
_PROTOTYPE( double log10, (double __x)					);
_PROTOTYPE( double modf,  (double __x, double *__iptr)			);
_PROTOTYPE( double pow,   (double __x, double __y)			);
_PROTOTYPE( double sin,   (double __x)					);
_PROTOTYPE( double sinh,  (double __x)					);
_PROTOTYPE( double sqrt,  (double __x)					);
_PROTOTYPE( double tan,   (double __x)					);
_PROTOTYPE( double tanh,  (double __x)					);

#endif /* _MATH_H */
