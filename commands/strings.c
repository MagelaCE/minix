/****************************************************************/
/*								*/
/*	strings.c						*/
/*								*/
/*		Find printable strings in a file.		*/
/*		This is the code for strings(1).		*/
/*								*/
/****************************************************************/
/*   origination        1988-Apr-14               T. Holm	*/
/****************************************************************/




/****************************************************************/
/*								*/
/*								*/
/*  Usage:    strings  [ -o ]  [ -number ]  [ file ]		*/
/*								*/
/*								*/
/*  The "file", or standard input if "file" is not specified,	*/
/*  is scanned for possible character strings. A "string" is	*/
/*  a sequence of 4 or more printable ASCII characters followed */
/*  by a '\0', '\n' or '\r'.					*/
/*								*/
/*								*/
/*    -o       Also display the character offset of the start	*/
/*	       of the string from the beginning of the file,	*/
/*	       in decimal.					*/
/*								*/
/*    -number  Minimum sequence length which is considered a	*/
/*	       string (the default is 4).			*/
/*								*/
/*								*/
/****************************************************************/




#include <stdio.h>


#define   FALSE			0
#define   TRUE 			1
#define   MAX_STRING_LENGTH	78





main( argc, argv )
  int   argc;
  char *argv[];

  {
  char  *my_name       = argv[0];   /*  The name of this program      */

  char   echo_position = FALSE;	    /*  "-o" means echo where found   */
  long   position      = 0;	    /*  Where string was found	      */

  int    string_size   = 4;	    /*  "-#" sets minimum string size */
  int    count         = 0;         /*  Size of string so far         */
  
  char   string[MAX_STRING_LENGTH]; /*  A possible printable string   */
  int    c;



  /********  Parse the arguments  ********/

  --argc;
  ++argv;


  if ( argc > 0  &&  strcmp( argv[0], "-o" ) == 0 )
    {
    echo_position = TRUE;

    --argc;
    ++argv;
    }


  if ( argc > 0  &&  *argv[0] == '-' )
    {
    string_size = atoi( argv[0]+1 );

    if ( string_size <= 0 )
      string_size = 1;

    --argc;
    ++argv;
    }


  if ( argc > 0 )
    {
    if( (freopen( argv[0], "r", stdin )) == NULL )
      {
      fprintf( stderr, "%s:  Can not open file \"%s\"\n", my_name, argv[0] );
      exit( 1 );
      }

    --argc;
    ++argv;
    }


  if ( argc > 0 )
    {
    fprintf( stderr, "Usage:  %s  [ -o ]  [ -number ]  [ file ]\n", my_name );
    exit( 1 );
    }




  /********  The main loop  ********/


  while( (c=getchar()) != EOF )
    {
    ++position;


    if (  c == '\0'  ||  c == '\n'  ||  c == '\r'  )
      {
      /*  Found the end of a printable string  */

      if ( count >= string_size )
	Print( string, count, echo_position, position - 1 - count );

      count = 0;
      }


    else if (  c >= ' '  &&  c < '\177'  ||  c == '\t'  )
      {
      /*  Append another printable character to the string  */

      string[count] = c;
      ++count;

      if ( count == MAX_STRING_LENGTH )
	{
	Print( string, count, echo_position, position - count );
	count = 0;
	}
      }


    else  /*  Not a printable character  */
      count = 0;

    }  /*  end while(getchar())  */

  exit( 0 );
  }








/****************************************************************/
/*								*/
/*	Print( string, count, echo_position, position )		*/
/*								*/
/*		If "echo_position" is TRUE then output the	*/
/*		"position" in the file. "count" printable	*/
/*		characters in "string" are output.		*/
/*								*/
/****************************************************************/


Print( string, count, echo_position, position )
  char   string[];
  int    count;
  char   echo_position;
  long   position;

  {
  if ( echo_position )
    printf( "%7D ", position );

  fwrite( string, 1, count, stdout );

  putchar( '\n' );
  }
