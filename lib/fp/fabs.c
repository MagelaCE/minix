/* fabs.c */

#include <math.h>

/* this could be done even more cheaply in assembly */

double fabs(x)
double x;
{
 if(x < 0)
    return -x;
 else
    return x;
}
