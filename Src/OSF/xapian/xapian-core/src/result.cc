/** @file
 * @brief A result inside an MSet
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

string Result::get_description() const
{
	string desc = "Result(";
	desc += str(did);
	desc += ", ";
	desc += str(weight);
	if(!sort_key.empty()) {
		desc += ", sort_key=";
		description_append(desc, sort_key);
	}
	if(!collapse_key.empty()) {
		desc += ", collapse_key=";
		description_append(desc, collapse_key);
	}
	if(collapse_count) {
		desc += ", collapse_count=";
		desc += str(collapse_count);
	}
	desc += ')';
	return desc;
}
