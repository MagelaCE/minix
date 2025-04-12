/* _mult.c */

#include <math.h>

/* "safe" multiply. Returns x * y if in range, and HUGE_VAL with proper
 * sign if not.
 */
double _mult(x, y)
double x, y;
{
 int exponent;

 x = frexp(x, &exponent);
 return ldexp(x * y, exponent);
} 
