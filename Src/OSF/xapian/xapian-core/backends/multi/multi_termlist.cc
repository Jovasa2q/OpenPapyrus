/** @file
 * @brief Adapter class for a TermList in a multidatabase
 */
/* Copyright (C) 2007,2008,2009,2011,2017 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <xapian-internal.h>
#pragma hdrstop

using namespace std;

MultiTermList::MultiTermList(const Xapian::Database::Internal* db_, TermList* real_termlist_) : real_termlist(real_termlist_), db(db_)
{
}

MultiTermList::~MultiTermList()
{
	delete real_termlist;
}

Xapian::termcount MultiTermList::get_approx_size() const
{
	return real_termlist->get_approx_size();
}

string MultiTermList::get_termname() const
{
	return real_termlist->get_termname();
}

Xapian::termcount MultiTermList::get_wdf() const
{
	return real_termlist->get_wdf();
}

Xapian::doccount MultiTermList::get_termfreq() const
{
	Xapian::doccount result;
	db->get_freqs(real_termlist->get_termname(), &result, NULL);
	return result;
}

TermList * MultiTermList::next()
{
	return real_termlist->next();
}

TermList * MultiTermList::skip_to(const std::string &term)
{
	return real_termlist->skip_to(term);
}

bool MultiTermList::at_end() const
{
	return real_termlist->at_end();
}

Xapian::termcount MultiTermList::positionlist_count() const
{
	return real_termlist->positionlist_count();
}

PositionList* MultiTermList::positionlist_begin() const
{
	return real_termlist->positionlist_begin();
}
