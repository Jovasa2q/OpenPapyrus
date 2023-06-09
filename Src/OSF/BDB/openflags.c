/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2011 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
/*
 * __db_openflags --
 *	Convert open(2) flags to DB flags.
 *
 * PUBLIC: uint32 __db_openflags(int);
 */
uint32 __db_openflags(int oflags)
{
	uint32 dbflags = 0;
	if(oflags&O_CREAT)
		dbflags |= DB_CREATE;
	if(oflags&O_TRUNC)
		dbflags |= DB_TRUNCATE;
	/*
	 * !!!
	 * Convert POSIX 1003.1 open(2) mode flags to DB flags.  This isn't
	 * an exact science as few POSIX implementations have a flag value
	 * for O_RDONLY, it's simply the lack of a write flag.
	 */
#ifndef O_ACCMODE
 #define O_ACCMODE       (O_RDONLY|O_RDWR|O_WRONLY)
#endif
	switch(oflags&O_ACCMODE) {
	    case O_RDWR:
	    case O_WRONLY:
		break;
	    default:
		dbflags |= DB_RDONLY;
		break;
	}
	return dbflags;
}
