/* ELLE - Copyright 1984, 1987 by Ken Harrenstien, SRI International
 *	This software is quasi-public; it may be used freely with
 *	like software, but may NOT be sold or made part of licensed
 *	products without permission of the author.
 */
/*
 * EESUN	SUN workstation support (mostly window/mouse routines)
 *	This code created 1984 by Andy Poggio, SRI International
 */

#include "elle.h"

#ifdef SUN		/* This conditional covers the entire file */

#include <sys/file.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "suntool/tool_hs.h"
#include "suntool/menu.h"
#include <sunwindow/cms.h>
#include <sunwindow/win_struct.h>
#include "sunwindow/win_ioctl.h"
#include "suntool/wmgr.h"
#include "suntool/ttytlsw.h"
#include "suntool/selection.h"
#include <pwd.h>

#define MAXNAMLEN 255
#define BUTTON_MASK( button) (1<<(button - BUT_FIRST))

/* Exported functions */
void sun_main(), sun_init();
int sun_input();
int f_stuffsel(), f_selregion();

/* Internal functions */
static void sun_submain(), sun_handle_mouse(), sun_interpret_mouse();
static void sel_read(), sel_write(), sel_clear();
static int sigwinchcatcher(), sigchldcatcher();
static int win_xydist();


/* Data decls */
static char *toolname = "Elle Tool 2.0";
static char sub_arg[] = "-SubWinProc";

static struct tool *tool;
static struct toolsw *ttysw;
static int pix_per_row, pix_per_col;



/* ELLE menu data declarations.
 * Init code is in eecmds.c, function init_menu()
 */
struct menu *menuptr;
struct menu menu = { MENU_IMAGESTRING, "Elle Menu", 0, 0, 0, 0};


/* Define cursor image */
static unsigned	cursor_data[8] = {
			0x18003C00, 0x7E00FF00, 0xFF00FF00, 0xFF00FF00,
			0xFF00FF00, 0xFF00FF00, 0xFF007E00, 0x3C001800
		};
mpr_static( currect, 8, 16, 1, cursor_data);    

static struct cursor e_cursor = {
    4, 7, 			/* hot spot */
    (PIX_SRC ^ PIX_DST),
    &currect			/* Point to cursor image */
};

