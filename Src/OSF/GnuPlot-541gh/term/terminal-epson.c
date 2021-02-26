// GNUPLOT - epson.trm 
// Copyright 1990 - 1993, 1998, 2004
//
/*
 * This file is included by ../term.c.
 *
 * This terminal driver supports:
 *  epson_lx800, nec_cp6, starc,
 *  epson_60dpi, tandy_60dpi
 *  epson_180dpi
 *  dpu414
 *
 * AUTHORS: Russell Lang, William Wilson
 * send your comments or suggestions to (gnuplot-info@lists.sourceforge.net).
 */
/*
 * adapted to the new terminal layout by Stefan Bodewig (1995)
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

// wire printers 
#define EPSONP // Epson LX-800, Star NL-10, NX-1000 and lots of others 
#define EPS60  // Epson-style 60-dot per inch printers 
#define EPS180 // Epson-style 180-dot per inch (24 pin) printers 
#define NEC
#define OKIDATA
#define STARC
#define DPU414  // Seiko DPU-414 thermal printer 
#define TANDY60 // Tandy DMP-130 series 60-dot per inch graphics 

#ifdef TERM_REGISTER
	#ifdef EPS180
		register_term(epson_180dpi)
	#endif /* EPS180 */
	#ifdef EPS60
		register_term(epson_60dpi)
	#endif /* EPS60 */
	#ifdef EPSONP
		register_term(epson_lx800)
	#endif /* EPSONP */
	#ifdef NEC
		register_term(nec_cp6)
	#endif /* NEC */
	#ifdef OKIDATA
		register_term(okidata)
	#endif /* OKIDATA */
	#ifdef STARC
		register_term(starc)
	#endif /* STARC */
	#ifdef TANDY60
		register_term(tandy_60dpi)
	#endif /* TANDY60 */
	#ifdef DPU414
		register_term(dpu414)
	#endif /* DPU414 */
#endif /* TERM_REGISTER */

//#ifdef TERM_PROTO
TERM_PUBLIC void EPSON_reset(GpTermEntry * pThis);
#if defined(EPS180) || defined(EPS60) || defined(EPSONP) || defined(TANDY60) || defined(OKIDATA)
	TERM_PUBLIC void EPSON_init(GpTermEntry * pThis);
	#if defined(EPS60) || defined(EPSONP) || defined(TANDY60) || defined(OKIDATA)
		#define EPSONVCHAR              FNT5X9_VCHAR
		#define EPSONHCHAR              FNT5X9_HCHAR
		#define EPSONVTIC               6
		#define EPSONHTIC               6
		#ifdef EPSONP
			#define EPSONXMAX       512
			#define EPSONYMAX       384
			TERM_PUBLIC void EPSON_graphics(GpTermEntry * pThis);
			TERM_PUBLIC void EPSON_text(GpTermEntry * pThis);
		#endif /* EPSONP */
	#endif /* four drivers */
#endif /* all five */
#ifdef EPS180
	TERM_PUBLIC void EPS180_graphics(GpTermEntry * pThis);
	TERM_PUBLIC void EPS180_text(GpTermEntry * pThis);
	#define EPSON180VCHAR           FNT13X25_VCHAR
	#define EPSON180HCHAR           FNT13X25_HCHAR
	#define EPSON180VTIC            18
	#define EPSON180HTIC            18
	/* make the total dimensions 8 inches by 6 inches */
	#define EPS180XMAX      1260
	#define EPS180YMAX      1080
#endif /* EPS180 */
#if defined(EPS60) || defined(TANDY60) || defined(OKIDATA)
	// make the total dimensions 8 inches by 5 inches 
	#define EPS60XMAX       480
	#define EPS60YMAX       360
	TERM_PUBLIC void EPS60_graphics(GpTermEntry * pThis);
	#ifdef TANDY60
		TERM_PUBLIC void TANDY60_text(GpTermEntry * pThis);
	#endif /* TANDY60 */
	#ifdef OKIDATA
		TERM_PUBLIC void OKIDATA_text(GpTermEntry * pThis);
	#endif /* OKIDATA */
	#ifdef EPS60
		TERM_PUBLIC void EPS60_text(GpTermEntry * pThis);
	#endif /* EPS60 */
#endif /* all three */
#ifdef NEC
	TERM_PUBLIC void NEC_options();
	TERM_PUBLIC void NEC_init(GpTermEntry * pThis);
	TERM_PUBLIC void NEC_graphics(GpTermEntry * pThis);
	TERM_PUBLIC void NEC_text(GpTermEntry * pThis);
	TERM_PUBLIC void NEC_linetype(GpTermEntry * pThis, int linetype);
	#define NECXMAX 400
	#define NECYMAX 320
	#define NECVCHAR                FNT5X9_VCHAR
	#define NECHCHAR                FNT5X9_HCHAR
	#define NECVTIC         6
	#define NECHTIC         6
#endif /* NEC */
#ifdef STARC
	TERM_PUBLIC void STARC_init(GpTermEntry * pThis);
	TERM_PUBLIC void STARC_graphics(GpTermEntry * pThis);
	TERM_PUBLIC void STARC_text(GpTermEntry * pThis);
	TERM_PUBLIC void STARC_linetype(GpTermEntry * pThis, int linetype);
	#define STARCXMAX       512
	#define STARCYMAX       384
	#define STARCVCHAR              FNT5X9_VCHAR
	#define STARCHCHAR              FNT5X9_HCHAR
	#define STARCVTIC               6
	#define STARCHTIC               6
