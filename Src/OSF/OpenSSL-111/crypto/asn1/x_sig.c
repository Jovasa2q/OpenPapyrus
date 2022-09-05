/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include "internal/cryptlib.h"
#pragma hdrstop
#include <x509_int.h>

ASN1_SEQUENCE(X509_SIG) = {
	ASN1_SIMPLE(X509_SIG, algor, X509_ALGOR),
	ASN1_SIMPLE(X509_SIG, digest, ASN1_OCTET_STRING)
} ASN1_SEQUENCE_END(X509_SIG)

IMPLEMENT_ASN1_FUNCTIONS(X509_SIG)

void X509_SIG_get0(const X509_SIG * sig, const X509_ALGOR ** palg, const ASN1_OCTET_STRING ** pdigest)
{
	ASSIGN_PTR(palg, sig->algor);
	ASSIGN_PTR(pdigest, sig->digest);
}

void X509_SIG_getm(X509_SIG * sig, X509_ALGOR ** palg, ASN1_OCTET_STRING ** pdigest)
{
	ASSIGN_PTR(palg, sig->algor);
	ASSIGN_PTR(pdigest, sig->digest);
}