/* Define Icon image */
static short icon_data[256] = {
	0x0000, 0x0000, 0x0000, 0x8000, 0x0000, 0x0000, 0x0001, 0x0000,
	0x07FF, 0xFFFF, 0xFFFF, 0xE000, 0x0400, 0x0000, 0x0002, 0x2100,
	0x0400, 0x0000, 0x0002, 0x2200, 0x047C, 0x4040, 0x7C04, 0x2400,
	0x0440, 0x4040, 0x4004, 0x2800, 0x0440, 0x4040, 0x4008, 0x3000,
	0x0440, 0x4040, 0x4010, 0x2000, 0x0470, 0x4040, 0x7010, 0x6000,
	0x0440, 0x4040, 0x4020, 0xA000, 0x0440, 0x4040, 0x4041, 0x2000,
	0x0440, 0x4040, 0x4042, 0x2000, 0x047C, 0x7C7C, 0x7C84, 0x2000,
	0x0400, 0x0000, 0x0088, 0x2000, 0x0400, 0x0000, 0x0110, 0x2000,
	0x0400, 0x0000, 0x0220, 0x2000, 0x0400, 0x0000, 0x0240, 0x2000,
	0x0400, 0x0000, 0x0480, 0x2000, 0x0400, 0x0000, 0x0900, 0x2000,
	0x0400, 0x0000, 0x1200, 0x2000, 0x0400, 0x0000, 0x2400, 0x2000,
	0x0400, 0x0000, 0x2800, 0x2000, 0x0400, 0x0000, 0x5000, 0x2000,
	0x0400, 0x0000, 0xA000, 0x2000, 0x0400, 0x0000, 0xC000, 0x2000,
	0x0400, 0x0001, 0x8000, 0x2000, 0x0400, 0x0001, 0x0000, 0x2000,
	0x0400, 0x0002, 0x0000, 0x2000, 0x0400, 0x0006, 0x0000, 0x2000,
	0x0400, 0x000C, 0x0000, 0x2000, 0x0400, 0x0014, 0x0000, 0x2000,
	0x0400, 0x0024, 0x0000, 0x2000, 0x0400, 0x0048, 0x0000, 0x2000,
	0x0400, 0x0088, 0x0000, 0x2000, 0x0400, 0x0108, 0x0000, 0x2000,
	0x0400, 0x0218, 0x00C0, 0x2000, 0x0400, 0x0410, 0x00C0, 0x2000,
	0x0400, 0x0810, 0x00C0, 0x2000, 0x0400, 0x7878, 0x3FFE, 0x2000,
	0x0401, 0x88C8, 0x3FFE, 0x2000, 0x040E, 0x0888, 0x1004, 0x2000,
	0x0418, 0x0984, 0x1004, 0x2000, 0x0410, 0x1104, 0x13E4, 0x2000,
	0x0430, 0x1102, 0x1224, 0x2000, 0x0420, 0x1302, 0x1224, 0x2000,
	0x0460, 0x1202, 0x13E4, 0x2000, 0x0440, 0x2402, 0x1204, 0x2000,
	0x0420, 0x2404, 0x1204, 0x2000, 0x0420, 0x4404, 0x1204, 0x2000,
	0x0430, 0x8408, 0x1004, 0x2000, 0x041F, 0x8630, 0x1004, 0x2000,
	0x0400, 0x03C0, 0x1FFC, 0x2000, 0x0400, 0x0000, 0x0000, 0x2000,
	0x0400, 0x0000, 0x0000, 0x2000, 0x0400, 0x0000, 0x0000, 0x2000,
	0x0400, 0x0000, 0x0000, 0x2000, 0x0400, 0x0000, 0x0000, 0x2000,
	0x0400, 0x0000, 0x0000, 0x2000, 0x0400, 0x0000, 0x0000, 0x2000,
	0x0400, 0x0000, 0x0000, 0x2000, 0x07FF, 0xFFFF, 0xFFFF, 0xE000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};
mpr_static(my_icon_pr, 64, 64, 1, icon_data);
static struct icon  my_icon = {
		TOOL_ICONWIDTH, TOOL_ICONHEIGHT, 0,
		{0, 0, TOOL_ICONWIDTH, TOOL_ICONHEIGHT},
		&my_icon_pr, {0, 0, 0, 0}, 0, 0, 0
};


/* SUN Initialization code.
 *	sun_main is called as the first thing in main(), before anything
 *		else!  If not running under the window system, nothing
 *		happens and none of the other routines are called either.
 *		This routine starts up an inferior fork that then
 *		calls sun_submain().
 *		The actual editor operation happens in the inferior.
 *		
 *	sun_init is called during normal startup initialization, after
 *		the initial buffers have been set up and the profile read.
 *		It merely initializes the menu (from profile) and gets the
 *		ELLE window size.
 *		
 */