#endif /* STARC */
#ifdef DPU414
	TERM_PUBLIC void DPU414_options(GpTermEntry * pThis, GnuPlot * pGp);
	TERM_PUBLIC void DPU414_init(GpTermEntry * pThis);
	TERM_PUBLIC void DPU414_graphics(GpTermEntry * pThis);
	TERM_PUBLIC void DPU414_text(GpTermEntry * pThis);
	TERM_PUBLIC void DPU414_setfont(GpTermEntry * pThis);
	#define DPU414XMAX      (640)
	#define DPU414YMAX      (480)
	#define DPU414VCHAR     FNT5X9_VCHAR
	#define DPU414HCHAR     FNT5X9_HCHAR
	#define DPU414VTIC      FNT5X9_HBITS
	#define DPU414HTIC      FNT5X9_HBITS
#endif /* DPU414 */
//#endif /* TERM_PROTO */

#ifndef TERM_PROTO_ONLY
#ifdef TERM_BODY

TERM_PUBLIC void EPSON_reset(GpTermEntry * pThis)
{
	fflush_binary(); /* Only needed for VMS */
}

#ifdef EPSONP

/* The following epson lx800 driver uses generic bit mapped graphics
   routines to build up a bit map in memory. */
/* by Russell Lang, rjl@monu1.cc.monash.edu.au */
/* On PC, print using 'copy file /b lpt1:', do NOT use 'print' */
/* EPSON_init changes gpoutfile to binary mode on PC's */

static void epson_dump(GpTermEntry * pThis);

#define EPSONXLAST (EPSONXMAX - 1)
#define EPSONYLAST (EPSONYMAX - 1)

TERM_PUBLIC void EPSON_init(GpTermEntry * pThis)
{
}

TERM_PUBLIC void EPSON_graphics(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	p_gp->BmpCharSize(FNT5X9);
	p_gp->BmpMakeBitmap((uint)(EPSONXMAX * p_gp->V.Size.x), (uint)(EPSONYMAX * p_gp->V.Size.y), 1);
}

TERM_PUBLIC void EPSON_text(GpTermEntry * pThis)
{
	epson_dump(pThis);
	pThis->P_Gp->BmpFreeBitmap();
}
//
// output file must be binary mode for epson_dump 
//
static void epson_dump(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	for(int j = (p_gp->_Bmp.b_ysize / 8) - 1; j >= 0; j--) {
		// select plotter graphics mode (square pixels) 
		fputs("\033J\030", gpoutfile); /* line feed 8/72" = 8 dots */
		fputs("\r\033*\005", gpoutfile);
		fputc((char)(p_gp->_Bmp.b_xsize % 256), gpoutfile);
		fputc((char)(p_gp->_Bmp.b_xsize / 256), gpoutfile);
		for(uint x = 0; x < p_gp->_Bmp.b_xsize; x++) {
			fputc((char)(*((*p_gp->_Bmp.b_p)[j] + x)), gpoutfile);
		}
	}
#ifdef PC
	fputs("Print using: COPY /B\n", stderr);
#endif
}

#endif /* EPSONP */

/* The following NEC CP6 Pinwriter driver uses generic bit mapped graphics
   routines to build up a bit map in memory. */
/* by Russell Lang, rjl@monu1.cc.monash.edu.au */
/* On PC, print using 'copy file /b lpt1:', do NOT use 'print' */
/* NEC_init changes gpoutfile to binary mode for PC's */

/* Add a Monochrome NEC printer (for faster speed and line types) jdc */

#ifdef NEC

static void nec_dump(GpTermEntry * pThis);
static void nec_draft_dump(GpTermEntry * pThis);

#define NECXLAST (NECXMAX - 1)
#define NECYLAST (NECYMAX - 1)

/* plane 0=black, 1=cyan(blue), 2=magenta(red), 3=yellow */
static uint neccolor[] = { 1, 8, 4, 2, 10, 12, 6, 14 };
static uint necpcolor[] = { 0, 2, 1, 4 };
static int NECmode;

enum NEC_id { NEC_MONOCHROME, NEC_COLOR, NEC_DRAFT, NEC_OTHER };

static struct gen_table NEC_opts[] =
{
	{ "m$onochrome", NEC_MONOCHROME },
	{ "c$olor", NEC_COLOR },
	{ "c$olour", NEC_COLOR },
	{ "d$raft", NEC_DRAFT },
	{ NULL, NEC_OTHER }
};

TERM_PUBLIC void NEC_options(GpTermEntry * pThis, GnuPlot * pGp)
{
	GnuPlot * p_gp = pThis->P_Gp;
	// default 
	if(p_gp->Pgm.EndOfCommand()) {
		strcpy(term_options, "monochrome");
		NECmode = 'm';
	}
	while(!p_gp->Pgm.EndOfCommand()) {
		switch(p_gp->Pgm.LookupTableForCurrentToken(&NEC_opts[0])) {
			case NEC_MONOCHROME:
			    p_gp->Pgm.Shift();
			    strcpy(term_options, "monochrome");
			    NECmode = 'm';
			    break;
			case NEC_COLOR:
			    p_gp->Pgm.Shift();
			    strcpy(term_options, "color");
			    NECmode = 'c';
			    break;
			case NEC_DRAFT:
			    p_gp->Pgm.Shift();
			    strcpy(term_options, "draft");
			    NECmode = 'd';
			    break;
			case NEC_OTHER:
			default:
			    // FIXME - not the most sensible thing to do 
			    // error, but since the terminal is already set, default to mono 
			    strcpy(term_options, "monochrome");
			    NECmode = 'm';
			    p_gp->IntErrorCurToken("modes: color, monochrome, draft");
			    break;
		}
	}
}

