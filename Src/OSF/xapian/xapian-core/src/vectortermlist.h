/** @file
 * @brief A vector-like container of terms which can be iterated.
 */
/* Copyright (C) 2011,2012,2017 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */
#ifndef XAPIAN_INCLUDED_VECTORTERMLIST_H
#define XAPIAN_INCLUDED_VECTORTERMLIST_H

/** This class stores a list of terms.
 *
 *  To be memory efficient, we store the terms in a single string using a
 *  suitable simple encoding.  This way the number of bytes needed will
 *  usually be the sum of the lengths of all the terms plus the number of
 *  terms.  If we used std::vector <std::string> here like we used to, that
 *  would need something like an additional 30 bytes per term (30 calculated
 *  for GCC 4.x on x86_64).
 */
class VectorTermList : public TermList {
	std::string data; /// The encoded terms.
	const char * p; /// Pointer to the next term's data, or NULL if we are at end.
	Xapian::termcount num_terms; /// The number of terms in the list.
	std::string current_term; /// The current term.
public:
	template <typename I> VectorTermList(I begin, I end) : num_terms(0)
	{
		// First calculate how much space we'll need so we can reserve it.
		size_t total_size = 0;
		for(I i = begin; i != end; ++i) {
			++num_terms;
			const std::string & s = *i;
			total_size += s.size() + 1;
			if(s.size() >= 128) {
				// Not a common case, so just assume the worst case rather than
				// trying to carefully calculate the exact size.
				total_size += 5;
			}
		}
		data.reserve(total_size);
		// Now encode all the terms into data.
		for(I i = begin; i != end; ++i) {
			pack_string(data, *i);
		}
		p = data.data();
	}
	Xapian::termcount get_approx_size() const;
	std::string get_termname() const;
	Xapian::termcount get_wdf() const;
	Xapian::doccount get_termfreq() const;
	TermList * next();
	TermList * skip_to(const std::string &);
	bool at_end() const;
	Xapian::termcount positionlist_count() const;
	PositionList* positionlist_begin() const;
};

#endif // XAPIAN_INCLUDED_VECTORTERMLIST_H
