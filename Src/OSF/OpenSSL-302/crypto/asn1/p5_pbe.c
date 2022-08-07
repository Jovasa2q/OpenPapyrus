/*
 * Copyright 1999-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include <internal/openssl-crypto-internal.h>
#pragma hdrstop

/* PKCS#5 password based encryption structure */

ASN1_SEQUENCE(PBEPARAM) = {
	ASN1_SIMPLE(PBEPARAM, salt, ASN1_OCTET_STRING),
	ASN1_SIMPLE(PBEPARAM, iter, ASN1_INTEGER)
} ASN1_SEQUENCE_END(PBEPARAM)

IMPLEMENT_ASN1_FUNCTIONS(PBEPARAM)

/* Set an algorithm identifier for a PKCS#5 PBE algorithm */

int PKCS5_pbe_set0_algor_ex(X509_ALGOR * algor, int alg, int iter, const uchar * salt, int saltlen, OSSL_LIB_CTX * ctx)
{
	ASN1_STRING * pbe_str = NULL;
	uchar * sstr = NULL;
	PBEPARAM * pbe = PBEPARAM_new();
	if(pbe == NULL) {
		ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
		goto err;
	}
	if(iter <= 0)
		iter = PKCS5_DEFAULT_ITER;
	if(!ASN1_INTEGER_set(pbe->iter, iter)) {
		ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
		goto err;
	}
	if(!saltlen)
		saltlen = PKCS5_SALT_LEN;
	if(saltlen < 0)
		goto err;
	sstr = (uchar *)OPENSSL_malloc(saltlen);
	if(sstr == NULL) {
		ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
		goto err;
	}
	if(salt)
		memcpy(sstr, salt, saltlen);
	else if(RAND_bytes_ex(ctx, sstr, saltlen, 0) <= 0)
		goto err;
	ASN1_STRING_set0(pbe->salt, sstr, saltlen);
	sstr = NULL;
	if(!ASN1_item_pack(pbe, ASN1_ITEM_rptr(PBEPARAM), &pbe_str)) {
		ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
		goto err;
	}
	PBEPARAM_free(pbe);
	pbe = NULL;
	if(X509_ALGOR_set0(algor, OBJ_nid2obj(alg), V_ASN1_SEQUENCE, pbe_str))
		return 1;
err:
	OPENSSL_free(sstr);
	PBEPARAM_free(pbe);
	ASN1_STRING_free(pbe_str);
	return 0;
}

int PKCS5_pbe_set0_algor(X509_ALGOR * algor, int alg, int iter, const uchar * salt, int saltlen)
{
	return PKCS5_pbe_set0_algor_ex(algor, alg, iter, salt, saltlen, NULL);
}

/* Return an algorithm identifier for a PKCS#5 PBE algorithm */
X509_ALGOR * PKCS5_pbe_set_ex(int alg, int iter, const uchar * salt, int saltlen, OSSL_LIB_CTX * ctx)
{
	X509_ALGOR * ret = X509_ALGOR_new();
	if(!ret) {
		ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	if(PKCS5_pbe_set0_algor_ex(ret, alg, iter, salt, saltlen, ctx))
		return ret;
	X509_ALGOR_free(ret);
	return NULL;
}

X509_ALGOR * PKCS5_pbe_set(int alg, int iter, const uchar * salt, int saltlen)
{
	return PKCS5_pbe_set_ex(alg, iter, salt, saltlen, NULL);
}