TERM_PUBLIC void NEC_init(GpTermEntry * pThis)
{
}

TERM_PUBLIC void NEC_graphics(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	p_gp->BmpCharSize(FNT5X9);
	p_gp->BmpMakeBitmap((uint)(NECXMAX * p_gp->V.Size.x), (uint)(NECYMAX * p_gp->V.Size.y), (NECmode == 'c' ? 4 : 1));
}

TERM_PUBLIC void NEC_text(GpTermEntry * pThis)
{
	if(NECmode == 'd') {
		nec_draft_dump(pThis);
	}
	else {
		nec_dump(pThis);
	}
	pThis->P_Gp->BmpFreeBitmap();
}

TERM_PUBLIC void NEC_linetype(GpTermEntry * pThis, int linetype)
{
	if(NECmode == 'c') {
		if(linetype >= 6)
			linetype %= 6;
		b_setvalue(neccolor[linetype + 2]);
	}
	else {
		b_setlinetype(pThis, linetype);
	}
}
//
// output file must be binary mode for nec_dump 
//
static void nec_dump(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	uint x;
	uint plane, offset;
	int j;
	uint column8;
	ulong column24;
	char column3, column2, column1;
	fputs("\033P\033l\005", gpoutfile); // 10cpi, left margin 5 char 
	for(j = (p_gp->_Bmp.b_ysize / 8) - 1; j >= 0; j--) {
		fputs("\033J\030", gpoutfile); // 24/180" line feed 
		for(plane = 0; plane < p_gp->_Bmp.b_planes; plane++) {
			offset = plane * p_gp->_Bmp.b_psize;
			if(p_gp->_Bmp.b_planes > 1) {
				// select colour for plane 
				fputs("\033r", gpoutfile);
				fputc((char)necpcolor[plane], gpoutfile);
			}
			// select plotter graphics mode (square pixels) 
			fputs("\r\033*\047", gpoutfile);
			fputc((char)((p_gp->_Bmp.b_xsize * 3) % 256), gpoutfile);
			fputc((char)((p_gp->_Bmp.b_xsize * 3) / 256), gpoutfile);
			for(x = 0; x < p_gp->_Bmp.b_xsize; x++) {
				column8 = (uint)(*((*p_gp->_Bmp.b_p)[j + offset] + x));
				column24 = 0;
				if(column8 & 0x01)
					column24 |= (long)0x000007;
				if(column8 & 0x02)
					column24 |= (long)0x000038;
				if(column8 & 0x04)
					column24 |= (long)0x0001c0;
				if(column8 & 0x08)
					column24 |= (long)0x000e00;
				if(column8 & 0x10)
					column24 |= (long)0x007000;
				if(column8 & 0x20)
					column24 |= (long)0x038000;
				if(column8 & 0x40)
					column24 |= (long)0x1c0000;
				if(column8 & 0x80)
					column24 |= (long)0xe00000;
				column1 = (char)(column24 & (long)0xff);
				column2 = (char)((column24 >> 8) & (long)0xff);
				column3 = (char)((column24 >> 16) & (long)0xff);
				fputc(column3, gpoutfile);
				fputc(column2, gpoutfile);
				fputc(column1, gpoutfile);
				fputc(column3, gpoutfile);
				fputc(column2, gpoutfile);
				fputc(column1, gpoutfile);
				fputc(column3, gpoutfile);
				fputc(column2, gpoutfile);
				fputc(column1, gpoutfile);
			}
		}
	}
	fputs("\r\033l", gpoutfile);
	fputc('\0', gpoutfile);   /* set left margin to 0 */
	if(p_gp->_Bmp.b_planes > 1) {
		fprintf(gpoutfile, "\033r");
		fputc('\0', gpoutfile); /* set color to black */
	}
#ifdef PC
	fputs("Print using: COPY /B\n", stderr);
#endif
	fflush_binary(); /* Only needed for VMS */
}
//
// output file must be binary mode for nec_dump 
//
static void nec_draft_dump(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	uint x;
	uint plane, offset;
	int j;
	fputs("\033P\033l\005\r", gpoutfile);   /* 10cpi, left margin 5 char */
	for(j = (p_gp->_Bmp.b_ysize / 8) - 1; j >= 0; j--) {
		fputs("\033J\030", gpoutfile); /* 24/180" line feed */
		for(plane = 0; plane < p_gp->_Bmp.b_planes; plane++) {
			offset = plane * p_gp->_Bmp.b_psize;
			if(p_gp->_Bmp.b_planes > 1) {
				// select colour for plane 
				fputs("\033r", gpoutfile);
				fputc((char)necpcolor[plane], gpoutfile);
			}
			// select plotter graphics mode (square pixels) 
			fputs("\r\033*", gpoutfile);
			fputc('\0', gpoutfile);
			fputc((char)(p_gp->_Bmp.b_xsize % 256), gpoutfile);
			fputc((char)(p_gp->_Bmp.b_xsize / 256), gpoutfile);
			for(x = 0; x < p_gp->_Bmp.b_xsize; x++) {
				fputc((char)(*((*p_gp->_Bmp.b_p)[j + offset] + x)), gpoutfile);
			}
		}
	}
	fputs("\r\033l", gpoutfile);
	fputc('\0', gpoutfile); // set left margin to 0 
	if(p_gp->_Bmp.b_planes > 1) {
		fputs("\033r", gpoutfile);
		fputc('\0', gpoutfile); // set color to black 
	}
#ifdef PC
	fputs("Print using: COPY /B\n", gpoutfile);
#endif
}

