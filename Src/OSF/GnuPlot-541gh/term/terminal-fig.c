// GNUPLOT - fig.trm 
// Copyright 1990 - 1993, 1998, 2004
/*
 * This file is included by ../term.c.
 * This terminal driver supports: Fig graphics language
 * AUTHORS: Micah Beck, David Kotz
 * send your comments or suggestions to (gnuplot-info@lists.sourceforge.net).
 *
 */
/*
 * Original for Fig code output by Micah Beck, 1989
 * Department of Computer Science, Cornell University
 * Updated by David Kotz for gnuplot 2.0
 * More efficient output by Ian Dall
 * Updated to FIG 2.1 (with color) format by Vivek Khera
 * Updated to FIG 3.1 (higher resolution) format by Ian MacPhedran, Jan 1995
 * Updated to conform to newterm format Ian MacPhedran, Apr 1995
 * Point-count option joachim.selinger@ins.uni-stuttgart.de (JFS) Feb  9 1996
 * More options (portrait/landscape, metric/inches, size, fontsize, thickness)
 * plus symbols and depth/thickness by bernlohr@eu1.mpi-hd.mpg.de (KB) Aug 15 1996
 * Added PM3D functionality Ian MacPhedran, April 15 1999
 * Take into account 'set palette maxcolors' Petr Mikulik, June 11 2002
 * Don't reset options when 'set term fig <new options>', Petr Mikulik, Aug 24 2002
 * Ethan A Merritt - May 2008:
 *   Bring into line with other terminals for point types, size syntax, font spec
 * Ethan A Merritt - Mar 2015:
 *   Load user-defined RGB linetype colors and gnuplot's set of named RGB colors
 *
 * Ethan A Merritt - April 2019: Update to conform with gnuplot version 5
 *
 *   Revise handling of fig "depth" to base it on gnuplot layers front/back/etc
 *   __________________________
 *
 *	'behind items'	900
 *	grid		801
 *	'back items'	800-701
 *	plots by number	700-204
 *	pm3d depthsort  203
 *	key box		202
 *	key titles	201
 *	'front items'	200-101
 *   __________________________
 *
 *   Revise dashtype handling to support gnuplot 5 dashtype property
 *   Remove obsolete terminal options (metric inches pointsmax depth dashed solid)
 *   Option selection and syntax now matches other terminals (except font names + flags)
 *
 *   TODO
 *	- ditch custom point types > 50?
 *	- LT_NODRAW
 *	- terminal option "dashlength"
 *
 *   NOTES
 *	-  Xfig 3.2.5 and earlier segfault on dashtype > 2
 *	   Xfig 3.2.7 is OK.  I don't know about 3.2.6
 *	- pointinterval and other styles that assume things are drawn in the
 *	  order given do not work because fig depth runs backwards to that assumption
 *	- utf8 is not possible because Xfig is limited both by X11 and by PostScript
 *      - fractional linewidths are not possible
 *	- RGB colors can only be accessed if they are present in a defined linetype
 *	  or match a named color or palette entry at the time of plotting.
 *	- nothing prevents out-of-range layers if there is a huge number of plots
 */
#include <gnuplot.h>
#pragma hdrstop
#include "driver.h"

// @experimental {
#define TERM_BODY
#define TERM_PUBLIC static
#define TERM_TABLE
#define TERM_TABLE_START(x) termentry x {
#define TERM_TABLE_END(x)   };
// } @experimental

#ifdef TERM_REGISTER
	register_term(fig)
#endif /* TERM_REGISTER */

//#ifdef TERM_PROTO
TERM_PUBLIC void FIG_options(TERMENTRY * pThis, GnuPlot * pGp);
TERM_PUBLIC void FIG_init(termentry * pThis);
TERM_PUBLIC void FIG_graphics();
TERM_PUBLIC void FIG_text();
TERM_PUBLIC void FIG_linetype(int linetype);
TERM_PUBLIC void FIG_dashtype(int type, t_dashtype * custom_dash_pattern);
TERM_PUBLIC void FIG_move(uint x, uint y);
TERM_PUBLIC void FIG_vector(uint ux, uint uy);
TERM_PUBLIC void FIG_arrow(uint sx, uint sy, uint ex, uint ey, int head);
TERM_PUBLIC void FIG_put_text(uint x, uint y, const char * str);
TERM_PUBLIC int FIG_justify_text(enum JUSTIFY mode);
TERM_PUBLIC int FIG_text_angle(int ang);
TERM_PUBLIC void FIG_pointsize(double arg_pointsize);
TERM_PUBLIC void FIG_linewidth(double linewidth);
TERM_PUBLIC void FIG_reset();
TERM_PUBLIC void FIG_lpoint(uint x, uint y, int number);
TERM_PUBLIC void FIG_boxfill(int style, uint x, uint y, uint w, uint h);
TERM_PUBLIC int FIG_make_palette(t_sm_palette *);
TERM_PUBLIC void FIG_set_color(const t_colorspec *);
TERM_PUBLIC void FIG_filled_polygon(int, gpiPoint *);
TERM_PUBLIC void FIG_layer(t_termlayer syncpoint);
//#endif // TERM_PROTO 

#ifndef TERM_PROTO_ONLY
#ifdef TERM_BODY

#include "fig_defs.h"           /* modified version of object.h from XFig distribution */
#define FIG_BACKGROUND 7        /* FIXME:  this is not guaranteed */
#define FIG_DEFAULT DEFAULT
#define FIG_ROMAN_FONT (0)      /* actually, the default font */
static int FIG_gray_index = 68; /* just a guess */
static int FIG_plotno = 0;

/* These should not be defined elsewhere - ACZ */
/* This is now 1200 per inch */
#define FIG_IRES        (1200)

#define FIG_COORD_SYS   2
#define FIG_ORIENT (FIG_portrait ? "Portrait" : "Landscape")
#define FIG_JUST        "Center" /* Could be "Flush Left" */
#define FIG_PAPER       (FIG_size_units == INCHES ? "Letter" : "A4")
#define FIG_MAGNIFICATION  100.0
#define FIG_MULTIPAGE   "Single"
/* Could be "Multiple" */
#define FIG_TRANSCOLOR  -2 /* none: -2; background: -1; 0..31: foreground colors */

#define FIG_XMAX 6000
#define FIG_YMAX 3600
#define FIG_HTIC  ((5*FIG_IRES)/80)
#define FIG_VTIC  ((5*FIG_IRES)/80)
#define FIG_VCHAR (FIG_FONT_S*FIG_IRES*0.75/72)
#define FIG_HCHAR (FIG_FONT_S*FIG_IRES*0.75*0.60/72)
#define FIG_FONT_S      (10)    /* size in points */
#define FIG_MAX_POINTS  99999L  /* almost infinite ;-) */

/* Text flags (ULIG) */
enum FIG_TEXT_STYLEBITS {
	FIG_TEXT_RIGID      = 1<<0,
	FIG_TEXT_SPECIAL    = 1<<1,
	FIG_TEXT_POSTSCRIPT = 1<<2,
	FIG_TEXT_HIDDEN     = 1<<3
};

#define FIG_TEXT_NORMAL      FIG_TEXT_POSTSCRIPT

static int FIG_current_layer;   /* behind|back|plot|front */

enum FIG_poly_stat {
	FIG_poly_new, FIG_poly_part
};

static int FIG_posx;
static int FIG_posy;
static long FIG_poly_vec_cnt;
static int FIG_linedepth = 500;
static int FIG_thickness = 1;
static double FIG_linewidth_factor = 1.0;
static double FIG_current_pointsize = 1.0;
static double FIG_current_linewidth = 1.0;
static GpSizeUnits FIG_size_units = INCHES;

/* Maximum number of points per POLYLINE. */
static int FIG_poly_vec_max = 999;      /* JFS */

static enum FIG_poly_stat FIG_polyvec_stat;

/* up to 128 slots total are used for user-defined RGB colors and
 * gnuplot's named colors. That leaves 128 slots for a palette.
 * Fig 3.2 says 512 user colors are possible, so we could expand.
 */
static uint FIG_RGB_colors[256];

/* Default size in pixels at IRES pixels/inch */
static int FIG_width  = 6000;
static int FIG_height = 3600;

static F_point * FIG_points = NULL;      /* Array for the collection of points for
                                            POLYLINE, allocated on demand. */
static F_line FIG_line;

static int FIG_type;            /* actually solid or dash pattern */
static float FIG_spacing;       /* length of dash or dot spacing */
static int FIG_justify;         /* Fig justification T_*_JUSTIFIED */
static float FIG_angle;         /* Fig text angle (in radians) */
static int FIG_use_color = TRUE;        /* do we use color or not? */
static int FIG_color = DEFAULT; /* which color to use */
static int FIG_xoff = FIG_IRES;
static int FIG_yoff = FIG_IRES;
static int FIG_font_id = FIG_ROMAN_FONT;
static int FIG_font_s = FIG_FONT_S;
static int FIG_portrait = FALSE;
static int FIG_text_flags = FIG_TEXT_NORMAL;    /* whether text is special or hidden etc. */
static int FIG_palette_set = FALSE;     /* PM3D Palette Set ? */
static int FIG_palette_size = 128;      /* Number of colours in palette */
static int FIG_palette_offst = 32;      /* Offset from zero for palette colours */
static int FIG_rgb_color_offset = 32;   /* First user-defined RGB color */
static int FIG_fill_style = 20;         /* Full saturation */

