// GNUPLOT - context.trm 
// Copyright (c) 2006-2011, Mojca Miklavec All rights reserved.
/*
 * AUTHORS:
 *   Mojca Miklavec
 *       <mojca.miklavec.lists at gmail.com>
 *     - author of this file
 *     - improvements to m-gnuplot.tex
 *     - mp-gnuplot.mp - metapost macros to support gnuplot-specifics
 *       inside ConTeXt
 *
 *   Hans Hagen (author of ConTeXt)
 *        <pragma at wxs.nl>
 *      - a lot of functionality added to ConTeXt to enable better support
 *        of Gnuplot module, many bugs fixed
 *      - author of the module m-gnuplot.tex to support inclusion of Gnuplot
 *        graphics into ConTeXt
 *      - constant support on the most tricky issues
 *   Taco Hoekwater (developer of MetaPost, pdfTeX & LuaTeX)
 *        <taco at elvenkind.com>
 *      - some code of this file, many tricks, lots of bug fixes,
 *        suggestions and testing
 *      - Hans's right hand in ConTeXt development:
 *        constant support on even more tricky issues
 *
 *   For questions, suggestions, comments, improvements please contact:
 *     - Mojca Miklavec <mojca.miklavec.lists at gmail.com> or
 *     - <gnuplot-context at googlegroups.com>
 *     - http://wiki.contextgarden.net/Gnuplot
 *
 * With special thanks to:
 * - Peter Münster (ctx) for the initiative
 * - Renaud Aubin (ctx) for maintaining the first source code repository
 * - Aditya Mahajan (ctx) for some tricky parts in m-gnuplot.tex
 * - Timothée Lecomte & Ethan A Merritt (gnuplot) for many suggestion about improving this terminal
 * - Bastian Märkisch (gnuplot) for code improvements
 *
 */
/*
 * GNUPLOT -- context.trm
 *
 * This driver creates a ConTeXt document with embded metapost (metafun)
 * commands and is consequently used for creation of PDF documents.
 *
 * It is in a way similar to pslatex, but specialized for ConTeXt,
 * where it can also be used in the following way:
 *
 * \usemodule[gnuplot]
 *
 * \starttext
 * \title{Drawing nice graphs with \sc gnuplot}
 * \startGNUPLOTscript{sin}
 * plot sin(x) t '$\sin(x)$'
 * \stopGNUPLOTscript
 * \useGNUPLOTgraphic[sin]
 * \stoptext
 *
 * For more information see http://wiki.contextgarden.net/Gnuplot
 *
 * Mostly based on:
 * - default settings copied from LaTeX terminal (because I liked it)
 * - path construction & drawing routines from MetaPost terminal
 * - advanced functionality with reference to PostScript terminal
 *
 * Future plans:
 * - most important
 *   - different syntax for font switching
 *   - improve support for (transparent) binary images
 * - add missing functionality:
 *   - improved support for palettes
 *   - smooth shading in color bars
 *   - gouraud shading (if it will be implemented in gnuplot)
 *   - other color spaces
 * - derive a better metapost terminal out of this one (to replace the old one)
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
	register_term(context)
#endif

//#ifdef TERM_PROTO
TERM_PUBLIC void CONTEXT_options();
TERM_PUBLIC void CONTEXT_init(termentry * pThis);
TERM_PUBLIC void CONTEXT_reset();
TERM_PUBLIC void CONTEXT_text();
TERM_PUBLIC void CONTEXT_graphics();
TERM_PUBLIC void CONTEXT_move(uint x, uint y);
TERM_PUBLIC void CONTEXT_vector(uint x, uint y);
TERM_PUBLIC void CONTEXT_linetype(int lt);
TERM_PUBLIC void CONTEXT_put_text(uint x, uint y, const char * str);
TERM_PUBLIC int CONTEXT_text_angle(int ang);
TERM_PUBLIC int CONTEXT_justify_text(enum JUSTIFY mode);
TERM_PUBLIC void CONTEXT_point(uint x, uint y, int number);
TERM_PUBLIC void CONTEXT_arrow(uint sx, uint sy, uint ex, uint ey, int head);
TERM_PUBLIC int CONTEXT_set_font(const char * font); /* "font,size" */
TERM_PUBLIC void CONTEXT_pointsize(double pointsize);
TERM_PUBLIC void CONTEXT_fillbox(int style, uint x1, uint y1, uint width, uint height); // clear part of multiplot
TERM_PUBLIC void CONTEXT_fill(int style);
TERM_PUBLIC void CONTEXT_linewidth(double linewidth);
TERM_PUBLIC int CONTEXT_make_palette(t_sm_palette * palette);
/* TERM_PUBLIC void CONTEXT_previous_palette(); do we need it? */
TERM_PUBLIC void CONTEXT_set_color(const t_colorspec * colorspec);
TERM_PUBLIC void CONTEXT_filled_polygon(int points, gpiPoint * corners);
TERM_PUBLIC void CONTEXT_image(unsigned, unsigned, coordval *, gpiPoint *, t_imagecolor);

/* Metapost < 1.750 can only deal with numbers between 2^{-16} and 2^12=4069 */
/* scale is 1cm = 1000 units */

#define CONTEXT_DPCM 1000
#define CONTEXT_DPI (2.54 * CONTEXT_DPCM)
/* default plot size will be 5in x 3in */
#define CONTEXT_XSIZE_VALUE 5
#define CONTEXT_YSIZE_VALUE 3
#define CONTEXT_SIZE_UNIT INCHES
#define CONTEXT_XMAX (CONTEXT_XSIZE_VALUE * CONTEXT_DPI)
#define CONTEXT_YMAX (CONTEXT_YSIZE_VALUE * CONTEXT_DPI)
/* default fontsize: 12pt */
#define CONTEXT_FONTSIZE 12.0
/* default height of char: 12pt by default */
#define CONTEXT_VCHAR (CONTEXT_DPI * CONTEXT_FONTSIZE / 72.27)
/* in ConTeXt 12pt LM font is the default;
 * at that size digits are 5.8749847pt wide
 * in LaTeX, which assumes 11pt, the ratio is 5.3/11, which is similar */
#define CONTEXT_LM_H_TO_V_RATIO 0.4895
#define CONTEXT_HCHAR (CONTEXT_LM_H_TO_V_RATIO * CONTEXT_VCHAR)
/* default tic size: 3.5bp (chosen to suit the size of plot approximately)
 * - in LaTeX it is 5bp
 * - in TikZ it is 0.18cm (approximately 5.1bp)
 * - in PostScript it is 3.15pt
 * - MetaPost uses 5pt or 5bp
 */
#define CONTEXT_HTIC (3.5 * CONTEXT_DPI / 72)
#define CONTEXT_VTIC CONTEXT_HTIC

//#endif /* TERM_PROTO */

#ifndef TERM_PROTO_ONLY
#ifdef TERM_BODY

/*
 * started counting when the code was included into CVS
 *
 * major number should change only in case of a complete rewrite or major incompatibility
 * minor number should change for every new functionality
 * patch number & date should change in every commit
 */
static const char CONTEXT_term_version[] = "1.0";
static const char CONTEXT_term_patch[]   = "0";
static const char CONTEXT_term_date[]    = "2011-11-05";

static void CONTEXT_params_reset();
static void CONTEXT_adjust_dimensions();
static void CONTEXT_fontstring_parse(char * from_string, char * to_string, int to_size, double * fontsize);
static void CONTEXT_startpath();
static void CONTEXT_endpath();
static void CONTEXT_write_palette(t_sm_palette * palette);
static void CONTEXT_write_palette_gradient(gradient_struct * gradient, int cnt);

/* Each number is divided by 100 (1/100th of a point is drawn) */

static int CONTEXT_posx;
static int CONTEXT_posy;
/* remembers where we started the path:
 * if we finish it in the same point, the path is closed with --cycle
 *
 * After I implemented this, the functionality has been added to gnuplot core
 * and PostScript terminal uses it, but to be on the safe side,
 * I prefer not to add last-minute patches which could potentially break something.
 * (the code written here proved to be rather safe & working so far)
 * In the next version the core functionality should be integrated & tested.
 */
static int CONTEXT_path_start_x;
static int CONTEXT_path_start_y;
/* fontname, fontsize */
static char CONTEXT_font[MAX_ID_LEN+1] = "";
static double CONTEXT_fontsize = CONTEXT_FONTSIZE;

/* fontname,fontsize to be put next to font labels if needed */
static char CONTEXT_font_explicit[2*MAX_ID_LEN+1] = "";
/* this is only set to >0 if asked for explicitly (for example with set_font(",15")) */
static double CONTEXT_fontsize_explicit = 0.0;

static double CONTEXT_old_pointsize = 1.0; // the last pointsize used (it will only be changed if it becomes different) 
static double CONTEXT_old_linewidth = 1.0; // the last linewidth used (it will only be changed if it becomes different) 
static int CONTEXT_old_linetype = -3; // the last linetype used (it will only be changed if it becomes different) 
static bool CONTEXT_color_changed = FALSE; // was the color changed explicitly? 
// the number of path nodes before a newline (doesn't really matter, could be set to 1000; check if any editors have problems with that) 
#define CONTEXT_LINEMAX 4
static uint CONTEXT_path_count = 0; // if we're inside a path (unfinished line) then path_count > 0 (PDF has PDF_pathIsOpen) 
static uint CONTEXT_path_is_dot = 0; // this true/false switch is used to help distinguish dots from paths 
static int CONTEXT_ang = 0; // angle of text rotation 
static enum JUSTIFY CONTEXT_justify = LEFT; // left/middle/right text justification 

#ifdef OS2 /* same constants are defined in os2emx.h */
	enum LINEJOIN { LINEJOIN_MITER = LINEJOIN_MITRE, _LINEJOIN_ROUND = LINEJOIN_ROUND, _LINEJOIN_BEVEL = LINEJOIN_BEVEL };
#else
	enum LINEJOIN { LINEJOIN_MITER, LINEJOIN_ROUND, LINEJOIN_BEVEL };
#endif
enum LINECAP { LINECAP_BUTT, LINECAP_ROUND, LINECAP_SQUARE };

/* whether points are drawn with metapost or typeset with TeX (easy configurable) */
enum CONTEXT_POINTS { CONTEXT_POINTS_WITH_METAPOST, CONTEXT_POINTS_WITH_TEX };

/* whether images are inline or written out as PNGs and included (in MKII only external work) */
enum CONTEXT_IMAGES { CONTEXT_IMAGES_INLINE, CONTEXT_IMAGES_EXTERNAL };

static int CONTEXT_counter = 0; /* counts the number of graphics */
static int CONTEXT_image_counter = 0; /* counts the number of external PNG images */
static int CONTEXT_image_filename_length = 0; /* length of basename for storing image name (including path, excluding extension) */
static int CONTEXT_image_filename_start = 0; /* if name is a path, remember at which index basename starts */
static char * CONTEXT_image_filename = NULL;
enum CONTEXT_IMAGES CONTEXT_images = CONTEXT_IMAGES_INLINE; /* whether images will be external or not */
static t_sm_palette * CONTEXT_old_palette; /* Palette has to be stored for the usage in later plots */