#endif /* NEC */

#ifdef STARC
/* The following Star color driver uses generic bit mapped graphics
   routines to build up a bit map in memory. */
/* Star Color changes made by William Wilson, wew@naucse.cse.nau.edu */
/* On PC, print using 'copy file /b lpt1:', do NOT use 'print' */
/* STARC_init changes gpoutfile to binary mode on PC's */

static void STARC_dump(GpTermEntry * pThis);

#define STARCXLAST (STARCXMAX - 1)
#define STARCYLAST (STARCYMAX - 1)

/* plane 0=black, 1=cyan(blue), 2=magenta(red), 3=yellow */
static uint STARCcolor[] = { 1, 8, 4, 2, 10, 12, 6, 14 };
static uint STARCpcolor[] = { 0, 2, 1, 4 };

TERM_PUBLIC void STARC_init(GpTermEntry * pThis)
{
}

TERM_PUBLIC void STARC_graphics(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	p_gp->BmpCharSize(FNT5X9);
	p_gp->BmpMakeBitmap((uint)(STARCXMAX * p_gp->V.Size.x), (uint)(STARCYMAX * p_gp->V.Size.y), 4);
}

TERM_PUBLIC void STARC_text(GpTermEntry * pThis)
{
	STARC_dump(pThis);
	pThis->P_Gp->BmpFreeBitmap();
}

TERM_PUBLIC void STARC_linetype(GpTermEntry * pThis, int linetype)
{
	if(linetype >= 6)
		linetype %= 6;
	b_setvalue(STARCcolor[linetype + 2]);
}
//
// output file must be binary mode for STARC_dump 
//
static void STARC_dump(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	for(int j = (p_gp->_Bmp.b_ysize / 8) - 1; j >= 0; j--) {
		fputs("\033J\030", gpoutfile); // line feed 8/72" = 8 dots 
		for(uint plane = 0; plane < p_gp->_Bmp.b_planes; plane++) {
			uint offset = plane * p_gp->_Bmp.b_psize;
			if(p_gp->_Bmp.b_planes > 1) {
				// select colour for plane 
				fputs("\033r", gpoutfile);
				fputc((char)STARCpcolor[plane], gpoutfile);
			}
			// select plotter graphics mode (square pixels) 
			fputs("\r\033*\005", gpoutfile);
			fputc((char)(p_gp->_Bmp.b_xsize % 256), gpoutfile);
			fputc((char)(p_gp->_Bmp.b_xsize / 256), gpoutfile);
			for(uint x = 0; x < p_gp->_Bmp.b_xsize; x++) {
				fputc((char)(*((*p_gp->_Bmp.b_p)[j + offset] + x)), gpoutfile);
			}
		}
	}
	if(p_gp->_Bmp.b_planes > 1) {
		fputs("\033r", gpoutfile);
		fputc('\0', gpoutfile); // set color to black 
	}
#ifdef PC
	fputs("Print using: COPY /B\n", stderr);
#endif
}

#endif /* STARC */

#ifdef EPS180

static void eps180_dump(GpTermEntry * pThis);

#define EPS180XLAST (EPS180XMAX - 1)
#define EPS180YLAST (EPS180YMAX - 1)

TERM_PUBLIC void EPS180_graphics(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	p_gp->BmpCharSize(FNT13X25);
	p_gp->BmpMakeBitmap((uint)(EPS180XMAX * p_gp->V.Size.x), (uint)(EPS180YMAX * p_gp->V.Size.y), 1);
}

TERM_PUBLIC void EPS180_text(GpTermEntry * pThis)
{
	eps180_dump(pThis);
	pThis->P_Gp->BmpFreeBitmap();
}
//
// output file must be binary mode for eps180_dump 
//
static void eps180_dump(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	// reset, set line spacing to 24/180", and move left margin 
	fputs("\033@\033+\060\033l\005", gpoutfile);
	for(int j = (p_gp->_Bmp.b_ysize / 8) - 1; j >= 0;) {
		// select printer graphics mode '39' 
		fputs("\r\n\033*'", gpoutfile);
		fputc((char)(p_gp->_Bmp.b_xsize % 256), gpoutfile);
		fputc((char)(p_gp->_Bmp.b_xsize / 256), gpoutfile);
		for(uint x = 0; x < p_gp->_Bmp.b_xsize; x++) {
			fputc((char)(*((*p_gp->_Bmp.b_p)[j] + x)), gpoutfile);
			fputc((char)(*((*p_gp->_Bmp.b_p)[j-1] + x)), gpoutfile);
			fputc((char)(*((*p_gp->_Bmp.b_p)[j - 2] + x)), gpoutfile);
		}
		j -= 3;
	}
	fputs("\r\n\033@\r\n", gpoutfile); // reset printer 
#ifdef PC
	fputs("Print using: COPY /B\n", stderr);
#endif
}

#endif /* EPS180 */

#ifdef EPS60

static void eps60_dump(GpTermEntry * pThis);

#define EPS60XLAST (EPS60XMAX - 1)
#define EPS60YLAST (EPS60YMAX - 1)

TERM_PUBLIC void EPS60_graphics(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	p_gp->BmpCharSize(FNT5X9);
	p_gp->BmpMakeBitmap((uint)(EPS60XMAX * p_gp->V.Size.x), (uint)(EPS60YMAX * p_gp->V.Size.y), 1);
}

