/* $Header: /cvs/maptools/cvsroot/libtiff/libtiff/tif_version.c,v 1.3 2010-03-10 18:56:49 bfriesen Exp $ */
/*
 * Copyright (c) 1992-1997 Sam Leffler
 * Copyright (c) 1992-1997 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 */
#include "tiffiop.h"
#pragma hdrstop

static const char TIFFVersion[] = TIFFLIB_VERSION_STR;

const char * TIFFGetVersion(void)
{
	return (TIFFVersion);
}