/*********************/
/* global parameters */
/*********************/

typedef struct CONTEXT_params_t {
	double xsize;                            /* 5in    */
	double ysize;                            /* 3in    */
	GpSizeUnits unit;                         /* INCHES */
	bool standalone;                     /* false  */
	bool timestamp;                      /* true   */
	char               * header;              /* ""     */
	bool color;                          /* true   */
	bool dashed;                         /* true   */
	enum LINEJOIN linejoin;                  /* MITER  */
	enum LINECAP linecap;                    /* BUTT   */
	double scale_dashlength;                 /* 1.0    */
	double scale_linewidth;                  /* 1.0    */
	double scale_text;                       /* 1.0    */
	enum CONTEXT_POINTS points;              /* CONTEXT_POINTS_WITH_METAPOST */
	enum CONTEXT_IMAGES images;              /* CONTEXT_IMAGES_INLINE */
	char font[MAX_ID_LEN+1];                 /* ""     */
	double fontsize;                         /* 12pt   */
} CONTEXT_params_t;

#define CONTEXT_PARAMS_DEFAULT { \
		CONTEXT_XSIZE_VALUE, \
		CONTEXT_YSIZE_VALUE, \
		INCHES, \
		FALSE, \
		TRUE, \
		NULL, \
		TRUE, \
		TRUE, \
		LINEJOIN_MITER, \
		LINECAP_BUTT, \
		1.0, \
		1.0, \
		1.0, \
		CONTEXT_POINTS_WITH_METAPOST, \
		CONTEXT_IMAGES_INLINE, \
		"", \
		CONTEXT_FONTSIZE \
}

static CONTEXT_params_t CONTEXT_params = CONTEXT_PARAMS_DEFAULT;

enum CONTEXT_id {
	CONTEXT_OPT_DEFAULT,
	CONTEXT_OPT_SIZE,
	CONTEXT_OPT_SIZE_DEFAULT,
	CONTEXT_OPT_INPUT, CONTEXT_OPT_STANDALONE,
	CONTEXT_OPT_TIMESTAMP, CONTEXT_OPT_NOTIMESTAMP,
	CONTEXT_OPT_HEADER, CONTEXT_OPT_NOHEADER,
	CONTEXT_OPT_COLOR, CONTEXT_OPT_MONOCHROME,
	CONTEXT_OPT_DASHED, CONTEXT_OPT_SOLID,
	CONTEXT_OPT_LINEJOIN_MITERED, CONTEXT_OPT_LINEJOIN_ROUNDED, CONTEXT_OPT_LINEJOIN_BEVELED,
	CONTEXT_OPT_LINECAP_BUTT, CONTEXT_OPT_LINECAP_ROUNDED, CONTEXT_OPT_LINECAP_SQUARED,
	CONTEXT_OPT_SCALE_DASHLENGTH, CONTEXT_OPT_SCALE_LINEWIDTH, CONTEXT_OPT_SCALE_TEXT,
	CONTEXT_OPT_POINTS_WITH_METAPOST, CONTEXT_OPT_POINTS_WITH_TEX,
	CONTEXT_OPT_IMAGES_INLINE, CONTEXT_OPT_IMAGES_EXTERNAL,
	CONTEXT_OPT_DEFAULTFONT,
	CONTEXT_OPT_FONT,
	CONTEXT_OPT_OTHER
};

static struct gen_table CONTEXT_opts[] = {
	{ "d$efault", CONTEXT_OPT_DEFAULT },
	{ "size", CONTEXT_OPT_SIZE },
	{ "defaultsize", CONTEXT_OPT_SIZE_DEFAULT },
	{ "inp$ut", CONTEXT_OPT_INPUT },
	{ "stand$alone", CONTEXT_OPT_STANDALONE },
	{ "time$stamp", CONTEXT_OPT_TIMESTAMP },
	{ "notime$stamp", CONTEXT_OPT_NOTIMESTAMP },
	{ "header", CONTEXT_OPT_HEADER },
	{ "noheader", CONTEXT_OPT_NOHEADER },
	{ "col$or", CONTEXT_OPT_COLOR },
	{ "col$our", CONTEXT_OPT_COLOR },
	{ "mono$chrome", CONTEXT_OPT_MONOCHROME },
	{ "da$shed", CONTEXT_OPT_DASHED },
	{ "so$lid", CONTEXT_OPT_SOLID },
	{ "miter$ed", CONTEXT_OPT_LINEJOIN_MITERED },
	{ "rounded", CONTEXT_OPT_LINEJOIN_ROUNDED },
	{ "bevel$ed", CONTEXT_OPT_LINEJOIN_BEVELED },
	{ "butt", CONTEXT_OPT_LINECAP_BUTT },
	{ "round", CONTEXT_OPT_LINECAP_ROUNDED },
	{ "square$d", CONTEXT_OPT_LINECAP_SQUARED },
	{ "dashl$ength", CONTEXT_OPT_SCALE_DASHLENGTH },
	{ "dl", CONTEXT_OPT_SCALE_DASHLENGTH },
	{ "linew$idth", CONTEXT_OPT_SCALE_LINEWIDTH },
	{ "lw", CONTEXT_OPT_SCALE_LINEWIDTH },
	{ "fontscale", CONTEXT_OPT_SCALE_TEXT },
	{ "textscale", CONTEXT_OPT_SCALE_TEXT },                   /* backward compatibility */
	{ "mp$points", CONTEXT_OPT_POINTS_WITH_METAPOST},
	{ "tex$points", CONTEXT_OPT_POINTS_WITH_TEX},
	{ "pointswithmp", CONTEXT_OPT_POINTS_WITH_METAPOST},       /* (removable) backward compatibility */
	{ "pointswithmetapost", CONTEXT_OPT_POINTS_WITH_METAPOST}, /* (removable) backward compatibility */
	{ "pointswithtex", CONTEXT_OPT_POINTS_WITH_TEX},           /* (removable) backward compatibility */
	{ "inline$images", CONTEXT_OPT_IMAGES_INLINE},
	{ "external$images", CONTEXT_OPT_IMAGES_EXTERNAL},
	{ "font", CONTEXT_OPT_FONT },
	{ "defaultfont", CONTEXT_OPT_DEFAULTFONT },
	{ NULL, CONTEXT_OPT_OTHER }
};

/* **********************
 * CONTEXT_params_reset *
 * **********************
 *
 * Resets all parameters of the terminal to their default value.
 */
static void CONTEXT_params_reset()
{
	static const CONTEXT_params_t CONTEXT_params_default = CONTEXT_PARAMS_DEFAULT;
	// free the memory with header first 
	ZFREE(CONTEXT_params.header);
	memcpy(&CONTEXT_params, &CONTEXT_params_default, sizeof(CONTEXT_params_t));
}

/* *****************
 * CONTEXT_options *
 * *****************
 *
 * Parses "set term context [options]".
 */
