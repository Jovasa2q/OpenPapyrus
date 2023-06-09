/* Copyright (C) 1998-1999, 2001, 2005-2007, 2009-2011 Free Software
   Foundation, Inc.
   This file is derived from the one in the GNU C Library.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <flexbison_common.h>
#pragma hdrstop

#if !_LIBC
	#define __gen_tempname gen_tempname
	#ifndef __GT_FILE
		#define __GT_FILE GT_FILE
	#endif
#endif

#ifndef __GT_FILE
	#define __GT_FILE 0
#endif

/* Generate a unique temporary file name from XTEMPLATE.
   The last six characters of XTEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the file name unique.
   Then open the file and return a fd. */
int mkstemp(char * xtemplate)
{
	return __gen_tempname(xtemplate, 0, 0, __GT_FILE);
}
