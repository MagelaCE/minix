/****************************************************************/
/*								*/
/*	whoami(1)						*/
/*								*/
/*  		The login name of the current effective user	*/
/*		is written to the standard output stream.	*/
/*								*/
/****************************************************************/
/*   origination        1987-Sep-24              T. Holm	*/
/****************************************************************/


#include <stdio.h>
#include <pwd.h>


struct passwd *getpwuid();



main()
  {
  struct passwd *pw_entry;

  pw_entry = getpwuid( geteuid() );

  if ( pw_entry == NULL )
    exit( 1 );

  puts( pw_entry->pw_name );

  exit( 0 );
  }
