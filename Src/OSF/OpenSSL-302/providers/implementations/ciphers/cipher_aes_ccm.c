/*
 * Copyright 2019-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include <internal/openssl-crypto-internal.h>
#pragma hdrstop
/*
 * AES low level APIs are deprecated for public use, but still ok for internal
 * use where we're using them to implement the higher level EVP interface, as is
 * the case here.
 */
#include "internal/deprecated.h"

/* Dispatch functions for AES CCM mode */

#include "cipher_aes_ccm.h"
#include "prov/implementations.h"
#include "prov/providercommon.h"

static void * aes_ccm_newctx(void * provctx, size_t keybits)
{
	PROV_AES_CCM_CTX * ctx;
	if(!ossl_prov_is_running())
		return NULL;
	ctx = (PROV_AES_CCM_CTX *)OPENSSL_zalloc(sizeof(*ctx));
	if(ctx)
		ossl_ccm_initctx(&ctx->base, keybits, ossl_prov_aes_hw_ccm(keybits));
	return ctx;
}

static OSSL_FUNC_cipher_freectx_fn aes_ccm_freectx;
static void aes_ccm_freectx(void * vctx)
{
	PROV_AES_CCM_CTX * ctx = (PROV_AES_CCM_CTX*)vctx;
	OPENSSL_clear_free(ctx,  sizeof(*ctx));
}

IMPLEMENT_aead_cipher(aes, ccm, CCM, AEAD_FLAGS, 128, 8, 96); /* ossl_aes128ccm_functions */
IMPLEMENT_aead_cipher(aes, ccm, CCM, AEAD_FLAGS, 192, 8, 96); /* ossl_aes192ccm_functions */
IMPLEMENT_aead_cipher(aes, ccm, CCM, AEAD_FLAGS, 256, 8, 96); /* ossl_aes256ccm_functions */
