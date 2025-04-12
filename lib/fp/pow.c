/* pow.c */

#include <limits.h>
#include <math.h>
#include <errno.h>
extern int errno;

extern double _mult(/* double x, double y */);

double pow(x, y)
double x, y;
{
 int sign = 0;
 double power, result;

 if(x == 0)
   {
    if(y <= 0)
      {
       errno = EDOM;
       return HUGE_VAL;
      }
    else
       return 0;
   }
 else if(x < 0)
  {
   if(floor(y) != y || y < (double) INT_MIN || y > (double) INT_MAX)
     {
      errno = EDOM;
      return HUGE_VAL;
     }
   if(((int) y) & 1)			/* odd power */
      sign = 1;
   x = -x;
  }

 /* we could save two multiplications (or so) by 'inlining' the code for
  * exp() and log() at this point. It's not worth it. */

 if((power = _mult(y, log(x))) == HUGE_VAL)
   {
    return HUGE_VAL;			/* with errno = ERANGE */
   }
 
 result = exp(power);

 return sign ? -result : result;
}
