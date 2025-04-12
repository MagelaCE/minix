/* _poly.c */

#include <math.h>

/* Polynomial evaluation using Horner's method. Should be called with
 * restricted-range |z| arguments, because no overflow checking is done.
 */
double _poly(z, coeff, ncoeff)
double z;
register double *coeff;
register int ncoeff;
{
 double result;

 coeff += ncoeff;
 result = *--coeff;

 while(--ncoeff > 0)
      {
       result = *--coeff + (z * result);
      }

 return result;
}