static void FIG_poly_clean(enum FIG_poly_stat fig_stat);

enum FIG_id {
	FIG_MONOCHROME, FIG_COLOR,
	FIG_SMALL, FIG_BIG,
	FIG_PORTRAIT, FIG_LANDSCAPE,
	FIG_SIZE, FIG_FONT, FIG_FONTSIZE,
	FIG_THICKNESS, FIG_DEPTH,
	FIG_SOLID, FIG_DASHED,
	FIG_NORMALTEXT, FIG_SPECIALTEXT, FIG_HIDDENTEXT, FIG_RIGIDTEXT,
	FIG_VERSION, FIG_OTHER
};

static struct gen_table FIG_opts[] =
{
	{ "big", FIG_BIG },
	{ "c$olor", FIG_COLOR },
	{ "c$olour", FIG_COLOR },
	{ "dash$ed", FIG_DASHED },
	{ "depth", FIG_DEPTH },
	{ "font", FIG_FONT },
	{ "fontsize", FIG_FONTSIZE },
	{ "fs", FIG_FONTSIZE },
	{ "land$scape", FIG_LANDSCAPE },
	{ "mono$chrome", FIG_MONOCHROME },
	{ "portrait", FIG_PORTRAIT },
	{ "size", FIG_SIZE },
	{ "small", FIG_SMALL },
	{ "solid", FIG_SOLID },
	{ "linew$idth", FIG_THICKNESS },
	{ "lw", FIG_THICKNESS },
	{ "texth$idden", FIG_HIDDENTEXT },
	{ "textn$ormal", FIG_NORMALTEXT },
	{ "textr$igid", FIG_RIGIDTEXT },
	{ "texts$pecial", FIG_SPECIALTEXT },
	{ NULL, FIG_OTHER }
};

const struct gen_table FIG_fonts[] =
{
	{ "Times Roman",  0 },
	{ "Times Italic",  1 },
	{ "Times Bold",  2 },
	{ "Times Bold Italic",  3 },
	{ "AvantGarde Book",  4 },
	{ "AvantGarde Book Oblique",  5 },
	{ "AvantGarde Demi",  6 },
	{ "AvantGarde Demi Oblique",  7 },
	{ "Bookman Light",  8 },
	{ "Bookman Light Italic",  9 },
	{ "Bookman Demi", 10 },
	{ "Bookman Demi Italic", 11 },
	{ "Courier", 12 },
	{ "Courier Oblique", 13 },
	{ "Courier Bold", 14 },
	{ "Courier Bold Oblique", 15 },
	{ "Helvetica", 16 },
	{ "Helvetica Oblique", 17 },
	{ "Helvetica Bold", 18 },
	{ "Helvetica Bold Oblique", 19 },
	{ "Helvetica Narrow", 20 },
	{ "Helvetica Narrow Oblique", 21 },
	{ "Helvetica Narrow Bold", 22 },
	{ "Helvetica Narrow Bold Oblique", 23 },
	{ "New Century Schoolbook Roman", 24 },
	{ "New Century Schoolbook Italic", 25 },
	{ "New Century Schoolbook Bold", 26 },
	{ "New Century Schoolbook Bold Italic", 27 },
	{ "Palatino Roman", 28 },
	{ "Palatino Italic", 29 },
	{ "Palatino Bold", 30 },
	{ "Palatino Bold Italic", 31 },
	{ "Symbol", 32 },
	{ "Zapf Chancery Medium Italic", 33 },
	{ "Zapf Dingbats", 34 },
	{ NULL, -1}
};

TERM_PUBLIC void FIG_options(TERMENTRY * pThis, GnuPlot * pGp)
{
	int parse_error = FALSE;
	float xsize_t = 0, ysize_t = 0;
	char tmp_term_options[MAX_LINE_LEN+1] = "";
	char text_flags[256]; /* for description only */
	while(!pGp->Pgm.EndOfCommand()) {
		switch(pGp->Pgm.LookupTableForCurrentToken(&FIG_opts[0])) {
			case FIG_MONOCHROME:
			    FIG_use_color = FALSE;
			    pGp->Pgm.Shift();
			    break;
			case FIG_SOLID:
			case FIG_DASHED:
			    // ignored 
			    pGp->Pgm.Shift();
			    break;
			case FIG_COLOR:
			    FIG_use_color = TRUE;
			    pGp->Pgm.Shift();
			    break;
			case FIG_SMALL:
			    FIG_width = 6000;
			    FIG_height = 3600;
			    pGp->Pgm.Shift();
			    break;
			case FIG_BIG:
			    FIG_width = 9600;
			    FIG_height = 6000;
			    pGp->Pgm.Shift();
			    break;
			case FIG_NORMALTEXT:
			    FIG_text_flags = FIG_TEXT_NORMAL;
			    pGp->Pgm.Shift();
			    break;
			case FIG_SPECIALTEXT:
			    FIG_text_flags |= FIG_TEXT_SPECIAL;
			    FIG_text_flags &= ~FIG_TEXT_POSTSCRIPT;
			    FIG_font_id = 0;
			    pGp->Pgm.Shift();
			    break;
			case FIG_HIDDENTEXT:
			    FIG_text_flags |= FIG_TEXT_HIDDEN;
			    pGp->Pgm.Shift();
			    break;
			case FIG_RIGIDTEXT:
			    FIG_text_flags |= FIG_TEXT_RIGID;
			    pGp->Pgm.Shift();
			    break;
			case FIG_PORTRAIT:
			    FIG_portrait = TRUE;
			    pGp->Pgm.Shift();
			    break;
			case FIG_LANDSCAPE:
			    FIG_portrait = FALSE;
			    pGp->Pgm.Shift();
			    break;
			case FIG_SIZE:
			    pGp->Pgm.Shift();
			    FIG_size_units = pGp->ParseTermSize(&xsize_t, &ysize_t, INCHES);
			    FIG_width  = static_cast<int>(xsize_t * FIG_IRES/GpResolution);
			    FIG_height = static_cast<int>(ysize_t * FIG_IRES/GpResolution);
			    break;
			case FIG_FONT:
		    {
			    char * fontname;
			    int sep;
			    pGp->Pgm.Shift();
			    if(pGp->Pgm.EndOfCommand() || !((fontname = pGp->TryToGetString())))
				    pGp->IntErrorCurToken("expecting font name");
			    sep = strcspn(fontname, ",");
			    sscanf(&(fontname[sep+1]), "%d", &FIG_font_s);
			    fontname[sep] = '\0';
			    FIG_font_id = lookup_table_entry(FIG_fonts, fontname);
			    if(FIG_font_id < 0)
				    FIG_font_id = FIG_ROMAN_FONT;
			    SAlloc::F(fontname);
			    break;
		    }
			case FIG_FONTSIZE:
			    pGp->Pgm.Shift();
			    FIG_font_s = pGp->IntExpression();
			    break;
			case FIG_THICKNESS:
			    pGp->Pgm.Shift();
			    if(pGp->Pgm.EndOfCommand()) {
				    pGp->IntErrorCurToken("linewidth: number expected");
			    }
			    else {
				    FIG_linewidth_factor = pGp->RealExpression();
				    if(!inrange(FIG_linewidth_factor, 0.1, 10.0)) {
					    pGp->IntWarn(pGp->Pgm.GetPrevTokenIdx(), "linewidth out of range");
					    FIG_linewidth_factor = 1.0;
				    }
			    }
			    break;
			case FIG_DEPTH:
			    pGp->Pgm.Shift();
			    // V5.3 - depth is now based on TERM_LAYER 
			    pGp->IntExpression();
			    break;
			case FIG_OTHER:
			default:
			    parse_error = TRUE;
			    pGp->IntWarn(pGp->Pgm.GetCurTokenIdx(), "unrecognized option");
				pGp->Pgm.Shift();
			    break;
		}
	}
	if(FIG_text_flags == FIG_TEXT_NORMAL) {
		strcpy(text_flags, " textnormal");
	}
	else {
		sprintf(text_flags, "%s%s%s", (FIG_text_flags & FIG_TEXT_SPECIAL) ? " textspecial" : "",
		    (FIG_text_flags & FIG_TEXT_HIDDEN) ? " texthidden" : "", (FIG_text_flags & FIG_TEXT_RIGID) ? " textrigid" : "");
	}
	sprintf(term_options, "%s %s %s %s \"%s,%d\" linewidth %.1f", FIG_use_color ? "color" : "monochrome", FIG_portrait ? "portrait" : "landscape",
	    text_flags, "font", FIG_fonts[FIG_font_id].key, FIG_font_s, FIG_linewidth_factor);
	/* Normalize and print size */
	if(FIG_portrait && FIG_width > FIG_height) {
		float tmp = static_cast<float>(FIG_width);
		FIG_width = FIG_height;
		FIG_height = static_cast<int>(tmp);
	}
	if(FIG_size_units == CM)
		sprintf(tmp_term_options, " size %.2fcm, %.2fcm ", 2.54*FIG_width/FIG_IRES, 2.54*FIG_height/FIG_IRES);
#if 0
	/* Not supported; convert to inches instead */
	else if(FIG_size_units == PIXELS)
		sprintf(tmp_term_options, " size %d, %d ", FIG_width, FIG_height);
#endif
	else
		sprintf(tmp_term_options, " size %.2fin, %.2fin ", (double)FIG_width/FIG_IRES, (double)FIG_height/FIG_IRES);
	strncat(term_options, tmp_term_options, /*sizeof(term_options)*/(MAX_LINE_LEN+1)-strlen(term_options)-1);
	pThis->MaxX = FIG_width;
	pThis->MaxY = FIG_height;
	pThis->TicV = FIG_VTIC;
	pThis->TicH = FIG_HTIC;
	// Empirical guess at font metrics 
	pThis->ChrV = static_cast<uint>(FIG_font_s * FIG_IRES/72. * 0.75);
	pThis->ChrH = static_cast<uint>(pThis->ChrV * 0.6);
	FIG_thickness = static_cast<int>(FIG_linewidth_factor);
	if(parse_error)
		pGp->IntErrorCurToken("unrecognized option");
}

