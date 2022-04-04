/** @file
 * @brief Glass class for value streams.
 */
/* Copyright (C) 2007,2008,2009,2011 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */
#ifndef XAPIAN_INCLUDED_GLASS_VALUELIST_H
#define XAPIAN_INCLUDED_GLASS_VALUELIST_H

#include "backends/valuelist.h"
#include "glass_values.h"

class GlassCursor;
class GlassDatabase;

/// Glass class for value streams.
class GlassValueList : public Xapian::ValueIterator::Internal {
	/// Don't allow assignment.
	void operator = (const GlassValueList &);
	/// Don't allow copying.
	GlassValueList(const GlassValueList &);
	GlassCursor * cursor;
	Glass::ValueChunkReader reader;
	Xapian::valueno slot;
	Xapian::Internal::intrusive_ptr<const GlassDatabase> db;
	/// Update @a reader to use the chunk currently pointed to by @a cursor.
	bool update_reader();
public:
	GlassValueList(Xapian::valueno slot_, Xapian::Internal::intrusive_ptr<const GlassDatabase> db_) : cursor(NULL), slot(slot_), db(db_) 
	{
	}
	~GlassValueList();
	Xapian::docid get_docid() const;
	Xapian::valueno get_valueno() const;
	std::string get_value() const;
	bool at_end() const;
	void next();
	void skip_to(Xapian::docid);
	bool check(Xapian::docid did);
	std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_GLASS_VALUELIST_H
