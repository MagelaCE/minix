/* floor.c */

#include <math.h>

double floor(x)
double x;
{
 double fract;
 double ipart;

 fract = modf(x, &ipart);

 if(fract < 0.0)
    return ipart - 1.0;
 else
    return ipart;
}
