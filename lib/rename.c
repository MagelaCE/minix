/* rename.c -- file renaming routine */

/* rename(from, to)
 * char *from, *to;
 */

int rename(from,to)
register char *from, *to;
{
    (void) unlink(to);
    if (link(from, to) < 0)
	return(-1);

    (void) unlink(from);
    return(0);
}

