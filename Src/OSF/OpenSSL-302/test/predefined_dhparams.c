/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include "testutil.h"
#include "predefined_dhparams.h"

#ifndef OPENSSL_NO_DH

static EVP_PKEY * get_dh_from_pg_bn(OSSL_LIB_CTX * libctx, const char * type, BIGNUM * p, BIGNUM * g, BIGNUM * q)
{
	EVP_PKEY_CTX * pctx = EVP_PKEY_CTX_new_from_name(libctx, type, NULL);
	OSSL_PARAM_BLD * tmpl = NULL;
	OSSL_PARAM * params = NULL;
	EVP_PKEY * dhpkey = NULL;
	if(pctx == NULL || EVP_PKEY_fromdata_init(pctx) <= 0)
		goto err;
	if((tmpl = OSSL_PARAM_BLD_new()) == NULL || !OSSL_PARAM_BLD_push_BN(tmpl, OSSL_PKEY_PARAM_FFC_P, p) || 
		!OSSL_PARAM_BLD_push_BN(tmpl, OSSL_PKEY_PARAM_FFC_G, g) || (q != NULL && !OSSL_PARAM_BLD_push_BN(tmpl, OSSL_PKEY_PARAM_FFC_Q, q)))
		goto err;
	params = OSSL_PARAM_BLD_to_param(tmpl);
	if(params == NULL || EVP_PKEY_fromdata(pctx, &dhpkey, EVP_PKEY_KEY_PARAMETERS, params) <= 0)
		goto err;
err:
	EVP_PKEY_CTX_free(pctx);
	OSSL_PARAM_free(params);
	OSSL_PARAM_BLD_free(tmpl);
	return dhpkey;
}

static EVP_PKEY * get_dh_from_pg(OSSL_LIB_CTX * libctx, const char * type, unsigned char * pdata, size_t plen,
    unsigned char * gdata, size_t glen, unsigned char * qdata, size_t qlen)
{
	EVP_PKEY * dhpkey = NULL;
	BIGNUM * q = NULL;
	BIGNUM * p = BN_bin2bn(pdata, plen, NULL);
	BIGNUM * g = BN_bin2bn(gdata, glen, NULL);
	if(p == NULL || g == NULL)
		goto err;
	if(qdata != NULL && (q = BN_bin2bn(qdata, qlen, NULL)) == NULL)
		goto err;
	dhpkey = get_dh_from_pg_bn(libctx, type, p, g, q);
err:
	BN_free(p);
	BN_free(g);
	BN_free(q);
	return dhpkey;
}

EVP_PKEY * get_dh512(OSSL_LIB_CTX * libctx)
{
	static unsigned char dh512_p[] = {
		0xCB, 0xC8, 0xE1, 0x86, 0xD0, 0x1F, 0x94, 0x17, 0xA6, 0x99, 0xF0, 0xC6,
		0x1F, 0x0D, 0xAC, 0xB6, 0x25, 0x3E, 0x06, 0x39, 0xCA, 0x72, 0x04, 0xB0,
		0x6E, 0xDA, 0xC0, 0x61, 0xE6, 0x7A, 0x77, 0x25, 0xE8, 0x3B, 0xB9, 0x5F,
		0x9A, 0xB6, 0xB5, 0xFE, 0x99, 0x0B, 0xA1, 0x93, 0x4E, 0x35, 0x33, 0xB8,
		0xE1, 0xF1, 0x13, 0x4F, 0x59, 0x1A, 0xD2, 0x57, 0xC0, 0x26, 0x21, 0x33,
		0x02, 0xC5, 0xAE, 0x23,
	};
	static unsigned char dh512_g[] = {
		0x02,
	};
	return get_dh_from_pg(libctx, "DH", dh512_p, sizeof(dh512_p), dh512_g, sizeof(dh512_g), NULL, 0);
}