void
sun_main(pargc, argv)	/* Called at very beginning of main() */
int *pargc;		/* Note pointer, not value!  So we can mung it. */
char *argv[];
{
	register int i;
	int argc;
	char wname[WIN_NAMESIZE];
#define MAXARGS 255
	char *nargv[MAXARGS+2];
	int pwinnum;
	struct rect screen_rect, invoke_rect, my_rect;
	char prog_name[ MAXNAMLEN];

	/* Find out if we are running in the window system,
	 * by trying to get a fd for the current window.
	 */
	if ((we_getmywindow(wname) != 0)
	 || (sun_winfd = open(wname, O_RDWR)) == -1)
	  {	sun_winfd = 0;		/* No window */
		return;
	  }

	/* Find out if main or sub process, by seeing if the
	 * last argument matches special string
	 */
	i = *pargc - 1;
	if(i > 0 && (strcmp(argv[i],sub_arg)==0))
	  {	*pargc = i;	/* Yes!  Decrement arg count */
		argv[i] = 0;	/* And ensure it's flushed from arg vector */
		sun_submain();
	  }

	/* Must be main program! */
	argc = *pargc;

	/*
	 * Find out about invoking and enclosing windows to size new window
	 */
	/* invoking window -- parent */
	pwinnum = win_getlink( sun_winfd, WL_PARENT);
	win_numbertoname( pwinnum, wname);
	close( sun_winfd);
	if(( sun_winfd = open( wname, O_RDONLY)) == -1)
	    printf( "cannot open window");
	if( win_getuserflags( sun_winfd) & WMGR_ICONIC)
	    win_getsavedrect( sun_winfd, &invoke_rect);
	else
	    win_getrect( sun_winfd, &invoke_rect);
	close( sun_winfd);

	/* enclosing window -- screen */
	if( we_getparentwindow( wname) != 0)
	    screen_rect.r_height = 0;
	else if(( sun_winfd = open( wname, O_RDONLY)) == -1)
	    screen_rect.r_height = 0;
	else {
	    win_getrect( sun_winfd, &screen_rect);
	    close( sun_winfd);
	}

	/*
	 * Create tool window and set its rect
	 */
	my_rect = invoke_rect;
	if ( screen_rect.r_height) {
	    my_rect.r_top +=   /* WMGR_NEXTTOOLYOFFSET */ 0;
	    my_rect.r_left +=  /* WMGR_NEXTTOOLXOFFSET */ 0;
	    if( ! rect_includesrect( &screen_rect, &my_rect))
					/* will not fit */
		my_rect = invoke_rect;  /* just overlay invoker */
	}
	
	tool = tool_create(toolname, TOOL_NAMESTRIPE, 0, &my_icon);
	sun_winfd = tool->tl_windowfd;
	win_setrect( sun_winfd, &my_rect);

	/*
	 * Create tty subwindow
	 */
	ttysw = ttytlsw_createtoolsubwindow(tool, "ttysw",
	 TOOL_SWEXTENDTOEDGE, TOOL_SWEXTENDTOEDGE);

	/*
	 * Install tool in tree of windows
	 */
	signal(SIGWINCH, sigwinchcatcher);
	signal(SIGCHLD, sigchldcatcher);
	tool_install(tool);

	/*
	 * fork off process in sub window
	 */
	if(argc > MAXARGS)
	  {	printf("ELLE: too many args, ignoring last %d\n", argc-MAXARGS);
		argc = MAXARGS;
	  }
	for(i = 0; i < argc; i++)	/* Copy arg vector */
		nargv[i] = argv[i];
	nargv[i] = sub_arg;	/* With special argument at end */
	nargv[++i] = 0;		/* Now terminate new arg vector */
	ttysw_fork(ttysw->ts_data, nargv, &ttysw->ts_io.tio_inputmask,
				&ttysw->ts_io.tio_outputmask,
				&ttysw->ts_io.tio_exceptmask);
	/*
	 * Handle Activities
	 */
	tool_select(tool, 1);

	/*
	 * Cleanup
	 */
	tool_destroy(tool);
	exit( 0);
}

static
sigchldcatcher()
{
	tool_sigchld(tool);
}

static
sigwinchcatcher()
{
	tool_sigwinch(tool);
}

/* patch routine until SUN offers more facilities */

my_select( nfds, readfds, writefds, exceptfds, timeout)
int nfds, *readfds, *writefds, *exceptfds;
struct timeval *timeout;
{
    *readfds &= ~( 1<<ttysw->ts_windowfd);
    return( select( nfds, readfds, writefds, exceptfds, timeout));
}

/* SUN ELLE inferior main setup.  The body of the editor executes in
 *	the subprocess.
 */
static void
sun_submain()
{
    struct inputmask im;

    /* setup input mask */
    input_imnull( &im);
    im.im_flags |= IM_NEGEVENT;
    im.im_flags |= IM_ASCII;
    im.im_flags |= IM_POSASCII;
    im.im_flags |= IM_META;
    win_setinputcodebit( &im, MS_LEFT);
    win_setinputcodebit( &im, MS_MIDDLE);
    win_setinputcodebit( &im, MS_RIGHT);
    win_setinputmask( sun_winfd, &im, 0, WIN_NULLLINK);
    win_setcursor(sun_winfd, &e_cursor);	/* Set up cursor */
}

