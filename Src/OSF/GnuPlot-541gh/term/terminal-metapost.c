// GNUPLOT - metapost.trm 
// Copyright 1990 - 1993, 1998, 2004
//
/* 1999/04/22
 *			GNUPLOT -- metapost.trm
 *
 *			This terminal driver supports:
 *		                Metapost Commands
 *
 * Based on metafont.trm, written by
 *		Pl Hedne
 *		Trondheim, Norway
 *		Pal.Hedne@termo.unit.no;
 *		with improvements by Carsten Steger
 *
 * and pstricks.trm, written by
 *		David Kotz and Raymond Toy
 *
 * Adapted to metapost by:
 *              Daniel H. Luecking <luecking@comp.uark.edu> and
 *              L Srinivasa Mohan <mohan@chemeng.iisc.ernet.in>
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
	register_term(mp)
#endif

//#ifdef TERM_PROTO
TERM_PUBLIC void MP_options();
TERM_PUBLIC void MP_init(GpTermEntry * pThis);
TERM_PUBLIC void MP_graphics(GpTermEntry * pThis);
TERM_PUBLIC void MP_text(GpTermEntry * pThis);
TERM_PUBLIC void MP_linetype(GpTermEntry * pThis, int linetype);
TERM_PUBLIC void MP_move(GpTermEntry * pThis, uint x, uint y);
TERM_PUBLIC void MP_point(GpTermEntry * pThis, uint x, uint y, int number);
TERM_PUBLIC void MP_pointsize(GpTermEntry * pThis, double size);
TERM_PUBLIC void MP_linewidth(GpTermEntry * pThis, double width);
TERM_PUBLIC void MP_vector(GpTermEntry * pThis, uint ux, uint uy);
TERM_PUBLIC void MP_arrow(GpTermEntry * pThis, uint sx, uint sy, uint ex, uint ey, int head);
TERM_PUBLIC void MP_put_text(GpTermEntry * pThis, uint x, uint y, const char str[]);
TERM_PUBLIC int  MP_justify_text(GpTermEntry * pThis, enum JUSTIFY mode);
TERM_PUBLIC int  MP_text_angle(GpTermEntry * pThis, int ang);
TERM_PUBLIC void MP_reset(GpTermEntry * pThis);
TERM_PUBLIC int  MP_set_font(GpTermEntry * pThis, const char * font);
TERM_PUBLIC void MP_boxfill(GpTermEntry * pThis, int style, uint x1, uint y1, uint width, uint height);
TERM_PUBLIC int  MP_make_palette(GpTermEntry * pThis, t_sm_palette *);
TERM_PUBLIC void MP_previous_palette(GpTermEntry * pThis);
TERM_PUBLIC void MP_set_color(GpTermEntry * pThis, const t_colorspec *);
TERM_PUBLIC void MP_filled_polygon(GpTermEntry * pThis, int, gpiPoint *);
TERM_PUBLIC void MP_dashtype(GpTermEntry * pThis, int type, t_dashtype * custom_dash_type);

/* 5 inches wide by 3 inches high (default) */
#define MP_XSIZE 5.0
#define MP_YSIZE 3.0

/* gnuplot units will be one pixel if printing device has this
   resolution. Too small resolutions (like 300) can give rough
   appearance to curves when user tries to smooth a curve by choosing
   high sampling rate. */
#define MP_DPI (2400)

#define MP_XMAX (MP_XSIZE*MP_DPI)
#define MP_YMAX (MP_YSIZE*MP_DPI)

#define MP_HTIC (5*MP_DPI/72)   /* nominally 5pt   */
#define MP_VTIC (5*MP_DPI/72)   /*    "      5pt   */
#define MP_HCHAR (MP_DPI*53/10/72)      /*    "      5.3pt */
#define MP_VCHAR (MP_DPI*11/72) /*    "      11pt  */
//#endif /* TERM_PROTO */

#ifndef TERM_PROTO_ONLY
#ifdef TERM_BODY

static double MP_xsize = MP_XSIZE;
static double MP_ysize = MP_YSIZE;
/* static double MP_xmax = MP_XMAX;
   static double MP_ymax = MP_YMAX;
 * unused, for now
 */
static int MP_posx;
static int MP_posy;
static char MP_fontname[MAX_ID_LEN + 1];
static double MP_fontsize;
static double MP_textmag;
static enum JUSTIFY MP_justify = LEFT;
static int MP_ang = 0;
static int MP_char_code = 0;

/* number of nodes in an output line so far */
static int MP_linecount = 1;

/* Number of point types */
#define MP_POINT_TYPES 10

/* Number of line types */
#define MP_LINE_TYPES 8

/* are we in the middle of a MP path? */
static bool MP_inline = FALSE;
/* colored or dashed lines? */
static bool MP_color = FALSE;
static bool MP_solid = FALSE;

/* compatibility mode*/
/* static bool MP_notex = FALSE; */
#define MP_NO_TEX 0
#define MP_TEX 1
#define MP_LATEX 2
static int MP_tex = MP_TEX;
/* add usepackage instructions for PSNFSS ? */
#define MP_PSNFSS_NONE 0
#define MP_PSNFSS_7    1
#define MP_PSNFSS_8    2
static int MP_psnfss = MP_PSNFSS_NONE;
/* should amstex packages be included? */
static int MP_amstex = 0;
/* add a4paper option to documentclass */
static int MP_a4paper = 0;
/* write a prologues line */
static int MP_prologues = -1;

/* has color changed? */
static int MP_color_changed = 0;

static int MP_dash_changed = 0;

/* has a font change taken place? */
static bool MP_fontchanged = FALSE;

/* The old types */
static int MP_oldline = -2;

/* The old sizes */
static double MP_oldptsize = 1.0;
static double MP_oldpen = 1.0;

/* terminate any path in progress */
static void MP_endline();

/* max number of path nodes before a newline */
#define MP_LINEMAX 5

enum MP_id {
	MP_OPT_MONOCHROME, MP_OPT_COLOUR,
	MP_OPT_SOLID, MP_OPT_DASHED,
	MP_OPT_NOTEX, MP_OPT_TEX, MP_OPT_LATEX,
	MP_OPT_A4PAPER,
	MP_OPT_PSNFSS, MP_OPT_PSNFSS_V7, MP_OPT_NOPSNFSS,
	MP_OPT_AMSTEX,
	MP_OPT_FONT, MP_OPT_FONTSIZE,
	MP_OPT_PROLOGUES, MP_OPT_NOPROLOGUES,
	MP_OPT_MAGNIFICATION, MP_OPT_OTHER
};

