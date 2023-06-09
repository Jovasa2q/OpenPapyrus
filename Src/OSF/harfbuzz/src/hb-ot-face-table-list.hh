/*
 * Copyright © 2007,2008,2009  Red Hat, Inc.
 * Copyright © 2012,2013  Google, Inc.
 * Copyright © 2019, Facebook Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * Red Hat Author(s): Behdad Esfahbod
 * Google Author(s): Behdad Esfahbod
 * Facebook Author(s): Behdad Esfahbod
 */
#ifndef HB_OT_FACE_TABLE_LIST_HH
#define HB_OT_FACE_TABLE_LIST_HH
#endif /* HB_OT_FACE_TABLE_LIST_HH */ /* Dummy header guards */

#ifndef HB_OT_ACCELERATOR
#define HB_OT_ACCELERATOR(Namespace, Type) HB_OT_TABLE(Namespace, Type)
#define _HB_OT_ACCELERATOR_UNDEF
#endif

/* This lists font tables that the hb_face_t will contain and lazily
 * load.  Don't add a table unless it's used though.  This is not
 * exactly free. */

/* v--- Add new tables in the right place here. */

/* OpenType fundamentals. */
HB_OT_TABLE(OT, head)
#if !defined(HB_NO_FACE_COLLECT_UNICODES) || !defined(HB_NO_OT_FONT)
HB_OT_ACCELERATOR(OT, cmap)
#endif
HB_OT_TABLE(OT, hhea)
HB_OT_ACCELERATOR(OT, hmtx)
HB_OT_TABLE(OT, OS2)
#if !defined(HB_NO_OT_FONT_GLYPH_NAMES) || !defined(HB_NO_METRICS) || !defined(HB_NO_STYLE)
HB_OT_ACCELERATOR(OT, post)
#endif
#ifndef HB_NO_NAME
HB_OT_ACCELERATOR(OT, name)
#endif
#ifndef HB_NO_STYLE
HB_OT_TABLE(OT, STAT)
#endif
#ifndef HB_NO_META
HB_OT_ACCELERATOR(OT, meta)
#endif

/* Vertical layout. */
HB_OT_TABLE(OT, vhea)
HB_OT_ACCELERATOR(OT, vmtx)

/* TrueType outlines. */
HB_OT_ACCELERATOR(OT, glyf)

/* CFF outlines. */
#ifndef HB_NO_CFF
HB_OT_ACCELERATOR(OT, cff1)
HB_OT_ACCELERATOR(OT, cff2)
HB_OT_TABLE(OT, VORG)
#endif

/* OpenType variations. */
#ifndef HB_NO_VAR
HB_OT_TABLE(OT, fvar)
HB_OT_TABLE(OT, avar)
HB_OT_ACCELERATOR(OT, gvar)
HB_OT_TABLE(OT, MVAR)
#endif

/* Legacy kern. */
#ifndef HB_NO_OT_KERN
HB_OT_TABLE(OT, kern)
#endif

/* OpenType shaping. */
#ifndef HB_NO_OT_LAYOUT
HB_OT_ACCELERATOR(OT, GDEF)
HB_OT_ACCELERATOR(OT, GSUB)
HB_OT_ACCELERATOR(OT, GPOS)
//HB_OT_TABLE (OT, JSTF)
#endif

/* OpenType baseline. */
#ifndef HB_NO_BASE
HB_OT_TABLE(OT, BASE)
#endif

/* AAT shaping. */
#ifndef HB_NO_AAT
HB_OT_TABLE(AAT, morx)
HB_OT_TABLE(AAT, mort)
HB_OT_TABLE(AAT, kerx)
HB_OT_TABLE(AAT, ankr)
HB_OT_TABLE(AAT, trak)
HB_OT_TABLE(AAT, ltag)
HB_OT_TABLE(AAT, feat)
// HB_OT_TABLE (AAT, opbd)
#endif

/* OpenType color fonts. */
#ifndef HB_NO_COLOR
HB_OT_TABLE(OT, COLR)
HB_OT_TABLE(OT, CPAL)
HB_OT_ACCELERATOR(OT, CBDT)
HB_OT_ACCELERATOR(OT, sbix)
HB_OT_ACCELERATOR(OT, SVG)
#endif

/* OpenType math. */
#ifndef HB_NO_MATH
HB_OT_TABLE(OT, MATH)
#endif

#ifdef _HB_OT_ACCELERATOR_UNDEF
#undef HB_OT_ACCELERATOR
#endif
