// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/********************************************************************
* COPYRIGHT:
* Copyright (c) 1997-2001, International Business Machines Corporation and
* others. All Rights Reserved.
********************************************************************/
/********************************************************************************
 *
 * File CBKITTST.C
 *
 * Modification History:
 *        Name                      Description
 *     Madhu Katragadda               Creation
 *********************************************************************************
 */
#include <icu-internal.h>
#pragma hdrstop
#include "unicode/utypes.h"

#if !UCONFIG_NO_BREAK_ITERATION

#include "cintltst.h"

void addBrkIterAPITest(TestNode**);

void addBreakIter(TestNode** root)
{
	addBrkIterAPITest(root);
}

#endif /* #if !UCONFIG_NO_BREAK_ITERATION */