static struct gen_table MP_opts[] = {
	{ "mo$nochrome", MP_OPT_MONOCHROME },
	{ "c$olor", MP_OPT_COLOUR },
	{ "c$olour", MP_OPT_COLOUR },
	{ "s$olid", MP_OPT_SOLID },
	{ "da$shed", MP_OPT_DASHED },
	{ "n$otex", MP_OPT_NOTEX },
	{ "t$ex", MP_OPT_TEX },
	{ "la$tex", MP_OPT_LATEX },
	{ "a4$paper", MP_OPT_A4PAPER },
	{ "am$stex", MP_OPT_AMSTEX },
	{ "ps$nfss", MP_OPT_PSNFSS },
	{ "psnfss-v$ersion7", MP_OPT_PSNFSS_V7 },
	{ "nops$nfss", MP_OPT_NOPSNFSS },
	{ "pro$logues", MP_OPT_PROLOGUES },
	{ "nopro$logues", MP_OPT_NOPROLOGUES },
	{ "ma$gnification", MP_OPT_MAGNIFICATION },
	{ "fo$nt", MP_OPT_FONT },
	{ NULL, MP_OPT_OTHER }
};

TERM_PUBLIC void MP_options(GpTermEntry * pThis, GnuPlot * pGp)
{
	// Annoying hack to handle the case of 'set termoption' after 
	// we have already initialized the terminal.                  
	if(!pGp->Pgm.AlmostEquals(pGp->Pgm.GetPrevTokenIdx(), "termopt$ion")) {
		MP_color = FALSE;
		MP_solid = FALSE;
		MP_tex = MP_TEX;
		MP_a4paper = 0;
		MP_amstex  = 0;
		MP_psnfss = MP_PSNFSS_NONE;
		MP_fontsize = 10.0;
		MP_textmag = 1.0;
		MP_prologues = -1;
		strcpy(MP_fontname, "cmr10");
		pThis->flags |= TERM_IS_LATEX;
	}
	while(!pGp->Pgm.EndOfCommand()) {
		int option = pGp->Pgm.LookupTableForCurrentToken(&MP_opts[0]);
		switch(option) {
			case MP_OPT_MONOCHROME:
			    MP_color = FALSE;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_COLOUR:
			    MP_color = TRUE;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_SOLID:
			    MP_solid = TRUE;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_DASHED:
			    MP_solid = FALSE;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_NOTEX:
			    MP_tex = MP_NO_TEX;
			    strcpy(MP_fontname, "pcrr8r");
			    pThis->flags &= ~TERM_IS_LATEX;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_TEX:
			    MP_tex = MP_TEX;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_LATEX:
			    MP_tex = MP_LATEX;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_AMSTEX:
			    MP_tex = MP_LATEX; /* only makes sense when using LaTeX */
			    MP_amstex = 1;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_A4PAPER:
			    MP_tex = MP_LATEX; /* only makes sense when using LaTeX */
			    MP_a4paper = 1;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_PSNFSS:
			    MP_tex = MP_LATEX; /* only makes sense when using LaTeX */
			    MP_psnfss = MP_PSNFSS_8;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_PSNFSS_V7:
			    MP_tex = MP_LATEX; /* only makes sense when using LaTeX */
			    MP_psnfss = MP_PSNFSS_7;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_NOPSNFSS:
			    MP_psnfss = MP_PSNFSS_NONE;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_PROLOGUES:
			    pGp->Pgm.Shift();
			    if(!(pGp->Pgm.EndOfCommand())) {
				    int dummy_for_prologues;
				    if(sscanf(gp_input_line + pGp->Pgm.GetCurTokenStartIndex(), "%d", &dummy_for_prologues) == 1) {
					    MP_prologues = dummy_for_prologues;
				    }
				    pGp->Pgm.Shift();
			    }
			    break;
			case MP_OPT_NOPROLOGUES:
			    MP_prologues = -1;
			    pGp->Pgm.Shift();
			    break;
			case MP_OPT_MAGNIFICATION:
			    pGp->Pgm.Shift();
			    if(!pGp->Pgm.EndOfCommand()) /* global text scaling */
				    MP_textmag = pGp->RealExpression();
			    break;
			case MP_OPT_FONT:
			    pGp->Pgm.Shift();
			case MP_OPT_OTHER:
			default:
		    {
			    char * s;
			    if((s = pGp->TryToGetString())) {
				    int sep = strcspn(s, ",");
				    if(0 < sep && sep < sizeof(MP_fontname)) {
					    strncpy(MP_fontname, s, sizeof(MP_fontname));
					    MP_fontname[sep] = '\0';
				    }
				    if(s[sep] == ',')
					    sscanf(&s[sep+1], "%lf", &MP_fontsize);
				    SAlloc::F(s);
			    }
			    else if(option == MP_OPT_FONT) {
				    pGp->IntErrorCurToken("expecting font name");
			    }
			    else if(!pGp->Pgm.EndOfCommand()) { /*font size */
				    MP_fontsize = pGp->RealExpression();
				    pGp->Pgm.Shift();
			    }
			    break;
		    }
		}
	}
	/* minimal error recovery: */
	if(MP_fontsize < 5.0)
		MP_fontsize = 5.0;
	if(MP_fontsize > 99.99)
		MP_fontsize = 99.99;
	pThis->ChrV = (uint)(MP_DPI * MP_fontsize * MP_textmag * 11 / 720);
	if(MP_tex == MP_NO_TEX) { /* Courier is a little wider than cmtt */
		pThis->ChrH = (uint)(MP_DPI * MP_fontsize * MP_textmag * 6.0 / 720 + 0.5);
	}
	else {
		pThis->ChrH = (uint)(MP_DPI * MP_fontsize * MP_textmag * 5.3 / 720 + 0.5);
	}
	if(MP_psnfss == MP_PSNFSS_NONE) { /* using the normal font scheme */
		sprintf(term_options, "%s %s %stex%s%s mag %.3f font \"%s,%.2f\" %sprologues(%d)",
		    MP_color ? "color" : "monochrome",
		    MP_solid ? "solid" : "dashed",
		    (MP_tex == MP_NO_TEX) ? "no" : (MP_tex == MP_LATEX) ? "la" : "",
		    MP_a4paper ? " a4paper" : "",
		    MP_amstex ? " amstex" : "",
		    MP_textmag,
		    MP_fontname, MP_fontsize,
		    (MP_prologues > -1) ? "" : "no", MP_prologues);
	}
	else { /* using postscript fonts */
		sprintf(term_options, "%s %s %stex%s%s mag %.3f %s %sprologues(%d)",
		    MP_color ? "color" : "monochrome",
		    MP_solid ? "solid" : "dashed",
		    (MP_tex == MP_NO_TEX) ? "no" : (MP_tex == MP_LATEX) ? "la" : "",
		    MP_a4paper ? " a4paper" : "",
		    MP_amstex ? " amstex" : "",
		    MP_textmag,
		    (MP_psnfss == MP_PSNFSS_7) ? "psnsfss(v7)" : "psnsfss",
		    (MP_prologues > -1) ? "" : "no", MP_prologues);
	};
}

