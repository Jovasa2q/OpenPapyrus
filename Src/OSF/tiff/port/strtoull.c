/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *  must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *  may be used to endorse or promote products derived from this software
 *  without specific prior written permission.
 */
#include "tiffiop.h"
#pragma hdrstop
/*
 * Convert a string to an uint64 integer.
 *
 * Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
uint64 strtoull(const char * nptr, char ** endptr, int base)
{
	uint64 acc;
	char c;
	uint64 cutoff;
	int neg, any, cutlim;
	/*
	 * See strtoq for comments as to the logic used.
	 */
	const char * s = nptr;
	do {
		c = *s++;
	} while(isspace((uchar)c));
	if(c == '-') {
		neg = 1;
		c = *s++;
	}
	else {
		neg = 0;
		if(c == '+')
			c = *s++;
	}
	if((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X') &&
	    ((s[1] >= '0' && s[1] <= '9') || (s[1] >= 'A' && s[1] <= 'F') || (s[1] >= 'a' && s[1] <= 'f'))) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if(base == 0)
		base = c == '0' ? 8 : 10;
	acc = any = 0;
	if(base < 2 || base > 36)
		goto noconv;

	cutoff = ULLONG_MAX / base;
	cutlim = ULLONG_MAX % base;
	for(;; c = *s++) {
		if(c >= '0' && c <= '9')
			c -= '0';
		else if(c >= 'A' && c <= 'Z')
			c -= 'A' - 10;
		else if(c >= 'a' && c <= 'z')
			c -= 'a' - 10;
		else
			break;
		if(c >= base)
			break;
		if(any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if(any < 0) {
		acc = ULLONG_MAX;
		errno = ERANGE;
	}
	else if(!any) {
noconv:
		errno = EINVAL;
	}
	else if(neg)
		acc = -acc;
	if(endptr != NULL)
		*endptr = (char *)(any ? s - 1 : nptr);
	return (acc);
}

