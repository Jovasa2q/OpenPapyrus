/** @file
 *  @brief File and path manipulation routines.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2008,2009,2010,2012 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */
#ifndef XAPIAN_INCLUDED_FILEUTILS_H
#define XAPIAN_INCLUDED_FILEUTILS_H

//#include <string>

/** Remove a directory, and its contents.
 *
 *  If dirname doesn't refer to a file or directory, no error is generated.
 *
 *  Note - this doesn't currently cope with directories which contain
 *  subdirectories.
 */
void removedir(const std::string &dirname);

/** Resolve @a path relative to @a base.
 *
 *  Return @a path qualified to work as if you did "chdir(<directory which base
 *  is in>)" first.
 */
void resolve_relative_path(std::string & path, const std::string & base);

#endif /* XAPIAN_INCLUDED_FILEUTILS_H */