TERM_PUBLIC void MP_init(GpTermEntry * pThis)
{
	time_t now;
	time(&now);
	MP_posx = MP_posy = 0;
	fprintf(gpoutfile, "%%GNUPLOT Metapost output: %s\n", asctime(localtime(&now)));
	if(MP_prologues > -1) {
		fprintf(gpoutfile, "prologues:=%d;\n", MP_prologues);
	}
	if(MP_tex == MP_LATEX) {
		fputs("\n\
%% Add \\documentclass and \\begin{dcoument} for latex\n\
%% NB you should set the environment variable TEX to the name of your\n\
%% latex executable (normally latex) inorder for metapost to work\n\
%% or run\n\
%% mpost --tex=latex ...\n\
\n\
% BEGPRE\n\
verbatimtex\n",
		    gpoutfile);
		if(MP_a4paper) {
			fputs("\\documentclass[a4paper]{article}\n", gpoutfile);
		}
		else {
			fputs("\\documentclass{article}\n", gpoutfile);
		}
		switch(MP_psnfss) {
			case MP_PSNFSS_7: {
			    fputs("\\usepackage[latin1]{inputenc}\n\
\\usepackage[T1]{fontenc}\n\
\\usepackage{times,mathptmx}\n\
\\usepackage{helvet}\n\
\\usepackage{courier}\n",
				gpoutfile);
		    }
		    break;
			case MP_PSNFSS_8: {
			    fputs("\\usepackage[latin1]{inputenc}\n\
\\usepackage[T1]{fontenc}\n\
\\usepackage{textcomp}\n\
\\usepackage{mathptmx}\n\
\\usepackage[scaled=.92]{helvet}\n\
\\usepackage{courier}\n\
\\usepackage{latexsym}\n",
				gpoutfile);
		    }
		    break;
		}
		if(MP_amstex) {
			fputs("\\usepackage[intlimits]{amsmath}\n\
\\usepackage{amsfonts}\n", gpoutfile);
		}
		;
		fputs("\\begin{document}\n\
etex\n% ENDPRE\n", gpoutfile);
	}

	fputs("\n\
warningcheck:=0;\n\
defaultmpt:=mpt:=4;\n\
th:=.6;\n\
%% Have nice sharp joins on our lines\n\
linecap:=butt;\n\
linejoin:=mitered;\n\
\n\
def scalepen expr n = pickup pencircle scaled (n*th) enddef;\n\
def ptsize expr n = mpt:=n*defaultmpt enddef;\n\
\n",
	    gpoutfile);

	fprintf(gpoutfile, "\ntextmag:=%6.3f;\n", MP_textmag);

	fputs("\
vardef makepic(expr str) =\n\
  if picture str : str scaled textmag\n\
  % otherwise a string\n\
  else: str infont defaultfont scaled (defaultscale*textmag)\n\
  fi\n\
enddef;\n\
\n\
def infontsize(expr str, size) =\n\
  infont str scaled (size / fontsize str)\n\
enddef;\n",
	    gpoutfile);

	if(MP_tex == MP_NO_TEX) {
		fprintf(gpoutfile, "\n\
defaultfont:= \"%s\";\n\
defaultscale := %6.3f/fontsize defaultfont;\n", MP_fontname, MP_fontsize);
	}
	else {
		if(MP_tex != MP_LATEX) {
			fputs("\n\
%font changes\n\
verbatimtex\n\
\\def\\setfont#1#2{%.\n\
  \\font\\gpfont=#1 at #2pt\n\
\\gpfont}\n", gpoutfile);
			fprintf(gpoutfile, "\\setfont{%s}{%5.2f}\netex\n",
			    MP_fontname, MP_fontsize);
		}
	}
	fputs("\n\
color currentcolor; currentcolor:=black;\n\
picture currentdash; currentdash:=dashpattern(on 1);\n\
color fillcolor;\n\
boolean colorlines,dashedlines;\n",
	    gpoutfile);
	if(MP_color) {
		fputs("colorlines:=true;\n", gpoutfile);
	}
	else {
		fputs("colorlines:=false;\n", gpoutfile);
	}
	if(MP_solid) {
		fputs("dashedlines:=false;\n", gpoutfile);
	}
	else {
		fputs("dashedlines:=true;\n", gpoutfile);
	}
	fputs("\n\
def _wc = withpen currentpen withcolor currentcolor dashed currentdash enddef;\n\
def _ac = addto currentpicture enddef;\n\
def _sms = scaled mpt shifted enddef;\n\
% drawing point-types\n\
def gpdraw (expr n, x, y) =\n\
  if n<0: _ac contour fullcircle _sms (x,y)\n\
  elseif (n=1) or (n=3):\n\
    _ac doublepath ptpath[n] _sms (x,y) _wc;\n\
    _ac doublepath ptpath[n] rotated 90 _sms (x,y) _wc\n\
  elseif n<6: _ac doublepath ptpath[n] _sms (x,y) _wc\n\
  else: _ac contour ptpath[n] _sms (x,y) _wc\n\
  fi\n\
enddef;\n\
\n\
% the point shapes\n\
path ptpath[];\n\
%diamond\n\
ptpath0 = ptpath6 = (-1/2,0)--(0,-1/2)--(1/2,0)--(0,1/2)--cycle;\n\
% plus sign\n\
ptpath1 = (-1/2,0)--(1/2,0);\n\
% square\n\
ptpath2 = ptpath7 = (-1/2,-1/2)--(1/2,-1/2)--(1/2,1/2)--(-1/2,1/2)--cycle;\n\
% cross\n\
ptpath3 := (-1/2,-1/2)--(1/2,1/2);\n\
% circle:\n\
ptpath4 = ptpath8:= fullcircle;\n\
% triangle\n\
ptpath5 = ptpath9 := (0,1/2)--(-1/2,-1/2)--(1/2,-1/2)--cycle;\n\
\n\
def linetype expr n =\n\
  if n > -3 :\n\
      currentcolor:= if colorlines : col[n] else: black fi;\n\
  fi\n\
  currentdash:= if dashedlines : lt[n] else: dashpattern(on 1) fi;\n\
  if n = -1 :\n\
      drawoptions(withcolor currentcolor withpen (currentpen scaled .5));\n\
  else :\n\
    drawoptions(_wc);\n\
  fi\n\
enddef;\n\
\n\
% dash patterns\n\
picture lt[];\n\
lt[-3]:=dashpattern(off 1 on 0);\n\
lt[-2]:=lt[-1]:=lt0:=dashpattern(on 1);\n\
lt1=dashpattern(on 2 off 2); % dashes\n\
lt2=dashpattern(on 2 off 2 on 0.2 off 2); %dash-dot\n\
lt3=lt1 scaled 1.414;\n\
lt4=lt2 scaled 1.414;\n\
lt5=lt1 scaled 2;\n\
lt6:=lt2 scaled 2;\n\
lt7=dashpattern(on 0.2 off 2); %dots\n\
\n\
color col[],cyan, magenta, yellow;\n\
cyan=blue+green; magenta=red+blue;yellow=green+red;\n\
col[-2]:=col[-1]:=col0:=black;\n\
col1:=red;\n\
col2:=(.2,.2,1); %blue\n\
col3:=(1,.66,0); %orange\n\
col4:=.85*green;\n\
col5:=.9*magenta;\n\
col6:=0.85*cyan;\n\
col7:=.85*yellow;\n\
\n\
%placing text\n\
picture GPtext;\n\
def put_text(expr pic, x, y, r, j) =\n\
  GPtext:=makepic(pic);\n\
  GPtext:=GPtext shifted\n\
    if j = 1: (-(ulcorner GPtext + llcorner GPtext)/2)\n\
    elseif j = 2: (-center GPtext)\n\
    else: (-(urcorner GPtext + lrcorner GPtext)/2)\n\
    fi\n\
    rotated r;\n\
  addto currentpicture also GPtext shifted (x,y)\n\
enddef;\n",
	    gpoutfile);
}

