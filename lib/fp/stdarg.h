/* stdarg.h */

#ifndef _STDARG_H
#define _STDARG_H

typedef char *va_list;

#define va_start(argp, lastarg)	((argp) = (char *)&(lastarg)+sizeof(lastarg))
#define va_arg(argp, type)	((type *)((argp) += sizeof(type)))[-1]
#define va_end(argp)

#endif /* _STDARG_H */
