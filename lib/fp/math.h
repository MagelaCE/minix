/* math.h */

#ifndef _MATH_H
#define _MATH_H

#define HUGE_VAL	1.7976931348623159E+308

extern double sin(/* double theta */);
extern double cos(/* double theta */);
extern double tan(/* double theta */);
extern double asin(/* double x */);
extern double acos(/* double x */);
extern double atan(/* double x */);
extern double atan2(/* double x, double y */);
extern double sinh(/* double x */);
extern double cosh(/* double x */);
extern double tanh(/* double x */);
extern double exp(/* double x */);
extern double log(/* double x */);
extern double log10(/* double x */);
extern double pow(/* double x */);
extern double sqrt(/* double x */);
extern double ceil(/* double x */);
extern double floor(/* double x */);
extern double fabs(/* double x */);
extern double ldexp(/* double x, int n */);
extern double frexp(/* double x, int *exp */);
extern double modf(/* double x, double *ip */);
extern double fmod(/* double x, double y */);

#endif /* _MATH_H */