static void FIG_poly_clean(enum FIG_poly_stat fig_stat)
{
	int i, j;
	int cap_style;
	if(fig_stat == FIG_poly_part) {
		cap_style = (FIG_line.style==DOTTED_LINE) ? CAP_ROUND : FIG_line.cap_style;
		fprintf(gpoutfile, "%d %d %d %d %d %d %d %d %d %9.3f %d %d %d %d %d %ld\n\t",
		    O_POLYLINE, FIG_line.type, FIG_line.style, FIG_line.thickness,
		    FIG_line.pen_color, FIG_line.fill_color, FIG_line.depth,
		    FIG_line.pen_style, FIG_line.fill_style, FIG_line.style_val,
		    FIG_line.join_style, cap_style, FIG_line.radius,
		    0, 0, FIG_poly_vec_cnt);

		j = 0;
		for(i = 0; i < FIG_poly_vec_cnt; i++) {
			fprintf(gpoutfile, " %d %d", FIG_points[i].x, FIG_points[i].y);
			if(j++ > 4 && i != FIG_poly_vec_cnt - 1) {
				fputs("\n\t", gpoutfile);
				j = 0; /* JFS */
			}
		}
		if(j != 0) {
			putc('\n', gpoutfile);
		}
		/* Give the memory back to the system because we are done with this
		 * polyline. Make sure FIG_points contains NULL afterwards!
		 */
		ZFREE(FIG_points);
	}
	FIG_polyvec_stat = FIG_poly_new;
}

TERM_PUBLIC void FIG_init(termentry * pThis)
{
	FIG_posx = FIG_posy = 0;
	FIG_polyvec_stat = FIG_poly_new;
	FIG_linetype(-1);
	FIG_justify_text(LEFT);
	FIG_text_angle(0);
	FIG_palette_set = FALSE; /* PM3D Palette Set ? */
	FIG_line.tagged = FIG_DEFAULT;
	FIG_line.distrib = FIG_DEFAULT;
	FIG_line.type = T_POLYLINE;
	FIG_line.style = 0;
	FIG_line.thickness = FIG_thickness;
	FIG_line.fill_style = -1;
	FIG_line.depth = FIG_linedepth;
	FIG_line.pen_style = 0;
	FIG_line.for_arrow = NULL;
	FIG_line.back_arrow = NULL;
	FIG_line.cap_style = CAP_BUTT;
	FIG_line.join_style = JOIN_MITER;
	FIG_line.style_val = 0.0;
	FIG_line.radius = 0;
	FIG_line.pic = NULL;
	FIG_line.next = NULL;
	/* Fig 3.2 is the only version of fig output supported */
	fprintf(gpoutfile, "#FIG 3.2\n%s\n%s\n%s\n%s\n%6.2f\n%s\n%d\n",
	    FIG_ORIENT, FIG_JUST, FIG_size_units == INCHES ? "Inches" : "Metric", FIG_PAPER,
	    FIG_MAGNIFICATION, FIG_MULTIPAGE, FIG_TRANSCOLOR);
	fprintf(gpoutfile, "# Produced by gnuplot version %s\n", gnuplot_version);
	fprintf(gpoutfile, "%d %d\n", FIG_IRES, FIG_COORD_SYS);
}

TERM_PUBLIC void FIG_graphics()
{
	int i, ncolors;
	struct linestyle_def * p_this;
	extern struct gen_table default_color_names_tbl[];
	FIG_posx = FIG_posy = 0;
	FIG_polyvec_stat = FIG_poly_new;
	if(!FIG_use_color)
		return;
	/* Dump current set of user-defined RGB linetype colors */
	ncolors = 0;
	memset(FIG_RGB_colors, 0, sizeof(FIG_RGB_colors));
	for(p_this = GPO.Gg.P_FirstPermLineStyle; p_this != NULL; p_this = p_this->next) {
		if(p_this->lp_properties.pm3d_color.type == TC_RGB) {
			/* Load color into FIG_RGB_colors */
			FIG_RGB_colors[ncolors] = p_this->lp_properties.pm3d_color.lt;
			FIG_RGB_colors[ncolors] &= 0xffffff; /* No alpha channel */
			/* Write it to the output file */
			fprintf(gpoutfile, "%d %d #%2.2x%2.2x%2.2x\n",
			    O_COLOR_DEF, FIG_rgb_color_offset + ncolors,
			    (FIG_RGB_colors[ncolors] >> 16) & 0xff,
			    (FIG_RGB_colors[ncolors] >> 8)  & 0xff,
			    (FIG_RGB_colors[ncolors]) & 0xff);
			ncolors++;
		}
		if(ncolors >= 128)
			break;
	}
	/* Now dump gnuplot's set of named RGB colors */
	for(i = 0; i<96; i++) {
		int colorval = default_color_names_tbl[i].value;
		if(FIG_rgb_color_offset + ncolors >= 128)
			break;
		fprintf(gpoutfile, "%d %d #%2.2x%2.2x%2.2x\n",
		    O_COLOR_DEF, FIG_rgb_color_offset + ncolors,
		    (colorval >> 16) & 0xff,
		    (colorval >> 8)  & 0xff,
		    (colorval) & 0xff);
		FIG_RGB_colors[ncolors] = colorval;
		if(colorval == 0x7f7f7f)
			FIG_gray_index = ncolors + FIG_rgb_color_offset;
		ncolors++;
	}
	/* This leaves space in FIG_RGB_colors[] for a 128-entry color palette */
	FIG_palette_offst = 32 + ncolors;
	/* Need p_this here in case of multiplot, which will not reset plotno */
	FIG_plotno = 0;
}

TERM_PUBLIC void FIG_text()
{
	/* there is no way to have separate pictures in a FIG file
	 * FIXME: The existence of a header keyword "multiple-page" suggests otherwise!
	 */
	FIG_poly_clean(FIG_polyvec_stat);
	FIG_posx = FIG_posy = 0;
	fflush(gpoutfile);
}

/* mapping of fig color codes to color sequence as in the postscript terminal */
#define npscolors 9
static int fig2pscolors[npscolors] = {
	4 /*red*/, 2 /*green*/, 1 /*blue*/,
	5 /*magenta*/, 3 /*cyan*/, 6 /*yellow*/,
	0 /*black*/, 26 /*brown*/,
	11 /*use LtBlue instead of light gray*/
};

TERM_PUBLIC void FIG_linetype(int linetype)
{
	int last_FIG_type = FIG_type;
	int last_FIG_spacing = static_cast<int>(FIG_spacing);
	int last_FIG_color = FIG_color;
	int last_FIG_depth = FIG_linedepth;
	int last_FIG_thickness = FIG_thickness;
	FIG_thickness = static_cast<int>(FIG_current_linewidth * FIG_linewidth_factor);
	FIG_color = DEFAULT;
	if(linetype <= LT_NODRAW)
		linetype = LT_BACKGROUND;
	switch(linetype) {
		case 0:
		case LT_BLACK:
		    FIG_type = SOLID_LINE;
		    FIG_spacing = 0.0;
		    if(FIG_use_color)
			    if(linetype==0) FIG_color = fig2pscolors[0]; /* red */
		    break;
		case LT_BACKGROUND:
		    FIG_type = SOLID_LINE;
		    FIG_color = FIG_BACKGROUND;
		    break;
		case LT_AXIS:
		    FIG_type = DOTTED_LINE;
		    FIG_spacing = 4.0; /* gap */
		    if(FIG_use_color)
			    FIG_color = BLACK;
		    break;
		default:
		    // FIXME:  not sure when/if this is needed by the revised driver 
		    if(FIG_use_color) {
			    FIG_type = (linetype >= npscolors); /* dashed line */
			    FIG_color = fig2pscolors[linetype % npscolors];
			    FIG_spacing = static_cast<float>((linetype / npscolors) * 3);
		    }
		    else {      /* monochrome */
			    FIG_dashtype(linetype, NULL);
		    }
		    break;
	} /* End switch */
	if(FIG_type != last_FIG_type || FIG_spacing != last_FIG_spacing || FIG_color != last_FIG_color || FIG_linedepth != last_FIG_depth ||
	    FIG_thickness != last_FIG_thickness)
		FIG_poly_clean(FIG_polyvec_stat);
}

