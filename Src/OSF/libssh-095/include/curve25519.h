/*
 * This file is part of the SSH Library
 *
 * Copyright (c) 2013 by Aris Adamantiadis <aris@badcode.be>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation,
 * version 2.1 of the License.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#ifndef CURVE25519_H_
#define CURVE25519_H_

#ifdef WITH_NACL
	#include <nacl/crypto_scalarmult_curve25519.h>

	#define CURVE25519_PUBKEY_SIZE crypto_scalarmult_curve25519_BYTES
	#define CURVE25519_PRIVKEY_SIZE crypto_scalarmult_curve25519_SCALARBYTES
	#define crypto_scalarmult_base crypto_scalarmult_curve25519_base
	#define crypto_scalarmult crypto_scalarmult_curve25519
#else
	#define CURVE25519_PUBKEY_SIZE 32
	#define CURVE25519_PRIVKEY_SIZE 32
	int crypto_scalarmult_base(uchar * q, const uchar * n);
	int crypto_scalarmult(uchar * q, const uchar * n, const uchar * p);
#endif /* WITH_NACL */
#ifdef HAVE_ECC
	#define HAVE_CURVE25519 1
#endif
typedef uchar ssh_curve25519_pubkey[CURVE25519_PUBKEY_SIZE];
typedef uchar ssh_curve25519_privkey[CURVE25519_PRIVKEY_SIZE];

int ssh_client_curve25519_init(ssh_session session);
#ifdef WITH_SERVER
	void ssh_server_curve25519_init(ssh_session session);
#endif /* WITH_SERVER */

#endif /* CURVE25519_H_ */