TERM_PUBLIC void CONTEXT_options(TERMENTRY * pThis, GnuPlot * pGp)
{
	char   * tmp_string;
	char tmp_term_options[MAX_LINE_LEN+1] = "";
	while(!pGp->Pgm.EndOfCommand()) {
		switch(pGp->Pgm.LookupTableForCurrentToken(&CONTEXT_opts[0])) {
			case CONTEXT_OPT_DEFAULT:
			    pGp->Pgm.Shift();
			    /* there should be a better way to do it, but I don't know how */
			    CONTEXT_params_reset();
			    break;
			case CONTEXT_OPT_SIZE: {
			    float xmax_t, ymax_t;
			    GpSizeUnits unit;
			    pGp->Pgm.Shift();
			    // size <xsize> {cm|in}, <ysize> {cm|in} 
			    unit = pGp->ParseTermSize(&xmax_t, &ymax_t, CM);
			    CONTEXT_params.xsize = (double)xmax_t / GpResolution;
			    CONTEXT_params.ysize = (double)ymax_t / GpResolution;
			    CONTEXT_params.unit  = unit;
			    if(unit == CM) {
				    CONTEXT_params.xsize *= 2.54;
				    CONTEXT_params.ysize *= 2.54;
			    }
			    break;
		    }
			case CONTEXT_OPT_SIZE_DEFAULT:
			    pGp->Pgm.Shift();
			    CONTEXT_params.xsize = CONTEXT_XSIZE_VALUE;
			    CONTEXT_params.ysize = CONTEXT_YSIZE_VALUE;
			    CONTEXT_params.unit  = CONTEXT_SIZE_UNIT;
			    break;
			case CONTEXT_OPT_INPUT:
			    pGp->Pgm.Shift();
			    CONTEXT_params.standalone = FALSE;
			    break;
			case CONTEXT_OPT_STANDALONE:
			    pGp->Pgm.Shift();
			    CONTEXT_params.standalone = TRUE;
			    break;
			case CONTEXT_OPT_TIMESTAMP:
			    pGp->Pgm.Shift();
			    CONTEXT_params.timestamp = TRUE;
			    break;
			case CONTEXT_OPT_NOTIMESTAMP:
			    pGp->Pgm.Shift();
			    CONTEXT_params.timestamp = FALSE;
			    break;
			case CONTEXT_OPT_HEADER:
			    pGp->Pgm.Shift();
			    /* parse the string */
			    tmp_string = pGp->TryToGetString();
			    if(!tmp_string) {
				    pGp->IntErrorCurToken("String containing header information expected");
				    // only touch the options if the string is OK 
			    }
			    else {
				    // remove the old header if any 
				    if(CONTEXT_params.header) {
					    SAlloc::F(CONTEXT_params.header);
					    CONTEXT_params.header = NULL;
				    }
				    // and set the new one if nonempty; empty header will be treated as 'noheader' 
				    if(strlen(tmp_string) > 0) {
					    CONTEXT_params.header = tmp_string;
				    }
				    else {
					    SAlloc::F(tmp_string);
				    }
			    }
			    break;
			case CONTEXT_OPT_NOHEADER:
			    pGp->Pgm.Shift();
			    /* delete the header if it exists */
			    if(CONTEXT_params.header) {
				    SAlloc::F(CONTEXT_params.header);
				    CONTEXT_params.header = NULL;
			    }
			    break;
			case CONTEXT_OPT_COLOR:
			    pGp->Pgm.Shift();
			    CONTEXT_params.color = TRUE;
			    /* just mimic other terminals; no idea what it does;
			       at the moment monochrome is not fully implemented either */
			    term->flags &= ~TERM_MONOCHROME;
			    break;
			case CONTEXT_OPT_MONOCHROME:
			    pGp->Pgm.Shift();
			    CONTEXT_params.color = FALSE;
			    term->flags |= TERM_MONOCHROME;
			    break;
			case CONTEXT_OPT_DASHED:
			    pGp->Pgm.Shift();
			    CONTEXT_params.dashed = TRUE;
			    break;
			case CONTEXT_OPT_SOLID:
			    pGp->Pgm.Shift();
			    CONTEXT_params.dashed = FALSE;
			    break;
			case CONTEXT_OPT_LINEJOIN_MITERED:
			    pGp->Pgm.Shift();
			    CONTEXT_params.linejoin = LINEJOIN_MITER;
			    break;
			case CONTEXT_OPT_LINEJOIN_ROUNDED:
			    pGp->Pgm.Shift();
			    CONTEXT_params.linejoin = LINEJOIN_ROUND;
			    break;
			case CONTEXT_OPT_LINEJOIN_BEVELED:
			    pGp->Pgm.Shift();
			    CONTEXT_params.linejoin = LINEJOIN_BEVEL;
			    break;
			case CONTEXT_OPT_LINECAP_BUTT:
			    pGp->Pgm.Shift();
			    CONTEXT_params.linecap = LINECAP_BUTT;
			    break;
			case CONTEXT_OPT_LINECAP_ROUNDED:
			    pGp->Pgm.Shift();
			    CONTEXT_params.linecap = LINECAP_ROUND;
			    break;
			case CONTEXT_OPT_LINECAP_SQUARED:
			    pGp->Pgm.Shift();
			    CONTEXT_params.linecap = LINECAP_SQUARE;
			    break;
			case CONTEXT_OPT_SCALE_DASHLENGTH:
			    pGp->Pgm.Shift();
			    CONTEXT_params.scale_dashlength = pGp->RealExpression();
			    break;
			case CONTEXT_OPT_SCALE_LINEWIDTH:
			    pGp->Pgm.Shift();
			    CONTEXT_params.scale_linewidth = pGp->RealExpression();
			    break;
			case CONTEXT_OPT_SCALE_TEXT:
			    pGp->Pgm.Shift();
			    CONTEXT_params.scale_text = pGp->RealExpression();
			    break;
			case CONTEXT_OPT_DEFAULTFONT:
			    pGp->Pgm.Shift();
			    /* CONTEXT_params.font should be an empty string */
			    CONTEXT_params.font[0] = 0;
			    /* default fontsize is 12pt */
			    CONTEXT_params.fontsize = CONTEXT_FONTSIZE;
			    break;
			case CONTEXT_OPT_POINTS_WITH_METAPOST:
			    pGp->Pgm.Shift();
			    CONTEXT_params.points = CONTEXT_POINTS_WITH_METAPOST;
			    break;
			case CONTEXT_OPT_POINTS_WITH_TEX:
			    pGp->Pgm.Shift();
			    CONTEXT_params.points = CONTEXT_POINTS_WITH_TEX;
			    break;
			case CONTEXT_OPT_IMAGES_INLINE:
			    pGp->Pgm.Shift();
			    CONTEXT_params.images = CONTEXT_IMAGES_INLINE;
			    break;
			case CONTEXT_OPT_IMAGES_EXTERNAL:
#ifdef WRITE_PNG_IMAGE
			    pGp->Pgm.Shift();
			    CONTEXT_params.images = CONTEXT_IMAGES_EXTERNAL;
#else
			    pGp->IntWarnCurToken("Gnuplot was built without support for PNG images. You cannot use this option unless you rebuild gnuplot.");
#endif
			    break;
			/*
			 * The preferred way to set the font is to set it in a document itself,
			 * labels in gnuplot in graphs will then inherit that font.
			 *
			 * However, it is important to tell gnuplot which size is going to be used,
			 * so that it can estimate size of text labels.
			 *
			 * Whenever you specify
			 *     set term context font "fontname,14"
			 *
			 * there are two possibilities:
			 * - if STANDALONE mode is on, then the whole string will be used as
			 *   \setupbodyfont[fontname,14pt]
			 *   somewhere on top of the document (you still have to make sure that you
			 *   included the proper typescript, so that "fontname" will be recognised
			 * - in the other case (INPUT mode) only the font size will be used
			 *   internally to estimate sizes of labels, but the font name
			 *   won't be written anywhere
			 */
			case CONTEXT_OPT_FONT: {
			    double tmp_fontsize;
			    char tmp_font[MAX_ID_LEN+1] = "";
			    pGp->Pgm.Shift();
			    if((tmp_string = pGp->TryToGetString()) && tmp_string) {
				    CONTEXT_fontstring_parse(tmp_string, tmp_font, MAX_ID_LEN+1, &tmp_fontsize);
				    /* copies font name to parameters */
				    safe_strncpy(CONTEXT_params.font, tmp_font, sizeof(CONTEXT_params.font));
				    tmp_font[MAX_ID_LEN] = NUL;
				    SAlloc::F(tmp_string);
				    /* save font size:
				     *
				     * - if size > 0, copy
				     * - if size < 0, fix the size to default value (12pt)
				     * - if size = 0, ignore
				     */
				    if(tmp_fontsize > 0)
					    CONTEXT_params.fontsize = tmp_fontsize;
				    else if(tmp_fontsize < 0)
					    CONTEXT_params.fontsize = CONTEXT_FONTSIZE;
			    }
			    break;
		    }
			case CONTEXT_OPT_OTHER:
			default:
			    /* error */
			    pGp->IntErrorCurToken("extraneous argument in set terminal %s", term->name);
			    break;
		}
	}
	CONTEXT_fontsize = CONTEXT_params.fontsize; // current font size in pt (to be used in CONTEXT_adjust_dimensions) 
	CONTEXT_adjust_dimensions(); // sets term->MaxX, ymax, vchar, hchar 
	snprintf(term_options, sizeof(term_options),
	    "size %g%s,%g%s %s %s %s",
	    CONTEXT_params.xsize,
	    (CONTEXT_params.unit == INCHES) ? "in" : "cm",
	    CONTEXT_params.ysize,
	    (CONTEXT_params.unit == INCHES) ? "in" : "cm",
	    CONTEXT_params.standalone ? "standalone" : "input",
	    CONTEXT_params.timestamp ? "timestamp" : "notimestamp",
	    CONTEXT_params.header == NULL ? "noheader \\\n   " : "\\\n   header ");

	if(CONTEXT_params.header != NULL) {
		strncat(term_options, "\"",                  sizeof(term_options)-strlen(term_options)-1);
		strncat(term_options, CONTEXT_params.header, sizeof(term_options)-strlen(term_options)-1);
		strncat(term_options, "\" \\\n   ",          sizeof(term_options)-strlen(term_options)-1);
	}
	strncat(term_options, CONTEXT_params.color ? "color " : "monochrome ", sizeof(term_options)-strlen(term_options)-1);
	switch(CONTEXT_params.linejoin) {
		case LINEJOIN_MITER:
		    strncat(term_options, "mitered ",
			sizeof(term_options)-strlen(term_options)-1);
		    break;
		case LINEJOIN_ROUND:
		    strncat(term_options, "rounded ",
			sizeof(term_options)-strlen(term_options)-1);
		    break;
		case LINEJOIN_BEVEL:
		    strncat(term_options, "beveled ",
			sizeof(term_options)-strlen(term_options)-1);
		    break;
	}
	switch(CONTEXT_params.linecap) {
		case LINECAP_BUTT:
		    strncat(term_options, "butt",
			sizeof(term_options)-strlen(term_options)-1);
		    break;
		case LINECAP_ROUND:
		    strncat(term_options, "round",
			sizeof(term_options)-strlen(term_options)-1);
		    break;
		case LINECAP_SQUARE:
		    strncat(term_options, "squared",
			sizeof(term_options)-strlen(term_options)-1);
		    break;
	}

	snprintf(tmp_term_options, sizeof(tmp_term_options),
	    " %s dashlength %g linewidth %g fontscale %g \\\n   ",
	    CONTEXT_params.dashed ? "dashed" : "solid",
	    CONTEXT_params.scale_dashlength,
	    CONTEXT_params.scale_linewidth,
	    CONTEXT_params.scale_text
	    );
	strncat(term_options, tmp_term_options, sizeof(term_options)-strlen(term_options)-1);
	switch(CONTEXT_params.points) {
		case CONTEXT_POINTS_WITH_TEX:
		    strncat(term_options, "texpoints ", sizeof(term_options)-strlen(term_options)-1);
		    break;
		case CONTEXT_POINTS_WITH_METAPOST:
		    strncat(term_options, "mppoints ", sizeof(term_options)-strlen(term_options)-1);
		    break;
	}
#ifdef WRITE_PNG_IMAGE
	switch(CONTEXT_params.images) {
		case CONTEXT_IMAGES_INLINE:
		    strncat(term_options, "inlineimages ", sizeof(term_options)-strlen(term_options)-1);
		    break;
		case CONTEXT_IMAGES_EXTERNAL:
		    strncat(term_options, "externalimages ", sizeof(term_options)-strlen(term_options)-1);
		    break;
	}
#endif
	snprintf(tmp_term_options, sizeof(tmp_term_options), "font \"%s,%g\"", CONTEXT_params.font, CONTEXT_params.fontsize);
	strncat(term_options, tmp_term_options, sizeof(term_options) - strlen(term_options)-1);
}

/* **************
 * CONTEXT_init *
 * **************
 *
 * Starts a new file.
 *
 * XXX: "set term context" multiple times will only include those graphics
 *      that were create before issuing a new "set term context options"
 *      this should be fixed in the core by reopening the file
 *      (removing previously written content).
 * - PDF & binary terminals start a new file
 * - PS & TeX-based terminals continue
 */
