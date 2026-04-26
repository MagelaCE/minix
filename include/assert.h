/* The <assert.h> header contains a macro called "assert" that allows 
 * programmers to put assertions in the code.  These assertions can be verified
 * at run time.  If an assertion fails, an error message is printed.  
 * Assertion checking can be disabled by adding the statement
 *
 *	#define NDEBUG
 *
 * to the program before the 
 *
 *	#include <assert.h>
 *
 * statement.
 */

#ifdef assert
#undef assert			/* make this file idempotent */
#endif

#ifdef NDEBUG
/* Debugging disabled -- do not evaluate assertions. */
#define assert(expr)  ((void) 0)
#else
/* Debugging enabled -- verify assertions at run time. */
#define assert(expr) ((void) ((expr) ? 0 : __assert(__FILE__,  __LINE__)))
#endif