TERM_PUBLIC void FIG_dashtype(int type, t_dashtype * /*custom_dash_pattern*/)
{
	double empirical_scale = 3.0;
	if(type <= 0)
		type = SOLID_LINE;
	FIG_type = type % 6;
	// FIXME: should pass through "dl" somehow 
	FIG_spacing = static_cast<float>(empirical_scale * FIG_current_linewidth);
	FIG_line.cap_style = (FIG_type == SOLID_LINE) ? CAP_BUTT : CAP_ROUND;
}

TERM_PUBLIC void FIG_move(uint x, uint y)
{
	int last_FIG_posx = FIG_posx;
	int last_FIG_posy = FIG_posy;
	FIG_posx = x;
	FIG_posy = y;
	if(FIG_posx != last_FIG_posx || FIG_posy != last_FIG_posy)
		FIG_poly_clean(FIG_polyvec_stat);
}

TERM_PUBLIC void FIG_vector(uint ux, uint uy)
{
	int x = ux, y = uy;
	if(FIG_polyvec_stat != FIG_poly_part) {
		FIG_line.pen_color = FIG_color;
		FIG_line.fill_color = FIG_color;
		FIG_line.style = FIG_type;
		FIG_line.style_val = FIG_spacing;
		FIG_line.depth = FIG_linedepth;
		FIG_line.thickness = FIG_thickness;
		FIG_poly_vec_cnt = 0;
		/* allocate memory for the first point */
		FIG_points = (F_point*)gp_realloc(FIG_points, sizeof(F_point), "FIG_points");   /* JFS */
		FIG_points[FIG_poly_vec_cnt].x = FIG_xoff + FIG_posx;
		FIG_points[FIG_poly_vec_cnt].y = term->MaxY + FIG_yoff - FIG_posy;

		FIG_poly_vec_cnt = 1;
		FIG_polyvec_stat = FIG_poly_part;
	}
	/* allocate memory for the next point */
	FIG_points = (F_point*)gp_realloc(FIG_points, (FIG_poly_vec_cnt + 1) * sizeof(F_point), "FIG_points"); /* JFS */
	FIG_points[FIG_poly_vec_cnt].x = FIG_xoff + x;
	FIG_points[FIG_poly_vec_cnt].y = term->MaxY + FIG_yoff - y;
	FIG_poly_vec_cnt++;
	if(FIG_poly_vec_cnt > FIG_poly_vec_max)
		FIG_poly_clean(FIG_polyvec_stat);
	FIG_posx = x;
	FIG_posy = y;
}

TERM_PUBLIC void FIG_arrow(uint sx, uint sy/* start coord */, uint ex, uint ey/* end coord */, int head)
{
	int cap_style;
	int arrow_depth = FIG_linedepth;
	double awidth, aheight; /* arrow head sizes */
	/* Adjust depth so that arrows are drawn after objects and labels */
	if(FIG_current_layer != LAYER_PLOT)
		arrow_depth -= 2;
	FIG_poly_clean(FIG_polyvec_stat);
	cap_style = (FIG_line.style==DOTTED_LINE) ? CAP_ROUND : FIG_line.cap_style;
	fprintf(gpoutfile, "%d %d %d %d %d %d %d %d %d %9.3f %d %d %d %d %d %d\n",
	    O_POLYLINE, FIG_line.type, FIG_type,
	    (head & HEADS_ONLY) ? 0 : FIG_thickness,
	    FIG_color, FIG_color, arrow_depth,
	    FIG_line.pen_style, FIG_line.fill_style, FIG_spacing,
	    FIG_line.join_style, cap_style, FIG_line.radius,
	    (head & END_HEAD) ? 1 : 0, (head & BACKHEAD) ? 1 : 0, 2);

	/* arrow head(s) */
	if(head) {
		uint headbackangleparameter = 0;
		uint headfillparameter = 0;
		/* arrow head size */
		if(curr_arrow_headlength == 0) {
			awidth  = (double)(term->TicH / 2 + 1);
			aheight = (double)term->TicH;
		}
		else {
			awidth  = (double)curr_arrow_headlength * 2*sin(curr_arrow_headangle*M_PI/180);
			aheight = (double)curr_arrow_headlength * cos(curr_arrow_headangle*M_PI/180);
		}
		// arrow head geometry 
		if(curr_arrow_headbackangle < 70)
			headbackangleparameter = 2;
		else if(curr_arrow_headbackangle > 110)
			headbackangleparameter = 3;
		else
			headbackangleparameter = 1;
		headfillparameter = (curr_arrow_headfilled==2) ? 1 : 0;
		// forward head 
		if((head & END_HEAD))
			fprintf(gpoutfile, "%d %d %.3f %.3f %.3f\n", headbackangleparameter, headfillparameter, 1.0, awidth, aheight);
		/* backward head */
		if((head & BACKHEAD))
			fprintf(gpoutfile, "%d %d %.3f %.3f %.3f\n", headbackangleparameter,  headfillparameter, 1.0, awidth, aheight);
	}
	/* arrow line */
	fprintf(gpoutfile, "%d %d %d %d\n", FIG_xoff + sx, FIG_yoff + term->MaxY - sy, FIG_yoff + ex, FIG_yoff + term->MaxY - ey);
	FIG_posx = ex;
	FIG_posy = ey;
}

TERM_PUBLIC void FIG_put_text(uint x, uint y, const char * str)
{
	char * s1, * s2, * output_string;
	int text_depth = FIG_linedepth;
	if(strlen(str) == 0)
		return;
	output_string = (char*)gp_alloc(2*strlen(str)+1, "FIG text");
	s1 = (char*)str;
	s2 = output_string;
	do {
		if(*s1 == '\\') *(s2++) = *s1;
		*(s2++) = *s1;
	} while(*(s1++) );
	FIG_poly_clean(FIG_polyvec_stat);
	if(FIG_angle == 0.)
		y -= term->ChrV / 2; /* assuming vertical center justified */
	else {
		x += (int)(term->ChrV*sin(FIG_angle)/4.0);
		y -= (int)(term->ChrV*cos(FIG_angle)/4.0);
	}
	/* Adjust depth so that labels are drawn after objects */
	if(FIG_current_layer != LAYER_PLOT)
		text_depth -= 1;
	fprintf(gpoutfile, "%d %d %d %d %d %d %6.3f %6.3f %d %6.3f %6.3f %d %d %s\\001\n",
	    OBJ_TEXT, FIG_justify, FIG_color, text_depth, FIG_DEFAULT,
	    FIG_font_id, (float)FIG_font_s,
	    FIG_angle, FIG_text_flags, (float)term->ChrV,
	    (float)term->ChrH * strlen(str),
	    FIG_xoff + x, term->MaxY + FIG_yoff - y, output_string);
	SAlloc::F(output_string);
}

TERM_PUBLIC int FIG_justify_text(enum JUSTIFY mode)
{
	switch(mode) {
		case LEFT: FIG_justify = T_LEFT_JUSTIFIED; break;
		case CENTRE: FIG_justify = T_CENTER_JUSTIFIED; break;
		case RIGHT: FIG_justify = T_RIGHT_JUSTIFIED; break;
		/* shouldn't happen */
		default:
		    FIG_justify = T_LEFT_JUSTIFIED;
		    return (FALSE);
		    break;
	}
	return (TRUE);
}

TERM_PUBLIC int FIG_text_angle(int ang)
{
	FIG_angle = static_cast<float>(ang * M_PI_2 / 90.0);
	return (TRUE);
}