TERM_PUBLIC void CONTEXT_init(termentry * pThis)
{
	time_t now;
	char timebuffer[100];
#ifdef WRITE_PNG_IMAGE
	int i;
#endif
	time(&now);
	CONTEXT_posx = CONTEXT_posy = 0;
	CONTEXT_path_count = 0;
	CONTEXT_path_is_dot = 0;
	CONTEXT_counter = 0;
	/* setup bitmap images */
	CONTEXT_image_counter = 0;
	/* the default is to use inline images */
	CONTEXT_images = CONTEXT_IMAGES_INLINE;
	/* only if external images are both requested and supported, we switch to them (double paranoia) */
	/* delete the stored filename first */
	if(CONTEXT_image_filename) {
		SAlloc::F(CONTEXT_image_filename);
		CONTEXT_image_filename = NULL;
		CONTEXT_image_filename_length = 0;
		CONTEXT_image_filename_start  = 0;
	}
	;
#ifdef WRITE_PNG_IMAGE
	if(CONTEXT_params.images == CONTEXT_IMAGES_EXTERNAL) {
		CONTEXT_images = CONTEXT_IMAGES_EXTERNAL;

		/* but only if 'set output' was set because we use that string as base for image names */
		if(outstr) {
			CONTEXT_image_filename_length = strlen(outstr);
			CONTEXT_image_filename_start  = strlen(outstr) - strlen(gp_basename(outstr));
			/* we will cut off the last .tex ending if present */
			/* find the last dot if present */
			for(i = CONTEXT_image_filename_length - 1; i >= 0 && outstr[i] != '.'; i--);
			if(outstr[i] == '.')
				CONTEXT_image_filename_length = i;
			/* it would also be very nice to do some sanity checks on filenames */

			/* <name>.xx.png; must be at least 7 characters long */
			CONTEXT_image_filename = (char*)gp_alloc(CONTEXT_image_filename_length + 10, "ConTeXt image filename");
			strncpy(CONTEXT_image_filename, outstr, CONTEXT_image_filename_length);
			CONTEXT_image_filename[CONTEXT_image_filename_length] = 0;
		}
		else {
			CONTEXT_image_filename_length = strlen("gp_image");
			CONTEXT_image_filename_start  = 0;
			/* <name>.xx.png; must be at least 7 characters long */
			CONTEXT_image_filename = (char*)gp_alloc(CONTEXT_image_filename_length + 10, "ConTeXt image filename");
			strncpy(CONTEXT_image_filename, "gp_image", CONTEXT_image_filename_length);
			CONTEXT_image_filename[CONTEXT_image_filename_length] = 0;
		}
	}
#endif

	fprintf(gpoutfile, "%% Written by ConTeXt terminal for GNUPLOT");
	if(CONTEXT_params.timestamp) {
		if(strftime(timebuffer, 100, "%Y-%m-%d %H:%M %Z", localtime(&now)) != 0)
			fprintf(gpoutfile, " on: %s", timebuffer);
	}
	fprintf(gpoutfile, "\n");
	fprintf(gpoutfile, "%% GNUPLOT version: %s.%s, terminal version: %s.%s (%s)\n",
	    gnuplot_version, gnuplot_patchlevel, CONTEXT_term_version, CONTEXT_term_patch, CONTEXT_term_date);
	fprintf(gpoutfile, "%% See also http://wiki.contextgarden.net/Gnuplot\n%%\n");

	/* place the header first if this is a standalone graphic */
	if(CONTEXT_params.standalone) {
		/* If encoding is explicitly set to UTF-8 by gnuplot, use that setting.
		 * \enableregime only makes a difference for pdfTeX; in LuaTeX and XeTeX UTF-8 is already default,
		 * so this line will be ignored.
		 *
		 * There is no extra support for other encodings on purpose:
		 * - ConTeXt doesn't support all encodings supported by Gnuplot.
		 * - In LuaTeX and XeTeX one should not use any other encoding anyway.
		 * - pdfTeX users are free to use "header '\enableregime[...]'" */
		switch(encoding) {
			case S_ENC_UTF8:
			    fputs("\\enableregime\n   [utf-8]\n", gpoutfile);
			    break;
			default:
			    /* do nothing */
			    break;
		}
		/* load the gnuplot module */
		fputs("\\usemodule\n   [gnuplot]\n", gpoutfile);
		/* enable or disable color (the only place where "color" is indeed used so far) */
		fprintf(gpoutfile, "\\setupcolors\n   [state=%s]\n", CONTEXT_params.color ? "start" : "stop");
		/* additional user-provided header information (if available) */
		if(CONTEXT_params.header)
			fprintf(gpoutfile, "%s\n", CONTEXT_params.header);
		/* for some reason setting \bodyfontenvironment is needed,
		 * otherwise \switchtobodyfont[name] doesn't work OK */
		if(!(CONTEXT_params.fontsize == CONTEXT_FONTSIZE))
			fprintf(gpoutfile, "\\definebodyfontenvironment\n   [%gpt]\n",
			    CONTEXT_params.fontsize);
		/* set the proper font: \setupbodyfont[{fontname,}fontsize sizeunit] */
		fprintf(gpoutfile, "\\setupbodyfont\n   [%s%s%gpt]\n",
		    CONTEXT_params.font,
		    /* write a comma only if the last string was non-empty */
		    ((strlen(CONTEXT_params.font)>0) ? "," : ""),
		    CONTEXT_params.fontsize);

		/*---------*
		* options *
		*---------*/
		fprintf(gpoutfile, "\\setupGNUPLOTterminal\n   [context]\n   [");

		/* color (gp_use_color): yes/no (true/false)
		 * default: yes
		 * - doesn't do anything useful yet;
		 *   and besides that, it's already set in \setupcolors[state=start] */
		/* fprintf(gpoutfile, "    color=%s, %% *yes* | no\n", CONTEXT_params.color ? "yes" : "no"); */

		/* linejoin: mitered/rounded/beveled
		 * default: mitered */
		fprintf(gpoutfile, "linejoin=");
		switch(CONTEXT_params.linejoin) {
			case LINEJOIN_MITER: fprintf(gpoutfile, "mitered"); break;
			case LINEJOIN_ROUND: fprintf(gpoutfile, "rounded"); break;
			case LINEJOIN_BEVEL: fprintf(gpoutfile, "beveled"); break;
		}
		fprintf(gpoutfile, ", %% *mitered* | rounded | beveled\n");

		/* linecap: butt/rounded/squared
		 * default: butt */
		fprintf(gpoutfile, "    linecap=");
		switch(CONTEXT_params.linecap) {
			case LINECAP_BUTT: fprintf(gpoutfile, "butt"); break;
			case LINECAP_ROUND: fprintf(gpoutfile, "rounded"); break;
			case LINECAP_SQUARE: fprintf(gpoutfile, "squared"); break;
		}
		fprintf(gpoutfile, ", %% *butt* | rounded | squared\n");

		/* dashed (gp_use_dashed): yes/no (true/false)
		 * default: yes */
		fprintf(gpoutfile, "    dashed=%s, %% *yes* | no\n", CONTEXT_params.dashed ? "yes" : "no");
		/* dashlength (gp_scale_dashlength): 1.0 */
		fprintf(gpoutfile, "    dashlength=%g, %% scaling factor for dash lengths\n", CONTEXT_params.scale_dashlength);

		/* linewidth (gp_scale_linewidth): 1.0 */
		fprintf(gpoutfile, "    linewidth=%g, %% scaling factor for line widths (1.0 means 0.5bp)\n",
		    CONTEXT_params.scale_linewidth);

		/* fontscale (gp_scale_text): 1.0 */
		/* written out just for reference - it's commented out since it needs to be part of graphic
		   and affects estimation of label sizes */
		fprintf(gpoutfile, "    %%fontscale=%g, %% scaling factor for text labels\n", CONTEXT_params.scale_text);

		/* points (gp_points_with): metapost/tex (gp_points_with_metapost/gp_points_with_tex)
		 * default: metapost */
		fprintf(gpoutfile, "    points=%s, %% *metapost* | tex (Should points be drawn with MetaPost or TeX?)\n",
		    CONTEXT_params.points == CONTEXT_POINTS_WITH_METAPOST ? "metapost" : "tex");

		/* images
		 * default: inline */
		fprintf(gpoutfile,
		    "    images=%s] %% *inline* | external (inline only works in MKIV, external requires png support in gnuplot)\n",
		    CONTEXT_images == CONTEXT_IMAGES_INLINE ? "inline" : "external");

		/*----------------*
		* end of options *
		*----------------*/

		fputs("\n\\starttext\n\n", gpoutfile);
	}
	else {
		/* Sorry, nothing! In non-standalone graphic, parameters make no sense.
		 * Setup everything in the file which includes such a graphic instead.
		 */
	}
}

/* ***************
 * CONTEXT_reset *
 * ***************
 *
 * finish writing the file
 */
TERM_PUBLIC void CONTEXT_reset()
{
	/* we only have to end the document if this is a stand-alone graphic */
	if(CONTEXT_params.standalone) {
		fputs("\\stoptext\n", gpoutfile);
	}
	else {
		/* This means that any subsequent plots to the same file will be ignored.
		 * I don't like that - gnuplot should delete the old contents instead,
		 * just as it does in case of PNG or PDF -
		 * but it will be at least consistent with standalone graphics that way
		 */
		fputs("\\endinput\n", gpoutfile);
	}
	/* deallocate image name if present */
	if(CONTEXT_image_filename) {
		SAlloc::F(CONTEXT_image_filename);
		CONTEXT_image_filename = NULL;
		CONTEXT_image_filename_length = 0;
		CONTEXT_image_filename_start  = 0;
	}
	;
}

/* **************
 * CONTEXT_text *
 * **************
 *
 * Ends the current graphic.
 */
TERM_PUBLIC void CONTEXT_text()
{
	/* close and draw the current path first */
	if(CONTEXT_path_count > 0)
		CONTEXT_endpath();

	fprintf(gpoutfile, "setbounds currentpicture to unitsquare xyscaled (w,h);\n");

	/* standalone graphic is a whole-page graphic */
	if(CONTEXT_params.standalone) {
		fputs("\\stopGNUPLOTpage\n", gpoutfile);
		/* otherwise we define a MPgraphic to be included later */
	}
	else {
		fputs("\\stopGNUPLOTgraphic\n", gpoutfile);
	}
}

/* ******************
 * CONTEXT_graphics *
 * ******************
 *
 * Starts a new graphic.
 */
TERM_PUBLIC void CONTEXT_graphics()
{
	/* standalone graphic is a whole-page graphic */
	if(CONTEXT_params.standalone) {
		fprintf(gpoutfile, "\\startGNUPLOTpage %% Graphic Nr. %d\n", ++CONTEXT_counter);
		/* otherwise we define a MPgraphic to be included later */
	}
	else {
		/* the first parameter holds the graphic number */
		fprintf(gpoutfile, "\\startGNUPLOTgraphic[%d]\n", ++CONTEXT_counter);
	}

	fprintf(gpoutfile, "string gnuplotversion; gnuplotversion := \"%s\";\n", gnuplot_version);
	fprintf(gpoutfile, "string termversion;    termversion    := \"%s\";\n", CONTEXT_term_version);
	/*
	 * MetaPost can only handle numbers up to 4096. Too high resolution
	 * would thus result in number overflow, that's why we scale down all the
	 * integers from gnuplot by 1000 and multiply those numbers later by
	 * appropriate scaling factor 'a' to get the proper dimensions.
	 */
	fprintf(gpoutfile, "%% scaling factor, width and height of the figure\na := 1cm; w := %.3fa; h := %.3fa; %% (%g%s, %g%s)\n",
	    CONTEXT_params.xsize * ((CONTEXT_params.unit == INCHES) ? 2.54 : 1),     /* cm */
	    CONTEXT_params.ysize * ((CONTEXT_params.unit == INCHES) ? 2.54 : 1),     /* cm */
	    CONTEXT_params.xsize, (CONTEXT_params.unit == INCHES) ? "in" : "cm",
	    CONTEXT_params.ysize, (CONTEXT_params.unit == INCHES) ? "in" : "cm");
	/* TODO: the following if-else could be slightly nicer */
	if(CONTEXT_images == CONTEXT_IMAGES_INLINE) {
		fprintf(gpoutfile, "%% temporary variable for storing the path and images\nsave p, img, ima; path p; string img, ima;\n");
	}
	else {
		fprintf(gpoutfile, "%% temporary variable for storing the path\nsave p; path p;\n");
	}
	fprintf(gpoutfile, "%% -------------------------\n");
	fprintf(gpoutfile, "%% Different initialisations\n");
	fprintf(gpoutfile, "%% -------------------------\n");
	fprintf(gpoutfile, "%% for additional user-defined settings\ngp_setup_before;\n");
	/* needed (depends on terminal settings & needs to be passed) */
	fprintf(gpoutfile, "%% text scaling factor for the whole figure\n");
	fprintf(gpoutfile, "gp_scale_text := %g;\n", CONTEXT_params.scale_text);
	fprintf(gpoutfile, "%% pointsize scaling factor\n");
	fprintf(gpoutfile, "gp_set_pointsize(%g);\n", CONTEXT_old_pointsize);
	/* needed (provided by terminal) */
	fprintf(gpoutfile, "%% linewidth scaling factor for individual lines\n");
	fprintf(gpoutfile, "gp_set_linewidth(%g);\n", CONTEXT_old_linewidth);
	fprintf(gpoutfile, "%% for additional user-defined settings\ngp_setup_after;\n");
	fprintf(gpoutfile, "%% -------------------------\n");

	/* since palette is initialized only once, subsequent plots wouldn't see it
	 * unless we write it on the top of relevant plots explicitly */
	if(is_plot_with_palette()) {
		CONTEXT_write_palette(CONTEXT_old_palette);
	}

	/* needed, otherwise the first linetype(-2) would be ignored */
	CONTEXT_old_linetype = -3;
	/* different initializations - not really needed, but they cannot hurt */
	CONTEXT_posx = CONTEXT_posy = 0;
	CONTEXT_path_count = 0;
	CONTEXT_path_is_dot = 0;
}