TERM_PUBLIC void MP_graphics(GpTermEntry * pThis)
{
	/* initialize "remembered" drawing parameters */
	MP_oldline = -2;
	MP_oldpen = 1.0;
	MP_oldptsize = GPO.Gg.PointSize;
	fprintf(gpoutfile, "\nbeginfig(%d);\nw:=%.3fin;h:=%.3fin;\n",
	    MP_char_code, MP_xsize, MP_ysize);
	/* MetaPost can only handle numbers up to 4096. When MP_DPI
	 * is larger than 819, this is exceeded by (pThis->MaxX). So we
	 * scale it and all coordinates down by factor of 10.0. And
	 * compensate by scaling a and b up.
	 */
	fprintf(gpoutfile, "a:=w/%.1f;b:=h/%.1f;\n", (pThis->MaxX) / 10.0, (pThis->MaxY) / 10.0);
	fprintf(gpoutfile, "scalepen 1; ptsize %.3f;linetype -2;\n", GPO.Gg.PointSize);
	MP_char_code++;
	/* reset MP_color_changed */
	MP_color_changed = 0;

	MP_dash_changed = 0;
}

TERM_PUBLIC void MP_text(GpTermEntry * pThis)
{
	if(MP_inline)
		MP_endline();
	fputs("endfig;\n", gpoutfile);
}

TERM_PUBLIC void MP_linetype(GpTermEntry * pThis, int lt)
{
	int linetype = lt;
	if(linetype >= MP_LINE_TYPES)
		linetype %= MP_LINE_TYPES;
	if(MP_inline)
		MP_endline();
	// reset the color in case it has been changed in MP_set_color() 
	if((MP_color_changed) || (MP_dash_changed)) {
		MP_oldline = linetype + 1;
		MP_color_changed = 0;
		MP_dash_changed = 0;
	}
	if(MP_oldline != linetype) {
		fprintf(gpoutfile, "linetype %d;\n", linetype);
		MP_oldline = linetype;
	}
}

TERM_PUBLIC void MP_move(GpTermEntry * pThis, uint x, uint y)
{
	if((x != MP_posx) || (y != MP_posy)) {
		if(MP_inline)
			MP_endline();
		MP_posx = x;
		MP_posy = y;
	}                       /* else we seem to be there already */
}

TERM_PUBLIC void MP_point(GpTermEntry * pThis, uint x, uint y, int pt)
{
	int pointtype = pt;
	if(MP_inline)
		MP_endline();
	// Print the shape defined by 'number'; number < 0 means to use a dot, otherwise one of the defined points. 
	if(pointtype >= MP_POINT_TYPES)
		pointtype %= MP_POINT_TYPES;
	// Change %d to %f, divide x,y by 10 
	fprintf(gpoutfile, "gpdraw(%d,%.1fa,%.1fb);\n", pointtype, x / 10.0, y / 10.0);
}

TERM_PUBLIC void MP_pointsize(GpTermEntry * pThis, double ps)
{
	if(ps < 0)
		ps = 1;
	if(MP_oldptsize != ps) {
		if(MP_inline)
			MP_endline();
		fprintf(gpoutfile, "ptsize %.3f;\n", ps);
		MP_oldptsize = ps;
	}
}

