// GNUPLOT - debug.trm 
// Copyright 1990 - 1993, 1998, 2004   Thomas Williams, Colin Kelley
/*
 * This file is included by ../term.c.
 *
 * This terminal driver supports: DEBUG
 * AUTHORS: luecken@udel.edu
 * send your comments or suggestions to (luecken@udel.edu).
 */
/*
 * adapted to the new terminal layout by Stefan Bodewig (Dec. 1995)
 * generalised to have *all* defined capabilities by HBB (June 1997)
 */
#include <gnuplot.h>
#pragma hdrstop
#include "driver.h"

// @experimental {
#define TERM_BODY
#define TERM_PUBLIC static
#define TERM_TABLE
#define TERM_TABLE_START(x) GpTermEntry x {
#define TERM_TABLE_END(x)   };
// } @experimental

#ifdef TERM_REGISTER
	register_term(debug)
#endif

//#ifdef TERM_PROTO
TERM_PUBLIC void DEBUG_init(GpTermEntry * pThis);
TERM_PUBLIC void DEBUG_graphics(GpTermEntry * pThis);
TERM_PUBLIC void DEBUG_text(GpTermEntry * pThis);
TERM_PUBLIC void DEBUG_linetype(GpTermEntry * pThis, int linetype);
TERM_PUBLIC void DEBUG_move(GpTermEntry * pThis, uint x, uint y);
TERM_PUBLIC void DEBUG_vector(GpTermEntry * pThis, uint x, uint y);
TERM_PUBLIC void DEBUG_put_text(GpTermEntry * pThis, uint x, uint y, const char * str);
TERM_PUBLIC void DEBUG_reset(GpTermEntry * pThis);
TERM_PUBLIC int  DEBUG_justify_text(GpTermEntry * pThis, enum JUSTIFY mode);
TERM_PUBLIC int  DEBUG_text_angle(GpTermEntry * pThis, int ang);
TERM_PUBLIC void DEBUG_point(GpTermEntry * pThis, uint x, uint y, int pointstyle);
TERM_PUBLIC void DEBUG_arrow(GpTermEntry * pThis, uint sx, uint sy, uint ex, uint ey, int head);
TERM_PUBLIC int  DEBUG_set_font(GpTermEntry * pThis, const char * font);
TERM_PUBLIC void DEBUG_pointsize(GpTermEntry * pThis, double pointsize);
TERM_PUBLIC void DEBUG_suspend(GpTermEntry * pThis);
TERM_PUBLIC void DEBUG_resume(GpTermEntry * pThis);
TERM_PUBLIC void DEBUG_fillbox(GpTermEntry * pThis, int style, uint x1, uint y1, uint width, uint height);
TERM_PUBLIC void DEBUG_linewidth(GpTermEntry * pThis, double linewidth);
TERM_PUBLIC void DEBUG_filled_polygon(GpTermEntry * pThis, int, gpiPoint *);
TERM_PUBLIC void DEBUG_set_color(GpTermEntry * pThis, const t_colorspec *);
TERM_PUBLIC void DEBUG_layer(GpTermEntry * pThis, t_termlayer syncpoint);
TERM_PUBLIC void DEBUG_path(GpTermEntry * pThis, int p);
TERM_PUBLIC void DEBUG_image(GpTermEntry * pThis, uint m, uint n, coordval * image, gpiPoint * corner, t_imagecolor color_mode);

#define DEBUG_XMAX 512
#define DEBUG_YMAX 390

#define DEBUG_XLAST (DEBUG_XMAX - 1)
#define DEBUG_YLAST (DEBUG_XMAX - 1)

// Assume a character size of 1, or a 7 x 10 grid. 
#define DEBUG_VCHAR     10
#define DEBUG_HCHAR     7
#define DEBUG_VTIC      (DEBUG_YMAX/70)
#define DEBUG_HTIC      (DEBUG_XMAX/75)
//#endif /* TERM_PROTO */

#ifndef TERM_PROTO_ONLY
#ifdef TERM_BODY

int DEBUG_linetype_last;
int DEBUG_xlast;
int DEBUG_ylast;

TERM_PUBLIC void DEBUG_init(GpTermEntry * pThis)
{
	fputs("init\n", gpoutfile);
	DEBUG_linetype_last = LT_NODRAW;
}

TERM_PUBLIC void DEBUG_graphics(GpTermEntry * pThis)
{
	DEBUG_xlast = DEBUG_ylast = 0;
	fputs("graphics\n", gpoutfile);
}

TERM_PUBLIC void DEBUG_text(GpTermEntry * pThis)
{
	fputs("text\n", gpoutfile);
}