/* ---------------
 * CONTEXT_endpath
 * ---------------
 *
 * Closes and strokes (draws) the current path.
 *
 * It the path ends where it started, it ends it with --cycle (we get cyclic path),
 * otherwise just with a semicolon.
 */
static void CONTEXT_endpath()
{
	/* if we have a dot, draw only the dot */
	if(CONTEXT_path_is_dot) {
		fprintf(gpoutfile, "gp_dot(%.3fa,%.3fa);\n", 0.001*CONTEXT_posx, 0.001*CONTEXT_posy);
		CONTEXT_path_is_dot = 0;

		/* cyclic path, so let's end it with "--cycle" */
	}
	else if((CONTEXT_posx == CONTEXT_path_start_x) && (CONTEXT_posy == CONTEXT_path_start_y)) {
		fputs("--cycle;\ngp_draw(p);\n", gpoutfile);

		/* regular non-cyclic path */
	}
	else {
		fprintf(gpoutfile, "--(%.3fa,%.3fa);\ngp_draw(p);\n", 0.001*CONTEXT_posx, 0.001*CONTEXT_posy);
	}

	/* we're not inside path any more */
	CONTEXT_path_count = 0;
}

/* **************
 * CONTEXT_move *
 * **************
 *
 * Remembers the new location for a path.
 *
 * If it doesn't differ from the current location, this is simply ignored.
 * If we're still in the middle of path construction and location differs,
 * it strokes the old path.
 *
 */
TERM_PUBLIC void CONTEXT_move(uint x, uint y)
{
	/* we seem to be there already */
	if((x == CONTEXT_posx) && (y == CONTEXT_posy))
		return;

	/* close and draw the current path before the move */
	if(CONTEXT_path_count > 0)
		CONTEXT_endpath();

	CONTEXT_posx = x;
	CONTEXT_posy = y;
}

/* -----------------
 * CONTEXT_startpath
 * -----------------
 *
 * Starts ([but not yet] drawing) a new path.
 */
static void CONTEXT_startpath()
{
	CONTEXT_path_start_x = CONTEXT_posx;
	CONTEXT_path_start_y = CONTEXT_posy;
	CONTEXT_path_count = 2;

	fprintf(gpoutfile, "p := (%.3fa,%.3fa)", 0.001*CONTEXT_posx, 0.001*CONTEXT_posy);
}

/* ****************
 * CONTEXT_vector *
 * ****************
 *
 * Prolongs the current path for an additional line (from the last point to (x,y))
 * unless it has a zero-length.
 *
 * Points have to be treated as a special case, since gnuplot sometimes tries
 * to draw them as moveto(a,b)--lineto(a,b) - without the proper linecap,
 * that wouldn't draw anything in MetaPost or PS.
 * (I hope that I got that part right, but I'm not completely sure.)
 */
TERM_PUBLIC void CONTEXT_vector(uint x, uint y)
{
	/* this is zero-length line (or a dot)
	 *
	 * stupid background, has to handle
	 * - move(0,0), vector(0,0), whatever: draw a dot
	 * - move(0,0), vector(0,0), vector (1,1): draw a line
	 */
	if((x == CONTEXT_posx) && (y==CONTEXT_posy)) {
		/* as long as this is still a dot candidate, mark it so
		 * however - further vector() commands may set this back to 0 */
		if(CONTEXT_path_count == 0)
			CONTEXT_path_is_dot = 1;
		/* if some path was already drawn up to this place, ignore (it's not a dot) */
		return;
	}

	/* start the path if none is started yet */
	if(CONTEXT_path_count == 0) {
		/* the path is not a dot */
		CONTEXT_path_is_dot = 0;
		CONTEXT_startpath();
	}
	else {
		/* or prevent too long lines if you're in the middle of a path */
		if((CONTEXT_path_count % CONTEXT_LINEMAX) == 2) {
			fputs("\n  ", gpoutfile);
			CONTEXT_path_count = 2;
		}
		/* and output the previous point */
		fprintf(gpoutfile, "--(%.3fa,%.3fa)", 0.001*CONTEXT_posx, 0.001*CONTEXT_posy);
	}

	CONTEXT_posx = x;
	CONTEXT_posy = y;
	CONTEXT_path_count++;
}

/* ******************
 * CONTEXT_linetype *
 * ******************
 *
 * - If only color has been changed (recently), but not the linetype,
 *   it resets only the color.
 * - If linetype was changed, it sets the new linetype
 */
TERM_PUBLIC void CONTEXT_linetype(int lt)
{
	// reset the color in case it has been changed in CONTEXT_set_color() 
	if(CONTEXT_old_linetype != lt || CONTEXT_color_changed) {
		// close and draw the current path first 
		if(CONTEXT_path_count > 0)
			CONTEXT_endpath();
		fprintf(gpoutfile, "gp_set_linetype(%d);\n", lt);
		CONTEXT_old_linetype = lt;
		CONTEXT_color_changed = FALSE;
	}
}

/* ******************
 * CONTEXT_put_text *
 * ******************
 *
 * Places text labels.
 */
TERM_PUBLIC void CONTEXT_put_text(uint x, uint y, const char str[])
{
	const char * s; /* pointer to string */
	int alignment = 0;
	char alignments[3][10] = {"center", "left", "right"};
	// ignore empty strings 
	if(!isempty(str)) {
		// close and draw the current path first 
		if(CONTEXT_path_count > 0)
			CONTEXT_endpath();
		// see the values of "alignments" above 
		switch(CONTEXT_justify) {
			case CENTRE: alignment = 0; break;
			case LEFT: alignment = 1; break;
			case RIGHT: alignment = 2; break;
		}
		/* remove whitespace at the beginning of string
		 * (usually a problem in positive numbers)
		 * They interfere with positioning: whitespace at the beginning takes
		 * some place although it's invisible, so left-aligned and centered labels
		 * are positioned incorrectly.
		 * Example:
		 *   -1 0 1 2: centered labels on x axis
		 * if we had a space in front, positive numbers would be "centered" in a weird way */
		for(s = str; s[0] == ' '; s++);
		// label position 
		fprintf(gpoutfile, "gp_put_text((%.3fa, %.3fa), ", 0.001*x, 0.001*y);
		// angle of rotation - optional and needed only if it's different from 0 
		if(CONTEXT_ang != 0)
			fprintf(gpoutfile, "angle(%d), ", CONTEXT_ang);
		// alignment - "center" is optional, but we'll add it anyway 
		fprintf(gpoutfile, "align(%s), \\sometxt[gp]", alignments[alignment]);
		// fontface/fontsize - optional second argument 
		if(CONTEXT_font_explicit[0] != 0)
			fprintf(gpoutfile, "[%s]", CONTEXT_font_explicit);
		fprintf(gpoutfile, "{%s});\n", s); // finally the text label itself 
	}
}

/* ********************
 * CONTEXT_text_angle *
 * ********************
 *
 * Saves text angle to be used for text labels.
 */
TERM_PUBLIC int CONTEXT_text_angle(int ang)
{
	CONTEXT_ang = ang;
	return TRUE;
}

/* **********************
 * CONTEXT_justify_text *
 * **********************
 *
 * Saves horizontal text justification (left/middle/right) to be used for text labels.
 */
TERM_PUBLIC int CONTEXT_justify_text(enum JUSTIFY mode)
{
	CONTEXT_justify = mode;
	return TRUE;
}

/* ***************
 * CONTEXT_point *
 * ***************
 *
 * There are two/three possible ways of drawing points in ConTeXt:
 * - let gnuplot draw the points with standard move/vector commands
 *   (points_default): not enabled
 * - use the points predefined in mp-gnuplot.mp (drawn with metapost)
 *   which can easily be redefined
 *   (points_with_metapost)
 * - use symbols from a symbol set predefined in m-gnuplot.tex (drawn with TeX)
 *   (points_with_tex)
 *
 * At first this routine took care of that, but now it's up to the high-level
 * user interface to switch between the last two options.
 */
TERM_PUBLIC void CONTEXT_point(uint x, uint y, int number)
{
	/* finish the current line first before the move */
	if(CONTEXT_path_count > 0)
		CONTEXT_endpath();

	fprintf(gpoutfile, "gp_point(%.3fa,%.3fa,%d);\n", 0.001*x, 0.001*y, number);
}

/* ***************
 * CONTEXT_arrow *
 * ***************
 * ConTeXt could draw nice arrows on its own and in such a way that user could
 * simply redefine arrow heads in the template file.
 *
 * This function is left here just in case that anyoune would find it useful
 * to define his own arrow-drawing commands.
 *
 * Currently it just calls the default gnuplot function for drawing arrows.
 */
TERM_PUBLIC void CONTEXT_arrow(uint sx, uint sy, uint ex, uint ey, int head)
{
	GnuPlot::DoArrow(sx, sy, ex, ey, head);
}

/* ------------------------
 * CONTEXT_fontstring_parse
 * ------------------------
 *
 * Parses from_string, which is divided by commas (,)
 * and copies the whole content to to_string except for the part
 * which starts with a numeric value.
 *
 * That value is interpreted separately and saved to fontsize.
 *
 * to_size is the size of to_string which may not be exceeded while copying the string
 */
static void CONTEXT_fontstring_parse(char * from_string, char * to_string, int to_size, double * fontsize)
{
	double tmp_fontsize = 0.0;
	size_t n;
	char * comma = strrchr(from_string, ',');
	if(comma) {
		sscanf(comma + 1, "%lf", &tmp_fontsize);
		n = comma - from_string;
	}
	else {
		n = strlen(from_string);
	}
	*fontsize = tmp_fontsize;
	if(static_cast<int>(n) >= to_size)
		n = to_size - 1;
	memcpy(to_string, from_string, n);
	to_string[n] = NUL;
}
/* -------------------------
 * CONTEXT_adjust_dimensions
 * -------------------------
 *
 * - sets the ChrV and ChrH based on current font size (approximation only)
 *   using CONTEXT_fontsize in points (from CONTEXT_params.fontsize) and CONTEXT_params.scale_text
 * - sets xmax and ymax based on CONTEXT_params.xsize and CONTEXT_params.ysize
 */