TERM_PUBLIC void MP_linewidth(GpTermEntry * pThis, double lw)
{
	if(MP_oldpen != lw) {
		if(MP_inline)
			MP_endline();
		fprintf(gpoutfile, "scalepen %.3f;\n", lw);
		MP_oldpen = lw;
	}
}

TERM_PUBLIC void MP_vector(GpTermEntry * pThis, uint ux, uint uy)
{
	if((ux == MP_posx) && (uy == MP_posy))
		return;         /* Zero length line */
	if(MP_inline) {
		if(MP_linecount++ >= MP_LINEMAX) {
			fputs("\n", gpoutfile);
			MP_linecount = 1;
		}
	}
	else {
		MP_inline = TRUE;
		fprintf(gpoutfile, "draw (%.1fa,%.1fb)", MP_posx / 10.0, MP_posy / 10.0);
		MP_linecount = 2;
	}
	MP_posx = ux;
	MP_posy = uy;
	fprintf(gpoutfile, "--(%.1fa,%.1fb)", MP_posx / 10.0, MP_posy / 10.0);
}

static void MP_endline()
{
	MP_inline = FALSE;
	fprintf(gpoutfile, ";\n");
}

TERM_PUBLIC void MP_arrow(GpTermEntry * pThis, uint sx, uint sy, uint ex, uint ey, int head)
{
	MP_move(pThis, sx, sy);
	if((head & HEADS_ONLY))
		fprintf(gpoutfile, "currentdash:=lt[%d];\n", LT_NODRAW);
	if((head & BOTH_HEADS) == BOTH_HEADS) {
		fprintf(gpoutfile, "%s (%.1fa,%.1fb)--(%.1fa,%.1fb);\n", "drawdblarrow", sx / 10.0, sy / 10.0, ex / 10.0, ey / 10.0);
	}
	else if((head & END_HEAD)) {
		fprintf(gpoutfile, "%s (%.1fa,%.1fb)--(%.1fa,%.1fb);\n", "drawarrow", sx / 10.0, sy / 10.0, ex / 10.0, ey / 10.0);
	}
	else if((head & BACKHEAD)) {
		fprintf(gpoutfile, "%s (%.1fa,%.1fb)--(%.1fa,%.1fb);\n", "drawarrow", ex / 10.0, ey / 10.0, sx / 10.0, sy / 10.0);
	}
	else if(!(head & HEADS_ONLY) && ((sx != ex) || (sy != ey))) {
		fprintf(gpoutfile, "draw (%.1fa,%.1fb)--(%.1fa,%.1fb);\n", sx / 10.0, sy / 10.0, ex / 10.0, ey / 10.0);
	}
	if((head & HEADS_ONLY))
		fprintf(gpoutfile, "currentdash:=lt[%d];\n", MP_oldline);
	MP_posx = ex;
	MP_posy = ey;
}

TERM_PUBLIC void MP_put_text(GpTermEntry * pThis, uint x, uint y, const char str[])
{
	int i, j = 0;
	char * text;
	// ignore empty strings 
	if(!str || !*str)
		return;
	text = sstrdup(str);
	if(MP_inline)
		MP_endline();
	switch(MP_justify) {
		case LEFT: j = 1; break;
		case CENTRE: j = 2; break;
		case RIGHT: j = 3; break;
	}
	if(MP_tex == MP_NO_TEX) {
		for(i = 0; i < sstrleni(text); i++)
			if(text[i] == '"')
				text[i] = '\''; /* Replace " with ' */
		if(MP_fontchanged) {
			fprintf(gpoutfile, "\
put_text(\"%s\" infontsize(\"%s\",%5.2f), %.1fa, %.1fb, %d, %d);\n",
			    text, MP_fontname, MP_fontsize,
			    x / 10.0, y / 10.0, MP_ang, j);
		}
		else {
			fprintf(gpoutfile, "put_text(\"%s\", %.1fa, %.1fb, %d, %d);\n",
			    text, x / 10.0, y / 10.0, MP_ang, j);
		}
	}
	else if(MP_fontchanged) {
		if(MP_tex != MP_LATEX) {
			fprintf(gpoutfile, "\
put_text( btex \\setfont{%s}{%5.2f} %s etex, %.1fa, %.1fb, %d, %d);\n",
			    MP_fontname, MP_fontsize, text,
			    x / 10.0, y / 10.0, MP_ang, j);
		}
		else {
			fprintf(gpoutfile, "put_text( btex %s etex, %.1fa, %.1fb, %d, %d);\n", text, x / 10.0, y / 10.0, MP_ang, j);
		}
	}
	else {
		fprintf(gpoutfile, "put_text( btex %s etex, %.1fa, %.1fb, %d, %d);\n", text, x / 10.0, y / 10.0, MP_ang, j);
	}

	SAlloc::F(text);
}

TERM_PUBLIC int MP_justify_text(GpTermEntry * pThis, enum JUSTIFY mode)
{
	MP_justify = mode;
	return TRUE;
}

TERM_PUBLIC int MP_text_angle(GpTermEntry * pThis, int ang)
{
	// Metapost code does the conversion 
	MP_ang = ang;
	return TRUE;
}

TERM_PUBLIC int MP_set_font(GpTermEntry * pThis, const char * font)
{
	if(*font) {
		size_t sep = strcspn(font, ",");
		if(sep < sizeof(MP_fontname))
			strncpy(MP_fontname, font, sizeof(MP_fontname)-1);
		sscanf(&(font[sep + 1]), "%lf", &MP_fontsize);
		if(MP_fontsize < 5)
			MP_fontsize = 5.0;
		if(MP_fontsize >= 100)
			MP_fontsize = 99.99;
		/*  */
		MP_fontchanged = TRUE;
	}
	else {
		MP_fontchanged = FALSE;
	}
	return TRUE;
}

TERM_PUBLIC void MP_reset(GpTermEntry * pThis)
{
	if(MP_tex == MP_LATEX) {
		fputs("% BEGPOST\n", gpoutfile);
		fputs("verbatimtex\n", gpoutfile);
		fputs(" \\end{document}\n", gpoutfile);
		fputs("etex\n", gpoutfile);
		fputs("% ENDPOST\n", gpoutfile);
	}
	;
	fputs("end.\n", gpoutfile);
}

