/** @file
 * @brief Mechanism for accessing a struct of constant information
 */
/* Copyright (C) 2013,2015 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */
#include <xapian-internal.h>
#pragma hdrstop
#include "languages/sbl-dispatch.h"

static const struct Xapian::Internal::constinfo const_info = {
#include "unicode/c_istab.h"
	XAPIAN_MAJOR_VERSION,
	XAPIAN_MINOR_VERSION,
	XAPIAN_REVISION,
	XAPIAN_VERSION,
	CONST_STRLEN(LANGSTRING),
	LANGSTRING
};

namespace Xapian {
	namespace Internal {
		const struct constinfo* get_constinfo_() noexcept { return &const_info; }
	}
}