void
sun_init()
{
	init_menu();		/* initialize the menu */

	pix_per_row = (win_getheight( sun_winfd) + (scr_ht/2)) / scr_ht;
	pix_per_col = (win_getwidth( sun_winfd) + (scr_wid/2)) / scr_wid;
}



/* SUN editor input routines */

int
sun_input( charsonly)
int charsonly;
{
    struct inputevent event;
    int error;
    int c, zero, wflag, done;

  zero = 0;
  for( done = FALSE; !done;) {
    do {
	wflag = 1 << sun_winfd;
	select( 16, &wflag, &zero, &zero, 0);
    } while( wflag != (1<<sun_winfd));

    error = input_readevent( sun_winfd, &event);
    if( error < 0) {
	if( errno != EWOULDBLOCK) perror( "Bad read event");
	    continue;
    }

    if((event.ie_code >= ASCII_FIRST) && (event.ie_code <= ASCII_LAST)) {
	c = (int) event.ie_code;
	done = TRUE;
    } else if(((event.ie_code&0377) >= META_FIRST)
    && ((event.ie_code&0377) <= META_LAST)) {
	c = (int) (event.ie_code&0377);
	done = TRUE;
    } else if((event.ie_code >= BUT_FIRST) && (event.ie_code <= BUT_LAST)) {
	if( ! charsonly) {
	    sun_handle_mouse( &event, ! win_inputnegevent( &event));
	    c = -1;
	    done = TRUE;
	}
    }
  }
  return(c);
}

static void
sun_handle_mouse( event, down)
struct inputevent *event;
int down; /* TRUE => button was pushed down */
{
    register int mask;
    static mouse_saved, /* bits for all buttons ever down for this "event" */
           mouse_now; /* bits for buttons */

    mask = BUTTON_MASK( event->ie_code);
    if( down) {
	mouse_saved |= mask;
	if(mouse_saved == BUTTON_MASK( MENU_BUT)) {
	    /* menu button is only button down - pass through */
	    sun_interpret_mouse( mouse_saved, event);
	    mouse_saved = 0;
	} else	mouse_now |= mask;
    } else { /* up */
	mouse_now &= ~mask;
	if( mouse_now == 0) { /* all buttons released, interpret the code */
	    sun_interpret_mouse( mouse_saved, event);
	    mouse_saved = 0;
	}
    };
}

/* Do functions invoked by mouse buttons
*/
static void
sun_interpret_mouse(mask, event)
struct inputevent *event;
{
	int newrow, newcol;

    switch(mask) {
	case BUTTON_MASK(MS_LEFT):
		ed_goxy(event->ie_locx/pix_per_col,	/* Go to mouse loc */
			event->ie_locy/pix_per_row);
		break;

	case BUTTON_MASK(MS_RIGHT):
		if( menuptr != 0)
		  { struct menuitem *menuitemp;
		    extern struct menuitem *menudisplay();
		    int (*funct)();

		    if(menuitemp = menu_display(&menuptr,event,sun_winfd))
		      {
			funct = (int (*)()) menuitemp->mi_data;
			(*funct)();
		      }
		  }
		break;

	case BUTTON_MASK(MS_MIDDLE):
		{
		chroff savdot = e_dot();	/* Save current context */
		struct window *w = cur_win;

		saynow("");			/* Clear feedback line */
		ed_goxy(event->ie_locx/pix_per_col,	/* Go to mouse loc */
			event->ie_locy/pix_per_row);
		redisplay();

		f_setmark();				/* set the mark */
		saynow("Mark set.");

		/* return cursor to old context */
		if(w != cur_win)
			f_othwind();		/* change to other window */
		ed_go(savdot);
		}
		break;

	case (BUTTON_MASK(MS_LEFT) | BUTTON_MASK(MS_MIDDLE)):
		exp = 1;
		f_bdchar();	/* Backward Delete Char */
		break;
    }
}

/* ED_GOXY - Go to given screen position.
 *	Set cur_wind, cur_buf, and cur_dot based on given x-y coords.
 *	This is a special routine in that it can change the current
 *	window (and thus the current buffer!) to be something else.
 */