static void CONTEXT_adjust_dimensions()
{
	/* sets vertical dimension of characters based on current fontsize in pt */
	term->ChrV = (uint)((double)CONTEXT_DPI *
	    CONTEXT_fontsize / 72.27 * CONTEXT_params.scale_text + 0.5);
	/* based on proportions of LM digits at 12pt */
	term->ChrH = (uint)(CONTEXT_LM_H_TO_V_RATIO * term->ChrV + 0.5);

	/* we might want to fix CONTEXT_DPI in case that the figure becomes too big */
	if(CONTEXT_params.unit == INCHES) {
		term->MaxX = (uint)((double)CONTEXT_DPI  * CONTEXT_params.xsize + 0.5);
		term->MaxY = (uint)((double)CONTEXT_DPI  * CONTEXT_params.ysize + 0.5);
	}
	else {
		term->MaxX = (uint)((double)CONTEXT_DPCM * CONTEXT_params.xsize + 0.5);
		term->MaxY = (uint)((double)CONTEXT_DPCM * CONTEXT_params.ysize + 0.5);
	}
}

/* *****************
 * CONTEXT_set_font*
 * *****************
 *
 * Official description:
 * - empty string restores the terminal's default font
 * - fonts are selected as strings "name,size",
 *   where size should be a floating point number interpreted as "pt" (point)
 *
 * It's the user's own responsibility to make sure that the proper typescripts
 * are included in the header, else the selected font won't work out-of-the-box
 *
 * The ConTeXt terminal should support things such as
 *   "iwona,ss,12"   (iwona sans serif) or
 *   ",10"           (10 points)  or
 *   "tt"            (typewriter)
 *
 * The routine saves font name to CONTEXT_font
 * and fontsize to CONTEXT_fontsize.
 *
 * The two are joined in CONTEXT_font_explicit for the usage in
 *   \sometxt[gp][fontname,fontsize]{label}
 */
TERM_PUBLIC int CONTEXT_set_font(const char * font)
{
	char tmp_fontstring[MAX_ID_LEN+1] = "";

	/* saves font name & family to CONTEXT_font */
	CONTEXT_fontstring_parse((char*)font, CONTEXT_font, sizeof(CONTEXT_font), &CONTEXT_fontsize_explicit);
	safe_strncpy(CONTEXT_font_explicit, CONTEXT_font, sizeof(CONTEXT_font_explicit));

	/* valid fontsize has been provided */
	if(CONTEXT_fontsize_explicit > 0.) {  /* XXX: if valid */
		CONTEXT_fontsize = CONTEXT_fontsize_explicit;

		snprintf(tmp_fontstring, sizeof(tmp_fontstring), ",%gpt", CONTEXT_fontsize_explicit);
		strncat(CONTEXT_font_explicit, tmp_fontstring,
		    sizeof(CONTEXT_font_explicit) - strlen(CONTEXT_font_explicit)-1);
		tmp_fontstring[MAX_ID_LEN] = NUL;

		/* no fontsize has been provided: switch back to default terminal fontsize */
	}
	else if(CONTEXT_fontsize_explicit == 0) {
		CONTEXT_fontsize = CONTEXT_params.fontsize;
	}

	/* tell to gnuplot how big the fonts in labels are */
	CONTEXT_adjust_dimensions();

	return TRUE;
}

/* *******************
 * CONTEXT_pointsize *
 * *******************
 *
 * Sets the (relative) point size for subsequent points
 *
 * The base point size is defined "somewhere else":
 * - depends on the font[size] used when "texpoints" option is on
 */
TERM_PUBLIC void CONTEXT_pointsize(double pointsize)
{
	/*
	 * my first thought was not to allow negative sizes of points,
	 * but I see no reason why one shouldn't be able to play with
	 * inverted point shapes, so finally I commented this out again
	 *
	 *   if (pointsize < 0)
	 *      pointsize = 1;
	 */

	if(CONTEXT_old_pointsize != pointsize) {
		/* close and draw the current path first */
		if(CONTEXT_path_count > 0)
			CONTEXT_endpath();
		fprintf(gpoutfile, "gp_set_pointsize(%.3f);\n", pointsize);
		CONTEXT_old_pointsize = pointsize;
	}
}

/* *****************
 * CONTEXT_fillbox *
 * *****************
 *
 * Creates the path for the rectangle and calls the CONTEXT_fill(style)
 * routine (shared with CONTEXT_filled_polygon) to actually fill that shape
 */
TERM_PUBLIC void CONTEXT_fillbox(int style, uint x1, uint y1, uint width, uint height)
{
	/* close and draw the current path first */
	if(CONTEXT_path_count > 0)
		CONTEXT_endpath();

	/* create a new path */
	fprintf(gpoutfile, "p := unitsquare xyscaled (%.3fa,%.3fa) shifted (%.3fa,%.3fa);\n", 0.001*width, 0.001*height, 0.001*x1,
	    0.001*y1);
	/* fprintf(gpoutfile, "p := gp_rect ((%.3fa,%.3fa),(%.3fa,%.3fa));\n", 0.001*x1, 0.001*y1, 0.001*width,
	   0.001*height); */

	/* fills the box according to the "style"
	 * the code went out of this routine because of undocumented behaviour
	 * that also the filled_polygon should paint with patterns ... */
	CONTEXT_fill(style);
}

/* **************
 * CONTEXT_fill *
 * **************
 *
 * Filling routine, yet another undocumented feature of Gnuplot.
 * The code was mostly cloned from the PostScript terminal,
 * so that you know whom to accuse if it doesn't do what you would expect
 * it to do. (In case of cloning errors please inform the author of this terminal.)
 *
 * The lowest 4 bits of "style" seem to represent the style of filling
 * (whether it is solid or pattern-based or whatever that might appear
 * in that place in the future).
 *
 * The rest of the bits of "style" represent either the density
 * (ranging from 0 to 100) or the number of the pattern that the polygon
 * should be filled with.
 *
 * Used by CONTEXT_fillbox(...) and CONTEXT_filled_polygon(...)
 */
TERM_PUBLIC void CONTEXT_fill(int style)
{
	int density;
	int pattern;

	/* used in FS_[TRANSPARENT_]SOLID --> fill with intensity according to filldensity
	   it extracts a percentage out of "style" */
	density = (style >> 4);
	if(density < 0)
		density = 0;
	if(density > 100)
		density = 100;

	fputs("gp_fill(p", gpoutfile);

	/* do some strange trickery */
	switch(style & 0xf) {
		case FS_DEFAULT:
		    break;

		case FS_TRANSPARENT_SOLID:
		    /* just a flag to tell the terminal that density() should be used to interpret transparency */
		    fprintf(gpoutfile, ",transparent");
		case FS_SOLID:
		    /* fill the box with density "density": if no parameter density is specified,
		       it implies 100% density by default */
		    if(density < 100)
			    fprintf(gpoutfile, ",density(%.2f)", 0.01*density);
		    break;

		case FS_TRANSPARENT_PATTERN:
		    /* just a flag that should be interpreted in metapost,
		       non-transparent patterns have to fill with background color before drawing the pattern */
		    fprintf(gpoutfile, ",transparent");
		case FS_PATTERN:
		    pattern = (style >> 4);
		    fprintf(gpoutfile, ",pattern(%d)", pattern);
		    break;

		default: /* style == 0 (FS_EMPTY) or unknown --> fill with background color */
		    fprintf(gpoutfile, ",density(0)");
	}
	/* TODO: FS_DEFAULT is missing; what should that one do? */

	/* gp_fill(p,...); */
	fputs(");\n", gpoutfile);
}

/* *******************
 * CONTEXT_linewidth *
 * *******************
 *
 * scale line width (similar to pointsize)
 * remembers the values locally (no serious need for that actually) and writes them into file
 */
TERM_PUBLIC void CONTEXT_linewidth(double linewidth)
{
	if(linewidth < 0)
		linewidth = 1.0;

	if(CONTEXT_old_linewidth != linewidth) {
		/* close and draw the current path first */
		if(CONTEXT_path_count > 0)
			CONTEXT_endpath();
		fprintf(gpoutfile, "gp_set_linewidth(%.3f);\n", linewidth);
		CONTEXT_old_linewidth = linewidth;
	}
}

/* ------------------------------
 * CONTEXT_write_palette_gradient
 * ------------------------------
 *
 * Writes the colors and positions for a gradient palette.
 */
static void CONTEXT_write_palette_gradient(gradient_struct * gradient, int cnt)
{
	int i;

	/* i-th color */
	fprintf(gpoutfile, "colors(");
	for(i = 0; i < cnt; i++) {
		if(i > 0)
			fprintf(gpoutfile, ",");
		fprintf(gpoutfile, "(%.3g,%.3g,%.3g)", gradient[i].col.r, gradient[i].col.g, gradient[i].col.b);
	}

	/* position of the i-th color */
	fprintf(gpoutfile, ");positions(");
	for(i = 0; i < cnt; i++) {
		if(i > 0)
			fprintf(gpoutfile, ",");
		fprintf(gpoutfile, "%.4g", gradient[i].pos);
	}
	fprintf(gpoutfile, ")");
}

/* ---------------------
 * CONTEXT_write_palette
 * ---------------------
 *
 */
static void CONTEXT_write_palette(t_sm_palette * palette)
{
	if(palette == NULL)
		return;

/* TODO
        // Color models: RGB, HSV, CMY, XYZ
        //
        // RGB: Red, Green, Blue
        // HSV: Hue, Saturation, Value
        // CMY
        //
        // two models that gnuplot uses, but which probably won't be supported by this terminal:
        // XYZ: three primary colors of the color model defined by the 'Commission Internationale de l'Eclairage' (CIE)
        // http://www.cs.rit.edu/~ncs/color/glossary.htm
        // http://cs.fit.edu/wds/classes/cse5255/cse5255/davis/index.html
 */
	fprintf(gpoutfile, "gp_make_palette(");
	switch(GPO.SmPltt.colorMode) {
		/* grayscale only */
		case SMPAL_COLOR_MODE_GRAY:
		    fprintf(gpoutfile, "color_mode(gray)");
		    break;
		/* one of several fixed transformations */
		case SMPAL_COLOR_MODE_RGB:
		    fprintf(gpoutfile, "color_mode(rgb);formulae(%d,%d,%d)", GPO.SmPltt.formulaR, GPO.SmPltt.formulaG, GPO.SmPltt.formulaB);
		    break;
		/* user defined transforms */
		case SMPAL_COLOR_MODE_FUNCTIONS:
		    fprintf(gpoutfile, "color_mode(functions)");
		    break;
		/* interpolated table: explicitly defined or read from file */
		case SMPAL_COLOR_MODE_GRADIENT:
		    fprintf(gpoutfile, "color_mode(gradient);");
		    CONTEXT_write_palette_gradient(palette->P_Gradient, palette->GradientNum);
		    break;
		case SMPAL_COLOR_MODE_NONE:
		    break;
		default:
		    break;
	}
	fprintf(gpoutfile, ");\n");
}

