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
 * The stub functions for signal handling.
 * WinCE does not support signal handling, so we just define stub functions to
 * avoid linkage errors for utilities build.
 */
void __db_util_siginit()
{
	return;
}

int __db_util_interrupted()
{
	return (0);
}

void __db_util_sigresend()
{
	return;
}