TERM_PUBLIC void EPS60_text(GpTermEntry * pThis)
{
	eps60_dump(pThis);
	pThis->P_Gp->BmpFreeBitmap();
}
//
// output file must be binary mode for eps60_dump 
//
static void eps60_dump(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	fprintf(gpoutfile, "\033%c\030", '3');  /* set line spacing 24/216" = 8 dots */
	for(int j = (p_gp->_Bmp.b_ysize / 8) - 1; j >= 0; j--) {
		// select printer graphics mode 'K' 
		fputs("\r\n\033K", gpoutfile);
		fputc((char)(p_gp->_Bmp.b_xsize % 256), gpoutfile);
		fputc((char)(p_gp->_Bmp.b_xsize / 256), gpoutfile);
		for(uint x = 0; x < p_gp->_Bmp.b_xsize; x++) {
			fputc((char)(*((*p_gp->_Bmp.b_p)[j] + x)), gpoutfile);
		}
	}
	fprintf(gpoutfile, "\033%c\044\r\n", '3'); // set line spacing 36/216" = 1/6" 
#ifdef PC
	fputs("Print using: COPY /B\n", stderr);
#endif
}

#endif /* EPS60 */

#ifdef TANDY60

/* The only difference between TANDY60 and EPS60 is the inclusion
   of codes to swap the Tandy printer into IBM mode and back
   into Tandy mode.  For a Tandy already in IBM mode, use EPS60. */

TERM_PUBLIC void TANDY60_text(GpTermEntry * pThis)
{
#ifdef PC
	fputs("Inserting Tandy/IBM mode conversion codes\n", stderr);
#endif
	// Switch to IBM mode, and leave 3 inches above the plot so as
	// to get rough vertical centring on the page.  Perform the
	// centring by setting 1" line feeds and issuing 3 of them. 
	fprintf(gpoutfile, "\033!\033%c%c\n\n\n", '3', 216);
	eps60_dump(pThis);
	pThis->P_Gp->BmpFreeBitmap();
	// A form feed must be sent before switching back to Tandy mode,
	// or else the form setting will be messed up. 
	fputs("\f\033!", gpoutfile);
}

#endif /* TANDY60 */

#ifdef OKIDATA

static void okidata_dump(GpTermEntry * pThis);

TERM_PUBLIC void OKIDATA_text(GpTermEntry * pThis)
{
	okidata_dump(pThis);
	pThis->P_Gp->BmpFreeBitmap();
}

static int OKIDATAbitrev_tbl[] =
{
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};
//
// output file must be binary mode for okidata_dump 
//
static void okidata_dump(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	char cur_char;
	// set line spacing 16/144" = 8 dots, turn on single density graphics mode: 
	fprintf(gpoutfile, "\033%c%c\020\033*eP:\003", '%', '9');
	for(int j = (p_gp->_Bmp.b_ysize / 8) - 1; j >= 0; j--) {
		fputs("\003\016", gpoutfile);
		//fputc((char)(p_gp->_Bmp.b_xsize%256), gpoutfile); 
		//fputc((char)(p_gp->_Bmp.b_xsize/256), gpoutfile); 
		for(uint x = 0; x < p_gp->_Bmp.b_xsize; x++) {
			if((cur_char = (char)(OKIDATAbitrev_tbl[(int)(*((*p_gp->_Bmp.b_p)[j] + x))])) == '\003') {
				fputs("\003\003", gpoutfile);
			}
			else {
				fputc((char)cur_char, gpoutfile);
			}
		}
	}
	fprintf(gpoutfile, "\003\002\033%c%c\030\r\n", '%', '9'); // Turned off graphics mode: set line spacing 24/144" = 1/6" 
#ifdef PC
	fputs("Print using: COPY /B\n", stderr);
#endif
}

#endif /* OKIDATA */

#ifdef DPU414

static void DPU414_dump(GpTermEntry * pThis);

static int DPU414_font = 2; /* medium font */
static int DPU414_quality = 1; /* normal */

enum DPU414_id {
	DPU414_SMALL, DPU414_MEDIUM, DPU414_LARGE,
	DPU414_NORMAL, DPU414_DRAFT,
	DPU414_OTHER
};

static struct gen_table DPU414_opts[] =
{
	{ "s$mall", DPU414_SMALL },
	{ "m$edium", DPU414_MEDIUM },
	{ "l$arge", DPU414_LARGE },
	{ "n$ormal", DPU414_NORMAL },
	{ "d$raft", DPU414_DRAFT },
	{ NULL, DPU414_OTHER }
};

TERM_PUBLIC void DPU414_options(GpTermEntry * pThis, GnuPlot * pGp)
{
	DPU414_font = 2;
	DPU414_quality = 1;
	term_options[0] = NUL;
	while(!pGp->Pgm.EndOfCommand()) {
		switch(pGp->Pgm.LookupTableForCurrentToken(&DPU414_opts[0])) {
			case DPU414_SMALL:
			    DPU414_font = 1;
			    pGp->Pgm.Shift();
			    break;
			case DPU414_MEDIUM:
			    DPU414_font = 2;
			    pGp->Pgm.Shift();
			    break;
			case DPU414_LARGE:
			    DPU414_font = 3;
			    pGp->Pgm.Shift();
			    break;
			case DPU414_NORMAL:
			    DPU414_quality = 1;
			    pGp->Pgm.Shift();
			    break;
			case DPU414_DRAFT:
			    DPU414_quality = 2;
			    pGp->Pgm.Shift();
			    break;
			case DPU414_OTHER:
			default:
			    /* reset to default, since term is already set */
			    DPU414_font = 2;
			    DPU414_quality = 1;
			    pGp->IntErrorCurToken("expecting: {small, medium, large} {normal, draft}");
			    break;
		}
	}
	pThis->TicV = (pThis->MaxX < pThis->MaxY) ? pThis->MaxX/100 : pThis->MaxY/100;
	if(pThis->TicV < 1)
		pThis->TicV = 1;
	pThis->TicH = pThis->TicV;
	// setup options string 
	switch(DPU414_font) {
		case 1: strcat(term_options, "small"); break;
		case 2: strcat(term_options, "medium"); break;
		case 3: strcat(term_options, "large"); break;
	}
	switch(DPU414_quality) {
		case 1:
		    strcat(term_options, " normal");
		    pThis->MaxX = DPU414XMAX;
		    pThis->MaxY = DPU414YMAX;
		    break;
		case 2:
		    strcat(term_options, " draft");
		    pThis->MaxX = DPU414XMAX / 2;
		    pThis->MaxY = DPU414YMAX / 2;
		    break;
	}
}

