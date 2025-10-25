/* clr - clear the screen		Author: Andy Tanenbaum */

main()
{
/* Clear the screen. */

  prints("\033[H\033[J");
  exit(0);
}
