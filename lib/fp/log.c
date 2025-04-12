/* log.c */

#include <math.h>
#include <errno.h>
extern int errno;

extern double _poly(/* double z, double coeff[], int ncoeff */);

/* the commented out number is the real answer, and the other is the
 * answer that makes log(1.0) come out to zero. Bleah!
 */
/* #define LOG2		6.9314718055994530942705877E-1 */
#define	LOG2		6.931471802121982E-1

/* These coefficients came from the BSD libm's log__L.c file */
static double logcoeff[] =
       {
	2.0,
	6.6666666666667340202E-1,
	3.9999999999416702146E-1,
	2.8571428742008753154E-1,
	2.2222198607186277597E-1,
	1.8183562745289935658E-1,
	1.5314087275331442206E-1,
	1.4795612545334174692E-1
       };

#define	NCOEFF	((sizeof logcoeff) / sizeof(double))

double log(x)
double x;
{
 double fract;
 double ratio;
 int exponent;

 if(x <= 0.0)
   {
    errno = EDOM;
    return -HUGE_VAL;
   }

 fract = frexp(x, &exponent);

 ratio = (fract - 1.0) / (fract + 1.0);

 x = exponent + ratio * _poly(ratio * ratio, logcoeff, NCOEFF) / LOG2;

 return x * LOG2;
}