TERM_PUBLIC void DPU414_init(GpTermEntry * pThis)
{
	DPU414_setfont(pThis);
}

TERM_PUBLIC void DPU414_setfont(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	switch(DPU414_font) {
		case 1:
		    p_gp->BmpCharSize(FNT5X9);
		    pThis->ChrV = FNT5X9_VCHAR;
		    pThis->ChrH = FNT5X9_HCHAR;
		    break;
		case 2:
		    p_gp->BmpCharSize(FNT9X17);
		    pThis->ChrV = FNT9X17_VCHAR;
		    pThis->ChrH = FNT9X17_HCHAR;
		    break;
		case 3:
		    p_gp->BmpCharSize(FNT13X25);
		    pThis->ChrV = FNT13X25_VCHAR;
		    pThis->ChrH = FNT13X25_HCHAR;
		    break;
	}
}

TERM_PUBLIC void DPU414_graphics(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	switch(DPU414_quality) {
		case 1:
		    p_gp->BmpMakeBitmap((uint)(DPU414XMAX * p_gp->V.Size.x), (uint)(DPU414YMAX * p_gp->V.Size.y), 1);
		    break;
		case 2:
		    pThis->MaxX = DPU414XMAX / 2;
		    pThis->MaxY = DPU414YMAX / 2;
		    p_gp->BmpMakeBitmap((uint)(DPU414XMAX / 2 * p_gp->V.Size.x), (uint)(DPU414YMAX / 2 * p_gp->V.Size.y), 1);
		    break;
	}
}

TERM_PUBLIC void DPU414_text(GpTermEntry * pThis)
{
	DPU414_dump(pThis);
	pThis->P_Gp->BmpFreeBitmap();
}

static void DPU414_dump(GpTermEntry * pThis)
{
	GnuPlot * p_gp = pThis->P_Gp;
	uint x;
	int j;
	fputs("\r", gpoutfile); //  carriage return 
	fprintf(gpoutfile, "\033%c", 'U'); // unidirectional printing 
	fputc((char)0, gpoutfile);
	switch(DPU414_quality) {
		case 1:
		    for(j = (p_gp->_Bmp.b_ysize / 8) - 1; j >= 0; j -= 2) {
			    // select 120-dpi, emulated 16-pin printer graphics mode 
			    // in reality it's 640/(89.6mm/25.4mm) = 181 dpi = appr. 180 dpi 
			    fputs("\033^\001", gpoutfile);
			    fputc((char)(p_gp->_Bmp.b_xsize % 256), gpoutfile);
			    fputc((char)(p_gp->_Bmp.b_xsize / 256), gpoutfile);
			    for(x = 0; x < p_gp->_Bmp.b_xsize; x++) {
				    fputc((char)(*((*p_gp->_Bmp.b_p)[j] + x)), gpoutfile);
				    fputc((char)(*((*p_gp->_Bmp.b_p)[j-1] + x)), gpoutfile);
			    }
			    fprintf(gpoutfile, "\033%c\x10\r", 'J');    /* advance 16 halfdots, carriage return */
		    }
		    break;
		case 2:
		    for(j = (p_gp->_Bmp.b_ysize / 8) - 1; j >= 0; j--) {
			    // select 60-dpi, 8-pin printer graphics mode 
			    // in reality it's 320/(89.6mm/25.4mm) = 91 dpi = appr. 90 dpi 
			    fputs("\033K", gpoutfile);
			    fputc((char)(p_gp->_Bmp.b_xsize % 256), gpoutfile);
			    fputc((char)(p_gp->_Bmp.b_xsize / 256), gpoutfile);
			    for(x = 0; x < p_gp->_Bmp.b_xsize; x++) {
				    fputc((char)(*((*p_gp->_Bmp.b_p)[j] + x)), gpoutfile);
			    }
			    fprintf(gpoutfile, "\033%c\x10\r", 'J');    /* advance 16 halfdots, carriage return */
		    }
		    break;
	}
	fputs("\r\n\033@\r\n", gpoutfile);      /* reset printer */
#ifdef PC
	fputs("Print using: COPY /B\n", stderr);
#endif
}

#endif /* DPU414 */
#endif /* TERM_BODY */

#ifdef TERM_TABLE

