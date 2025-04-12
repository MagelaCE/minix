/* file: strtod.c */

#include <ctype.h>
/* #include <stddef.h> */
/* #include <stdlib.h> */

#ifndef NULL
#define NULL 0
#endif

#define MSIGN	1		/* mantissa has negative sign */
#define ESIGN	2		/* exponent has negative sign */
#define DECPT	4		/* decimal point encountered */

double strtod(s, endp)
register char *s;
char **endp;
{
 int flags = 0;
 int decexp = 0;		/* decimal exponent */
 double value = 0;		/* accumulated value - actually */
				/* a 64-bit integer */

 extern int _mul10add(/* double *valuep, int digit */);
 extern double _adjust(/* double *valuep, int decexp, int negflag */);

 while(isspace(*s))		/* skip leading white space */
       ++s;

 if(*s == '+')
    ++s;
 else if(*s == '-')
   {
    ++s;
    flags |= MSIGN;		/* mantissa is negative */
   }

 for(; ; ++s)
    {
     if(isdigit(*s))
       {
	if(_mul10add(&value, *s - '0'))
  	   ++decexp;
	if(flags & DECPT)
	   --decexp;
       }
     else if(*s == '.')
       {
	flags |= DECPT;
       }
     else
	break;
    }

 if(*s == 'e' || *s == 'E')
   {
    int eacc = 0;		/* exponent accumulator */

    ++s;
  
    if(*s == '+')
       ++s;
    else if(*s == '-')
      {
       ++s;
       flags |= ESIGN;		/* decimal exponent is negative */
      }
    
    while(isdigit(*s))
         {
	  if(eacc < 1000)
	     eacc = eacc * 10 + (*s - '0');
          ++s;
         }

    if(flags & ESIGN)
       decexp -= eacc;
    else
       decexp += eacc;	
   }

 if(decexp > 350)		/* outrageously large */
    decexp = 350;
 else if(decexp < -350)		/* outrageously small */
    decexp = -350;
 
 if(endp != (char **)NULL)	/* store endp if desired */
    *endp = s;

 if(value == 0)
    return 0;
 else
    return _adjust(&value, decexp, flags & MSIGN);
}