TERM_PUBLIC void MP_boxfill(GpTermEntry * pThis, int style, uint x1, uint y1, uint wd, uint ht)
{
	/* fillpar:
	 * - solid   : 0 - 100% intensity
	 * - pattern : 0 - n    pattern number
	 */
	int fillpar = style >> 4;
	style &= 0xf;
	if(MP_inline)
		MP_endline();
	switch(style) {
		case FS_EMPTY: /* fill with background color */
		    fprintf(gpoutfile, "\
fill (%.1fa,%.1fb)--(%.1fa,%.1fb)--(%.1fa,%.1fb)--(%.1fa,%.1fb)--cycle withcolor background;\n",
			x1 / 10.0, y1 / 10.0, (x1 + wd) / 10.0, y1 / 10.0,
			(x1 + wd) / 10.0, (y1 + ht) / 10.0, x1 / 10.0,
			(y1 + ht) / 10.0);
		    break;

		case FS_PATTERN: /* pattern fill */
		case FS_TRANSPARENT_PATTERN:
		    /* FIXME: not yet implemented, dummy it up as fill density */
		    fillpar *= 12;

		default:
		case FS_SOLID: /* solid fill */
		case FS_TRANSPARENT_SOLID:
		    if(fillpar < 100) {
			    double density = (100-fillpar) * 0.01;
			    fprintf(gpoutfile, "fillcolor:=currentcolor*%.2f+background*%.2f;\n",
				1.0-density, density);
			    MP_color_changed = 1;
		    }
		    else
			    fprintf(gpoutfile, "fillcolor:=currentcolor;\n");
		    fprintf(gpoutfile,
			"\
fill (%.1fa,%.1fb)--(%.1fa,%.1fb)--(%.1fa,%.1fb)--(%.1fa,%.1fb)--cycle withpen (pencircle scaled 0pt) withcolor fillcolor;\n",
			x1 / 10.0,
			y1 / 10.0,
			(x1 + wd) / 10.0,
			y1 / 10.0,
			(x1 + wd) / 10.0,
			(y1 + ht) / 10.0,
			x1 / 10.0,
			(y1 + ht) / 10.0);
		    break;
	}
}

TERM_PUBLIC int MP_make_palette(GpTermEntry * pThis, t_sm_palette * palette)
{
	return 0; // metapost can do continuous number of colours 
}

TERM_PUBLIC void MP_set_color(GpTermEntry * pThis, const t_colorspec * colorspec)
{
	double gray = colorspec->value;
	rgb_color color;
	// remember that we changed the color, needed to reset color in MP_linetype()
	// FIXME: only set this if the color really did change (compare to previous color) 
	MP_color_changed = 1;
	if(MP_inline)
		MP_endline();

	if(!MP_color) {         /* gray mode */
		if(gray < 1e-3) gray = 0;
		fprintf(gpoutfile, "currentcolor:=%.3gwhite;\n", gray);
	}
	else {                  /* color mode */
		if(colorspec->type == TC_LT) {
			int linecolor = colorspec->lt;
			if(linecolor >= MP_LINE_TYPES)
				linecolor %= MP_LINE_TYPES;
			if(linecolor == -1)
				fprintf(gpoutfile, "currentcolor:=black;\n");
			else if(linecolor >= 0)
				fprintf(gpoutfile, "currentcolor:=col%d;\n", linecolor);
		}
		if(colorspec->type == TC_FRAC) {
			if(GPO.SmPltt.Colors) /* finite nb of colors explicitly requested */
				gray = (gray >= ((double)(GPO.SmPltt.Colors-1)) / GPO.SmPltt.Colors) ? 1 : floor(gray * GPO.SmPltt.Colors) / GPO.SmPltt.Colors;
			GPO.Rgb1FromGray(gray, &color);
		}
		else if(colorspec->type == TC_RGB) {
			color.r = (double)((colorspec->lt >> 16 ) & 255) / 255.0;
			color.g = (double)((colorspec->lt >> 8 ) & 255) / 255.0;
			color.b = (double)(colorspec->lt & 255) / 255.0;
		}
		else
			return;
		if(color.r < 1e-4) color.r = 0.0;
		if(color.g < 1e-4) color.g = 0.0;
		if(color.b < 1e-4) color.b = 0.0;
		fprintf(gpoutfile, "currentcolor:=%.4g*red+%.4g*green+%.4g*blue;\n", color.r, color.g, color.b);
	}
}

TERM_PUBLIC void MP_filled_polygon(GpTermEntry * pThis, int points, gpiPoint * corners)
{
	int i;
	int fillpar = corners->style >> 4;
	int style = corners->style & 0xf;
	if(MP_inline)
		MP_endline();
	switch(style) {
		case FS_EMPTY: /* fill with background color */
		    fprintf(gpoutfile, "fillcolor:=background;\n");
		    break;
		case FS_PATTERN: /* pattern fill implemented as partial density */
		case FS_TRANSPARENT_PATTERN:
		    fillpar *= 12;
		case FS_SOLID: /* solid fill */
		case FS_TRANSPARENT_SOLID:
		    if(fillpar < 100) {
			    double density = (100-fillpar) * 0.01;
			    fprintf(gpoutfile, "fillcolor:=currentcolor*%.2f+background*%.2f;\n",
				1.0-density, density);
		    }
		    else {
			    fprintf(gpoutfile, "fillcolor:=currentcolor;\n");
		    }
		default:
		    break;
	}

	fprintf(gpoutfile, "fill ");
	for(i = 0; i < points; i++)
		fprintf(gpoutfile, "(%.1fa,%.1fb)%s",
		    corners[i].x / 10.0, corners[i].y / 10.0,
		    (i < points - 1 && (i + 1) % MP_LINEMAX == 0) ? "\n--" : "--");
	fprintf(gpoutfile, "cycle withcolor fillcolor;\n");
}

