/* sqrt.c */

#include <math.h>
#include <errno.h>
extern int errno;

#define	SQRT2		1.4142135623730951455E0

/* initial minimum mean-squared-error linear approximation constants */
#define	C1	.582905362174731941742E0
#define	C0	.424749790911252694093E0

double sqrt(x)
double x;
{
 int exponent;
 double y;

 if(x < 0.0)
   {
    errno = EDOM;
    return 0;
   }

 if(x == 0)
    return 0;

 x = frexp(x, &exponent);		/* work in interval [.5,1) */

 y = C1 * x + C0;			/* start with linear approximation */

 y += x / y;				/* iterate twice */
 y = (0.25 * y) + (x / y);

 y += x / y;			
 y = (0.25 * y) + (x / y);

 if(exponent & 1)				
    y *= SQRT2;

 return ldexp(y, exponent >> 1);
}  