/* *********************
 * CONTEXT_make_palette*
 * *********************
 *
 * 1. if palette==NULL, then return nice/suitable
 *    maximal number of colours supported by this terminal.
 *    Returns 0 if it can make colours without palette (like
 *    postscript).
 * 2. if palette!=NULL, then allocate its own palette
 *    return value is undefined
 * 3. available: some negative values of max_colors for whatever
 *    can be useful
 */
TERM_PUBLIC int CONTEXT_make_palette(t_sm_palette * palette)
{
	if(palette == NULL)
		return 0;  /* ConTeXt can do continuous colors */

	/* save the palette */
	CONTEXT_old_palette = palette;

	return 0;
}

/*
 * most probably this one is not needed
 *
 * TERM_PUBLIC void
 * CONTEXT_previous_palette(){}
 *
 */

/* *******************
 * CONTEXT_set_color *
 * *******************
 *
 * typedef struct t_colorspec {
 *    int    type;    // TC_ DEFAULT, LT, LINESTYLE, RGB, CB, FRAC, Z
 *    int    lt;      // used for TC_LT, TC_LINESTYLE and TC_RGB
 *    double value;   // used for TC_CB and TC_FRAC
 * } t_colorspec;
 */
TERM_PUBLIC void CONTEXT_set_color(const t_colorspec * colorspec)
{
	double gray, r, g, b;
	/* ConTeXt doesn't offer full support for palettes yet
	   (I don't know how to trick metapost to accept the full palette specification)
	   so we convert the colors from palettes to RGB instead.
	   If terminal starts supporting palettes, this behaviour will change.
	 */
	rgb_color rgb1;

/* If (colorspec->type == TC_FRAC):
   Set current color according to colorspec->value, where 0 <= value <= 1.
   If using a palette, first map value to an integer i in the interval
   [0...num_colors-1], then set to the ith color in the palette.
   If (colorspec->type == TC_RGB):
   Set current color to the rgb triple given in colorspec->lt. */
	/* close and draw the current path first */
	if(CONTEXT_path_count > 0)
		CONTEXT_endpath();
	switch(colorspec->type) {
		/* TC_DEFAULT, TC_CB, TC_Z: probably unused; what about linestyle? */
		/* color equal as that of linetype in colorspec->lt */
		case TC_LT:
		    fprintf(gpoutfile, "gp_set_color(lt(%d));\n", colorspec->lt);
		    CONTEXT_color_changed = TRUE;
		    break;
		/* rgb color */
		case TC_RGB:
		    r = (double)((colorspec->lt >> 16 ) & 255) / 255.0;
		    g = (double)((colorspec->lt >> 8 ) & 255) / 255.0;
		    b = (double)(colorspec->lt & 255) / 255.0;
		    fprintf(gpoutfile, "gp_set_color(rgb(%3.2f,%3.2f,%3.2f));\n", r, g, b);
		    CONTEXT_color_changed = TRUE;
		    break;

		/* map [0:1] to gray colors or to the corresponding color from the palette */
		case TC_FRAC:
		    gray = colorspec->value;
		    /* limit negative and >1 values to [0:1] first */
		    if(gray < 0) gray = 0;
		    if(gray > 1) gray = 1;
		    /* TODO: if ConTeXt start supporting palettes, we'll uncomment the following: */
		    fprintf(gpoutfile, "%%gp_set_color(frac(%.4f));\n", gray);
		    /* but now it doesn't, so let's use the fallback instead: */
		    GPO.Rgb1MaxColorsFromGray(gray, &rgb1);
		    fprintf(gpoutfile, "gp_set_color(rgb(%3.2f,%3.2f,%3.2f));\n", rgb1.r, rgb1.g, rgb1.b);
		    CONTEXT_color_changed = TRUE;
		    break;
		default:
		    GPO.IntWarn(NO_CARET, "context.trm set_color unknown colorspec->type %i", colorspec->type);
		    break;
	}
}

/* ************************
 * CONTEXT_filled_polygon *
 * ************************
 *
 * Draws a polygon with the fill color set by set_color, and no border.
 */
TERM_PUBLIC void CONTEXT_filled_polygon(int points, gpiPoint * corners)
{
	int i;

	/* nothing to be filled if less than 3 points */
	if(points < 3)
		return;

	/* close and draw the current path first */
	if(CONTEXT_path_count > 0)
		CONTEXT_endpath();

	/* if the first point equals the last one, skip the last point; --cycle does that already
	 * this condition is probably always true in gnuplot, so the if condition may not be needed */
	if((corners[0].x == corners[points-1].x) && (corners[0].y == corners[points-1].y))
		points--;

	/* create new path with corners */
	fputs("p := ", gpoutfile);
	fprintf(gpoutfile, "(%.3fa,%.3fa)", 0.001*corners[0].x, 0.001*corners[0].y);
	for(i = 1; i < points; i++) {
		if(i % CONTEXT_LINEMAX == 0)
			fputs("\n  ", gpoutfile);
		fprintf(gpoutfile, "--(%.3fa,%.3fa)", 0.001*corners[i].x, 0.001*corners[i].y);
	}
	/* and fill it */
	fprintf(gpoutfile, "--cycle;\n");

	/* fill the polygon
	 * undocumented gnuplot behaviour, copied from PostScript terminal
	 * see comments for CONTEXT_fill for details */
	CONTEXT_fill(corners->style);
}

/* ***************
 * CONTEXT_image *
 * ***************
 *
   Plot a pixel-based image on the display device.
   'M' is the number of pixels along the y-dimension of the image and
   'N' is the number of pixels along the x-dimension of the image.  The
   coordval pointer 'image' is the pixel values normalized to the range
   [0:1].  These values should be scaled appropriately for the output
   device.  The 'image' data starts in the upper left corner and scans
   along rows finishing in the lower right corner.  If 'color_mode' is
   IC_PALETTE, the terminal is to use palette lookup to generate color
   information.  In this scenario the size of 'image' is M*N.  If
   'color_mode' is IC_RGB, successive bytes of the data structure are
   interpreted as RGB components of a single pixel.  In this scenario
   the size of 'image' is 3*M*N.  The data appears in RGB triples, i.e.,
   image[0] = R(1,1), image[1] = G(1,1), image[2] = B(1,1),
   image[3] = R(1,2), image[4] = G(1,2), ..., image[3*M*N-1] = B(M,N).
   The mode IC_RGBA is similar except that four values red, green, blue,
   alpha per pixel are passed to the terminal in the image structure.
   The 'image' is actually an "input" image in the sense that
   it must also be properly resampled for the output device.  Many output
   media, e.g., PostScript, do this work via various driver functions.
   To determine the appropriate rescaling, the 'corner' information
   should be used.  There are four entries in the gpiPoint data array.
   'corner[0]' is the upper left corner (in terms of plot location) of
   the outer edge of the image.  Similarly, 'corner[1]' is the lower
   right corner of the outer edge of the image.  (Outer edge means the
   outer extent of the corner pixels, not the middle of the corner
   pixels.)  'corner[2]' is the upper left corner of the visible part
   of the image, and 'corner[3]' is the lower right corner of the visible
   part of the image.  The information is provided in this way because
   often it is necessary to clip a portion of the outer pixels of the
   image.
 */

/* TODO: this code needs some improvements
 *
 * There are two different modes:
 *
 * 1. creating PNG image and including it
 *    - it only works if gnuplot has been compiled with cairo or gdlib libraries
 *    - there are some bugs in MKII/MKIV handling: a different syntax is required
 *      and can only be fixed in ConTeXt core; I could abstract the code a bit
 * 2. printing out a string with colors
 *    - MKIV: works, but transparency is not yet implemented
 *    - MKII: requires MetaPost > 0.750 and is not implemented at all
 *            two possible approaches: drawing rectangles & creating proper image
 *            it might require one additional level of abstraction like gp_image(...)
 */
TERM_PUBLIC void CONTEXT_image(unsigned M, unsigned N, coordval * image, gpiPoint * corner, t_imagecolor color_mode)
{
	int i, k, line_length, components_per_color;
	rgb_color color;
	bool is_clipped = FALSE;
	if((corner[2].x > corner[0].x) || (corner[0].y > corner[2].y) || (corner[1].x > corner[3].x) || (corner[3].y > corner[1].y))
		is_clipped = TRUE;
	if(CONTEXT_images == CONTEXT_IMAGES_EXTERNAL) {
#ifdef WRITE_PNG_IMAGE
		/* we reserved 10 extra bytes, so we can afford images up to 9999 */
		if(CONTEXT_image_counter < 9999)
			sprintf(CONTEXT_image_filename + CONTEXT_image_filename_length, "_%02d.png", ++CONTEXT_image_counter);
		write_png_image(M, N, image, color_mode, CONTEXT_image_filename);
		if(is_clipped)
			fprintf(gpoutfile, "draw image(\n  ");
		fprintf(gpoutfile,
		    "externalfigure \"%s\" xyscaled (%.3fa,%.3fa) shifted (%.3fa,%.3fa);\n",
		    CONTEXT_image_filename + CONTEXT_image_filename_start,
		    0.001*(corner[1].x-corner[0].x),
		    0.001*(corner[0].y-corner[1].y),
		    0.001*corner[0].x,
		    0.001*corner[1].y);
		if(is_clipped) {
			fprintf(gpoutfile, "  clip currentpicture to unitsquare xyscaled (%.3fa,%.3fa) shifted (%.3fa,%.3fa););\n",
			    0.001*(corner[3].x-corner[2].x), 0.001*(corner[2].y-corner[3].y), 0.001*corner[2].x, 0.001*corner[3].y);
		}
#endif
	}
	else {
		/* Palette colors have to be converted into RGB first */
		if(color_mode == IC_PALETTE) {
			/* write the image data */
			fprintf(gpoutfile, "img := \"%%\n");
			line_length = 0;
			for(i = 0; i < M * N; i++) {
				if(line_length++ >= 16) {
					line_length = 1;
					fprintf(gpoutfile, "%%\n");
				}
				GPO.Rgb1MaxColorsFromGray(image[i], &color);
				fprintf(gpoutfile, "%02x%02x%02x", (uchar)(255*color.r), (uchar)(255*color.g), (uchar)(255*color.b));
			}
			fprintf(gpoutfile, "\";\n");
			/* IC_RGB or IC_RGBA */
		}
		else {
			if(color_mode == IC_RGBA) {
				components_per_color = 4;
			}
			else {   /* IC_RGB */
				components_per_color = 3;
			}

			/* write the image data */
			fprintf(gpoutfile, "img := \"%%\n");
			line_length = 0;
			for(i = 0; i < M * N; i++) {
				if(line_length++ >= 16) {
					line_length = 1;
					fprintf(gpoutfile, "%%\n");
				}
				for(k = 0; k < 3; k++) {
					fprintf(gpoutfile, "%02x", (uchar)(image[i*components_per_color+k]*255));
				}
			}
			fprintf(gpoutfile, "\";\n");
			/* transparency mask */
			if(color_mode == IC_RGBA) {
				fprintf(gpoutfile, "ima := \"%%\n");
				line_length = 0;
				for(i = 0; i < M * N; i++) {
					if(line_length++ >= 3*16) {
						line_length = 1;
						fprintf(gpoutfile, "%%\n");
					}
					fprintf(gpoutfile, "%02x", (uchar)(image[i*components_per_color+3]*255));
				}
				fprintf(gpoutfile, "\";\n");
			}
		}
		/* TODO: transparency handling is not yet supported in ConTeXt */
		if(is_clipped)
			fprintf(gpoutfile, "draw image(\n  ");
		fprintf(gpoutfile, "draw bitmapimage (%u,%u,img) xyscaled (%.3fa,%.3fa) shifted (%.3fa,%.3fa);\n",
		    N, M, 0.001*(corner[1].x-corner[0].x), 0.001*(corner[0].y-corner[1].y), 0.001*corner[0].x, 0.001*corner[1].y);
		if(is_clipped) {
			fprintf(gpoutfile, "  clip currentpicture to unitsquare xyscaled (%.3fa,%.3fa) shifted (%.3fa,%.3fa););\n",
			    0.001*(corner[3].x-corner[2].x), 0.001*(corner[2].y-corner[3].y), 0.001*corner[2].x, 0.001*corner[3].y);
		}

		/* (alternative implementation) *
		   fprintf(gpoutfile, "gp_image_rgb");
		   if (color_mode == IC_RGB)
		        fprintf(gpoutfile, "_alpha");
		   fprintf(gpoutfile, "((%u,%u),(%.3fa,%.3fa),(%.3fa,%.3fa));\n",
		        N, M, 0.001*corner[0].x, 0.001*corner[1].y, 0.001*corner[1].x, 0.001*corner[0].y);
		 */
	}
}