TERM_PUBLIC void FIG_lpoint(uint x, uint y, int number)
{
	/* FIXME: Some complicated gnuplot 4 convention for packing all sorts
	 *        of stuff into the pointtype
	 */
	if(number % 100 >= 49 && number % 100 < 99) {   /* circles, squares, triangles */
		int r, d, h, xpc, ypc;
		int line_color, fill_color, fill_style;
		int cnum, tnum;
		FIG_poly_clean(FIG_polyvec_stat);
		number %= 100;
		cnum = (number + 1) % 10;
		tnum = (number - 49) / 10;
		if(cnum < 5)
			line_color = (FIG_use_color ? BLACK : DEFAULT);
		else
			line_color = FIG_color;
		fill_color = (FIG_use_color) ? FIG_color : DEFAULT;
		if(cnum == 0 || cnum == 5)
			fill_style = -1;
		else
			fill_style = (cnum % 5) * 5;
		xpc = FIG_xoff + x;
		ypc = term->MaxY + FIG_yoff - y;
		if(tnum == 0) { /* circle */
			r = static_cast<int>(FIG_current_pointsize * term->ChrV / 4 + 1);
			fprintf(gpoutfile, "1 3 %d %d %d %d %d %d %d %6.3f 1 0.000 %d %d %d %d %d %d %d %d\n",
			    SOLID_LINE, FIG_thickness, line_color, fill_color, FIG_linedepth, 0, fill_style, FIG_spacing, xpc, ypc, r, r, xpc, ypc, xpc, ypc - r);
		}
		else {
			fprintf(gpoutfile, "2 3 %d %d %d %d %d %d %d %6.3f 0 0 0 0 0 ",
			    SOLID_LINE, FIG_thickness, line_color, fill_color, FIG_linedepth, 0, fill_style, FIG_spacing);
			if(tnum == 1) { /* square */
				d = static_cast<int>(FIG_current_pointsize * term->ChrV / 4 + 1);
				fprintf(gpoutfile, "5\n\t%d %d %d %d %d %d %d %d %d %d\n",
				    xpc - d, ypc - d, xpc - d, ypc + d, xpc + d, ypc + d, xpc + d, ypc - d, xpc - d, ypc - d);
			}
			else if(tnum == 2) { /* diamond */
				d = static_cast<int>(FIG_current_pointsize * term->ChrV / 3 + 1);
				fprintf(gpoutfile, "5\n\t%d %d %d %d %d %d %d %d %d %d\n", xpc - d, ypc, xpc, ypc + d, xpc + d, ypc, xpc, ypc - d, xpc - d, ypc);
			}
			else if(tnum == 3) { /* triangle up */
				d = static_cast<int>(FIG_current_pointsize * term->ChrV / 3 + 1);
				h = d * 4 / 7; /* About d times one 3rd of sqrt(3) */
				fprintf(gpoutfile, "4\n\t%d %d %d %d %d %d %d %d\n", xpc - d, ypc + h, xpc, ypc - 2 * h, xpc + d, ypc + h, xpc - d, ypc + h);
			}
			else if(tnum == 4) { /* triangle down */
				d = static_cast<int>(FIG_current_pointsize * term->ChrV / 3 + 1);
				h = d * 4 / 7;
				fprintf(gpoutfile, "4\n\t%d %d %d %d %d %d %d %d\n",
				    xpc - d, ypc - h, xpc, ypc + 2 * h, xpc + d, ypc - h, xpc - d, ypc - h);
			}
		}
	}
	else {
		int pt = number % 13;
		switch(pt) {
			default:        GnuPlot::DoPoint(x, y, pt); break;
			case 3:         FIG_lpoint(x, y, 64); break;
			case 4:         FIG_lpoint(x, y, 68); break;
			case 5:         FIG_lpoint(x, y, 54); break;
			case 6:         FIG_lpoint(x, y, 58); break;
			case 7:         FIG_lpoint(x, y, 84); break;
			case 8:         FIG_lpoint(x, y, 88); break;
			case 9:         FIG_lpoint(x, y, 94); break;
			case 10:        FIG_lpoint(x, y, 98); break;
			case 11:        FIG_lpoint(x, y, 74); break;
			case 12:        FIG_lpoint(x, y, 78); break;
		}
	}
}

TERM_PUBLIC void FIG_pointsize(double arg_pointsize)
{
	FIG_current_pointsize = arg_pointsize < 0. ? 1. : arg_pointsize;
	GnuPlot::DoPointSize(arg_pointsize * FIG_font_s / (double)FIG_FONT_S);
}

TERM_PUBLIC void FIG_linewidth(double linewidth)
{
	if(linewidth < 1)
		linewidth = 1;
	FIG_current_linewidth = linewidth;
}

TERM_PUBLIC void FIG_reset()
{
	FIG_poly_clean(FIG_polyvec_stat);
	FIG_posx = FIG_posy = 0;
	fflush(gpoutfile);
}

TERM_PUBLIC void FIG_boxfill(int style, uint x, uint y, uint w, uint h)
{
	int pen_color, fill_color, fill_style, fill_dens;
	FIG_poly_clean(FIG_polyvec_stat);
	FIG_line.pen_color = FIG_color;
	switch(style & 0xf) {
		case FS_SOLID:
		case FS_TRANSPARENT_SOLID:
		    /* style == 1 --> filled with intensity according to filldensity */
		    pen_color = FIG_line.pen_color;
		    fill_color = FIG_line.pen_color;
		    fill_dens = style >> 4;
		    if(fill_dens < 0) fill_dens = 0;
		    if(fill_dens > 100) fill_dens = 100;
		    if(FIG_color == -1 || FIG_color == 0)
			    /* default color or black: solid 0%...100% -> 0...20 */
			    fill_style = fill_dens / 5;
		    else
			    /* all other colors: solid 0%...100% -> 40...20 */
			    fill_style = 40 - fill_dens / 5;
		    break;
		case FS_PATTERN:
		case FS_TRANSPARENT_PATTERN:
		    /* style == 2 --> filled with pattern according to fillpattern */
		    pen_color = FIG_line.pen_color;
		    fill_color = FIG_BACKGROUND;
		    fill_style = 41 + ( ( (style>>4) < 0 ) ? 0 : style>>4 );
		    break;
		case FS_EMPTY:
		    pen_color = FIG_BACKGROUND;
		    fill_color = FIG_BACKGROUND;
		    fill_style = -1;
		    break;
		default:
		    /* style == 0 or unknown --> filled with background color */
		    pen_color = FIG_line.pen_color;
		    fill_color = FIG_line.pen_color;
		    fill_style = 20;
		    break;
	}

	x = FIG_xoff + x;
	y = term->MaxY + FIG_yoff - y;

	fprintf(gpoutfile, "%d %d %d %d %d %d %d %d %d %6.3f %d %d %d %d %d %d\n"
	    "  %d %d %d %d %d %d %d %d %d %d\n",
	    O_POLYLINE, T_BOX, FIG_line.style, FIG_line.thickness,
	    pen_color, fill_color, FIG_linedepth, FIG_line.pen_style,
	    fill_style, FIG_line.style_val, FIG_line.join_style,
	    FIG_line.cap_style, FIG_line.radius, 0, 0, 5,
	    x, y, x+w, y, x+w, y-h, x, y-h, x, y);
}

TERM_PUBLIC int FIG_make_palette(t_sm_palette * palette)
{
	int i;
	// Query to determine palette size 
	if(palette == NULL)
		return FIG_palette_size; /* How big is palette ? */
	if(FIG_palette_set == FALSE) {
		// Create new palette 
		FIG_palette_set = TRUE;
		if(FIG_use_color == FALSE || GPO.SmPltt.colorMode == SMPAL_COLOR_MODE_GRAY) {
			// Gray palette 
			if(FIG_use_color == FALSE && GPO.SmPltt.colorMode == SMPAL_COLOR_MODE_RGB)
				fprintf(stderr, "Monochrome fig file: using gray palette instead of color\n");
			for(i = 0; i < GPO.SmPltt.Colors; i++) {
				int j = (int)(i * 255.0 / (GPO.SmPltt.Colors-1) + 0.5);
				fprintf(gpoutfile, "%d %d #%2.2x%2.2x%2.2x\n", O_COLOR_DEF, (i + FIG_palette_offst), j, j, j);
			}
		}
		else {
			// Create colour/normal palette 
			for(i = 0; i < GPO.SmPltt.Colors; i++) {
				fprintf(gpoutfile, "%d %d #%2.2x%2.2x%2.2x\n", O_COLOR_DEF, (i + FIG_palette_offst),
				    (int)(palette->P_Color[i].r * 255 + 0.5), (int)(palette->P_Color[i].g * 255 + 0.5), (int)(palette->P_Color[i].b * 255 + 0.5));
				FIG_RGB_colors[FIG_palette_offst - FIG_rgb_color_offset + i] = (int)(palette->P_Color[i].r * 255 + 0.5) << 16 |
				    (int)(palette->P_Color[i].g * 255 + 0.5) << 8  | (int)(palette->P_Color[i].b * 255 + 0.5); 
			}
		}
	}
	else
		fprintf(stderr, "fig: Attempt to set palette twice\n");
	return 0;
}

TERM_PUBLIC void FIG_set_color(const t_colorspec * colorspec)
{
	double gray = colorspec->value;
	int new_color = FIG_color;
	int i;
	uint rgbval;
	switch(colorspec->type) {
		case TC_LT:
		    if(colorspec->lt == LT_BLACK)
			    new_color = BLACK;
		    else if(colorspec->lt == LT_AXIS)
			    new_color = FIG_gray_index;
		    else if(colorspec->lt <= LT_NODRAW)
			    new_color = FIG_BACKGROUND;
		    else
			    new_color = fig2pscolors[colorspec->lt % npscolors];
		    break;
		case TC_FRAC:
		    if(GPO.SmPltt.UseMaxColors != 0)
			    gray = GPO.QuantizeGray(gray);
		    new_color = (gray <= 0) ? 0 : ((gray >= 1) ? (GPO.SmPltt.Colors-1) : (int)(gray * GPO.SmPltt.Colors));
		    if(new_color >= FIG_palette_size)
			    new_color = FIG_palette_size - 1;
		    if(FIG_palette_set == FALSE)
			    GPO.IntWarn(NO_CARET, "fig: Palette used before set\n");
		    new_color += FIG_palette_offst;
		    break;
		case TC_RGB:
		    /* Look through all of user colors, named colors, and palette colors */
		    rgbval = colorspec->lt & 0xffffff;
		    if(rgbval == 0) {
			    new_color = BLACK;
			    break;
		    }
		    for(i = 0, new_color = -1; i < 256; i++) {
			    if(rgbval == FIG_RGB_colors[i]) {
				    new_color = FIG_rgb_color_offset + i;
				    break;
			    }
		    }
		    if(new_color < 0) {
			    /* FIXME: We could fill a separate array of up to 256 RGB colors
			     * encountered as we go.  They would then be available on "replot".
			     */
			    fprintf(stderr, "fig: RGB color 0x%x not found\n", rgbval);
			    fprintf(stderr, "     Try using it in a 'set linetype' command prior to plotting.\n");
			    new_color = FIG_gray_index;
		    }
		    break;
		default:
		    break;
	}
	if(FIG_color != new_color) {
		FIG_poly_clean(FIG_polyvec_stat);
		FIG_color = new_color;
	}
}

