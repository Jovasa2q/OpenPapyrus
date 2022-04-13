/** @file
 * @brief Implementation of closefrom() function.
 */
/* Copyright (C) 2010,2011 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef XAPIAN_INCLUDED_CLOSEFROM_H
#define XAPIAN_INCLUDED_CLOSEFROM_H

#ifndef PACKAGE
	#error config.h must be included first in each C++ source file
#endif
// We don't currently need closefrom() on __WIN32__.
#ifndef __WIN32__
#ifdef HAVE_CLOSEFROM
	// NB: not <cstdlib> as Sun's C++ compiler omits non-standard functions there,
	// and closefrom() isn't standardised.
	#include <stdlib.h>
#else
	namespace Xapian {
		namespace Internal {
			void closefrom(int fd);
		}
	}

	using ::Xapian::Internal::closefrom;
#endif
#endif
#endif // XAPIAN_INCLUDED_CLOSEFROM_H
