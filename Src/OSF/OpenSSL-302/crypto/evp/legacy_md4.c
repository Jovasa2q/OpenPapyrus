/*
 * Copyright 2015-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include <internal/openssl-crypto-internal.h>
#pragma hdrstop
/*
 * MD4 low level APIs are deprecated for public use, but still ok for internal use.
 */
#include "crypto/evp.h"
#include "legacy_meth.h"

IMPLEMENT_LEGACY_EVP_MD_METH(MD4_CTX, md4, MD4)

static const EVP_MD md4_md = {
    NID_md4,
    NID_md4WithRSAEncryption,
    MD4_DIGEST_LENGTH,
    0,
    EVP_ORIG_GLOBAL,
    LEGACY_EVP_MD_METH_TABLE(md4_init, md4_update, md4_final, NULL, MD4_CBLOCK),
};

const EVP_MD *EVP_md4(void)
{
    return &md4_md;
}
