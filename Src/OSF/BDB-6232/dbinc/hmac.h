/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#ifndef	_DB_HMAC_H_
#define	_DB_HMAC_H_

#if defined(__cplusplus)
extern "C" {
#endif

#define	HMAC_OUTPUT_SIZE	20
#define	HMAC_BLOCK_SIZE	64

/*
 * Algorithm specific information.
 */
/*
 * SHA1 checksumming
 */
typedef struct {
	uint32	state[5];
	uint32	count[2];
	unsigned char	buffer[64];
} SHA1_CTX;

/*
 * AES assumes the SHA1 checksumming (also called MAC)
 */
#define	DB_MAC_MAGIC	"mac derivation key magic value"
#define	DB_ENC_MAGIC	"encryption and decryption key value magic"

#if defined(__cplusplus)
}
#endif

#include "dbinc_auto/hmac_ext.h"
#endif /* !_DB_HMAC_H_ */