TERM_PUBLIC void MP_dashtype(GpTermEntry * pThis, int type, t_dashtype * custom_dash_type)
{
	switch(type) {
		case DASHTYPE_SOLID:
		    fprintf(gpoutfile, "%%MP_dashtype%% DASHTYPE_SOLID\n");
		    break;
		case DASHTYPE_AXIS:
		    fprintf(gpoutfile, "%%MP_dashtype%% DASHTYPE_AXIS\n");
		    /* Currently handled elsewhere? */
		    break;
		case DASHTYPE_CUSTOM:
		    fprintf(gpoutfile, "%%MP_dashtype%% DASHTYPE_CUSTOM: ");
		    if(custom_dash_type) {
			    int i;
			    if(custom_dash_type->dstring[0] != '\0')
				    fprintf(gpoutfile, "\"%s\"; ", custom_dash_type->dstring);
			    fprintf(gpoutfile, "[");
			    for(i = 0; i < DASHPATTERN_LENGTH && custom_dash_type->pattern[i] > 0; i++)
				    fprintf(gpoutfile, i ? ", %.2f" : "%.2f", custom_dash_type->pattern[i]);
			    fprintf(gpoutfile, "]");
			    fprintf(gpoutfile, "\n");

			    MP_dash_changed = 1;
			    fprintf(gpoutfile, "currentdash:=dashpattern(");
			    for(i = 0; i < DASHPATTERN_LENGTH && custom_dash_type->pattern[i] > 0; i++)
				    fprintf(gpoutfile, "%s %.2f ", i%2 ? "off" : "on", custom_dash_type->pattern[i]);
			    fprintf(gpoutfile, ");\n");
		    }
		    else {
			    fprintf(gpoutfile, "\n");
		    }
		    break;
		default:
		    fprintf(gpoutfile, "%%MP_dashtype%% type = %i\n", type);
		    if(type>0)
			    MP_linetype(pThis, type);
		    break;
	}
}

TERM_PUBLIC void MP_previous_palette(GpTermEntry * pThis)
{
}

#endif /* TERM_BODY */

#ifdef TERM_TABLE

TERM_TABLE_START(mp_driver)
	"mp", 
	"MetaPost plotting standard",
	static_cast<uint>(MP_XMAX),
	static_cast<uint>(MP_YMAX),
	MP_VCHAR, 
	MP_HCHAR,
	MP_VTIC, 
	MP_HTIC, 
	MP_options, 
	MP_init, 
	MP_reset,
	MP_text, 
	GnuPlot::NullScale, 
	MP_graphics, 
	MP_move, 
	MP_vector,
	MP_linetype, 
	MP_put_text, 
	MP_text_angle,
	MP_justify_text, 
	MP_point, 
	MP_arrow, 
	MP_set_font, 
	MP_pointsize,
	TERM_BINARY|TERM_CAN_CLIP|TERM_CAN_DASH|TERM_IS_LATEX,          /*flags*/
	0, 
	0, 
	MP_boxfill, 
	MP_linewidth,
	#ifdef USE_MOUSE
	0, 
	0, 
	0, 
	0, 
	0,                /* no mouse support for metapost */
	#endif
	MP_make_palette,
	MP_previous_palette,            /* write grestore */
	MP_set_color,
	MP_filled_polygon,
	0, // image
	0, // enhanced_open
	0, // enhanced_flush
	0, // enhanced_writec
	0, // layer
	0, // path
	0.0, // tscale
	0, // hypertext
	0, // boxed_text
	0, // modify_plots
	MP_dashtype // dashtype
TERM_TABLE_END(mp_driver)
#undef LAST_TERM
#define LAST_TERM mp_driver

#endif                          /* TERM_TABLE */
#endif                          /* TERM_PROTO_ONLY */

