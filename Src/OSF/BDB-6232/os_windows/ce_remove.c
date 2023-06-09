/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2012, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
/*
 * remove implementation on WinCE.
 *
 * PUBLIC: #ifdef DB_WINCE
 * PUBLIC: int __ce_remove __P((const char *path));
 * PUBLIC: #endif
 */
int __ce_remove(const char * path)
{
	return __os_unlink(NULL, path, 0);
}
