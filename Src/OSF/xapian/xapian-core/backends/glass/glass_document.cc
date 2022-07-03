/** @file
 * @brief A document read from a GlassDatabase.
 */
/* Copyright 2017 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */
#include <xapian-internal.h>
#pragma hdrstop

using namespace std;

string GlassDocument::fetch_value(Xapian::valueno slot) const
{
	return value_manager->get_value(did, slot);
}

void GlassDocument::fetch_all_values(map<Xapian::valueno, string>& values_) const
{
	value_manager->get_all_values(values_, did);
}

string GlassDocument::fetch_data() const
{
	return docdata_table->get_document_data(did);
}
