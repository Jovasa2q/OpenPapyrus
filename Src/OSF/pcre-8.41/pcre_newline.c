//
// Perl-Compatible Regular Expressions
//
// PCRE is a library of functions to support regular expressions whose syntax
// and semantics are as close as possible to those of the Perl 5 language.
// Written by Philip Hazel Copyright (c) 1997-2016 University of Cambridge
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// -- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// -- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// -- Neither the name of the University of Cambridge nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
/* This module contains internal functions for testing newlines when more than
   one kind of newline is to be recognized. When a newline is found, its length is
   returned. In principle, we could implement several newline "types", each
   referring to a different set of newline characters. At present, PCRE supports
   only NLTYPE_FIXED, which gets handled without these functions, NLTYPE_ANYCRLF,
   and NLTYPE_ANY. The full list of Unicode newline characters is taken from
   http://unicode.org/unicode/reports/tr18/. */
#include "pcre_internal.h"
#pragma hdrstop

/*************************************************
*      Check for newline at given position       *
*************************************************/

/* It is guaranteed that the initial value of ptr is less than the end of the
   string that is being processed.

   Arguments:
   ptr          pointer to possible newline
   type         the newline type
   endptr       pointer to the end of the string
   lenptr       where to return the length
   utf          TRUE if in utf mode

   Returns:       TRUE or FALSE
 */

BOOL PRIV(is_newline) (PCRE_PUCHAR ptr, int type, PCRE_PUCHAR endptr, int * lenptr, BOOL utf)
{
	uint32 c;
	(void)utf;
#ifdef SUPPORT_UTF
	if(utf) {
		GETCHAR(c, ptr);
	}
	else
#endif  /* SUPPORT_UTF */
	c = *ptr;
	/* Note that this function is called only for ANY or ANYCRLF. */
	if(type == NLTYPE_ANYCRLF) 
		switch(c) {
			case CHAR_LF: *lenptr = 1; return TRUE;
			case CHAR_CR: *lenptr = (ptr < endptr - 1 && ptr[1] == CHAR_LF) ? 2 : 1;
			    return TRUE;
			default: return FALSE;
		}
/* NLTYPE_ANY */
	else 
		switch(c) {
#ifdef EBCDIC
			case CHAR_NEL:
#endif
			case CHAR_LF:
			case CHAR_VT:
			case CHAR_FF: *lenptr = 1; return TRUE;

			case CHAR_CR:
			    *lenptr = (ptr < endptr - 1 && ptr[1] == CHAR_LF) ? 2 : 1;
			    return TRUE;

#ifndef EBCDIC
#ifdef COMPILE_PCRE8
			case CHAR_NEL: *lenptr = utf ? 2 : 1; return TRUE;
			case 0x2028:                 /* LS */
			case 0x2029: *lenptr = 3; return TRUE; /* PS */
#else /* COMPILE_PCRE16 || COMPILE_PCRE32 */
			case CHAR_NEL:
			case 0x2028:                 /* LS */
			case 0x2029: *lenptr = 1; return TRUE; /* PS */
#endif  /* COMPILE_PCRE8 */
#endif  /* Not EBCDIC */

			default: return FALSE;
		}
}

/*************************************************
*     Check for newline at previous position     *
*************************************************/

/* It is guaranteed that the initial value of ptr is greater than the start of
   the string that is being processed.

   Arguments:
   ptr          pointer to possible newline
   type         the newline type
   startptr     pointer to the start of the string
   lenptr       where to return the length
   utf          TRUE if in utf mode

   Returns:       TRUE or FALSE
 */
BOOL PRIV(was_newline) (PCRE_PUCHAR ptr, int type, PCRE_PUCHAR startptr, int * lenptr, BOOL utf)
{
	uint32 c;
	(void)utf;
	ptr--;
#ifdef SUPPORT_UTF
	if(utf) {
		BACKCHAR(ptr);
		GETCHAR(c, ptr);
	}
	else
#endif  /* SUPPORT_UTF */
	c = *ptr;
/* Note that this function is called only for ANY or ANYCRLF. */
	if(type == NLTYPE_ANYCRLF) switch(c)
		{
			case CHAR_LF:
			    *lenptr = (ptr > startptr && ptr[-1] == CHAR_CR) ? 2 : 1;
			    return TRUE;

			case CHAR_CR: *lenptr = 1; return TRUE;
			default: return FALSE;
		}

/* NLTYPE_ANY */

	else switch(c)
		{
			case CHAR_LF:
			    *lenptr = (ptr > startptr && ptr[-1] == CHAR_CR) ? 2 : 1;
			    return TRUE;

#ifdef EBCDIC
			case CHAR_NEL:
#endif
			case CHAR_VT:
			case CHAR_FF:
			case CHAR_CR: *lenptr = 1; return TRUE;

#ifndef EBCDIC
#ifdef COMPILE_PCRE8
			case CHAR_NEL: *lenptr = utf ? 2 : 1; return TRUE;
			case 0x2028:                 /* LS */
			case 0x2029: *lenptr = 3; return TRUE; /* PS */
#else /* COMPILE_PCRE16 || COMPILE_PCRE32 */
			case CHAR_NEL:
			case 0x2028:                 /* LS */
			case 0x2029: *lenptr = 1; return TRUE; /* PS */
#endif  /* COMPILE_PCRE8 */
#endif  /* NotEBCDIC */

			default: return FALSE;
		}
}

/* End of pcre_newline.c */