#ifdef EPS180
TERM_TABLE_START(epson180_driver)
	"epson_180dpi", 
	"Epson LQ-style 180-dot per inch (24 pin) printers",
	EPS180XMAX, 
	EPS180YMAX, 
	EPSON180VCHAR, 
	EPSON180HCHAR,
	EPSON180VTIC, 
	EPSON180HTIC, 
	GnuPlot::OptionsNull, 
	EPSON_init, 
	EPSON_reset,
	EPS180_text, 
	GnuPlot::NullScale, 
	EPS180_graphics, 
	b_move, 
	b_vector,
	b_setlinetype, 
	b_put_text, 
	b_text_angle,
	GnuPlot::NullJustifyText, 
	GnuPlot::DoPoint, 
	GnuPlot::DoArrow, 
	set_font_null, 
	0, 
	TERM_BINARY,
	0, 
	0, 
	b_boxfill 
TERM_TABLE_END(epson180_driver)

#undef LAST_TERM
#define LAST_TERM epson180_driver
#endif /* EPS180 */

#ifdef EPS60
TERM_TABLE_START(epson60_driver)
	"epson_60dpi", 
	"Epson-style 60-dot per inch printers",
	EPS60XMAX, 
	EPS60YMAX, 
	EPSONVCHAR, 
	EPSONHCHAR,
	EPSONVTIC, 
	EPSONHTIC, 
	GnuPlot::OptionsNull, 
	EPSON_init, 
	EPSON_reset,
	EPS60_text, 
	GnuPlot::NullScale, 
	EPS60_graphics, 
	b_move, 
	b_vector,
	b_setlinetype, 
	b_put_text, 
	b_text_angle,
	GnuPlot::NullJustifyText, 
	GnuPlot::DoPoint, 
	GnuPlot::DoArrow, 
	set_font_null, 
	0, 
	TERM_BINARY,
	0, 
	0, 
	b_boxfill 
TERM_TABLE_END(epson60_driver)

#undef LAST_TERM
#define LAST_TERM epson60_driver
#endif /* EPS60 */

#ifdef EPSONP
TERM_TABLE_START(epsonlx_driver)
	"epson_lx800", 
	"Epson LX-800, Star NL-10, NX-1000, PROPRINTER ...",
	EPSONXMAX, 
	EPSONYMAX, 
	EPSONVCHAR, 
	EPSONHCHAR,
	EPSONVTIC, 
	EPSONHTIC, 
	GnuPlot::OptionsNull, 
	EPSON_init, 
	EPSON_reset,
	EPSON_text, 
	GnuPlot::NullScale, 
	EPSON_graphics, 
	b_move, 
	b_vector,
	b_setlinetype, 
	b_put_text, 
	b_text_angle,
	GnuPlot::NullJustifyText, 
	GnuPlot::LineAndPoint, 
	GnuPlot::DoArrow, 
	set_font_null, 
	0, 
	TERM_BINARY,
	0, 
	0, 
	b_boxfill 
TERM_TABLE_END(epsonlx_driver)

#undef LAST_TERM
#define LAST_TERM epsonlx_driver
#endif /* EPSONP */

#ifdef NEC
TERM_TABLE_START(nec_driver)
	"nec_cp6", 
	"NEC printer CP6, Epson LQ-800 [monochrome color draft]",
	NECXMAX, 
	NECYMAX, 
	NECVCHAR, 
	NECHCHAR,
	NECVTIC, 
	NECHTIC, 
	NEC_options, 
	NEC_init, 
	EPSON_reset,
	NEC_text, 
	GnuPlot::NullScale, 
	NEC_graphics, 
	b_move, 
	b_vector,
	NEC_linetype, 
	b_put_text, 
	b_text_angle,
	GnuPlot::NullJustifyText, 
	GnuPlot::LineAndPoint, 
	GnuPlot::DoArrow, 
	set_font_null, 
	0, 
	TERM_BINARY,
	0,
	0, 
	b_boxfill 
TERM_TABLE_END(nec_driver)

#undef LAST_TERM
#define LAST_TERM nec_driver
#endif /* NEC */

#ifdef OKIDATA
TERM_TABLE_START(okidata_driver)
	"okidata", 
	"OKIDATA 320/321 Standard",
	EPS60XMAX, 
	EPS60YMAX, 
	EPSONVCHAR, 
	EPSONHCHAR,
	EPSONVTIC, 
	EPSONHTIC, 
	GnuPlot::OptionsNull, 
	EPSON_init, 
	EPSON_reset,
	OKIDATA_text, 
	GnuPlot::NullScale, 
	EPS60_graphics, 
	b_move, 
	b_vector,
	b_setlinetype, 
	b_put_text, 
	b_text_angle,
	GnuPlot::NullJustifyText, 
	GnuPlot::DoPoint, 
	GnuPlot::DoArrow, 
	set_font_null, 
	0, 
	TERM_BINARY,
	0, 
	0, 
	b_boxfill 
TERM_TABLE_END(okidata_driver)

#undef LAST_TERM
#define LAST_TERM okidata_driver
#endif /* OKIDATA */

#ifdef STARC
TERM_TABLE_START(starc_driver)
	"starc", 
	"Star Color Printer",
	STARCXMAX, 
	STARCYMAX, 
	STARCVCHAR, 
	STARCHCHAR,
	STARCVTIC, 
	STARCHTIC, 
	GnuPlot::OptionsNull, 
	STARC_init, 
	EPSON_reset,
	STARC_text, 
	GnuPlot::NullScale, 
	STARC_graphics, 
	b_move, 
	b_vector,
	STARC_linetype, 
	b_put_text, 
	b_text_angle,
	GnuPlot::NullJustifyText, 
	GnuPlot::LineAndPoint, 
	GnuPlot::DoArrow, 
	set_font_null, 
	0, 
	TERM_BINARY,
	0, 
	0, 
	b_boxfill 
