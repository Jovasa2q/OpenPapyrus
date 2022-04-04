/** @file
 * @brief BACKEND_* constants
 */
/* Copyright (C) 2015,2017,2018 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef XAPIAN_INCLUDED_BACKENDS_H
#define XAPIAN_INCLUDED_BACKENDS_H

enum {
	BACKEND_OLD = -2,
	BACKEND_UNKNOWN = -1,
	BACKEND_REMOTE = 0,
	BACKEND_INMEMORY = 1,
	BACKEND_GLASS = 2,
	BACKEND_HONEY = 3,
	BACKEND_MAX_
};

inline const char * backend_name(int code)
{
	if(code < 0 || code > BACKEND_MAX_) 
		code = BACKEND_MAX_;
	const char * p =
	    "remote\0\0\0"
	    "inmemory\0"
	    "glass\0\0\0\0"
	    "honey\0\0\0\0"
	    "?";
	return p + code * 9;
}

#endif