TERM_PUBLIC void DEBUG_linetype(GpTermEntry * pThis, int linetype)
{
	/*
	   if (linetype != DEBUG_linetype_last){
	   fprintf(gpoutfile,"l%d",linetype);
	   DEBUG_linetype_last = linetype;
	   }
	 */
	fprintf(gpoutfile, "line %d\n", linetype);
}

TERM_PUBLIC void DEBUG_move(GpTermEntry * pThis, uint x, uint y)
{
	/*
	   if (x != DEBUG_xlast || y != DEBUG_ylast){
	   fprintf(gpoutfile,"mm");
	   DEBUG_xlast = x;
	   DEBUG_ylast = y;
	   }
	 */
	fprintf(gpoutfile, "move %d, %d\t(%d, %d)\n", x, y, x - DEBUG_xlast, y - DEBUG_ylast);
	DEBUG_xlast = x;
	DEBUG_ylast = y;
}

TERM_PUBLIC void DEBUG_vector(GpTermEntry * pThis, uint x, uint y)
{
	/*
	   if (x != DEBUG_xlast || y != DEBUG_ylast){
	   fprintf(gpoutfile,"vv");
	   DEBUG_xlast = x;
	   DEBUG_ylast = y;
	   }
	 */
	fprintf(gpoutfile, "vect %d, %d\t(%d, %d)\n", x, y, x - DEBUG_xlast, y - DEBUG_ylast);
	DEBUG_xlast = x;
	DEBUG_ylast = y;
}

TERM_PUBLIC void DEBUG_put_text(GpTermEntry * pThis, uint x, uint y, const char * str)
{
	/*
	   DEBUG_move(x,y);
	   fprintf(gpoutfile,"tx%s\r",str);
	 */
	fputs("put_text calls:", gpoutfile);
	DEBUG_move(pThis, x, y);
	fprintf(gpoutfile, "put_text '%s'\n", str);
}

TERM_PUBLIC void DEBUG_reset(GpTermEntry * pThis)
{
	fputs("reset", gpoutfile);
}

TERM_PUBLIC int DEBUG_justify_text(GpTermEntry * pThis, enum JUSTIFY mode)
{
	fputs("justify ", gpoutfile);
	switch(mode) {
		case CENTRE: fputs("centre", gpoutfile); break;
		case RIGHT: fputs("right", gpoutfile); break;
		default: 
		case LEFT: fputs("left", gpoutfile); break;
	}
	fputs("\n", gpoutfile);
	return TRUE;
}

TERM_PUBLIC int DEBUG_text_angle(GpTermEntry * pThis, int ang)
{
	fprintf(gpoutfile, "text_angle %d:", ang);
	switch(ang) {
		case 0: fputs(": horizontal\n", gpoutfile); break;
		case 1: fputs(": upwards\n", gpoutfile); break;
		default: fputs(": \a*undefined*\n", gpoutfile); break;
	}
	return TRUE;
}

TERM_PUBLIC void DEBUG_point(GpTermEntry * pThis, uint x, uint y, int pointstyle)
{
	fprintf(gpoutfile, "point at (%ud,%ud), pointstyle %d\n", x, y, pointstyle);
}

TERM_PUBLIC void DEBUG_arrow(GpTermEntry * pThis, uint sx, uint sy, uint ex, uint ey, int head)
{
	fprintf(gpoutfile, "arrow from (%ud,%ud) to (%ud,%ud), %s head\n", sx, sy, ex, ey, head ? "with" : "without");
}

TERM_PUBLIC int DEBUG_set_font(GpTermEntry * pThis, const char * font)
{
	fprintf(gpoutfile, "set font to \"%s\"\n", font ? (*font ? font : "\aempty string!") : "\aNULL string!");
	return TRUE;
}

TERM_PUBLIC void DEBUG_pointsize(GpTermEntry * pThis, double pointsize)
{
	fprintf(gpoutfile, "set pointsize to %lf\n", pointsize);
}

TERM_PUBLIC void DEBUG_suspend(GpTermEntry * pThis)
{
	fputs("suspended terminal driver\n", gpoutfile);
}

TERM_PUBLIC void DEBUG_resume(GpTermEntry * pThis)
{
	fputs("resumed terminal driver\n", gpoutfile);
}

TERM_PUBLIC void DEBUG_fillbox(GpTermEntry * pThis, int style, uint x1, uint y1, uint width, uint height)
{
	fprintf(gpoutfile, "fillbox/clear at (%ud,%ud), area (%ud,%ud), style %d)\n", x1, y1, width, height, style);
}