EVP_PKEY * get_dhx512(OSSL_LIB_CTX * libctx)
{
	static unsigned char dhx512_p[] = {
		0x00, 0xe8, 0x1a, 0xb7, 0x9a, 0x02, 0x65, 0x64, 0x94, 0x7b, 0xba, 0x09,
		0x1c, 0x12, 0x27, 0x1e, 0xea, 0x89, 0x32, 0x64, 0x78, 0xf8, 0x1c, 0x78,
		0x8e, 0x96, 0xc3, 0xc6, 0x9f, 0x41, 0x05, 0x41, 0x65, 0xae, 0xe3, 0x05,
		0xea, 0x66, 0x21, 0xf7, 0x38, 0xb7, 0x2b, 0x32, 0x40, 0x5a, 0x14, 0x86,
		0x51, 0x94, 0xb1, 0xcf, 0x01, 0xe3, 0x27, 0x28, 0xf6, 0x75, 0xa3, 0x15,
		0xbb, 0x12, 0x4d, 0x99, 0xe7,
	};
	static unsigned char dhx512_g[] = {
		0x00, 0x91, 0xc1, 0x43, 0x6d, 0x0d, 0xb0, 0xa4, 0xde, 0x41, 0xb7, 0x93,
		0xad, 0x51, 0x94, 0x1b, 0x43, 0xd8, 0x42, 0xf1, 0x5e, 0x46, 0x83, 0x5d,
		0xf1, 0xd1, 0xf0, 0x41, 0x10, 0xd1, 0x1c, 0x5e, 0xad, 0x9b, 0x68, 0xb1,
		0x6f, 0xf5, 0x8e, 0xaa, 0x6d, 0x71, 0x88, 0x37, 0xdf, 0x05, 0xf7, 0x6e,
		0x7a, 0xb4, 0x25, 0x10, 0x6c, 0x7f, 0x38, 0xb4, 0xc8, 0xfc, 0xcc, 0x0c,
		0x6a, 0x02, 0x08, 0x61, 0xf6,
	};
	static unsigned char dhx512_q[] = {
		0x00, 0xdd, 0xf6, 0x35, 0xad, 0xfa, 0x70, 0xc7, 0xe7, 0xa8, 0xf0, 0xe3,
		0xda, 0x79, 0x34, 0x3f, 0x5b, 0xcf, 0x73, 0x82, 0x91,
	};
	return get_dh_from_pg(libctx, "X9.42 DH", dhx512_p, sizeof(dhx512_p), dhx512_g, sizeof(dhx512_g), dhx512_q, sizeof(dhx512_q));
}

