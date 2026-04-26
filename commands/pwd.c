/*
 * Print Working Directory (pwd)
 */

#include <limits.h>

char dir[PATH_MAX+1];

main()
{
	printf("%s\n", getcwd(dir,PATH_MAX));
}
