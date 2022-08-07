/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include <internal/openssl-crypto-internal.h>
#pragma hdrstop
#include <openssl/cterr.h>
#include "crypto/cterr.h"

#ifndef OPENSSL_NO_CT

#ifndef OPENSSL_NO_ERR

static const ERR_STRING_DATA CT_str_reasons[] = {
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_BASE64_DECODE_ERROR), "base64 decode error"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_INVALID_LOG_ID_LENGTH), "invalid log id length"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_LOG_CONF_INVALID), "log conf invalid"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_LOG_CONF_INVALID_KEY), "log conf invalid key"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_LOG_CONF_MISSING_DESCRIPTION), "log conf missing description"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_LOG_CONF_MISSING_KEY), "log conf missing key"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_LOG_KEY_INVALID), "log key invalid"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_FUTURE_TIMESTAMP), "sct future timestamp"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_INVALID), "sct invalid"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_INVALID_SIGNATURE), "sct invalid signature"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_LIST_INVALID), "sct list invalid"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_LOG_ID_MISMATCH), "sct log id mismatch"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_NOT_SET), "sct not set"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_UNSUPPORTED_VERSION), "sct unsupported version"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_UNRECOGNIZED_SIGNATURE_NID), "unrecognized signature nid"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_UNSUPPORTED_ENTRY_TYPE), "unsupported entry type"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_UNSUPPORTED_VERSION), "unsupported version"},
	{0, NULL}
};

#endif

int ossl_err_load_CT_strings(void)
{
#ifndef OPENSSL_NO_ERR
	if(ERR_reason_error_string(CT_str_reasons[0].error) == NULL)
		ERR_load_strings_const(CT_str_reasons);
#endif
	return 1;
}

#else
NON_EMPTY_TRANSLATION_UNIT
#endif