TERM_PUBLIC void FIG_filled_polygon(int points, gpiPoint * corners)
{
	int i, j;
	FIG_poly_clean(FIG_polyvec_stat); /* Clean up current data */
	fprintf(gpoutfile, "%d %d %d %d %d %d %d %d %d %9.3f %d %d %d %d %d %ld\n\t",
	    O_POLYLINE, T_POLYGON, FIG_line.style, 0,
	    FIG_color, FIG_color, FIG_linedepth,
	    FIG_line.pen_style, FIG_fill_style, FIG_line.style_val,
	    FIG_line.join_style, FIG_line.cap_style, FIG_line.radius,
	    0, 0, (long)(points+1));
	/* set thickness (arg 4) to 0 */
	j = 0;
	for(i = 0; i < points; i++) {
		fprintf(gpoutfile, " %d %d", FIG_xoff + corners[i].x,
		    term->MaxY + FIG_yoff - corners[i].y);
		if(j++ > 4 && i != points - 1) {
			fputs("\n\t", gpoutfile);
			j = 0;  /* JFS */
		}
	}
	fprintf(gpoutfile, " %d %d", FIG_xoff + corners[0].x, term->MaxY + FIG_yoff - corners[0].y);
	j++;
	if(j != 0) {
		putc('\n', gpoutfile);
	}
}

TERM_PUBLIC void FIG_layer(t_termlayer syncpoint)
{
	static int save_depth = FIG_DEPTH;
	// We must ignore all syncpoints that we don't recognize 
	switch(syncpoint) {
		default:
		    break;
		case TERM_LAYER_BEFORE_PLOT:
		    FIG_poly_clean(FIG_polyvec_stat);
		    fputs("6", gpoutfile);
		    /* Bounding box?  Give it the entire plot area */
		    fprintf(gpoutfile, " %d %d %d %d\n",
			FIG_xoff + GPO.V.BbPlot.xleft,
			term->MaxY + FIG_yoff - GPO.V.BbPlot.ytop,
			FIG_xoff + GPO.V.BbPlot.xright,
			term->MaxY + FIG_yoff - GPO.V.BbPlot.ybot);
		    fprintf(gpoutfile, "# Begin plot #%d\n", ++FIG_plotno);
		    FIG_current_layer = LAYER_PLOT;
		    FIG_linedepth = 700 - FIG_plotno;
		    break;
		case TERM_LAYER_AFTER_PLOT:
		    FIG_poly_clean(FIG_polyvec_stat);
		    fprintf(gpoutfile, "# End plot #%d\n", FIG_plotno);
		    fputs("-6\n", gpoutfile);
		    FIG_current_layer = LAYER_FRONT;
		    FIG_linedepth = 200;
		    break;
		case TERM_LAYER_RESET:
		    if(!multiplot)
			    FIG_plotno = 0;
		    FIG_current_layer = LAYER_BEHIND;
		    FIG_linedepth = 900;
		    break;
		case TERM_LAYER_BACKTEXT:
		    FIG_current_layer = LAYER_BACK;
		    FIG_linedepth = 800;
		    break;
		case TERM_LAYER_BEGIN_GRID:
		    save_depth = FIG_linedepth;
		    FIG_linedepth = 801;
		    break;
		case TERM_LAYER_END_GRID:
		    FIG_linedepth = save_depth;
		    break;
		case TERM_LAYER_BEGIN_KEYSAMPLE:
		    save_depth = FIG_linedepth;
		    FIG_linedepth = 201;
		    break;
		case TERM_LAYER_END_KEYSAMPLE:
		    FIG_linedepth = save_depth;
		    break;
		case TERM_LAYER_KEYBOX:
		    FIG_linedepth = 202;
		    break;
		case TERM_LAYER_BEGIN_PM3D_FLUSH:
		    save_depth = FIG_linedepth;
		    FIG_linedepth = 203;
		    break;
		case TERM_LAYER_END_PM3D_FLUSH:
		    FIG_linedepth = save_depth;
		    break;
	}
}

#endif /* TERM_BODY */

#ifdef TERM_TABLE

TERM_TABLE_START(fig_driver)
	"fig", 
	"FIG graphics language V3.2 for XFIG graphics editor",
	FIG_XMAX, 
	FIG_YMAX, 
	static_cast<uint>(FIG_VCHAR), 
	static_cast<uint>(FIG_HCHAR),
	FIG_VTIC, 
	FIG_HTIC, 
	FIG_options, 
	FIG_init, 
	FIG_reset,
	FIG_text, 
	GnuPlot::NullScale, 
	FIG_graphics, 
	FIG_move, 
	FIG_vector,
	FIG_linetype, 
	FIG_put_text, 
	FIG_text_angle, 
	FIG_justify_text,
	FIG_lpoint, 
	FIG_arrow, 
	set_font_null, 
	FIG_pointsize,
	TERM_BINARY|TERM_CAN_DASH, /*flags */
	0, /*suspend */ 
	0, /*resume */
	FIG_boxfill, 
	FIG_linewidth,
	#ifdef USE_MOUSE
	0, 
	0, 
	0, 
	0, 
	0,    /* no mouse support for the fig terminal */
	#endif
	FIG_make_palette, 
	0/*previous_palette*/, 
	FIG_set_color, 
	FIG_filled_polygon,
	0,
	0, 
	0, 
	0,       /* no enhanced text support */
	FIG_layer,
	NULL, 
	0.0, 
	NULL,       /* path, scale, hypertext */
	NULL, 
	NULL,            /* boxed_text, modify_plots */
	FIG_dashtype 
TERM_TABLE_END(fig_driver)

#undef LAST_TERM
#define LAST_TERM fig_driver
#endif /* TERM_TABLE */
#endif /* TERM_PROTO_ONLY */

#ifdef TERM_HELP
START_HELP(fig)
"1 fig",
"?commands set terminal fig",
"?set terminal fig",
"?set term fig",
"?terminal fig",
"?term fig",
"?fig",
"?xfig",
" The `fig` terminal device generates output in the Fig graphics language",
" for import into the xfig interactive drawing tool.",
" Notes: ",
"       The fig terminal was significantly revised in gnuplot version 5.3.",
"       Currently only version 3.2 of the fig file format is supported.",
"       Use of dash patterns may require Xfig 3.2.6 or newer.",
"",
" Syntax:",
"       set terminal fig {monochrome | color}",
"                        {small | big | size <xsize>{in|cm},<ysize>{in|cm}}",
"                        {landscape | portrait}",
"                        {font \"<fontname>{,<fontsize>}\"} {fontsize <size>}",
"                        {textnormal | {textspecial texthidden textrigid}}",
"                        {{linewidth|lw} <multiplier>}",
"",
" The default settings are",
"       set term fig color small landscape font \"Times Roman,10\" lw 1.0",
"",
" `size` sets the size of the drawing area to <xsize>*<ysize> in units of",
" inches (default) or centimeters. The default is `size 5in,3in`.",
" `small` is shorthand for `size 5in,3in` (3in,5in in portrait mode).",
" `big` is shorthand for `size 8in,5in`.",
"",
" `font` sets the text font face to <fontname> and its size to <fontsize>",
" points. Choice is limited to the 35 standard PostScript fonts.",
" `textnormal` resets the text flags and selects postscript fonts,",
" `textspecial` sets the text flags for LaTeX specials,",
" `texthidden` sets the hidden flag and `textrigid` the rigid flag.",
"",
" `linewidth` is a multiplier for the linewidth property of all lines.",
"",
" Additional point-plot symbols are also available in the `fig` driver. The",
" symbols can be used through `pointtype` values % 100 above 50, with different",
" fill intensities controlled by <pointtype> % 5 and outlines in black (for",
" <pointtype> % 10 < 5) or in the current color.  Available symbols are",
"         50 - 59:  circles",
"         60 - 69:  squares",
"         70 - 79:  diamonds",
"         80 - 89:  upwards triangles",
"         90 - 99:  downwards triangles",
" The size of these symbols scales with the font size.",
"",
" RGB colors will be replaced with gray unless they have been defined in a",
" linetype prior to plotting or match a known named color or palette value.",
" See `colornames`.",
" E.g.",
"       set linetype 999 lc rgb '#aabbcc'",
"       plot $data with fillecurve fillcolor rgb '#aabbcc'",
""
END_HELP(fig)
#endif /* TERM_HELP */

#if 0
/*		FORMAT3.2		*/
/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2002 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 1995 by C. Blanc and C. Schlick
 *
 * Any party obtaining a copy of these files is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and
 * documentation files (the "Software"), including without limitation the
 * rights to use, copy, modify, merge, publish and/or distribute copies of
 * the Software, and to permit persons who receive copies from any such
 * party to do so, with the only requirement being that this copyright
 * notice remain intact.
 */