/*
 * TODO: Implement this function for smooth shading
 *
 * you need to fix draw_color_smooth_box(MODE_SPLOT) in graph3d.c -> color.c
 *
   static void
   CONTEXT_draw_inside_color_smooth_box()
   {
   }
 */

#endif /* TERM_BODY */

#ifdef TERM_TABLE

TERM_TABLE_START(context_driver)
	"context", 
	"ConTeXt with MetaFun (for PDF documents)",
	static_cast<uint>(CONTEXT_XMAX),
	static_cast<uint>(CONTEXT_YMAX),
	static_cast<uint>(CONTEXT_VCHAR),
	static_cast<uint>(CONTEXT_HCHAR),
	static_cast<uint>(CONTEXT_VTIC),
	static_cast<uint>(CONTEXT_HTIC),
	CONTEXT_options, 
	CONTEXT_init,
	CONTEXT_reset, 
	CONTEXT_text, 
	GnuPlot::NullScale, 
	CONTEXT_graphics,
	CONTEXT_move, 
	CONTEXT_vector,
	CONTEXT_linetype, 
	CONTEXT_put_text, 
	CONTEXT_text_angle,
	CONTEXT_justify_text, 
	CONTEXT_point, 
	CONTEXT_arrow, 
	CONTEXT_set_font,
	CONTEXT_pointsize,
	/* also add TERM_CAN_CLIP once you understand what it does */
	TERM_CAN_DASH | TERM_ALPHA_CHANNEL | TERM_LINEWIDTH | TERM_FONTSCALE | TERM_IS_LATEX /* flags */,
	0 /* suspend */, 
	0 /* resume*/,
	CONTEXT_fillbox, 
	CONTEXT_linewidth,
	#ifdef USE_MOUSE
	0, 
	0, 
	0, 
	0, 
	0,         /* no mouse support */
	#endif /* USE_MOUSE */
	CONTEXT_make_palette,
	0, // XXX: CONTEXT_previous_palette: not sure if we need it at all (PS does "grestore") 
	CONTEXT_set_color,
	CONTEXT_filled_polygon,
	CONTEXT_image,
	0, 0, 0, // No enhanced text mode because this is TeX 
	0, // layer (used to signal front/back text, used in canvas, PS, epslatex, ...); XXX: figure out what this has to do 
	0 // path (Path control for end-joins of closed polygons on PostScript-like devices); TODO: implement it (CONTEXT_path) 
TERM_TABLE_END(context_driver)
#undef LAST_TERM
#define LAST_TERM context_driver

#endif /* TERM_TABLE */
#endif /* TERM_PROTO_ONLY */

#ifdef TERM_HELP
START_HELP(context)
"1 context",
"?commands set terminal context",
"?set terminal context",
"?terminal context",
"?set term context",
"?term context",
"?context",
" ConTeXt is a macro package for TeX, highly integrated with Metapost",
" (for drawing figures) and intended for creation of high-quality PDF documents.",
" The terminal outputs Metafun source, which can be edited manually,",
" but you should be able to configure most things from outside.",
"",
" For an average user of ConTeXt + gnuplot module it's recommended to refer to",
" `Using ConTeXt` rather than reading this page",
" or to read the manual of the gnuplot module for ConTeXt.",
"",
" The `context` terminal supports the following options:",
"",
" Syntax:",
"      set term context {default}",
"              {defaultsize | size <scale> | size <xsize>{in|cm}, <ysize>{in|cm}}",
"              {input | standalone}",
"              {timestamp | notimestamp}",
"              {noheader | header \"<header>\"}",
"              {color | colour | monochrome}",
"              {rounded | mitered | beveled} {round | butt | squared}",
"              {dashed | solid} {dashlength | dl <dl>}",
"              {linewidth | lw <lw>}",
"              {fontscale <fontscale>}",
"              {mppoints | texpoints}",
"              {inlineimages | externalimages}",
"              {defaultfont | font \"{<fontname>}{,<fontsize>}\"}",
"",
" In non-standalone (`input`) graphic only parameters `size` to select graphic",
" size, `fontscale` to scale all the labels for a factor <fontscale>",
" and font size, make sense, the rest is silently",
" ignored and should be configured in the .tex file which inputs the graphic.",
" It's highly recommended to set the proper fontsize if document font differs from",
" 12pt, so that gnuplot will know how much space to reserve for labels.",
"",
" `default` resets all the options to their default values.",
"",
" `defaultsize` sets the plot size to 5in,3in.",
" `size` <scale> sets the plot size to <scale> times <default value>.",
" If two arguments are given (separated with ','), the first one sets",
" the horizontal size and the second one the vertical size.",
" Size may be given without units (in which case it means relative to the default",
" value), with inches ('in') or centimeters ('cm').",
"",
" `input` (default) creates a graphic that can be included into another ConTeXt",
" document.",
" `standalone` adds some lines, so that the document might be compiled as-is.",
" You might also want to add `header` in that case.",
"",
" Use `header` for any additional settings/definitions/macros",
" that you might want to include in a standalone graphic. `noheader` is the default.",
"",
" `notimestamp` prevents printing creation time in comments",
" (if version control is used, one may prefer not to commit new version when only date changes).",
"",
" `color` to make color plots is the default, but `monochrome` doesn't do anything special yet.",
" If you have any good ideas how the behaviour should differ to suit the monochrome printers better,",
" your suggestions are welcome.",
"",
" `rounded` (default), `mitered` and `beveled` control the shape of line joins.",
" `round` (default), `butt` and `squared` control the shape of line caps.",
" See PostScript or PDF Reference Manual for explanation. For wild-behaving functions",
" and thick lines",
" it is better to use `rounded` and `round` to prevent sharp corners in line joins.",
" (Some general support for this should be added to Gnuplot, so that the same options",
" could be set for each line (style) separately).",
"",
" `dashed` (default) uses different dash patterns for different line types,",
" `solid` draws all plots with solid lines.",
"",
" `dashlength` or `dl` scales the length of the dashed-line segments by <dl>.",
" `linewidth` or `lw` scales all linewidths by <lw>.",
" (lw 1 stands for 0.5bp, which is the default line width when drawing with Metapost.)",
" `fontscale` scales text labels for factor <fontscale> relative to default document font.",
"",
" `mppoints` uses predefined point shapes, drawn in Metapost.",
" `texpoints` uses easily configurable set of symbols, defined with ConTeXt",
" in the following way:",
"      \\defineconversion[my own points][+,{\\ss x},\\mathematics{\\circ}]",
"      \\setupGNUPLOTterminal[context][points=tex,pointset=my own points]",
"",
" `inlineimages` writes binary images to a string and only works in ConTeXt MKIV.",
" `externalimages` writes PNG files to disk and also works with ConTeXt MKII.",
" Gnuplot needs to have support for PNG images built in for this to work.",
"",
" With `font` you can set font name and size in standalone graphics.",
" In non-standalone (`input`) mode only the font size is important",
" to reserve enough space for text labels.",
" The command",
"      set term context font \"myfont,ss,10\"",
" will result in",
"      \\setupbodyfont[myfont,ss,10pt]",
" If you additionally set `fontscale` to 0.8 for example,",
" then the resulting font will be 8pt big and",
"      set label ... font \"myfont,12\"",
" will come out as 9.6pt.",
"",
" It is your own responsibility to provide proper typescripts (and header),",
" otherwise switching the font will have no effect.",
" For a standard font in ConTeXt MKII (pdfTeX) you could use:",
"      set terminal context standalone header '\\usetypescript[iwona][ec]' \\",
"          font \"iwona,ss,11\"",
" Please take a look into ConTeXt documentation, wiki or mailing list (archives)",
" for any up-to-date information about font usage.",
"",
" Examples:",
"      set terminal context size 10cm, 5cm     # 10cm, 5cm",
"      set terminal context size 4in, 3in      # 4in, 3in",
" For standalone (whole-page) plots with labels in UTF-8 encoding:",
"      set terminal context standalone header '\\enableregime[utf-8]'",
"", /* TODO: LaTeX formatting */
"2 Requirements",
" You need gnuplot module for ConTeXt",
"^ <a href=\"http://ctan.org/pkg/context-gnuplot\">",
"      http://ctan.org/pkg/context-gnuplot",
"^ </a>",
" and a recent version of ConTeXt.",
" If you want to call gnuplot on-the-fly, you also need write18 enabled.",
" In most TeX distributions this can be set with shell_escape=t in texmf.cnf.",
"",
" See",
"^ <a href=\"http://wiki.contextgarden.net/Gnuplot\">",
"           http://wiki.contextgarden.net/Gnuplot",
"^ </a>",
" for details about this terminal and for more exhaustive help & examples.",
"",
"2 Calling gnuplot from ConTeXt",
" The easiest way to make plots in ConTeXt documents is",
"      \\usemodule[gnuplot]",
"      \\starttext",
"      \\title{How to draw nice plots with {\\sc gnuplot}?}",
"      \\startGNUPLOTscript[sin]",
"      set format y \"%.1f\"",
"      plot sin(x) t '$\\sin(x)$'",
"      \\stopGNUPLOTscript",
"      \\useGNUPLOTgraphic[sin]",
"      \\stoptext",
" This will run gnuplot automatically and include the resulting figure in the document."
END_HELP(context)
#endif /* TERM_HELP */
