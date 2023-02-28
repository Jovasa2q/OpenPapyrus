/**
 * @file char_table.h
 * A simple table to help tokenize stuff.
 * Used to parse strings (paired char) and words.
 *
 * @author  Ben Gardner
 * @license GPL v2+
 */

#ifndef CHAR_TABLE_H_INCLUDED
#define CHAR_TABLE_H_INCLUDED

#include "base_types.h"
#include <cstddef>

/**
 * bit0-7 = paired char
 * bit8 = OK for keyword 1st char
 * bit9 = OK for keyword 2+ char
 */
struct CharTable {
	static size_t chars[128];

	enum {
		KW1 = 0x0100,
		KW2 = 0x0200,
	};

	static inline size_t Get(size_t ch)
	{
		if(ch < SIZEOFARRAY(chars)) {
			return (chars[ch]);
		}
		/*
		 * HACK: If the top bit is set, then we are likely dealing with UTF-8,
		 * and since that is only allowed in identifiers, then assume that is
		 * what this is. This only prevents corruption, it does not properly
		 * handle UTF-8 because the byte length and screen size are assumed to be
		 * the same.
		 */
		return (KW1 | KW2);
	}
	static inline bool IsKw1(size_t ch) { return ((Get(ch) & KW1) != 0); }
	static inline bool IsKw2(size_t ch) { return ((Get(ch) & KW2) != 0); }
};

#if 0 // @sobolev (moved to uncrustify.cpp) {
	#ifdef DEFINE_CHAR_TABLE
	size_t CharTable::chars[128] = {
		0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, // [........]
		0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, // [........]
		0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, // [........]
		0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, // [........]
		0x000, 0x000, 0x022, 0x000, 0x300, 0x000, 0x000, 0x027, // [ !"#$%&']
		0x029, 0x028, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, // [()*+,-./]
		0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, // [01234567]
		0x200, 0x200, 0x000, 0x000, 0x03e, 0x000, 0x03c, 0x000, // [89:;<=>?]
		0x200, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, // [@ABCDEFG]
		0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, // [HIJKLMNO]
		0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, // [PQRSTUVW]
		0x300, 0x300, 0x300, 0x05d, 0x000, 0x05b, 0x000, 0x300, // [XYZ[\]^_]
		0x060, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, // [`abcdefg]
		0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, // [hijklmno]
		0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, 0x300, // [pqrstuvw]
		0x300, 0x300, 0x300, 0x07d, 0x000, 0x07b, 0x000, 0x000, // [xyz{|}~.]
	};
	#endif /* DEFINE_CHAR_TABLE */
#endif // } @sobolev
#endif /* CHAR_TABLE_H_INCLUDED */