#ifdef TERM_HELP
START_HELP(mp)
"1 mp",
"?commands set terminal mpost",
"?set terminal mp",
"?set term mp",
"?terminal mp",
"?term mp",
"?mp",
"?metapost",
"",
" Note: legacy terminal (not built by default).",
" The `mp` driver produces output intended to be input to the Metapost program.",
" Running Metapost on the file creates EPS files containing the plots. By",
" default, Metapost passes all text through TeX.  This has the advantage of",
" allowing essentially  any TeX symbols in titles and labels.",
"",
" Syntax:",
"    set term mp {color | colour | monochrome}",
"                {solid | dashed}",
"                {notex | tex | latex}",
"                {magnification <magsize>}",
"                {psnfss | psnfss-version7 | nopsnfss}",
"                {prologues <value>}",
"                {a4paper}",
"                {amstex}",
"                {\"<fontname> {,<fontsize>}\"} ",
"",
" The option `color` causes lines to be drawn in color (on a printer or display",
" that supports it), `monochrome` (or nothing) selects black lines.  The option",
" `solid` draws solid lines, while `dashed` (or nothing) selects lines with",
" different patterns of dashes.  If `solid` is selected but `color` is not,",
" nearly all lines will be identical.  This may occasionally be useful, so it is",
" allowed.",
"",
" The option `notex` bypasses TeX entirely, therefore no TeX code can be used in",
" labels under this option.  This is intended for use on old plot files or files",
" that make frequent use of common characters like `$` and `%` that require",
" special handling in TeX.",
"",
" The option `tex` sets the terminal to output its text for TeX to process.",
"",
" The option `latex` sets the terminal to output its text for processing by",
" LaTeX. This allows things like \\frac for fractions which LaTeX knows about",
" but TeX does not.  Note that you must set the environment variable TEX to the",
" name of your LaTeX executable (normally latex) if you use this option or use",
" `mpost --tex=<name of LaTeX executable> ...`. Otherwise metapost will try and",
" use TeX to process the text and it won't work.",
"",
" Changing font sizes in TeX has no effect on the size of mathematics, and there",
" is no foolproof way to make such a change, except by globally  setting a",
" magnification factor. This is the purpose of the `magnification` option. It",
" must be followed by a scaling factor. All text (NOT the graphs) will be scaled",
" by this factor. Use this if you have math that you want at some size other",
" than the default 10pt. Unfortunately, all math will be the same size, but see",
" the discussion below on editing the MP output. `mag` will also work under",
" `notex` but there seems no point in using it as the font size option (below)",
" works as well.",
"",
" The option `psnfss` uses postscript fonts in combination with LaTeX. Since",
" this option only makes sense, if LaTeX is being used, the `latex` option is selected",
" automatically. This option includes the following packages for LaTeX:",
" inputenc(latin1), fontenc(T1), mathptmx, helvet(scaled=09.2), courier, latexsym ",
" and textcomp.",
"",
" The option `psnfss-version7` uses also postscript fonts in LaTeX (option `latex`",
" is also automatically selected), but uses the following packages with LaTeX:",
" inputenc(latin1), fontenc(T1), times, mathptmx, helvet and courier.",
"",
" The option `nopsnfss` is the default and uses the standard font (cmr10 if not",
" otherwise specified).",
"",
" The option `prologues` takes a value as an additional argument and adds the line",
" `prologues:=<value>` to the metapost file. If a value of `2` is specified metapost",
" uses postscript fonts to generate the eps-file, so that the result can be viewed",
" using e.g. ghostscript. Normally the output of metapost uses TeX fonts and therefore",
" has to be included in a (La)TeX file before you can look at it.",
"",
" The option `noprologues` is the default. No additional line specifying the prologue",
" will be added.",
"",
" The option `a4paper` adds a `[a4paper]` to the documentclass. Normally letter paper",
" is used (default). Since this option is only used in case of LaTeX, the `latex` option",
" is selected automatically.",
"",
" The option `amstex` automatically selects the `latex` option and includes the following",
" LaTeX packages: amsfonts, amsmath(intlimits). By default these packages are not",
" included.",
"",
" A name in quotes selects the font that will be used when no explicit font is",
" given in a `set label` or `set title`.  A name recognized by TeX (a TFM file",
" exists) must be used.  The default is \"cmr10\" unless `notex` is selected,",
" then it is \"pcrr8r\" (Courier).  Even under `notex`, a TFM file is needed by",
" Metapost. The file `pcrr8r.tfm` is the name given to Courier in LaTeX's psnfss",
" package.  If you change the font from the `notex` default, choose a font that",
" matches the ASCII encoding at least in the range 32-126.  `cmtt10` almost",
" works, but it has a nonblank character in position 32 (space).",
"",
" The size can be any number between 5.0 and 99.99.  If it is omitted, 10.0 is",
" used.  It is advisable to use `magstep` sizes: 10 times an integer or",
" half-integer power of 1.2, rounded to two decimals, because those are the most",
" available sizes of fonts in TeX systems.",
"",
" All the options are optional.  If font information is given, it must be at the",
" end, with size (if present) last.  The size is needed to select a size for the",
" font, even if the font name includes size information.  For example,",
" `set term mp \"cmtt12\"` selects cmtt12 shrunk to the default size 10.  This",
" is probably not what you want or you would have used cmtt10.",
"",
" The following common ascii characters need special treatment in TeX:",
"    $, &, #, %, _;  |, <, >;  ^, ~,  \\, {, and }",
" The five characters $, #, &, _, and % can simply be escaped, e.g., `\\$`.",
" The three characters <, >, and | can be wrapped in math mode, e.g., `$<$`.",
" The remainder require some TeX work-arounds.  Any good book on TeX will give",
" some guidance.",
"",
" If you type your labels inside double quotes, backslashes in TeX code need to",
" be escaped (doubled). Using single quotes will avoid having to do this, but",
" then you cannot use `\\n` for line breaks.  As of this writing, version 3.7 of",
" gnuplot processes titles given in a `plot` command differently than in other",
" places, and backslashes in TeX commands need to be doubled regardless of the",
" style of quotes.",
"",
" Metapost pictures are typically used in TeX documents.  Metapost deals with",
" fonts pretty much the same way TeX does, which is different from most other",
" document preparation programs.  If the picture is included in a LaTeX document",
" using the graphics package, or in a plainTeX document via epsf.tex, and then",
" converted to PostScript with dvips (or other dvi-to-ps converter), the text in",
" the plot will usually be handled correctly.  However, the text may not appear",
" if you send the Metapost output as-is to a PostScript interpreter.",
"",
"2 Metapost Instructions",
"?commands set terminal mp detailed",
"?set terminal mp detailed",
"?set term mp detailed",
"?mp detailed",
"?metapost detailed",
"",
" - Set your terminal to Metapost, e.g.:",
"    set terminal mp mono \"cmtt12\" 12",
"",
" - Select an output-file, e.g.:",
"    set output \"figure.mp\"",
"",
" - Create your pictures.  Each plot (or multiplot group) will generate a",
" separate Metapost beginfig...endfig group.  Its default size will be 5 by 3",
" inches.  You can change the size by saying `set size 0.5,0.5` or whatever",
" fraction of the default size you want to have.",
"",
" - Quit gnuplot.",
"",
" - Generate EPS files by running Metapost on the output of gnuplot:",
"    mpost figure.mp  OR  mp figure.mp",
" The name of the Metapost program depends on the system, typically `mpost` for",
" a Unix machine and `mp` on many others.  Metapost will generate one EPS file",
" for each picture.",
"",
" - To include your pictures in your document you can use the graphics package",
" in LaTeX or epsf.tex in plainTeX:",
"    \\usepackage{graphics} % LaTeX",
"    \\input epsf.tex       % plainTeX",
" If you use a driver other than dvips for converting TeX DVI output to PS, you",
" may need to add the following line in your LaTeX document:",
"    \\DeclareGraphicsRule{*}{eps}{*}{}",
" Each picture you made is in a separate file.  The first picture is in, e.g.,",
" figure.0, the second in figure.1, and so on....  To place the third picture in",
" your document, for example, all you have to do is:",
"    \\includegraphics{figure.2} % LaTeX",
"    \\epsfbox{figure.2}         % plainTeX",
"",
" The advantage, if any, of the mp terminal over a postscript terminal is",
" editable output.  Considerable effort went into making this output as clean as",
" possible.  For those knowledgeable in the Metapost language, the default line",
" types and colors can be changed by editing the arrays `lt[]` and `col[]`.",
" The choice of solid vs dashed lines, and color vs black lines can be change by",
" changing the values assigned to the booleans `dashedlines` and `colorlines`.",
" If the default `tex` option was in effect, global changes to the text of",
" labels can be achieved by editing the `vebatimtex...etex` block.  In",
" particular, a LaTeX preamble can be added if desired, and then LaTeX's",
" built-in size changing commands can be used for maximum flexibility. Be sure",
" to set the appropriate MP configuration variable to force Metapost to run",
" LaTeX instead of plainTeX."
END_HELP(mp)
#endif                          /* TERM_HELP */