EVP_PKEY * get_dh1024dsa(OSSL_LIB_CTX * libctx)
{
	static unsigned char dh1024_p[] = {
		0xC8, 0x00, 0xF7, 0x08, 0x07, 0x89, 0x4D, 0x90, 0x53, 0xF3, 0xD5, 0x00,
		0x21, 0x1B, 0xF7, 0x31, 0xA6, 0xA2, 0xDA, 0x23, 0x9A, 0xC7, 0x87, 0x19,
		0x3B, 0x47, 0xB6, 0x8C, 0x04, 0x6F, 0xFF, 0xC6, 0x9B, 0xB8, 0x65, 0xD2,
		0xC2, 0x5F, 0x31, 0x83, 0x4A, 0xA7, 0x5F, 0x2F, 0x88, 0x38, 0xB6, 0x55,
		0xCF, 0xD9, 0x87, 0x6D, 0x6F, 0x9F, 0xDA, 0xAC, 0xA6, 0x48, 0xAF, 0xFC,
		0x33, 0x84, 0x37, 0x5B, 0x82, 0x4A, 0x31, 0x5D, 0xE7, 0xBD, 0x52, 0x97,
		0xA1, 0x77, 0xBF, 0x10, 0x9E, 0x37, 0xEA, 0x64, 0xFA, 0xCA, 0x28, 0x8D,
		0x9D, 0x3B, 0xD2, 0x6E, 0x09, 0x5C, 0x68, 0xC7, 0x45, 0x90, 0xFD, 0xBB,
		0x70, 0xC9, 0x3A, 0xBB, 0xDF, 0xD4, 0x21, 0x0F, 0xC4, 0x6A, 0x3C, 0xF6,
		0x61, 0xCF, 0x3F, 0xD6, 0x13, 0xF1, 0x5F, 0xBC, 0xCF, 0xBC, 0x26, 0x9E,
		0xBC, 0x0B, 0xBD, 0xAB, 0x5D, 0xC9, 0x54, 0x39,
	};
	static unsigned char dh1024_g[] = {
		0x3B, 0x40, 0x86, 0xE7, 0xF3, 0x6C, 0xDE, 0x67, 0x1C, 0xCC, 0x80, 0x05,
		0x5A, 0xDF, 0xFE, 0xBD, 0x20, 0x27, 0x74, 0x6C, 0x24, 0xC9, 0x03, 0xF3,
		0xE1, 0x8D, 0xC3, 0x7D, 0x98, 0x27, 0x40, 0x08, 0xB8, 0x8C, 0x6A, 0xE9,
		0xBB, 0x1A, 0x3A, 0xD6, 0x86, 0x83, 0x5E, 0x72, 0x41, 0xCE, 0x85, 0x3C,
		0xD2, 0xB3, 0xFC, 0x13, 0xCE, 0x37, 0x81, 0x9E, 0x4C, 0x1C, 0x7B, 0x65,
		0xD3, 0xE6, 0xA6, 0x00, 0xF5, 0x5A, 0x95, 0x43, 0x5E, 0x81, 0xCF, 0x60,
		0xA2, 0x23, 0xFC, 0x36, 0xA7, 0x5D, 0x7A, 0x4C, 0x06, 0x91, 0x6E, 0xF6,
		0x57, 0xEE, 0x36, 0xCB, 0x06, 0xEA, 0xF5, 0x3D, 0x95, 0x49, 0xCB, 0xA7,
		0xDD, 0x81, 0xDF, 0x80, 0x09, 0x4A, 0x97, 0x4D, 0xA8, 0x22, 0x72, 0xA1,
		0x7F, 0xC4, 0x70, 0x56, 0x70, 0xE8, 0x20, 0x10, 0x18, 0x8F, 0x2E, 0x60,
		0x07, 0xE7, 0x68, 0x1A, 0x82, 0x5D, 0x32, 0xA2,
	};
	return get_dh_from_pg(libctx, "DH", dh1024_p, sizeof(dh1024_p), dh1024_g, sizeof(dh1024_g), NULL, 0);
}

EVP_PKEY * get_dh2048(OSSL_LIB_CTX * libctx)
{
	BIGNUM * p = NULL;
	EVP_PKEY * dhpkey = NULL;
	BIGNUM * g = BN_new();
	if(g == NULL || !BN_set_word(g, 2))
		goto err;
	p = BN_get_rfc3526_prime_2048(NULL);
	if(p == NULL)
		goto err;
	dhpkey = get_dh_from_pg_bn(libctx, "DH", p, g, NULL);
err:
	BN_free(p);
	BN_free(g);
	return dhpkey;
}

EVP_PKEY * get_dh4096(OSSL_LIB_CTX * libctx)
{
	BIGNUM * p = NULL;
	EVP_PKEY * dhpkey = NULL;
	BIGNUM * g = BN_new();
	if(g == NULL || !BN_set_word(g, 2))
		goto err;
	p = BN_get_rfc3526_prime_4096(NULL);
	if(p == NULL)
		goto err;
	dhpkey = get_dh_from_pg_bn(libctx, "DH", p, g, NULL);
err:
	BN_free(p);
	BN_free(g);
	return dhpkey;
}

#endif
