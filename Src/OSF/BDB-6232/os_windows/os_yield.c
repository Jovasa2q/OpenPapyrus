/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
/*
 * __os_yield --
 *	Yield the processor, optionally pausing until running again.
 */
void __os_yield(ENV *env, u_long secs, u_long usecs/* Seconds and microseconds*/)
{
	COMPQUIET(env, NULL);
	/* Don't require the values be normalized. */
	for(; usecs >= US_PER_SEC; usecs -= US_PER_SEC)
		++secs;
	/*
	 * Yield the processor so other processes or threads can run.
	 *
	 * Sheer raving paranoia -- don't sleep for 0 time, in case some
	 * implementation doesn't yield the processor in that case.
	 */
	Sleep(secs * MS_PER_SEC + (usecs / US_PER_MS) + 1);
}
