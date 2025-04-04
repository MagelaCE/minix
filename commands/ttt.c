/***** Noughts and Crosses ****/

/* Copyright (C) 1988 Warren Toomey.
	You may use, copy, modify, or give this away provided you 
		1. make the source available with every copy. 
		2. include this notice. 
		3. don't use this for military purposes.
	(but you can take credit for improvements, etc.)
*/

/* Compile with cc -o tic tic.c -lcurses -ltermcap */

#define CURSES
#ifdef CURSES
/*  #include <curses.h> 		Used by the real curses  */
#endif

#ifndef CURSES
#define printw printf
#endif


typedef struct { int value;		/* The move returned by the    */
		 int path;		/* alphabeta consists of a value */
	       } MOVE;			/* and an actual move (path)   */


	/* Static evaluator. Returns 100 if we have 3 in a row
				    -100 if they have 3 in a row

	  Game board is array of 9 ints, where 0=empty square
					       1=our move
					       4= their move

	  and board is indices	0 1 2
				3 4 5
				6 7 8
	*/


int stateval(board,whosemove)
 int board[];
 
 {
  static int row[8][3]= { {0,1,2}, {3,4,5}, {6,7,8}, /* Indices of 3in-a-rows */
		   	  {0,3,6}, {1,4,7}, {2,5,8},
		   	  {0,4,8}, {2,4,6} };

 int temp;					/* Temp row results */
 int i,j;					/* Loop counters */
 int side;					/* Depth multiplier */
 int win,lose;

 if (whosemove==1) {win=100; lose= -100; side= 1;}	/* Multiply by -1 if */
 else {win= -100; lose=100; side= -1;}			/* not out move */
 for (i=0;i<8;i++)				/* For every 3-in-a-row */
    {
     temp=0;
     for (j=0;j<3;j++)				/* Add up the board values */
	temp += board[row[i][j]];
	
     if (temp==3) return(win);			/* We've got 3 in a row */
     if (temp==12) return(lose);		/* They've got 3 in a row */
    }
  return(0);					/* Finally return sum */
 }


MOVE alphabeta(board,whosemove,alpha,beta)	/* Alphabeta: takes a board, */
 int board[];				/* whose move, alpha & beta cutoffs, */
 int whosemove;				/* and returns a move to make and */
 int alpha;				/* the value that the move has */
 int beta;
 {
  MOVE result,successor;
  int best_score,i,best_path,mademove;

  result.value=stateval(board,whosemove);	/* Work out the board's */
  						/* static value */
  if ((result.value==100)||			/* If a win or loss already */
      (result.value==-100)) return(result);	/* return the result */

  best_score= beta;				/* Ok, set worst score */
  mademove=0;					/* to the beta cutoff */
  for (i=0;i<9;i++)
   { if (board[i]==0)				/* For all valid moves */
       { mademove=1;
         board[i]=whosemove;			/* make the move on board */
	 successor=alphabeta(board,5-whosemove,-best_score-1,-alpha-1);
						/* Get value of the move */
	 board[i]=0;				/* Take move back */
	 if (-successor.value>best_score)	/* If a better score */
	   { best_score= -successor.value;	/* update our score */
	     best_path=i;			/* and move */
             if (best_score>alpha) break;	/* If we've beaten alpha */
	   }					/* return immediately */
       }
   }
  if (mademove)
    { result.value=best_score;		/* Finally return best score */
      result.path=best_path;		/* and best move */
    }
  return(result);			/* If no move, return static result */
 }


draw(board)				/* Draw the board */
 int board[];
 {
  int i,j,row;
  static char out[]=" X  O";		/* Lookup table for character */

  row=6;
#ifdef CURSES
  move(row,0);
#endif
  for (j=0;j<9;j+=3)
   {
     printw(" %d | %d | %d     ",j,j+1,j+2);
     for (i=0;i<3;i++)
        { printw("%c ",out[board[j+i]]);
          if (i<2) printw("| ");
        }
     if (j<4) 
       {
#ifdef CURSES
         move(++row,0);
#else
         printw("\n");
#endif
         printw("---+---+---   ---+---+---");
       }
#ifdef CURSES
     move(++row,0);
#else
     printw("\n");
#endif
   }
#ifdef CURSES
  refresh();
#else
  printw("\n");
#endif
 }


getmove(board)				/* Get a player's move */
 int board[];
 {
  int Move;

  do 
   {
    do {
#ifdef CURSES
        move(9,40);
        printw("Your move: ");		/* Prompt for move */
        refresh();
#else
        printw("Your move: ");		/* Prompt for move */
#endif
       }
    while (scanf("%d",&Move)!=1);	/* Input the move */
   }
  while (board[Move]);
  board[Move]=4;			/* If legal, add to board */
  draw(board);				/* Draw the board */
 }


int endofgame(board)			/* Determine end of the game */
 int board[];
 {
  int eval;
  int count;

  eval=stateval(board,1);
#ifdef CURSES
  move(20,25);
#endif
  if (eval==100) { printw("I have beaten you.\n"); return(1);}
  if (eval==-100) { printw("Bus error (core dumped)\n"); return(1);}
  count=0;
  for (eval=0;eval<9;eval++) if (board[eval]!=0) count++;
  if (count==9) { printw("A draw!\n"); return(1);}
#ifdef CURSES
  refresh();
#endif
  return(0);
 }


int randommove()			/* Make an initial random move */
 {
  long time();				/* based on current time */
  int i;

  i=abs((int) time((long *)0));
  return(i%9);
 }


main()					/* The actual game */
 {
  int i,board[9];
  char ch;
  MOVE ourmove;

  for (i=0;i<9;i++) board[i]=0;		/* Initialise the board */
#ifdef CURSES
  initscr();
  clear();
  refresh();
#endif
  printw("                           TIC TAC TOE   \n\n");
  printw("                        Your moves are 'O'\n");
  printw("                         My moves are 'X'\n\n");
#ifdef CURSES
  move(5,0);
  printw("Do you wish to move first: ");
  refresh();
  while (scanf("%c",&ch)!=1);
  move(5,0);
  printw("                         .......");		/* Kludge to get rid */
  refresh();
  move(5,0);
  printw("                                ");		/* of input letter */
  refresh();
#else
  do printw("Do you wish to move first: ");
  while (scanf("%c",&ch)!=1);
#endif
  if ((ch!='y')&& (ch!='Y'))
    { i=randommove();				/* If we move first */
      board[i]=1;				/* make it random */
#ifdef CURSES
      move(7,42);
      printw("My move: %d\n",i);
      refresh();
#else
      printw("My move: %d\n",i);
#endif
    }
  draw(board);
  getmove(board);

  while (1)
   { ourmove=alphabeta(board,1,99,-99);	/* Get a move for us; return wins */
					/* immediately & ignore losses */
     board[ourmove.path]=1;		/* and make it */
#ifdef CURSES
     move(7,42);
     printw("My move: %d\n",ourmove.path);
     refresh();
#else
     printw("My move: %d\n",ourmove.path);
#endif
     draw(board);
     if (endofgame(board)) break;	/* If end of game, exit */
     getmove(board);			/* Get opponent's move */
     if (endofgame(board)) break;	/* If end of game, exit */
   }
#ifdef CURSES
  endwin();
#endif
 }
