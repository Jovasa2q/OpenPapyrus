/*
 * Copyright © 2010,2011  Google, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * Google Author(s): Behdad Esfahbod
 */
#include "harfbuzz-internal.h"
#pragma hdrstop

#ifdef HB_NO_OPEN
	#define hb_blob_create_from_file(x)  hb_blob_get_empty()
#endif

int main(int argc, char ** argv)
{
	if(argc != 4 && argc != 5) {
		slfprintf_stderr("usage: %s font-file lookup-index first-glyph [second-glyph]\n", argv[0]);
		exit(1);
	}
	/* Create the face */
	hb_blob_t * blob = hb_blob_create_from_file(argv[1]);
	hb_face_t * face = hb_face_create(blob, 0 /* first face */);
	hb_blob_destroy(blob);
	blob = nullptr;
	hb_font_t * font = hb_font_create(face);
#ifdef HAVE_FREETYPE
	hb_ft_font_set_funcs(font);
#endif
	uint len = argc - 3;
	hb_codepoint_t glyphs[2];
	if(!hb_font_glyph_from_string(font, argv[3], -1, &glyphs[0]) || (argc > 4 && !hb_font_glyph_from_string(font, argv[4], -1, &glyphs[1])))
		return 2;
	return !hb_ot_layout_lookup_would_substitute(face, strtol(argv[2], nullptr, 0), glyphs, len, false);
}
