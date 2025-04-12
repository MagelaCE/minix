/* exp.c */

#include <limits.h>
#include <math.h>
#include <errno.h>
extern int errno;

extern double _poly(/* double z, double coeff[], int ncoeff */);

#define	LOG2E	1.4426950408889633870E0			/* 1/(ln 2) */
#define LOG2	6.9314718055994530942705877E-1		/* ln 2 */

static double expcoeff[] =
       {
	1.0000000000000000000E0,	/* 1/0! */
	1.0000000000000000000E0,	/* 1/1! */
	0.5000000000000000000E0,	/* 1/2! and so on */
	0.1666666666666666667E0,
	0.0416666666666666667E0,
	0.0083333333333333333E0,
	0.0013888888888888889E0,
	1.9841269841269841270E-4,
	2.4801587301587301587E-5,
	2.7557319223985894396E-6,
	2.7557319223985894396E-7,
	2.5052108385441722582E-8,
	2.0876756987868101412E-9,	/* after 1/10! things get dubious. */
	1.6059043836821618179E-10,
	1.1470745597729725684E-11,
	7.6471637318198171229E-13,
	4.7794773323873860349E-14,
	2.8114572543455219389E-15,
	1.5619206968586232698E-16,
       };

#define	NCOEFF	((sizeof expcoeff) / sizeof(double))

double exp(x)
double x;
{
 double ipart, frac;

 frac = modf(x * LOG2E, &ipart);

 if(frac > 0.5)
   {
    ipart += 1.0;
    frac -= 1.0;
   }
 else if(frac < -0.5)
   {
    ipart -= 1.0;
    frac += 1.0;
   }

 if(ipart < (double) INT_MIN)		/* really small */
    return 0;
 else if(ipart > (double) INT_MAX)	/* really big */
    ipart = (double) INT_MAX;

 return ldexp(_poly(frac * LOG2, expcoeff, NCOEFF), (int) ipart);
}
