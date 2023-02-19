// iso8859_6.c -  Oniguruma (regular expression library)
//
/*-
 * Copyright (c) 2002-2019  K.Kosako All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 */
#include "regint.h"
#pragma hdrstop

#define ENC_IS_ISO_8859_6_CTYPE(code, ctype) ((EncISO_8859_6_CtypeTable[code] & CTYPE_TO_BIT(ctype)) != 0)

static const ushort EncISO_8859_6_CtypeTable[256] = {
	0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008,
	0x4008, 0x420c, 0x4209, 0x4208, 0x4208, 0x4208, 0x4008, 0x4008,
	0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008,
	0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008,
	0x4284, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0,
	0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0,
	0x78b0, 0x78b0, 0x78b0, 0x78b0, 0x78b0, 0x78b0, 0x78b0, 0x78b0,
	0x78b0, 0x78b0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0,
	0x41a0, 0x7ca2, 0x7ca2, 0x7ca2, 0x7ca2, 0x7ca2, 0x7ca2, 0x74a2,
	0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2,
	0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2,
	0x74a2, 0x74a2, 0x74a2, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x51a0,
	0x41a0, 0x78e2, 0x78e2, 0x78e2, 0x78e2, 0x78e2, 0x78e2, 0x70e2,
	0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2,
	0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2,
	0x70e2, 0x70e2, 0x70e2, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x4008,
	0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008,
	0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008,
	0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008,
	0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008,
	0x0284, 0x0000, 0x0000, 0x0000, 0x00a0, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x01a0, 0x01a0, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x01a0, 0x0000, 0x0000, 0x0000, 0x01a0,
	0x0000, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2,
	0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2,
	0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2,
	0x30a2, 0x30a2, 0x30a2, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2,
	0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2, 0x30a2,
	0x30a2, 0x30a2, 0x30a2, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

static int is_code_ctype(OnigCodePoint code, uint ctype) { return (code < 256) ? ENC_IS_ISO_8859_6_CTYPE(code, ctype) : FALSE; }

OnigEncodingType OnigEncodingISO_8859_6 = {
	onigenc_single_byte_mbc_enc_len,
	"ISO-8859-6", /* name */
	1,       /* max enc length */
	1,       /* min enc length */
	onigenc_is_mbc_newline_0x0a,
	onigenc_single_byte_mbc_to_code,
	onigenc_single_byte_code_to_mbclen,
	onigenc_single_byte_code_to_mbc,
	onigenc_ascii_mbc_case_fold,
	onigenc_ascii_apply_all_case_fold,
	onigenc_ascii_get_case_fold_codes_by_str,
	onigenc_minimum_property_name_to_ctype,
	is_code_ctype,
	onigenc_not_support_get_ctype_code_range,
	onigenc_single_byte_left_adjust_char_head,
	onigenc_always_true_is_allowed_reverse_match,
	NULL, /* init */
	NULL, /* is_initialized */
	onigenc_always_true_is_valid_mbc_string,
	ENC_FLAG_ASCII_COMPATIBLE|ENC_FLAG_SKIP_OFFSET_1,
	0, 0
};