/*

   The new components in protocol 3.2 are the paper size, magnification,
   single/multiple page indicator and transparent color for GIF export in the
   header.

   The other modification between version 3.1 and version 3.2 of the
   protocol is the mathematical model used for splines. The new version
   uses X-splines which allows the user to mix interpolation and approximation
   points in a same curve. More precisely, it means that an X-spline curve
   is neither an interpolated spline nor an approximated one, it is BOTH
   (the behaviour of each point is controlled by one single parameter
   called "shape factor"). For additional information about X-splines, see:

   "X-Splines: A Spline Model Designed for the End User"
   by C. Blanc and C. Schlick, Proceedings of SIGGRAPH95

   Caveat: Because spline models of previous versions (quadratic B-splines
   and Bezier with hidden points) are no longer supported, curves that are
   present in version 3.1 and older files are automatically converted to
   X-splines. This translation is only an approximation process. It means
   that the converted curves are not exactly the same as the original ones.
   Though the translation usually provides almost identical curves, some
   hand-fitting may be needed in some pathological cases.

   -------------------------------------------------------------------------------
   Description of the Fig Format Follows
   -------------------------------------------------------------------------------

   (1) The very first line is a comment line containing the name and version:
 #FIG 3.2

    The character # at the first column of a line indicates that the line
    is a comment line which will be preserved when the Fig file is read in.
    The user may edit them with the popup editor.

    The comment line(s) must immediately precede the object to which they
    are associated.  In the case of the "whole figure comments" mentioned
    below, they immediately precede the (resolution,coord_system) line.

   (2) The first non-comment line consists of the following:

        string	orientation		("Landscape" or "Portrait")
        string	justification		("Center" or "Flush Left")
        string	units			("Metric" or "Inches")
        string	papersize		("Letter", "Legal", "Ledger", "Tabloid",
                                         "A", "B", "C", "D", "E",
                                         "A4",   "A3", "A2", "A1", "A0" and "B5")
        float	magnification		(export and print magnification, %)
        string	multiple-page		("Single" or "Multiple" pages)
        int	transparent color	(color number for transparent color for GIF
                                         export. -3=background, -2=None, -1=Default,
                                         0-31 for standard colors or 32- for user colors)
 # optional comment		(An optional set of comments may be here,
                                         which are associated with the whole figure)
        int	resolution coord_system	(Fig units/inch and coordinate system:
                                           1: origin at lower left corner (NOT USED)
                                           2: upper left)

    Fig_resolution is the resolution of the figure in the file.
    Xfig will always write the file with a resolution of 1200ppi so it
    will scale the figure upon reading it in if its resolution is different
    from 1200ppi.  Pixels are assumed to be square.

    Note about metric units:  To preserve a regular grid on the canvas the
    centimeter is defined to be 450 Fig units and not 472.4 (1200/2.54).
    For drawings done in metric units, fig2dev magnifies the output when
    exporting or printing to compensate for the difference (472.4/450).
    Also, if you make a drawing in one unit scale and switch to the other
    units in xfig, the drawing will be rescaled on the screen by xfig to
    remain consistent.

    Xfig will read the orientation string and change the canvas to match
    either the Landscape or Portrait mode of the figure file.

    The units specification is self-explanatory.

    The coordinate_system variable is ignored - the origin is ALWAYS the
    upper-left corner.

 ** Coordinates are given in "fig_resolution" units.
 ** Line thicknesses are given in 1/80 inch (0.3175mm) or 1 screen pixel.
       When exporting to EPS, PostScript or any bitmap format (e.g. GIF),  the
       line thickness is reduced to 1/160 inch (0.159mm) to "lighten" the look.
 ** dash-lengths/dot-gaps are given in 80-ths of an inch.


   (3) The rest of the file contains various objects.  An object can be one
    of six classes (or types).

        0)	Color pseudo-object.
        1)	Ellipse which is a generalization of circle.
        2)	Polyline which includes polygon and box.
        3)	Spline which includes
                closed/open approximated/interpolated/x-spline spline.
        4)	Text.
        5)	Arc.
        6)	Compound object which is composed of one or more objects.

    In the following elaboration on object formats, every value of fig
    output are separated by blank characters or new line ('\n').  The
    value of the unused parameters will be -1.

    Some fields are described as "enumeration type" or "bit vector"; the
    values which these fields can take are defined in the header file object.h.
    The pen_style field is unused.
    These values may be defined in some future version of Fig.

    The two color fields (pen and fill; pen only, for texts) are
    defined as follows:

            -1 = Default
             0 = Black
             1 = Blue
             2 = Green
             3 = Cyan
             4 = Red
             5 = Magenta
             6 = Yellow
             7 = White
          8-11 = four shades of blue (dark to lighter)
         12-14 = three shades of green (dark to lighter)
         15-17 = three shades of cyan (dark to lighter)
         18-20 = three shades of red (dark to lighter)
         21-23 = three shades of magenta (dark to lighter)
         24-26 = three shades of brown (dark to lighter)
         27-30 = four shades of pink (dark to lighter)
            31 = Gold

         values from 32 to 543 (512 total) are user colors and
         are defined in color pseudo-objects (type 0)

         Your X server may limit the number of colors to something less
         than this, especially on a 8-bit PseudoColor visual, where
         the number of usable colors will be 256 minus the number of colors
         xfig preallocates for itself and the 32 standard colors (about 48).

    For WHITE color, the area fill field is defined as follows:

        -1 = not filled
         0 = black
        ...  values from 1 to 19 are shades of grey, from darker to lighter
        20 = white
        21-40 not used
        41-56 see patterns for colors, below

    For BLACK or DEFAULT color, the area fill field is defined as follows:

        -1 = not filled
         0 = white
        ...  values from 1 to 19 are shades of grey, from lighter to darker
        20 = black
        21-40 not used
        41-56 see patterns for colors, below

    For all other colors, the area fill field is defined as follows:

        -1 = not filled
         0 = black
        ...  values from 1 to 19 are "shades" of the color, from darker to lighter.
                A shade is defined as the color mixed with black
        20 = full saturation of the color
        ...  values from 21 to 39 are "tints" of the color from the color to white.
                A tint is defined as the color mixed with white
        40 = white
        41 = 30 degree left diagonal pattern
        42 = 30 degree right diagonal pattern
        43 = 30 degree crosshatch
        44 = 45 degree left diagonal pattern
        45 = 45 degree right diagonal pattern
        46 = 45 degree crosshatch
        47 = horizontal bricks
        48 = vertical bricks
        49 = horizontal lines
        50 = vertical lines
        51 = crosshatch
        52 = horizontal "shingles" skewed to the right
        53 = horizontal "shingles" skewed to the left
        54 = vertical "shingles" skewed one way
        55 = vertical "shingles"skewed the other way
        56 = fish scales
        57 = small fish scales
        58 = circles
        59 = hexagons
        60 = octagons
        61 = horizontal "tire treads"
        62 = vertical "tire treads"

    The depth field is defined as follows:

         0 ... 999 where larger value means object is deeper than (under)
                   objects with smaller depth

    The line_style field is defined as follows:

        -1 = Default
         0 = Solid
         1 = Dashed
         2 = Dotted
         3 = Dash-dotted
         4 = Dash-double-dotted
         5 = Dash-triple-dotted

    The style_val field is defined as the length, in 1/80 inches, of the on/off
    dashes for dashed lines, and the distance between the dots, in 1/80 inches,
    for dotted lines.

    The join_style field is defined FOR LINES only as follows:

         0 = Miter (the default in xfig 2.1 and earlier)
         1 = Round
         2 = Bevel

    The cap_style field is defined FOR LINES, OPEN SPLINES and ARCS only as follows:

         0 = Butt (the default in xfig 2.1 and earlier)
         1 = Round
         2 = Projecting

    The arrow_type field is defined for LINES, ARCS and OPEN SPLINES
    only as follows:

         0 = Stick-type (the default in xfig 2.1 and earlier)

 \
 \
        _______________\
                       /
                    /
                 /

         1 = Closed triangle:

 |\
 |   \
        ________|      \
 |      /
 |   /
 |/

         2 = Closed with "indented" butt:

 |\
 \   \
 \     \
        __________\       \
                  /       /
                 /     /
                /   /
 |/

         3 = Closed with "pointed" butt:

                   /\
                  /    \
                 /        \
        ________/            \
 \            /
 \        /
 \    /
 \/

    The arrow_style field is defined for LINES, ARCS and OPEN SPLINES
    only as follows:

         0 = Hollow (actually filled with white)
         1 = Filled with pen_color

   (3.0) OBJECT DEFINITIONS:

    ================================================
    (3.1) Color Pseudo-objects (user-defined colors)
    ================================================
          This is used to define arbitrary colors beyond the 32 standard colors.
          The color objects must be defined before any other Fig objects.

    First line:
        type	name			(brief description)
        ----	----			-------------------
        int	object_code		(always 0)
        int	color_number		(color number, from 32-543 (512 total))
        hex string  rgb values		(hexadecimal string describing red,
                                         green and blue values (e.g. #330099) )

    ============================================================================
    (3.2) ARC
    =========

    First line:
        type	name			(brief description)
        ----	----			-------------------
        int	object_code		(always 5)
        int	sub_type		(1: open ended arc
                                         2: pie-wedge (closed) )
        int	line_style		(enumeration type, solid, dash, dotted, etc.)
        int	line_thickness		(1/80 inch)
        int	pen_color		(enumeration type, pen color)
        int	fill_color		(enumeration type, fill color)
        int	depth			(enumeration type)
        int	pen_style		(pen style, not used)
        int	area_fill		(enumeration type, -1 = no fill)
        float	style_val		(1/80 inch, specification for dash/dotted lines)
        int	cap_style		(enumeration type)
        int	direction		(0: clockwise, 1: counterclockwise)
        int	forward_arrow		(0: no forward arrow, 1: on)
        int	backward_arrow		(0: no backward arrow, 1: on)
        float	center_x, center_y	(center of the arc)
        int	x1, y1			(Fig units, the 1st point the user entered)
        int	x2, y2			(Fig units, the 2nd point)
        int	x3, y3			(Fig units, the last point)

    Forward arrow line (Optional; absent if forward_arrow is 0):
        type	name			(brief description)
        ----	----			-------------------
        int	arrow_type		(enumeration type)
        int	arrow_style		(enumeration type)
        float	arrow_thickness		(1/80 inch)
        float   arrow_width		(Fig units)
        float	arrow_height		(Fig units)

    Backward arrow line (Optional; absent if backward_arrow is 0):
        type	name			(brief description)
        ----	----			-------------------
        int	arrow_type		(enumeration type)
        int	arrow_style		(enumeration type)
        float	arrow_thickness		(1/80 inch)
        float	arrow_width		(Fig units)
        float	arrow_height		(Fig units)

    ============================================================================
    (3.3) COMPOUND
    ==============

    A line with object code 6 signifies the start of a compound.
    There are four more numbers on this line which indicate the
    upper left corner and the lower right corner of the bounding
    box of this compound.  A line with object code -6 signifies
    the end of the compound.  Compound may be nested.

    First line:
        type	name			(brief description)
        ----	----			-------------------
        int	object_code		(always 6)
        int	upperleft_corner_x	(Fig units)
        int	upperleft_corner_y	(Fig units)
        int	lowerright_corner_x	(Fig units)
        int	lowerright_corner_y	(Fig units)

    Subsequent lines:
        objects
        .
        .

    Last line:
        -6

    ============================================================================
    (3.4) ELLIPSE
    =============

    First line:
        type	name			(brief description)
        ----	----			-------------------
        int	object_code		(always 1)
        int	sub_type		(1: ellipse defined by radii
                                         2: ellipse defined by diameters
                                         3: circle defined by radius
                                         4: circle defined by diameter)
        int	line_style		(enumeration type, solid, dash, dotted, etc.)
        int	thickness		(1/80 inch)
        int	pen_color		(enumeration type, pen color)
        int	fill_color		(enumeration type, fill color)
        int	depth			(enumeration type)
        int	pen_style		(pen style, not used)
        int	area_fill		(enumeration type, -1 = no fill)
        float	style_val		(1/80 inch, specification for dash/dotted lines)
        int	direction		(always 1)
        float	angle			(radians, the angle of the x-axis)
        int	center_x, center_y	(Fig units)
        int	radius_x, radius_y	(Fig units)
        int	start_x, start_y	(Fig units; the 1st point entered)
        int	end_x, end_y		(Fig units; the last point entered)

    ============================================================================
    (3.5) POLYLINE
    ==============

    First line:
        type	name			(brief description)
        ----	----			-------------------
        int	object_code		(always 2)
        int	sub_type		(1: polyline
                                         2: box
                                         3: polygon
                                         4: arc-box)
                                         5: imported-picture bounding-box)
        int	line_style		(enumeration type, solid, dash, dotted, etc.)
        int	thickness		(1/80 inch)
        int	pen_color		(enumeration type, pen color)
        int	fill_color		(enumeration type, fill color)
        int	depth			(enumeration type)
        int	pen_style		(pen style, not used)
        int	area_fill		(enumeration type, -1 = no fill)
        float	style_val		(1/80 inch, specification for dash/dotted lines)
        int	join_style		(enumeration type)
        int	cap_style		(enumeration type, only used for POLYLINE)
        int	radius			(1/80 inch, radius of arc-boxes)
        int	forward_arrow		(0: off, 1: on)
        int	backward_arrow		(0: off, 1: on)
        int	npoints			(number of points in line)

    Forward arrow line: same as ARC object

    Backward arrow line: same as ARC object

    For picture (type 5) the following line follows:

        type	name			(brief description)
        ----	----			-------------------
        boolean	flipped			orientation = normal (0) or flipped (1)
        char	file[]			name of picture file to import

    Points line(s).  The x,y coordinates follow, any number to a line, with
    as many lines as are necessary:

        type	name			(brief description)
        ----	----			-------------------
        int	x1, y1			(Fig units)
        int	x2, y2			(Fig units)
          .
          .
        int	xnpoints ynpoints	(this will be the same as the 1st
                                        point for polygon and box)

    ============================================================================
    (3.6) SPLINE
    ============

    First line:
        type	name			(brief description)
        ----	----			-------------------
        int	object_code		(always 3)
        int	sub_type		(0: open approximated spline
                                             1: closed approximated spline
                                             2: open   interpolated spline
                                             3: closed interpolated spline
                                             4: open   x-spline
                                             5: closed x-spline)
        int	line_style		(enumeration type, solid, dash, dotted, etc.)
        int	thickness		(1/80 inch)
        int	pen_color		(enumeration type, pen color)
        int	fill_color		(enumeration type, fill color)
        int	depth			(enumeration type)
        int	pen_style		(pen style, not used)
        int	area_fill		(enumeration type, -1 = no fill)
        float	style_val		(1/80 inch, specification for dash/dotted lines)
        int	cap_style		(enumeration type, only used for open splines)
        int	forward_arrow		(0: off, 1: on)
        int	backward_arrow		(0: off, 1: on)
        int	npoints			(number of control points in spline)

    Forward arrow line: same as ARC object

    Backward arrow line: same as ARC object

    Points line: same as POLYLINE object

    Control points line :

    There is one shape factor for each point. The value of this factor
    must be between -1 (which means that the spline is interpolated at
    this point) and 1 (which means that the spline is approximated at
    this point). The spline is always smooth in the neighbourhood of a
    control point, except when the value of	the factor is 0 for which
    there is a first-order discontinuity (i.e. angular point).


    ============================================================================
    (3.7) TEXT
    ==========

        type	name			(brief description)
        ----	----			-------------------
        int	object                  (always 4)
        int	sub_type		(0: Left justified
                                             1: Center justified
                                             2: Right justified)
        int	color			(enumeration type)
        int	depth			(enumeration type)
        int	pen_style		(enumeration , not used)
        int	font                    (enumeration type)
        float	font_size               (font size in points)
        float	angle			(radians, the angle of the text)
        int	font_flags		(bit vector)
        float	height			(Fig units)
        float	length			(Fig units)
        int	x, y			(Fig units, coordinate of the origin
                                         of the string.  If sub_type = 0, it is
                                         the lower left corner of the string.
                                         If sub_type = 1, it is the lower
                                         center.  Otherwise it is the lower
                                         right corner of the string.)
        char	string[]		(ASCII characters; starts after a blank
                                         character following the last number and
                                         ends before the sequence '\001'.  This
                                         sequence is not part of the string.
                                         Characters above octal 177 are
                                         represented by \xxx where xxx is the
                                         octal value.  This permits fig files to
                                         be edited with 7-bit editors and sent
                                         by e-mail without data loss.
                                         Note that the string may contain '\n'.)

    The font_flags field is defined as follows:

         Bit	Description

          0	Rigid text (text does not scale when scaling compound objects)
          1	Special text (for LaTeX)
          2	PostScript font (otherwise LaTeX font is used)
          3	Hidden text

    The font field is defined as follows:

        For font_flags bit 2 = 0 (LaTeX fonts):

         0	Default font
         1	Roman
         2	Bold
         3	Italic
         4	Sans Serif
         5	Typewriter

        For font_flags bit 2 = 1 (PostScript fonts):

        -1	Default font
         0	Times Roman
         1	Times Italic
         2	Times Bold
         3	Times Bold Italic
         4	AvantGarde Book
         5	AvantGarde Book Oblique
         6	AvantGarde Demi
         7	AvantGarde Demi Oblique
         8	Bookman Light
         9	Bookman Light Italic
        10	Bookman Demi
        11	Bookman Demi Italic
        12	Courier
        13	Courier Oblique
        14	Courier Bold
        15	Courier Bold Oblique
        16	Helvetica
        17	Helvetica Oblique
        18	Helvetica Bold
        19	Helvetica Bold Oblique
        20	Helvetica Narrow
        21	Helvetica Narrow Oblique
        22	Helvetica Narrow Bold
        23	Helvetica Narrow Bold Oblique
        24	New Century Schoolbook Roman
        25	New Century Schoolbook Italic
        26	New Century Schoolbook Bold
        27	New Century Schoolbook Bold Italic
        28	Palatino Roman
        29	Palatino Italic
        30	Palatino Bold
        31	Palatino Bold Italic
        32	Symbol
        33	Zapf Chancery Medium Italic
        34	Zapf Dingbats

 */
#endif