TERM_TABLE_END(starc_driver)

#undef LAST_TERM
#define LAST_TERM starc_driver
#endif /* STARC */

#ifdef TANDY60
TERM_TABLE_START(tandy60_driver)
	"tandy_60dpi", 
	"Tandy DMP-130 series 60-dot per inch graphics",
	EPS60XMAX, 
	EPS60YMAX, 
	EPSONVCHAR, 
	EPSONHCHAR,
	EPSONVTIC, 
	EPSONHTIC, 
	GnuPlot::OptionsNull, 
	EPSON_init, 
	EPSON_reset,
	TANDY60_text, 
	GnuPlot::NullScale, 
	EPS60_graphics, 
	b_move, 
	b_vector,
	b_setlinetype, 
	b_put_text, 
	b_text_angle,
	GnuPlot::NullJustifyText, 
	GnuPlot::DoPoint, 
	GnuPlot::DoArrow, 
	set_font_null, 
	0, 
	TERM_BINARY,
	0, 
	0, 
	b_boxfill 
TERM_TABLE_END(tandy60_driver)

#undef LAST_TERM
	#define LAST_TERM tandy60_driver
#endif /* TANDY60 */

#ifdef DPU414
TERM_TABLE_START(dpu414_driver)
	"dpu414", 
	"Seiko DPU-414 thermal printer [small medium large]",
	DPU414XMAX, 
	DPU414YMAX, 
	DPU414VCHAR, 
	DPU414HCHAR,
	DPU414VTIC, 
	DPU414HTIC, 
	DPU414_options, 
	DPU414_init, 
	EPSON_reset,
	DPU414_text, 
	GnuPlot::NullScale, 
	DPU414_graphics, 
	b_move, 
	b_vector,
	b_setlinetype, 
	b_put_text, 
	b_text_angle,
	GnuPlot::NullJustifyText, 
	GnuPlot::LineAndPoint, 
	GnuPlot::DoArrow, 
	set_font_null, 
	0, 
	TERM_BINARY,
	0, 
	0, 
	b_boxfill 
TERM_TABLE_END(dpu414_driver)

#undef LAST_TERM
	#define LAST_TERM dpu414_driver
#endif /* DPU414 */

#endif /* TERM_TABLE */
#endif /* TERM_PROTO_ONLY */

#ifdef TERM_HELP
START_HELP(epson_180dpi)
"1 epson_180dpi",
"?commands set terminal epson_180dpi",
"?set terminal epson_180dpi",
"?set term epson_180dpi",
"?terminal epson_180dpi",
"?term epson_180dpi",
"?epson_180dpi",
"?commands set terminal epson_60dpi",
"?set terminal epson_60dpi",
"?set term epson_60dpi",
"?terminal epson_60dpi",
"?term epson_60dpi",
"?epson_60dpi",
"?commands set terminal epson_lx800",
"?set terminal epson_lx800",
"?set term epson_lx800",
"?terminal epson_lx800",
"?term epson_lx800",
"?epson_lx800",
"?commands set terminal nec_cp6",
"?set terminal nec_cp6",
"?set term nec_cp6",
"?terminal nec_cp6",
"?term nec_cp6",
"?nec_cp6",
"?commands set terminal okidata",
"?set terminal okidata",
"?set term okidata",
"?terminal okidata",
"?term okidata",
"?okidata",
"?commands set terminal starc",
"?set terminal starc",
"?set term starc",
"?terminal starc",
"?term starc",
"?starc",
"?commands set terminal tandy_60dpi",
"?set terminal tandy_60dpi",
"?set term tandy_60dpi",
"?terminal tandy_60dpi",
"?term tandy_60dpi",
"?tandy_60dpi",
"?commands set terminal dpu414",
"?set terminal dpu414",
"?set term dpu414",
"?terminal dpu414",
"?term dpu414",
"?dpu414",
" Note: only available if gnuplot is configured --with-bitmap-terminals.",
" This driver supports a family of Epson printers and derivatives.",
"",
" `epson_180dpi` and `epson_60dpi` are drivers for Epson LQ-style 24-pin",
" printers with resolutions of 180 and 60 dots per inch, respectively.",
"",
" `epson_lx800` is a generic 9-pin driver appropriate for printers like the",
" Epson LX-800, the Star NL-10 and NX-1000, the PROPRINTER, and so forth.",
"",
" `nec_cp6` is generic 24-pin driver that can be used for printers like the",
" NEC CP6 and the Epson LQ-800.",
"",
" The `okidata` driver supports the 9-pin OKIDATA 320/321 Standard printers.",
"",
" The `starc` driver is for the Star Color Printer.",
"",
" The `tandy_60dpi` driver is for the Tandy DMP-130 series of 9-pin, 60-dpi",
" printers.",
"",
" The `dpu414` driver is for the Seiko DPU-414 thermal printer.",
"",
" `nec_cp6` has the options:",
"",
" Syntax:",
"       set terminal nec_cp6 {monochrome | colour | draft}",
"",
" which defaults to monochrome.",
"",
" `dpu414` has the options:",
"",
" Syntax:",
"       set terminal dpu414 {small | medium | large} {normal | draft}",
"",
" which defaults to medium (=font size) and normal.",
" Preferred combinations are `medium normal` and `small draft`.",
#ifdef MSDOS
"",
" With each of these drivers, a binary copy is required on a PC to print.",
" Do not use `print`---use instead `copy file /b lpt1:`.",
#endif
""
END_HELP(epson_180dpi)
#endif /* TERM_HELP */