ed_goxy(x, y)
register int x, y;
{
    register struct window *w, *testw;
    unsigned distance, testdistance;
    struct scr_line *line;

    /* find the appropriate window */
	w = cur_win;
	distance = win_xydist( w, x, y);
	for( testw = win_head; testw; testw = testw->w_next)
	    if( testw == oth_win) { /* this test may change */
		if( distance > (testdistance = win_xydist(testw, x, y))) {
		    w = testw;
		    distance = testdistance;
		}
	    }
	if( w != cur_win) f_othwind(); /* change to only other window */

	d_goxy(x, y);		/* Go to buffer pos matching these coords */
	ed_setcur();		/* Make that be cur dot and say it moved */
}

/* D_GOXY - Move to given screen position in current window/buffer.
 *	If position isn't within current win/buff, goes as close as it
 *	can.
 */
d_goxy(x, y)
register int x, y;
{
	register struct window *w = cur_win;
	struct scr_line *line;
	int col;

	if(w->w_pos > y)		/* pointing above window */
		y = w->w_pos;
	else if((w->w_pos + w->w_ht) <= y) /* pointing below window */
		y = w->w_pos + w->w_ht - 1;

	/* Inside window now, but must check to ensure the line isn't
	** an empty line after EOF, and move back up to first line with
	** real stuff on it.
	*/
	while(( y > w->w_pos) && ( scr[y-1]->sl_len == 0))
		--y;

	line = scr[y];
	if ((x > line->sl_col)
	 || (col = inindex(line->sl_boff, x)) == -1)
	  {	col = line->sl_len;		/* Just go to EOL */
		if(line->sl_flg & SL_EOL)
			col -= 1;
	  }
	e_go(line->sl_boff + col);		/* Go there */
}

/* WIN_XYDIST - Give positive euclidean distance from this window */
static int
win_xydist(w, x, y)
register struct window *w;
int x, y;
{
    /* only y coord to consider for now -- returns 0 if inside window */
    if( w->w_pos > y)			/* point is above window */
	return( w->w_pos - y);
    if( (w->w_pos + w->w_ht) <= y)	/* point is below window */
	return( y - (w->w_pos + w->w_ht -1));
    return( 0); /* in window */
}

/* Text Selection functions */

/* EFUN: "Stuff Selection" */
/* stuff (insert) the current window selection into the current
 * buffer at the current position -- use exp as count
 */
f_stuffsel()
{
    register int i;

    i = exp; /* iterate here if exp is > 1 */
    exp = 1; /* set exp to 1 so as not to iterate in f_insself proc */
    for(; i > 0; i--) 
	selection_get( sel_read, sun_winfd);
}


/* EFUN: "Select Region" */
f_selregion()
{
    struct selection sel;

    /* check for good region */
	if( ! chkmark()) return;

    /* fill in sel struct */
	sel.sel_type = SELTYPE_CHAR;
	sel.sel_itembytes = 1;
	sel.sel_items = mark_dot - cur_dot;
	if( sel.sel_items < 0) sel.sel_items = - sel.sel_items;
	sel.sel_pubflags = SEL_PRIMARY;
	sel.sel_privdata = 0;

    saynow("");
    selection_set( &sel, sel_write, sel_clear, sun_winfd);
    saynow( "Region selected.");
}


static void
sel_read( sel, file) /* low level routine to read in the current window
			selection into the current buffer at the current
			position */
struct selection *sel;
FILE *file;
{
    register int nchars;

    switch( sel->sel_type) {
	case SELTYPE_NULL:
	    ding( "No selection");
	    break;

	case SELTYPE_CHAR:
	    for( nchars = 0; nchars < sel->sel_items; nchars++)
		f_insself( getc( file));
	    break;

	default:
	    ding( "Not a text selection");
	    break;

    }
}

static void
sel_write( sel, file) /* write out selection from region */
struct selection *sel;
FILE *file;
{
    chroff dotcnt;

    if((dotcnt = mark_dot - cur_dot) < 0) {
	e_goff(dotcnt);
	dotcnt = -dotcnt;
    } else e_gocur();

    while(--dotcnt >= 0)
	putc(e_getc(), file);

    e_gocur();
}

static void
sel_clear() /* clear selection -- nothing to do */
{
}

#else SUN
f_stuffsel() {}
f_selregion() {}
#endif SUN		/* End of global conditional */
