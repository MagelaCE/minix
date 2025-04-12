/* log10.c */

#include <math.h>

#define LOG10	 2.3025850929940456790E0

double log10(x)
double x;
{
 double alog;

 if((alog = log(x)) == -HUGE_VAL)
    return alog;			/* with errno set to EDOM */
 return alog * LOG10;
}
