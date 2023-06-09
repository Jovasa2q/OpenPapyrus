/** @file
 * @brief TermGenerator class internals
 */
// Copyright (C) 2007,2012,2016 Olly Betts
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */
#ifndef XAPIAN_INCLUDED_TERMGENERATOR_INTERNAL_H
#define XAPIAN_INCLUDED_TERMGENERATOR_INTERNAL_H

#include "xapian/intrusive_ptr.h"
#include <xapian/database.h>
#include <xapian/document.h>
#include <xapian/termgenerator.h>
#include <xapian/queryparser.h> // For Xapian::Stopper
#include <xapian/stem.h>

namespace Xapian {
class Stopper;

class TermGenerator::Internal : public Xapian::Internal::intrusive_base {
	friend class TermGenerator;
	Stem stemmer;
	stem_strategy strategy;
	Xapian::Internal::opt_intrusive_ptr<const Stopper> stopper;
	stop_strategy stop_mode;
	Document doc;
	termpos cur_pos;
	TermGenerator::flags flags;
	uint   max_word_length;
	WritableDatabase db;
public:
	Internal() : strategy(STEM_SOME), stopper(NULL), stop_mode(STOP_STEMMED), cur_pos(0), flags(TermGenerator::flags(0)), max_word_length(64) 
	{
	}
	void index_text(Utf8Iterator itor, termcount weight, const std::string & prefix, bool with_positions);
};
}

#endif // XAPIAN_INCLUDED_TERMGENERATOR_INTERNAL_H
