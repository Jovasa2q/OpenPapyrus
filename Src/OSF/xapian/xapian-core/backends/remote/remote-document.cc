/** @file
 * @brief A document read from a RemoteDatabase.
 */
/* Copyright (C) 2008 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */
#include <xapian-internal.h>
#pragma hdrstop
#include "remote-document.h"

using namespace std;

string RemoteDocument::fetch_value(Xapian::valueno) const
{
	LOGCALL(DB, string, "RemoteDocument::fetch_value", Literal("[slot]"));
	// Our ctor sets the values, so we should never get here.
	Assert(false);
	RETURN(string());
}

void RemoteDocument::fetch_all_values(map<Xapian::valueno, string> &) const
{
	LOGCALL_VOID(DB, "RemoteDocument::fetch_all_values", Literal("[&values_]"));
	// Our ctor sets the values, so we should never get here.
	Assert(false);
}

string RemoteDocument::fetch_data() const
{
	LOGCALL(DB, string, "RemoteDocument::fetch_data", NO_ARGS);
	// Our ctor sets the data, so we should never get here.
	Assert(false);
	RETURN(string());
}