TERM_PUBLIC void DEBUG_linewidth(GpTermEntry * pThis, double linewidth)
{
	fprintf(gpoutfile, "set linewidth %lf\n", linewidth);
}

TERM_PUBLIC void DEBUG_filled_polygon(GpTermEntry * pThis, int points, gpiPoint * corners)
{
	fprintf(gpoutfile, "polygon with %d vertices\n", points);
}

TERM_PUBLIC void DEBUG_set_color(GpTermEntry * pThis, const t_colorspec * colorspec)
{
	//extern void save_pm3dcolor();
	fprintf(gpoutfile, "set_color:  ");
	save_pm3dcolor(gpoutfile, colorspec);
	fprintf(gpoutfile, "\n");
}

TERM_PUBLIC void DEBUG_layer(GpTermEntry * pThis, t_termlayer syncpoint)
{
	char * l = "";
	switch(syncpoint) {
		case TERM_LAYER_RESET:              l = "reset"; break;
		case TERM_LAYER_BACKTEXT:           l = "backtext"; break;
		case TERM_LAYER_FRONTTEXT:          l = "fronttext"; break;
		case TERM_LAYER_BEGIN_GRID:         l = "begin grid"; break;
		case TERM_LAYER_END_GRID:           l = "end grid"; break;
		case TERM_LAYER_END_TEXT:           l = "end text"; break;
		case TERM_LAYER_BEFORE_PLOT:        l = "before plot"; break;
		case TERM_LAYER_AFTER_PLOT:         l = "after plot"; break;
		case TERM_LAYER_BEGIN_KEYSAMPLE:    l = "begin keysample"; break;
		case TERM_LAYER_END_KEYSAMPLE:      l = "end keysample"; break;
		case TERM_LAYER_RESET_PLOTNO:       l = "reset plotno"; break;
		case TERM_LAYER_BEFORE_ZOOM:        l = "before zoom"; break;
		case TERM_LAYER_BEGIN_PM3D_MAP:     l = "begin pm3d map"; break;
		case TERM_LAYER_END_PM3D_MAP:       l = "end pm3d map"; break;
		default:                            l = "unknown"; break;
	}
	fprintf(gpoutfile, "layer %s\n", l);
}

TERM_PUBLIC void DEBUG_path(GpTermEntry * pThis, int p)
{
	fprintf(gpoutfile, "path %d\n", p);
}

TERM_PUBLIC void DEBUG_image(GpTermEntry * pThis, uint m, uint n, coordval * image, gpiPoint * corner, t_imagecolor color_mode)
{
	fprintf(gpoutfile, "image size = %d x %d\n", m, n);
}

#endif /* TERM_BODY */

#ifdef TERM_TABLE

TERM_TABLE_START(debug_driver)
	"debug", 
	"debugging driver",
	DEBUG_XMAX, 
	DEBUG_YMAX, 
	DEBUG_VCHAR, 
	DEBUG_HCHAR,
	DEBUG_VTIC, 
	DEBUG_HTIC, 
	GnuPlot::OptionsNull, 
	DEBUG_init, 
	DEBUG_reset,
	DEBUG_text, 
	GnuPlot::NullScale, 
	DEBUG_graphics, 
	DEBUG_move, 
	DEBUG_vector,
	DEBUG_linetype, 
	DEBUG_put_text, 
	DEBUG_text_angle,
	DEBUG_justify_text, 
	DEBUG_point, 
	DEBUG_arrow, 
	DEBUG_set_font,
	DEBUG_pointsize,
	TERM_CAN_MULTIPLOT,
	DEBUG_suspend, 
	DEBUG_resume, 
	DEBUG_fillbox, 
	DEBUG_linewidth,
	#ifdef USE_MOUSE
		0, 
		0, 
		0, 
		0, 
		0,     /* no mouse support */
	#endif
	0, 
	0,     /* no palette */
	DEBUG_set_color,
	DEBUG_filled_polygon,
	DEBUG_image,
	0, 
	0, 
	0,     /* no enhanced text */
	DEBUG_layer, 
	DEBUG_path 
TERM_TABLE_END(debug_driver)

#undef LAST_TERM
#define LAST_TERM debug_driver

#endif /* TERM_TABLE */
#endif /* TERM_PROTO_ONLY */

#ifdef TERM_HELP
START_HELP(debug)
"1 debug",
"?commands set terminal debug",
"?set terminal debug",
"?set term debug",
"?terminal debug",
"?term debug",
"?debug",
" This terminal is provided to allow for the debugging of `gnuplot`.  It is",
" likely to be of use only for users who are modifying the source code."
END_HELP(debug)
#endif
