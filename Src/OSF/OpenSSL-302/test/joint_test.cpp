﻿// JOINT_TEST.CPP
//
#define STATIC_LEGACY
#include "internal/deprecated.h"
#include "testutil.h"
#include <openssl/async.h>
#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <openssl/kdf.h>
#include "internal/packet.h"
#include "crypto/rsa.h"
#include "crypto/sm2.h"
#include "crypto/poly1305.h"
#include "internal/sm3.h"
#include "crypto/sm4.h"
#include "internal/crypto/rsa_local.h"
#include "crypto/sparse_array.h"
#include <openssl/rc4.h>
#include <openssl/md5.h>
#include "crypto/chacha.h"
#include "internal/constant_time.h"
#include "internal/numbers.h"
#include "internal/crypto/bn_prime.h"
#include "crypto/bn.h"
#include "../ssl/ssl_local.h"
#include "internal/property.h"
#include "internal/crypto/property_local.h"
#include "internal/namemap.h"
#include "internal/crypto/tbl_standard.h"
#include "crypto/asn1.h"
#include "internal/crypto/standard_methods.h"
#include "crypto/asn1_dsa.h"
#include <openssl/rand.h>
#include <openssl/asn1t.h>
#include <openssl/obj_mac.h>
#include <openssl/decoder.h>
#include <openssl/encoder.h>
#include <openssl/store.h>
#ifdef __TANDEM
	#include <strings.h> // strcasecmp
#endif
#include "bn_rand_range.h"
#include <openssl/mdc2.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
//#include "apps_ui.h"
//#include <openssl/ui.h>
#ifndef OPENSSL_NO_BF
	#include <openssl/blowfish.h>
#endif
#ifndef OPENSSL_NO_CAST
	#include <openssl/cast.h>
#endif
#ifndef OPENSSL_NO_DES
	#include <openssl/des.h>
#endif
#ifndef OPENSSL_NO_IDEA
	#include <openssl/idea.h>
#endif
#ifndef OPENSSL_NO_RC2
	#include <openssl/rc2.h>
#endif
#ifndef OPENSSL_NO_RC4
	#include <openssl/rc4.h>
#endif
#ifndef OPENSSL_NO_RC5
	#include <openssl/rc5.h>
#endif
#ifndef OPENSSL_NO_DH
	#include <openssl/dh.h>
	#include "crypto/bn_dh.h"
	#include "crypto/dh.h"
#endif
#if defined(OPENSSL_NO_DES) && !defined(OPENSSL_NO_MDC2)
	#define OPENSSL_NO_MDC2
#endif
#ifdef CHARSET_EBCDIC
	#include <openssl/ebcdic.h>
#endif
#include "cmp_testlib.h"
#ifdef OPENSSL_SYS_WINDOWS
	#define strcasecmp _stricmp
#endif

DEFINE_STACK_OF(BIGNUM)
//
// AESGCM
//
static const uchar TestData_AesGcm_gcm_key[] = {
	0xee, 0xbc, 0x1f, 0x57, 0x48, 0x7f, 0x51, 0x92, 0x1c, 0x04, 0x65, 0x66,
	0x5f, 0x8a, 0xe6, 0xd1, 0x65, 0x8b, 0xb2, 0x6d, 0xe6, 0xf8, 0xa0, 0x69,
	0xa3, 0x52, 0x02, 0x93, 0xa5, 0x72, 0x07, 0x8f
};

static const uchar TestData_AesGcm_gcm_iv[] = {
	0x99, 0xaa, 0x3e, 0x68, 0xed, 0x81, 0x73, 0xa0, 0xee, 0xd0, 0x66, 0x84
};

static const uchar TestData_AesGcm_gcm_pt[] = {
	0xf5, 0x6e, 0x87, 0x05, 0x5b, 0xc3, 0x2d, 0x0e, 0xeb, 0x31, 0xb2, 0xea,
	0xcc, 0x2b, 0xf2, 0xa5
};

static const uchar TestData_AesGcm_gcm_aad[] = {
	0x4d, 0x23, 0xc3, 0xce, 0xc3, 0x34, 0xb4, 0x9b, 0xdb, 0x37, 0x0c, 0x43,
	0x7f, 0xec, 0x78, 0xde
};

static const uchar TestData_AesGcm_gcm_ct[] = {
	0xf7, 0x26, 0x44, 0x13, 0xa8, 0x4c, 0x0e, 0x7c, 0xd5, 0x36, 0x86, 0x7e,
	0xb9, 0xf2, 0x17, 0x36
};

static const uchar TestData_AesGcm_gcm_tag[] = {
	0x67, 0xba, 0x05, 0x10, 0x26, 0x2a, 0xe4, 0x87, 0xd7, 0x37, 0xee, 0x62,
	0x98, 0xf7, 0x7e, 0x0c
};

class TestInnerBlock_AesGcm {
public:
	static int do_encrypt(uchar * iv_gen, uchar * ct, int * ct_len, uchar * tag, int * tag_len)
	{
		int ret = 0;
		EVP_CIPHER_CTX * ctx = NULL;
		int outlen;
		uchar outbuf[64];
		*tag_len = 16;
		ret = TEST_ptr(ctx = EVP_CIPHER_CTX_new()) && TEST_true(EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) > 0)
			&& TEST_true(EVP_EncryptInit_ex(ctx, NULL, NULL, TestData_AesGcm_gcm_key, iv_gen ? NULL : TestData_AesGcm_gcm_iv) > 0)
			&& TEST_true(EVP_EncryptUpdate(ctx, NULL, &outlen, TestData_AesGcm_gcm_aad, sizeof(TestData_AesGcm_gcm_aad)) > 0)
			&& TEST_true(EVP_EncryptUpdate(ctx, ct, ct_len, TestData_AesGcm_gcm_pt, sizeof(TestData_AesGcm_gcm_pt)) > 0)
			&& TEST_true(EVP_EncryptFinal_ex(ctx, outbuf, &outlen) > 0)
			&& TEST_int_eq(EVP_CIPHER_CTX_get_tag_length(ctx), 16)
			&& TEST_true(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, 16, tag) > 0)
			&& TEST_true(iv_gen == NULL || EVP_CIPHER_CTX_get_original_iv(ctx, iv_gen, 12));
		EVP_CIPHER_CTX_free(ctx);
		return ret;
	}
	static int do_decrypt(const uchar * iv, const uchar * ct, int ct_len, const uchar * tag, int tag_len)
	{
		int ret = 0;
		EVP_CIPHER_CTX * ctx = NULL;
		int outlen, ptlen;
		uchar pt[32];
		uchar outbuf[32];
		ret = TEST_ptr(ctx = EVP_CIPHER_CTX_new())
			&& TEST_true(EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) > 0)
			&& TEST_true(EVP_DecryptInit_ex(ctx, NULL, NULL, TestData_AesGcm_gcm_key, iv) > 0)
			&& TEST_int_eq(EVP_CIPHER_CTX_get_tag_length(ctx), 16)
			&& TEST_true(EVP_DecryptUpdate(ctx, NULL, &outlen, TestData_AesGcm_gcm_aad, sizeof(TestData_AesGcm_gcm_aad)) > 0)
			&& TEST_true(EVP_DecryptUpdate(ctx, pt, &ptlen, ct, ct_len) > 0)
			&& TEST_true(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, tag_len, (void*)tag) > 0)
			&& TEST_true(EVP_DecryptFinal_ex(ctx, outbuf, &outlen) > 0)
			&& TEST_mem_eq(TestData_AesGcm_gcm_pt, sizeof(TestData_AesGcm_gcm_pt), pt, ptlen);
		EVP_CIPHER_CTX_free(ctx);
		return ret;
	}
	static int kat_test()
	{
		uchar tag[32];
		uchar ct[32];
		int ctlen = 0, taglen = 0;
		return do_encrypt(NULL, ct, &ctlen, tag, &taglen) && TEST_mem_eq(TestData_AesGcm_gcm_ct, sizeof(TestData_AesGcm_gcm_ct), ct, ctlen)
			   && TEST_mem_eq(TestData_AesGcm_gcm_tag, sizeof(TestData_AesGcm_gcm_tag), tag, taglen) && do_decrypt(TestData_AesGcm_gcm_iv, ct, ctlen, tag, taglen);
	}
	static int badkeylen_test()
	{
		EVP_CIPHER_CTX * ctx = NULL;
		const EVP_CIPHER * cipher;
		int ret = TEST_ptr(cipher = EVP_aes_192_gcm()) && TEST_ptr(ctx = EVP_CIPHER_CTX_new())
			&& TEST_true(EVP_EncryptInit_ex(ctx, cipher, NULL, NULL, NULL)) && TEST_false(EVP_CIPHER_CTX_set_key_length(ctx, 2));
		EVP_CIPHER_CTX_free(ctx);
		return ret;
	}
	#ifdef FIPS_MODULE
	static int ivgen_test()
	{
		uchar iv_gen[16];
		uchar tag[32];
		uchar ct[32];
		int ctlen = 0, taglen = 0;
		return do_encrypt(iv_gen, ct, &ctlen, tag, &taglen) && do_decrypt(iv_gen, ct, ctlen, tag, taglen);
	}
	#endif
};
//
// Bad DTLS
//
// Unit test for Cisco DTLS1_BAD_VER session resume, as used by
// AnyConnect VPN protocol.
//
// This is designed to exercise the code paths in
// http://git.infradead.org/users/dwmw2/openconnect.git/blob/HEAD:/dtls.c
// which have frequently been affected by regressions in DTLS1_BAD_VER support.
//
// Note that unlike other SSL tests, we don't test against our own SSL
// server method. Firstly because we don't have one; we *only* support
// DTLS1_BAD_VER as a client. And secondly because even if that were
// fixed up it's the wrong thing to test against - because if changes
// are made in generic DTLS code which don't take DTLS1_BAD_VER into
// account, there's plenty of scope for making those changes such that
// they break *both* the client and the server in the same way.
//
// So we handle the server side manually. In a session resume there isn't
// much to be done anyway.
//
//
#define MAC_OFFSET (DTLS1_RT_HEADER_LENGTH + DTLS1_HM_HEADER_LENGTH) // For DTLS1_BAD_VER packets the MAC doesn't include the handshake header 

#define NODROP(x) { x ## UL, 0 }
#define DROP(x)   { x ## UL, 1 }

static struct {
	uint64_t seq;
	int drop;
} TestData_BadDTLS_tests[] = {
	NODROP(1), NODROP(3), NODROP(2),
	NODROP(0x1234), NODROP(0x1230), NODROP(0x1235),
	NODROP(0xffff), NODROP(0x10001), NODROP(0xfffe), NODROP(0x10000),
	DROP(0x10001), DROP(0xff), NODROP(0x100000), NODROP(0x800000), NODROP(0x7fffe1),
	NODROP(0xffffff), NODROP(0x1000000), NODROP(0xfffffe), DROP(0xffffff), NODROP(0x1000010),
	NODROP(0xfffffd), NODROP(0x1000011), DROP(0x12), NODROP(0x1000012),
	NODROP(0x1ffffff), NODROP(0x2000000), DROP(0x1ff00fe), NODROP(0x2000001),
	NODROP(0x20fffff), NODROP(0x2105500), DROP(0x20ffffe), NODROP(0x21054ff),
	NODROP(0x211ffff), DROP(0x2110000), NODROP(0x2120000)
	/* The last test should be NODROP, because a DROP wouldn't get tested. */
};

static uchar TestData_BadDTLS_client_random[SSL3_RANDOM_SIZE];
static uchar TestData_BadDTLS_server_random[SSL3_RANDOM_SIZE];
// These are all generated locally, sized purely according to our own whim
static uchar TestData_BadDTLS_session_id[32];
static uchar TestData_BadDTLS_master_secret[48];
static uchar TestData_BadDTLS_cookie[20];
// We've hard-coded the cipher suite; we know it's 104 bytes
static uchar TestData_BadDTLS_key_block[104];
#define mac_key (TestData_BadDTLS_key_block + 20)
#define dec_key (TestData_BadDTLS_key_block + 40)
#define enc_key (TestData_BadDTLS_key_block + 56)
static EVP_MD_CTX * TestData_BadDTLS_handshake_md;

class TestInnerBlock_BadDTLS {
public:
	static int do_PRF(const void * seed1, int seed1_len, const void * seed2, int seed2_len, const void * seed3, int seed3_len, uchar * out, int olen)
	{
		EVP_PKEY_CTX * pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_TLS1_PRF, NULL);
		size_t outlen = olen;
		/* No error handling. If it all screws up, the test will fail anyway */
		EVP_PKEY_derive_init(pctx);
		EVP_PKEY_CTX_set_tls1_prf_md(pctx, EVP_md5_sha1());
		EVP_PKEY_CTX_set1_tls1_prf_secret(pctx, TestData_BadDTLS_master_secret, sizeof(TestData_BadDTLS_master_secret));
		EVP_PKEY_CTX_add1_tls1_prf_seed(pctx, (const uchar *)seed1, seed1_len);
		EVP_PKEY_CTX_add1_tls1_prf_seed(pctx, (const uchar *)seed2, seed2_len);
		EVP_PKEY_CTX_add1_tls1_prf_seed(pctx, (const uchar *)seed3, seed3_len);
		EVP_PKEY_derive(pctx, out, &outlen);
		EVP_PKEY_CTX_free(pctx);
		return 1;
	}

	static SSL_SESSION * client_session()
	{
		static uchar session_asn1[] = {
			0x30, 0x5F,      /* SEQUENCE, length 0x5F */
			0x02, 0x01, 0x01, /* INTEGER, SSL_SESSION_ASN1_VERSION */
			0x02, 0x02, 0x01, 0x00, /* INTEGER, DTLS1_BAD_VER */
			0x04, 0x02, 0x00, 0x2F, /* OCTET_STRING, AES128-SHA */
			0x04, 0x20,      /* OCTET_STRING, session id */
	#define SS_SESSID_OFS 15 /* Session ID goes here */
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x04, 0x30,      /* OCTET_STRING, master secret */
	#define SS_SECRET_OFS 49 /* Master secret goes here */
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		};
		const uchar * p = session_asn1;
		// Copy the randomly-generated fields into the above ASN1 
		memcpy(session_asn1 + SS_SESSID_OFS, TestData_BadDTLS_session_id, sizeof(TestData_BadDTLS_session_id));
		memcpy(session_asn1 + SS_SECRET_OFS, TestData_BadDTLS_master_secret, sizeof(TestData_BadDTLS_master_secret));
		return d2i_SSL_SESSION(NULL, &p, sizeof(session_asn1));
	}
	// Returns 1 for initial ClientHello, 2 for ClientHello with cookie 
	static int validate_client_hello(BIO * wbio)
	{
		PACKET pkt, pkt2;
		long len;
		uchar * data;
		int cookie_found = 0;
		unsigned int u = 0;
		if((len = BIO_get_mem_data(wbio, (char**)&data)) < 0)
			return 0;
		if(!PACKET_buf_init(&pkt, data, len))
			return 0;
		/* Check record header type */
		if(!PACKET_get_1(&pkt, &u) || u != SSL3_RT_HANDSHAKE)
			return 0;
		/* Version */
		if(!PACKET_get_net_2(&pkt, &u) || u != DTLS1_BAD_VER)
			return 0;
		/* Skip the rest of the record header */
		if(!PACKET_forward(&pkt, DTLS1_RT_HEADER_LENGTH - 3))
			return 0;
		/* Check it's a ClientHello */
		if(!PACKET_get_1(&pkt, &u) || u != SSL3_MT_CLIENT_HELLO)
			return 0;
		/* Skip the rest of the handshake message header */
		if(!PACKET_forward(&pkt, DTLS1_HM_HEADER_LENGTH - 1))
			return 0;
		/* Check client version */
		if(!PACKET_get_net_2(&pkt, &u) || u != DTLS1_BAD_VER)
			return 0;
		/* Store random */
		if(!PACKET_copy_bytes(&pkt, TestData_BadDTLS_client_random, SSL3_RANDOM_SIZE))
			return 0;
		/* Check session id length and content */
		if(!PACKET_get_length_prefixed_1(&pkt, &pkt2) || !PACKET_equal(&pkt2, TestData_BadDTLS_session_id, sizeof(TestData_BadDTLS_session_id)))
			return 0;
		/* Check cookie */
		if(!PACKET_get_length_prefixed_1(&pkt, &pkt2))
			return 0;
		if(PACKET_remaining(&pkt2)) {
			if(!PACKET_equal(&pkt2, TestData_BadDTLS_cookie, sizeof(TestData_BadDTLS_cookie)))
				return 0;
			cookie_found = 1;
		}
		/* Skip ciphers */
		if(!PACKET_get_net_2(&pkt, &u) || !PACKET_forward(&pkt, u))
			return 0;
		/* Skip compression */
		if(!PACKET_get_1(&pkt, &u) || !PACKET_forward(&pkt, u))
			return 0;
		/* Skip extensions */
		if(!PACKET_get_net_2(&pkt, &u) || !PACKET_forward(&pkt, u))
			return 0;
		/* Now we are at the end */
		if(PACKET_remaining(&pkt))
			return 0;
		/* Update handshake MAC for second ClientHello (with cookie) */
		if(cookie_found && !EVP_DigestUpdate(TestData_BadDTLS_handshake_md, data + MAC_OFFSET, len - MAC_OFFSET))
			return 0;
		(void)BIO_reset(wbio);
		return 1 + cookie_found;
	}

	static int send_hello_verify(BIO * rbio)
	{
		static uchar hello_verify[] = {
			0x16, /* Handshake */
			0x01, 0x00, /* DTLS1_BAD_VER */
			0x00, 0x00, /* Epoch 0 */
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Seq# 0 */
			0x00, 0x23, /* Length */
			0x03, /* Hello Verify */
			0x00, 0x00, 0x17, /* Length */
			0x00, 0x00, /* Seq# 0 */
			0x00, 0x00, 0x00, /* Fragment offset */
			0x00, 0x00, 0x17, /* Fragment length */
			0x01, 0x00, /* DTLS1_BAD_VER */
			0x14, /* Cookie length */
	#define HV_COOKIE_OFS 28 /* Cookie goes here */
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
		};
		memcpy(hello_verify + HV_COOKIE_OFS, TestData_BadDTLS_cookie, sizeof(TestData_BadDTLS_cookie));
		BIO_write(rbio, hello_verify, sizeof(hello_verify));
		return 1;
	}

	static int send_server_hello(BIO * rbio)
	{
		static uchar server_hello[] = {
			0x16, /* Handshake */
			0x01, 0x00, /* DTLS1_BAD_VER */
			0x00, 0x00, /* Epoch 0 */
			0x00, 0x00, 0x00, 0x00, 0x00, 0x01, /* Seq# 1 */
			0x00, 0x52, /* Length */
			0x02, /* Server Hello */
			0x00, 0x00, 0x46, /* Length */
			0x00, 0x01, /* Seq# */
			0x00, 0x00, 0x00, /* Fragment offset */
			0x00, 0x00, 0x46, /* Fragment length */
			0x01, 0x00, /* DTLS1_BAD_VER */
	#define SH_RANDOM_OFS 27 /* Server random goes here */
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x20, /* Session ID length */
	#define SH_SESSID_OFS 60 /* Session ID goes here */
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x2f, /* Cipher suite AES128-SHA */
			0x00, /* Compression null */
		};
		static uchar change_cipher_spec[] = {
			0x14, /* Change Cipher Spec */
			0x01, 0x00, /* DTLS1_BAD_VER */
			0x00, 0x00, /* Epoch 0 */
			0x00, 0x00, 0x00, 0x00, 0x00, 0x02, /* Seq# 2 */
			0x00, 0x03, /* Length */
			0x01, 0x00, 0x02, /* Message */
		};
		memcpy(server_hello + SH_RANDOM_OFS, TestData_BadDTLS_server_random, sizeof(TestData_BadDTLS_server_random));
		memcpy(server_hello + SH_SESSID_OFS, TestData_BadDTLS_session_id, sizeof(TestData_BadDTLS_session_id));
		if(!EVP_DigestUpdate(TestData_BadDTLS_handshake_md, server_hello + MAC_OFFSET, sizeof(server_hello) - MAC_OFFSET))
			return 0;
		BIO_write(rbio, server_hello, sizeof(server_hello));
		BIO_write(rbio, change_cipher_spec, sizeof(change_cipher_spec));
		return 1;
	}

	/* Create header, HMAC, pad, encrypt and send a record */
	static int send_record(BIO * rbio, uchar type, uint64_t seqnr, const void * msg, size_t len)
	{
		/* Note that the order of the record header fields on the wire,
		 * and in the HMAC, is different. So we just keep them in separate
		 * variables and handle them individually. */
		static uchar epoch[2] = { 0x00, 0x01 };
		static uchar seq[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		static uchar ver[2] = { 0x01, 0x00 }; /* DTLS1_BAD_VER */
		uchar lenbytes[2];
		EVP_MAC * hmac = NULL;
		EVP_MAC_CTX * ctx = NULL;
		EVP_CIPHER_CTX * enc_ctx = NULL;
		uchar iv[16];
		uchar pad;
		uchar * enc;
		OSSL_PARAM params[2];
		int ret = 0;

		seq[0] = (seqnr >> 40) & 0xff;
		seq[1] = (seqnr >> 32) & 0xff;
		seq[2] = (seqnr >> 24) & 0xff;
		seq[3] = (seqnr >> 16) & 0xff;
		seq[4] = (seqnr >> 8) & 0xff;
		seq[5] = seqnr & 0xff;

		pad = 15 - ((len + SHA_DIGEST_LENGTH) % 16);
		enc = (uchar *)OPENSSL_malloc(len + SHA_DIGEST_LENGTH + 1 + pad);
		if(enc == NULL)
			return 0;

		/* Copy record to encryption buffer */
		memcpy(enc, msg, len);

		/* Append HMAC to data */
		if(!TEST_ptr(hmac = EVP_MAC_fetch(NULL, "HMAC", NULL)) || !TEST_ptr(ctx = EVP_MAC_CTX_new(hmac)))
			goto end;
		params[0] = OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_DIGEST, "SHA1", 0);
		params[1] = OSSL_PARAM_construct_end();
		lenbytes[0] = (uchar)(len >> 8);
		lenbytes[1] = (uchar)(len);
		if(!EVP_MAC_init(ctx, mac_key, 20, params)
			|| !EVP_MAC_update(ctx, epoch, 2)
			|| !EVP_MAC_update(ctx, seq, 6)
			|| !EVP_MAC_update(ctx, &type, 1)
			|| !EVP_MAC_update(ctx, ver, 2)      /* Version */
			|| !EVP_MAC_update(ctx, lenbytes, 2) /* Length */
			|| !EVP_MAC_update(ctx, enc, len)    /* Finally the data itself */
			|| !EVP_MAC_final(ctx, enc + len, NULL, SHA_DIGEST_LENGTH))
			goto end;

		/* Append padding bytes */
		len += SHA_DIGEST_LENGTH;
		do {
			enc[len++] = pad;
		} while(len % 16);

		/* Generate IV, and encrypt */
		if(!TEST_true(RAND_bytes(iv, sizeof(iv)))
			|| !TEST_ptr(enc_ctx = EVP_CIPHER_CTX_new())
			|| !TEST_true(EVP_CipherInit_ex(enc_ctx, EVP_aes_128_cbc(), NULL, enc_key, iv, 1))
			|| !TEST_int_ge(EVP_Cipher(enc_ctx, enc, enc, len), 0))
			goto end;
		/* Finally write header (from fragmented variables), IV and encrypted record */
		BIO_write(rbio, &type, 1);
		BIO_write(rbio, ver, 2);
		BIO_write(rbio, epoch, 2);
		BIO_write(rbio, seq, 6);
		lenbytes[0] = (uchar)((len + sizeof(iv)) >> 8);
		lenbytes[1] = (uchar)(len + sizeof(iv));
		BIO_write(rbio, lenbytes, 2);
		BIO_write(rbio, iv, sizeof(iv));
		BIO_write(rbio, enc, len);
		ret = 1;
	end:
		EVP_MAC_free(hmac);
		EVP_MAC_CTX_free(ctx);
		EVP_CIPHER_CTX_free(enc_ctx);
		OPENSSL_free(enc);
		return ret;
	}

	static int send_finished(SSL * s, BIO * rbio)
	{
		static uchar finished_msg[DTLS1_HM_HEADER_LENGTH +
		TLS1_FINISH_MAC_LENGTH] = {
			0x14, /* Finished */
			0x00, 0x00, 0x0c, /* Length */
			0x00, 0x03, /* Seq# 3 */
			0x00, 0x00, 0x00, /* Fragment offset */
			0x00, 0x00, 0x0c, /* Fragment length */
			/* Finished MAC (12 bytes) */
		};
		uchar handshake_hash[EVP_MAX_MD_SIZE];
		// Derive key material 
		do_PRF(TLS_MD_KEY_EXPANSION_CONST, TLS_MD_KEY_EXPANSION_CONST_SIZE, TestData_BadDTLS_server_random, SSL3_RANDOM_SIZE, 
			TestData_BadDTLS_client_random, SSL3_RANDOM_SIZE, TestData_BadDTLS_key_block, sizeof(TestData_BadDTLS_key_block));
		// Generate Finished MAC 
		if(!EVP_DigestFinal_ex(TestData_BadDTLS_handshake_md, handshake_hash, NULL))
			return 0;
		do_PRF(TLS_MD_SERVER_FINISH_CONST, TLS_MD_SERVER_FINISH_CONST_SIZE, handshake_hash, 
			EVP_MD_CTX_get_size(TestData_BadDTLS_handshake_md), NULL, 0, finished_msg + DTLS1_HM_HEADER_LENGTH, TLS1_FINISH_MAC_LENGTH);
		return send_record(rbio, SSL3_RT_HANDSHAKE, 0, finished_msg, sizeof(finished_msg));
	}

	static int validate_ccs(BIO * wbio)
	{
		PACKET pkt;
		uchar * data;
		unsigned int u;
		long len = BIO_get_mem_data(wbio, (char**)&data);
		if(len < 0)
			return 0;
		if(!PACKET_buf_init(&pkt, data, len))
			return 0;
		/* Check record header type */
		if(!PACKET_get_1(&pkt, &u) || u != SSL3_RT_CHANGE_CIPHER_SPEC)
			return 0;
		/* Version */
		if(!PACKET_get_net_2(&pkt, &u) || u != DTLS1_BAD_VER)
			return 0;
		/* Skip the rest of the record header */
		if(!PACKET_forward(&pkt, DTLS1_RT_HEADER_LENGTH - 3))
			return 0;
		/* Check ChangeCipherSpec message */
		if(!PACKET_get_1(&pkt, &u) || u != SSL3_MT_CCS)
			return 0;
		/* A DTLS1_BAD_VER ChangeCipherSpec also contains the
		 * handshake sequence number (which is 2 here) */
		if(!PACKET_get_net_2(&pkt, &u) || u != 0x0002)
			return 0;
		/* Now check the Finished packet */
		if(!PACKET_get_1(&pkt, &u) || u != SSL3_RT_HANDSHAKE)
			return 0;
		if(!PACKET_get_net_2(&pkt, &u) || u != DTLS1_BAD_VER)
			return 0;
		/* Check epoch is now 1 */
		if(!PACKET_get_net_2(&pkt, &u) || u != 0x0001)
			return 0;
		/* That'll do for now. If OpenSSL accepted *our* Finished packet
		 * then it's evidently remembered that DTLS1_BAD_VER doesn't
		 * include the handshake header in the MAC. There's not a lot of
		 * point in implementing decryption here, just to check that it
		 * continues to get it right for one more packet. */
		return 1;
	}
	static int test_bad_dtls()
	{
		SSL_SESSION * sess = NULL;
		SSL_CTX * ctx = NULL;
		SSL * con = NULL;
		BIO * rbio = NULL;
		BIO * wbio = NULL;
		time_t now = 0;
		int testresult = 0;
		int ret;
		int i;
		RAND_bytes(TestData_BadDTLS_session_id, sizeof(TestData_BadDTLS_session_id));
		RAND_bytes(TestData_BadDTLS_master_secret, sizeof(TestData_BadDTLS_master_secret));
		RAND_bytes(TestData_BadDTLS_cookie, sizeof(TestData_BadDTLS_cookie));
		RAND_bytes(TestData_BadDTLS_server_random + 4, sizeof(TestData_BadDTLS_server_random) - 4);
		now = time(NULL);
		memcpy(TestData_BadDTLS_server_random, &now, sizeof(now));
		sess = client_session();
		if(!TEST_ptr(sess))
			goto end;
		TestData_BadDTLS_handshake_md = EVP_MD_CTX_new();
		if(!TEST_ptr(TestData_BadDTLS_handshake_md) || !TEST_true(EVP_DigestInit_ex(TestData_BadDTLS_handshake_md, EVP_md5_sha1(), NULL)))
			goto end;
		ctx = SSL_CTX_new(DTLS_client_method());
		if(!TEST_ptr(ctx)
			|| !TEST_true(SSL_CTX_set_min_proto_version(ctx, DTLS1_BAD_VER))
			|| !TEST_true(SSL_CTX_set_max_proto_version(ctx, DTLS1_BAD_VER))
			|| !TEST_true(SSL_CTX_set_options(ctx, SSL_OP_LEGACY_SERVER_CONNECT))
			|| !TEST_true(SSL_CTX_set_cipher_list(ctx, "AES128-SHA")))
			goto end;
		con = SSL_new(ctx);
		if(!TEST_ptr(con) || !TEST_true(SSL_set_session(con, sess)))
			goto end;
		SSL_SESSION_free(sess);
		rbio = BIO_new(BIO_s_mem());
		wbio = BIO_new(BIO_s_mem());
		if(!TEST_ptr(rbio) || !TEST_ptr(wbio))
			goto end;
		SSL_set_bio(con, rbio, wbio);
		if(!TEST_true(BIO_up_ref(rbio))) {
			/*
			 * We can't up-ref but we assigned ownership to con, so we shouldn't
			 * free in the "end" block
			 */
			rbio = wbio = NULL;
			goto end;
		}
		if(!TEST_true(BIO_up_ref(wbio))) {
			wbio = NULL;
			goto end;
		}
		SSL_set_connect_state(con);
		/* Send initial ClientHello */
		ret = SSL_do_handshake(con);
		if(!TEST_int_le(ret, 0)
			|| !TEST_int_eq(SSL_get_error(con, ret), SSL_ERROR_WANT_READ)
			|| !TEST_int_eq(validate_client_hello(wbio), 1)
			|| !TEST_true(send_hello_verify(rbio)))
			goto end;
		ret = SSL_do_handshake(con);
		if(!TEST_int_le(ret, 0)
			|| !TEST_int_eq(SSL_get_error(con, ret), SSL_ERROR_WANT_READ)
			|| !TEST_int_eq(validate_client_hello(wbio), 2)
			|| !TEST_true(send_server_hello(rbio)))
			goto end;

		ret = SSL_do_handshake(con);
		if(!TEST_int_le(ret, 0)
			|| !TEST_int_eq(SSL_get_error(con, ret), SSL_ERROR_WANT_READ)
			|| !TEST_true(send_finished(con, rbio)))
			goto end;

		ret = SSL_do_handshake(con);
		if(!TEST_int_gt(ret, 0)
			|| !TEST_true(validate_ccs(wbio)))
			goto end;

		/* While we're here and crafting packets by hand, we might as well do a
		   bit of a stress test on the DTLS record replay handling. Not Cisco-DTLS
		   specific but useful anyway for the general case. It's been broken
		   before, and in fact was broken even for a basic 0, 2, 1 test case
		   when this test was first added.... */
		for(i = 0; i < (int)SIZEOFARRAY(TestData_BadDTLS_tests); i++) {
			uint64_t recv_buf[2];
			if(!TEST_true(send_record(rbio, SSL3_RT_APPLICATION_DATA, TestData_BadDTLS_tests[i].seq, &TestData_BadDTLS_tests[i].seq, sizeof(uint64_t)))) {
				TEST_error("Failed to send data seq #0x%x%08x (%d)\n", (uint)(TestData_BadDTLS_tests[i].seq >> 32), (uint)TestData_BadDTLS_tests[i].seq, i);
				goto end;
			}
			if(TestData_BadDTLS_tests[i].drop)
				continue;
			ret = SSL_read(con, recv_buf, 2 * sizeof(uint64_t));
			if(!TEST_int_eq(ret, (int)sizeof(uint64_t))) {
				TEST_error("SSL_read failed or wrong size on seq#0x%x%08x (%d)\n", (uint)(TestData_BadDTLS_tests[i].seq >> 32), (uint)TestData_BadDTLS_tests[i].seq, i);
				goto end;
			}
			if(!TEST_true(recv_buf[0] == TestData_BadDTLS_tests[i].seq))
				goto end;
		}
		/* The last test cannot be DROP() */
		if(!TEST_false(TestData_BadDTLS_tests[i-1].drop))
			goto end;
		testresult = 1;
	end:
		BIO_free(rbio);
		BIO_free(wbio);
		SSL_free(con);
		SSL_CTX_free(ctx);
		EVP_MD_CTX_free(TestData_BadDTLS_handshake_md);
		return testresult;
	}
};
// 
// RSA low level APIs are deprecated for public use, but still ok for internal use.
// test vectors from p1ovect1.txt
// 
#define Test_Rsa_SetKey \
	RSA_set0_key(key, BN_bin2bn(n, sizeof(n)-1, NULL), BN_bin2bn(e, sizeof(e)-1, NULL), BN_bin2bn(d, sizeof(d)-1, NULL)); \
	RSA_set0_factors(key, BN_bin2bn(p, sizeof(p)-1, NULL), BN_bin2bn(q, sizeof(q)-1, NULL));          \
	RSA_set0_crt_params(key, BN_bin2bn(dmp1, sizeof(dmp1)-1, NULL), BN_bin2bn(dmq1, sizeof(dmq1)-1, NULL), BN_bin2bn(iqmp, sizeof(iqmp)-1, NULL)); \
	if(c)                                              \
		memcpy(c, ctext_ex, sizeof(ctext_ex) - 1);              \
	return sizeof(ctext_ex) - 1;

static const struct {
	int bits;
	uint r;
} TestData_Rsa_rsa_security_bits_cases[] = {
	/* NIST SP 800-56B rev 2 (draft) Appendix D Table 5 */
	{ 2048,     112 },
	{ 3072,     128 },
	{ 4096,     152 },
	{ 6144,     176 },
	{ 8192,     200 },
	/* NIST FIPS 140-2 IG 7.5 */
	{ 7680,     192 },
	{ 15360,    256 },
	/* Older values */
	{ 256,      40  },
	{ 512,      56  },
	{ 1024,     80  },
	/* Some other values */
	{ 8888,     208 },
	{ 2468,     120 },
	{ 13456,    248 },
	/* Edge points */
	{ 15359,    256 },
	{ 15361,    264 },
	{ 7679,     192 },
	{ 7681,     200 },
};

class TestInnerBlock_Rsa {
public:
	static int key1(RSA * key, uchar * c)
	{
		static uchar n[] =
			"\x00\xAA\x36\xAB\xCE\x88\xAC\xFD\xFF\x55\x52\x3C\x7F\xC4\x52\x3F"
			"\x90\xEF\xA0\x0D\xF3\x77\x4A\x25\x9F\x2E\x62\xB4\xC5\xD9\x9C\xB5"
			"\xAD\xB3\x00\xA0\x28\x5E\x53\x01\x93\x0E\x0C\x70\xFB\x68\x76\x93"
			"\x9C\xE6\x16\xCE\x62\x4A\x11\xE0\x08\x6D\x34\x1E\xBC\xAC\xA0\xA1"
			"\xF5";
		static uchar e[] = "\x11";
		static uchar d[] =
			"\x0A\x03\x37\x48\x62\x64\x87\x69\x5F\x5F\x30\xBC\x38\xB9\x8B\x44"
			"\xC2\xCD\x2D\xFF\x43\x40\x98\xCD\x20\xD8\xA1\x38\xD0\x90\xBF\x64"
			"\x79\x7C\x3F\xA7\xA2\xCD\xCB\x3C\xD1\xE0\xBD\xBA\x26\x54\xB4\xF9"
			"\xDF\x8E\x8A\xE5\x9D\x73\x3D\x9F\x33\xB3\x01\x62\x4A\xFD\x1D\x51";
		static uchar p[] =
			"\x00\xD8\x40\xB4\x16\x66\xB4\x2E\x92\xEA\x0D\xA3\xB4\x32\x04\xB5"
			"\xCF\xCE\x33\x52\x52\x4D\x04\x16\xA5\xA4\x41\xE7\x00\xAF\x46\x12"
			"\x0D";
		static uchar q[] =
			"\x00\xC9\x7F\xB1\xF0\x27\xF4\x53\xF6\x34\x12\x33\xEA\xAA\xD1\xD9"
			"\x35\x3F\x6C\x42\xD0\x88\x66\xB1\xD0\x5A\x0F\x20\x35\x02\x8B\x9D"
			"\x89";
		static uchar dmp1[] =
			"\x59\x0B\x95\x72\xA2\xC2\xA9\xC4\x06\x05\x9D\xC2\xAB\x2F\x1D\xAF"
			"\xEB\x7E\x8B\x4F\x10\xA7\x54\x9E\x8E\xED\xF5\xB4\xFC\xE0\x9E\x05";
		static uchar dmq1[] =
			"\x00\x8E\x3C\x05\x21\xFE\x15\xE0\xEA\x06\xA3\x6F\xF0\xF1\x0C\x99"
			"\x52\xC3\x5B\x7A\x75\x14\xFD\x32\x38\xB8\x0A\xAD\x52\x98\x62\x8D"
			"\x51";
		static uchar iqmp[] =
			"\x36\x3F\xF7\x18\x9D\xA8\xE9\x0B\x1D\x34\x1F\x71\xD0\x9B\x76\xA8"
			"\xA9\x43\xE1\x1D\x10\xB2\x4D\x24\x9F\x2D\xEA\xFE\xF8\x0C\x18\x26";
		static uchar ctext_ex[] =
			"\x1b\x8f\x05\xf9\xca\x1a\x79\x52\x6e\x53\xf3\xcc\x51\x4f\xdb\x89"
			"\x2b\xfb\x91\x93\x23\x1e\x78\xb9\x92\xe6\x8d\x50\xa4\x80\xcb\x52"
			"\x33\x89\x5c\x74\x95\x8d\x5d\x02\xab\x8c\x0f\xd0\x40\xeb\x58\x44"
			"\xb0\x05\xc3\x9e\xd8\x27\x4a\x9d\xbf\xa8\x06\x71\x40\x94\x39\xd2";
		Test_Rsa_SetKey;
	}

	static int key2(RSA * key, uchar * c)
	{
		static uchar n[] =
			"\x00\xA3\x07\x9A\x90\xDF\x0D\xFD\x72\xAC\x09\x0C\xCC\x2A\x78\xB8"
			"\x74\x13\x13\x3E\x40\x75\x9C\x98\xFA\xF8\x20\x4F\x35\x8A\x0B\x26"
			"\x3C\x67\x70\xE7\x83\xA9\x3B\x69\x71\xB7\x37\x79\xD2\x71\x7B\xE8"
			"\x34\x77\xCF";
		static uchar e[] = "\x3";
		static uchar d[] =
			"\x6C\xAF\xBC\x60\x94\xB3\xFE\x4C\x72\xB0\xB3\x32\xC6\xFB\x25\xA2"
			"\xB7\x62\x29\x80\x4E\x68\x65\xFC\xA4\x5A\x74\xDF\x0F\x8F\xB8\x41"
			"\x3B\x52\xC0\xD0\xE5\x3D\x9B\x59\x0F\xF1\x9B\xE7\x9F\x49\xDD\x21"
			"\xE5\xEB";
		static uchar p[] =
			"\x00\xCF\x20\x35\x02\x8B\x9D\x86\x98\x40\xB4\x16\x66\xB4\x2E\x92"
			"\xEA\x0D\xA3\xB4\x32\x04\xB5\xCF\xCE\x91";
		static uchar q[] =
			"\x00\xC9\x7F\xB1\xF0\x27\xF4\x53\xF6\x34\x12\x33\xEA\xAA\xD1\xD9"
			"\x35\x3F\x6C\x42\xD0\x88\x66\xB1\xD0\x5F";
		static uchar dmp1[] =
			"\x00\x8A\x15\x78\xAC\x5D\x13\xAF\x10\x2B\x22\xB9\x99\xCD\x74\x61"
			"\xF1\x5E\x6D\x22\xCC\x03\x23\xDF\xDF\x0B";
		static uchar dmq1[] =
			"\x00\x86\x55\x21\x4A\xC5\x4D\x8D\x4E\xCD\x61\x77\xF1\xC7\x36\x90"
			"\xCE\x2A\x48\x2C\x8B\x05\x99\xCB\xE0\x3F";
		static uchar iqmp[] =
			"\x00\x83\xEF\xEF\xB8\xA9\xA4\x0D\x1D\xB6\xED\x98\xAD\x84\xED\x13"
			"\x35\xDC\xC1\x08\xF3\x22\xD0\x57\xCF\x8D";
		static uchar ctext_ex[] =
			"\x14\xbd\xdd\x28\xc9\x83\x35\x19\x23\x80\xe8\xe5\x49\xb1\x58\x2a"
			"\x8b\x40\xb4\x48\x6d\x03\xa6\xa5\x31\x1f\x1f\xd5\xf0\xa1\x80\xe4"
			"\x17\x53\x03\x29\xa9\x34\x90\x74\xb1\x52\x13\x54\x29\x08\x24\x52"
			"\x62\x51";
		Test_Rsa_SetKey;
	}
	static int key3(RSA * key, uchar * c)
	{
		static uchar n[] =
			"\x00\xBB\xF8\x2F\x09\x06\x82\xCE\x9C\x23\x38\xAC\x2B\x9D\xA8\x71"
			"\xF7\x36\x8D\x07\xEE\xD4\x10\x43\xA4\x40\xD6\xB6\xF0\x74\x54\xF5"
			"\x1F\xB8\xDF\xBA\xAF\x03\x5C\x02\xAB\x61\xEA\x48\xCE\xEB\x6F\xCD"
			"\x48\x76\xED\x52\x0D\x60\xE1\xEC\x46\x19\x71\x9D\x8A\x5B\x8B\x80"
			"\x7F\xAF\xB8\xE0\xA3\xDF\xC7\x37\x72\x3E\xE6\xB4\xB7\xD9\x3A\x25"
			"\x84\xEE\x6A\x64\x9D\x06\x09\x53\x74\x88\x34\xB2\x45\x45\x98\x39"
			"\x4E\xE0\xAA\xB1\x2D\x7B\x61\xA5\x1F\x52\x7A\x9A\x41\xF6\xC1\x68"
			"\x7F\xE2\x53\x72\x98\xCA\x2A\x8F\x59\x46\xF8\xE5\xFD\x09\x1D\xBD"
			"\xCB";
		static uchar e[] = "\x11";
		static uchar d[] =
			"\x00\xA5\xDA\xFC\x53\x41\xFA\xF2\x89\xC4\xB9\x88\xDB\x30\xC1\xCD"
			"\xF8\x3F\x31\x25\x1E\x06\x68\xB4\x27\x84\x81\x38\x01\x57\x96\x41"
			"\xB2\x94\x10\xB3\xC7\x99\x8D\x6B\xC4\x65\x74\x5E\x5C\x39\x26\x69"
			"\xD6\x87\x0D\xA2\xC0\x82\xA9\x39\xE3\x7F\xDC\xB8\x2E\xC9\x3E\xDA"
			"\xC9\x7F\xF3\xAD\x59\x50\xAC\xCF\xBC\x11\x1C\x76\xF1\xA9\x52\x94"
			"\x44\xE5\x6A\xAF\x68\xC5\x6C\x09\x2C\xD3\x8D\xC3\xBE\xF5\xD2\x0A"
			"\x93\x99\x26\xED\x4F\x74\xA1\x3E\xDD\xFB\xE1\xA1\xCE\xCC\x48\x94"
			"\xAF\x94\x28\xC2\xB7\xB8\x88\x3F\xE4\x46\x3A\x4B\xC8\x5B\x1C\xB3"
			"\xC1";
		static uchar p[] =
			"\x00\xEE\xCF\xAE\x81\xB1\xB9\xB3\xC9\x08\x81\x0B\x10\xA1\xB5\x60"
			"\x01\x99\xEB\x9F\x44\xAE\xF4\xFD\xA4\x93\xB8\x1A\x9E\x3D\x84\xF6"
			"\x32\x12\x4E\xF0\x23\x6E\x5D\x1E\x3B\x7E\x28\xFA\xE7\xAA\x04\x0A"
			"\x2D\x5B\x25\x21\x76\x45\x9D\x1F\x39\x75\x41\xBA\x2A\x58\xFB\x65"
			"\x99";
		static uchar q[] =
			"\x00\xC9\x7F\xB1\xF0\x27\xF4\x53\xF6\x34\x12\x33\xEA\xAA\xD1\xD9"
			"\x35\x3F\x6C\x42\xD0\x88\x66\xB1\xD0\x5A\x0F\x20\x35\x02\x8B\x9D"
			"\x86\x98\x40\xB4\x16\x66\xB4\x2E\x92\xEA\x0D\xA3\xB4\x32\x04\xB5"
			"\xCF\xCE\x33\x52\x52\x4D\x04\x16\xA5\xA4\x41\xE7\x00\xAF\x46\x15"
			"\x03";
		static uchar dmp1[] =
			"\x54\x49\x4C\xA6\x3E\xBA\x03\x37\xE4\xE2\x40\x23\xFC\xD6\x9A\x5A"
			"\xEB\x07\xDD\xDC\x01\x83\xA4\xD0\xAC\x9B\x54\xB0\x51\xF2\xB1\x3E"
			"\xD9\x49\x09\x75\xEA\xB7\x74\x14\xFF\x59\xC1\xF7\x69\x2E\x9A\x2E"
			"\x20\x2B\x38\xFC\x91\x0A\x47\x41\x74\xAD\xC9\x3C\x1F\x67\xC9\x81";
		static uchar dmq1[] =
			"\x47\x1E\x02\x90\xFF\x0A\xF0\x75\x03\x51\xB7\xF8\x78\x86\x4C\xA9"
			"\x61\xAD\xBD\x3A\x8A\x7E\x99\x1C\x5C\x05\x56\xA9\x4C\x31\x46\xA7"
			"\xF9\x80\x3F\x8F\x6F\x8A\xE3\x42\xE9\x31\xFD\x8A\xE4\x7A\x22\x0D"
			"\x1B\x99\xA4\x95\x84\x98\x07\xFE\x39\xF9\x24\x5A\x98\x36\xDA\x3D";
		static uchar iqmp[] =
			"\x00\xB0\x6C\x4F\xDA\xBB\x63\x01\x19\x8D\x26\x5B\xDB\xAE\x94\x23"
			"\xB3\x80\xF2\x71\xF7\x34\x53\x88\x50\x93\x07\x7F\xCD\x39\xE2\x11"
			"\x9F\xC9\x86\x32\x15\x4F\x58\x83\xB1\x67\xA9\x67\xBF\x40\x2B\x4E"
			"\x9E\x2E\x0F\x96\x56\xE6\x98\xEA\x36\x66\xED\xFB\x25\x79\x80\x39"
			"\xF7";
		static uchar ctext_ex[] =
			"\xb8\x24\x6b\x56\xa6\xed\x58\x81\xae\xb5\x85\xd9\xa2\x5b\x2a\xd7"
			"\x90\xc4\x17\xe0\x80\x68\x1b\xf1\xac\x2b\xc3\xde\xb6\x9d\x8b\xce"
			"\xf0\xc4\x36\x6f\xec\x40\x0a\xf0\x52\xa7\x2e\x9b\x0e\xff\xb5\xb3"
			"\xf2\xf1\x92\xdb\xea\xca\x03\xc1\x27\x40\x05\x71\x13\xbf\x1f\x06"
			"\x69\xac\x22\xe9\xf3\xa7\x85\x2e\x3c\x15\xd9\x13\xca\xb0\xb8\x86"
			"\x3a\x95\xc9\x92\x94\xce\x86\x74\x21\x49\x54\x61\x03\x46\xf4\xd4"
			"\x74\xb2\x6f\x7c\x48\xb4\x2e\xe6\x8e\x1f\x57\x2a\x1f\xc4\x02\x6a"
			"\xc4\x56\xb4\xf5\x9f\x7b\x62\x1e\xa1\xb9\xd8\x8f\x64\x20\x2f\xb1";
		Test_Rsa_SetKey;
	}
	static int rsa_setkey(RSA** key, uchar * ctext, int idx)
	{
		int clen = 0;
		*key = RSA_new();
		if(*key)
			switch(idx) {
				case 0:
					clen = key1(*key, ctext);
					break;
				case 1:
					clen = key2(*key, ctext);
					break;
				case 2:
					clen = key3(*key, ctext);
					break;
			}
		return clen;
	}

	static int test_rsa_simple(int idx, int en_pad_type, int de_pad_type, int success, uchar * ctext_ex, int * clen, RSA ** retkey)
	{
		int ret = 0;
		RSA * key;
		uchar ptext[256];
		uchar ctext[256];
		static uchar ptext_ex[] = "\x54\x85\x9b\x34\x2c\x49\xea\x2a";
		int num;
		int plen = sizeof(ptext_ex) - 1;
		int clentmp = rsa_setkey(&key, ctext_ex, idx);
		if(clen)
			*clen = clentmp;
		num = RSA_public_encrypt(plen, ptext_ex, ctext, key, en_pad_type);
		if(!TEST_int_eq(num, clentmp))
			goto err;

		num = RSA_private_decrypt(num, ctext, ptext, key, de_pad_type);
		if(success) {
			if(!TEST_int_gt(num, 0) || !TEST_mem_eq(ptext, num, ptext_ex, plen))
				goto err;
		}
		else {
			if(!TEST_int_lt(num, 0))
				goto err;
		}
		ret = 1;
		if(retkey) {
			*retkey = key;
			key = NULL;
		}
	err:
		RSA_free(key);
		return ret;
	}
	static int test_rsa_pkcs1(int idx)
	{
		return test_rsa_simple(idx, RSA_PKCS1_PADDING, RSA_PKCS1_PADDING, 1, NULL, NULL, NULL);
	}
	static int test_rsa_oaep(int idx)
	{
		int ret = 0;
		RSA * key = NULL;
		uchar ptext[256];
		static uchar ptext_ex[] = "\x54\x85\x9b\x34\x2c\x49\xea\x2a";
		uchar ctext_ex[256];
		int plen;
		int clen = 0;
		int num;
		int n;
		if(!test_rsa_simple(idx, RSA_PKCS1_OAEP_PADDING, RSA_PKCS1_OAEP_PADDING, 1, ctext_ex, &clen, &key))
			goto err;
		plen = sizeof(ptext_ex) - 1;
		/* Different ciphertexts. Try decrypting ctext_ex */
		num = RSA_private_decrypt(clen, ctext_ex, ptext, key, RSA_PKCS1_OAEP_PADDING);
		if(num <= 0 || !TEST_mem_eq(ptext, num, ptext_ex, plen))
			goto err;
		/* Try decrypting corrupted ciphertexts. */
		for(n = 0; n < clen; ++n) {
			ctext_ex[n] ^= 1;
			num = RSA_private_decrypt(clen, ctext_ex, ptext, key,
				RSA_PKCS1_OAEP_PADDING);
			if(!TEST_int_le(num, 0))
				goto err;
			ctext_ex[n] ^= 1;
		}

		/* Test truncated ciphertexts, as well as negative length. */
		for(n = -1; n < clen; ++n) {
			num = RSA_private_decrypt(n, ctext_ex, ptext, key,
				RSA_PKCS1_OAEP_PADDING);
			if(!TEST_int_le(num, 0))
				goto err;
		}
		ret = 1;
	err:
		RSA_free(key);
		return ret;
	}
	static int test_rsa_security_bit(int n)
	{
		static const uchar vals[8] = { 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40 };
		RSA * key = RSA_new();
		const int bits = TestData_Rsa_rsa_security_bits_cases[n].bits;
		const int result = TestData_Rsa_rsa_security_bits_cases[n].r;
		const int bytes = (bits + 7) / 8;
		int r = 0;
		uchar num[2000];
		if(!TEST_ptr(key) || !TEST_int_le(bytes, (int)sizeof(num)))
			goto err;
		/*
		 * It is necessary to set the RSA key in order to ask for the strength.
		 * A BN of an appropriate size is created, in general it won't have the
		 * properties necessary for RSA to function.  This is okay here since
		 * the RSA key is never used.
		 */
		memset(num, vals[bits % 8], bytes);
		/*
		 * The 'e' parameter is set to the same value as 'n'.  This saves having
		 * an extra BN to hold a sensible value for 'e'.  This is safe since the
		 * RSA key is not used.  The 'd' parameter can be NULL safely.
		 */
		if(TEST_true(RSA_set0_key(key, BN_bin2bn(num, bytes, NULL), BN_bin2bn(num, bytes, NULL), NULL)) && TEST_uint_eq(RSA_security_bits(key), result))
			r = 1;
	err:
		RSA_free(key);
		return r;
	}
};
// 
// This aims to test the setting functions, including internal ones
// RSA low level APIs are deprecated for public use, but still ok for internal use.
// 
#define Test_RsaMp_NUM_EXTRA_PRIMES 1

// C90 requires string should <= 509 bytes 
static const uchar TestData_RsaMp_n[] =
    "\x92\x60\xd0\x75\x0a\xe1\x17\xee\xe5\x5c\x3f\x3d\xea\xba\x74\x91"
    "\x75\x21\xa2\x62\xee\x76\x00\x7c\xdf\x8a\x56\x75\x5a\xd7\x3a\x15"
    "\x98\xa1\x40\x84\x10\xa0\x14\x34\xc3\xf5\xbc\x54\xa8\x8b\x57\xfa"
    "\x19\xfc\x43\x28\xda\xea\x07\x50\xa4\xc4\x4e\x88\xcf\xf3\xb2\x38"
    "\x26\x21\xb8\x0f\x67\x04\x64\x43\x3e\x43\x36\xe6\xd0\x03\xe8\xcd"
    "\x65\xbf\xf2\x11\xda\x14\x4b\x88\x29\x1c\x22\x59\xa0\x0a\x72\xb7"
    "\x11\xc1\x16\xef\x76\x86\xe8\xfe\xe3\x4e\x4d\x93\x3c\x86\x81\x87"
    "\xbd\xc2\x6f\x7b\xe0\x71\x49\x3c\x86\xf7\xa5\x94\x1c\x35\x10\x80"
    "\x6a\xd6\x7b\x0f\x94\xd8\x8f\x5c\xf5\xc0\x2a\x09\x28\x21\xd8\x62"
    "\x6e\x89\x32\xb6\x5c\x5b\xd8\xc9\x20\x49\xc2\x10\x93\x2b\x7a\xfa"
    "\x7a\xc5\x9c\x0e\x88\x6a\xe5\xc1\xed\xb0\x0d\x8c\xe2\xc5\x76\x33"
    "\xdb\x26\xbd\x66\x39\xbf\xf7\x3c\xee\x82\xbe\x92\x75\xc4\x02\xb4"
    "\xcf\x2a\x43\x88\xda\x8c\xf8\xc6\x4e\xef\xe1\xc5\xa0\xf5\xab\x80"
    "\x57\xc3\x9f\xa5\xc0\x58\x9c\x3e\x25\x3f\x09\x60\x33\x23\x00\xf9"
    "\x4b\xea\x44\x87\x7b\x58\x8e\x1e\xdb\xde\x97\xcf\x23\x60\x72\x7a"
    "\x09\xb7\x75\x26\x2d\x7e\xe5\x52\xb3\x31\x9b\x92\x66\xf0\x5a\x25";

static const uchar TestData_RsaMp_e[] = "\x01\x00\x01";

static const uchar TestData_RsaMp_d[] =
    "\x6a\x7d\xf2\xca\x63\xea\xd4\xdd\xa1\x91\xd6\x14\xb6\xb3\x85\xe0"
    "\xd9\x05\x6a\x3d\x6d\x5c\xfe\x07\xdb\x1d\xaa\xbe\xe0\x22\xdb\x08"
    "\x21\x2d\x97\x61\x3d\x33\x28\xe0\x26\x7c\x9d\xd2\x3d\x78\x7a\xbd"
    "\xe2\xaf\xcb\x30\x6a\xeb\x7d\xfc\xe6\x92\x46\xcc\x73\xf5\xc8\x7f"
    "\xdf\x06\x03\x01\x79\xa2\x11\x4b\x76\x7d\xb1\xf0\x83\xff\x84\x1c"
    "\x02\x5d\x7d\xc0\x0c\xd8\x24\x35\xb9\xa9\x0f\x69\x53\x69\xe9\x4d"
    "\xf2\x3d\x2c\xe4\x58\xbc\x3b\x32\x83\xad\x8b\xba\x2b\x8f\xa1\xba"
    "\x62\xe2\xdc\xe9\xac\xcf\xf3\x79\x9a\xae\x7c\x84\x00\x16\xf3\xba"
    "\x8e\x00\x48\xc0\xb6\xcc\x43\x39\xaf\x71\x61\x00\x3a\x5b\xeb\x86"
    "\x4a\x01\x64\xb2\xc1\xc9\x23\x7b\x64\xbc\x87\x55\x69\x94\x35\x1b"
    "\x27\x50\x6c\x33\xd4\xbc\xdf\xce\x0f\x9c\x49\x1a\x7d\x6b\x06\x28"
    "\xc7\xc8\x52\xbe\x4f\x0a\x9c\x31\x32\xb2\xed\x3a\x2c\x88\x81\xe9"
    "\xaa\xb0\x7e\x20\xe1\x7d\xeb\x07\x46\x91\xbe\x67\x77\x76\xa7\x8b"
    "\x5c\x50\x2e\x05\xd9\xbd\xde\x72\x12\x6b\x37\x38\x69\x5e\x2d\xd1"
    "\xa0\xa9\x8a\x14\x24\x7c\x65\xd8\xa7\xee\x79\x43\x2a\x09\x2c\xb0"
    "\x72\x1a\x12\xdf\x79\x8e\x44\xf7\xcf\xce\x0c\x49\x81\x47\xa9\xb1";

static const uchar TestData_RsaMp_p[] =
    "\x06\x77\xcd\xd5\x46\x9b\xc1\xd5\x58\x00\x81\xe2\xf3\x0a\x36\xb1"
    "\x6e\x29\x89\xd5\x2f\x31\x5f\x92\x22\x3b\x9b\x75\x30\x82\xfa\xc5"
    "\xf5\xde\x8a\x36\xdb\xc6\xe5\x8f\xef\x14\x37\xd6\x00\xf9\xab\x90"
    "\x9b\x5d\x57\x4c\xf5\x1f\x77\xc4\xbb\x8b\xdd\x9b\x67\x11\x45\xb2"
    "\x64\xe8\xac\xa8\x03\x0f\x16\x0d\x5d\x2d\x53\x07\x23\xfb\x62\x0d"
    "\xe6\x16\xd3\x23\xe8\xb3";

static const uchar TestData_RsaMp_q[] =
    "\x06\x66\x9a\x70\x53\xd6\x72\x74\xfd\xea\x45\xc3\xc0\x17\xae\xde"
    "\x79\x17\xae\x79\xde\xfc\x0e\xf7\xa4\x3a\x8c\x43\x8f\xc7\x8a\xa2"
    "\x2c\x51\xc4\xd0\x72\x89\x73\x5c\x61\xbe\xfd\x54\x3f\x92\x65\xde"
    "\x4d\x65\x71\x70\xf6\xf2\xe5\x98\xb9\x0f\xd1\x0b\xe6\x95\x09\x4a"
    "\x7a\xdf\xf3\x10\x16\xd0\x60\xfc\xa5\x10\x34\x97\x37\x6f\x0a\xd5"
    "\x5d\x8f\xd4\xc3\xa0\x5b";

static const uchar TestData_RsaMp_dmp1[] =
    "\x05\x7c\x9e\x1c\xbd\x90\x25\xe7\x40\x86\xf5\xa8\x3b\x7a\x3f\x99"
    "\x56\x95\x60\x3a\x7b\x95\x4b\xb8\xa0\xd7\xa5\xf1\xcc\xdc\x5f\xb5"
    "\x8c\xf4\x62\x95\x54\xed\x2e\x12\x62\xc2\xe8\xf6\xde\xce\xed\x8e"
    "\x77\x6d\xc0\x40\x25\x74\xb3\x5a\x2d\xaa\xe1\xac\x11\xcb\xe2\x2f"
    "\x0a\x51\x23\x1e\x47\xb2\x05\x88\x02\xb2\x0f\x4b\xf0\x67\x30\xf0"
    "\x0f\x6e\xef\x5f\xf7\xe7";

static const uchar TestData_RsaMp_dmq1[] =
    "\x01\xa5\x6b\xbc\xcd\xe3\x0e\x46\xc6\x72\xf5\x04\x56\x28\x01\x22"
    "\x58\x74\x5d\xbc\x1c\x3c\x29\x41\x49\x6c\x81\x5c\x72\xe2\xf7\xe5"
    "\xa3\x8e\x58\x16\xe0\x0e\x37\xac\x1f\xbb\x75\xfd\xaf\xe7\xdf\xe9"
    "\x1f\x70\xa2\x8f\x52\x03\xc0\x46\xd9\xf9\x96\x63\x00\x27\x7e\x5f"
    "\x38\x60\xd6\x6b\x61\xe2\xaf\xbe\xea\x58\xd3\x9d\xbc\x75\x03\x8d"
    "\x42\x65\xd6\x6b\x85\x97";

static const uchar TestData_RsaMp_iqmp[] =
    "\x03\xa1\x8b\x80\xe4\xd8\x87\x25\x17\x5d\xcc\x8d\xa9\x8a\x22\x2b"
    "\x6c\x15\x34\x6f\x80\xcc\x1c\x44\x04\x68\xbc\x03\xcd\x95\xbb\x69"
    "\x37\x61\x48\xb4\x23\x13\x08\x16\x54\x6a\xa1\x7c\xf5\xd4\x3a\xe1"
    "\x4f\xa4\x0c\xf5\xaf\x80\x85\x27\x06\x0d\x70\xc0\xc5\x19\x28\xfe"
    "\xee\x8e\x86\x21\x98\x8a\x37\xb7\xe5\x30\x25\x70\x93\x51\x2d\x49"
    "\x85\x56\xb3\x0c\x2b\x96";

static const uchar TestData_RsaMp_ex_prime[] =
    "\x03\x89\x22\xa0\xb7\x3a\x91\xcb\x5e\x0c\xfd\x73\xde\xa7\x38\xa9"
    "\x47\x43\xd6\x02\xbf\x2a\xb9\x3c\x48\xf3\x06\xd6\x58\x35\x50\x56"
    "\x16\x5c\x34\x9b\x61\x87\xc8\xaa\x0a\x5d\x8a\x0a\xcd\x9c\x41\xd9"
    "\x96\x24\xe0\xa9\x9b\x26\xb7\xa8\x08\xc9\xea\xdc\xa7\x15\xfb\x62"
    "\xa0\x2d\x90\xe6\xa7\x55\x6e\xc6\x6c\xff\xd6\x10\x6d\xfa\x2e\x04"
    "\x50\xec\x5c\x66\xe4\x05";

static const uchar TestData_RsaMp_ex_exponent[] =
    "\x02\x0a\xcd\xc3\x82\xd2\x03\xb0\x31\xac\xd3\x20\x80\x34\x9a\x57"
    "\xbc\x60\x04\x57\x25\xd0\x29\x9a\x16\x90\xb9\x1c\x49\x6a\xd1\xf2"
    "\x47\x8c\x0e\x9e\xc9\x20\xc2\xd8\xe4\x8f\xce\xd2\x1a\x9c\xec\xb4"
    "\x1f\x33\x41\xc8\xf5\x62\xd1\xa5\xef\x1d\xa1\xd8\xbd\x71\xc6\xf7"
    "\xda\x89\x37\x2e\xe2\xec\x47\xc5\xb8\xe3\xb4\xe3\x5c\x82\xaa\xdd"
    "\xb7\x58\x2e\xaf\x07\x79";

static const uchar TestData_RsaMp_ex_coefficient[] =
    "\x00\x9c\x09\x88\x9b\xc8\x57\x08\x69\x69\xab\x2d\x9e\x29\x1c\x3c"
    "\x6d\x59\x33\x12\x0d\x2b\x09\x2e\xaf\x01\x2c\x27\x01\xfc\xbd\x26"
    "\x13\xf9\x2d\x09\x22\x4e\x49\x11\x03\x82\x88\x87\xf4\x43\x1d\xac"
    "\xca\xec\x86\xf7\x23\xf1\x64\xf3\xf5\x81\xf0\x37\x36\xcf\x67\xff"
    "\x1a\xff\x7a\xc7\xf9\xf9\x67\x2d\xa0\x9d\x61\xf8\xf6\x47\x5c\x2f"
    "\xe7\x66\xe8\x3c\x3a\xe8";

class TestInnerBlock_RsaMp {
public:
	static int key2048_key(RSA * key)
	{
		if(!TEST_int_eq(RSA_set0_key(key, BN_bin2bn(TestData_RsaMp_n, sizeof(TestData_RsaMp_n) - 1, NULL), 
			BN_bin2bn(TestData_RsaMp_e, sizeof(TestData_RsaMp_e) - 1, NULL), BN_bin2bn(TestData_RsaMp_d, sizeof(TestData_RsaMp_d) - 1, NULL)), 1))
			return 0;
		return RSA_size(key);
	}
	static int key2048p3_v1(RSA * key)
	{
		BIGNUM ** pris = NULL, ** exps = NULL, ** coeffs = NULL;
		int rv = RSA_size(key);
		if(!TEST_int_eq(RSA_set0_factors(key,
			BN_bin2bn(TestData_RsaMp_p, sizeof(TestData_RsaMp_p) - 1, NULL),
			BN_bin2bn(TestData_RsaMp_q, sizeof(TestData_RsaMp_q) - 1, NULL)), 1))
			goto err;
		if(!TEST_int_eq(RSA_set0_crt_params(key,
			BN_bin2bn(TestData_RsaMp_dmp1, sizeof(TestData_RsaMp_dmp1) - 1, NULL),
			BN_bin2bn(TestData_RsaMp_dmq1, sizeof(TestData_RsaMp_dmq1) - 1, NULL),
			BN_bin2bn(TestData_RsaMp_iqmp, sizeof(TestData_RsaMp_iqmp) - 1, NULL)), 1))
			return 0;
		pris = (BIGNUM **)OPENSSL_zalloc(sizeof(BIGNUM *));
		exps = (BIGNUM **)OPENSSL_zalloc(sizeof(BIGNUM *));
		coeffs = (BIGNUM **)OPENSSL_zalloc(sizeof(BIGNUM *));
		if(!TEST_ptr(pris) || !TEST_ptr(exps) || !TEST_ptr(coeffs))
			goto err;
		pris[0] = BN_bin2bn(TestData_RsaMp_ex_prime, sizeof(TestData_RsaMp_ex_prime) - 1, NULL);
		exps[0] = BN_bin2bn(TestData_RsaMp_ex_exponent, sizeof(TestData_RsaMp_ex_exponent) - 1, NULL);
		coeffs[0] = BN_bin2bn(TestData_RsaMp_ex_coefficient, sizeof(TestData_RsaMp_ex_coefficient) - 1, NULL);
		if(!TEST_ptr(pris[0]) || !TEST_ptr(exps[0]) || !TEST_ptr(coeffs[0]))
			goto err;
		if(!TEST_true(RSA_set0_multi_prime_params(key, pris, exps, coeffs, Test_RsaMp_NUM_EXTRA_PRIMES)))
			goto err;
	ret:
		OPENSSL_free(pris);
		OPENSSL_free(exps);
		OPENSSL_free(coeffs);
		return rv;
	err:
		if(pris)
			BN_free(pris[0]);
		if(exps)
			BN_free(exps[0]);
		if(coeffs)
			BN_free(coeffs[0]);
		rv = 0;
		goto ret;
	}
	static int key2048p3_v2(RSA * key)
	{
		STACK_OF(BIGNUM) * primes = NULL;
		STACK_OF(BIGNUM) * exps = NULL;
		STACK_OF(BIGNUM) * coeffs = NULL;
		BIGNUM * num = NULL;
		int rv = RSA_size(key);
		if(!TEST_ptr(primes = sk_BIGNUM_new_null())
			|| !TEST_ptr(exps = sk_BIGNUM_new_null())
			|| !TEST_ptr(coeffs = sk_BIGNUM_new_null()))
			goto err;
		if(!TEST_ptr(num = BN_bin2bn(TestData_RsaMp_p, sizeof(TestData_RsaMp_p) - 1, NULL))
			|| !TEST_int_ne(sk_BIGNUM_push(primes, num), 0)
			|| !TEST_ptr(num = BN_bin2bn(TestData_RsaMp_q, sizeof(TestData_RsaMp_q) - 1, NULL))
			|| !TEST_int_ne(sk_BIGNUM_push(primes, num), 0)
			|| !TEST_ptr(num = BN_bin2bn(TestData_RsaMp_ex_prime, sizeof(TestData_RsaMp_ex_prime) - 1, NULL))
			|| !TEST_int_ne(sk_BIGNUM_push(primes, num), 0))
			goto err;
		if(!TEST_ptr(num = BN_bin2bn(TestData_RsaMp_dmp1, sizeof(TestData_RsaMp_dmp1) - 1, NULL))
			|| !TEST_int_ne(sk_BIGNUM_push(exps, num), 0)
			|| !TEST_ptr(num = BN_bin2bn(TestData_RsaMp_dmq1, sizeof(TestData_RsaMp_dmq1) - 1, NULL))
			|| !TEST_int_ne(sk_BIGNUM_push(exps, num), 0)
			|| !TEST_ptr(num = BN_bin2bn(TestData_RsaMp_ex_exponent, sizeof(TestData_RsaMp_ex_exponent) - 1, NULL))
			|| !TEST_int_ne(sk_BIGNUM_push(exps, num), 0))
			goto err;
		if(!TEST_ptr(num = BN_bin2bn(TestData_RsaMp_iqmp, sizeof(TestData_RsaMp_iqmp) - 1, NULL))
			|| !TEST_int_ne(sk_BIGNUM_push(coeffs, num), 0)
			|| !TEST_ptr(num = BN_bin2bn(TestData_RsaMp_ex_coefficient, sizeof(TestData_RsaMp_ex_coefficient) - 1, NULL))
			|| !TEST_int_ne(sk_BIGNUM_push(coeffs, num), 0))
			goto err;
		if(!TEST_true(ossl_rsa_set0_all_params(key, primes, exps, coeffs)))
			goto err;
	ret:
		sk_BIGNUM_free(primes);
		sk_BIGNUM_free(exps);
		sk_BIGNUM_free(coeffs);
		return rv;
	err:
		sk_BIGNUM_pop_free(primes, BN_free);
		sk_BIGNUM_pop_free(exps, BN_free);
		sk_BIGNUM_pop_free(coeffs, BN_free);
		primes = exps = coeffs = NULL;
		rv = 0;
		goto ret;
	}
	static int test_rsa_mp(int i)
	{
		int ret = 0;
		uchar ptext[256];
		uchar ctext[256];
		static uchar ptext_ex[] = "\x54\x85\x9b\x34\x2c\x49\xea\x2a";
		int clen = 0;
		int num;
		static int (*param_set[])(RSA *) = { key2048p3_v1, key2048p3_v2, };
		int plen = sizeof(ptext_ex) - 1;
		RSA * key = RSA_new();
		if(!TEST_ptr(key))
			goto err;
		if(!TEST_int_eq((clen = key2048_key(key)), 256) || !TEST_int_eq((clen = param_set[i](key)), 256))
			goto err;
		if(!TEST_true(RSA_check_key_ex(key, NULL)))
			goto err;
		num = RSA_public_encrypt(plen, ptext_ex, ctext, key, RSA_PKCS1_PADDING);
		if(!TEST_int_eq(num, clen))
			goto err;
		num = RSA_private_decrypt(num, ctext, ptext, key, RSA_PKCS1_PADDING);
		if(!TEST_mem_eq(ptext, num, ptext_ex, plen))
			goto err;
		ret = 1;
	err:
		RSA_free(key);
		return ret;
	}
};
// 
// RSA low level APIs are deprecated for public use, but still ok for internal use.
// 
// taken from RSA2 cavs data 
//
static const uchar TestData_Rsa_Sp800_56b_cav_e[] = { 0x01, 0x00, 0x01 };

static const uchar TestData_Rsa_Sp800_56b_cav_p[] = {
	0xcf, 0x72, 0x1b, 0x9a, 0xfd, 0x0d, 0x22, 0x1a, 0x74, 0x50, 0x97, 0x22, 0x76, 0xd8, 0xc0,
	0xc2, 0xfd, 0x08, 0x81, 0x05, 0xdd, 0x18, 0x21, 0x99, 0x96, 0xd6, 0x5c, 0x79, 0xe3, 0x02,
	0x81, 0xd7, 0x0e, 0x3f, 0x3b, 0x34, 0xda, 0x61, 0xc9, 0x2d, 0x84, 0x86, 0x62, 0x1e, 0x3d,
	0x5d, 0xbf, 0x92, 0x2e, 0xcd, 0x35, 0x3d, 0x6e, 0xb9, 0x59, 0x16, 0xc9, 0x82, 0x50, 0x41,
	0x30, 0x45, 0x67, 0xaa, 0xb7, 0xbe, 0xec, 0xea, 0x4b, 0x9e, 0xa0, 0xc3, 0x05, 0xbc, 0x4c,
	0x01, 0xa5, 0x4b, 0xbd, 0xa4, 0x20, 0xb5, 0x20, 0xd5, 0x59, 0x6f, 0x82, 0x5c, 0x8f, 0x4f,
	0xe0, 0x3a, 0x4e, 0x7e, 0xfe, 0x44, 0xf3, 0x3c, 0xc0, 0x0e, 0x14, 0x2b, 0x32, 0xe6, 0x28,
	0x8b, 0x63, 0x87, 0x00, 0xc3, 0x53, 0x4a, 0x5b, 0x71, 0x7a, 0x5b, 0x28, 0x40, 0xc4, 0x18,
	0xb6, 0x77, 0x0b, 0xab, 0x59, 0xa4, 0x96, 0x7d
};

static const uchar TestData_Rsa_Sp800_56b_cav_q[] = {
	0xfe, 0xab, 0xf2, 0x7c, 0x16, 0x4a, 0xf0, 0x8d, 0x31, 0xc6, 0x0a, 0x82, 0xe2, 0xae, 0xbb,
	0x03, 0x7e, 0x7b, 0x20, 0x4e, 0x64, 0xb0, 0x16, 0xad, 0x3c, 0x01, 0x1a, 0xd3, 0x54, 0xbf,
	0x2b, 0xa4, 0x02, 0x9e, 0xc3, 0x0d, 0x60, 0x3d, 0x1f, 0xb9, 0xc0, 0x0d, 0xe6, 0x97, 0x68,
	0xbb, 0x8c, 0x81, 0xd5, 0xc1, 0x54, 0x96, 0x0f, 0x99, 0xf0, 0xa8, 0xa2, 0xf3, 0xc6, 0x8e,
	0xec, 0xbc, 0x31, 0x17, 0x70, 0x98, 0x24, 0xa3, 0x36, 0x51, 0xa8, 0x54, 0xc4, 0x44, 0xdd,
	0xf7, 0x7e, 0xda, 0x47, 0x4a, 0x67, 0x44, 0x5d, 0x4e, 0x75, 0xf0, 0x4d, 0x00, 0x68, 0xe1,
	0x4a, 0xec, 0x1f, 0x45, 0xf9, 0xe6, 0xca, 0x38, 0x95, 0x48, 0x6f, 0xdc, 0x9d, 0x1b, 0xa3,
	0x4b, 0xfd, 0x08, 0x4b, 0x54, 0xcd, 0xeb, 0x3d, 0xef, 0x33, 0x11, 0x6e, 0xce, 0xe4, 0x5d,
	0xef, 0xa9, 0x58, 0x5c, 0x87, 0x4d, 0xc8, 0xcf
};

static const uchar TestData_Rsa_Sp800_56b_cav_n[] = {
	0xce, 0x5e, 0x8d, 0x1a, 0xa3, 0x08, 0x7a, 0x2d, 0xb4, 0x49, 0x48, 0xf0, 0x06, 0xb6, 0xfe,
	0xba, 0x2f, 0x39, 0x7c, 0x7b, 0xe0, 0x5d, 0x09, 0x2d, 0x57, 0x4e, 0x54, 0x60, 0x9c, 0xe5,
	0x08, 0x4b, 0xe1, 0x1a, 0x73, 0xc1, 0x5e, 0x2f, 0xb6, 0x46, 0xd7, 0x81, 0xca, 0xbc, 0x98,
	0xd2, 0xf9, 0xef, 0x1c, 0x92, 0x8c, 0x8d, 0x99, 0x85, 0x28, 0x52, 0xd6, 0xd5, 0xab, 0x70,
	0x7e, 0x9e, 0xa9, 0x87, 0x82, 0xc8, 0x95, 0x64, 0xeb, 0xf0, 0x6c, 0x0f, 0x3f, 0xe9, 0x02,
	0x29, 0x2e, 0x6d, 0xa1, 0xec, 0xbf, 0xdc, 0x23, 0xdf, 0x82, 0x4f, 0xab, 0x39, 0x8d, 0xcc,
	0xac, 0x21, 0x51, 0x14, 0xf8, 0xef, 0xec, 0x73, 0x80, 0x86, 0xa3, 0xcf, 0x8f, 0xd5, 0xcf,
	0x22, 0x1f, 0xcc, 0x23, 0x2f, 0xba, 0xcb, 0xf6, 0x17, 0xcd, 0x3a, 0x1f, 0xd9, 0x84, 0xb9,
	0x88, 0xa7, 0x78, 0x0f, 0xaa, 0xc9, 0x04, 0x01, 0x20, 0x72, 0x5d, 0x2a, 0xfe, 0x5b, 0xdd,
	0x16, 0x5a, 0xed, 0x83, 0x02, 0x96, 0x39, 0x46, 0x37, 0x30, 0xc1, 0x0d, 0x87, 0xc2, 0xc8,
	0x33, 0x38, 0xed, 0x35, 0x72, 0xe5, 0x29, 0xf8, 0x1f, 0x23, 0x60, 0xe1, 0x2a, 0x5b, 0x1d,
	0x6b, 0x53, 0x3f, 0x07, 0xc4, 0xd9, 0xbb, 0x04, 0x0c, 0x5c, 0x3f, 0x0b, 0xc4, 0xd4, 0x61,
	0x96, 0x94, 0xf1, 0x0f, 0x4a, 0x49, 0xac, 0xde, 0xd2, 0xe8, 0x42, 0xb3, 0x4a, 0x0b, 0x64,
	0x7a, 0x32, 0x5f, 0x2b, 0x5b, 0x0f, 0x8b, 0x8b, 0xe0, 0x33, 0x23, 0x34, 0x64, 0xf8, 0xb5,
	0x7f, 0x69, 0x60, 0xb8, 0x71, 0xe9, 0xff, 0x92, 0x42, 0xb1, 0xf7, 0x23, 0xa8, 0xa7, 0x92,
	0x04, 0x3d, 0x6b, 0xff, 0xf7, 0xab, 0xbb, 0x14, 0x1f, 0x4c, 0x10, 0x97, 0xd5, 0x6b, 0x71,
	0x12, 0xfd, 0x93, 0xa0, 0x4a, 0x3b, 0x75, 0x72, 0x40, 0x96, 0x1c, 0x5f, 0x40, 0x40, 0x57,
	0x13
};

static const uchar TestData_Rsa_Sp800_56b_cav_d[] = {
	0x47, 0x47, 0x49, 0x1d, 0x66, 0x2a, 0x4b, 0x68, 0xf5, 0xd8, 0x4a, 0x24, 0xfd, 0x6c, 0xbf,
	0x56, 0xb7, 0x70, 0xf7, 0x9a, 0x21, 0xc8, 0x80, 0x9e, 0xf4, 0x84, 0xcd, 0x88, 0x01, 0x28,
	0xea, 0x50, 0xab, 0x13, 0x63, 0xdf, 0xea, 0x14, 0x38, 0xb5, 0x07, 0x42, 0x81, 0x2f, 0xda,
	0xe9, 0x24, 0x02, 0x7e, 0xaf, 0xef, 0x74, 0x09, 0x0e, 0x80, 0xfa, 0xfb, 0xd1, 0x19, 0x41,
	0xe5, 0xba, 0x0f, 0x7c, 0x0a, 0xa4, 0x15, 0x55, 0xa2, 0x58, 0x8c, 0x3a, 0x48, 0x2c, 0xc6,
	0xde, 0x4a, 0x76, 0xfb, 0x72, 0xb6, 0x61, 0xe6, 0xd2, 0x10, 0x44, 0x4c, 0x33, 0xb8, 0xd2,
	0x74, 0xb1, 0x9d, 0x3b, 0xcd, 0x2f, 0xb1, 0x4f, 0xc3, 0x98, 0xbd, 0x83, 0xb7, 0x7e, 0x75,
	0xe8, 0xa7, 0x6a, 0xee, 0xcc, 0x51, 0x8c, 0x99, 0x17, 0x67, 0x7f, 0x27, 0xf9, 0x0d, 0x6a,
	0xb7, 0xd4, 0x80, 0x17, 0x89, 0x39, 0x9c, 0xf3, 0xd7, 0x0f, 0xdf, 0xb0, 0x55, 0x80, 0x1d,
	0xaf, 0x57, 0x2e, 0xd0, 0xf0, 0x4f, 0x42, 0x69, 0x55, 0xbc, 0x83, 0xd6, 0x97, 0x83, 0x7a,
	0xe6, 0xc6, 0x30, 0x6d, 0x3d, 0xb5, 0x21, 0xa7, 0xc4, 0x62, 0x0a, 0x20, 0xce, 0x5e, 0x5a,
	0x17, 0x98, 0xb3, 0x6f, 0x6b, 0x9a, 0xeb, 0x6b, 0xa3, 0xc4, 0x75, 0xd8, 0x2b, 0xdc, 0x5c,
	0x6f, 0xec, 0x5d, 0x49, 0xac, 0xa8, 0xa4, 0x2f, 0xb8, 0x8c, 0x4f, 0x2e, 0x46, 0x21, 0xee,
	0x72, 0x6a, 0x0e, 0x22, 0x80, 0x71, 0xc8, 0x76, 0x40, 0x44, 0x61, 0x16, 0xbf, 0xa5, 0xf8,
	0x89, 0xc7, 0xe9, 0x87, 0xdf, 0xbd, 0x2e, 0x4b, 0x4e, 0xc2, 0x97, 0x53, 0xe9, 0x49, 0x1c,
	0x05, 0xb0, 0x0b, 0x9b, 0x9f, 0x21, 0x19, 0x41, 0xe9, 0xf5, 0x61, 0xd7, 0x33, 0x2e, 0x2c,
	0x94, 0xb8, 0xa8, 0x9a, 0x3a, 0xcc, 0x6a, 0x24, 0x8d, 0x19, 0x13, 0xee, 0xb9, 0xb0, 0x48,
	0x61
};

static int TestData_Rsa_Sp800_56b_keygen_size[] = { 2048, 3072 };

class TestInnerBlock_Rsa_Sp800_56b {
public:
	// helper function 
	static BIGNUM * bn_load_new(const uchar * data, int sz)
	{
		BIGNUM * ret = BN_new();
		if(ret)
			BN_bin2bn(data, sz, ret);
		return ret;
	}
	/* Check that small rsa exponents are allowed in non FIPS mode */
	static int test_check_public_exponent()
	{
		int ret = 0;
		BIGNUM * e = NULL;

		ret = TEST_ptr(e = BN_new())
			/* e is too small will fail */
			&& TEST_true(BN_set_word(e, 1))
			&& TEST_false(ossl_rsa_check_public_exponent(e))
			/* e is even will fail */
			&& TEST_true(BN_set_word(e, 65536))
			&& TEST_false(ossl_rsa_check_public_exponent(e))
			/* e is ok */
			&& TEST_true(BN_set_word(e, 3))
			&& TEST_true(ossl_rsa_check_public_exponent(e))
			&& TEST_true(BN_set_word(e, 17))
			&& TEST_true(ossl_rsa_check_public_exponent(e))
			&& TEST_true(BN_set_word(e, 65537))
			&& TEST_true(ossl_rsa_check_public_exponent(e))
			/* e = 2^256 + 1 is ok */
			&& TEST_true(BN_lshift(e, BN_value_one(), 256))
			&& TEST_true(BN_add(e, e, BN_value_one()))
			&& TEST_true(ossl_rsa_check_public_exponent(e));
		BN_free(e);
		return ret;
	}
	static int test_check_prime_factor_range()
	{
		int ret = 0;
		BN_CTX * ctx = NULL;
		BIGNUM * p = NULL;
		BIGNUM * bn_p1 = NULL, * bn_p2 = NULL, * bn_p3 = NULL, * bn_p4 = NULL;
		/* Some range checks that are larger than 32 bits */
		static const uchar p1[] = { 0x0B, 0x50, 0x4F, 0x33, 0x3F };
		static const uchar p2[] = { 0x10, 0x00, 0x00, 0x00, 0x00 };
		static const uchar p3[] = { 0x0B, 0x50, 0x4F, 0x33, 0x40 };
		static const uchar p4[] = { 0x0F, 0xFF, 0xFF, 0xFF, 0xFF };

		/* (√2)(2^(nbits/2 - 1) <= p <= 2^(nbits/2) - 1
		 * For 8 bits:   0xB.504F <= p <= 0xF
		 * for 72 bits:  0xB504F333F. <= p <= 0xF_FFFF_FFFF
		 */
		ret = TEST_ptr(p = BN_new())
			&& TEST_ptr(bn_p1 = bn_load_new(p1, sizeof(p1)))
			&& TEST_ptr(bn_p2 = bn_load_new(p2, sizeof(p2)))
			&& TEST_ptr(bn_p3 = bn_load_new(p3, sizeof(p3)))
			&& TEST_ptr(bn_p4 = bn_load_new(p4, sizeof(p4)))
			&& TEST_ptr(ctx = BN_CTX_new())
			&& TEST_true(BN_set_word(p, 0xA))
			&& TEST_false(ossl_rsa_check_prime_factor_range(p, 8, ctx))
			&& TEST_true(BN_set_word(p, 0x10))
			&& TEST_false(ossl_rsa_check_prime_factor_range(p, 8, ctx))
			&& TEST_true(BN_set_word(p, 0xB))
			&& TEST_false(ossl_rsa_check_prime_factor_range(p, 8, ctx))
			&& TEST_true(BN_set_word(p, 0xC))
			&& TEST_true(ossl_rsa_check_prime_factor_range(p, 8, ctx))
			&& TEST_true(BN_set_word(p, 0xF))
			&& TEST_true(ossl_rsa_check_prime_factor_range(p, 8, ctx))
			&& TEST_false(ossl_rsa_check_prime_factor_range(bn_p1, 72, ctx))
			&& TEST_false(ossl_rsa_check_prime_factor_range(bn_p2, 72, ctx))
			&& TEST_true(ossl_rsa_check_prime_factor_range(bn_p3, 72, ctx))
			&& TEST_true(ossl_rsa_check_prime_factor_range(bn_p4, 72, ctx));

		BN_free(bn_p4);
		BN_free(bn_p3);
		BN_free(bn_p2);
		BN_free(bn_p1);
		BN_free(p);
		BN_CTX_free(ctx);
		return ret;
	}
	static int test_check_prime_factor()
	{
		BN_CTX * ctx = NULL;
		BIGNUM * p = NULL, * e = NULL;
		BIGNUM * bn_p1 = NULL, * bn_p2 = NULL, * bn_p3 = NULL;
		/* Some range checks that are larger than 32 bits */
		static const uchar p1[] = { 0x0B, 0x50, 0x4f, 0x33, 0x73 };
		static const uchar p2[] = { 0x0B, 0x50, 0x4f, 0x33, 0x75 };
		static const uchar p3[] = { 0x0F, 0x50, 0x00, 0x03, 0x75 };
		int ret = TEST_ptr(p = BN_new())
			&& TEST_ptr(bn_p1 = bn_load_new(p1, sizeof(p1)))
			&& TEST_ptr(bn_p2 = bn_load_new(p2, sizeof(p2)))
			&& TEST_ptr(bn_p3 = bn_load_new(p3, sizeof(p3)))
			&& TEST_ptr(e = BN_new())
			&& TEST_ptr(ctx = BN_CTX_new())
			/* Fails the prime test */
			&& TEST_true(BN_set_word(e, 0x1))
			&& TEST_false(ossl_rsa_check_prime_factor(bn_p1, e, 72, ctx))
			/* p is prime and in range and gcd(p-1, e) = 1 */
			&& TEST_true(ossl_rsa_check_prime_factor(bn_p2, e, 72, ctx))
			/* gcd(p-1,e) = 1 test fails */
			&& TEST_true(BN_set_word(e, 0x2))
			&& TEST_false(ossl_rsa_check_prime_factor(p, e, 72, ctx))
			/* p fails the range check */
			&& TEST_true(BN_set_word(e, 0x1))
			&& TEST_false(ossl_rsa_check_prime_factor(bn_p3, e, 72, ctx));

		BN_free(bn_p3);
		BN_free(bn_p2);
		BN_free(bn_p1);
		BN_free(e);
		BN_free(p);
		BN_CTX_free(ctx);
		return ret;
	}
	/* This test uses legacy functions because they can take invalid numbers */
	static int test_check_private_exponent()
	{
		RSA * key = NULL;
		BN_CTX * ctx = NULL;
		BIGNUM * p = NULL, * q = NULL, * e = NULL, * d = NULL, * n = NULL;
		int ret = TEST_ptr(key = RSA_new())
			&& TEST_ptr(ctx = BN_CTX_new())
			&& TEST_ptr(p = BN_new())
			&& TEST_ptr(q = BN_new())
			/* lcm(15-1,17-1) = 14*16 / 2 = 112 */
			&& TEST_true(BN_set_word(p, 15))
			&& TEST_true(BN_set_word(q, 17))
			&& TEST_true(RSA_set0_factors(key, p, q));
		if(!ret) {
			BN_free(p);
			BN_free(q);
			goto end;
		}
		ret = TEST_ptr(e = BN_new())
			&& TEST_ptr(d = BN_new())
			&& TEST_ptr(n = BN_new())
			&& TEST_true(BN_set_word(e, 5))
			&& TEST_true(BN_set_word(d, 157))
			&& TEST_true(BN_set_word(n, 15*17))
			&& TEST_true(RSA_set0_key(key, n, e, d));
		if(!ret) {
			BN_free(e);
			BN_free(d);
			BN_free(n);
			goto end;
		}
		/* fails since d >= lcm(p-1, q-1) */
		ret = TEST_false(ossl_rsa_check_private_exponent(key, 8, ctx))
			&& TEST_true(BN_set_word(d, 45))
			/* d is correct size and 1 = e.d mod lcm(p-1, q-1) */
			&& TEST_true(ossl_rsa_check_private_exponent(key, 8, ctx))
			/* d is too small compared to nbits */
			&& TEST_false(ossl_rsa_check_private_exponent(key, 16, ctx))
			/* d is too small compared to nbits */
			&& TEST_true(BN_set_word(d, 16))
			&& TEST_false(ossl_rsa_check_private_exponent(key, 8, ctx))
			/* fail if 1 != e.d mod lcm(p-1, q-1) */
			&& TEST_true(BN_set_word(d, 46))
			&& TEST_false(ossl_rsa_check_private_exponent(key, 8, ctx));
	end:
		RSA_free(key);
		BN_CTX_free(ctx);
		return ret;
	}
	static int test_check_crt_components()
	{
		const int P = 15;
		const int Q = 17;
		const int E = 5;
		const int N = P*Q;
		const int DP = 3;
		const int DQ = 13;
		const int QINV = 8;
		RSA * key = NULL;
		BN_CTX * ctx = NULL;
		BIGNUM * p = NULL, * q = NULL, * e = NULL;
		int ret = TEST_ptr(key = RSA_new())
			&& TEST_ptr(ctx = BN_CTX_new())
			&& TEST_ptr(p = BN_new())
			&& TEST_ptr(q = BN_new())
			&& TEST_ptr(e = BN_new())
			&& TEST_true(BN_set_word(p, P))
			&& TEST_true(BN_set_word(q, Q))
			&& TEST_true(BN_set_word(e, E))
			&& TEST_true(RSA_set0_factors(key, p, q));
		if(!ret) {
			BN_free(p);
			BN_free(q);
			goto end;
		}
		ret = TEST_true(ossl_rsa_sp800_56b_derive_params_from_pq(key, 8, e, ctx))
			&& TEST_BN_eq_word(key->n, N)
			&& TEST_BN_eq_word(key->dmp1, DP)
			&& TEST_BN_eq_word(key->dmq1, DQ)
			&& TEST_BN_eq_word(key->iqmp, QINV)
			&& TEST_true(ossl_rsa_check_crt_components(key, ctx))
			/* (a) 1 < dP < (p – 1). */
			&& TEST_true(BN_set_word(key->dmp1, 1))
			&& TEST_false(ossl_rsa_check_crt_components(key, ctx))
			&& TEST_true(BN_set_word(key->dmp1, P-1))
			&& TEST_false(ossl_rsa_check_crt_components(key, ctx))
			&& TEST_true(BN_set_word(key->dmp1, DP))
			/* (b) 1 < dQ < (q - 1). */
			&& TEST_true(BN_set_word(key->dmq1, 1))
			&& TEST_false(ossl_rsa_check_crt_components(key, ctx))
			&& TEST_true(BN_set_word(key->dmq1, Q-1))
			&& TEST_false(ossl_rsa_check_crt_components(key, ctx))
			&& TEST_true(BN_set_word(key->dmq1, DQ))
			/* (c) 1 < qInv < p */
			&& TEST_true(BN_set_word(key->iqmp, 1))
			&& TEST_false(ossl_rsa_check_crt_components(key, ctx))
			&& TEST_true(BN_set_word(key->iqmp, P))
			&& TEST_false(ossl_rsa_check_crt_components(key, ctx))
			&& TEST_true(BN_set_word(key->iqmp, QINV))
			/* (d) 1 = (dP . e) mod (p - 1)*/
			&& TEST_true(BN_set_word(key->dmp1, DP+1))
			&& TEST_false(ossl_rsa_check_crt_components(key, ctx))
			&& TEST_true(BN_set_word(key->dmp1, DP))
			/* (e) 1 = (dQ . e) mod (q - 1) */
			&& TEST_true(BN_set_word(key->dmq1, DQ-1))
			&& TEST_false(ossl_rsa_check_crt_components(key, ctx))
			&& TEST_true(BN_set_word(key->dmq1, DQ))
			/* (f) 1 = (qInv . q) mod p */
			&& TEST_true(BN_set_word(key->iqmp, QINV+1))
			&& TEST_false(ossl_rsa_check_crt_components(key, ctx))
			&& TEST_true(BN_set_word(key->iqmp, QINV))
			/* check defaults are still valid */
			&& TEST_true(ossl_rsa_check_crt_components(key, ctx));
	end:
		BN_free(e);
		RSA_free(key);
		BN_CTX_free(ctx);
		return ret;
	}
	static int test_pq_diff()
	{
		BIGNUM * tmp = NULL, * p = NULL, * q = NULL;
		int ret = TEST_ptr(tmp = BN_new())
			&& TEST_ptr(p = BN_new())
			&& TEST_ptr(q = BN_new())
			/* |1-(2+1)| > 2^1 */
			&& TEST_true(BN_set_word(p, 1))
			&& TEST_true(BN_set_word(q, 1+2))
			&& TEST_false(ossl_rsa_check_pminusq_diff(tmp, p, q, 202))
			/* Check |p - q| > 2^(nbits/2 - 100) */
			&& TEST_true(BN_set_word(q, 1+3))
			&& TEST_true(ossl_rsa_check_pminusq_diff(tmp, p, q, 202))
			&& TEST_true(BN_set_word(p, 1+3))
			&& TEST_true(BN_set_word(q, 1))
			&& TEST_true(ossl_rsa_check_pminusq_diff(tmp, p, q, 202));
		BN_free(p);
		BN_free(q);
		BN_free(tmp);
		return ret;
	}
	static int test_invalid_keypair()
	{
		RSA * key = NULL;
		BN_CTX * ctx = NULL;
		BIGNUM * p = NULL, * q = NULL, * n = NULL, * e = NULL, * d = NULL;
		int ret = TEST_ptr(key = RSA_new())
			&& TEST_ptr(ctx = BN_CTX_new())
			/* NULL parameters */
			&& TEST_false(ossl_rsa_sp800_56b_check_keypair(key, NULL, -1, 2048))
			/* load key */
			&& TEST_ptr(p = bn_load_new(TestData_Rsa_Sp800_56b_cav_p, sizeof(TestData_Rsa_Sp800_56b_cav_p)))
			&& TEST_ptr(q = bn_load_new(TestData_Rsa_Sp800_56b_cav_q, sizeof(TestData_Rsa_Sp800_56b_cav_q)))
			&& TEST_true(RSA_set0_factors(key, p, q));
		if(!ret) {
			BN_free(p);
			BN_free(q);
			goto end;
		}
		ret = TEST_ptr(e = bn_load_new(TestData_Rsa_Sp800_56b_cav_e, sizeof(TestData_Rsa_Sp800_56b_cav_e)))
			&& TEST_ptr(n = bn_load_new(TestData_Rsa_Sp800_56b_cav_n, sizeof(TestData_Rsa_Sp800_56b_cav_n)))
			&& TEST_ptr(d = bn_load_new(TestData_Rsa_Sp800_56b_cav_d, sizeof(TestData_Rsa_Sp800_56b_cav_d)))
			&& TEST_true(RSA_set0_key(key, n, e, d));
		if(!ret) {
			BN_free(e);
			BN_free(n);
			BN_free(d);
			goto end;
		}
		/* bad strength/key size */
		ret = TEST_false(ossl_rsa_sp800_56b_check_keypair(key, NULL, 100, 2048))
			&& TEST_false(ossl_rsa_sp800_56b_check_keypair(key, NULL, 112, 1024))
			&& TEST_false(ossl_rsa_sp800_56b_check_keypair(key, NULL, 128, 2048))
			&& TEST_false(ossl_rsa_sp800_56b_check_keypair(key, NULL, 140, 3072))
			/* mismatching exponent */
			&& TEST_false(ossl_rsa_sp800_56b_check_keypair(key, BN_value_one(),
			-1, 2048))
			/* bad exponent */
			&& TEST_true(BN_add_word(e, 1))
			&& TEST_false(ossl_rsa_sp800_56b_check_keypair(key, NULL, -1, 2048))
			&& TEST_true(BN_sub_word(e, 1))

			/* mismatch between bits and modulus */
			&& TEST_false(ossl_rsa_sp800_56b_check_keypair(key, NULL, -1, 3072))
			&& TEST_true(ossl_rsa_sp800_56b_check_keypair(key, e, 112, 2048))
			/* check n == pq failure */
			&& TEST_true(BN_add_word(n, 1))
			&& TEST_false(ossl_rsa_sp800_56b_check_keypair(key, NULL, -1, 2048))
			&& TEST_true(BN_sub_word(n, 1))
			/* check p  */
			&& TEST_true(BN_sub_word(p, 2))
			&& TEST_true(BN_mul(n, p, q, ctx))
			&& TEST_false(ossl_rsa_sp800_56b_check_keypair(key, NULL, -1, 2048))
			&& TEST_true(BN_add_word(p, 2))
			&& TEST_true(BN_mul(n, p, q, ctx))
			/* check q  */
			&& TEST_true(BN_sub_word(q, 2))
			&& TEST_true(BN_mul(n, p, q, ctx))
			&& TEST_false(ossl_rsa_sp800_56b_check_keypair(key, NULL, -1, 2048))
			&& TEST_true(BN_add_word(q, 2))
			&& TEST_true(BN_mul(n, p, q, ctx));
	end:
		RSA_free(key);
		BN_CTX_free(ctx);
		return ret;
	}
	static int test_sp80056b_keygen(int id)
	{
		RSA * key = NULL;
		int sz = TestData_Rsa_Sp800_56b_keygen_size[id];
		int ret = TEST_ptr(key = RSA_new())
			&& TEST_true(ossl_rsa_sp800_56b_generate_key(key, sz, NULL, NULL))
			&& TEST_true(ossl_rsa_sp800_56b_check_public(key))
			&& TEST_true(ossl_rsa_sp800_56b_check_private(key))
			&& TEST_true(ossl_rsa_sp800_56b_check_keypair(key, NULL, -1, sz));
		RSA_free(key);
		return ret;
	}
	static int test_check_private_key()
	{
		BIGNUM * n = NULL, * d = NULL, * e = NULL;
		RSA * key = NULL;
		int ret = TEST_ptr(key = RSA_new())
			/* check NULL pointers fail */
			&& TEST_false(ossl_rsa_sp800_56b_check_private(key))
			/* load private key */
			&& TEST_ptr(n = bn_load_new(TestData_Rsa_Sp800_56b_cav_n, sizeof(TestData_Rsa_Sp800_56b_cav_n)))
			&& TEST_ptr(d = bn_load_new(TestData_Rsa_Sp800_56b_cav_d, sizeof(TestData_Rsa_Sp800_56b_cav_d)))
			&& TEST_ptr(e = bn_load_new(TestData_Rsa_Sp800_56b_cav_e, sizeof(TestData_Rsa_Sp800_56b_cav_e)))
			&& TEST_true(RSA_set0_key(key, n, e, d));
		if(!ret) {
			BN_free(n);
			BN_free(e);
			BN_free(d);
			goto end;
		}
		/* check d is in range */
		ret = TEST_true(ossl_rsa_sp800_56b_check_private(key))
			/* check d is too low */
			&& TEST_true(BN_set_word(d, 0))
			&& TEST_false(ossl_rsa_sp800_56b_check_private(key))
			/* check d is too high */
			&& TEST_ptr(BN_copy(d, n))
			&& TEST_false(ossl_rsa_sp800_56b_check_private(key));
	end:
		RSA_free(key);
		return ret;
	}
	static int test_check_public_key()
	{
		BIGNUM * n = NULL, * e = NULL;
		RSA * key = NULL;
		int ret = TEST_ptr(key = RSA_new())
			/* check NULL pointers fail */
			&& TEST_false(ossl_rsa_sp800_56b_check_public(key))
			/* load public key */
			&& TEST_ptr(e = bn_load_new(TestData_Rsa_Sp800_56b_cav_e, sizeof(TestData_Rsa_Sp800_56b_cav_e)))
			&& TEST_ptr(n = bn_load_new(TestData_Rsa_Sp800_56b_cav_n, sizeof(TestData_Rsa_Sp800_56b_cav_n)))
			&& TEST_true(RSA_set0_key(key, n, e, NULL));
		if(!ret) {
			BN_free(e);
			BN_free(n);
			goto end;
		}
		/* check public key is valid */
		ret = TEST_true(ossl_rsa_sp800_56b_check_public(key))
			/* check fail if n is even */
			&& TEST_true(BN_add_word(n, 1))
			&& TEST_false(ossl_rsa_sp800_56b_check_public(key))
			&& TEST_true(BN_sub_word(n, 1))
			/* check fail if n is wrong number of bits */
			&& TEST_true(BN_lshift1(n, n))
			&& TEST_false(ossl_rsa_sp800_56b_check_public(key))
			&& TEST_true(BN_rshift1(n, n))
			/* test odd exponent fails */
			&& TEST_true(BN_add_word(e, 1))
			&& TEST_false(ossl_rsa_sp800_56b_check_public(key))
			&& TEST_true(BN_sub_word(e, 1))
			/* modulus fails composite check */
			&& TEST_true(BN_add_word(n, 2))
			&& TEST_false(ossl_rsa_sp800_56b_check_public(key));
	end:
		RSA_free(key);
		return ret;
	}
};
//
//
//
static fake_random_generate_cb TestData_Sm2Internal_get_faked_bytes;
static OSSL_PROVIDER * TestData_Sm2Internal_fake_rand = NULL;
static uint8_t * TestData_Sm2Internal_fake_rand_bytes = NULL;
static size_t TestData_Sm2Internal_fake_rand_bytes_offset = 0;
static size_t TestData_Sm2Internal_fake_rand_size = 0;

class TestInnerBlock_Sm2Internal {;
public:
	static int get_faked_bytes(uchar * buf, size_t num, ossl_unused const char * name, ossl_unused EVP_RAND_CTX * ctx)
	{
		if(!TEST_ptr(TestData_Sm2Internal_fake_rand_bytes) || !TEST_size_t_gt(TestData_Sm2Internal_fake_rand_size, 0))
			return 0;
		while(num-- > 0) {
			if(TestData_Sm2Internal_fake_rand_bytes_offset >= TestData_Sm2Internal_fake_rand_size)
				TestData_Sm2Internal_fake_rand_bytes_offset = 0;
			*buf++ = TestData_Sm2Internal_fake_rand_bytes[TestData_Sm2Internal_fake_rand_bytes_offset++];
		}
		return 1;
	}
	static int start_fake_rand(const char * hex_bytes)
	{
		OPENSSL_free(TestData_Sm2Internal_fake_rand_bytes);
		TestData_Sm2Internal_fake_rand_bytes_offset = 0;
		TestData_Sm2Internal_fake_rand_size = strlen(hex_bytes) / 2;
		if(!TEST_ptr(TestData_Sm2Internal_fake_rand_bytes = OPENSSL_hexstr2buf(hex_bytes, NULL)))
			return 0;
		// use own random function
		fake_rand_set_public_private_callbacks(NULL, get_faked_bytes);
		return 1;
	}
	static void restore_rand()
	{
		fake_rand_set_public_private_callbacks(NULL, NULL);
		OPENSSL_free(TestData_Sm2Internal_fake_rand_bytes);
		TestData_Sm2Internal_fake_rand_bytes = NULL;
		TestData_Sm2Internal_fake_rand_bytes_offset = 0;
	}
	static EC_GROUP * create_EC_group(const char * p_hex, const char * a_hex, const char * b_hex, const char * x_hex,
		const char * y_hex, const char * order_hex, const char * cof_hex)
	{
		BIGNUM * p = NULL;
		BIGNUM * a = NULL;
		BIGNUM * b = NULL;
		BIGNUM * g_x = NULL;
		BIGNUM * g_y = NULL;
		BIGNUM * order = NULL;
		BIGNUM * cof = NULL;
		EC_POINT * generator = NULL;
		EC_GROUP * group = NULL;
		int ok = 0;
		if(!TEST_true(BN_hex2bn(&p, p_hex)) || !TEST_true(BN_hex2bn(&a, a_hex)) || !TEST_true(BN_hex2bn(&b, b_hex)))
			goto done;
		group = EC_GROUP_new_curve_GFp(p, a, b, NULL);
		if(!TEST_ptr(group))
			goto done;
		generator = EC_POINT_new(group);
		if(!TEST_ptr(generator))
			goto done;
		if(!TEST_true(BN_hex2bn(&g_x, x_hex)) || !TEST_true(BN_hex2bn(&g_y, y_hex)) || !TEST_true(EC_POINT_set_affine_coordinates(group, generator, g_x, g_y, NULL)))
			goto done;
		if(!TEST_true(BN_hex2bn(&order, order_hex)) || !TEST_true(BN_hex2bn(&cof, cof_hex)) || !TEST_true(EC_GROUP_set_generator(group, generator, order, cof)))
			goto done;
		ok = 1;
	done:
		BN_free(p);
		BN_free(a);
		BN_free(b);
		BN_free(g_x);
		BN_free(g_y);
		EC_POINT_free(generator);
		BN_free(order);
		BN_free(cof);
		if(!ok) {
			EC_GROUP_free(group);
			group = NULL;
		}
		return group;
	}
	static int test_sm2_crypt(const EC_GROUP * group, const EVP_MD * digest, const char * privkey_hex, const char * message, const char * k_hex, const char * ctext_hex)
	{
		const size_t msg_len = strlen(message);
		BIGNUM * priv = NULL;
		EC_KEY * key = NULL;
		EC_POINT * pt = NULL;
		uchar * expected = OPENSSL_hexstr2buf(ctext_hex, NULL);
		size_t ctext_len = 0;
		size_t ptext_len = 0;
		uint8_t * ctext = NULL;
		uint8_t * recovered = NULL;
		size_t recovered_len = msg_len;
		int rc = 0;

		if(!TEST_ptr(expected)
			|| !TEST_true(BN_hex2bn(&priv, privkey_hex)))
			goto done;

		key = EC_KEY_new();
		if(!TEST_ptr(key)
			|| !TEST_true(EC_KEY_set_group(key, group))
			|| !TEST_true(EC_KEY_set_private_key(key, priv)))
			goto done;

		pt = EC_POINT_new(group);
		if(!TEST_ptr(pt)
			|| !TEST_true(EC_POINT_mul(group, pt, priv, NULL, NULL, NULL))
			|| !TEST_true(EC_KEY_set_public_key(key, pt))
			|| !TEST_true(ossl_sm2_ciphertext_size(key, digest, msg_len,
			&ctext_len)))
			goto done;
		ctext = (uint8_t *)OPENSSL_zalloc(ctext_len);
		if(!TEST_ptr(ctext))
			goto done;
		start_fake_rand(k_hex);
		if(!TEST_true(ossl_sm2_encrypt(key, digest,
			(const uint8_t*)message, msg_len,
			ctext, &ctext_len))) {
			restore_rand();
			goto done;
		}
		restore_rand();
		if(!TEST_mem_eq(ctext, ctext_len, expected, ctext_len))
			goto done;
		if(!TEST_true(ossl_sm2_plaintext_size(ctext, ctext_len, &ptext_len)) || !TEST_int_eq(ptext_len, msg_len))
			goto done;
		recovered = (uint8_t *)OPENSSL_zalloc(ptext_len);
		if(!TEST_ptr(recovered)
			|| !TEST_true(ossl_sm2_decrypt(key, digest, ctext, ctext_len,
			recovered, &recovered_len))
			|| !TEST_int_eq(recovered_len, msg_len)
			|| !TEST_mem_eq(recovered, recovered_len, message, msg_len))
			goto done;

		rc = 1;
	done:
		BN_free(priv);
		EC_POINT_free(pt);
		OPENSSL_free(ctext);
		OPENSSL_free(recovered);
		OPENSSL_free(expected);
		EC_KEY_free(key);
		return rc;
	}

	static int sm2_crypt_test()
	{
		int testresult = 0;
		EC_GROUP * gm_group = NULL;
		EC_GROUP * test_group = create_EC_group("8542D69E4C044F18E8B92435BF6FF7DE457283915C45517D722EDB8B08F1DFC3",
			"787968B4FA32C3FD2417842E73BBFEFF2F3C848B6831D7E0EC65228B3937E498",
			"63E4C6D3B23B0C849CF84241484BFE48F61D59A5B16BA06E6E12D1DA27C5249A",
			"421DEBD61B62EAB6746434EBC3CC315E32220B3BADD50BDC4C4E6C147FEDD43D",
			"0680512BCBB42C07D47349D2153B70C4E5D7FDFCBFA36EA1A85841B9E46E09A2",
			"8542D69E4C044F18E8B92435BF6FF7DD297720630485628D5AE74EE7C32E79B7",
			"1");
		if(!TEST_ptr(test_group))
			goto done;
		if(!test_sm2_crypt(test_group, EVP_sm3(),
				"1649AB77A00637BD5E2EFE283FBF353534AA7F7CB89463F208DDBC2920BB0DA0",
				"encryption standard",
				"004C62EEFD6ECFC2B95B92FD6C3D9575148AFA17425546D49018E5388D49DD7B4F"
				"0092e8ff62146873c258557548500ab2df2a365e0609ab67640a1f6d57d7b17820"
				"008349312695a3e1d2f46905f39a766487f2432e95d6be0cb009fe8c69fd8825a7",
				"307B0220245C26FB68B1DDDDB12C4B6BF9F2B6D5FE60A383B0D18D1C4144ABF1"
				"7F6252E7022076CB9264C2A7E88E52B19903FDC47378F605E36811F5C07423A2"
				"4B84400F01B804209C3D7360C30156FAB7C80A0276712DA9D8094A634B766D3A"
				"285E07480653426D0413650053A89B41C418B0C3AAD00D886C00286467"))
			goto done;

		/* Same test as above except using SHA-256 instead of SM3 */
		if(!test_sm2_crypt(test_group, EVP_sha256(),
				"1649AB77A00637BD5E2EFE283FBF353534AA7F7CB89463F208DDBC2920BB0DA0",
				"encryption standard",
				"004C62EEFD6ECFC2B95B92FD6C3D9575148AFA17425546D49018E5388D49DD7B4F"
				"003da18008784352192d70f22c26c243174a447ba272fec64163dd4742bae8bc98"
				"00df17605cf304e9dd1dfeb90c015e93b393a6f046792f790a6fa4228af67d9588",
				"307B0220245C26FB68B1DDDDB12C4B6BF9F2B6D5FE60A383B0D18D1C4144ABF17F"
				"6252E7022076CB9264C2A7E88E52B19903FDC47378F605E36811F5C07423A24B84"
				"400F01B80420BE89139D07853100EFA763F60CBE30099EA3DF7F8F364F9D10A5E9"
				"88E3C5AAFC0413229E6C9AEE2BB92CAD649FE2C035689785DA33"))
			goto done;

		/* From Annex C in both GM/T0003.5-2012 and GB/T 32918.5-2016.*/
		gm_group = create_EC_group(
			"fffffffeffffffffffffffffffffffffffffffff00000000ffffffffffffffff",
			"fffffffeffffffffffffffffffffffffffffffff00000000fffffffffffffffc",
			"28e9fa9e9d9f5e344d5a9e4bcf6509a7f39789f515ab8f92ddbcbd414d940e93",
			"32c4ae2c1f1981195f9904466a39c9948fe30bbff2660be1715a4589334c74c7",
			"bc3736a2f4f6779c59bdcee36b692153d0a9877cc62a474002df32e52139f0a0",
			"fffffffeffffffffffffffffffffffff7203df6b21c6052b53bbf40939d54123",
			"1");

		if(!TEST_ptr(gm_group))
			goto done;

		if(!test_sm2_crypt(
				gm_group,
				EVP_sm3(),
				/* privkey (from which the encrypting public key is derived) */
				"3945208F7B2144B13F36E38AC6D39F95889393692860B51A42FB81EF4DF7C5B8",
				/* plaintext message */
				"encryption standard",
				/* ephemeral nonce k */
				"59276E27D506861A16680F3AD9C02DCCEF3CC1FA3CDBE4CE6D54B80DEAC1BC21",
				/*
				 * expected ciphertext, the field values are from GM/T 0003.5-2012
				 * (Annex C), but serialized following the ASN.1 format specified
				 * in GM/T 0009-2012 (Sec. 7.2).
				 */
				"307C" /* SEQUENCE, 0x7c bytes */
				"0220" /* INTEGER, 0x20 bytes */
				"04EBFC718E8D1798620432268E77FEB6415E2EDE0E073C0F4F640ECD2E149A73"
				"0221" /* INTEGER, 0x21 bytes */
				"00" /* leading 00 due to DER for pos. int with topmost bit set */
				"E858F9D81E5430A57B36DAAB8F950A3C64E6EE6A63094D99283AFF767E124DF0"
				"0420" /* OCTET STRING, 0x20 bytes */
				"59983C18F809E262923C53AEC295D30383B54E39D609D160AFCB1908D0BD8766"
				"0413" /* OCTET STRING, 0x13 bytes */
				"21886CA989CA9C7D58087307CA93092D651EFA"))
			goto done;

		testresult = 1;
	done:
		EC_GROUP_free(test_group);
		EC_GROUP_free(gm_group);

		return testresult;
	}
	static int test_sm2_sign(const EC_GROUP * group, const char * userid, const char * privkey_hex, const char * message, const char * k_hex, const char * r_hex, const char * s_hex)
	{
		const size_t msg_len = strlen(message);
		int ok = 0;
		BIGNUM * priv = NULL;
		EC_POINT * pt = NULL;
		EC_KEY * key = NULL;
		ECDSA_SIG * sig = NULL;
		const BIGNUM * sig_r = NULL;
		const BIGNUM * sig_s = NULL;
		BIGNUM * r = NULL;
		BIGNUM * s = NULL;
		if(!TEST_true(BN_hex2bn(&priv, privkey_hex)))
			goto done;
		key = EC_KEY_new();
		if(!TEST_ptr(key) || !TEST_true(EC_KEY_set_group(key, group)) || !TEST_true(EC_KEY_set_private_key(key, priv)))
			goto done;
		pt = EC_POINT_new(group);
		if(!TEST_ptr(pt) || !TEST_true(EC_POINT_mul(group, pt, priv, NULL, NULL, NULL)) || !TEST_true(EC_KEY_set_public_key(key, pt)))
			goto done;
		start_fake_rand(k_hex);
		sig = ossl_sm2_do_sign(key, EVP_sm3(), (const uint8_t*)userid, strlen(userid), (const uint8_t*)message, msg_len);
		if(!TEST_ptr(sig)) {
			restore_rand();
			goto done;
		}
		restore_rand();
		ECDSA_SIG_get0(sig, &sig_r, &sig_s);
		if(!TEST_true(BN_hex2bn(&r, r_hex)) || !TEST_true(BN_hex2bn(&s, s_hex)) || !TEST_BN_eq(r, sig_r) || !TEST_BN_eq(s, sig_s))
			goto done;
		ok = ossl_sm2_do_verify(key, EVP_sm3(), sig, (const uint8_t*)userid, strlen(userid), (const uint8_t*)message, msg_len);
		/* We goto done whether this passes or fails */
		TEST_true(ok);
	done:
		ECDSA_SIG_free(sig);
		EC_POINT_free(pt);
		EC_KEY_free(key);
		BN_free(priv);
		BN_free(r);
		BN_free(s);
		return ok;
	}

	static int sm2_sig_test()
	{
		int testresult = 0;
		/* From draft-shen-sm2-ecdsa-02 */
		EC_GROUP * test_group =
			create_EC_group
				("8542D69E4C044F18E8B92435BF6FF7DE457283915C45517D722EDB8B08F1DFC3",
			"787968B4FA32C3FD2417842E73BBFEFF2F3C848B6831D7E0EC65228B3937E498",
			"63E4C6D3B23B0C849CF84241484BFE48F61D59A5B16BA06E6E12D1DA27C5249A",
			"421DEBD61B62EAB6746434EBC3CC315E32220B3BADD50BDC4C4E6C147FEDD43D",
			"0680512BCBB42C07D47349D2153B70C4E5D7FDFCBFA36EA1A85841B9E46E09A2",
			"8542D69E4C044F18E8B92435BF6FF7DD297720630485628D5AE74EE7C32E79B7",
			"1");

		if(!TEST_ptr(test_group))
			goto done;
		if(!TEST_true(test_sm2_sign(test_group,
				"ALICE123@YAHOO.COM",
				"128B2FA8BD433C6C068C8D803DFF79792A519A55171B1B650C23661D15897263",
				"message digest",
				"006CB28D99385C175C94F94E934817663FC176D925DD72B727260DBAAE1FB2F96F"
				"007c47811054c6f99613a578eb8453706ccb96384fe7df5c171671e760bfa8be3a",
				"40F1EC59F793D9F49E09DCEF49130D4194F79FB1EED2CAA55BACDB49C4E755D1",
				"6FC6DAC32C5D5CF10C77DFB20F7C2EB667A457872FB09EC56327A67EC7DEEBE7")))
			goto done;

		testresult = 1;
	done:
		EC_GROUP_free(test_group);
		return testresult;
	}
};
//#endif
//
//
// The macros below generate unused functions which error out one of the clang
// builds.  We disable this check here.
#ifdef __clang__
	#pragma clang diagnostic ignored "-Wunused-function"
#endif

DEFINE_SPARSE_ARRAY_OF(char);

class TestInnerBlock_SparseArray {
public:
	static int test_sparse_array()
	{
		static const struct {
			ossl_uintmax_t n;
			char * v;
		} cases[] = {
			{ 22, "a" }, { 0, "z" }, { 1, "b" }, { 290, "c" },
			{ INT_MAX, "m" }, { 6666666, "d" }, { (ossl_uintmax_t)-1, "H" },
			{ 99, "e" }
		};
		SPARSE_ARRAY_OF(char) *sa;
		size_t i, j;
		int res = 0;
		if(!TEST_ptr(sa = ossl_sa_char_new())
			|| !TEST_ptr_null(ossl_sa_char_get(sa, 3))
			|| !TEST_ptr_null(ossl_sa_char_get(sa, 0))
			|| !TEST_ptr_null(ossl_sa_char_get(sa, UINT_MAX)))
			goto err;
		for(i = 0; i < SIZEOFARRAY(cases); i++) {
			if(!TEST_true(ossl_sa_char_set(sa, cases[i].n, cases[i].v))) {
				TEST_note("iteration %zu", i + 1);
				goto err;
			}
			for(j = 0; j <= i; j++)
				if(!TEST_str_eq(ossl_sa_char_get(sa, cases[j].n), cases[j].v)) {
					TEST_note("iteration %zu / %zu", i + 1, j + 1);
					goto err;
				}
		}
		res = 1;
	err:
		ossl_sa_char_free(sa);
		return res;
	}
	static int test_sparse_array_num()
	{
		static const struct {
			size_t num;
			ossl_uintmax_t n;
			char * v;
		} cases[] = {
			{ 1, 22, "a" }, { 2, 1021, "b" }, { 3, 3, "c" }, { 2, 22, NULL },
			{ 2, 3, "d" }, { 3, 22, "e" }, { 3, 666, NULL }, { 4, 666, "f" },
			{ 3, 3, NULL }, { 2, 22, NULL }, { 1, 666, NULL }, { 2, 64000, "g" },
			{ 1, 1021, NULL }, { 0, 64000, NULL }, { 1, 23, "h" }, { 0, 23, NULL }
		};
		SPARSE_ARRAY_OF(char) *sa = NULL;
		size_t i;
		int res = 0;
		if(!TEST_size_t_eq(ossl_sa_char_num(NULL), 0) || !TEST_ptr(sa = ossl_sa_char_new()) || !TEST_size_t_eq(ossl_sa_char_num(sa), 0))
			goto err;
		for(i = 0; i < SIZEOFARRAY(cases); i++)
			if(!TEST_true(ossl_sa_char_set(sa, cases[i].n, cases[i].v)) || !TEST_size_t_eq(ossl_sa_char_num(sa), cases[i].num))
				goto err;
		res = 1;
	err:
		ossl_sa_char_free(sa);
		return res;
	}

	struct index_cases_st {
		ossl_uintmax_t n;
		char * v;
		int del;
	};

	struct doall_st {
		SPARSE_ARRAY_OF(char) *sa;
		size_t num_cases;
		const struct index_cases_st * cases;
		int res;
		int all;
	};

	static void leaf_check_all(ossl_uintmax_t n, char * value, void * arg)
	{
		struct doall_st * doall_data = (struct doall_st *)arg;
		const struct index_cases_st * cases = doall_data->cases;
		size_t i;
		doall_data->res = 0;
		for(i = 0; i < doall_data->num_cases; i++)
			if((doall_data->all || !cases[i].del) && n == cases[i].n && strcmp(value, cases[i].v) == 0) {
				doall_data->res = 1;
				return;
			}
		TEST_error("Index %ju with value %s not found", n, value);
	}
	static void leaf_delete(ossl_uintmax_t n, char * value, void * arg)
	{
		struct doall_st * doall_data = (struct doall_st *)arg;
		const struct index_cases_st * cases = doall_data->cases;
		size_t i;
		doall_data->res = 0;
		for(i = 0; i < doall_data->num_cases; i++)
			if(n == cases[i].n && strcmp(value, cases[i].v) == 0) {
				doall_data->res = 1;
				ossl_sa_char_set(doall_data->sa, n, NULL);
				return;
			}
		TEST_error("Index %ju with value %s not found", n, value);
	}
	static int test_sparse_array_doall()
	{
		static const struct index_cases_st cases[] = {
			{ 22, "A", 1 }, { 1021, "b", 0 }, { 3, "c", 0 }, { INT_MAX, "d", 1 },
			{ (ossl_uintmax_t)-1, "H", 0 }, { (ossl_uintmax_t)-2, "i", 1 },
			{ 666666666, "s", 1 }, { 1234567890, "t", 0 },
		};
		struct doall_st doall_data;
		size_t i;
		SPARSE_ARRAY_OF(char) *sa = NULL;
		int res = 0;

		if(!TEST_ptr(sa = ossl_sa_char_new()))
			goto err;
		doall_data.num_cases = SIZEOFARRAY(cases);
		doall_data.cases = cases;
		doall_data.all = 1;
		doall_data.sa = NULL;
		for(i = 0; i <  SIZEOFARRAY(cases); i++)
			if(!TEST_true(ossl_sa_char_set(sa, cases[i].n, cases[i].v))) {
				TEST_note("failed at iteration %zu", i + 1);
				goto err;
			}

		ossl_sa_char_doall_arg(sa, &leaf_check_all, &doall_data);
		if(doall_data.res == 0) {
			TEST_info("while checking all elements");
			goto err;
		}
		doall_data.all = 0;
		doall_data.sa = sa;
		ossl_sa_char_doall_arg(sa, &leaf_delete, &doall_data);
		if(doall_data.res == 0) {
			TEST_info("while deleting selected elements");
			goto err;
		}
		ossl_sa_char_doall_arg(sa, &leaf_check_all, &doall_data);
		if(doall_data.res == 0) {
			TEST_info("while checking for deleted elements");
			goto err;
		}
		res = 1;

	err:
		ossl_sa_char_free(sa);
		return res;
	}
};
//
//
//
//
// Internal tests for the poly1305 module
//
typedef struct {
	size_t size;
	const uchar data[1024];
} Poly1305Internal_SIZED_DATA;

typedef struct {
	Poly1305Internal_SIZED_DATA input;
	Poly1305Internal_SIZED_DATA key;
	Poly1305Internal_SIZED_DATA expected;
} Poly1305Internal_TESTDATA;
// 
// Test of poly1305 internal functions
// 
static Poly1305Internal_TESTDATA TestData_Poly1305Internal_tests[] = {
	/*
	 * RFC7539
	 */
	{
		{
			34,
			{
				0x43, 0x72, 0x79, 0x70, 0x74, 0x6f, 0x67, 0x72,
				0x61, 0x70, 0x68, 0x69, 0x63, 0x20, 0x46, 0x6f,
				0x72, 0x75, 0x6d, 0x20, 0x52, 0x65, 0x73, 0x65,
				0x61, 0x72, 0x63, 0x68, 0x20, 0x47, 0x72, 0x6f,

				0x75, 0x70
			}
		},
		{
			32,
			{
				0x85, 0xd6, 0xbe, 0x78, 0x57, 0x55, 0x6d, 0x33,
				0x7f, 0x44, 0x52, 0xfe, 0x42, 0xd5, 0x06, 0xa8,
				0x01, 0x03, 0x80, 0x8a, 0xfb, 0x0d, 0xb2, 0xfd,
				0x4a, 0xbf, 0xf6, 0xaf, 0x41, 0x49, 0xf5, 0x1b
			}
		},
		{
			16,
			{
				0xa8, 0x06, 0x1d, 0xc1, 0x30, 0x51, 0x36, 0xc6,
				0xc2, 0x2b, 0x8b, 0xaf, 0x0c, 0x01, 0x27, 0xa9
			}
		}
	},
	/*
	 * test vectors from "The Poly1305-AES message-authentication code"
	 */
	{
		{
			2,
			{
				0xf3, 0xf6
			}
		},
		{
			32,
			{
				0x85, 0x1f, 0xc4, 0x0c, 0x34, 0x67, 0xac, 0x0b,
				0xe0, 0x5c, 0xc2, 0x04, 0x04, 0xf3, 0xf7, 0x00,
				0x58, 0x0b, 0x3b, 0x0f, 0x94, 0x47, 0xbb, 0x1e,
				0x69, 0xd0, 0x95, 0xb5, 0x92, 0x8b, 0x6d, 0xbc
			}
		},
		{
			16,
			{
				0xf4, 0xc6, 0x33, 0xc3, 0x04, 0x4f, 0xc1, 0x45,
				0xf8, 0x4f, 0x33, 0x5c, 0xb8, 0x19, 0x53, 0xde
			}
		}
	},
	{
		{
			0,
			{
				0
			}
		},
		{
			32,
			{
				0xa0, 0xf3, 0x08, 0x00, 0x00, 0xf4, 0x64, 0x00,
				0xd0, 0xc7, 0xe9, 0x07, 0x6c, 0x83, 0x44, 0x03,
				0xdd, 0x3f, 0xab, 0x22, 0x51, 0xf1, 0x1a, 0xc7,
				0x59, 0xf0, 0x88, 0x71, 0x29, 0xcc, 0x2e, 0xe7
			}
		},
		{
			16,
			{
				0xdd, 0x3f, 0xab, 0x22, 0x51, 0xf1, 0x1a, 0xc7,
				0x59, 0xf0, 0x88, 0x71, 0x29, 0xcc, 0x2e, 0xe7
			}
		}
	},
	{
		{
			32,
			{
				0x66, 0x3c, 0xea, 0x19, 0x0f, 0xfb, 0x83, 0xd8,
				0x95, 0x93, 0xf3, 0xf4, 0x76, 0xb6, 0xbc, 0x24,
				0xd7, 0xe6, 0x79, 0x10, 0x7e, 0xa2, 0x6a, 0xdb,
				0x8c, 0xaf, 0x66, 0x52, 0xd0, 0x65, 0x61, 0x36
			}
		},
		{
			32,
			{
				0x48, 0x44, 0x3d, 0x0b, 0xb0, 0xd2, 0x11, 0x09,
				0xc8, 0x9a, 0x10, 0x0b, 0x5c, 0xe2, 0xc2, 0x08,
				0x83, 0x14, 0x9c, 0x69, 0xb5, 0x61, 0xdd, 0x88,
				0x29, 0x8a, 0x17, 0x98, 0xb1, 0x07, 0x16, 0xef
			}
		},
		{
			16,
			{
				0x0e, 0xe1, 0xc1, 0x6b, 0xb7, 0x3f, 0x0f, 0x4f,
				0xd1, 0x98, 0x81, 0x75, 0x3c, 0x01, 0xcd, 0xbe
			}
		}
	},
	{
		{
			63,
			{
				0xab, 0x08, 0x12, 0x72, 0x4a, 0x7f, 0x1e, 0x34,
				0x27, 0x42, 0xcb, 0xed, 0x37, 0x4d, 0x94, 0xd1,
				0x36, 0xc6, 0xb8, 0x79, 0x5d, 0x45, 0xb3, 0x81,
				0x98, 0x30, 0xf2, 0xc0, 0x44, 0x91, 0xfa, 0xf0,

				0x99, 0x0c, 0x62, 0xe4, 0x8b, 0x80, 0x18, 0xb2,
				0xc3, 0xe4, 0xa0, 0xfa, 0x31, 0x34, 0xcb, 0x67,
				0xfa, 0x83, 0xe1, 0x58, 0xc9, 0x94, 0xd9, 0x61,
				0xc4, 0xcb, 0x21, 0x09, 0x5c, 0x1b, 0xf9
			}
		},
		{
			32,
			{
				0x12, 0x97, 0x6a, 0x08, 0xc4, 0x42, 0x6d, 0x0c,
				0xe8, 0xa8, 0x24, 0x07, 0xc4, 0xf4, 0x82, 0x07,
				0x80, 0xf8, 0xc2, 0x0a, 0xa7, 0x12, 0x02, 0xd1,
				0xe2, 0x91, 0x79, 0xcb, 0xcb, 0x55, 0x5a, 0x57
			}
		},
		{
			16,
			{
				0x51, 0x54, 0xad, 0x0d, 0x2c, 0xb2, 0x6e, 0x01,
				0x27, 0x4f, 0xc5, 0x11, 0x48, 0x49, 0x1f, 0x1b
			}
		},
	},
	/*
	 * self-generated vectors exercise "significant" lengths, such that
	 * are handled by different code paths
	 */
	{
		{
			64,
			{
				0xab, 0x08, 0x12, 0x72, 0x4a, 0x7f, 0x1e, 0x34,
				0x27, 0x42, 0xcb, 0xed, 0x37, 0x4d, 0x94, 0xd1,
				0x36, 0xc6, 0xb8, 0x79, 0x5d, 0x45, 0xb3, 0x81,
				0x98, 0x30, 0xf2, 0xc0, 0x44, 0x91, 0xfa, 0xf0,

				0x99, 0x0c, 0x62, 0xe4, 0x8b, 0x80, 0x18, 0xb2,
				0xc3, 0xe4, 0xa0, 0xfa, 0x31, 0x34, 0xcb, 0x67,
				0xfa, 0x83, 0xe1, 0x58, 0xc9, 0x94, 0xd9, 0x61,
				0xc4, 0xcb, 0x21, 0x09, 0x5c, 0x1b, 0xf9, 0xaf
			}
		},
		{
			32,
			{
				0x12, 0x97, 0x6a, 0x08, 0xc4, 0x42, 0x6d, 0x0c,
				0xe8, 0xa8, 0x24, 0x07, 0xc4, 0xf4, 0x82, 0x07,
				0x80, 0xf8, 0xc2, 0x0a, 0xa7, 0x12, 0x02, 0xd1,
				0xe2, 0x91, 0x79, 0xcb, 0xcb, 0x55, 0x5a, 0x57
			}
		},
		{
			16,
			{
				0x81, 0x20, 0x59, 0xa5, 0xda, 0x19, 0x86, 0x37,
				0xca, 0xc7, 0xc4, 0xa6, 0x31, 0xbe, 0xe4, 0x66
			}
		},
	},
	{
		{
			48,
			{
				0xab, 0x08, 0x12, 0x72, 0x4a, 0x7f, 0x1e, 0x34,
				0x27, 0x42, 0xcb, 0xed, 0x37, 0x4d, 0x94, 0xd1,
				0x36, 0xc6, 0xb8, 0x79, 0x5d, 0x45, 0xb3, 0x81,
				0x98, 0x30, 0xf2, 0xc0, 0x44, 0x91, 0xfa, 0xf0,

				0x99, 0x0c, 0x62, 0xe4, 0x8b, 0x80, 0x18, 0xb2,
				0xc3, 0xe4, 0xa0, 0xfa, 0x31, 0x34, 0xcb, 0x67
			}
		},
		{
			32,
			{
				0x12, 0x97, 0x6a, 0x08, 0xc4, 0x42, 0x6d, 0x0c,
				0xe8, 0xa8, 0x24, 0x07, 0xc4, 0xf4, 0x82, 0x07,
				0x80, 0xf8, 0xc2, 0x0a, 0xa7, 0x12, 0x02, 0xd1,
				0xe2, 0x91, 0x79, 0xcb, 0xcb, 0x55, 0x5a, 0x57
			}
		},
		{
			16,
			{
				0x5b, 0x88, 0xd7, 0xf6, 0x22, 0x8b, 0x11, 0xe2,
				0xe2, 0x85, 0x79, 0xa5, 0xc0, 0xc1, 0xf7, 0x61
			}
		},
	},
	{
		{
			96,
			{
				0xab, 0x08, 0x12, 0x72, 0x4a, 0x7f, 0x1e, 0x34,
				0x27, 0x42, 0xcb, 0xed, 0x37, 0x4d, 0x94, 0xd1,
				0x36, 0xc6, 0xb8, 0x79, 0x5d, 0x45, 0xb3, 0x81,
				0x98, 0x30, 0xf2, 0xc0, 0x44, 0x91, 0xfa, 0xf0,

				0x99, 0x0c, 0x62, 0xe4, 0x8b, 0x80, 0x18, 0xb2,
				0xc3, 0xe4, 0xa0, 0xfa, 0x31, 0x34, 0xcb, 0x67,
				0xfa, 0x83, 0xe1, 0x58, 0xc9, 0x94, 0xd9, 0x61,
				0xc4, 0xcb, 0x21, 0x09, 0x5c, 0x1b, 0xf9, 0xaf,

				0x66, 0x3c, 0xea, 0x19, 0x0f, 0xfb, 0x83, 0xd8,
				0x95, 0x93, 0xf3, 0xf4, 0x76, 0xb6, 0xbc, 0x24,
				0xd7, 0xe6, 0x79, 0x10, 0x7e, 0xa2, 0x6a, 0xdb,
				0x8c, 0xaf, 0x66, 0x52, 0xd0, 0x65, 0x61, 0x36
			}
		},
		{
			32,
			{
				0x12, 0x97, 0x6a, 0x08, 0xc4, 0x42, 0x6d, 0x0c,
				0xe8, 0xa8, 0x24, 0x07, 0xc4, 0xf4, 0x82, 0x07,
				0x80, 0xf8, 0xc2, 0x0a, 0xa7, 0x12, 0x02, 0xd1,
				0xe2, 0x91, 0x79, 0xcb, 0xcb, 0x55, 0x5a, 0x57
			}
		},
		{
			16,
			{
				0xbb, 0xb6, 0x13, 0xb2, 0xb6, 0xd7, 0x53, 0xba,
				0x07, 0x39, 0x5b, 0x91, 0x6a, 0xae, 0xce, 0x15
			}
		},
	},
	{
		{
			112,
			{
				0xab, 0x08, 0x12, 0x72, 0x4a, 0x7f, 0x1e, 0x34,
				0x27, 0x42, 0xcb, 0xed, 0x37, 0x4d, 0x94, 0xd1,
				0x36, 0xc6, 0xb8, 0x79, 0x5d, 0x45, 0xb3, 0x81,
				0x98, 0x30, 0xf2, 0xc0, 0x44, 0x91, 0xfa, 0xf0,

				0x99, 0x0c, 0x62, 0xe4, 0x8b, 0x80, 0x18, 0xb2,
				0xc3, 0xe4, 0xa0, 0xfa, 0x31, 0x34, 0xcb, 0x67,
				0xfa, 0x83, 0xe1, 0x58, 0xc9, 0x94, 0xd9, 0x61,
				0xc4, 0xcb, 0x21, 0x09, 0x5c, 0x1b, 0xf9, 0xaf,

				0x48, 0x44, 0x3d, 0x0b, 0xb0, 0xd2, 0x11, 0x09,
				0xc8, 0x9a, 0x10, 0x0b, 0x5c, 0xe2, 0xc2, 0x08,
				0x83, 0x14, 0x9c, 0x69, 0xb5, 0x61, 0xdd, 0x88,
				0x29, 0x8a, 0x17, 0x98, 0xb1, 0x07, 0x16, 0xef,

				0x66, 0x3c, 0xea, 0x19, 0x0f, 0xfb, 0x83, 0xd8,
				0x95, 0x93, 0xf3, 0xf4, 0x76, 0xb6, 0xbc, 0x24
			}
		},
		{
			32,
			{
				0x12, 0x97, 0x6a, 0x08, 0xc4, 0x42, 0x6d, 0x0c,
				0xe8, 0xa8, 0x24, 0x07, 0xc4, 0xf4, 0x82, 0x07,
				0x80, 0xf8, 0xc2, 0x0a, 0xa7, 0x12, 0x02, 0xd1,
				0xe2, 0x91, 0x79, 0xcb, 0xcb, 0x55, 0x5a, 0x57
			}
		},
		{
			16,
			{
				0xc7, 0x94, 0xd7, 0x05, 0x7d, 0x17, 0x78, 0xc4,
				0xbb, 0xee, 0x0a, 0x39, 0xb3, 0xd9, 0x73, 0x42
			}
		},
	},
	{
		{
			128,
			{
				0xab, 0x08, 0x12, 0x72, 0x4a, 0x7f, 0x1e, 0x34,
				0x27, 0x42, 0xcb, 0xed, 0x37, 0x4d, 0x94, 0xd1,
				0x36, 0xc6, 0xb8, 0x79, 0x5d, 0x45, 0xb3, 0x81,
				0x98, 0x30, 0xf2, 0xc0, 0x44, 0x91, 0xfa, 0xf0,

				0x99, 0x0c, 0x62, 0xe4, 0x8b, 0x80, 0x18, 0xb2,
				0xc3, 0xe4, 0xa0, 0xfa, 0x31, 0x34, 0xcb, 0x67,
				0xfa, 0x83, 0xe1, 0x58, 0xc9, 0x94, 0xd9, 0x61,
				0xc4, 0xcb, 0x21, 0x09, 0x5c, 0x1b, 0xf9, 0xaf,

				0x48, 0x44, 0x3d, 0x0b, 0xb0, 0xd2, 0x11, 0x09,
				0xc8, 0x9a, 0x10, 0x0b, 0x5c, 0xe2, 0xc2, 0x08,
				0x83, 0x14, 0x9c, 0x69, 0xb5, 0x61, 0xdd, 0x88,
				0x29, 0x8a, 0x17, 0x98, 0xb1, 0x07, 0x16, 0xef,

				0x66, 0x3c, 0xea, 0x19, 0x0f, 0xfb, 0x83, 0xd8,
				0x95, 0x93, 0xf3, 0xf4, 0x76, 0xb6, 0xbc, 0x24,
				0xd7, 0xe6, 0x79, 0x10, 0x7e, 0xa2, 0x6a, 0xdb,
				0x8c, 0xaf, 0x66, 0x52, 0xd0, 0x65, 0x61, 0x36
			}
		},
		{
			32,
			{
				0x12, 0x97, 0x6a, 0x08, 0xc4, 0x42, 0x6d, 0x0c,
				0xe8, 0xa8, 0x24, 0x07, 0xc4, 0xf4, 0x82, 0x07,
				0x80, 0xf8, 0xc2, 0x0a, 0xa7, 0x12, 0x02, 0xd1,
				0xe2, 0x91, 0x79, 0xcb, 0xcb, 0x55, 0x5a, 0x57
			}
		},
		{
			16,
			{
				0xff, 0xbc, 0xb9, 0xb3, 0x71, 0x42, 0x31, 0x52,
				0xd7, 0xfc, 0xa5, 0xad, 0x04, 0x2f, 0xba, 0xa9
			}
		},
	},
	{
		{
			144,
			{
				0xab, 0x08, 0x12, 0x72, 0x4a, 0x7f, 0x1e, 0x34,
				0x27, 0x42, 0xcb, 0xed, 0x37, 0x4d, 0x94, 0xd1,
				0x36, 0xc6, 0xb8, 0x79, 0x5d, 0x45, 0xb3, 0x81,
				0x98, 0x30, 0xf2, 0xc0, 0x44, 0x91, 0xfa, 0xf0,

				0x99, 0x0c, 0x62, 0xe4, 0x8b, 0x80, 0x18, 0xb2,
				0xc3, 0xe4, 0xa0, 0xfa, 0x31, 0x34, 0xcb, 0x67,
				0xfa, 0x83, 0xe1, 0x58, 0xc9, 0x94, 0xd9, 0x61,
				0xc4, 0xcb, 0x21, 0x09, 0x5c, 0x1b, 0xf9, 0xaf,

				0x48, 0x44, 0x3d, 0x0b, 0xb0, 0xd2, 0x11, 0x09,
				0xc8, 0x9a, 0x10, 0x0b, 0x5c, 0xe2, 0xc2, 0x08,
				0x83, 0x14, 0x9c, 0x69, 0xb5, 0x61, 0xdd, 0x88,
				0x29, 0x8a, 0x17, 0x98, 0xb1, 0x07, 0x16, 0xef,

				0x66, 0x3c, 0xea, 0x19, 0x0f, 0xfb, 0x83, 0xd8,
				0x95, 0x93, 0xf3, 0xf4, 0x76, 0xb6, 0xbc, 0x24,
				0xd7, 0xe6, 0x79, 0x10, 0x7e, 0xa2, 0x6a, 0xdb,
				0x8c, 0xaf, 0x66, 0x52, 0xd0, 0x65, 0x61, 0x36,

				0x81, 0x20, 0x59, 0xa5, 0xda, 0x19, 0x86, 0x37,
				0xca, 0xc7, 0xc4, 0xa6, 0x31, 0xbe, 0xe4, 0x66
			}
		},
		{
			32,
			{
				0x12, 0x97, 0x6a, 0x08, 0xc4, 0x42, 0x6d, 0x0c,
				0xe8, 0xa8, 0x24, 0x07, 0xc4, 0xf4, 0x82, 0x07,
				0x80, 0xf8, 0xc2, 0x0a, 0xa7, 0x12, 0x02, 0xd1,
				0xe2, 0x91, 0x79, 0xcb, 0xcb, 0x55, 0x5a, 0x57
			}
		},
		{
			16,
			{
				0x06, 0x9e, 0xd6, 0xb8, 0xef, 0x0f, 0x20, 0x7b,
				0x3e, 0x24, 0x3b, 0xb1, 0x01, 0x9f, 0xe6, 0x32
			}
		},
	},
	{
		{
			160,
			{
				0xab, 0x08, 0x12, 0x72, 0x4a, 0x7f, 0x1e, 0x34,
				0x27, 0x42, 0xcb, 0xed, 0x37, 0x4d, 0x94, 0xd1,
				0x36, 0xc6, 0xb8, 0x79, 0x5d, 0x45, 0xb3, 0x81,
				0x98, 0x30, 0xf2, 0xc0, 0x44, 0x91, 0xfa, 0xf0,

				0x99, 0x0c, 0x62, 0xe4, 0x8b, 0x80, 0x18, 0xb2,
				0xc3, 0xe4, 0xa0, 0xfa, 0x31, 0x34, 0xcb, 0x67,
				0xfa, 0x83, 0xe1, 0x58, 0xc9, 0x94, 0xd9, 0x61,
				0xc4, 0xcb, 0x21, 0x09, 0x5c, 0x1b, 0xf9, 0xaf,

				0x48, 0x44, 0x3d, 0x0b, 0xb0, 0xd2, 0x11, 0x09,
				0xc8, 0x9a, 0x10, 0x0b, 0x5c, 0xe2, 0xc2, 0x08,
				0x83, 0x14, 0x9c, 0x69, 0xb5, 0x61, 0xdd, 0x88,
				0x29, 0x8a, 0x17, 0x98, 0xb1, 0x07, 0x16, 0xef,

				0x66, 0x3c, 0xea, 0x19, 0x0f, 0xfb, 0x83, 0xd8,
				0x95, 0x93, 0xf3, 0xf4, 0x76, 0xb6, 0xbc, 0x24,
				0xd7, 0xe6, 0x79, 0x10, 0x7e, 0xa2, 0x6a, 0xdb,
				0x8c, 0xaf, 0x66, 0x52, 0xd0, 0x65, 0x61, 0x36,

				0x81, 0x20, 0x59, 0xa5, 0xda, 0x19, 0x86, 0x37,
				0xca, 0xc7, 0xc4, 0xa6, 0x31, 0xbe, 0xe4, 0x66,
				0x5b, 0x88, 0xd7, 0xf6, 0x22, 0x8b, 0x11, 0xe2,
				0xe2, 0x85, 0x79, 0xa5, 0xc0, 0xc1, 0xf7, 0x61
			}
		},
		{
			32,
			{
				0x12, 0x97, 0x6a, 0x08, 0xc4, 0x42, 0x6d, 0x0c,
				0xe8, 0xa8, 0x24, 0x07, 0xc4, 0xf4, 0x82, 0x07,
				0x80, 0xf8, 0xc2, 0x0a, 0xa7, 0x12, 0x02, 0xd1,
				0xe2, 0x91, 0x79, 0xcb, 0xcb, 0x55, 0x5a, 0x57
			}
		},
		{
			16,
			{
				0xcc, 0xa3, 0x39, 0xd9, 0xa4, 0x5f, 0xa2, 0x36,
				0x8c, 0x2c, 0x68, 0xb3, 0xa4, 0x17, 0x91, 0x33
			}
		},
	},
	{
		{
			288,
			{
				0xab, 0x08, 0x12, 0x72, 0x4a, 0x7f, 0x1e, 0x34,
				0x27, 0x42, 0xcb, 0xed, 0x37, 0x4d, 0x94, 0xd1,
				0x36, 0xc6, 0xb8, 0x79, 0x5d, 0x45, 0xb3, 0x81,
				0x98, 0x30, 0xf2, 0xc0, 0x44, 0x91, 0xfa, 0xf0,

				0x99, 0x0c, 0x62, 0xe4, 0x8b, 0x80, 0x18, 0xb2,
				0xc3, 0xe4, 0xa0, 0xfa, 0x31, 0x34, 0xcb, 0x67,
				0xfa, 0x83, 0xe1, 0x58, 0xc9, 0x94, 0xd9, 0x61,
				0xc4, 0xcb, 0x21, 0x09, 0x5c, 0x1b, 0xf9, 0xaf,

				0x48, 0x44, 0x3d, 0x0b, 0xb0, 0xd2, 0x11, 0x09,
				0xc8, 0x9a, 0x10, 0x0b, 0x5c, 0xe2, 0xc2, 0x08,
				0x83, 0x14, 0x9c, 0x69, 0xb5, 0x61, 0xdd, 0x88,
				0x29, 0x8a, 0x17, 0x98, 0xb1, 0x07, 0x16, 0xef,

				0x66, 0x3c, 0xea, 0x19, 0x0f, 0xfb, 0x83, 0xd8,
				0x95, 0x93, 0xf3, 0xf4, 0x76, 0xb6, 0xbc, 0x24,
				0xd7, 0xe6, 0x79, 0x10, 0x7e, 0xa2, 0x6a, 0xdb,
				0x8c, 0xaf, 0x66, 0x52, 0xd0, 0x65, 0x61, 0x36,

				0x81, 0x20, 0x59, 0xa5, 0xda, 0x19, 0x86, 0x37,
				0xca, 0xc7, 0xc4, 0xa6, 0x31, 0xbe, 0xe4, 0x66,
				0x5b, 0x88, 0xd7, 0xf6, 0x22, 0x8b, 0x11, 0xe2,
				0xe2, 0x85, 0x79, 0xa5, 0xc0, 0xc1, 0xf7, 0x61,

				0xab, 0x08, 0x12, 0x72, 0x4a, 0x7f, 0x1e, 0x34,
				0x27, 0x42, 0xcb, 0xed, 0x37, 0x4d, 0x94, 0xd1,
				0x36, 0xc6, 0xb8, 0x79, 0x5d, 0x45, 0xb3, 0x81,
				0x98, 0x30, 0xf2, 0xc0, 0x44, 0x91, 0xfa, 0xf0,

				0x99, 0x0c, 0x62, 0xe4, 0x8b, 0x80, 0x18, 0xb2,
				0xc3, 0xe4, 0xa0, 0xfa, 0x31, 0x34, 0xcb, 0x67,
				0xfa, 0x83, 0xe1, 0x58, 0xc9, 0x94, 0xd9, 0x61,
				0xc4, 0xcb, 0x21, 0x09, 0x5c, 0x1b, 0xf9, 0xaf,

				0x48, 0x44, 0x3d, 0x0b, 0xb0, 0xd2, 0x11, 0x09,
				0xc8, 0x9a, 0x10, 0x0b, 0x5c, 0xe2, 0xc2, 0x08,
				0x83, 0x14, 0x9c, 0x69, 0xb5, 0x61, 0xdd, 0x88,
				0x29, 0x8a, 0x17, 0x98, 0xb1, 0x07, 0x16, 0xef,

				0x66, 0x3c, 0xea, 0x19, 0x0f, 0xfb, 0x83, 0xd8,
				0x95, 0x93, 0xf3, 0xf4, 0x76, 0xb6, 0xbc, 0x24,
				0xd7, 0xe6, 0x79, 0x10, 0x7e, 0xa2, 0x6a, 0xdb,
				0x8c, 0xaf, 0x66, 0x52, 0xd0, 0x65, 0x61, 0x36
			}
		},
		{
			32,
			{
				0x12, 0x97, 0x6a, 0x08, 0xc4, 0x42, 0x6d, 0x0c,
				0xe8, 0xa8, 0x24, 0x07, 0xc4, 0xf4, 0x82, 0x07,
				0x80, 0xf8, 0xc2, 0x0a, 0xa7, 0x12, 0x02, 0xd1,
				0xe2, 0x91, 0x79, 0xcb, 0xcb, 0x55, 0x5a, 0x57
			}
		},
		{
			16,
			{
				0x53, 0xf6, 0xe8, 0x28, 0xa2, 0xf0, 0xfe, 0x0e,
				0xe8, 0x15, 0xbf, 0x0b, 0xd5, 0x84, 0x1a, 0x34
			}
		},
	},
	{
		{
			320,
			{
				0xab, 0x08, 0x12, 0x72, 0x4a, 0x7f, 0x1e, 0x34,
				0x27, 0x42, 0xcb, 0xed, 0x37, 0x4d, 0x94, 0xd1,
				0x36, 0xc6, 0xb8, 0x79, 0x5d, 0x45, 0xb3, 0x81,
				0x98, 0x30, 0xf2, 0xc0, 0x44, 0x91, 0xfa, 0xf0,

				0x99, 0x0c, 0x62, 0xe4, 0x8b, 0x80, 0x18, 0xb2,
				0xc3, 0xe4, 0xa0, 0xfa, 0x31, 0x34, 0xcb, 0x67,
				0xfa, 0x83, 0xe1, 0x58, 0xc9, 0x94, 0xd9, 0x61,
				0xc4, 0xcb, 0x21, 0x09, 0x5c, 0x1b, 0xf9, 0xaf,

				0x48, 0x44, 0x3d, 0x0b, 0xb0, 0xd2, 0x11, 0x09,
				0xc8, 0x9a, 0x10, 0x0b, 0x5c, 0xe2, 0xc2, 0x08,
				0x83, 0x14, 0x9c, 0x69, 0xb5, 0x61, 0xdd, 0x88,
				0x29, 0x8a, 0x17, 0x98, 0xb1, 0x07, 0x16, 0xef,

				0x66, 0x3c, 0xea, 0x19, 0x0f, 0xfb, 0x83, 0xd8,
				0x95, 0x93, 0xf3, 0xf4, 0x76, 0xb6, 0xbc, 0x24,
				0xd7, 0xe6, 0x79, 0x10, 0x7e, 0xa2, 0x6a, 0xdb,
				0x8c, 0xaf, 0x66, 0x52, 0xd0, 0x65, 0x61, 0x36,

				0x81, 0x20, 0x59, 0xa5, 0xda, 0x19, 0x86, 0x37,
				0xca, 0xc7, 0xc4, 0xa6, 0x31, 0xbe, 0xe4, 0x66,
				0x5b, 0x88, 0xd7, 0xf6, 0x22, 0x8b, 0x11, 0xe2,
				0xe2, 0x85, 0x79, 0xa5, 0xc0, 0xc1, 0xf7, 0x61,

				0xab, 0x08, 0x12, 0x72, 0x4a, 0x7f, 0x1e, 0x34,
				0x27, 0x42, 0xcb, 0xed, 0x37, 0x4d, 0x94, 0xd1,
				0x36, 0xc6, 0xb8, 0x79, 0x5d, 0x45, 0xb3, 0x81,
				0x98, 0x30, 0xf2, 0xc0, 0x44, 0x91, 0xfa, 0xf0,

				0x99, 0x0c, 0x62, 0xe4, 0x8b, 0x80, 0x18, 0xb2,
				0xc3, 0xe4, 0xa0, 0xfa, 0x31, 0x34, 0xcb, 0x67,
				0xfa, 0x83, 0xe1, 0x58, 0xc9, 0x94, 0xd9, 0x61,
				0xc4, 0xcb, 0x21, 0x09, 0x5c, 0x1b, 0xf9, 0xaf,

				0x48, 0x44, 0x3d, 0x0b, 0xb0, 0xd2, 0x11, 0x09,
				0xc8, 0x9a, 0x10, 0x0b, 0x5c, 0xe2, 0xc2, 0x08,
				0x83, 0x14, 0x9c, 0x69, 0xb5, 0x61, 0xdd, 0x88,
				0x29, 0x8a, 0x17, 0x98, 0xb1, 0x07, 0x16, 0xef,

				0x66, 0x3c, 0xea, 0x19, 0x0f, 0xfb, 0x83, 0xd8,
				0x95, 0x93, 0xf3, 0xf4, 0x76, 0xb6, 0xbc, 0x24,
				0xd7, 0xe6, 0x79, 0x10, 0x7e, 0xa2, 0x6a, 0xdb,
				0x8c, 0xaf, 0x66, 0x52, 0xd0, 0x65, 0x61, 0x36,

				0x81, 0x20, 0x59, 0xa5, 0xda, 0x19, 0x86, 0x37,
				0xca, 0xc7, 0xc4, 0xa6, 0x31, 0xbe, 0xe4, 0x66,
				0x5b, 0x88, 0xd7, 0xf6, 0x22, 0x8b, 0x11, 0xe2,
				0xe2, 0x85, 0x79, 0xa5, 0xc0, 0xc1, 0xf7, 0x61
			}
		},
		{
			32,
			{
				0x12, 0x97, 0x6a, 0x08, 0xc4, 0x42, 0x6d, 0x0c,
				0xe8, 0xa8, 0x24, 0x07, 0xc4, 0xf4, 0x82, 0x07,
				0x80, 0xf8, 0xc2, 0x0a, 0xa7, 0x12, 0x02, 0xd1,
				0xe2, 0x91, 0x79, 0xcb, 0xcb, 0x55, 0x5a, 0x57
			}
		},
		{
			16,
			{
				0xb8, 0x46, 0xd4, 0x4e, 0x9b, 0xbd, 0x53, 0xce,
				0xdf, 0xfb, 0xfb, 0xb6, 0xb7, 0xfa, 0x49, 0x33
			}
		},
	},
	/*
	 * 4th power of the key spills to 131th bit in SIMD key setup
	 */
	{
		{
			256,
			{
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
			}
		},
		{
			32,
			{
				0xad, 0x62, 0x81, 0x07, 0xe8, 0x35, 0x1d, 0x0f,
				0x2c, 0x23, 0x1a, 0x05, 0xdc, 0x4a, 0x41, 0x06,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			16,
			{
				0x07, 0x14, 0x5a, 0x4c, 0x02, 0xfe, 0x5f, 0xa3,
				0x20, 0x36, 0xde, 0x68, 0xfa, 0xbe, 0x90, 0x66
			}
		},
	},
	/*
	 * poly1305_ieee754.c failed this in final stage
	 */
	{
		{
			252,
			{
				0x84, 0x23, 0x64, 0xe1, 0x56, 0x33, 0x6c, 0x09,
				0x98, 0xb9, 0x33, 0xa6, 0x23, 0x77, 0x26, 0x18,
				0x0d, 0x9e, 0x3f, 0xdc, 0xbd, 0xe4, 0xcd, 0x5d,
				0x17, 0x08, 0x0f, 0xc3, 0xbe, 0xb4, 0x96, 0x14,

				0xd7, 0x12, 0x2c, 0x03, 0x74, 0x63, 0xff, 0x10,
				0x4d, 0x73, 0xf1, 0x9c, 0x12, 0x70, 0x46, 0x28,
				0xd4, 0x17, 0xc4, 0xc5, 0x4a, 0x3f, 0xe3, 0x0d,
				0x3c, 0x3d, 0x77, 0x14, 0x38, 0x2d, 0x43, 0xb0,

				0x38, 0x2a, 0x50, 0xa5, 0xde, 0xe5, 0x4b, 0xe8,
				0x44, 0xb0, 0x76, 0xe8, 0xdf, 0x88, 0x20, 0x1a,
				0x1c, 0xd4, 0x3b, 0x90, 0xeb, 0x21, 0x64, 0x3f,
				0xa9, 0x6f, 0x39, 0xb5, 0x18, 0xaa, 0x83, 0x40,

				0xc9, 0x42, 0xff, 0x3c, 0x31, 0xba, 0xf7, 0xc9,
				0xbd, 0xbf, 0x0f, 0x31, 0xae, 0x3f, 0xa0, 0x96,
				0xbf, 0x8c, 0x63, 0x03, 0x06, 0x09, 0x82, 0x9f,
				0xe7, 0x2e, 0x17, 0x98, 0x24, 0x89, 0x0b, 0xc8,

				0xe0, 0x8c, 0x31, 0x5c, 0x1c, 0xce, 0x2a, 0x83,
				0x14, 0x4d, 0xbb, 0xff, 0x09, 0xf7, 0x4e, 0x3e,
				0xfc, 0x77, 0x0b, 0x54, 0xd0, 0x98, 0x4a, 0x8f,
				0x19, 0xb1, 0x47, 0x19, 0xe6, 0x36, 0x35, 0x64,

				0x1d, 0x6b, 0x1e, 0xed, 0xf6, 0x3e, 0xfb, 0xf0,
				0x80, 0xe1, 0x78, 0x3d, 0x32, 0x44, 0x54, 0x12,
				0x11, 0x4c, 0x20, 0xde, 0x0b, 0x83, 0x7a, 0x0d,
				0xfa, 0x33, 0xd6, 0xb8, 0x28, 0x25, 0xff, 0xf4,

				0x4c, 0x9a, 0x70, 0xea, 0x54, 0xce, 0x47, 0xf0,
				0x7d, 0xf6, 0x98, 0xe6, 0xb0, 0x33, 0x23, 0xb5,
				0x30, 0x79, 0x36, 0x4a, 0x5f, 0xc3, 0xe9, 0xdd,
				0x03, 0x43, 0x92, 0xbd, 0xde, 0x86, 0xdc, 0xcd,

				0xda, 0x94, 0x32, 0x1c, 0x5e, 0x44, 0x06, 0x04,
				0x89, 0x33, 0x6c, 0xb6, 0x5b, 0xf3, 0x98, 0x9c,
				0x36, 0xf7, 0x28, 0x2c, 0x2f, 0x5d, 0x2b, 0x88,
				0x2c, 0x17, 0x1e, 0x74
			}
		},
		{
			32,
			{
				0x95, 0xd5, 0xc0, 0x05, 0x50, 0x3e, 0x51, 0x0d,
				0x8c, 0xd0, 0xaa, 0x07, 0x2c, 0x4a, 0x4d, 0x06,
				0x6e, 0xab, 0xc5, 0x2d, 0x11, 0x65, 0x3d, 0xf4,
				0x7f, 0xbf, 0x63, 0xab, 0x19, 0x8b, 0xcc, 0x26
			}
		},
		{
			16,
			{
				0xf2, 0x48, 0x31, 0x2e, 0x57, 0x8d, 0x9d, 0x58,
				0xf8, 0xb7, 0xbb, 0x4d, 0x19, 0x10, 0x54, 0x31
			}
		},
	},
	/*
	 * AVX2 in poly1305-x86.pl failed this with 176+32 split
	 */
	{
		{
			208,
			{
				0x24, 0x8a, 0xc3, 0x10, 0x85, 0xb6, 0xc2, 0xad,
				0xaa, 0xa3, 0x82, 0x59, 0xa0, 0xd7, 0x19, 0x2c,
				0x5c, 0x35, 0xd1, 0xbb, 0x4e, 0xf3, 0x9a, 0xd9,
				0x4c, 0x38, 0xd1, 0xc8, 0x24, 0x79, 0xe2, 0xdd,

				0x21, 0x59, 0xa0, 0x77, 0x02, 0x4b, 0x05, 0x89,
				0xbc, 0x8a, 0x20, 0x10, 0x1b, 0x50, 0x6f, 0x0a,
				0x1a, 0xd0, 0xbb, 0xab, 0x76, 0xe8, 0x3a, 0x83,
				0xf1, 0xb9, 0x4b, 0xe6, 0xbe, 0xae, 0x74, 0xe8,

				0x74, 0xca, 0xb6, 0x92, 0xc5, 0x96, 0x3a, 0x75,
				0x43, 0x6b, 0x77, 0x61, 0x21, 0xec, 0x9f, 0x62,
				0x39, 0x9a, 0x3e, 0x66, 0xb2, 0xd2, 0x27, 0x07,
				0xda, 0xe8, 0x19, 0x33, 0xb6, 0x27, 0x7f, 0x3c,

				0x85, 0x16, 0xbc, 0xbe, 0x26, 0xdb, 0xbd, 0x86,
				0xf3, 0x73, 0x10, 0x3d, 0x7c, 0xf4, 0xca, 0xd1,
				0x88, 0x8c, 0x95, 0x21, 0x18, 0xfb, 0xfb, 0xd0,
				0xd7, 0xb4, 0xbe, 0xdc, 0x4a, 0xe4, 0x93, 0x6a,

				0xff, 0x91, 0x15, 0x7e, 0x7a, 0xa4, 0x7c, 0x54,
				0x44, 0x2e, 0xa7, 0x8d, 0x6a, 0xc2, 0x51, 0xd3,
				0x24, 0xa0, 0xfb, 0xe4, 0x9d, 0x89, 0xcc, 0x35,
				0x21, 0xb6, 0x6d, 0x16, 0xe9, 0xc6, 0x6a, 0x37,

				0x09, 0x89, 0x4e, 0x4e, 0xb0, 0xa4, 0xee, 0xdc,
				0x4a, 0xe1, 0x94, 0x68, 0xe6, 0x6b, 0x81, 0xf2,

				0x71, 0x35, 0x1b, 0x1d, 0x92, 0x1e, 0xa5, 0x51,
				0x04, 0x7a, 0xbc, 0xc6, 0xb8, 0x7a, 0x90, 0x1f,
				0xde, 0x7d, 0xb7, 0x9f, 0xa1, 0x81, 0x8c, 0x11,
				0x33, 0x6d, 0xbc, 0x07, 0x24, 0x4a, 0x40, 0xeb
			}
		},
		{
			32,
			{
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
				0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			16,
			{
				0xbc, 0x93, 0x9b, 0xc5, 0x28, 0x14, 0x80, 0xfa,
				0x99, 0xc6, 0xd6, 0x8c, 0x25, 0x8e, 0xc4, 0x2f
			}
		},
	},
	/*
	 * test vectors from Google
	 */
	{
		{
			0,
			{
				0x00,
			}
		},
		{
			32,
			{
				0xc8, 0xaf, 0xaa, 0xc3, 0x31, 0xee, 0x37, 0x2c,
				0xd6, 0x08, 0x2d, 0xe1, 0x34, 0x94, 0x3b, 0x17,
				0x47, 0x10, 0x13, 0x0e, 0x9f, 0x6f, 0xea, 0x8d,
				0x72, 0x29, 0x38, 0x50, 0xa6, 0x67, 0xd8, 0x6c
			}
		},
		{
			16,
			{
				0x47, 0x10, 0x13, 0x0e, 0x9f, 0x6f, 0xea, 0x8d,
				0x72, 0x29, 0x38, 0x50, 0xa6, 0x67, 0xd8, 0x6c
			}
		},
	},
	{
		{
			12,
			{
				0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f,
				0x72, 0x6c, 0x64, 0x21
			}
		},
		{
			32,
			{
				0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
				0x33, 0x32, 0x2d, 0x62, 0x79, 0x74, 0x65, 0x20,
				0x6b, 0x65, 0x79, 0x20, 0x66, 0x6f, 0x72, 0x20,
				0x50, 0x6f, 0x6c, 0x79, 0x31, 0x33, 0x30, 0x35
			}
		},
		{
			16,
			{
				0xa6, 0xf7, 0x45, 0x00, 0x8f, 0x81, 0xc9, 0x16,
				0xa2, 0x0d, 0xcc, 0x74, 0xee, 0xf2, 0xb2, 0xf0
			}
		},
	},
	{
		{
			32,
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			32,
			{
				0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
				0x33, 0x32, 0x2d, 0x62, 0x79, 0x74, 0x65, 0x20,
				0x6b, 0x65, 0x79, 0x20, 0x66, 0x6f, 0x72, 0x20,
				0x50, 0x6f, 0x6c, 0x79, 0x31, 0x33, 0x30, 0x35
			}
		},
		{
			16,
			{
				0x49, 0xec, 0x78, 0x09, 0x0e, 0x48, 0x1e, 0xc6,
				0xc2, 0x6b, 0x33, 0xb9, 0x1c, 0xcc, 0x03, 0x07
			}
		},
	},
	{
		{
			128,
			{
				0x89, 0xda, 0xb8, 0x0b, 0x77, 0x17, 0xc1, 0xdb,
				0x5d, 0xb4, 0x37, 0x86, 0x0a, 0x3f, 0x70, 0x21,
				0x8e, 0x93, 0xe1, 0xb8, 0xf4, 0x61, 0xfb, 0x67,
				0x7f, 0x16, 0xf3, 0x5f, 0x6f, 0x87, 0xe2, 0xa9,

				0x1c, 0x99, 0xbc, 0x3a, 0x47, 0xac, 0xe4, 0x76,
				0x40, 0xcc, 0x95, 0xc3, 0x45, 0xbe, 0x5e, 0xcc,
				0xa5, 0xa3, 0x52, 0x3c, 0x35, 0xcc, 0x01, 0x89,
				0x3a, 0xf0, 0xb6, 0x4a, 0x62, 0x03, 0x34, 0x27,

				0x03, 0x72, 0xec, 0x12, 0x48, 0x2d, 0x1b, 0x1e,
				0x36, 0x35, 0x61, 0x69, 0x8a, 0x57, 0x8b, 0x35,
				0x98, 0x03, 0x49, 0x5b, 0xb4, 0xe2, 0xef, 0x19,
				0x30, 0xb1, 0x7a, 0x51, 0x90, 0xb5, 0x80, 0xf1,

				0x41, 0x30, 0x0d, 0xf3, 0x0a, 0xdb, 0xec, 0xa2,
				0x8f, 0x64, 0x27, 0xa8, 0xbc, 0x1a, 0x99, 0x9f,
				0xd5, 0x1c, 0x55, 0x4a, 0x01, 0x7d, 0x09, 0x5d,
				0x8c, 0x3e, 0x31, 0x27, 0xda, 0xf9, 0xf5, 0x95
			}
		},
		{
			32,
			{
				0x2d, 0x77, 0x3b, 0xe3, 0x7a, 0xdb, 0x1e, 0x4d,
				0x68, 0x3b, 0xf0, 0x07, 0x5e, 0x79, 0xc4, 0xee,
				0x03, 0x79, 0x18, 0x53, 0x5a, 0x7f, 0x99, 0xcc,
				0xb7, 0x04, 0x0f, 0xb5, 0xf5, 0xf4, 0x3a, 0xea
			}
		},
		{
			16,
			{
				0xc8, 0x5d, 0x15, 0xed, 0x44, 0xc3, 0x78, 0xd6,
				0xb0, 0x0e, 0x23, 0x06, 0x4c, 0x7b, 0xcd, 0x51
			}
		},
	},
	{
		{
			528,
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b,
				0x17, 0x03, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00,

				0x06, 0xdb, 0x1f, 0x1f, 0x36, 0x8d, 0x69, 0x6a,
				0x81, 0x0a, 0x34, 0x9c, 0x0c, 0x71, 0x4c, 0x9a,
				0x5e, 0x78, 0x50, 0xc2, 0x40, 0x7d, 0x72, 0x1a,
				0xcd, 0xed, 0x95, 0xe0, 0x18, 0xd7, 0xa8, 0x52,

				0x66, 0xa6, 0xe1, 0x28, 0x9c, 0xdb, 0x4a, 0xeb,
				0x18, 0xda, 0x5a, 0xc8, 0xa2, 0xb0, 0x02, 0x6d,
				0x24, 0xa5, 0x9a, 0xd4, 0x85, 0x22, 0x7f, 0x3e,
				0xae, 0xdb, 0xb2, 0xe7, 0xe3, 0x5e, 0x1c, 0x66,

				0xcd, 0x60, 0xf9, 0xab, 0xf7, 0x16, 0xdc, 0xc9,
				0xac, 0x42, 0x68, 0x2d, 0xd7, 0xda, 0xb2, 0x87,
				0xa7, 0x02, 0x4c, 0x4e, 0xef, 0xc3, 0x21, 0xcc,
				0x05, 0x74, 0xe1, 0x67, 0x93, 0xe3, 0x7c, 0xec,

				0x03, 0xc5, 0xbd, 0xa4, 0x2b, 0x54, 0xc1, 0x14,
				0xa8, 0x0b, 0x57, 0xaf, 0x26, 0x41, 0x6c, 0x7b,
				0xe7, 0x42, 0x00, 0x5e, 0x20, 0x85, 0x5c, 0x73,
				0xe2, 0x1d, 0xc8, 0xe2, 0xed, 0xc9, 0xd4, 0x35,

				0xcb, 0x6f, 0x60, 0x59, 0x28, 0x00, 0x11, 0xc2,
				0x70, 0xb7, 0x15, 0x70, 0x05, 0x1c, 0x1c, 0x9b,
				0x30, 0x52, 0x12, 0x66, 0x20, 0xbc, 0x1e, 0x27,
				0x30, 0xfa, 0x06, 0x6c, 0x7a, 0x50, 0x9d, 0x53,

				0xc6, 0x0e, 0x5a, 0xe1, 0xb4, 0x0a, 0xa6, 0xe3,
				0x9e, 0x49, 0x66, 0x92, 0x28, 0xc9, 0x0e, 0xec,
				0xb4, 0xa5, 0x0d, 0xb3, 0x2a, 0x50, 0xbc, 0x49,
				0xe9, 0x0b, 0x4f, 0x4b, 0x35, 0x9a, 0x1d, 0xfd,

				0x11, 0x74, 0x9c, 0xd3, 0x86, 0x7f, 0xcf, 0x2f,
				0xb7, 0xbb, 0x6c, 0xd4, 0x73, 0x8f, 0x6a, 0x4a,
				0xd6, 0xf7, 0xca, 0x50, 0x58, 0xf7, 0x61, 0x88,
				0x45, 0xaf, 0x9f, 0x02, 0x0f, 0x6c, 0x3b, 0x96,

				0x7b, 0x8f, 0x4c, 0xd4, 0xa9, 0x1e, 0x28, 0x13,
				0xb5, 0x07, 0xae, 0x66, 0xf2, 0xd3, 0x5c, 0x18,
				0x28, 0x4f, 0x72, 0x92, 0x18, 0x60, 0x62, 0xe1,
				0x0f, 0xd5, 0x51, 0x0d, 0x18, 0x77, 0x53, 0x51,

				0xef, 0x33, 0x4e, 0x76, 0x34, 0xab, 0x47, 0x43,
				0xf5, 0xb6, 0x8f, 0x49, 0xad, 0xca, 0xb3, 0x84,
				0xd3, 0xfd, 0x75, 0xf7, 0x39, 0x0f, 0x40, 0x06,
				0xef, 0x2a, 0x29, 0x5c, 0x8c, 0x7a, 0x07, 0x6a,

				0xd5, 0x45, 0x46, 0xcd, 0x25, 0xd2, 0x10, 0x7f,
				0xbe, 0x14, 0x36, 0xc8, 0x40, 0x92, 0x4a, 0xae,
				0xbe, 0x5b, 0x37, 0x08, 0x93, 0xcd, 0x63, 0xd1,
				0x32, 0x5b, 0x86, 0x16, 0xfc, 0x48, 0x10, 0x88,

				0x6b, 0xc1, 0x52, 0xc5, 0x32, 0x21, 0xb6, 0xdf,
				0x37, 0x31, 0x19, 0x39, 0x32, 0x55, 0xee, 0x72,
				0xbc, 0xaa, 0x88, 0x01, 0x74, 0xf1, 0x71, 0x7f,
				0x91, 0x84, 0xfa, 0x91, 0x64, 0x6f, 0x17, 0xa2,

				0x4a, 0xc5, 0x5d, 0x16, 0xbf, 0xdd, 0xca, 0x95,
				0x81, 0xa9, 0x2e, 0xda, 0x47, 0x92, 0x01, 0xf0,
				0xed, 0xbf, 0x63, 0x36, 0x00, 0xd6, 0x06, 0x6d,
				0x1a, 0xb3, 0x6d, 0x5d, 0x24, 0x15, 0xd7, 0x13,

				0x51, 0xbb, 0xcd, 0x60, 0x8a, 0x25, 0x10, 0x8d,
				0x25, 0x64, 0x19, 0x92, 0xc1, 0xf2, 0x6c, 0x53,
				0x1c, 0xf9, 0xf9, 0x02, 0x03, 0xbc, 0x4c, 0xc1,
				0x9f, 0x59, 0x27, 0xd8, 0x34, 0xb0, 0xa4, 0x71,

				0x16, 0xd3, 0x88, 0x4b, 0xbb, 0x16, 0x4b, 0x8e,
				0xc8, 0x83, 0xd1, 0xac, 0x83, 0x2e, 0x56, 0xb3,
				0x91, 0x8a, 0x98, 0x60, 0x1a, 0x08, 0xd1, 0x71,
				0x88, 0x15, 0x41, 0xd5, 0x94, 0xdb, 0x39, 0x9c,

				0x6a, 0xe6, 0x15, 0x12, 0x21, 0x74, 0x5a, 0xec,
				0x81, 0x4c, 0x45, 0xb0, 0xb0, 0x5b, 0x56, 0x54,
				0x36, 0xfd, 0x6f, 0x13, 0x7a, 0xa1, 0x0a, 0x0c,
				0x0b, 0x64, 0x37, 0x61, 0xdb, 0xd6, 0xf9, 0xa9,

				0xdc, 0xb9, 0x9b, 0x1a, 0x6e, 0x69, 0x08, 0x54,
				0xce, 0x07, 0x69, 0xcd, 0xe3, 0x97, 0x61, 0xd8,
				0x2f, 0xcd, 0xec, 0x15, 0xf0, 0xd9, 0x2d, 0x7d,
				0x8e, 0x94, 0xad, 0xe8, 0xeb, 0x83, 0xfb, 0xe0
			}
		},
		{
			32,
			{
				0x99, 0xe5, 0x82, 0x2d, 0xd4, 0x17, 0x3c, 0x99,
				0x5e, 0x3d, 0xae, 0x0d, 0xde, 0xfb, 0x97, 0x74,
				0x3f, 0xde, 0x3b, 0x08, 0x01, 0x34, 0xb3, 0x9f,
				0x76, 0xe9, 0xbf, 0x8d, 0x0e, 0x88, 0xd5, 0x46
			}
		},
		{
			16,
			{
				0x26, 0x37, 0x40, 0x8f, 0xe1, 0x30, 0x86, 0xea,
				0x73, 0xf9, 0x71, 0xe3, 0x42, 0x5e, 0x28, 0x20
			}
		},
	},
	/*
	 * test vectors from Hanno Böck
	 */
	{
		{
			257,
			{
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
				0xcc, 0x80, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,

				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
				0xcc, 0xcc, 0xcc, 0xcc, 0xce, 0xcc, 0xcc, 0xcc,

				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xc5,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,

				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xe3, 0xcc, 0xcc,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,

				0xcc, 0xcc, 0xcc, 0xcc, 0xac, 0xcc, 0xcc, 0xcc,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xe6,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x00, 0x00, 0x00,
				0xaf, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,

				0xcc, 0xcc, 0xff, 0xff, 0xff, 0xf5, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

				0x00, 0xff, 0xff, 0xff, 0xe7, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x71, 0x92, 0x05, 0xa8, 0x52, 0x1d,

				0xfc
			}
		},
		{
			32,
			{
				0x7f, 0x1b, 0x02, 0x64, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc
			}
		},
		{
			16,
			{
				0x85, 0x59, 0xb8, 0x76, 0xec, 0xee, 0xd6, 0x6e,
				0xb3, 0x77, 0x98, 0xc0, 0x45, 0x7b, 0xaf, 0xf9
			}
		},
	},
	{
		{
			39,
			{
				0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
				0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
				0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
				0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00,

				0x00, 0x00, 0x00, 0x00, 0x80, 0x02, 0x64
			}
		},
		{
			32,
			{
				0xe0, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
				0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa
			}
		},
		{
			16,
			{
				0x00, 0xbd, 0x12, 0x58, 0x97, 0x8e, 0x20, 0x54,
				0x44, 0xc9, 0xaa, 0xaa, 0x82, 0x00, 0x6f, 0xed
			}
		},
	},
	{
		{
			2,
			{
				0x02, 0xfc
			}
		},
		{
			32,
			{
				0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
				0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
				0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
				0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c
			}
		},
		{
			16,
			{
				0x06, 0x12, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
				0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c
			}
		},
	},
	{
		{
			415,
			{
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,

				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7a, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,

				0x7b, 0x7b, 0x5c, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,

				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x6e, 0x7b, 0x00, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,

				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7a, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,

				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x5c,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,

				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,
				0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b,

				0x7b, 0x6e, 0x7b, 0x00, 0x13, 0x00, 0x00, 0x00,
				0x00, 0xb3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

				0xf2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x20, 0x00, 0xef, 0xff, 0x00,
				0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

				0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
				0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x64, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00,

				0xb3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf2,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x20, 0x00, 0xef, 0xff, 0x00, 0x09,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x7a, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,

				0x00, 0x09, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc
			}
		},
		{
			32,
			{
				0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7b, 0x7b
			}
		},
		{
			16,
			{
				0x33, 0x20, 0x5b, 0xbf, 0x9e, 0x9f, 0x8f, 0x72,
				0x12, 0xab, 0x9e, 0x2a, 0xb9, 0xb7, 0xe4, 0xa5
			}
		},
	},
	{
		{
			118,
			{
				0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
				0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
				0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
				0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,

				0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
				0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
				0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
				0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,

				0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
				0x77, 0x77, 0x77, 0x77, 0xff, 0xff, 0xff, 0xe9,
				0xe9, 0xac, 0xac, 0xac, 0xac, 0xac, 0xac, 0xac,
				0xac, 0xac, 0xac, 0xac, 0x00, 0x00, 0xac, 0xac,

				0xec, 0x01, 0x00, 0xac, 0xac, 0xac, 0x2c, 0xac,
				0xa2, 0xac, 0xac, 0xac, 0xac, 0xac, 0xac, 0xac,
				0xac, 0xac, 0xac, 0xac, 0x64, 0xf2
			}
		},
		{
			32,
			{
				0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x7f,
				0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0xcf, 0x77, 0x77, 0x77, 0x77, 0x77,
				0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77
			}
		},
		{
			16,
			{
				0x02, 0xee, 0x7c, 0x8c, 0x54, 0x6d, 0xde, 0xb1,
				0xa4, 0x67, 0xe4, 0xc3, 0x98, 0x11, 0x58, 0xb9
			}
		},
	},
	/*
	 * test vectors from Andrew Moon
	 */
	{ /* nacl */
		{
			131,
			{
				0x8e, 0x99, 0x3b, 0x9f, 0x48, 0x68, 0x12, 0x73,
				0xc2, 0x96, 0x50, 0xba, 0x32, 0xfc, 0x76, 0xce,
				0x48, 0x33, 0x2e, 0xa7, 0x16, 0x4d, 0x96, 0xa4,
				0x47, 0x6f, 0xb8, 0xc5, 0x31, 0xa1, 0x18, 0x6a,

				0xc0, 0xdf, 0xc1, 0x7c, 0x98, 0xdc, 0xe8, 0x7b,
				0x4d, 0xa7, 0xf0, 0x11, 0xec, 0x48, 0xc9, 0x72,
				0x71, 0xd2, 0xc2, 0x0f, 0x9b, 0x92, 0x8f, 0xe2,
				0x27, 0x0d, 0x6f, 0xb8, 0x63, 0xd5, 0x17, 0x38,

				0xb4, 0x8e, 0xee, 0xe3, 0x14, 0xa7, 0xcc, 0x8a,
				0xb9, 0x32, 0x16, 0x45, 0x48, 0xe5, 0x26, 0xae,
				0x90, 0x22, 0x43, 0x68, 0x51, 0x7a, 0xcf, 0xea,
				0xbd, 0x6b, 0xb3, 0x73, 0x2b, 0xc0, 0xe9, 0xda,

				0x99, 0x83, 0x2b, 0x61, 0xca, 0x01, 0xb6, 0xde,
				0x56, 0x24, 0x4a, 0x9e, 0x88, 0xd5, 0xf9, 0xb3,
				0x79, 0x73, 0xf6, 0x22, 0xa4, 0x3d, 0x14, 0xa6,
				0x59, 0x9b, 0x1f, 0x65, 0x4c, 0xb4, 0x5a, 0x74,

				0xe3, 0x55, 0xa5
			}
		},
		{
			32,
			{
				0xee, 0xa6, 0xa7, 0x25, 0x1c, 0x1e, 0x72, 0x91,
				0x6d, 0x11, 0xc2, 0xcb, 0x21, 0x4d, 0x3c, 0x25,
				0x25, 0x39, 0x12, 0x1d, 0x8e, 0x23, 0x4e, 0x65,
				0x2d, 0x65, 0x1f, 0xa4, 0xc8, 0xcf, 0xf8, 0x80
			}
		},
		{
			16,
			{
				0xf3, 0xff, 0xc7, 0x70, 0x3f, 0x94, 0x00, 0xe5,
				0x2a, 0x7d, 0xfb, 0x4b, 0x3d, 0x33, 0x05, 0xd9
			}
		},
	},
	{ /* wrap 2^130-5 */
		{
			16,
			{
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
			}
		},
		{
			32,
			{
				0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			16,
			{
				0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
	},
	{ /* wrap 2^128 */
		{
			16,
			{
				0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			32,
			{
				0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
			}
		},
		{
			16,
			{
				0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
	},
	{ /* limb carry */
		{
			48,
			{
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

				0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			32,
			{
				0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			16,
			{
				0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
	},
	{ /* 2^130-5 */
		{
			48,
			{
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xfb, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
				0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,

				0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
				0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
			}
		},
		{
			32,
			{
				0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			16,
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
	},
	{ /* 2^130-6 */
		{
			16,
			{
				0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
			}
		},
		{
			32,
			{
				0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			16,
			{
				0xfa, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
			}
		},
	},
	{ /* 5*H+L reduction intermediate */
		{
			64,
			{
				0xe3, 0x35, 0x94, 0xd7, 0x50, 0x5e, 0x43, 0xb9,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x33, 0x94, 0xd7, 0x50, 0x5e, 0x43, 0x79, 0xcd,
				0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			32,
			{
				0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			16,
			{
				0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
	},
	{ /* 5*H+L reduction final */
		{
			48,
			{
				0xe3, 0x35, 0x94, 0xd7, 0x50, 0x5e, 0x43, 0xb9,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x33, 0x94, 0xd7, 0x50, 0x5e, 0x43, 0x79, 0xcd,
				0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			32,
			{
				0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		},
		{
			16,
			{
				0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}
		}
	}
};

class TestInnerBlock_Poly1305Internal {
public:
	static int test_poly1305(int idx)
	{
		POLY1305 poly1305;
		const Poly1305Internal_TESTDATA test = TestData_Poly1305Internal_tests[idx];
		const uchar * in = test.input.data;
		size_t inlen = test.input.size;
		const uchar * key = test.key.data;
		const uchar * expected = test.expected.data;
		size_t expectedlen = test.expected.size;
		uchar out[16];
		if(!TEST_size_t_eq(expectedlen, sizeof(out)))
			return 0;
		Poly1305_Init(&poly1305, key);
		Poly1305_Update(&poly1305, in, inlen);
		Poly1305_Final(&poly1305, out);
		if(!TEST_mem_eq(out, expectedlen, expected, expectedlen)) {
			TEST_info("Poly1305 test #%d failed.", idx);
			return 0;
		}
		if(inlen > 16) {
			Poly1305_Init(&poly1305, key);
			Poly1305_Update(&poly1305, in, 1);
			Poly1305_Update(&poly1305, in+1, inlen-1);
			Poly1305_Final(&poly1305, out);
			if(!TEST_mem_eq(out, expectedlen, expected, expectedlen)) {
				TEST_info("Poly1305 test #%d/1+(N-1) failed.", idx);
				return 0;
			}
		}
		if(inlen > 32) {
			size_t half = inlen / 2;
			Poly1305_Init(&poly1305, key);
			Poly1305_Update(&poly1305, in, half);
			Poly1305_Update(&poly1305, in+half, inlen-half);
			Poly1305_Final(&poly1305, out);
			if(!TEST_mem_eq(out, expectedlen, expected, expectedlen)) {
				TEST_info("Poly1305 test #%d/2 failed.", idx);
				return 0;
			}
			for(half = 16; half < inlen; half += 16) {
				Poly1305_Init(&poly1305, key);
				Poly1305_Update(&poly1305, in, half);
				Poly1305_Update(&poly1305, in+half, inlen-half);
				Poly1305_Final(&poly1305, out);
				if(!TEST_mem_eq(out, expectedlen, expected, expectedlen)) {
					TEST_info("Poly1305 test #%d/%zu+%zu failed.", idx, half, inlen-half);
					return 0;
				}
			}
		}
		return 1;
	}
};
//
//
//
#if !defined OPENSSL_NO_RC4 && !defined OPENSSL_NO_MD5 || !defined OPENSSL_NO_DES && !defined OPENSSL_NO_SHA1
static const char TestData_Pbe_pbe_password[] = "MyVoiceIsMyPassport";
static uchar TestData_Pbe_pbe_salt[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, };
static const int TestData_Pbe_pbe_iter = 1000;
static uchar TestData_Pbe_pbe_plaintext[] = {
	0x57, 0x65, 0x20, 0x61, 0x72, 0x65, 0x20, 0x61,
	0x6c, 0x6c, 0x20, 0x6d, 0x61, 0x64, 0x65, 0x20,
	0x6f, 0x66, 0x20, 0x73, 0x74, 0x61, 0x72, 0x73,
};
#endif
// 
// Expected output generated using OpenSSL 1.1.1
// 
#if !defined OPENSSL_NO_RC4 && !defined OPENSSL_NO_MD5
static const uchar TestData_Pbe_pbe_ciphertext_rc4_md5[] = {
	0x21, 0x90, 0xfa, 0xee, 0x95, 0x66, 0x59, 0x45,
	0xfa, 0x1e, 0x9f, 0xe2, 0x25, 0xd2, 0xf9, 0x71,
	0x94, 0xe4, 0x3d, 0xc9, 0x7c, 0xb0, 0x07, 0x23,
};
#endif
#if !defined OPENSSL_NO_DES && !defined OPENSSL_NO_SHA1
static const uchar TestData_Pbe_pbe_ciphertext_des_sha1[] = {
	0xce, 0x4b, 0xb0, 0x0a, 0x7b, 0x48, 0xd7, 0xe3,
	0x9a, 0x9f, 0x46, 0xd6, 0x41, 0x42, 0x4b, 0x44,
	0x36, 0x45, 0x5f, 0x60, 0x8f, 0x3c, 0xd0, 0x55,
	0xd0, 0x8d, 0xa9, 0xab, 0x78, 0x5b, 0x63, 0xaf,
};
#endif
class TestInnerBlock_Pbe {
public:
	#if !defined OPENSSL_NO_RC4 && !defined OPENSSL_NO_MD5 || !defined OPENSSL_NO_DES && !defined OPENSSL_NO_SHA1
	static int test_pkcs5_pbe(const EVP_CIPHER * cipher, const EVP_MD * md, const uchar * exp, const int exp_len)
	{
		int ret = 0;
		X509_ALGOR * algor = NULL;
		int i, outlen;
		uchar out[32];
		EVP_CIPHER_CTX * ctx = EVP_CIPHER_CTX_new();
		if(!TEST_ptr(ctx))
			goto err;
		algor = X509_ALGOR_new();
		if(!TEST_ptr(algor))
			goto err;
		if(!TEST_true(PKCS5_pbe_set0_algor(algor, EVP_CIPHER_nid(cipher), TestData_Pbe_pbe_iter, TestData_Pbe_pbe_salt, sizeof(TestData_Pbe_pbe_salt))))
			goto err;
		if(!TEST_true(PKCS5_PBE_keyivgen(ctx, TestData_Pbe_pbe_password, strlen(TestData_Pbe_pbe_password), algor->parameter, cipher, md, 1)))
			goto err;
		if(!TEST_true(EVP_CipherUpdate(ctx, out, &i, TestData_Pbe_pbe_plaintext, sizeof(TestData_Pbe_pbe_plaintext))))
			goto err;
		outlen = i;
		if(!TEST_true(EVP_CipherFinal_ex(ctx, out + i, &i)))
			goto err;
		outlen += i;
		if(!TEST_mem_eq(out, outlen, exp, exp_len))
			goto err;
		/* Decrypt */
		if(!TEST_true(PKCS5_PBE_keyivgen(ctx, TestData_Pbe_pbe_password, strlen(TestData_Pbe_pbe_password),
			algor->parameter, cipher, md, 0))
			|| !TEST_true(EVP_CipherUpdate(ctx, out, &i, exp, exp_len)))
			goto err;
		outlen = i;
		if(!TEST_true(EVP_CipherFinal_ex(ctx, out + i, &i)))
			goto err;
		if(!TEST_mem_eq(out, outlen, TestData_Pbe_pbe_plaintext, sizeof(TestData_Pbe_pbe_plaintext)))
			goto err;
		ret = 1;
	err:
		EVP_CIPHER_CTX_free(ctx);
		X509_ALGOR_free(algor);
		return ret;
	}
	#endif
	#if !defined OPENSSL_NO_RC4 && !defined OPENSSL_NO_MD5
	static int test_pkcs5_pbe_rc4_md5()
	{
		return test_pkcs5_pbe(EVP_rc4(), EVP_md5(), TestData_Pbe_pbe_ciphertext_rc4_md5, sizeof(TestData_Pbe_pbe_ciphertext_rc4_md5));
	}
	#endif
	#if !defined OPENSSL_NO_DES && !defined OPENSSL_NO_SHA1
	static int test_pkcs5_pbe_des_sha1()
	{
		return test_pkcs5_pbe(EVP_des_cbc(), EVP_sha1(), TestData_Pbe_pbe_ciphertext_des_sha1, sizeof(TestData_Pbe_pbe_ciphertext_des_sha1));
	}
	#endif
};
// 
// CAST low level APIs are deprecated for public use, but still ok for internal use.
// 
#ifndef OPENSSL_NO_CAST
static uchar TestData_Cast_k[16] = {
	0x01, 0x23, 0x45, 0x67, 0x12, 0x34, 0x56, 0x78,
	0x23, 0x45, 0x67, 0x89, 0x34, 0x56, 0x78, 0x9A
};

static uchar TestData_Cast_in[8] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
static int TestData_Cast_k_len[3] = { 16, 10, 5 };

static uchar TestData_Cast_c[3][8] = {
	{0x23, 0x8B, 0x4F, 0xE5, 0x84, 0x7E, 0x44, 0xB2},
	{0xEB, 0x6A, 0x71, 0x1A, 0x2C, 0x02, 0x27, 0x1B},
	{0x7A, 0xC8, 0x16, 0xD1, 0x6E, 0x9B, 0x30, 0x2E},
};

static uchar TestData_Cast_in_a[16] = { 0x01, 0x23, 0x45, 0x67, 0x12, 0x34, 0x56, 0x78, 0x23, 0x45, 0x67, 0x89, 0x34, 0x56, 0x78, 0x9A };
static uchar TestData_Cast_in_b[16] = { 0x01, 0x23, 0x45, 0x67, 0x12, 0x34, 0x56, 0x78, 0x23, 0x45, 0x67, 0x89, 0x34, 0x56, 0x78, 0x9A };
static uchar TestData_Cast_c_a[16] = { 0xEE, 0xA9, 0xD0, 0xA2, 0x49, 0xFD, 0x3B, 0xA6, 0xB3, 0x43, 0x6F, 0xB8, 0x9D, 0x6D, 0xCA, 0x92 };
static uchar TestData_Cast_c_b[16] = { 0xB2, 0xC9, 0x5E, 0xB0, 0x0C, 0x31, 0xAD, 0x71, 0x80, 0xAC, 0x05, 0xB8, 0xE8, 0x3D, 0x69, 0x6E };

class TestInnerBlock_Cast {
public:
	static int cast_test_vector(int z)
	{
		int testresult = 1;
		CAST_KEY key;
		uchar out[80];
		CAST_set_key(&key, TestData_Cast_k_len[z], TestData_Cast_k);
		CAST_ecb_encrypt(TestData_Cast_in, out, &key, CAST_ENCRYPT);
		if(!TEST_mem_eq(out, sizeof(TestData_Cast_c[z]), TestData_Cast_c[z], sizeof(TestData_Cast_c[z]))) {
			TEST_info("CAST_ENCRYPT iteration %d failed (len=%d)", z, TestData_Cast_k_len[z]);
			testresult = 0;
		}
		CAST_ecb_encrypt(out, out, &key, CAST_DECRYPT);
		if(!TEST_mem_eq(out, sizeof(TestData_Cast_in), TestData_Cast_in, sizeof(TestData_Cast_in))) {
			TEST_info("CAST_DECRYPT iteration %d failed (len=%d)", z, TestData_Cast_k_len[z]);
			testresult = 0;
		}
		return testresult;
	}
	static int cast_test_iterations()
	{
		long l;
		int testresult = 1;
		CAST_KEY key, key_b;
		uchar out_a[16], out_b[16];
		memcpy(out_a, TestData_Cast_in_a, sizeof(TestData_Cast_in_a));
		memcpy(out_b, TestData_Cast_in_b, sizeof(TestData_Cast_in_b));
		for(l = 0; l < 1000000L; l++) {
			CAST_set_key(&key_b, 16, out_b);
			CAST_ecb_encrypt(&(out_a[0]), &(out_a[0]), &key_b, CAST_ENCRYPT);
			CAST_ecb_encrypt(&(out_a[8]), &(out_a[8]), &key_b, CAST_ENCRYPT);
			CAST_set_key(&key, 16, out_a);
			CAST_ecb_encrypt(&(out_b[0]), &(out_b[0]), &key, CAST_ENCRYPT);
			CAST_ecb_encrypt(&(out_b[8]), &(out_b[8]), &key, CAST_ENCRYPT);
		}
		if(!TEST_mem_eq(out_a, sizeof(TestData_Cast_c_a), TestData_Cast_c_a, sizeof(TestData_Cast_c_a)) || !TEST_mem_eq(out_b, sizeof(TestData_Cast_c_b), TestData_Cast_c_b, sizeof(TestData_Cast_c_b)))
			testresult = 0;
		return testresult;
	}
};
#endif
// 
// Internal tests for the chacha module. EVP tests would exercise
// complete 32-byte blocks. This test goes per byte...
// 
static const uint TestData_ChachaInternal_key[] = { 0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c, 0x13121110, 0x17161514, 0x1b1a1918, 0x1f1e1d1c };
static const uint TestData_ChachaInternal_ivp[] = { 0x00000000, 0x00000000, 0x03020100, 0x07060504 };
static const uchar TestData_ChachaInternal_ref[] = {
	0xf7, 0x98, 0xa1, 0x89, 0xf1, 0x95, 0xe6, 0x69,
	0x82, 0x10, 0x5f, 0xfb, 0x64, 0x0b, 0xb7, 0x75,
	0x7f, 0x57, 0x9d, 0xa3, 0x16, 0x02, 0xfc, 0x93,
	0xec, 0x01, 0xac, 0x56, 0xf8, 0x5a, 0xc3, 0xc1,
	0x34, 0xa4, 0x54, 0x7b, 0x73, 0x3b, 0x46, 0x41,
	0x30, 0x42, 0xc9, 0x44, 0x00, 0x49, 0x17, 0x69,
	0x05, 0xd3, 0xbe, 0x59, 0xea, 0x1c, 0x53, 0xf1,
	0x59, 0x16, 0x15, 0x5c, 0x2b, 0xe8, 0x24, 0x1a,
	0x38, 0x00, 0x8b, 0x9a, 0x26, 0xbc, 0x35, 0x94,
	0x1e, 0x24, 0x44, 0x17, 0x7c, 0x8a, 0xde, 0x66,
	0x89, 0xde, 0x95, 0x26, 0x49, 0x86, 0xd9, 0x58,
	0x89, 0xfb, 0x60, 0xe8, 0x46, 0x29, 0xc9, 0xbd,
	0x9a, 0x5a, 0xcb, 0x1c, 0xc1, 0x18, 0xbe, 0x56,
	0x3e, 0xb9, 0xb3, 0xa4, 0xa4, 0x72, 0xf8, 0x2e,
	0x09, 0xa7, 0xe7, 0x78, 0x49, 0x2b, 0x56, 0x2e,
	0xf7, 0x13, 0x0e, 0x88, 0xdf, 0xe0, 0x31, 0xc7,
	0x9d, 0xb9, 0xd4, 0xf7, 0xc7, 0xa8, 0x99, 0x15,
	0x1b, 0x9a, 0x47, 0x50, 0x32, 0xb6, 0x3f, 0xc3,
	0x85, 0x24, 0x5f, 0xe0, 0x54, 0xe3, 0xdd, 0x5a,
	0x97, 0xa5, 0xf5, 0x76, 0xfe, 0x06, 0x40, 0x25,
	0xd3, 0xce, 0x04, 0x2c, 0x56, 0x6a, 0xb2, 0xc5,
	0x07, 0xb1, 0x38, 0xdb, 0x85, 0x3e, 0x3d, 0x69,
	0x59, 0x66, 0x09, 0x96, 0x54, 0x6c, 0xc9, 0xc4,
	0xa6, 0xea, 0xfd, 0xc7, 0x77, 0xc0, 0x40, 0xd7,
	0x0e, 0xaf, 0x46, 0xf7, 0x6d, 0xad, 0x39, 0x79,
	0xe5, 0xc5, 0x36, 0x0c, 0x33, 0x17, 0x16, 0x6a,
	0x1c, 0x89, 0x4c, 0x94, 0xa3, 0x71, 0x87, 0x6a,
	0x94, 0xdf, 0x76, 0x28, 0xfe, 0x4e, 0xaa, 0xf2,
	0xcc, 0xb2, 0x7d, 0x5a, 0xaa, 0xe0, 0xad, 0x7a,
	0xd0, 0xf9, 0xd4, 0xb6, 0xad, 0x3b, 0x54, 0x09,
	0x87, 0x46, 0xd4, 0x52, 0x4d, 0x38, 0x40, 0x7a,
	0x6d, 0xeb, 0x3a, 0xb7, 0x8f, 0xab, 0x78, 0xc9,
	0x42, 0x13, 0x66, 0x8b, 0xbb, 0xd3, 0x94, 0xc5,
	0xde, 0x93, 0xb8, 0x53, 0x17, 0x8a, 0xdd, 0xd6,
	0xb9, 0x7f, 0x9f, 0xa1, 0xec, 0x3e, 0x56, 0xc0,
	0x0c, 0x9d, 0xdf, 0xf0, 0xa4, 0x4a, 0x20, 0x42,
	0x41, 0x17, 0x5a, 0x4c, 0xab, 0x0f, 0x96, 0x1b,
	0xa5, 0x3e, 0xde, 0x9b, 0xdf, 0x96, 0x0b, 0x94,
	0xf9, 0x82, 0x9b, 0x1f, 0x34, 0x14, 0x72, 0x64,
	0x29, 0xb3, 0x62, 0xc5, 0xb5, 0x38, 0xe3, 0x91,
	0x52, 0x0f, 0x48, 0x9b, 0x7e, 0xd8, 0xd2, 0x0a,
	0xe3, 0xfd, 0x49, 0xe9, 0xe2, 0x59, 0xe4, 0x43,
	0x97, 0x51, 0x4d, 0x61, 0x8c, 0x96, 0xc4, 0x84,
	0x6b, 0xe3, 0xc6, 0x80, 0xbd, 0xc1, 0x1c, 0x71,
	0xdc, 0xbb, 0xe2, 0x9c, 0xcf, 0x80, 0xd6, 0x2a,
	0x09, 0x38, 0xfa, 0x54, 0x93, 0x91, 0xe6, 0xea,
	0x57, 0xec, 0xbe, 0x26, 0x06, 0x79, 0x0e, 0xc1,
	0x5d, 0x22, 0x24, 0xae, 0x30, 0x7c, 0x14, 0x42,
	0x26, 0xb7, 0xc4, 0xe8, 0xc2, 0xf9, 0x7d, 0x2a,
	0x1d, 0x67, 0x85, 0x2d, 0x29, 0xbe, 0xba, 0x11,
	0x0e, 0xdd, 0x44, 0x51, 0x97, 0x01, 0x20, 0x62,
	0xa3, 0x93, 0xa9, 0xc9, 0x28, 0x03, 0xad, 0x3b,
	0x4f, 0x31, 0xd7, 0xbc, 0x60, 0x33, 0xcc, 0xf7,
	0x93, 0x2c, 0xfe, 0xd3, 0xf0, 0x19, 0x04, 0x4d,
	0x25, 0x90, 0x59, 0x16, 0x77, 0x72, 0x86, 0xf8,
	0x2f, 0x9a, 0x4c, 0xc1, 0xff, 0xe4, 0x30, 0xff,
	0xd1, 0xdc, 0xfc, 0x27, 0xde, 0xed, 0x32, 0x7b,
	0x9f, 0x96, 0x30, 0xd2, 0xfa, 0x96, 0x9f, 0xb6,
	0xf0, 0x60, 0x3c, 0xd1, 0x9d, 0xd9, 0xa9, 0x51,
	0x9e, 0x67, 0x3b, 0xcf, 0xcd, 0x90, 0x14, 0x12,
	0x52, 0x91, 0xa4, 0x46, 0x69, 0xef, 0x72, 0x85,
	0xe7, 0x4e, 0xd3, 0x72, 0x9b, 0x67, 0x7f, 0x80,
	0x1c, 0x3c, 0xdf, 0x05, 0x8c, 0x50, 0x96, 0x31,
	0x68, 0xb4, 0x96, 0x04, 0x37, 0x16, 0xc7, 0x30,
	0x7c, 0xd9, 0xe0, 0xcd, 0xd1, 0x37, 0xfc, 0xcb,
	0x0f, 0x05, 0xb4, 0x7c, 0xdb, 0xb9, 0x5c, 0x5f,
	0x54, 0x83, 0x16, 0x22, 0xc3, 0x65, 0x2a, 0x32,
	0xb2, 0x53, 0x1f, 0xe3, 0x26, 0xbc, 0xd6, 0xe2,
	0xbb, 0xf5, 0x6a, 0x19, 0x4f, 0xa1, 0x96, 0xfb,
	0xd1, 0xa5, 0x49, 0x52, 0x11, 0x0f, 0x51, 0xc7,
	0x34, 0x33, 0x86, 0x5f, 0x76, 0x64, 0xb8, 0x36,
	0x68, 0x5e, 0x36, 0x64, 0xb3, 0xd8, 0x44, 0x4a,
	0xf8, 0x9a, 0x24, 0x28, 0x05, 0xe1, 0x8c, 0x97,
	0x5f, 0x11, 0x46, 0x32, 0x49, 0x96, 0xfd, 0xe1,
	0x70, 0x07, 0xcf, 0x3e, 0x6e, 0x8f, 0x4e, 0x76,
	0x40, 0x22, 0x53, 0x3e, 0xdb, 0xfe, 0x07, 0xd4,
	0x73, 0x3e, 0x48, 0xbb, 0x37, 0x2d, 0x75, 0xb0,
	0xef, 0x48, 0xec, 0x98, 0x3e, 0xb7, 0x85, 0x32,
	0x16, 0x1c, 0xc5, 0x29, 0xe5, 0xab, 0xb8, 0x98,
	0x37, 0xdf, 0xcc, 0xa6, 0x26, 0x1d, 0xbb, 0x37,
	0xc7, 0xc5, 0xe6, 0xa8, 0x74, 0x78, 0xbf, 0x41,
	0xee, 0x85, 0xa5, 0x18, 0xc0, 0xf4, 0xef, 0xa9,
	0xbd, 0xe8, 0x28, 0xc5, 0xa7, 0x1b, 0x8e, 0x46,
	0x59, 0x7b, 0x63, 0x4a, 0xfd, 0x20, 0x4d, 0x3c,
	0x50, 0x13, 0x34, 0x23, 0x9c, 0x34, 0x14, 0x28,
	0x5e, 0xd7, 0x2d, 0x3a, 0x91, 0x69, 0xea, 0xbb,
	0xd4, 0xdc, 0x25, 0xd5, 0x2b, 0xb7, 0x51, 0x6d,
	0x3b, 0xa7, 0x12, 0xd7, 0x5a, 0xd8, 0xc0, 0xae,
	0x5d, 0x49, 0x3c, 0x19, 0xe3, 0x8a, 0x77, 0x93,
	0x9e, 0x7a, 0x05, 0x8d, 0x71, 0x3e, 0x9c, 0xcc,
	0xca, 0x58, 0x04, 0x5f, 0x43, 0x6b, 0x43, 0x4b,
	0x1c, 0x80, 0xd3, 0x65, 0x47, 0x24, 0x06, 0xe3,
	0x92, 0x95, 0x19, 0x87, 0xdb, 0x69, 0x05, 0xc8,
	0x0d, 0x43, 0x1d, 0xa1, 0x84, 0x51, 0x13, 0x5b,
	0xe7, 0xe8, 0x2b, 0xca, 0xb3, 0x58, 0xcb, 0x39,
	0x71, 0xe6, 0x14, 0x05, 0xb2, 0xff, 0x17, 0x98,
	0x0d, 0x6e, 0x7e, 0x67, 0xe8, 0x61, 0xe2, 0x82,
	0x01, 0xc1, 0xee, 0x30, 0xb4, 0x41, 0x04, 0x0f,
	0xd0, 0x68, 0x78, 0xd6, 0x50, 0x42, 0xc9, 0x55,
	0x82, 0xa4, 0x31, 0x82, 0x07, 0xbf, 0xc7, 0x00,
	0xbe, 0x0c, 0xe3, 0x28, 0x89, 0xae, 0xc2, 0xff,
	0xe5, 0x08, 0x5e, 0x89, 0x67, 0x91, 0x0d, 0x87,
	0x9f, 0xa0, 0xe8, 0xc0, 0xff, 0x85, 0xfd, 0xc5,
	0x10, 0xb9, 0xff, 0x2f, 0xbf, 0x87, 0xcf, 0xcb,
	0x29, 0x57, 0x7d, 0x68, 0x09, 0x9e, 0x04, 0xff,
	0xa0, 0x5f, 0x75, 0x2a, 0x73, 0xd3, 0x77, 0xc7,
	0x0d, 0x3a, 0x8b, 0xc2, 0xda, 0x80, 0xe6, 0xe7,
	0x80, 0xec, 0x05, 0x71, 0x82, 0xc3, 0x3a, 0xd1,
	0xde, 0x38, 0x72, 0x52, 0x25, 0x8a, 0x1e, 0x18,
	0xe6, 0xfa, 0xd9, 0x10, 0x32, 0x7c, 0xe7, 0xf4,
	0x2f, 0xd1, 0xe1, 0xe0, 0x51, 0x5f, 0x95, 0x86,
	0xe2, 0xf2, 0xef, 0xcb, 0x9f, 0x47, 0x2b, 0x1d,
	0xbd, 0xba, 0xc3, 0x54, 0xa4, 0x16, 0x21, 0x51,
	0xe9, 0xd9, 0x2c, 0x79, 0xfb, 0x08, 0xbb, 0x4d,
	0xdc, 0x56, 0xf1, 0x94, 0x48, 0xc0, 0x17, 0x5a,
	0x46, 0xe2, 0xe6, 0xc4, 0x91, 0xfe, 0xc7, 0x14,
	0x19, 0xaa, 0x43, 0xa3, 0x49, 0xbe, 0xa7, 0x68,
	0xa9, 0x2c, 0x75, 0xde, 0x68, 0xfd, 0x95, 0x91,
	0xe6, 0x80, 0x67, 0xf3, 0x19, 0x70, 0x94, 0xd3,
	0xfb, 0x87, 0xed, 0x81, 0x78, 0x5e, 0xa0, 0x75,
	0xe4, 0xb6, 0x5e, 0x3e, 0x4c, 0x78, 0xf8, 0x1d,
	0xa9, 0xb7, 0x51, 0xc5, 0xef, 0xe0, 0x24, 0x15,
	0x23, 0x01, 0xc4, 0x8e, 0x63, 0x24, 0x5b, 0x55,
	0x6c, 0x4c, 0x67, 0xaf, 0xf8, 0x57, 0xe5, 0xea,
	0x15, 0xa9, 0x08, 0xd8, 0x3a, 0x1d, 0x97, 0x04,
	0xf8, 0xe5, 0x5e, 0x73, 0x52, 0xb2, 0x0b, 0x69,
	0x4b, 0xf9, 0x97, 0x02, 0x98, 0xe6, 0xb5, 0xaa,
	0xd3, 0x3e, 0xa2, 0x15, 0x5d, 0x10, 0x5d, 0x4e
};

class TestInnerBlock_ChachaInternal {
public:
	static int test_cha_cha_internal(int n)
	{
		uchar buf[sizeof(TestData_ChachaInternal_ref)];
		uint i = n + 1, j;
		memzero(buf, i);
		memcpy(buf + i, TestData_ChachaInternal_ref + i, sizeof(TestData_ChachaInternal_ref) - i);
		ChaCha20_ctr32(buf, buf, i, TestData_ChachaInternal_key, TestData_ChachaInternal_ivp);
		// 
		// Idea behind checking for whole sizeof(ref) is that if
		// ChaCha20_ctr32 oversteps i-th byte, then we'd know
		// 
		for(j = 0; j < sizeof(TestData_ChachaInternal_ref); j++)
			if(!TEST_uchar_eq(buf[j], TestData_ChachaInternal_ref[j])) {
				TEST_info("%d failed at %u (%02x)\n", i, j, buf[j]);
				return 0;
			}
		return 1;
	}
};
//
//
//
typedef struct cipher_id_name { // @todo replace with SIntToSymbTabEntry
	int id;
	const char * name;
} CIPHER_ID_NAME;

// Cipher suites, copied from t1_trce.c 
static CIPHER_ID_NAME TestData_Ciphername_cipher_names[] = {
	{0x0000, "TLS_NULL_WITH_NULL_NULL"},
	{0x0001, "TLS_RSA_WITH_NULL_MD5"},
	{0x0002, "TLS_RSA_WITH_NULL_SHA"},
	{0x0003, "TLS_RSA_EXPORT_WITH_RC4_40_MD5"},
	{0x0004, "TLS_RSA_WITH_RC4_128_MD5"},
	{0x0005, "TLS_RSA_WITH_RC4_128_SHA"},
	{0x0006, "TLS_RSA_EXPORT_WITH_RC2_CBC_40_MD5"},
	{0x0007, "TLS_RSA_WITH_IDEA_CBC_SHA"},
	{0x0008, "TLS_RSA_EXPORT_WITH_DES40_CBC_SHA"},
	{0x0009, "TLS_RSA_WITH_DES_CBC_SHA"},
	{0x000A, "TLS_RSA_WITH_3DES_EDE_CBC_SHA"},
	{0x000B, "TLS_DH_DSS_EXPORT_WITH_DES40_CBC_SHA"},
	{0x000C, "TLS_DH_DSS_WITH_DES_CBC_SHA"},
	{0x000D, "TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA"},
	{0x000E, "TLS_DH_RSA_EXPORT_WITH_DES40_CBC_SHA"},
	{0x000F, "TLS_DH_RSA_WITH_DES_CBC_SHA"},
	{0x0010, "TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA"},
	{0x0011, "TLS_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA"},
	{0x0012, "TLS_DHE_DSS_WITH_DES_CBC_SHA"},
	{0x0013, "TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA"},
	{0x0014, "TLS_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA"},
	{0x0015, "TLS_DHE_RSA_WITH_DES_CBC_SHA"},
	{0x0016, "TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA"},
	{0x0017, "TLS_DH_anon_EXPORT_WITH_RC4_40_MD5"},
	{0x0018, "TLS_DH_anon_WITH_RC4_128_MD5"},
	{0x0019, "TLS_DH_anon_EXPORT_WITH_DES40_CBC_SHA"},
	{0x001A, "TLS_DH_anon_WITH_DES_CBC_SHA"},
	{0x001B, "TLS_DH_anon_WITH_3DES_EDE_CBC_SHA"},
	{0x001D, "SSL_FORTEZZA_KEA_WITH_FORTEZZA_CBC_SHA"},
	{0x001E, "SSL_FORTEZZA_KEA_WITH_RC4_128_SHA"},
	{0x001F, "TLS_KRB5_WITH_3DES_EDE_CBC_SHA"},
	{0x0020, "TLS_KRB5_WITH_RC4_128_SHA"},
	{0x0021, "TLS_KRB5_WITH_IDEA_CBC_SHA"},
	{0x0022, "TLS_KRB5_WITH_DES_CBC_MD5"},
	{0x0023, "TLS_KRB5_WITH_3DES_EDE_CBC_MD5"},
	{0x0024, "TLS_KRB5_WITH_RC4_128_MD5"},
	{0x0025, "TLS_KRB5_WITH_IDEA_CBC_MD5"},
	{0x0026, "TLS_KRB5_EXPORT_WITH_DES_CBC_40_SHA"},
	{0x0027, "TLS_KRB5_EXPORT_WITH_RC2_CBC_40_SHA"},
	{0x0028, "TLS_KRB5_EXPORT_WITH_RC4_40_SHA"},
	{0x0029, "TLS_KRB5_EXPORT_WITH_DES_CBC_40_MD5"},
	{0x002A, "TLS_KRB5_EXPORT_WITH_RC2_CBC_40_MD5"},
	{0x002B, "TLS_KRB5_EXPORT_WITH_RC4_40_MD5"},
	{0x002C, "TLS_PSK_WITH_NULL_SHA"},
	{0x002D, "TLS_DHE_PSK_WITH_NULL_SHA"},
	{0x002E, "TLS_RSA_PSK_WITH_NULL_SHA"},
	{0x002F, "TLS_RSA_WITH_AES_128_CBC_SHA"},
	{0x0030, "TLS_DH_DSS_WITH_AES_128_CBC_SHA"},
	{0x0031, "TLS_DH_RSA_WITH_AES_128_CBC_SHA"},
	{0x0032, "TLS_DHE_DSS_WITH_AES_128_CBC_SHA"},
	{0x0033, "TLS_DHE_RSA_WITH_AES_128_CBC_SHA"},
	{0x0034, "TLS_DH_anon_WITH_AES_128_CBC_SHA"},
	{0x0035, "TLS_RSA_WITH_AES_256_CBC_SHA"},
	{0x0036, "TLS_DH_DSS_WITH_AES_256_CBC_SHA"},
	{0x0037, "TLS_DH_RSA_WITH_AES_256_CBC_SHA"},
	{0x0038, "TLS_DHE_DSS_WITH_AES_256_CBC_SHA"},
	{0x0039, "TLS_DHE_RSA_WITH_AES_256_CBC_SHA"},
	{0x003A, "TLS_DH_anon_WITH_AES_256_CBC_SHA"},
	{0x003B, "TLS_RSA_WITH_NULL_SHA256"},
	{0x003C, "TLS_RSA_WITH_AES_128_CBC_SHA256"},
	{0x003D, "TLS_RSA_WITH_AES_256_CBC_SHA256"},
	{0x003E, "TLS_DH_DSS_WITH_AES_128_CBC_SHA256"},
	{0x003F, "TLS_DH_RSA_WITH_AES_128_CBC_SHA256"},
	{0x0040, "TLS_DHE_DSS_WITH_AES_128_CBC_SHA256"},
	{0x0041, "TLS_RSA_WITH_CAMELLIA_128_CBC_SHA"},
	{0x0042, "TLS_DH_DSS_WITH_CAMELLIA_128_CBC_SHA"},
	{0x0043, "TLS_DH_RSA_WITH_CAMELLIA_128_CBC_SHA"},
	{0x0044, "TLS_DHE_DSS_WITH_CAMELLIA_128_CBC_SHA"},
	{0x0045, "TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA"},
	{0x0046, "TLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA"},
	{0x0067, "TLS_DHE_RSA_WITH_AES_128_CBC_SHA256"},
	{0x0068, "TLS_DH_DSS_WITH_AES_256_CBC_SHA256"},
	{0x0069, "TLS_DH_RSA_WITH_AES_256_CBC_SHA256"},
	{0x006A, "TLS_DHE_DSS_WITH_AES_256_CBC_SHA256"},
	{0x006B, "TLS_DHE_RSA_WITH_AES_256_CBC_SHA256"},
	{0x006C, "TLS_DH_anon_WITH_AES_128_CBC_SHA256"},
	{0x006D, "TLS_DH_anon_WITH_AES_256_CBC_SHA256"},
	{0x0084, "TLS_RSA_WITH_CAMELLIA_256_CBC_SHA"},
	{0x0085, "TLS_DH_DSS_WITH_CAMELLIA_256_CBC_SHA"},
	{0x0086, "TLS_DH_RSA_WITH_CAMELLIA_256_CBC_SHA"},
	{0x0087, "TLS_DHE_DSS_WITH_CAMELLIA_256_CBC_SHA"},
	{0x0088, "TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA"},
	{0x0089, "TLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA"},
	{0x008A, "TLS_PSK_WITH_RC4_128_SHA"},
	{0x008B, "TLS_PSK_WITH_3DES_EDE_CBC_SHA"},
	{0x008C, "TLS_PSK_WITH_AES_128_CBC_SHA"},
	{0x008D, "TLS_PSK_WITH_AES_256_CBC_SHA"},
	{0x008E, "TLS_DHE_PSK_WITH_RC4_128_SHA"},
	{0x008F, "TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA"},
	{0x0090, "TLS_DHE_PSK_WITH_AES_128_CBC_SHA"},
	{0x0091, "TLS_DHE_PSK_WITH_AES_256_CBC_SHA"},
	{0x0092, "TLS_RSA_PSK_WITH_RC4_128_SHA"},
	{0x0093, "TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA"},
	{0x0094, "TLS_RSA_PSK_WITH_AES_128_CBC_SHA"},
	{0x0095, "TLS_RSA_PSK_WITH_AES_256_CBC_SHA"},
	{0x0096, "TLS_RSA_WITH_SEED_CBC_SHA"},
	{0x0097, "TLS_DH_DSS_WITH_SEED_CBC_SHA"},
	{0x0098, "TLS_DH_RSA_WITH_SEED_CBC_SHA"},
	{0x0099, "TLS_DHE_DSS_WITH_SEED_CBC_SHA"},
	{0x009A, "TLS_DHE_RSA_WITH_SEED_CBC_SHA"},
	{0x009B, "TLS_DH_anon_WITH_SEED_CBC_SHA"},
	{0x009C, "TLS_RSA_WITH_AES_128_GCM_SHA256"},
	{0x009D, "TLS_RSA_WITH_AES_256_GCM_SHA384"},
	{0x009E, "TLS_DHE_RSA_WITH_AES_128_GCM_SHA256"},
	{0x009F, "TLS_DHE_RSA_WITH_AES_256_GCM_SHA384"},
	{0x00A0, "TLS_DH_RSA_WITH_AES_128_GCM_SHA256"},
	{0x00A1, "TLS_DH_RSA_WITH_AES_256_GCM_SHA384"},
	{0x00A2, "TLS_DHE_DSS_WITH_AES_128_GCM_SHA256"},
	{0x00A3, "TLS_DHE_DSS_WITH_AES_256_GCM_SHA384"},
	{0x00A4, "TLS_DH_DSS_WITH_AES_128_GCM_SHA256"},
	{0x00A5, "TLS_DH_DSS_WITH_AES_256_GCM_SHA384"},
	{0x00A6, "TLS_DH_anon_WITH_AES_128_GCM_SHA256"},
	{0x00A7, "TLS_DH_anon_WITH_AES_256_GCM_SHA384"},
	{0x00A8, "TLS_PSK_WITH_AES_128_GCM_SHA256"},
	{0x00A9, "TLS_PSK_WITH_AES_256_GCM_SHA384"},
	{0x00AA, "TLS_DHE_PSK_WITH_AES_128_GCM_SHA256"},
	{0x00AB, "TLS_DHE_PSK_WITH_AES_256_GCM_SHA384"},
	{0x00AC, "TLS_RSA_PSK_WITH_AES_128_GCM_SHA256"},
	{0x00AD, "TLS_RSA_PSK_WITH_AES_256_GCM_SHA384"},
	{0x00AE, "TLS_PSK_WITH_AES_128_CBC_SHA256"},
	{0x00AF, "TLS_PSK_WITH_AES_256_CBC_SHA384"},
	{0x00B0, "TLS_PSK_WITH_NULL_SHA256"},
	{0x00B1, "TLS_PSK_WITH_NULL_SHA384"},
	{0x00B2, "TLS_DHE_PSK_WITH_AES_128_CBC_SHA256"},
	{0x00B3, "TLS_DHE_PSK_WITH_AES_256_CBC_SHA384"},
	{0x00B4, "TLS_DHE_PSK_WITH_NULL_SHA256"},
	{0x00B5, "TLS_DHE_PSK_WITH_NULL_SHA384"},
	{0x00B6, "TLS_RSA_PSK_WITH_AES_128_CBC_SHA256"},
	{0x00B7, "TLS_RSA_PSK_WITH_AES_256_CBC_SHA384"},
	{0x00B8, "TLS_RSA_PSK_WITH_NULL_SHA256"},
	{0x00B9, "TLS_RSA_PSK_WITH_NULL_SHA384"},
	{0x00BA, "TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256"},
	{0x00BB, "TLS_DH_DSS_WITH_CAMELLIA_128_CBC_SHA256"},
	{0x00BC, "TLS_DH_RSA_WITH_CAMELLIA_128_CBC_SHA256"},
	{0x00BD, "TLS_DHE_DSS_WITH_CAMELLIA_128_CBC_SHA256"},
	{0x00BE, "TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256"},
	{0x00BF, "TLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA256"},
	{0x00C0, "TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256"},
	{0x00C1, "TLS_DH_DSS_WITH_CAMELLIA_256_CBC_SHA256"},
	{0x00C2, "TLS_DH_RSA_WITH_CAMELLIA_256_CBC_SHA256"},
	{0x00C3, "TLS_DHE_DSS_WITH_CAMELLIA_256_CBC_SHA256"},
	{0x00C4, "TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256"},
	{0x00C5, "TLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA256"},
	{0x00FF, "TLS_EMPTY_RENEGOTIATION_INFO_SCSV"},
	{0x5600, "TLS_FALLBACK_SCSV"},
	{0xC001, "TLS_ECDH_ECDSA_WITH_NULL_SHA"},
	{0xC002, "TLS_ECDH_ECDSA_WITH_RC4_128_SHA"},
	{0xC003, "TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA"},
	{0xC004, "TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA"},
	{0xC005, "TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA"},
	{0xC006, "TLS_ECDHE_ECDSA_WITH_NULL_SHA"},
	{0xC007, "TLS_ECDHE_ECDSA_WITH_RC4_128_SHA"},
	{0xC008, "TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA"},
	{0xC009, "TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA"},
	{0xC00A, "TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA"},
	{0xC00B, "TLS_ECDH_RSA_WITH_NULL_SHA"},
	{0xC00C, "TLS_ECDH_RSA_WITH_RC4_128_SHA"},
	{0xC00D, "TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA"},
	{0xC00E, "TLS_ECDH_RSA_WITH_AES_128_CBC_SHA"},
	{0xC00F, "TLS_ECDH_RSA_WITH_AES_256_CBC_SHA"},
	{0xC010, "TLS_ECDHE_RSA_WITH_NULL_SHA"},
	{0xC011, "TLS_ECDHE_RSA_WITH_RC4_128_SHA"},
	{0xC012, "TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA"},
	{0xC013, "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA"},
	{0xC014, "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA"},
	{0xC015, "TLS_ECDH_anon_WITH_NULL_SHA"},
	{0xC016, "TLS_ECDH_anon_WITH_RC4_128_SHA"},
	{0xC017, "TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA"},
	{0xC018, "TLS_ECDH_anon_WITH_AES_128_CBC_SHA"},
	{0xC019, "TLS_ECDH_anon_WITH_AES_256_CBC_SHA"},
	{0xC01A, "TLS_SRP_SHA_WITH_3DES_EDE_CBC_SHA"},
	{0xC01B, "TLS_SRP_SHA_RSA_WITH_3DES_EDE_CBC_SHA"},
	{0xC01C, "TLS_SRP_SHA_DSS_WITH_3DES_EDE_CBC_SHA"},
	{0xC01D, "TLS_SRP_SHA_WITH_AES_128_CBC_SHA"},
	{0xC01E, "TLS_SRP_SHA_RSA_WITH_AES_128_CBC_SHA"},
	{0xC01F, "TLS_SRP_SHA_DSS_WITH_AES_128_CBC_SHA"},
	{0xC020, "TLS_SRP_SHA_WITH_AES_256_CBC_SHA"},
	{0xC021, "TLS_SRP_SHA_RSA_WITH_AES_256_CBC_SHA"},
	{0xC022, "TLS_SRP_SHA_DSS_WITH_AES_256_CBC_SHA"},
	{0xC023, "TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256"},
	{0xC024, "TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384"},
	{0xC025, "TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256"},
	{0xC026, "TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384"},
	{0xC027, "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256"},
	{0xC028, "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384"},
	{0xC029, "TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256"},
	{0xC02A, "TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384"},
	{0xC02B, "TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256"},
	{0xC02C, "TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384"},
	{0xC02D, "TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256"},
	{0xC02E, "TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384"},
	{0xC02F, "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256"},
	{0xC030, "TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384"},
	{0xC031, "TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256"},
	{0xC032, "TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384"},
	{0xC033, "TLS_ECDHE_PSK_WITH_RC4_128_SHA"},
	{0xC034, "TLS_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA"},
	{0xC035, "TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA"},
	{0xC036, "TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA"},
	{0xC037, "TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256"},
	{0xC038, "TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384"},
	{0xC039, "TLS_ECDHE_PSK_WITH_NULL_SHA"},
	{0xC03A, "TLS_ECDHE_PSK_WITH_NULL_SHA256"},
	{0xC03B, "TLS_ECDHE_PSK_WITH_NULL_SHA384"},
	{0xC03C, "TLS_RSA_WITH_ARIA_128_CBC_SHA256"},
	{0xC03D, "TLS_RSA_WITH_ARIA_256_CBC_SHA384"},
	{0xC03E, "TLS_DH_DSS_WITH_ARIA_128_CBC_SHA256"},
	{0xC03F, "TLS_DH_DSS_WITH_ARIA_256_CBC_SHA384"},
	{0xC040, "TLS_DH_RSA_WITH_ARIA_128_CBC_SHA256"},
	{0xC041, "TLS_DH_RSA_WITH_ARIA_256_CBC_SHA384"},
	{0xC042, "TLS_DHE_DSS_WITH_ARIA_128_CBC_SHA256"},
	{0xC043, "TLS_DHE_DSS_WITH_ARIA_256_CBC_SHA384"},
	{0xC044, "TLS_DHE_RSA_WITH_ARIA_128_CBC_SHA256"},
	{0xC045, "TLS_DHE_RSA_WITH_ARIA_256_CBC_SHA384"},
	{0xC046, "TLS_DH_anon_WITH_ARIA_128_CBC_SHA256"},
	{0xC047, "TLS_DH_anon_WITH_ARIA_256_CBC_SHA384"},
	{0xC048, "TLS_ECDHE_ECDSA_WITH_ARIA_128_CBC_SHA256"},
	{0xC049, "TLS_ECDHE_ECDSA_WITH_ARIA_256_CBC_SHA384"},
	{0xC04A, "TLS_ECDH_ECDSA_WITH_ARIA_128_CBC_SHA256"},
	{0xC04B, "TLS_ECDH_ECDSA_WITH_ARIA_256_CBC_SHA384"},
	{0xC04C, "TLS_ECDHE_RSA_WITH_ARIA_128_CBC_SHA256"},
	{0xC04D, "TLS_ECDHE_RSA_WITH_ARIA_256_CBC_SHA384"},
	{0xC04E, "TLS_ECDH_RSA_WITH_ARIA_128_CBC_SHA256"},
	{0xC04F, "TLS_ECDH_RSA_WITH_ARIA_256_CBC_SHA384"},
	{0xC050, "TLS_RSA_WITH_ARIA_128_GCM_SHA256"},
	{0xC051, "TLS_RSA_WITH_ARIA_256_GCM_SHA384"},
	{0xC052, "TLS_DHE_RSA_WITH_ARIA_128_GCM_SHA256"},
	{0xC053, "TLS_DHE_RSA_WITH_ARIA_256_GCM_SHA384"},
	{0xC054, "TLS_DH_RSA_WITH_ARIA_128_GCM_SHA256"},
	{0xC055, "TLS_DH_RSA_WITH_ARIA_256_GCM_SHA384"},
	{0xC056, "TLS_DHE_DSS_WITH_ARIA_128_GCM_SHA256"},
	{0xC057, "TLS_DHE_DSS_WITH_ARIA_256_GCM_SHA384"},
	{0xC058, "TLS_DH_DSS_WITH_ARIA_128_GCM_SHA256"},
	{0xC059, "TLS_DH_DSS_WITH_ARIA_256_GCM_SHA384"},
	{0xC05A, "TLS_DH_anon_WITH_ARIA_128_GCM_SHA256"},
	{0xC05B, "TLS_DH_anon_WITH_ARIA_256_GCM_SHA384"},
	{0xC05C, "TLS_ECDHE_ECDSA_WITH_ARIA_128_GCM_SHA256"},
	{0xC05D, "TLS_ECDHE_ECDSA_WITH_ARIA_256_GCM_SHA384"},
	{0xC05E, "TLS_ECDH_ECDSA_WITH_ARIA_128_GCM_SHA256"},
	{0xC05F, "TLS_ECDH_ECDSA_WITH_ARIA_256_GCM_SHA384"},
	{0xC060, "TLS_ECDHE_RSA_WITH_ARIA_128_GCM_SHA256"},
	{0xC061, "TLS_ECDHE_RSA_WITH_ARIA_256_GCM_SHA384"},
	{0xC062, "TLS_ECDH_RSA_WITH_ARIA_128_GCM_SHA256"},
	{0xC063, "TLS_ECDH_RSA_WITH_ARIA_256_GCM_SHA384"},
	{0xC064, "TLS_PSK_WITH_ARIA_128_CBC_SHA256"},
	{0xC065, "TLS_PSK_WITH_ARIA_256_CBC_SHA384"},
	{0xC066, "TLS_DHE_PSK_WITH_ARIA_128_CBC_SHA256"},
	{0xC067, "TLS_DHE_PSK_WITH_ARIA_256_CBC_SHA384"},
	{0xC068, "TLS_RSA_PSK_WITH_ARIA_128_CBC_SHA256"},
	{0xC069, "TLS_RSA_PSK_WITH_ARIA_256_CBC_SHA384"},
	{0xC06A, "TLS_PSK_WITH_ARIA_128_GCM_SHA256"},
	{0xC06B, "TLS_PSK_WITH_ARIA_256_GCM_SHA384"},
	{0xC06C, "TLS_DHE_PSK_WITH_ARIA_128_GCM_SHA256"},
	{0xC06D, "TLS_DHE_PSK_WITH_ARIA_256_GCM_SHA384"},
	{0xC06E, "TLS_RSA_PSK_WITH_ARIA_128_GCM_SHA256"},
	{0xC06F, "TLS_RSA_PSK_WITH_ARIA_256_GCM_SHA384"},
	{0xC070, "TLS_ECDHE_PSK_WITH_ARIA_128_CBC_SHA256"},
	{0xC071, "TLS_ECDHE_PSK_WITH_ARIA_256_CBC_SHA384"},
	{0xC072, "TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256"},
	{0xC073, "TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384"},
	{0xC074, "TLS_ECDH_ECDSA_WITH_CAMELLIA_128_CBC_SHA256"},
	{0xC075, "TLS_ECDH_ECDSA_WITH_CAMELLIA_256_CBC_SHA384"},
	{0xC076, "TLS_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256"},
	{0xC077, "TLS_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384"},
	{0xC078, "TLS_ECDH_RSA_WITH_CAMELLIA_128_CBC_SHA256"},
	{0xC079, "TLS_ECDH_RSA_WITH_CAMELLIA_256_CBC_SHA384"},
	{0xC07A, "TLS_RSA_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC07B, "TLS_RSA_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC07C, "TLS_DHE_RSA_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC07D, "TLS_DHE_RSA_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC07E, "TLS_DH_RSA_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC07F, "TLS_DH_RSA_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC080, "TLS_DHE_DSS_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC081, "TLS_DHE_DSS_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC082, "TLS_DH_DSS_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC083, "TLS_DH_DSS_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC084, "TLS_DH_anon_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC085, "TLS_DH_anon_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC086, "TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC087, "TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC088, "TLS_ECDH_ECDSA_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC089, "TLS_ECDH_ECDSA_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC08A, "TLS_ECDHE_RSA_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC08B, "TLS_ECDHE_RSA_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC08C, "TLS_ECDH_RSA_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC08D, "TLS_ECDH_RSA_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC08E, "TLS_PSK_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC08F, "TLS_PSK_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC090, "TLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC091, "TLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC092, "TLS_RSA_PSK_WITH_CAMELLIA_128_GCM_SHA256"},
	{0xC093, "TLS_RSA_PSK_WITH_CAMELLIA_256_GCM_SHA384"},
	{0xC094, "TLS_PSK_WITH_CAMELLIA_128_CBC_SHA256"},
	{0xC095, "TLS_PSK_WITH_CAMELLIA_256_CBC_SHA384"},
	{0xC096, "TLS_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256"},
	{0xC097, "TLS_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384"},
	{0xC098, "TLS_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256"},
	{0xC099, "TLS_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384"},
	{0xC09A, "TLS_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256"},
	{0xC09B, "TLS_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384"},
	{0xC09C, "TLS_RSA_WITH_AES_128_CCM"},
	{0xC09D, "TLS_RSA_WITH_AES_256_CCM"},
	{0xC09E, "TLS_DHE_RSA_WITH_AES_128_CCM"},
	{0xC09F, "TLS_DHE_RSA_WITH_AES_256_CCM"},
	{0xC0A0, "TLS_RSA_WITH_AES_128_CCM_8"},
	{0xC0A1, "TLS_RSA_WITH_AES_256_CCM_8"},
	{0xC0A2, "TLS_DHE_RSA_WITH_AES_128_CCM_8"},
	{0xC0A3, "TLS_DHE_RSA_WITH_AES_256_CCM_8"},
	{0xC0A4, "TLS_PSK_WITH_AES_128_CCM"},
	{0xC0A5, "TLS_PSK_WITH_AES_256_CCM"},
	{0xC0A6, "TLS_DHE_PSK_WITH_AES_128_CCM"},
	{0xC0A7, "TLS_DHE_PSK_WITH_AES_256_CCM"},
	{0xC0A8, "TLS_PSK_WITH_AES_128_CCM_8"},
	{0xC0A9, "TLS_PSK_WITH_AES_256_CCM_8"},
	{0xC0AA, "TLS_PSK_DHE_WITH_AES_128_CCM_8"},
	{0xC0AB, "TLS_PSK_DHE_WITH_AES_256_CCM_8"},
	{0xC0AC, "TLS_ECDHE_ECDSA_WITH_AES_128_CCM"},
	{0xC0AD, "TLS_ECDHE_ECDSA_WITH_AES_256_CCM"},
	{0xC0AE, "TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8"},
	{0xC0AF, "TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8"},
	{0xCCA8, "TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256"},
	{0xCCA9, "TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256"},
	{0xCCAA, "TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256"},
	{0xCCAB, "TLS_PSK_WITH_CHACHA20_POLY1305_SHA256"},
	{0xCCAC, "TLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256"},
	{0xCCAD, "TLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256"},
	{0xCCAE, "TLS_RSA_PSK_WITH_CHACHA20_POLY1305_SHA256"},
	{0x1301, "TLS_AES_128_GCM_SHA256"},
	{0x1302, "TLS_AES_256_GCM_SHA384"},
	{0x1303, "TLS_CHACHA20_POLY1305_SHA256"},
	{0x1304, "TLS_AES_128_CCM_SHA256"},
	{0x1305, "TLS_AES_128_CCM_8_SHA256"},
	{0xFEFE, "SSL_RSA_FIPS_WITH_DES_CBC_SHA"},
	{0xFEFF, "SSL_RSA_FIPS_WITH_3DES_EDE_CBC_SHA"},
};

class TestInnerBlock_Ciphername {
public:
	static const char * get_std_name_by_id(int id)
	{
		for(size_t i = 0; i < SIZEOFARRAY(TestData_Ciphername_cipher_names); i++)
			if(TestData_Ciphername_cipher_names[i].id == id)
				return TestData_Ciphername_cipher_names[i].name;
		return NULL;
	}
	static int test_cipher_name()
	{
		SSL_CTX * ctx = NULL;
		SSL * ssl = NULL;
		const SSL_CIPHER * c;
		STACK_OF(SSL_CIPHER) *sk = NULL;
		const char * ciphers = "ALL:eNULL", * q, * r;
		int i, id = 0, ret = 0;
		// tests for invalid input 
		const char * p = SSL_CIPHER_standard_name(NULL);
		if(!TEST_str_eq(p, "(NONE)")) {
			TEST_info("test_cipher_name(std) failed: NULL input doesn't return \"(NONE)\"\n");
			goto err;
		}
		p = OPENSSL_cipher_name(NULL);
		if(!TEST_str_eq(p, "(NONE)")) {
			TEST_info("test_cipher_name(ossl) failed: NULL input doesn't return \"(NONE)\"\n");
			goto err;
		}
		p = OPENSSL_cipher_name("This is not a valid cipher");
		if(!TEST_str_eq(p, "(NONE)")) {
			TEST_info("test_cipher_name(ossl) failed: invalid input doesn't return \"(NONE)\"\n");
			goto err;
		}
		/* tests for valid input */
		ctx = SSL_CTX_new(TLS_server_method());
		if(!ctx) {
			TEST_info("test_cipher_name failed: internal error\n");
			goto err;
		}
		if(!SSL_CTX_set_cipher_list(ctx, ciphers)) {
			TEST_info("test_cipher_name failed: internal error\n");
			goto err;
		}
		ssl = SSL_new(ctx);
		if(ssl == NULL) {
			TEST_info("test_cipher_name failed: internal error\n");
			goto err;
		}
		sk = SSL_get_ciphers(ssl);
		if(!sk) {
			TEST_info("test_cipher_name failed: internal error\n");
			goto err;
		}
		for(i = 0; i < sk_SSL_CIPHER_num(sk); i++) {
			c = sk_SSL_CIPHER_value(sk, i);
			id = SSL_CIPHER_get_id(c) & 0xFFFF;
			if((id == 0xC102) || (id == 0xFF85) ||(id == 0xFF87))
				continue; // skip GOST2012-GOST8912-GOST891 and GOST2012-NULL-GOST12 
			p = SSL_CIPHER_standard_name(c);
			q = get_std_name_by_id(id);
			if(!TEST_ptr(p)) {
				TEST_info("test_cipher_name failed: expected %s, got NULL, cipher %x\n", q, id);
				goto err;
			}
			/* check if p is a valid standard name */
			if(!TEST_str_eq(p, q)) {
				TEST_info("test_cipher_name(std) failed: expected %s, got %s, cipher %x\n", q, p, id);
				goto err;
			}
			/* test OPENSSL_cipher_name */
			q = SSL_CIPHER_get_name(c);
			r = OPENSSL_cipher_name(p);
			if(!TEST_str_eq(r, q)) {
				TEST_info("test_cipher_name(ossl) failed: expected %s, got %s, cipher %x\n", q, r, id);
				goto err;
			}
		}
		ret = 1;
	err:
		SSL_CTX_free(ctx);
		SSL_free(ssl);
		return ret;
	}
};
//
//
//
static const uint CONSTTIME_TRUE = (uint)(~0);
static const uint CONSTTIME_FALSE = 0;
static const uchar CONSTTIME_TRUE_8 = 0xff;
static const uchar CONSTTIME_FALSE_8 = 0;
static const size_t CONSTTIME_TRUE_S = ~((size_t)0);
static const size_t CONSTTIME_FALSE_S = 0;
static uint32_t CONSTTIME_TRUE_32 = (uint32_t)(~(uint32_t)0);
static uint32_t CONSTTIME_FALSE_32 = 0;
static uint64_t CONSTTIME_TRUE_64 = (uint64_t)(~(uint64_t)0);
static uint64_t CONSTTIME_FALSE_64 = 0;

static uint TestData_ConstantTime_test_values[] = {
	0, 1, 1024, 12345, 32000, UINT_MAX / 2 - 1,
	UINT_MAX / 2, UINT_MAX / 2 + 1, UINT_MAX - 1,
	UINT_MAX
};

static uchar TestData_ConstantTime_test_values_8[] = {
	0, 1, 2, 20, 32, 127, 128, 129, 255
};

static int TestData_ConstantTime_signed_test_values[] = {
	0, 1, -1, 1024, -1024, 12345, -12345,
	32000, -32000, INT_MAX, INT_MIN, INT_MAX - 1,
	INT_MIN + 1
};

static size_t TestData_ConstantTime_test_values_s[] = {
	0, 1, 1024, 12345, 32000, SIZE_MAX / 2 - 1,
	SIZE_MAX / 2, SIZE_MAX / 2 + 1, SIZE_MAX - 1,
	SIZE_MAX
};

static uint32_t TestData_ConstantTime_test_values_32[] = {
	0, 1, 1024, 12345, 32000, UINT32_MAX / 2, UINT32_MAX / 2 + 1,
	UINT32_MAX - 1, UINT32_MAX
};

static uint64_t TestData_ConstantTime_test_values_64[] = {
	0, 1, 1024, 12345, 32000, 32000000, 32000000001, UINT64_MAX / 2,
	UINT64_MAX / 2 + 1, UINT64_MAX - 1, UINT64_MAX
};

class TestInnerBlock_ConstantTime {
public:
	static int test_binary_op(uint (*op)(uint a, uint b), const char * op_name, uint a, uint b, int is_true)
	{
		if(is_true && !TEST_uint_eq(op(a, b), CONSTTIME_TRUE))
			return 0;
		if(!is_true && !TEST_uint_eq(op(a, b), CONSTTIME_FALSE))
			return 0;
		return 1;
	}
	static int test_binary_op_8(uchar (*op)(uint a, uint b), const char * op_name, uint a, uint b, int is_true)
	{
		if(is_true && !TEST_uint_eq(op(a, b), CONSTTIME_TRUE_8))
			return 0;
		if(!is_true && !TEST_uint_eq(op(a, b), CONSTTIME_FALSE_8))
			return 0;
		return 1;
	}
	static int test_binary_op_s(size_t (*op)(size_t a, size_t b), const char * op_name, size_t a, size_t b, int is_true)
	{
		if(is_true && !TEST_size_t_eq(op(a, b), CONSTTIME_TRUE_S))
			return 0;
		if(!is_true && !TEST_uint_eq(op(a, b), CONSTTIME_FALSE_S))
			return 0;
		return 1;
	}
	static int test_binary_op_64(uint64_t (*op)(uint64_t a, uint64_t b), const char * op_name, uint64_t a, uint64_t b, int is_true)
	{
		uint64_t c = op(a, b);
		if(is_true && c != CONSTTIME_TRUE_64) {
			TEST_error("TRUE %s op failed", op_name);
			BIO_printf(bio_err, "a=%jx b=%jx\n", a, b);
			return 0;
		}
		else if(!is_true && c != CONSTTIME_FALSE_64) {
			TEST_error("FALSE %s op failed", op_name);
			BIO_printf(bio_err, "a=%jx b=%jx\n", a, b);
			return 0;
		}
		return 1;
	}
	static int test_is_zero(int i)
	{
		uint a = TestData_ConstantTime_test_values[i];
		if(a == 0 && !TEST_uint_eq(constant_time_is_zero(a), CONSTTIME_TRUE))
			return 0;
		if(a != 0 && !TEST_uint_eq(constant_time_is_zero(a), CONSTTIME_FALSE))
			return 0;
		return 1;
	}
	static int test_is_zero_8(int i)
	{
		uint a = TestData_ConstantTime_test_values_8[i];
		if(a == 0 && !TEST_uint_eq(constant_time_is_zero_8(a), CONSTTIME_TRUE_8))
			return 0;
		if(a != 0 && !TEST_uint_eq(constant_time_is_zero_8(a), CONSTTIME_FALSE_8))
			return 0;
		return 1;
	}
	static int test_is_zero_32(int i)
	{
		uint32_t a = TestData_ConstantTime_test_values_32[i];
		if(a == 0 && !TEST_true(constant_time_is_zero_32(a) == CONSTTIME_TRUE_32))
			return 0;
		if(a != 0 && !TEST_true(constant_time_is_zero_32(a) == CONSTTIME_FALSE_32))
			return 0;
		return 1;
	}
	static int test_is_zero_s(int i)
	{
		size_t a = TestData_ConstantTime_test_values_s[i];
		if(a == 0 && !TEST_size_t_eq(constant_time_is_zero_s(a), CONSTTIME_TRUE_S))
			return 0;
		if(a != 0 && !TEST_uint_eq(constant_time_is_zero_s(a), CONSTTIME_FALSE_S))
			return 0;
		return 1;
	}
	static int test_select(uint a, uint b)
	{
		if(!TEST_uint_eq(constant_time_select(CONSTTIME_TRUE, a, b), a))
			return 0;
		if(!TEST_uint_eq(constant_time_select(CONSTTIME_FALSE, a, b), b))
			return 0;
		return 1;
	}
	static int test_select_8(uchar a, uchar b)
	{
		if(!TEST_uint_eq(constant_time_select_8(CONSTTIME_TRUE_8, a, b), a))
			return 0;
		if(!TEST_uint_eq(constant_time_select_8(CONSTTIME_FALSE_8, a, b), b))
			return 0;
		return 1;
	}
	static int test_select_32(uint32_t a, uint32_t b)
	{
		if(!TEST_true(constant_time_select_32(CONSTTIME_TRUE_32, a, b) == a))
			return 0;
		if(!TEST_true(constant_time_select_32(CONSTTIME_FALSE_32, a, b) == b))
			return 0;
		return 1;
	}
	static int test_select_s(size_t a, size_t b)
	{
		if(!TEST_uint_eq(constant_time_select_s(CONSTTIME_TRUE_S, a, b), a))
			return 0;
		if(!TEST_uint_eq(constant_time_select_s(CONSTTIME_FALSE_S, a, b), b))
			return 0;
		return 1;
	}
	static int test_select_64(uint64_t a, uint64_t b)
	{
		uint64_t selected = constant_time_select_64(CONSTTIME_TRUE_64, a, b);
		if(selected != a) {
			TEST_error("test_select_64 TRUE failed");
			BIO_printf(bio_err, "a=%jx b=%jx got %jx wanted a\n", a, b, selected);
			return 0;
		}
		selected = constant_time_select_64(CONSTTIME_FALSE_64, a, b);
		if(selected != b) {
			BIO_printf(bio_err, "a=%jx b=%jx got %jx wanted b\n", a, b, selected);
			return 0;
		}
		return 1;
	}
	static int test_select_int(int a, int b)
	{
		if(!TEST_int_eq(constant_time_select_int(CONSTTIME_TRUE, a, b), a))
			return 0;
		if(!TEST_int_eq(constant_time_select_int(CONSTTIME_FALSE, a, b), b))
			return 0;
		return 1;
	}
	static int test_eq_int_8(int a, int b)
	{
		if(a == b && !TEST_int_eq(constant_time_eq_int_8(a, b), CONSTTIME_TRUE_8))
			return 0;
		if(a != b && !TEST_int_eq(constant_time_eq_int_8(a, b), CONSTTIME_FALSE_8))
			return 0;
		return 1;
	}
	static int test_eq_s(size_t a, size_t b)
	{
		if(a == b && !TEST_size_t_eq(constant_time_eq_s(a, b), CONSTTIME_TRUE_S))
			return 0;
		if(a != b && !TEST_int_eq(constant_time_eq_s(a, b), CONSTTIME_FALSE_S))
			return 0;
		return 1;
	}
	static int test_eq_int(int a, int b)
	{
		if(a == b && !TEST_uint_eq(constant_time_eq_int(a, b), CONSTTIME_TRUE))
			return 0;
		if(a != b && !TEST_uint_eq(constant_time_eq_int(a, b), CONSTTIME_FALSE))
			return 0;
		return 1;
	}
	static int test_sizeofs()
	{
		if(!TEST_uint_eq(SIZEOFARRAY(TestData_ConstantTime_test_values), SIZEOFARRAY(TestData_ConstantTime_test_values_s)))
			return 0;
		return 1;
	}
	static int test_binops(int i)
	{
		uint a = TestData_ConstantTime_test_values[i];
		int ret = 1;
		for(int j = 0; j < (int)SIZEOFARRAY(TestData_ConstantTime_test_values); ++j) {
			uint b = TestData_ConstantTime_test_values[j];
			if(!test_select(a, b) || !test_binary_op(&constant_time_lt, "ct_lt", a, b, a < b)
				|| !test_binary_op(&constant_time_lt, "constant_time_lt", b, a, b < a)
				|| !test_binary_op(&constant_time_ge, "constant_time_ge", a, b, a >= b)
				|| !test_binary_op(&constant_time_ge, "constant_time_ge", b, a, b >= a)
				|| !test_binary_op(&constant_time_eq, "constant_time_eq", a, b, a == b)
				|| !test_binary_op(&constant_time_eq, "constant_time_eq", b, a, b == a))
				ret = 0;
		}
		return ret;
	}
	static int test_binops_8(int i)
	{
		uint a = TestData_ConstantTime_test_values_8[i];
		int ret = 1;
		for(int j = 0; j < (int)SIZEOFARRAY(TestData_ConstantTime_test_values_8); ++j) {
			uint b = TestData_ConstantTime_test_values_8[j];
			if(!test_binary_op_8(&constant_time_lt_8, "constant_time_lt_8", a, b, a < b)
				|| !test_binary_op_8(&constant_time_lt_8, "constant_time_lt_8", b, a, b < a)
				|| !test_binary_op_8(&constant_time_ge_8, "constant_time_ge_8", a, b, a >= b)
				|| !test_binary_op_8(&constant_time_ge_8, "constant_time_ge_8", b, a, b >= a)
				|| !test_binary_op_8(&constant_time_eq_8, "constant_time_eq_8", a, b, a == b)
				|| !test_binary_op_8(&constant_time_eq_8, "constant_time_eq_8", b, a, b == a))
				ret = 0;
		}
		return ret;
	}
	static int test_binops_s(int i)
	{
		size_t a = TestData_ConstantTime_test_values_s[i];
		int ret = 1;
		for(int j = 0; j < (int)SIZEOFARRAY(TestData_ConstantTime_test_values_s); ++j) {
			size_t b = TestData_ConstantTime_test_values_s[j];
			if(!test_select_s(a, b) || !test_eq_s(a, b)
				|| !test_binary_op_s(&constant_time_lt_s, "constant_time_lt_s", a, b, a < b)
				|| !test_binary_op_s(&constant_time_lt_s, "constant_time_lt_s", b, a, b < a)
				|| !test_binary_op_s(&constant_time_ge_s, "constant_time_ge_s", a, b, a >= b)
				|| !test_binary_op_s(&constant_time_ge_s, "constant_time_ge_s", b, a, b >= a)
				|| !test_binary_op_s(&constant_time_eq_s, "constant_time_eq_s", a, b, a == b)
				|| !test_binary_op_s(&constant_time_eq_s, "constant_time_eq_s", b, a, b == a))
				ret = 0;
		}
		return ret;
	}
	static int test_signed(int i)
	{
		int c = TestData_ConstantTime_signed_test_values[i];
		int ret = 1;
		for(uint j = 0; j < SIZEOFARRAY(TestData_ConstantTime_signed_test_values); ++j) {
			int d = TestData_ConstantTime_signed_test_values[j];
			if(!test_select_int(c, d) || !test_eq_int(c, d) || !test_eq_int_8(c, d))
				ret = 0;
		}
		return ret;
	}
	static int test_8values(int i)
	{
		uchar e = TestData_ConstantTime_test_values_8[i];
		int ret = 1;
		for(uint j = 0; j < sizeof(TestData_ConstantTime_test_values_8); ++j) {
			uchar f = TestData_ConstantTime_test_values_8[j];
			if(!test_select_8(e, f))
				ret = 0;
		}
		return ret;
	}
	static int test_32values(int i)
	{
		uint32_t e = TestData_ConstantTime_test_values_32[i];
		int ret = 1;
		for(size_t j = 0; j < SIZEOFARRAY(TestData_ConstantTime_test_values_32); j++) {
			uint32_t f = TestData_ConstantTime_test_values_32[j];
			if(!test_select_32(e, f))
				ret = 0;
		}
		return ret;
	}
	static int test_64values(int i)
	{
		uint64_t g = TestData_ConstantTime_test_values_64[i];
		int ret = 1;
		for(int j = i + 1; j < (int)SIZEOFARRAY(TestData_ConstantTime_test_values_64); j++) {
			uint64_t h = TestData_ConstantTime_test_values_64[j];
			if(!test_binary_op_64(&constant_time_lt_64, "constant_time_lt_64", g, h, g < h) || !test_select_64(g, h)) {
				TEST_info("test_64values failed i=%d j=%d", i, j);
				ret = 0;
			}
		}
		return ret;
	}
};
//
//
//
static BN_CTX * TestData_BnInternal_ctx;
static int TestData_BnInternal_composites[] = { 9, 21, 77, 81, 265 };

class TestInnerBlock_BnInternal {
public:
	static int test_is_prime_enhanced()
	{
		int status = 0;
		BIGNUM * bn = NULL;
		int ret = TEST_ptr(bn = BN_new())
			/* test passing a prime returns the correct status */
			&& TEST_true(BN_set_word(bn, 11))
			/* return extra parameters related to composite */
			&& TEST_true(ossl_bn_miller_rabin_is_prime(bn, 10, TestData_BnInternal_ctx, NULL, 1, &status))
			&& TEST_int_eq(status, BN_PRIMETEST_PROBABLY_PRIME);
		BN_free(bn);
		return ret;
	}
	static int test_is_composite_enhanced(int id)
	{
		int status = 0;
		BIGNUM * bn = NULL;
		int ret = TEST_ptr(bn = BN_new())
			/* negative tests for different composite numbers */
			&& TEST_true(BN_set_word(bn, TestData_BnInternal_composites[id]))
			&& TEST_true(ossl_bn_miller_rabin_is_prime(bn, 10, TestData_BnInternal_ctx, NULL, 1, &status))
			&& TEST_int_ne(status, BN_PRIMETEST_PROBABLY_PRIME);
		BN_free(bn);
		return ret;
	}
	// Test that multiplying all the small primes from 3 to 751 equals a constant.
	// This test is mainly used to test that both 32 and 64 bit are correct.
	static int test_bn_small_factors()
	{
		int ret = 0;
		size_t i;
		BIGNUM * b = NULL;
		size_t prime_tab_count = 0;
		const ushort * p_prime_tab = GetPrimeTab(&prime_tab_count);
		if(!TEST_ptr(p_prime_tab)) // @sobolev
			goto err;
		if(!(TEST_ptr(b = BN_new()) && TEST_true(BN_set_word(b, 3))))
			goto err;
		for(i = 1; i < /*NUMPRIMES*/prime_tab_count; i++) {
			prime_t p = /*primes*/p_prime_tab[i];
			if(p > 3 && p <= 751 && !BN_mul_word(b, p))
				goto err;
			if(p > 751)
				break;
		}
		ret = TEST_BN_eq(ossl_bn_get0_small_factors(), b);
	err:
		BN_free(b);
		return ret;
	}
};
//
//
//
static SSL_CTX * TestData_CipherBytes_ctx;
static SSL * TestData_CipherBytes_s;

class TestInnerBlock_CipherBytes {
public:
	static int test_empty()
	{
		STACK_OF(SSL_CIPHER) * sk = NULL;
		STACK_OF(SSL_CIPHER) * scsv = NULL;
		const uchar bytes[] = {0x00};
		int ret = 0;
		if(!TEST_int_eq(SSL_bytes_to_cipher_list(TestData_CipherBytes_s, bytes, 0, 0, &sk, &scsv), 0) || !TEST_ptr_null(sk) || !TEST_ptr_null(scsv))
			goto err;
		ret = 1;
	err:
		sk_SSL_CIPHER_free(sk);
		sk_SSL_CIPHER_free(scsv);
		return ret;
	}
	static int test_unsupported()
	{
		STACK_OF(SSL_CIPHER) *sk, *scsv;
		// ECDH-RSA-AES256 (unsupported), ECDHE-ECDSA-AES128, <unassigned> 
		const uchar bytes[] = {0xc0, 0x0f, 0x00, 0x2f, 0x01, 0x00};
		int ret = 0;
		if(!TEST_true(SSL_bytes_to_cipher_list(TestData_CipherBytes_s, bytes, sizeof(bytes), 0, &sk, &scsv))
			|| !TEST_ptr(sk)
			|| !TEST_int_eq(sk_SSL_CIPHER_num(sk), 1)
			|| !TEST_ptr(scsv)
			|| !TEST_int_eq(sk_SSL_CIPHER_num(scsv), 0)
			|| !TEST_str_eq(SSL_CIPHER_get_name(sk_SSL_CIPHER_value(sk, 0)), "AES128-SHA"))
			goto err;
		ret = 1;
	err:
		sk_SSL_CIPHER_free(sk);
		sk_SSL_CIPHER_free(scsv);
		return ret;
	}
	static int test_v2()
	{
		STACK_OF(SSL_CIPHER) *sk, *scsv;
		// ECDHE-ECDSA-AES256GCM, SSL2_RC4_1238_WITH_MD5, ECDHE-ECDSA-CHACHA20-POLY1305
		const uchar bytes[] = {0x00, 0x00, 0x35, 0x01, 0x00, 0x80, 0x00, 0x00, 0x33};
		int ret = 0;
		if(!TEST_true(SSL_bytes_to_cipher_list(TestData_CipherBytes_s, bytes, sizeof(bytes), 1, &sk, &scsv))
			|| !TEST_ptr(sk)
			|| !TEST_int_eq(sk_SSL_CIPHER_num(sk), 2)
			|| !TEST_ptr(scsv)
			|| !TEST_int_eq(sk_SSL_CIPHER_num(scsv), 0))
			goto err;
		if(strcmp(SSL_CIPHER_get_name(sk_SSL_CIPHER_value(sk, 0)), "AES256-SHA") != 0 ||
			strcmp(SSL_CIPHER_get_name(sk_SSL_CIPHER_value(sk, 1)), "DHE-RSA-AES128-SHA") != 0)
			goto err;
		ret = 1;
	err:
		sk_SSL_CIPHER_free(sk);
		sk_SSL_CIPHER_free(scsv);
		return ret;
	}
	static int test_v3()
	{
		STACK_OF(SSL_CIPHER) *sk = NULL, *scsv = NULL;
		// ECDHE-ECDSA-AES256GCM, ECDHE-ECDSA-CHACHAPOLY, DHE-RSA-AES256GCM, EMPTY-RENEGOTIATION-INFO-SCSV, FALLBACK-SCSV */
		const uchar bytes[] = {0x00, 0x2f, 0x00, 0x33, 0x00, 0x9f, 0x00, 0xff, 0x56, 0x00};
		int ret = 0;
		if(!SSL_bytes_to_cipher_list(TestData_CipherBytes_s, bytes, sizeof(bytes), 0, &sk, &scsv)
			|| !TEST_ptr(sk)
			|| !TEST_int_eq(sk_SSL_CIPHER_num(sk), 3)
			|| !TEST_ptr(scsv)
			|| !TEST_int_eq(sk_SSL_CIPHER_num(scsv), 2)
			|| !TEST_str_eq(SSL_CIPHER_get_name(sk_SSL_CIPHER_value(sk, 0)), "AES128-SHA")
			|| !TEST_str_eq(SSL_CIPHER_get_name(sk_SSL_CIPHER_value(sk, 1)), "DHE-RSA-AES128-SHA")
			|| !TEST_str_eq(SSL_CIPHER_get_name(sk_SSL_CIPHER_value(sk, 2)), "DHE-RSA-AES256-GCM-SHA384")
			|| !TEST_str_eq(SSL_CIPHER_get_name(sk_SSL_CIPHER_value(scsv, 0)), "TLS_EMPTY_RENEGOTIATION_INFO_SCSV")
			|| !TEST_str_eq(SSL_CIPHER_get_name(sk_SSL_CIPHER_value(scsv, 1)), "TLS_FALLBACK_SCSV"))
			goto err;
		ret = 1;
	err:
		sk_SSL_CIPHER_free(sk);
		sk_SSL_CIPHER_free(scsv);
		return ret;
	}
};
// 
// DES low level APIs are deprecated for public use, but still ok for internal use.
// 
#ifndef OPENSSL_NO_DES
#define TestData_DES_TEST_cs_eq  TEST_uint_eq // In case any platform doesn't use unsigned int for its checksums 
#define TestData_DES_DATA_BUF_SIZE      20

// tisk tisk - the test keys don't all have odd parity :-( 
// test data 
#define TestData_DES_NUM_TESTS 34
static uchar TestData_DES_key_data[TestData_DES_NUM_TESTS][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10},
	{0x7C, 0xA1, 0x10, 0x45, 0x4A, 0x1A, 0x6E, 0x57},
	{0x01, 0x31, 0xD9, 0x61, 0x9D, 0xC1, 0x37, 0x6E},
	{0x07, 0xA1, 0x13, 0x3E, 0x4A, 0x0B, 0x26, 0x86},
	{0x38, 0x49, 0x67, 0x4C, 0x26, 0x02, 0x31, 0x9E},
	{0x04, 0xB9, 0x15, 0xBA, 0x43, 0xFE, 0xB5, 0xB6},
	{0x01, 0x13, 0xB9, 0x70, 0xFD, 0x34, 0xF2, 0xCE},
	{0x01, 0x70, 0xF1, 0x75, 0x46, 0x8F, 0xB5, 0xE6},
	{0x43, 0x29, 0x7F, 0xAD, 0x38, 0xE3, 0x73, 0xFE},
	{0x07, 0xA7, 0x13, 0x70, 0x45, 0xDA, 0x2A, 0x16},
	{0x04, 0x68, 0x91, 0x04, 0xC2, 0xFD, 0x3B, 0x2F},
	{0x37, 0xD0, 0x6B, 0xB5, 0x16, 0xCB, 0x75, 0x46},
	{0x1F, 0x08, 0x26, 0x0D, 0x1A, 0xC2, 0x46, 0x5E},
	{0x58, 0x40, 0x23, 0x64, 0x1A, 0xBA, 0x61, 0x76},
	{0x02, 0x58, 0x16, 0x16, 0x46, 0x29, 0xB0, 0x07},
	{0x49, 0x79, 0x3E, 0xBC, 0x79, 0xB3, 0x25, 0x8F},
	{0x4F, 0xB0, 0x5E, 0x15, 0x15, 0xAB, 0x73, 0xA7},
	{0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBF},
	{0x01, 0x83, 0x10, 0xDC, 0x40, 0x9B, 0x26, 0xD6},
	{0x1C, 0x58, 0x7F, 0x1C, 0x13, 0x92, 0x4F, 0xEF},
	{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
	{0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E},
	{0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1, 0xFE},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10}
};

static uchar TestData_DES_plain_data[TestData_DES_NUM_TESTS][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0x01, 0xA1, 0xD6, 0xD0, 0x39, 0x77, 0x67, 0x42},
	{0x5C, 0xD5, 0x4C, 0xA8, 0x3D, 0xEF, 0x57, 0xDA},
	{0x02, 0x48, 0xD4, 0x38, 0x06, 0xF6, 0x71, 0x72},
	{0x51, 0x45, 0x4B, 0x58, 0x2D, 0xDF, 0x44, 0x0A},
	{0x42, 0xFD, 0x44, 0x30, 0x59, 0x57, 0x7F, 0xA2},
	{0x05, 0x9B, 0x5E, 0x08, 0x51, 0xCF, 0x14, 0x3A},
	{0x07, 0x56, 0xD8, 0xE0, 0x77, 0x47, 0x61, 0xD2},
	{0x76, 0x25, 0x14, 0xB8, 0x29, 0xBF, 0x48, 0x6A},
	{0x3B, 0xDD, 0x11, 0x90, 0x49, 0x37, 0x28, 0x02},
	{0x26, 0x95, 0x5F, 0x68, 0x35, 0xAF, 0x60, 0x9A},
	{0x16, 0x4D, 0x5E, 0x40, 0x4F, 0x27, 0x52, 0x32},
	{0x6B, 0x05, 0x6E, 0x18, 0x75, 0x9F, 0x5C, 0xCA},
	{0x00, 0x4B, 0xD6, 0xEF, 0x09, 0x17, 0x60, 0x62},
	{0x48, 0x0D, 0x39, 0x00, 0x6E, 0xE7, 0x62, 0xF2},
	{0x43, 0x75, 0x40, 0xC8, 0x69, 0x8F, 0x3C, 0xFA},
	{0x07, 0x2D, 0x43, 0xA0, 0x77, 0x07, 0x52, 0x92},
	{0x02, 0xFE, 0x55, 0x77, 0x81, 0x17, 0xF1, 0x2A},
	{0x1D, 0x9D, 0x5C, 0x50, 0x18, 0xF7, 0x28, 0xC2},
	{0x30, 0x55, 0x32, 0x28, 0x6D, 0x6F, 0x29, 0x5A},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

static uchar TestData_DES_cipher_data[TestData_DES_NUM_TESTS][8] = {
	{0x8C, 0xA6, 0x4D, 0xE9, 0xC1, 0xB1, 0x23, 0xA7},
	{0x73, 0x59, 0xB2, 0x16, 0x3E, 0x4E, 0xDC, 0x58},
	{0x95, 0x8E, 0x6E, 0x62, 0x7A, 0x05, 0x55, 0x7B},
	{0xF4, 0x03, 0x79, 0xAB, 0x9E, 0x0E, 0xC5, 0x33},
	{0x17, 0x66, 0x8D, 0xFC, 0x72, 0x92, 0x53, 0x2D},
	{0x8A, 0x5A, 0xE1, 0xF8, 0x1A, 0xB8, 0xF2, 0xDD},
	{0x8C, 0xA6, 0x4D, 0xE9, 0xC1, 0xB1, 0x23, 0xA7},
	{0xED, 0x39, 0xD9, 0x50, 0xFA, 0x74, 0xBC, 0xC4},
	{0x69, 0x0F, 0x5B, 0x0D, 0x9A, 0x26, 0x93, 0x9B},
	{0x7A, 0x38, 0x9D, 0x10, 0x35, 0x4B, 0xD2, 0x71},
	{0x86, 0x8E, 0xBB, 0x51, 0xCA, 0xB4, 0x59, 0x9A},
	{0x71, 0x78, 0x87, 0x6E, 0x01, 0xF1, 0x9B, 0x2A},
	{0xAF, 0x37, 0xFB, 0x42, 0x1F, 0x8C, 0x40, 0x95},
	{0x86, 0xA5, 0x60, 0xF1, 0x0E, 0xC6, 0xD8, 0x5B},
	{0x0C, 0xD3, 0xDA, 0x02, 0x00, 0x21, 0xDC, 0x09},
	{0xEA, 0x67, 0x6B, 0x2C, 0xB7, 0xDB, 0x2B, 0x7A},
	{0xDF, 0xD6, 0x4A, 0x81, 0x5C, 0xAF, 0x1A, 0x0F},
	{0x5C, 0x51, 0x3C, 0x9C, 0x48, 0x86, 0xC0, 0x88},
	{0x0A, 0x2A, 0xEE, 0xAE, 0x3F, 0xF4, 0xAB, 0x77},
	{0xEF, 0x1B, 0xF0, 0x3E, 0x5D, 0xFA, 0x57, 0x5A},
	{0x88, 0xBF, 0x0D, 0xB6, 0xD7, 0x0D, 0xEE, 0x56},
	{0xA1, 0xF9, 0x91, 0x55, 0x41, 0x02, 0x0B, 0x56},
	{0x6F, 0xBF, 0x1C, 0xAF, 0xCF, 0xFD, 0x05, 0x56},
	{0x2F, 0x22, 0xE4, 0x9B, 0xAB, 0x7C, 0xA1, 0xAC},
	{0x5A, 0x6B, 0x61, 0x2C, 0xC2, 0x6C, 0xCE, 0x4A},
	{0x5F, 0x4C, 0x03, 0x8E, 0xD1, 0x2B, 0x2E, 0x41},
	{0x63, 0xFA, 0xC0, 0xD0, 0x34, 0xD9, 0xF7, 0x93},
	{0x61, 0x7B, 0x3A, 0x0C, 0xE8, 0xF0, 0x71, 0x00},
	{0xDB, 0x95, 0x86, 0x05, 0xF8, 0xC8, 0xC6, 0x06},
	{0xED, 0xBF, 0xD1, 0xC6, 0x6C, 0x29, 0xCC, 0xC7},
	{0x35, 0x55, 0x50, 0xB2, 0x15, 0x0E, 0x24, 0x51},
	{0xCA, 0xAA, 0xAF, 0x4D, 0xEA, 0xF1, 0xDB, 0xAE},
	{0xD5, 0xD4, 0x4F, 0xF7, 0x20, 0x68, 0x3D, 0x0D},
	{0x2A, 0x2B, 0xB0, 0x08, 0xDF, 0x97, 0xC2, 0xF2}
};

static uchar TestData_DES_cipher_ecb2[TestData_DES_NUM_TESTS - 1][8] = {
	{0x92, 0x95, 0xB5, 0x9B, 0xB3, 0x84, 0x73, 0x6E},
	{0x19, 0x9E, 0x9D, 0x6D, 0xF3, 0x9A, 0xA8, 0x16},
	{0x2A, 0x4B, 0x4D, 0x24, 0x52, 0x43, 0x84, 0x27},
	{0x35, 0x84, 0x3C, 0x01, 0x9D, 0x18, 0xC5, 0xB6},
	{0x4A, 0x5B, 0x2F, 0x42, 0xAA, 0x77, 0x19, 0x25},
	{0xA0, 0x6B, 0xA9, 0xB8, 0xCA, 0x5B, 0x17, 0x8A},
	{0xAB, 0x9D, 0xB7, 0xFB, 0xED, 0x95, 0xF2, 0x74},
	{0x3D, 0x25, 0x6C, 0x23, 0xA7, 0x25, 0x2F, 0xD6},
	{0xB7, 0x6F, 0xAB, 0x4F, 0xBD, 0xBD, 0xB7, 0x67},
	{0x8F, 0x68, 0x27, 0xD6, 0x9C, 0xF4, 0x1A, 0x10},
	{0x82, 0x57, 0xA1, 0xD6, 0x50, 0x5E, 0x81, 0x85},
	{0xA2, 0x0F, 0x0A, 0xCD, 0x80, 0x89, 0x7D, 0xFA},
	{0xCD, 0x2A, 0x53, 0x3A, 0xDB, 0x0D, 0x7E, 0xF3},
	{0xD2, 0xC2, 0xBE, 0x27, 0xE8, 0x1B, 0x68, 0xE3},
	{0xE9, 0x24, 0xCF, 0x4F, 0x89, 0x3C, 0x5B, 0x0A},
	{0xA7, 0x18, 0xC3, 0x9F, 0xFA, 0x9F, 0xD7, 0x69},
	{0x77, 0x2C, 0x79, 0xB1, 0xD2, 0x31, 0x7E, 0xB1},
	{0x49, 0xAB, 0x92, 0x7F, 0xD0, 0x22, 0x00, 0xB7},
	{0xCE, 0x1C, 0x6C, 0x7D, 0x85, 0xE3, 0x4A, 0x6F},
	{0xBE, 0x91, 0xD6, 0xE1, 0x27, 0xB2, 0xE9, 0x87},
	{0x70, 0x28, 0xAE, 0x8F, 0xD1, 0xF5, 0x74, 0x1A},
	{0xAA, 0x37, 0x80, 0xBB, 0xF3, 0x22, 0x1D, 0xDE},
	{0xA6, 0xC4, 0xD2, 0x5E, 0x28, 0x93, 0xAC, 0xB3},
	{0x22, 0x07, 0x81, 0x5A, 0xE4, 0xB7, 0x1A, 0xAD},
	{0xDC, 0xCE, 0x05, 0xE7, 0x07, 0xBD, 0xF5, 0x84},
	{0x26, 0x1D, 0x39, 0x2C, 0xB3, 0xBA, 0xA5, 0x85},
	{0xB4, 0xF7, 0x0F, 0x72, 0xFB, 0x04, 0xF0, 0xDC},
	{0x95, 0xBA, 0xA9, 0x4E, 0x87, 0x36, 0xF2, 0x89},
	{0xD4, 0x07, 0x3A, 0xF1, 0x5A, 0x17, 0x82, 0x0E},
	{0xEF, 0x6F, 0xAF, 0xA7, 0x66, 0x1A, 0x7E, 0x89},
	{0xC1, 0x97, 0xF5, 0x58, 0x74, 0x8A, 0x20, 0xE7},
	{0x43, 0x34, 0xCF, 0xDA, 0x22, 0xC4, 0x86, 0xC8},
	{0x08, 0xD7, 0xB4, 0xFB, 0x62, 0x9D, 0x08, 0x85}
};

static uchar TestData_DES_cbc_key[8] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
static uchar TestData_DES_cbc2_key[8] = { 0xf1, 0xe0, 0xd3, 0xc2, 0xb5, 0xa4, 0x97, 0x86 };
static uchar TestData_DES_cbc3_key[8] = { 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 };
static uchar TestData_DES_cbc_iv[8] = { 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 };
// 
// Changed the following text constant to binary so it will work on ebcdic machines :-)
// 
// static char cbc_data[40]="7654321 Now is the time for \0001"; */
static uchar TestData_DES_cbc_data[40] = { 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x20, 0x4E, 0x6F, 0x77, 0x20, 0x69, 0x73, 0x20, 0x74,
	0x68, 0x65, 0x20, 0x74, 0x69, 0x6D, 0x65, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
static uchar cbc_ok[32] = { 0xcc, 0xd1, 0x73, 0xff, 0xab, 0x20, 0x39, 0xf4, 0xac, 0xd8, 0xae, 0xfd, 0xdf, 0xd8, 0xa1, 0xeb,
	0x46, 0x8e, 0x91, 0x15, 0x78, 0x88, 0xba, 0x68, 0x1d, 0x26, 0x93, 0x97, 0xf7, 0xfe, 0x62, 0xb4 };

#ifdef SCREW_THE_PARITY
#error "SCREW_THE_PARITY is not meant to be defined."
#error "Original vectors are preserved for reference only."
static uchar TestData_DES_cbc2_key[8] = { 0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87 };
static uchar TestData_DES_xcbc_ok[32] = { 0x86, 0x74, 0x81, 0x0D, 0x61, 0xA4, 0xA5, 0x48, 0xB9, 0x93, 0x03, 0xE1, 0xB8, 0xBB, 0xBD, 0xBD,
	0x64, 0x30, 0x0B, 0xB9, 0x06, 0x65, 0x81, 0x76, 0x04, 0x1D, 0x77, 0x62, 0x17, 0xCA, 0x2B, 0xD2, };
#else
static uchar TestData_DES_xcbc_ok[32] = { 0x84, 0x6B, 0x29, 0x14, 0x85, 0x1E, 0x9A, 0x29, 0x54, 0x73, 0x2F, 0x8A, 0xA0, 0xA6, 0x11, 0xC1,
	0x15, 0xCD, 0xC2, 0xD7, 0x95, 0x1B, 0x10, 0x53, 0xA6, 0x3C, 0x5E, 0x03, 0xB2, 0x1A, 0xA3, 0xC4, };
#endif
static uchar TestData_DES_cbc3_ok[32] = { 0x3F, 0xE3, 0x01, 0xC9, 0x62, 0xAC, 0x01, 0xD0, 0x22, 0x13, 0x76, 0x3C, 0x1C, 0xBD, 0x4C, 0xDC, 
	0x79, 0x96, 0x57, 0xC0, 0x64, 0xEC, 0xF5, 0xD4, 0x1C, 0x67, 0x38, 0x12, 0xCF, 0xDE, 0x96, 0x75 };
static uchar TestData_DES_pcbc_ok[32] = { 0xcc, 0xd1, 0x73, 0xff, 0xab, 0x20, 0x39, 0xf4, 0x6d, 0xec, 0xb4, 0x70, 0xa0, 0xe5, 0x6b, 0x15, 
	0xae, 0xa6, 0xbf, 0x61, 0xed, 0x7d, 0x9c, 0x9f, 0xf7, 0x17, 0x46, 0x3b, 0x8a, 0xb3, 0xcc, 0x88 };
static uchar TestData_DES_cfb_key[8] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
static uchar TestData_DES_cfb_iv[8] = { 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef };
static uchar TestData_DES_cfb_buf1[40];
static uchar TestData_DES_cfb_buf2[40];
static uchar TestData_DES_cfb_tmp[8];
static uchar TestData_DES_plain[24] = { 0x4e, 0x6f, 0x77, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x74, 0x69, 0x6d, 0x65, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x61, 0x6c, 0x6c, 0x20 };
static uchar TestData_DES_cfb_cipher8[24] = { 0xf3, 0x1f, 0xda, 0x07, 0x01, 0x14, 0x62, 0xee, 0x18, 0x7f, 0x43, 0xd8, 0x0a, 0x7c, 0xd9, 0xb5, 0xb0, 0xd2, 0x90, 0xda, 0x6e, 0x5b, 0x9a, 0x87 };
static uchar TestData_DES_cfb_cipher16[24] = { 0xF3, 0x09, 0x87, 0x87, 0x7F, 0x57, 0xF7, 0x3C, 0x36, 0xB6, 0xDB, 0x70, 0xD8, 0xD5, 0x34, 0x19, 0xD3, 0x86, 0xB2, 0x23, 0xB7, 0xB2, 0xAD, 0x1B };
static uchar TestData_DES_cfb_cipher32[24] = { 0xF3, 0x09, 0x62, 0x49, 0xA4, 0xDF, 0xA4, 0x9F, 0x33, 0xDC, 0x7B, 0xAD, 0x4C, 0xC8, 0x9F, 0x64, 0xE4, 0x53, 0xE5, 0xEC, 0x67, 0x20, 0xDA, 0xB6 };
static uchar TestData_DES_cfb_cipher48[24] = { 0xF3, 0x09, 0x62, 0x49, 0xC7, 0xF4, 0x30, 0xB5, 0x15, 0xEC, 0xBB, 0x85, 0x97, 0x5A, 0x13, 0x8C, 0x68, 0x60, 0xE2, 0x38, 0x34, 0x3C, 0xDC, 0x1F };
static uchar TestData_DES_cfb_cipher64[24] = { 0xF3, 0x09, 0x62, 0x49, 0xC7, 0xF4, 0x6E, 0x51, 0xA6, 0x9E, 0x83, 0x9B, 0x1A, 0x92, 0xF7, 0x84, 0x03, 0x46, 0x71, 0x33, 0x89, 0x8E, 0xA6, 0x22 };
static uchar TestData_DES_ofb_key[8] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
static uchar TestData_DES_ofb_iv[8] = { 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef };
static uchar TestData_DES_ofb_buf1[24];
static uchar TestData_DES_ofb_buf2[24];
static uchar TestData_DES_ofb_tmp[8];
static uchar TestData_DES_ofb_cipher[24] = { 0xf3, 0x09, 0x62, 0x49, 0xc7, 0xf4, 0x6e, 0x51, 0x35, 0xf2, 0x4a, 0x24, 0x2e, 0xeb, 0x3d, 0x3f, 0x3d, 0x6d, 0x5b, 0xe3, 0x25, 0x5a, 0xf8, 0xc3 };
static DES_LONG TestData_DES_cbc_cksum_ret = 0xF7FE62B4L;
static uchar TestData_DES_cbc_cksum_data[8] = { 0x1D, 0x26, 0x93, 0x97, 0xf7, 0xfe, 0x62, 0xb4 };
// 
// Test TDES based key wrapping.
// The wrapping process uses a randomly generated IV so it is difficult to
// undertake KATs.  End to end testing is performed instead.
// 
static const int TestData_DES_test_des_key_wrap_sizes[] = { 8, 16, 24, 32, 64, 80 };
// 
// Weak and semi weak keys as taken from
// %A D.W. Davies
// %A W.L. Price
// %T Security for Computer Networks
// %I John Wiley & Sons
// %D 1984
// 
static struct {
	const DES_cblock key;
	int expect;
} TestData_DES_weak_keys[] = {
	/* weak keys */
	{{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}, 1 },
	{{0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE}, 1 },
	{{0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E}, 1 },
	{{0xE0, 0xE0, 0xE0, 0xE0, 0xF1, 0xF1, 0xF1, 0xF1}, 1 },
	/* semi-weak keys */
	{{0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE}, 1 },
	{{0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01}, 1 },
	{{0x1F, 0xE0, 0x1F, 0xE0, 0x0E, 0xF1, 0x0E, 0xF1}, 1 },
	{{0xE0, 0x1F, 0xE0, 0x1F, 0xF1, 0x0E, 0xF1, 0x0E}, 1 },
	{{0x01, 0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1}, 1 },
	{{0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1, 0x01}, 1 },
	{{0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E, 0xFE}, 1 },
	{{0xFE, 0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E}, 1 },
	{{0x01, 0x1F, 0x01, 0x1F, 0x01, 0x0E, 0x01, 0x0E}, 1 },
	{{0x1F, 0x01, 0x1F, 0x01, 0x0E, 0x01, 0x0E, 0x01}, 1 },
	{{0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1, 0xFE}, 1 },
	{{0xFE, 0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1}, 1 },
	/* good key */
	{{0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBF}, 0 }
};

static struct {
	const DES_cblock key;
	int expect;
} TestData_DES_bad_parity_keys[] = {
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0 },
	{{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 0 },
	/* Perturb each byte in turn to create even parity */
	{{0x48, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBF}, 0 },
	{{0x49, 0xE8, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBF}, 0 },
	{{0x49, 0xE9, 0x5C, 0x6D, 0x4C, 0xA2, 0x29, 0xBF}, 0 },
	{{0x49, 0xE9, 0x5D, 0x7D, 0x4C, 0xA2, 0x29, 0xBF}, 0 },
	{{0x49, 0xE9, 0x5D, 0x6D, 0x5C, 0xA2, 0x29, 0xBF}, 0 },
	{{0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA3, 0x29, 0xBF}, 0 },
	{{0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x39, 0xBF}, 0 },
	{{0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBE}, 0 },
	/* Odd parity version of above */
	{{0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBF}, 1 }
};

class TestInnerBlock_DES {
public:
	static char * pt(const uchar * p, char buf[TestData_DES_DATA_BUF_SIZE])
	{
		static const char * f = "0123456789ABCDEF";
		char * ret = &(buf[0]);
		for(int i = 0; i < 8; i++) {
			ret[i * 2] = f[(p[i] >> 4) & 0xf];
			ret[i * 2 + 1] = f[p[i] & 0xf];
		}
		ret[16] = '\0';
		return ret;
	}
	static int test_des_ecb(int i)
	{
		DES_key_schedule ks;
		DES_cblock in, out, outin;
		char b1[TestData_DES_DATA_BUF_SIZE], b2[TestData_DES_DATA_BUF_SIZE];
		DES_set_key_unchecked(&TestData_DES_key_data[i], &ks);
		memcpy(in, TestData_DES_plain_data[i], 8);
		memzero(out, 8);
		memzero(outin, 8);
		DES_ecb_encrypt(&in, &out, &ks, DES_ENCRYPT);
		DES_ecb_encrypt(&out, &outin, &ks, DES_DECRYPT);
		if(!TEST_mem_eq(out, 8, TestData_DES_cipher_data[i], 8)) {
			TEST_info("Encryption error %2d k=%s p=%s", i + 1, pt(TestData_DES_key_data[i], b1), pt(in, b2));
			return 0;
		}
		if(!TEST_mem_eq(in, 8, outin, 8)) {
			TEST_info("Decryption error %2d k=%s p=%s", i + 1, pt(TestData_DES_key_data[i], b1), pt(out, b2));
			return 0;
		}
		return 1;
	}
	static int test_des_ede_ecb(int i)
	{
		DES_cblock in, out, outin;
		DES_key_schedule ks, ks2, ks3;
		char b1[TestData_DES_DATA_BUF_SIZE], b2[TestData_DES_DATA_BUF_SIZE];
		DES_set_key_unchecked(&TestData_DES_key_data[i], &ks);
		DES_set_key_unchecked(&TestData_DES_key_data[i + 1], &ks2);
		DES_set_key_unchecked(&TestData_DES_key_data[i + 2], &ks3);
		memcpy(in, TestData_DES_plain_data[i], 8);
		memzero(out, 8);
		memzero(outin, 8);
		DES_ecb3_encrypt(&in, &out, &ks, &ks2, &ks, DES_ENCRYPT);
		DES_ecb3_encrypt(&out, &outin, &ks, &ks2, &ks, DES_DECRYPT);
		if(!TEST_mem_eq(out, 8, TestData_DES_cipher_ecb2[i], 8)) {
			TEST_info("Encryption error %2d k=%s p=%s", i + 1, pt(TestData_DES_key_data[i], b1), pt(in, b2));
			return 0;
		}
		if(!TEST_mem_eq(in, 8, outin, 8)) {
			TEST_info("Decryption error %2d k=%s p=%s ", i + 1, pt(TestData_DES_key_data[i], b1), pt(out, b2));
			return 0;
		}
		return 1;
	}
	static int test_des_cbc()
	{
		uchar cbc_in[40];
		uchar cbc_out[40];
		DES_cblock iv3;
		DES_key_schedule ks;
		const size_t cbc_data_len = strlen((char*)TestData_DES_cbc_data);
		if(!TEST_int_eq(DES_set_key_checked(&TestData_DES_cbc_key, &ks), 0))
			return 0;
		memzero(cbc_out, sizeof(cbc_out));
		memzero(cbc_in, sizeof(cbc_in));
		memcpy(iv3, TestData_DES_cbc_iv, sizeof(TestData_DES_cbc_iv));
		DES_ncbc_encrypt(TestData_DES_cbc_data, cbc_out, cbc_data_len + 1, &ks, &iv3, DES_ENCRYPT);
		if(!TEST_mem_eq(cbc_out, 32, cbc_ok, 32))
			return 0;
		memcpy(iv3, TestData_DES_cbc_iv, sizeof(TestData_DES_cbc_iv));
		DES_ncbc_encrypt(cbc_out, cbc_in, cbc_data_len + 1, &ks, &iv3, DES_DECRYPT);
		return TEST_mem_eq(cbc_in, cbc_data_len, TestData_DES_cbc_data, cbc_data_len);
	}
	static int test_des_ede_cbc()
	{
		DES_cblock iv3;
		DES_key_schedule ks;
		uchar cbc_in[40];
		uchar cbc_out[40];
		const size_t n = strlen((char*)TestData_DES_cbc_data) + 1;
		if(!TEST_int_eq(DES_set_key_checked(&TestData_DES_cbc_key, &ks), 0))
			return 0;
		memzero(cbc_out, sizeof(cbc_out));
		memzero(cbc_in, sizeof(cbc_in));
		memcpy(iv3, TestData_DES_cbc_iv, sizeof(TestData_DES_cbc_iv));
		DES_xcbc_encrypt(TestData_DES_cbc_data, cbc_out, n, &ks, &iv3, &TestData_DES_cbc2_key, &TestData_DES_cbc3_key, DES_ENCRYPT);
		if(!TEST_mem_eq(cbc_out, sizeof(TestData_DES_xcbc_ok), TestData_DES_xcbc_ok, sizeof(TestData_DES_xcbc_ok)))
			return 0;
		memcpy(iv3, TestData_DES_cbc_iv, sizeof(TestData_DES_cbc_iv));
		DES_xcbc_encrypt(cbc_out, cbc_in, n, &ks, &iv3, &TestData_DES_cbc2_key, &TestData_DES_cbc3_key, DES_DECRYPT);
		return TEST_mem_eq(TestData_DES_cbc_data, n, TestData_DES_cbc_data, n);
	}
	static int test_ede_cbc()
	{
		DES_cblock iv3;
		DES_key_schedule ks, ks2, ks3;
		uchar cbc_in[40];
		uchar cbc_out[40];
		const size_t i = strlen((char*)TestData_DES_cbc_data) + 1;
		const size_t n = (i + 7) / 8 * 8;
		if(!TEST_int_eq(DES_set_key_checked(&TestData_DES_cbc_key, &ks), 0))
			return 0;
		if(!TEST_int_eq(DES_set_key_checked(&TestData_DES_cbc2_key, &ks2), 0))
			return 0;
		if(!TEST_int_eq(DES_set_key_checked(&TestData_DES_cbc3_key, &ks3), 0))
			return 0;
		memzero(cbc_out, sizeof(cbc_out));
		memzero(cbc_in, sizeof(cbc_in));
		memcpy(iv3, TestData_DES_cbc_iv, sizeof(TestData_DES_cbc_iv));
		DES_ede3_cbc_encrypt(TestData_DES_cbc_data, cbc_out, 16L, &ks, &ks2, &ks3, &iv3, DES_ENCRYPT);
		DES_ede3_cbc_encrypt(&TestData_DES_cbc_data[16], &cbc_out[16], i - 16, &ks, &ks2, &ks3, &iv3, DES_ENCRYPT);
		if(!TEST_mem_eq(cbc_out, n, TestData_DES_cbc3_ok, n))
			return 0;
		memcpy(iv3, TestData_DES_cbc_iv, sizeof(TestData_DES_cbc_iv));
		DES_ede3_cbc_encrypt(cbc_out, cbc_in, i, &ks, &ks2, &ks3, &iv3,
			DES_DECRYPT);
		return TEST_mem_eq(cbc_in, i, TestData_DES_cbc_data, i);
	}
	static int test_input_align(int i)
	{
		uchar cbc_out[40];
		DES_cblock iv;
		DES_key_schedule ks;
		const size_t n = strlen(i + (char*)TestData_DES_cbc_data) + 1;
		memzero(cbc_out, sizeof(cbc_out));
		memcpy(iv, TestData_DES_cbc_iv, sizeof(TestData_DES_cbc_iv));
		if(!TEST_int_eq(DES_set_key_checked(&TestData_DES_cbc_key, &ks), 0))
			return 0;
		DES_ncbc_encrypt(&TestData_DES_cbc_data[i], cbc_out, n, &ks, &iv, DES_ENCRYPT);
		return 1;
	}
	static int test_output_align(int i)
	{
		uchar cbc_out[40];
		DES_cblock iv;
		DES_key_schedule ks;
		const size_t n = strlen((char*)TestData_DES_cbc_data) + 1;
		memzero(cbc_out, sizeof(cbc_out));
		memcpy(iv, TestData_DES_cbc_iv, sizeof(TestData_DES_cbc_iv));
		if(!TEST_int_eq(DES_set_key_checked(&TestData_DES_cbc_key, &ks), 0))
			return 0;
		DES_ncbc_encrypt(TestData_DES_cbc_data, &cbc_out[i], n, &ks, &iv, DES_ENCRYPT);
		return 1;
	}
	static int test_des_crypt()
	{
		if(!TEST_str_eq("efGnQx2725bI2", DES_crypt("testing", "ef")))
			return 0;
		if(!TEST_str_eq("yA1Rp/1hZXIJk", DES_crypt("bca76;23", "yA")))
			return 0;
		if(!TEST_ptr_null(DES_crypt("testing", "y\202")))
			return 0;
		if(!TEST_ptr_null(DES_crypt("testing", "\0A")))
			return 0;
		if(!TEST_ptr_null(DES_crypt("testing", "A")))
			return 0;
		return 1;
	}
	static int test_des_pcbc()
	{
		uchar cbc_in[40];
		uchar cbc_out[40];
		DES_key_schedule ks;
		const int n = strlen((char*)TestData_DES_cbc_data) + 1;
		if(!TEST_int_eq(DES_set_key_checked(&TestData_DES_cbc_key, &ks), 0))
			return 0;
		memzero(cbc_out, sizeof(cbc_out));
		memzero(cbc_in, sizeof(cbc_in));
		DES_pcbc_encrypt(TestData_DES_cbc_data, cbc_out, n, &ks, &TestData_DES_cbc_iv, DES_ENCRYPT);
		if(!TEST_mem_eq(cbc_out, sizeof(TestData_DES_pcbc_ok), TestData_DES_pcbc_ok, sizeof(TestData_DES_pcbc_ok)))
			return 0;
		DES_pcbc_encrypt(cbc_out, cbc_in, n, &ks, &TestData_DES_cbc_iv, DES_DECRYPT);
		return TEST_mem_eq(cbc_in, n, TestData_DES_cbc_data, n);
	}
	static int cfb_test(int bits, uchar * cfb_cipher)
	{
		DES_key_schedule ks;
		DES_set_key_checked(&TestData_DES_cfb_key, &ks);
		memcpy(TestData_DES_cfb_tmp, TestData_DES_cfb_iv, sizeof(TestData_DES_cfb_iv));
		DES_cfb_encrypt(TestData_DES_plain, TestData_DES_cfb_buf1, bits, sizeof(TestData_DES_plain), &ks, &TestData_DES_cfb_tmp, DES_ENCRYPT);
		if(!TEST_mem_eq(cfb_cipher, sizeof(TestData_DES_plain), TestData_DES_cfb_buf1, sizeof(TestData_DES_plain)))
			return 0;
		memcpy(TestData_DES_cfb_tmp, TestData_DES_cfb_iv, sizeof(TestData_DES_cfb_iv));
		DES_cfb_encrypt(TestData_DES_cfb_buf1, TestData_DES_cfb_buf2, bits, sizeof(TestData_DES_plain), &ks, &TestData_DES_cfb_tmp, DES_DECRYPT);
		return TEST_mem_eq(TestData_DES_plain, sizeof(TestData_DES_plain), TestData_DES_cfb_buf2, sizeof(TestData_DES_plain));
	}
	static int test_des_cfb8() { return cfb_test(8, TestData_DES_cfb_cipher8); }
	static int test_des_cfb16() { return cfb_test(16, TestData_DES_cfb_cipher16); }
	static int test_des_cfb32() { return cfb_test(32, TestData_DES_cfb_cipher32); }
	static int test_des_cfb48() { return cfb_test(48, TestData_DES_cfb_cipher48); }
	static int test_des_cfb64()
	{
		DES_key_schedule ks;
		int n;
		size_t i;
		if(!cfb_test(64, TestData_DES_cfb_cipher64))
			return 0;
		DES_set_key_checked(&TestData_DES_cfb_key, &ks);
		memcpy(TestData_DES_cfb_tmp, TestData_DES_cfb_iv, sizeof(TestData_DES_cfb_iv));
		n = 0;
		DES_cfb64_encrypt(TestData_DES_plain, TestData_DES_cfb_buf1, 12, &ks, &TestData_DES_cfb_tmp, &n, DES_ENCRYPT);
		DES_cfb64_encrypt(&TestData_DES_plain[12], &TestData_DES_cfb_buf1[12], sizeof(TestData_DES_plain) - 12, &ks, &TestData_DES_cfb_tmp, &n, DES_ENCRYPT);
		if(!TEST_mem_eq(TestData_DES_cfb_cipher64, sizeof(TestData_DES_plain), TestData_DES_cfb_buf1, sizeof(TestData_DES_plain)))
			return 0;
		memcpy(TestData_DES_cfb_tmp, TestData_DES_cfb_iv, sizeof(TestData_DES_cfb_iv));
		n = 0;
		DES_cfb64_encrypt(TestData_DES_cfb_buf1, TestData_DES_cfb_buf2, 17, &ks, &TestData_DES_cfb_tmp, &n, DES_DECRYPT);
		DES_cfb64_encrypt(&TestData_DES_cfb_buf1[17], &TestData_DES_cfb_buf2[17], sizeof(TestData_DES_plain) - 17, &ks, &TestData_DES_cfb_tmp, &n, DES_DECRYPT);
		if(!TEST_mem_eq(TestData_DES_plain, sizeof(TestData_DES_plain), TestData_DES_cfb_buf2, sizeof(TestData_DES_plain)))
			return 0;
		memcpy(TestData_DES_cfb_tmp, TestData_DES_cfb_iv, sizeof(TestData_DES_cfb_iv));
		for(i = 0; i < sizeof(TestData_DES_plain); i++)
			DES_cfb_encrypt(&TestData_DES_plain[i], &TestData_DES_cfb_buf1[i], 8, 1, &ks, &TestData_DES_cfb_tmp, DES_ENCRYPT);
		if(!TEST_mem_eq(TestData_DES_cfb_cipher8, sizeof(TestData_DES_plain), TestData_DES_cfb_buf1, sizeof(TestData_DES_plain)))
			return 0;
		memcpy(TestData_DES_cfb_tmp, TestData_DES_cfb_iv, sizeof(TestData_DES_cfb_iv));
		for(i = 0; i < sizeof(TestData_DES_plain); i++)
			DES_cfb_encrypt(&TestData_DES_cfb_buf1[i], &TestData_DES_cfb_buf2[i], 8, 1, &ks, &TestData_DES_cfb_tmp, DES_DECRYPT);
		return TEST_mem_eq(TestData_DES_plain, sizeof(TestData_DES_plain), TestData_DES_cfb_buf2, sizeof(TestData_DES_plain));
	}
	static int test_des_ede_cfb64()
	{
		DES_key_schedule ks;
		int n;
		DES_set_key_checked(&TestData_DES_cfb_key, &ks);
		memcpy(TestData_DES_cfb_tmp, TestData_DES_cfb_iv, sizeof(TestData_DES_cfb_iv));
		n = 0;
		DES_ede3_cfb64_encrypt(TestData_DES_plain, TestData_DES_cfb_buf1, 12, &ks, &ks, &ks, &TestData_DES_cfb_tmp, &n, DES_ENCRYPT);
		DES_ede3_cfb64_encrypt(&TestData_DES_plain[12], &TestData_DES_cfb_buf1[12], sizeof(TestData_DES_plain) - 12, &ks, &ks, &ks, &TestData_DES_cfb_tmp, &n, DES_ENCRYPT);
		if(!TEST_mem_eq(TestData_DES_cfb_cipher64, sizeof(TestData_DES_plain), TestData_DES_cfb_buf1, sizeof(TestData_DES_plain)))
			return 0;
		memcpy(TestData_DES_cfb_tmp, TestData_DES_cfb_iv, sizeof(TestData_DES_cfb_iv));
		n = 0;
		DES_ede3_cfb64_encrypt(TestData_DES_cfb_buf1, TestData_DES_cfb_buf2, (long)17, &ks, &ks, &ks, &TestData_DES_cfb_tmp, &n, DES_DECRYPT);
		DES_ede3_cfb64_encrypt(&TestData_DES_cfb_buf1[17], &TestData_DES_cfb_buf2[17], sizeof(TestData_DES_plain) - 17, &ks, &ks, &ks, &TestData_DES_cfb_tmp, &n, DES_DECRYPT);
		return TEST_mem_eq(TestData_DES_plain, sizeof(TestData_DES_plain), TestData_DES_cfb_buf2, sizeof(TestData_DES_plain));
	}
	static int test_des_ofb()
	{
		DES_key_schedule ks;
		DES_set_key_checked(&TestData_DES_ofb_key, &ks);
		memcpy(TestData_DES_ofb_tmp, TestData_DES_ofb_iv, sizeof(TestData_DES_ofb_iv));
		DES_ofb_encrypt(TestData_DES_plain, TestData_DES_ofb_buf1, 64, sizeof(TestData_DES_plain) / 8, &ks, &TestData_DES_ofb_tmp);
		if(!TEST_mem_eq(TestData_DES_ofb_cipher, sizeof(TestData_DES_ofb_buf1), TestData_DES_ofb_buf1, sizeof(TestData_DES_ofb_buf1)))
			return 0;
		memcpy(TestData_DES_ofb_tmp, TestData_DES_ofb_iv, sizeof(TestData_DES_ofb_iv));
		DES_ofb_encrypt(TestData_DES_ofb_buf1, TestData_DES_ofb_buf2, 64, sizeof(TestData_DES_ofb_buf1) / 8, &ks, &TestData_DES_ofb_tmp);
		return TEST_mem_eq(TestData_DES_plain, sizeof(TestData_DES_ofb_buf2), TestData_DES_ofb_buf2, sizeof(TestData_DES_ofb_buf2));
	}
	static int test_des_ofb64()
	{
		DES_key_schedule ks;
		int num;
		size_t i;
		DES_set_key_checked(&TestData_DES_ofb_key, &ks);
		memcpy(TestData_DES_ofb_tmp, TestData_DES_ofb_iv, sizeof(TestData_DES_ofb_iv));
		memzero(TestData_DES_ofb_buf1, sizeof(TestData_DES_ofb_buf1));
		memzero(TestData_DES_ofb_buf2, sizeof(TestData_DES_ofb_buf1));
		num = 0;
		for(i = 0; i < sizeof(TestData_DES_plain); i++) {
			DES_ofb64_encrypt(&TestData_DES_plain[i], &TestData_DES_ofb_buf1[i], 1, &ks, &TestData_DES_ofb_tmp, &num);
		}
		if(!TEST_mem_eq(TestData_DES_ofb_cipher, sizeof(TestData_DES_ofb_buf1), TestData_DES_ofb_buf1, sizeof(TestData_DES_ofb_buf1)))
			return 0;
		memcpy(TestData_DES_ofb_tmp, TestData_DES_ofb_iv, sizeof(TestData_DES_ofb_iv));
		num = 0;
		DES_ofb64_encrypt(TestData_DES_ofb_buf1, TestData_DES_ofb_buf2, sizeof(TestData_DES_ofb_buf1), &ks, &TestData_DES_ofb_tmp, &num);
		return TEST_mem_eq(TestData_DES_plain, sizeof(TestData_DES_ofb_buf2), TestData_DES_ofb_buf2, sizeof(TestData_DES_ofb_buf2));
	}
	static int test_des_ede_ofb64()
	{
		DES_key_schedule ks;
		int num;
		size_t i;
		DES_set_key_checked(&TestData_DES_ofb_key, &ks);
		memcpy(TestData_DES_ofb_tmp, TestData_DES_ofb_iv, sizeof(TestData_DES_ofb_iv));
		memzero(TestData_DES_ofb_buf1, sizeof(TestData_DES_ofb_buf1));
		memzero(TestData_DES_ofb_buf2, sizeof(TestData_DES_ofb_buf1));
		num = 0;
		for(i = 0; i < sizeof(TestData_DES_plain); i++) {
			DES_ede3_ofb64_encrypt(&TestData_DES_plain[i], &TestData_DES_ofb_buf1[i], 1, &ks, &ks, &ks, &TestData_DES_ofb_tmp, &num);
		}
		if(!TEST_mem_eq(TestData_DES_ofb_cipher, sizeof(TestData_DES_ofb_buf1), TestData_DES_ofb_buf1, sizeof(TestData_DES_ofb_buf1)))
			return 0;
		memcpy(TestData_DES_ofb_tmp, TestData_DES_ofb_iv, sizeof(TestData_DES_ofb_iv));
		num = 0;
		DES_ede3_ofb64_encrypt(TestData_DES_ofb_buf1, TestData_DES_ofb_buf2, sizeof(TestData_DES_ofb_buf1), &ks, &ks, &ks, &TestData_DES_ofb_tmp, &num);
		return TEST_mem_eq(TestData_DES_plain, sizeof(TestData_DES_ofb_buf2), TestData_DES_ofb_buf2, sizeof(TestData_DES_ofb_buf2));
	}
	static int test_des_cbc_cksum()
	{
		DES_LONG cs;
		DES_key_schedule ks;
		uchar cret[8];
		DES_set_key_checked(&TestData_DES_cbc_key, &ks);
		cs = DES_cbc_cksum(TestData_DES_cbc_data, &cret, strlen((char*)TestData_DES_cbc_data), &ks, &TestData_DES_cbc_iv);
		if(!TestData_DES_TEST_cs_eq(cs, TestData_DES_cbc_cksum_ret))
			return 0;
		return TEST_mem_eq(cret, 8, TestData_DES_cbc_cksum_data, 8);
	}
	static int test_des_quad_cksum()
	{
		DES_LONG lqret[4];
		DES_LONG cs = DES_quad_cksum(TestData_DES_cbc_data, (DES_cblock*)lqret, (long)strlen((char*)TestData_DES_cbc_data), 2, (DES_cblock*)TestData_DES_cbc_iv);
		if(!TestData_DES_TEST_cs_eq(cs, 0x70d7a63aL))
			return 0;
		if(!TestData_DES_TEST_cs_eq(lqret[0], 0x327eba8dL))
			return 0;
		if(!TestData_DES_TEST_cs_eq(lqret[1], 0x201a49ccL))
			return 0;
		if(!TestData_DES_TEST_cs_eq(lqret[2], 0x70d7a63aL))
			return 0;
		if(!TestData_DES_TEST_cs_eq(lqret[3], 0x501c2c26L))
			return 0;
		return 1;
	}
	static int test_des_key_wrap(int idx)
	{
		int in_bytes = TestData_DES_test_des_key_wrap_sizes[idx];
		uchar in[100], c_txt[200], p_txt[200], key[24];
		int clen, clen_upd, clen_fin, plen, plen_upd, plen_fin, expect, bs, i;
		EVP_CIPHER * cipher = NULL;
		EVP_CIPHER_CTX * ctx = NULL;
		int res = 0;
		// Some sanity checks and cipher loading 
		if(!TEST_size_t_le(in_bytes, sizeof(in))
			|| !TEST_ptr(cipher = EVP_CIPHER_fetch(NULL, "DES3-WRAP", NULL))
			|| !TEST_int_eq(bs = EVP_CIPHER_get_block_size(cipher), 8)
			|| !TEST_size_t_eq(bs * 3u, sizeof(key))
			|| !TEST_true(in_bytes % bs == 0)
			|| !TEST_ptr(ctx = EVP_CIPHER_CTX_new()))
			goto err;
		// Create random data to end to end test 
		for(i = 0; i < in_bytes; i++)
			in[i] = test_random();
		// Build the key 
		memcpy(key, TestData_DES_cbc_key, sizeof(TestData_DES_cbc_key));
		memcpy(key + sizeof(TestData_DES_cbc_key), TestData_DES_cbc2_key, sizeof(TestData_DES_cbc2_key));
		memcpy(key + sizeof(TestData_DES_cbc_key) + sizeof(TestData_DES_cbc3_key), TestData_DES_cbc_key, sizeof(TestData_DES_cbc3_key));
		// Wrap / encrypt the key 
		clen_upd = sizeof(c_txt);
		if(!TEST_true(EVP_EncryptInit(ctx, cipher, key, NULL)) || !TEST_true(EVP_EncryptUpdate(ctx, c_txt, &clen_upd, in, in_bytes)))
			goto err;
		expect = (in_bytes + (bs - 1)) / bs * bs + 2 * bs;
		if(!TEST_int_eq(clen_upd, expect))
			goto err;
		clen_fin = sizeof(c_txt) - clen_upd;
		if(!TEST_true(EVP_EncryptFinal(ctx, c_txt + clen_upd, &clen_fin)) || !TEST_int_eq(clen_fin, 0))
			goto err;
		clen = clen_upd + clen_fin;
		// Decrypt the wrapped key 
		plen_upd = sizeof(p_txt);
		if(!TEST_true(EVP_DecryptInit(ctx, cipher, key, NULL)) || !TEST_true(EVP_DecryptUpdate(ctx, p_txt, &plen_upd,
			c_txt, clen)))
			goto err;
		plen_fin = sizeof(p_txt) - plen_upd;
		if(!TEST_true(EVP_DecryptFinal(ctx, p_txt + plen_upd, &plen_fin)))
			goto err;
		plen = plen_upd + plen_fin;
		if(!TEST_mem_eq(in, in_bytes, p_txt, plen))
			goto err;
		res = 1;
	err:
		EVP_CIPHER_free(cipher);
		EVP_CIPHER_CTX_free(ctx);
		return res;
	}
	static int test_des_weak_keys(int n)
	{
		const_DES_cblock * key = (uchar (*)[8])TestData_DES_weak_keys[n].key;
		return TEST_int_eq(DES_is_weak_key(key), TestData_DES_weak_keys[n].expect);
	}
	static int test_des_check_bad_parity(int n)
	{
		const_DES_cblock * key = (uchar (*)[8])TestData_DES_bad_parity_keys[n].key;
		return TEST_int_eq(DES_check_key_parity(key), TestData_DES_bad_parity_keys[n].expect);
	}
};
#endif
//
//
//
static const struct {
	const char * defn;
	const char * query;
	int e;
} TestData_Property_parser_tests[] = {
	{ "", "sky=blue", -1 },
	{ "", "sky!=blue", 1 },
	{ "groan", "", 0 },
	{ "cold=yes", "cold=yes", 1 },
	{ "cold=yes", "cold", 1 },
	{ "cold=yes", "cold!=no", 1 },
	{ "groan", "groan=yes", 1 },
	{ "groan", "groan=no", -1 },
	{ "groan", "groan!=yes", -1 },
	{ "cold=no", "cold", -1 },
	{ "cold=no", "?cold", 0 },
	{ "cold=no", "cold=no", 1 },
	{ "groan", "cold", -1 },
	{ "groan", "cold=no", 1 },
	{ "groan", "cold!=yes", 1 },
	{ "groan=blue", "groan=yellow", -1 },
	{ "groan=blue", "?groan=yellow", 0 },
	{ "groan=blue", "groan!=yellow", 1 },
	{ "groan=blue", "?groan!=yellow", 1 },
	{ "today=monday, tomorrow=3", "today!=2", 1 },
	{ "today=monday, tomorrow=3", "today!='monday'", -1 },
	{ "today=monday, tomorrow=3", "tomorrow=3", 1 },
	{ "n=0x3", "n=3", 1 },
	{ "n=0x3", "n=-3", -1 },
	{ "n=0x33", "n=51", 1 },
	{ "n=033", "n=27", 1 },
	{ "n=0", "n=00", 1 },
	{ "n=0x0", "n=0", 1 },
	{ "n=0, sky=blue", "?n=0, sky=blue", 2 },
	{ "n=1, sky=blue", "?n=0, sky=blue", 1 },
};

static const struct {
	int query;
	const char * ps;
} TestData_Property_parse_error_tests[] = {
	{ 0, "n=1, n=1" },      /* duplicate name */
	{ 0, "n=1, a=hi, n=1" }, /* duplicate name */
	{ 1, "n=1, a=bye, ?n=0" }, /* duplicate name */
	{ 0, "a=abc,#@!, n=1" }, /* non-ASCII character located */
	{ 1, "a='Hello" },      /* Unterminated string */
	{ 0, "a=\"World" },     /* Unterminated string */
	{ 1, "a=2, n=012345678" }, /* Bad octal digit */
	{ 0, "n=0x28FG, a=3" }, /* Bad hex digit */
	{ 0, "n=145d, a=2" },   /* Bad decimal digit */
	{ 1, "@='hello'" },     /* Invalid name */
	{ 1, "n0123456789012345678901234567890123456789"
	  "0123456789012345678901234567890123456789"
	  "0123456789012345678901234567890123456789"
	  "0123456789012345678901234567890123456789=yes" }, /* Name too long */
	{ 0, ".n=3" },          /* Invalid name */
	{ 1, "fnord.fnord.=3" } /* Invalid name */
};

static const struct {
	const char * q_global;
	const char * q_local;
	const char * prop;
} TestData_Property_merge_tests[] = {
	{ "", "colour=blue", "colour=blue" },
	{ "colour=blue", "", "colour=blue" },
	{ "colour=red", "colour=blue", "colour=blue" },
	{ "clouds=pink, urn=red", "urn=blue, colour=green",
	  "urn=blue, colour=green, clouds=pink" },
	{ "pot=gold", "urn=blue", "pot=gold, urn=blue" },
	{ "night", "day", "day=yes, night=yes" },
	{ "day", "night", "day=yes, night=yes" },
	{ "", "", "" },
	/*
	 * The following four leave 'day' unspecified in the query, and will match
	 * any definition
	 */
	{ "day=yes", "-day", "day=no" },
	{ "day=yes", "-day", "day=yes" },
	{ "day=yes", "-day", "day=arglebargle" },
	{ "day=yes", "-day", "pot=sesquioxidizing" },
	{ "day, night", "-night, day", "day=yes, night=no" },
	{ "-day", "day=yes", "day=yes" },
};

static const struct {
	const char * defn;
	const char * query;
	int e;
} TestData_Property_definition_tests[] = {
	{ "alpha", "alpha=yes", 1 },
	{ "alpha=no", "alpha", -1 },
	{ "alpha=1", "alpha=1", 1 },
	{ "alpha=2", "alpha=1", -1 },
	{ "alpha", "omega", -1 },
	{ "alpha", "?omega", 0 },
	{ "alpha", "?omega=1", 0 },
	{ "alpha", "?omega=no", 1 },
	{ "alpha", "?omega=yes", 0 },
	{ "alpha, omega", "?omega=yes", 1 },
	{ "alpha, omega", "?omega=no", 0 }
};

static struct {
	const char * in;
	const char * out;
} TestData_Property_to_string_tests[] = {
	{ "fips=yes", "fips=yes" },
	{ "fips!=yes", "fips!=yes" },
	{ "fips = yes", "fips=yes" },
	{ "fips", "fips=yes" },
	{ "fips=no", "fips=no" },
	{ "-fips", "-fips" },
	{ "?fips=yes", "?fips=yes" },
	{ "fips=yes,provider=fips", "fips=yes,provider=fips" },
	{ "fips = yes , provider = fips", "fips=yes,provider=fips" },
	{ "fips=yes,provider!=fips", "fips=yes,provider!=fips" },
	{ "fips=yes,?provider=fips", "fips=yes,?provider=fips" },
	{ "fips=yes,-provider", "fips=yes,-provider" },
	/* foo is an unknown internal name */
	{ "foo=yes,fips=yes", "fips=yes"},
	{ "", "" },
	{ "fips=3", "fips=3" },
	{ "fips=-3", "fips=-3" },
	{ NULL, "" }
};
// 
// We make our OSSL_PROVIDER for testing purposes.  All we really need is
// a pointer.  We know that as long as we don't try to use the method
// cache flush functions, the provider pointer is merely a pointer being
// passed around, and used as a tag of sorts.
// 
struct ossl_provider_st {
	int x;
};

class TestInnerBlock_Property {
public:
	static int add_property_names(const char * n, ...)
	{
		va_list args;
		int res = 1;
		va_start(args, n);
		do {
			if(!TEST_int_ne(ossl_property_name(NULL, n, 1), 0))
				res = 0;
		} while((n = va_arg(args, const char *)) != NULL);
		va_end(args);
		return res;
	}
	static int up_ref(void * p)
	{
		return 1;
	}
	static void down_ref(void * p)
	{
	}
	static int test_property_string()
	{
		OSSL_METHOD_STORE * store;
		int res = 0;
		OSSL_PROPERTY_IDX i, j;
		if(TEST_ptr(store = ossl_method_store_new(NULL))
			&& TEST_int_eq(ossl_property_name(NULL, "fnord", 0), 0)
			&& TEST_int_ne(ossl_property_name(NULL, "fnord", 1), 0)
			&& TEST_int_ne(ossl_property_name(NULL, "name", 1), 0)
			/* Property value checks */
			&& TEST_int_eq(ossl_property_value(NULL, "fnord", 0), 0)
			&& TEST_int_ne(i = ossl_property_value(NULL, "no", 0), 0)
			&& TEST_int_ne(j = ossl_property_value(NULL, "yes", 0), 0)
			&& TEST_int_ne(i, j)
			&& TEST_int_eq(ossl_property_value(NULL, "yes", 1), j)
			&& TEST_int_eq(ossl_property_value(NULL, "no", 1), i)
			&& TEST_int_ne(i = ossl_property_value(NULL, "illuminati", 1), 0)
			&& TEST_int_eq(j = ossl_property_value(NULL, "fnord", 1), i + 1)
			&& TEST_int_eq(ossl_property_value(NULL, "fnord", 1), j)
			/* Check name and values are distinct */
			&& TEST_int_eq(ossl_property_value(NULL, "cold", 0), 0)
			&& TEST_int_ne(ossl_property_name(NULL, "fnord", 0),
			ossl_property_value(NULL, "fnord", 0)))
			res = 1;
		ossl_method_store_free(store);
		return res;
	}
	static int test_property_parse(int n)
	{
		OSSL_METHOD_STORE * store;
		OSSL_PROPERTY_LIST * p = NULL, * q = NULL;
		int r = 0;
		if(TEST_ptr(store = ossl_method_store_new(NULL))
			&& add_property_names("sky", "groan", "cold", "today", "tomorrow", "n", NULL)
			&& TEST_ptr(p = ossl_parse_property(NULL, TestData_Property_parser_tests[n].defn))
			&& TEST_ptr(q = ossl_parse_query(NULL, TestData_Property_parser_tests[n].query, 0))
			&& TEST_int_eq(ossl_property_match_count(q, p), TestData_Property_parser_tests[n].e))
			r = 1;
		ossl_property_free(p);
		ossl_property_free(q);
		ossl_method_store_free(store);
		return r;
	}
	static int test_property_query_value_create()
	{
		OSSL_METHOD_STORE * store;
		OSSL_PROPERTY_LIST * p = NULL, * q = NULL, * o = NULL;
		int r = 0;
		/* The property value used here must not be used in other test cases */
		if(TEST_ptr(store = ossl_method_store_new(NULL))
			&& add_property_names("wood", NULL)
			&& TEST_ptr(p = ossl_parse_query(NULL, "wood=oak", 0)) /* undefined */
			&& TEST_ptr(q = ossl_parse_query(NULL, "wood=oak", 1)) /* creates */
			&& TEST_ptr(o = ossl_parse_query(NULL, "wood=oak", 0)) /* defined */
			&& TEST_int_eq(ossl_property_match_count(q, p), -1)
			&& TEST_int_eq(ossl_property_match_count(q, o), 1))
			r = 1;
		ossl_property_free(o);
		ossl_property_free(p);
		ossl_property_free(q);
		ossl_method_store_free(store);
		return r;
	}
	static int test_property_parse_error(int n)
	{
		OSSL_METHOD_STORE * store;
		OSSL_PROPERTY_LIST * p = NULL;
		int r = 0;
		const char * ps;
		if(!TEST_ptr(store = ossl_method_store_new(NULL)) || !add_property_names("a", "n", NULL))
			goto err;
		ps = TestData_Property_parse_error_tests[n].ps;
		if(TestData_Property_parse_error_tests[n].query) {
			if(!TEST_ptr_null(p = ossl_parse_query(NULL, ps, 1)))
				goto err;
		}
		else if(!TEST_ptr_null(p = ossl_parse_property(NULL, ps))) {
			goto err;
		}
		r = 1;
	err:
		ossl_property_free(p);
		ossl_method_store_free(store);
		return r;
	}
	static int test_property_merge(int n)
	{
		OSSL_METHOD_STORE * store;
		OSSL_PROPERTY_LIST * q_global = NULL, * q_local = NULL;
		OSSL_PROPERTY_LIST * q_combined = NULL, * prop = NULL;
		int r = 0;
		if(TEST_ptr(store = ossl_method_store_new(NULL))
			&& add_property_names("colour", "urn", "clouds", "pot", "day", "night", NULL)
			&& TEST_ptr(prop = ossl_parse_property(NULL, TestData_Property_merge_tests[n].prop))
			&& TEST_ptr(q_global = ossl_parse_query(NULL, TestData_Property_merge_tests[n].q_global, 0))
			&& TEST_ptr(q_local = ossl_parse_query(NULL, TestData_Property_merge_tests[n].q_local, 0))
			&& TEST_ptr(q_combined = ossl_property_merge(q_local, q_global))
			&& TEST_int_ge(ossl_property_match_count(q_combined, prop), 0))
			r = 1;
		ossl_property_free(q_global);
		ossl_property_free(q_local);
		ossl_property_free(q_combined);
		ossl_property_free(prop);
		ossl_method_store_free(store);
		return r;
	}
	static int test_property_defn_cache()
	{
		OSSL_METHOD_STORE * store;
		OSSL_PROPERTY_LIST * red, * blue;
		int r = 0;
		if(TEST_ptr(store = ossl_method_store_new(NULL))
			&& add_property_names("red", "blue", NULL)
			&& TEST_ptr(red = ossl_parse_property(NULL, "red"))
			&& TEST_ptr(blue = ossl_parse_property(NULL, "blue"))
			&& TEST_ptr_ne(red, blue)
			&& TEST_true(ossl_prop_defn_set(NULL, "red", red))
			&& TEST_true(ossl_prop_defn_set(NULL, "blue", blue))
			&& TEST_ptr_eq(ossl_prop_defn_get(NULL, "red"), red)
			&& TEST_ptr_eq(ossl_prop_defn_get(NULL, "blue"), blue))
			r = 1;
		ossl_method_store_free(store);
		return r;
	}
	static int test_definition_compares(int n)
	{
		OSSL_METHOD_STORE * store;
		OSSL_PROPERTY_LIST * d = NULL, * q = NULL;
		int r = TEST_ptr(store = ossl_method_store_new(NULL))
			&& add_property_names("alpha", "omega", NULL)
			&& TEST_ptr(d = ossl_parse_property(NULL, TestData_Property_definition_tests[n].defn))
			&& TEST_ptr(q = ossl_parse_query(NULL, TestData_Property_definition_tests[n].query, 0))
			&& TEST_int_eq(ossl_property_match_count(q, d), TestData_Property_definition_tests[n].e);
		ossl_property_free(d);
		ossl_property_free(q);
		ossl_method_store_free(store);
		return r;
	}
	static int test_register_deregister()
	{
		static const struct {
			int nid;
			const char * prop;
			char * impl;
		} impls[] = {
			{ 6, "position=1", "a" },
			{ 6, "position=2", "b" },
			{ 6, "position=3", "c" },
			{ 6, "position=4", "d" },
		};
		size_t i;
		int ret = 0;
		OSSL_METHOD_STORE * store;
		OSSL_PROVIDER prov = { 1 };
		if(!TEST_ptr(store = ossl_method_store_new(NULL)) || !add_property_names("position", NULL))
			goto err;
		for(i = 0; i < SIZEOFARRAY(impls); i++)
			if(!TEST_true(ossl_method_store_add(store, &prov, impls[i].nid, impls[i].prop, impls[i].impl, &up_ref, &down_ref))) {
				TEST_note("iteration %zd", i + 1);
				goto err;
			}

		/* Deregister in a different order to registration */
		for(i = 0; i < SIZEOFARRAY(impls); i++) {
			const size_t j = (1 + i * 3) % SIZEOFARRAY(impls);
			int nid = impls[j].nid;
			void * impl = impls[j].impl;

			if(!TEST_true(ossl_method_store_remove(store, nid, impl))
				|| !TEST_false(ossl_method_store_remove(store, nid, impl))) {
				TEST_note("iteration %zd, position %zd", i + 1, j + 1);
				goto err;
			}
		}

		if(TEST_false(ossl_method_store_remove(store, impls[0].nid, impls[0].impl)))
			ret = 1;
	err:
		ossl_method_store_free(store);
		return ret;
	}
	static int test_property()
	{
		static OSSL_PROVIDER fake_provider1 = { 1 };
		static OSSL_PROVIDER fake_provider2 = { 2 };
		static const OSSL_PROVIDER * fake_prov1 = &fake_provider1;
		static const OSSL_PROVIDER * fake_prov2 = &fake_provider2;
		static const struct {
			const OSSL_PROVIDER ** prov;
			int nid;
			const char * prop;
			char * impl;
		} impls[] = {
			{ &fake_prov1, 1, "fast=no, colour=green", "a" },
			{ &fake_prov1, 1, "fast, colour=blue", "b" },
			{ &fake_prov1, 1, "", "-" },
			{ &fake_prov2, 9, "sky=blue, furry", "c" },
			{ &fake_prov2, 3, NULL, "d" },
			{ &fake_prov2, 6, "sky.colour=blue, sky=green, old.data", "e" },
		};
		static struct {
			const OSSL_PROVIDER ** prov;
			int nid;
			const char * prop;
			char * expected;
		} queries[] = {
			{ &fake_prov1, 1, "fast", "b" },
			{ &fake_prov1, 1, "fast=yes", "b" },
			{ &fake_prov1, 1, "fast=no, colour=green", "a" },
			{ &fake_prov1, 1, "colour=blue, fast", "b" },
			{ &fake_prov1, 1, "colour=blue", "b" },
			{ &fake_prov2, 9, "furry", "c" },
			{ &fake_prov2, 6, "sky.colour=blue", "e" },
			{ &fake_prov2, 6, "old.data", "e" },
			{ &fake_prov2, 9, "furry=yes, sky=blue", "c" },
			{ &fake_prov1, 1, "", "a" },
			{ &fake_prov2, 3, "", "d" },
		};
		OSSL_METHOD_STORE * store;
		size_t i;
		int ret = 0;
		void * result = NULL;
		if(!TEST_ptr(store = ossl_method_store_new(NULL)) || !add_property_names("fast", "colour", "sky", "furry", NULL))
			goto err;
		for(i = 0; i < SIZEOFARRAY(impls); i++)
			if(!TEST_true(ossl_method_store_add(store, *impls[i].prov,
				impls[i].nid, impls[i].prop,
				impls[i].impl,
				&up_ref, &down_ref))) {
				TEST_note("iteration %zd", i + 1);
				goto err;
			}
		/*
		 * The first check of queries is with NULL given as provider.  All
		 * queries are expected to succeed.
		 */
		for(i = 0; i < SIZEOFARRAY(queries); i++) {
			const OSSL_PROVIDER * nullprov = NULL;
			OSSL_PROPERTY_LIST * pq = NULL;
			if(!TEST_true(ossl_method_store_fetch(store, queries[i].nid, queries[i].prop, &nullprov, &result))
				|| !TEST_str_eq((char*)result, queries[i].expected)) {
				TEST_note("iteration %zd", i + 1);
				ossl_property_free(pq);
				goto err;
			}
			ossl_property_free(pq);
		}
		/*
		 * The second check of queries is with &address1 given as provider.
		 */
		for(i = 0; i < SIZEOFARRAY(queries); i++) {
			OSSL_PROPERTY_LIST * pq = NULL;
			result = NULL;
			if(queries[i].prov == &fake_prov1) {
				if(!TEST_true(ossl_method_store_fetch(store,
					queries[i].nid,
					queries[i].prop,
					&fake_prov1, &result))
					|| !TEST_ptr_eq(fake_prov1, &fake_provider1)
					|| !TEST_str_eq((char*)result, queries[i].expected)) {
					TEST_note("iteration %zd", i + 1);
					ossl_property_free(pq);
					goto err;
				}
			}
			else {
				if(!TEST_false(ossl_method_store_fetch(store,
					queries[i].nid,
					queries[i].prop,
					&fake_prov1, &result))
					|| !TEST_ptr_eq(fake_prov1, &fake_provider1)
					|| !TEST_ptr_null(result)) {
					TEST_note("iteration %zd", i + 1);
					ossl_property_free(pq);
					goto err;
				}
			}
			ossl_property_free(pq);
		}
		/*
		 * The third check of queries is with &address2 given as provider.
		 */
		for(i = 0; i < SIZEOFARRAY(queries); i++) {
			OSSL_PROPERTY_LIST * pq = NULL;
			result = NULL;
			if(queries[i].prov == &fake_prov2) {
				if(!TEST_true(ossl_method_store_fetch(store,
					queries[i].nid,
					queries[i].prop,
					&fake_prov2, &result))
					|| !TEST_ptr_eq(fake_prov2, &fake_provider2)
					|| !TEST_str_eq((char*)result, queries[i].expected)) {
					TEST_note("iteration %zd", i + 1);
					ossl_property_free(pq);
					goto err;
				}
			}
			else {
				if(!TEST_false(ossl_method_store_fetch(store,
					queries[i].nid,
					queries[i].prop,
					&fake_prov2, &result))
					|| !TEST_ptr_eq(fake_prov2, &fake_provider2)
					|| !TEST_ptr_null(result)) {
					TEST_note("iteration %zd", i + 1);
					ossl_property_free(pq);
					goto err;
				}
			}
			ossl_property_free(pq);
		}
		ret = 1;
	err:
		ossl_method_store_free(store);
		return ret;
	}
	static int test_query_cache_stochastic()
	{
		const int max = 10000, tail = 10;
		OSSL_METHOD_STORE * store;
		int i, res = 0;
		char buf[50];
		void * result;
		int errors = 0;
		int v[10001];
		OSSL_PROVIDER prov = { 1 };
		if(!TEST_ptr(store = ossl_method_store_new(NULL)) || !add_property_names("n", NULL))
			goto err;
		for(i = 1; i <= max; i++) {
			v[i] = 2 * i;
			BIO_snprintf(buf, sizeof(buf), "n=%d\n", i);
			if(!TEST_true(ossl_method_store_add(store, &prov, i, buf, "abc",
				&up_ref, &down_ref))
				|| !TEST_true(ossl_method_store_cache_set(store, &prov, i,
				buf, v + i,
				&up_ref, &down_ref))
				|| !TEST_true(ossl_method_store_cache_set(store, &prov, i,
				"n=1234", "miss",
				&up_ref, &down_ref))) {
				TEST_note("iteration %d", i);
				goto err;
			}
		}
		for(i = 1; i <= max; i++) {
			BIO_snprintf(buf, sizeof(buf), "n=%d\n", i);
			if(!ossl_method_store_cache_get(store, NULL, i, buf, &result)
				|| result != v + i)
				errors++;
		}
		/* There is a tiny probability that this will fail when it shouldn't */
		res = TEST_int_gt(errors, tail) && TEST_int_lt(errors, max - tail);
	err:
		ossl_method_store_free(store);
		return res;
	}
	static int test_fips_mode()
	{
		int ret = 0;
		OSSL_LIB_CTX * ctx = NULL;
		if(!TEST_ptr(ctx = OSSL_LIB_CTX_new()))
			goto err;
		ret = TEST_true(EVP_set_default_properties(ctx, "default=yes,fips=yes"))
			&& TEST_true(EVP_default_properties_is_fips_enabled(ctx))
			&& TEST_true(EVP_set_default_properties(ctx, "fips=no,default=yes"))
			&& TEST_false(EVP_default_properties_is_fips_enabled(ctx))
			&& TEST_true(EVP_set_default_properties(ctx, "fips=no"))
			&& TEST_false(EVP_default_properties_is_fips_enabled(ctx))
			&& TEST_true(EVP_set_default_properties(ctx, "fips!=no"))
			&& TEST_true(EVP_default_properties_is_fips_enabled(ctx))
			&& TEST_true(EVP_set_default_properties(ctx, "fips=no"))
			&& TEST_false(EVP_default_properties_is_fips_enabled(ctx))
			&& TEST_true(EVP_set_default_properties(ctx, "fips=no,default=yes"))
			&& TEST_true(EVP_default_properties_enable_fips(ctx, 1))
			&& TEST_true(EVP_default_properties_is_fips_enabled(ctx))
			&& TEST_true(EVP_default_properties_enable_fips(ctx, 0))
			&& TEST_false(EVP_default_properties_is_fips_enabled(ctx));
	err:
		OSSL_LIB_CTX_free(ctx);
		return ret;
	}
	static int test_property_list_to_string(int i)
	{
		OSSL_PROPERTY_LIST * pl = NULL;
		int ret = 0;
		size_t bufsize;
		char * buf = NULL;
		if(TestData_Property_to_string_tests[i].in && !TEST_ptr(pl = ossl_parse_query(NULL, TestData_Property_to_string_tests[i].in, 1)))
			goto err;
		bufsize = ossl_property_list_to_string(NULL, pl, NULL, 0);
		if(!TEST_size_t_gt(bufsize, 0))
			goto err;
		buf = (char*)OPENSSL_malloc(bufsize);
		if(!TEST_ptr(buf) || !TEST_size_t_eq(ossl_property_list_to_string(NULL, pl, buf, bufsize), bufsize)
			|| !TEST_str_eq(TestData_Property_to_string_tests[i].out, buf)
			|| !TEST_size_t_eq(bufsize, strlen(TestData_Property_to_string_tests[i].out) + 1))
			goto err;

		ret = 1;
	err:
		OPENSSL_free(buf);
		ossl_property_free(pl);
		return ret;
	}
};
//
//
//
#define TestData_Packet_BUF_LEN 255
static uchar TestData_Packet_smbuf[TestData_Packet_BUF_LEN];

class TestInnerBlock_Packet {
public:
	static int test_PACKET_remaining()
	{
		PACKET pkt;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, sizeof(TestData_Packet_smbuf)))
			|| !TEST_size_t_eq(PACKET_remaining(&pkt), TestData_Packet_BUF_LEN)
			|| !TEST_true(PACKET_forward(&pkt, TestData_Packet_BUF_LEN - 1))
			|| !TEST_size_t_eq(PACKET_remaining(&pkt), 1)
			|| !TEST_true(PACKET_forward(&pkt, 1))
			|| !TEST_size_t_eq(PACKET_remaining(&pkt), 0))
			return 0;
		return 1;
	}
	static int test_PACKET_end()
	{
		PACKET pkt;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, sizeof(TestData_Packet_smbuf)))
			|| !TEST_size_t_eq(PACKET_remaining(&pkt), TestData_Packet_BUF_LEN)
			|| !TEST_ptr_eq(PACKET_end(&pkt), TestData_Packet_smbuf + TestData_Packet_BUF_LEN)
			|| !TEST_true(PACKET_forward(&pkt, TestData_Packet_BUF_LEN - 1))
			|| !TEST_ptr_eq(PACKET_end(&pkt), TestData_Packet_smbuf + TestData_Packet_BUF_LEN)
			|| !TEST_true(PACKET_forward(&pkt, 1))
			|| !TEST_ptr_eq(PACKET_end(&pkt), TestData_Packet_smbuf + TestData_Packet_BUF_LEN))
			return 0;
		return 1;
	}
	static int test_PACKET_get_1()
	{
		uint i = 0;
		PACKET pkt;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_get_1(&pkt, &i))
			|| !TEST_uint_eq(i, 0x02)
			|| !TEST_true(PACKET_forward(&pkt, TestData_Packet_BUF_LEN - 2))
			|| !TEST_true(PACKET_get_1(&pkt, &i))
			|| !TEST_uint_eq(i, 0xfe)
			|| !TEST_false(PACKET_get_1(&pkt, &i)))
			return 0;
		return 1;
	}
	static int test_PACKET_get_4()
	{
		ulong i = 0;
		PACKET pkt;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_get_4(&pkt, &i))
			|| !TEST_ulong_eq(i, 0x08060402UL)
			|| !TEST_true(PACKET_forward(&pkt, TestData_Packet_BUF_LEN - 8))
			|| !TEST_true(PACKET_get_4(&pkt, &i))
			|| !TEST_ulong_eq(i, 0xfefcfaf8UL)
			|| !TEST_false(PACKET_get_4(&pkt, &i)))
			return 0;
		return 1;
	}
	static int test_PACKET_get_net_2()
	{
		uint i = 0;
		PACKET pkt;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_get_net_2(&pkt, &i))
			|| !TEST_uint_eq(i, 0x0204)
			|| !TEST_true(PACKET_forward(&pkt, TestData_Packet_BUF_LEN - 4))
			|| !TEST_true(PACKET_get_net_2(&pkt, &i))
			|| !TEST_uint_eq(i, 0xfcfe)
			|| !TEST_false(PACKET_get_net_2(&pkt, &i)))
			return 0;
		return 1;
	}
	static int test_PACKET_get_net_3()
	{
		ulong i = 0;
		PACKET pkt;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_get_net_3(&pkt, &i))
			|| !TEST_ulong_eq(i, 0x020406UL)
			|| !TEST_true(PACKET_forward(&pkt, TestData_Packet_BUF_LEN - 6))
			|| !TEST_true(PACKET_get_net_3(&pkt, &i))
			|| !TEST_ulong_eq(i, 0xfafcfeUL)
			|| !TEST_false(PACKET_get_net_3(&pkt, &i)))
			return 0;
		return 1;
	}
	static int test_PACKET_get_net_4()
	{
		ulong i = 0;
		PACKET pkt;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_get_net_4(&pkt, &i))
			|| !TEST_ulong_eq(i, 0x02040608UL)
			|| !TEST_true(PACKET_forward(&pkt, TestData_Packet_BUF_LEN - 8))
			|| !TEST_true(PACKET_get_net_4(&pkt, &i))
			|| !TEST_ulong_eq(i, 0xf8fafcfeUL)
			|| !TEST_false(PACKET_get_net_4(&pkt, &i)))
			return 0;
		return 1;
	}
	static int test_PACKET_get_sub_packet()
	{
		PACKET pkt, subpkt;
		unsigned long i = 0;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_get_sub_packet(&pkt, &subpkt, 4))
			|| !TEST_true(PACKET_get_net_4(&subpkt, &i))
			|| !TEST_ulong_eq(i, 0x02040608UL)
			|| !TEST_size_t_eq(PACKET_remaining(&subpkt), 0)
			|| !TEST_true(PACKET_forward(&pkt, TestData_Packet_BUF_LEN - 8))
			|| !TEST_true(PACKET_get_sub_packet(&pkt, &subpkt, 4))
			|| !TEST_true(PACKET_get_net_4(&subpkt, &i))
			|| !TEST_ulong_eq(i, 0xf8fafcfeUL)
			|| !TEST_size_t_eq(PACKET_remaining(&subpkt), 0)
			|| !TEST_false(PACKET_get_sub_packet(&pkt, &subpkt, 4)))
			return 0;
		return 1;
	}
	static int test_PACKET_get_bytes()
	{
		const uchar * bytes = NULL;
		PACKET pkt;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_get_bytes(&pkt, &bytes, 4))
			|| !TEST_uchar_eq(bytes[0], 2)
			|| !TEST_uchar_eq(bytes[1], 4)
			|| !TEST_uchar_eq(bytes[2], 6)
			|| !TEST_uchar_eq(bytes[3], 8)
			|| !TEST_size_t_eq(PACKET_remaining(&pkt), TestData_Packet_BUF_LEN -4)
			|| !TEST_true(PACKET_forward(&pkt, TestData_Packet_BUF_LEN - 8))
			|| !TEST_true(PACKET_get_bytes(&pkt, &bytes, 4))
			|| !TEST_uchar_eq(bytes[0], 0xf8)
			|| !TEST_uchar_eq(bytes[1], 0xfa)
			|| !TEST_uchar_eq(bytes[2], 0xfc)
			|| !TEST_uchar_eq(bytes[3], 0xfe)
			|| !TEST_false(PACKET_remaining(&pkt)))
			return 0;

		return 1;
	}
	static int test_PACKET_copy_bytes()
	{
		uchar bytes[4];
		PACKET pkt;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_copy_bytes(&pkt, bytes, 4))
			|| !TEST_char_eq(bytes[0], 2)
			|| !TEST_char_eq(bytes[1], 4)
			|| !TEST_char_eq(bytes[2], 6)
			|| !TEST_char_eq(bytes[3], 8)
			|| !TEST_size_t_eq(PACKET_remaining(&pkt), TestData_Packet_BUF_LEN - 4)
			|| !TEST_true(PACKET_forward(&pkt, TestData_Packet_BUF_LEN - 8))
			|| !TEST_true(PACKET_copy_bytes(&pkt, bytes, 4))
			|| !TEST_uchar_eq(bytes[0], 0xf8)
			|| !TEST_uchar_eq(bytes[1], 0xfa)
			|| !TEST_uchar_eq(bytes[2], 0xfc)
			|| !TEST_uchar_eq(bytes[3], 0xfe)
			|| !TEST_false(PACKET_remaining(&pkt)))
			return 0;
		return 1;
	}
	static int test_PACKET_copy_all()
	{
		uchar tmp[TestData_Packet_BUF_LEN];
		PACKET pkt;
		size_t len;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_copy_all(&pkt, tmp, TestData_Packet_BUF_LEN, &len))
			|| !TEST_size_t_eq(len, TestData_Packet_BUF_LEN)
			|| !TEST_mem_eq(TestData_Packet_smbuf, TestData_Packet_BUF_LEN, tmp, TestData_Packet_BUF_LEN)
			|| !TEST_size_t_eq(PACKET_remaining(&pkt), TestData_Packet_BUF_LEN)
			|| !TEST_false(PACKET_copy_all(&pkt, tmp, TestData_Packet_BUF_LEN - 1, &len)))
			return 0;
		return 1;
	}
	static int test_PACKET_memdup()
	{
		uchar * data = NULL;
		size_t len;
		PACKET pkt;
		int result = 0;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_memdup(&pkt, &data, &len))
			|| !TEST_size_t_eq(len, TestData_Packet_BUF_LEN)
			|| !TEST_mem_eq(data, len, PACKET_data(&pkt), len)
			|| !TEST_true(PACKET_forward(&pkt, 10))
			|| !TEST_true(PACKET_memdup(&pkt, &data, &len))
			|| !TEST_size_t_eq(len, TestData_Packet_BUF_LEN - 10)
			|| !TEST_mem_eq(data, len, PACKET_data(&pkt), len))
			goto end;
		result = 1;
	end:
		OPENSSL_free(data);
		return result;
	}
	static int test_PACKET_strndup()
	{
		char buf1[10], buf2[10];
		char * data = NULL;
		PACKET pkt;
		int result = 0;
		memset(buf1, 'x', 10);
		memset(buf2, 'y', 10);
		buf2[5] = '\0';
		if(!TEST_true(PACKET_buf_init(&pkt, (uchar *)buf1, 10))
			|| !TEST_true(PACKET_strndup(&pkt, &data))
			|| !TEST_size_t_eq(strlen(data), 10)
			|| !TEST_strn_eq(data, buf1, 10)
			|| !TEST_true(PACKET_buf_init(&pkt, (uchar *)buf2, 10))
			|| !TEST_true(PACKET_strndup(&pkt, &data))
			|| !TEST_size_t_eq(strlen(data), 5)
			|| !TEST_str_eq(data, buf2))
			goto end;
		result = 1;
	end:
		OPENSSL_free(data);
		return result;
	}
	static int test_PACKET_contains_zero_byte()
	{
		char buf1[10], buf2[10];
		PACKET pkt;
		memset(buf1, 'x', 10);
		memset(buf2, 'y', 10);
		buf2[5] = '\0';
		if(!TEST_true(PACKET_buf_init(&pkt, (uchar *)buf1, 10))
			|| !TEST_false(PACKET_contains_zero_byte(&pkt))
			|| !TEST_true(PACKET_buf_init(&pkt, (uchar *)buf2, 10))
			|| !TEST_true(PACKET_contains_zero_byte(&pkt)))
			return 0;
		return 1;
	}
	static int test_PACKET_forward()
	{
		const uchar * byte = NULL;
		PACKET pkt;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_forward(&pkt, 1))
			|| !TEST_true(PACKET_get_bytes(&pkt, &byte, 1))
			|| !TEST_uchar_eq(byte[0], 4)
			|| !TEST_true(PACKET_forward(&pkt, TestData_Packet_BUF_LEN - 3))
			|| !TEST_true(PACKET_get_bytes(&pkt, &byte, 1))
			|| !TEST_uchar_eq(byte[0], 0xfe))
			return 0;
		return 1;
	}
	static int test_PACKET_buf_init()
	{
		uchar buf1[TestData_Packet_BUF_LEN] = { 0 };
		PACKET pkt;
		/* Also tests PACKET_remaining() */
		if(!TEST_true(PACKET_buf_init(&pkt, buf1, 4))
			|| !TEST_size_t_eq(PACKET_remaining(&pkt), 4)
			|| !TEST_true(PACKET_buf_init(&pkt, buf1, TestData_Packet_BUF_LEN))
			|| !TEST_size_t_eq(PACKET_remaining(&pkt), TestData_Packet_BUF_LEN)
			|| !TEST_false(PACKET_buf_init(&pkt, buf1, -1)))
			return 0;
		return 1;
	}
	static int test_PACKET_null_init()
	{
		PACKET pkt;
		PACKET_null_init(&pkt);
		if(!TEST_size_t_eq(PACKET_remaining(&pkt), 0) || !TEST_false(PACKET_forward(&pkt, 1)))
			return 0;
		return 1;
	}
	static int test_PACKET_equal()
	{
		PACKET pkt;
		if(!TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, 4))
			|| !TEST_true(PACKET_equal(&pkt, TestData_Packet_smbuf, 4))
			|| !TEST_false(PACKET_equal(&pkt, TestData_Packet_smbuf + 1, 4))
			|| !TEST_true(PACKET_buf_init(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_equal(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN))
			|| !TEST_false(PACKET_equal(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN - 1))
			|| !TEST_false(PACKET_equal(&pkt, TestData_Packet_smbuf, TestData_Packet_BUF_LEN + 1))
			|| !TEST_false(PACKET_equal(&pkt, TestData_Packet_smbuf, 0)))
			return 0;
		return 1;
	}
	static int test_PACKET_get_length_prefixed_1()
	{
		uchar buf1[TestData_Packet_BUF_LEN];
		const size_t len = 16;
		uint i;
		PACKET pkt, short_pkt, subpkt;
		memzero(&subpkt, sizeof(subpkt));
		buf1[0] = (uchar)len;
		for(i = 1; i < TestData_Packet_BUF_LEN; i++)
			buf1[i] = (i * 2) & 0xff;
		if(!TEST_true(PACKET_buf_init(&pkt, buf1, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_buf_init(&short_pkt, buf1, len))
			|| !TEST_true(PACKET_get_length_prefixed_1(&pkt, &subpkt))
			|| !TEST_size_t_eq(PACKET_remaining(&subpkt), len)
			|| !TEST_true(PACKET_get_net_2(&subpkt, &i))
			|| !TEST_uint_eq(i, 0x0204)
			|| !TEST_false(PACKET_get_length_prefixed_1(&short_pkt, &subpkt))
			|| !TEST_size_t_eq(PACKET_remaining(&short_pkt), len))
			return 0;
		return 1;
	}
	static int test_PACKET_get_length_prefixed_2()
	{
		uchar buf1[1024];
		const size_t len = 516; /* 0x0204 */
		uint i;
		PACKET pkt, short_pkt, subpkt;
		memzero(&subpkt, sizeof(subpkt));
		for(i = 1; i <= 1024; i++)
			buf1[i - 1] = (i * 2) & 0xff;
		if(!TEST_true(PACKET_buf_init(&pkt, buf1, 1024))
			|| !TEST_true(PACKET_buf_init(&short_pkt, buf1, len))
			|| !TEST_true(PACKET_get_length_prefixed_2(&pkt, &subpkt))
			|| !TEST_size_t_eq(PACKET_remaining(&subpkt), len)
			|| !TEST_true(PACKET_get_net_2(&subpkt, &i))
			|| !TEST_uint_eq(i, 0x0608)
			|| !TEST_false(PACKET_get_length_prefixed_2(&short_pkt, &subpkt))
			|| !TEST_size_t_eq(PACKET_remaining(&short_pkt), len))
			return 0;
		return 1;
	}
	static int test_PACKET_get_length_prefixed_3()
	{
		uchar buf1[1024];
		const size_t len = 516; /* 0x000204 */
		uint i;
		PACKET pkt, short_pkt, subpkt;
		memzero(&subpkt, sizeof(subpkt));
		for(i = 0; i < 1024; i++)
			buf1[i] = (i * 2) & 0xff;
		if(!TEST_true(PACKET_buf_init(&pkt, buf1, 1024))
			|| !TEST_true(PACKET_buf_init(&short_pkt, buf1, len))
			|| !TEST_true(PACKET_get_length_prefixed_3(&pkt, &subpkt))
			|| !TEST_size_t_eq(PACKET_remaining(&subpkt), len)
			|| !TEST_true(PACKET_get_net_2(&subpkt, &i))
			|| !TEST_uint_eq(i, 0x0608)
			|| !TEST_false(PACKET_get_length_prefixed_3(&short_pkt, &subpkt))
			|| !TEST_size_t_eq(PACKET_remaining(&short_pkt), len))
			return 0;

		return 1;
	}
	static int test_PACKET_as_length_prefixed_1()
	{
		uchar buf1[TestData_Packet_BUF_LEN];
		const size_t len = 16;
		uint i;
		PACKET pkt, exact_pkt, subpkt;
		memzero(&subpkt, sizeof(subpkt));
		buf1[0] = (uchar)len;
		for(i = 1; i < TestData_Packet_BUF_LEN; i++)
			buf1[i] = (i * 2) & 0xff;
		if(!TEST_true(PACKET_buf_init(&pkt, buf1, TestData_Packet_BUF_LEN))
			|| !TEST_true(PACKET_buf_init(&exact_pkt, buf1, len + 1))
			|| !TEST_false(PACKET_as_length_prefixed_1(&pkt, &subpkt))
			|| !TEST_size_t_eq(PACKET_remaining(&pkt), TestData_Packet_BUF_LEN)
			|| !TEST_true(PACKET_as_length_prefixed_1(&exact_pkt, &subpkt))
			|| !TEST_size_t_eq(PACKET_remaining(&exact_pkt), 0)
			|| !TEST_size_t_eq(PACKET_remaining(&subpkt), len))
			return 0;
		return 1;
	}
	static int test_PACKET_as_length_prefixed_2()
	{
		uchar buf[1024];
		const size_t len = 516; /* 0x0204 */
		uint i;
		PACKET pkt, exact_pkt, subpkt;
		memzero(&subpkt, sizeof(subpkt));
		for(i = 1; i <= 1024; i++)
			buf[i-1] = (i * 2) & 0xff;
		if(!TEST_true(PACKET_buf_init(&pkt, buf, 1024))
			|| !TEST_true(PACKET_buf_init(&exact_pkt, buf, len + 2))
			|| !TEST_false(PACKET_as_length_prefixed_2(&pkt, &subpkt))
			|| !TEST_size_t_eq(PACKET_remaining(&pkt), 1024)
			|| !TEST_true(PACKET_as_length_prefixed_2(&exact_pkt, &subpkt))
			|| !TEST_size_t_eq(PACKET_remaining(&exact_pkt), 0)
			|| !TEST_size_t_eq(PACKET_remaining(&subpkt), len))
			return 0;
		return 1;
	}
};
//
//
//
#define TestData_NameMapInternal_NAME1 "name1"
#define TestData_NameMapInternal_NAME2 "name2"
#define TestData_NameMapInternal_ALIAS1 "alias1"
#define TestData_NameMapInternal_ALIAS1_UC "ALIAS1"

class TestInnerBlock_NameMapInternal {
public:
	static int test_namemap_empty()
	{
		OSSL_NAMEMAP * nm = NULL;
		int ok = TEST_int_eq(ossl_namemap_empty(NULL), 1) && TEST_ptr(nm = ossl_namemap_new()) && TEST_int_eq(ossl_namemap_empty(nm), 1)
			&& TEST_int_ne(ossl_namemap_add_name(nm, 0, TestData_NameMapInternal_NAME1), 0) && TEST_int_eq(ossl_namemap_empty(nm), 0);
		ossl_namemap_free(nm);
		return ok;
	}
	static int test_namemap(OSSL_NAMEMAP * nm)
	{
		int num1 = ossl_namemap_add_name(nm, 0, TestData_NameMapInternal_NAME1);
		int num2 = ossl_namemap_add_name(nm, 0, TestData_NameMapInternal_NAME2);
		int num3 = ossl_namemap_add_name(nm, num1, TestData_NameMapInternal_ALIAS1);
		int num4 = ossl_namemap_add_name(nm, 0, TestData_NameMapInternal_ALIAS1_UC);
		int check1 = ossl_namemap_name2num(nm, TestData_NameMapInternal_NAME1);
		int check2 = ossl_namemap_name2num(nm, TestData_NameMapInternal_NAME2);
		int check3 = ossl_namemap_name2num(nm, TestData_NameMapInternal_ALIAS1);
		int check4 = ossl_namemap_name2num(nm, TestData_NameMapInternal_ALIAS1_UC);
		int false1 = ossl_namemap_name2num(nm, "cookie");
		return TEST_int_ne(num1, 0) && TEST_int_ne(num2, 0) && TEST_int_eq(num1, num3) && TEST_int_eq(num3, num4)
			   && TEST_int_eq(num1, check1) && TEST_int_eq(num2, check2) && TEST_int_eq(num3, check3) && TEST_int_eq(num4, check4) && TEST_int_eq(false1, 0);
	}
	static int test_namemap_independent()
	{
		OSSL_NAMEMAP * nm = ossl_namemap_new();
		int ok = TEST_ptr(nm) && test_namemap(nm);
		ossl_namemap_free(nm);
		return ok;
	}
	static int test_namemap_stored()
	{
		OSSL_NAMEMAP * nm = ossl_namemap_stored(NULL);
		return TEST_ptr(nm) && test_namemap(nm);
	}
	// 
	// Test that EVP_get_digestbyname() will use the namemap when it can't find entries in the legacy method database.
	// 
	static int test_digestbyname()
	{
		int id;
		OSSL_NAMEMAP * nm = ossl_namemap_stored(NULL);
		const EVP_MD * sha256, * foo;
		if(!TEST_ptr(nm))
			return 0;
		id = ossl_namemap_add_name(nm, 0, "SHA256");
		if(!TEST_int_ne(id, 0))
			return 0;
		if(!TEST_int_eq(ossl_namemap_add_name(nm, id, "foo"), id))
			return 0;
		sha256 = EVP_get_digestbyname("SHA256");
		if(!TEST_ptr(sha256))
			return 0;
		foo = EVP_get_digestbyname("foo");
		if(!TEST_ptr_eq(sha256, foo))
			return 0;

		return 1;
	}
	// 
	// Test that EVP_get_cipherbyname() will use the namemap when it can't find entries in the legacy method database.
	// 
	static int test_cipherbyname()
	{
		int id;
		OSSL_NAMEMAP * nm = ossl_namemap_stored(NULL);
		const EVP_CIPHER * aes128, * bar;
		if(!TEST_ptr(nm))
			return 0;
		id = ossl_namemap_add_name(nm, 0, "AES-128-CBC");
		if(!TEST_int_ne(id, 0))
			return 0;
		if(!TEST_int_eq(ossl_namemap_add_name(nm, id, "bar"), id))
			return 0;
		aes128 = EVP_get_cipherbyname("AES-128-CBC");
		if(!TEST_ptr(aes128))
			return 0;
		bar = EVP_get_cipherbyname("bar");
		if(!TEST_ptr_eq(aes128, bar))
			return 0;
		return 1;
	}
	// 
	// Test that EVP_CIPHER_is_a() responds appropriately, even for ciphers that are entirely legacy.
	// 
	static int test_cipher_is_a()
	{
		EVP_CIPHER * fetched = EVP_CIPHER_fetch(NULL, "AES-256-CCM", NULL);
		int rv = 1;
		if(!TEST_ptr(fetched))
			return 0;
		if(!TEST_true(EVP_CIPHER_is_a(fetched, "id-aes256-CCM")) || !TEST_false(EVP_CIPHER_is_a(fetched, "AES-128-GCM")))
			rv = 0;
		if(!TEST_true(EVP_CIPHER_is_a(EVP_aes_256_gcm(), "AES-256-GCM")) || !TEST_false(EVP_CIPHER_is_a(EVP_aes_256_gcm(), "AES-128-CCM")))
			rv = 0;
		EVP_CIPHER_free(fetched);
		return rv;
	}
	// 
	// Test that EVP_MD_is_a() responds appropriately, even for MDs that are entirely legacy.
	// 
	static int test_digest_is_a()
	{
		EVP_MD * fetched = EVP_MD_fetch(NULL, "SHA2-512", NULL);
		int rv = 1;
		if(!TEST_ptr(fetched))
			return 0;
		if(!TEST_true(EVP_MD_is_a(fetched, "SHA512")) || !TEST_false(EVP_MD_is_a(fetched, "SHA1")))
			rv = 0;
		if(!TEST_true(EVP_MD_is_a(EVP_sha256(), "SHA2-256")) || !TEST_false(EVP_MD_is_a(EVP_sha256(), "SHA3-256")))
			rv = 0;
		EVP_MD_free(fetched);
		return rv;
	}
};
//
//
//
//
// Time tests for the asn1 module
//
struct TestData_Asn1Time_testdata {
	char * data;        /* TIME string value */
	int type;           /* GENERALIZED OR UTC */
	int expected_type;  /* expected type after set/set_string_gmt */
	int check_result;   /* check result */
	time_t t;           /* expected time_t*/
	int cmp_result;     /* comparison to baseline result */
	int convert_result; /* conversion result */
};

static struct TestData_Asn1Time_testdata TestData_Asn1Time_tbl_testdata_pos[] = {
	{ "0",                 V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },/* Bad time */
	{ "ABCD",              V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "0ABCD",             V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "1-700101000000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "`9700101000000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "19700101000000Z",   V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         0,           0,  0, 0, },
	{ "A00101000000Z",     V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         0,           0,  0, 0, },
	{ "A9700101000000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "1A700101000000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "19A00101000000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "197A0101000000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "1970A101000000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "19700A01000000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "197001A1000000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "1970010A000000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "19700101A00000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "197001010A0000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "1970010100A000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "19700101000A00Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "197001010000A0Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "1970010100000AZ",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "700101000000X",     V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         0,           0,  0, 0, },
	{ "19700101000000X",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 0,           0,  0, 0, },
	{ "19700101000000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,           0, -1, 1, },/* Epoch begins */
	{ "700101000000Z",     V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         1,           0, -1, 1, },/* ditto */
	{ "20380119031407Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,  0x7FFFFFFF,  1, 1, },/* Max 32bit time_t */
	{ "380119031407Z",     V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         1,  0x7FFFFFFF,  1, 1, },
	{ "20371231235959Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,  2145916799,  1, 1, },/* Just before 2038 */
	{ "20371231235959Z",   V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         0,           0,  0, 1, },/* Bad UTC time */
	{ "371231235959Z",     V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         1,  2145916799,  1, 1, },
	{ "19701006121456Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,    24063296, -1, 1, },
	{ "701006121456Z",     V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         1,    24063296, -1, 1, },
	{ "19991231000000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,   946598400,  0, 1, },/* Match baseline */
	{ "199912310000Z",     V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,   946598400,  0, 1, },/* In various flavors */
	{ "991231000000Z",     V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         1,   946598400,  0, 1, },
	{ "9912310000Z",       V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         1,   946598400,  0, 1, },
	{ "9912310000+0000",   V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         1,   946598400,  0, 1, },
	{ "199912310000+0000", V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,   946598400,  0, 1, },
	{ "9912310000-0000",   V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         1,   946598400,  0, 1, },
	{ "199912310000-0000", V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,   946598400,  0, 1, },
	{ "199912310100+0100", V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,   946598400,  0, 1, },
	{ "199912302300-0100", V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,   946598400,  0, 1, },
	{ "199912302300-A000", V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         0,   946598400,  0, 1, },
	{ "199912302300-0A00", V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         0,   946598400,  0, 1, },
	{ "9912310100+0100",   V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         1,   946598400,  0, 1, },
	{ "9912302300-0100",   V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         1,   946598400,  0, 1, },
};

// ASSUMES SIGNED TIME_T 
static struct TestData_Asn1Time_testdata TestData_Asn1Time_tbl_testdata_neg[] = {
	{ "19011213204552Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 1,     INT_MIN, -1, 0, },
	{ "691006121456Z",     V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         1,    -7472704, -1, 1, },
	{ "19691006121456Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,    -7472704, -1, 1, },
};

// explicit casts to time_t short warnings on systems with 32-bit time_t 
static struct TestData_Asn1Time_testdata TestData_Asn1Time_tbl_testdata_pos_64bit[] = {
	{ "20380119031408Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,  (time_t)0x80000000,  1, 1, },
	{ "20380119031409Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_UTCTIME,         1,  (time_t)0x80000001,  1, 1, },
	{ "380119031408Z",     V_ASN1_UTCTIME,         V_ASN1_UTCTIME,         1,  (time_t)0x80000000,  1, 1, },
	{ "20500101120000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 1,  (time_t)0x967b1ec0,  1, 0, },
};

// ASSUMES SIGNED TIME_T 
static struct TestData_Asn1Time_testdata TestData_Asn1Time_tbl_testdata_neg_64bit[] = {
	{ "19011213204551Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 1, (time_t)-2147483649LL, -1, 0, },
	{ "19000101120000Z",   V_ASN1_GENERALIZEDTIME, V_ASN1_GENERALIZEDTIME, 1, (time_t)-2208945600LL, -1, 0, },
};

// A baseline time to compare to 
static ASN1_TIME TestData_Asn1Time_gtime = {
	15,
	V_ASN1_GENERALIZEDTIME,
	(uchar *)"19991231000000Z",
	0
};
static time_t TestData_Asn1Time_gtime_t = 946598400;

struct TestData_Asn1Time_compare_testdata {
	ASN1_TIME t1;
	ASN1_TIME t2;
	int result;
};

static uchar TestData_Asn1Time_TODAY_GEN_STR[] = "20170825000000Z";
static uchar TestData_Asn1Time_TOMORROW_GEN_STR[] = "20170826000000Z";
static uchar TestData_Asn1Time_TODAY_UTC_STR[] = "170825000000Z";
static uchar TestData_Asn1Time_TOMORROW_UTC_STR[] = "170826000000Z";

#define TestData_Asn1Time_TODAY_GEN    { sizeof(TestData_Asn1Time_TODAY_GEN_STR)-1, V_ASN1_GENERALIZEDTIME, TestData_Asn1Time_TODAY_GEN_STR, 0 }
#define TestData_Asn1Time_TOMORROW_GEN { sizeof(TestData_Asn1Time_TOMORROW_GEN_STR)-1, V_ASN1_GENERALIZEDTIME, TestData_Asn1Time_TOMORROW_GEN_STR, 0 }
#define TestData_Asn1Time_TODAY_UTC    { sizeof(TestData_Asn1Time_TODAY_UTC_STR)-1, V_ASN1_UTCTIME, TestData_Asn1Time_TODAY_UTC_STR, 0 }
#define TestData_Asn1Time_TOMORROW_UTC { sizeof(TestData_Asn1Time_TOMORROW_UTC_STR)-1, V_ASN1_UTCTIME, TestData_Asn1Time_TOMORROW_UTC_STR, 0 }

static struct TestData_Asn1Time_compare_testdata TestData_Asn1Time_tbl_compare_testdata[] = {
	{ TestData_Asn1Time_TODAY_GEN,    TestData_Asn1Time_TODAY_GEN,     0 },
	{ TestData_Asn1Time_TODAY_GEN,    TestData_Asn1Time_TODAY_UTC,     0 },
	{ TestData_Asn1Time_TODAY_GEN,    TestData_Asn1Time_TOMORROW_GEN, -1 },
	{ TestData_Asn1Time_TODAY_GEN,    TestData_Asn1Time_TOMORROW_UTC, -1 },
	{ TestData_Asn1Time_TODAY_UTC,    TestData_Asn1Time_TODAY_GEN,     0 },
	{ TestData_Asn1Time_TODAY_UTC,    TestData_Asn1Time_TODAY_UTC,     0 },
	{ TestData_Asn1Time_TODAY_UTC,    TestData_Asn1Time_TOMORROW_GEN, -1 },
	{ TestData_Asn1Time_TODAY_UTC,    TestData_Asn1Time_TOMORROW_UTC, -1 },
	{ TestData_Asn1Time_TOMORROW_GEN, TestData_Asn1Time_TODAY_GEN,     1 },
	{ TestData_Asn1Time_TOMORROW_GEN, TestData_Asn1Time_TODAY_UTC,     1 },
	{ TestData_Asn1Time_TOMORROW_GEN, TestData_Asn1Time_TOMORROW_GEN,  0 },
	{ TestData_Asn1Time_TOMORROW_GEN, TestData_Asn1Time_TOMORROW_UTC,  0 },
	{ TestData_Asn1Time_TOMORROW_UTC, TestData_Asn1Time_TODAY_GEN,     1 },
	{ TestData_Asn1Time_TOMORROW_UTC, TestData_Asn1Time_TODAY_UTC,     1 },
	{ TestData_Asn1Time_TOMORROW_UTC, TestData_Asn1Time_TOMORROW_GEN,  0 },
	{ TestData_Asn1Time_TOMORROW_UTC, TestData_Asn1Time_TOMORROW_UTC,  0 }
};

class TestInnerBlock_Asn1Time {
public:
	static int test_table(struct TestData_Asn1Time_testdata * tbl, int idx)
	{
		int error = 0;
		ASN1_TIME atime;
		ASN1_TIME * ptime;
		struct TestData_Asn1Time_testdata * td = &tbl[idx];
		int day, sec;
		atime.data = (uchar *)td->data;
		atime.length = strlen((char*)atime.data);
		atime.type = td->type;
		atime.flags = 0;
		if(!TEST_int_eq(ASN1_TIME_check(&atime), td->check_result)) {
			TEST_info("ASN1_TIME_check(%s) unexpected result", atime.data);
			error = 1;
		}
		if(td->check_result == 0)
			return 1;

		if(!TEST_int_eq(ASN1_TIME_cmp_time_t(&atime, td->t), 0)) {
			TEST_info("ASN1_TIME_cmp_time_t(%s vs %ld) compare failed", atime.data, (long)td->t);
			error = 1;
		}
		if(!TEST_true(ASN1_TIME_diff(&day, &sec, &atime, &atime))) {
			TEST_info("ASN1_TIME_diff(%s) to self failed", atime.data);
			error = 1;
		}
		if(!TEST_int_eq(day, 0) || !TEST_int_eq(sec, 0)) {
			TEST_info("ASN1_TIME_diff(%s) to self not equal", atime.data);
			error = 1;
		}
		if(!TEST_true(ASN1_TIME_diff(&day, &sec, &TestData_Asn1Time_gtime, &atime))) {
			TEST_info("ASN1_TIME_diff(%s) to baseline failed", atime.data);
			error = 1;
		}
		else if(!((td->cmp_result == 0 && TEST_true((day == 0 && sec == 0))) ||
			(td->cmp_result == -1 && TEST_true((day < 0 || sec < 0))) ||
			(td->cmp_result == 1 && TEST_true((day > 0 || sec > 0))))) {
			TEST_info("ASN1_TIME_diff(%s) to baseline bad comparison", atime.data);
			error = 1;
		}
		if(!TEST_int_eq(ASN1_TIME_cmp_time_t(&atime, TestData_Asn1Time_gtime_t), td->cmp_result)) {
			TEST_info("ASN1_TIME_cmp_time_t(%s) to baseline bad comparison", atime.data);
			error = 1;
		}
		ptime = ASN1_TIME_set(NULL, td->t);
		if(!TEST_ptr(ptime)) {
			TEST_info("ASN1_TIME_set(%ld) failed", (long)td->t);
			error = 1;
		}
		else {
			int local_error = 0;
			if(!TEST_int_eq(ASN1_TIME_cmp_time_t(ptime, td->t), 0)) {
				TEST_info("ASN1_TIME_set(%ld) compare failed (%s->%s)", (long)td->t, td->data, ptime->data);
				local_error = error = 1;
			}
			if(!TEST_int_eq(ptime->type, td->expected_type)) {
				TEST_info("ASN1_TIME_set(%ld) unexpected type", (long)td->t);
				local_error = error = 1;
			}
			if(local_error)
				TEST_info("ASN1_TIME_set() = %*s", ptime->length, ptime->data);
			ASN1_TIME_free(ptime);
		}
		ptime = ASN1_TIME_new();
		if(!TEST_ptr(ptime)) {
			TEST_info("ASN1_TIME_new() failed");
			error = 1;
		}
		else {
			int local_error = 0;
			if(!TEST_int_eq(ASN1_TIME_set_string(ptime, td->data), td->check_result)) {
				TEST_info("ASN1_TIME_set_string_gmt(%s) failed", td->data);
				local_error = error = 1;
			}
			if(!TEST_int_eq(ASN1_TIME_normalize(ptime), td->check_result)) {
				TEST_info("ASN1_TIME_normalize(%s) failed", td->data);
				local_error = error = 1;
			}
			if(!TEST_int_eq(ptime->type, td->expected_type)) {
				TEST_info("ASN1_TIME_set_string_gmt(%s) unexpected type", td->data);
				local_error = error = 1;
			}
			day = sec = 0;
			if(!TEST_true(ASN1_TIME_diff(&day, &sec, ptime, &atime)) || !TEST_int_eq(day, 0) || !TEST_int_eq(sec, 0)) {
				TEST_info("ASN1_TIME_diff(day=%d, sec=%d, %s) after ASN1_TIME_set_string_gmt() failed", day, sec, td->data);
				local_error = error = 1;
			}
			if(!TEST_int_eq(ASN1_TIME_cmp_time_t(ptime, TestData_Asn1Time_gtime_t), td->cmp_result)) {
				TEST_info("ASN1_TIME_cmp_time_t(%s) after ASN1_TIME_set_string_gnt() to baseline bad comparison", td->data);
				local_error = error = 1;
			}
			if(local_error)
				TEST_info("ASN1_TIME_set_string_gmt() = %*s", ptime->length, ptime->data);
			ASN1_TIME_free(ptime);
		}
		ptime = ASN1_TIME_new();
		if(!TEST_ptr(ptime)) {
			TEST_info("ASN1_TIME_new() failed");
			error = 1;
		}
		else {
			int local_error = 0;
			if(!TEST_int_eq(ASN1_TIME_set_string(ptime, td->data), td->check_result)) {
				TEST_info("ASN1_TIME_set_string(%s) failed", td->data);
				local_error = error = 1;
			}
			day = sec = 0;
			if(!TEST_true(ASN1_TIME_diff(&day, &sec, ptime, &atime)) || !TEST_int_eq(day, 0) || !TEST_int_eq(sec, 0)) {
				TEST_info("ASN1_TIME_diff(day=%d, sec=%d, %s) after ASN1_TIME_set_string() failed", day, sec, td->data);
				local_error = error = 1;
			}
			if(!TEST_int_eq(ASN1_TIME_cmp_time_t(ptime, TestData_Asn1Time_gtime_t), td->cmp_result)) {
				TEST_info("ASN1_TIME_cmp_time_t(%s) after ASN1_TIME_set_string() to baseline bad comparison", td->data);
				local_error = error = 1;
			}
			if(local_error)
				TEST_info("ASN1_TIME_set_string() = %*s", ptime->length, ptime->data);
			ASN1_TIME_free(ptime);
		}

		if(td->type == V_ASN1_UTCTIME) {
			ptime = ASN1_TIME_to_generalizedtime(&atime, NULL);
			if(td->convert_result == 1 && !TEST_ptr(ptime)) {
				TEST_info("ASN1_TIME_to_generalizedtime(%s) failed", atime.data);
				error = 1;
			}
			else if(td->convert_result == 0 && !TEST_ptr_null(ptime)) {
				TEST_info("ASN1_TIME_to_generalizedtime(%s) should have failed", atime.data);
				error = 1;
			}
			if(ptime && !TEST_int_eq(ASN1_TIME_cmp_time_t(ptime, td->t), 0)) {
				TEST_info("ASN1_TIME_to_generalizedtime(%s->%s) bad result", atime.data, ptime->data);
				error = 1;
			}
			ASN1_TIME_free(ptime);
		}
		/* else cannot simply convert GENERALIZEDTIME to UTCTIME */
		if(error)
			TEST_error("atime=%s", atime.data);
		return !error;
	}

	static int test_table_pos(int idx) { return test_table(TestData_Asn1Time_tbl_testdata_pos, idx); }
	static int test_table_neg(int idx) { return test_table(TestData_Asn1Time_tbl_testdata_neg, idx); }
	static int test_table_pos_64bit(int idx) { return test_table(TestData_Asn1Time_tbl_testdata_pos_64bit, idx); }
	static int test_table_neg_64bit(int idx) { return test_table(TestData_Asn1Time_tbl_testdata_neg_64bit, idx); }
	static int test_table_compare(int idx)
	{
		struct TestData_Asn1Time_compare_testdata * td = &TestData_Asn1Time_tbl_compare_testdata[idx];
		return TEST_int_eq(ASN1_TIME_compare(&td->t1, &td->t2), td->result);
	}
	static int test_time_dup()
	{
		int ret = 0;
		ASN1_TIME * asn1_time_dup = NULL;
		ASN1_TIME * asn1_gentime = NULL;
		ASN1_TIME * asn1_time = ASN1_TIME_adj(NULL, time(NULL), 0, 0);
		if(asn1_time == NULL) {
			TEST_info("Internal error.");
			goto err;
		}
		asn1_gentime = ASN1_TIME_to_generalizedtime(asn1_time, NULL);
		if(asn1_gentime == NULL) {
			TEST_info("Internal error.");
			goto err;
		}
		asn1_time_dup = ASN1_TIME_dup(asn1_time);
		if(!TEST_ptr_ne(asn1_time_dup, NULL)) {
			TEST_info("ASN1_TIME_dup() failed.");
			goto err;
		}
		if(!TEST_int_eq(ASN1_TIME_compare(asn1_time, asn1_time_dup), 0)) {
			TEST_info("ASN1_TIME_dup() duplicated non-identical value.");
			goto err;
		}
		ASN1_STRING_free(asn1_time_dup);
		asn1_time_dup = ASN1_UTCTIME_dup(asn1_time);
		if(!TEST_ptr_ne(asn1_time_dup, NULL)) {
			TEST_info("ASN1_UTCTIME_dup() failed.");
			goto err;
		}
		if(!TEST_int_eq(ASN1_TIME_compare(asn1_time, asn1_time_dup), 0)) {
			TEST_info("ASN1_UTCTIME_dup() duplicated non-identical UTCTIME value.");
			goto err;
		}
		ASN1_STRING_free(asn1_time_dup);
		asn1_time_dup = ASN1_GENERALIZEDTIME_dup(asn1_gentime);
		if(!TEST_ptr_ne(asn1_time_dup, NULL)) {
			TEST_info("ASN1_GENERALIZEDTIME_dup() failed.");
			goto err;
		}
		if(!TEST_int_eq(ASN1_TIME_compare(asn1_gentime, asn1_time_dup), 0)) {
			TEST_info("ASN1_GENERALIZEDTIME_dup() dup'ed non-identical value.");
			goto err;
		}
		ret = 1;
	err:
		ASN1_STRING_free(asn1_time);
		ASN1_STRING_free(asn1_gentime);
		ASN1_STRING_free(asn1_time_dup);
		return ret;
	}
};
//
// BN
// 
// Things in boring, not in openssl.
// 
#define TestData_BN_HAVE_BN_SQRT 0

typedef struct filetest_st {
	const char * name;
	int (*func)(STANZA *s);
} FILETEST;

typedef struct mpitest_st {
	const char * base10;
	const char * mpi;
	size_t mpi_len;
} MPITEST;

static const int TestData_BN_NUM0 = 100;           /* number of tests */
static const int TestData_BN_NUM1 = 50;            /* additional tests for some functions */
static BN_CTX * TestData_BN_ctx;
// 
// Polynomial coefficients used in GFM tests.
// 
#ifndef OPENSSL_NO_EC2M
	static int TestData_BN_p0[] = { 163, 7, 6, 3, 0, -1 };
	static int TestData_BN_p1[] = { 193, 15, 0, -1 };
#endif
static int TestData_BN_primes[] = { 2, 3, 5, 7, 17863 };
static int TestData_BN_not_primes[] = { -1, 0, 1, 4 };

static struct {
	int n;
	int divisor;
	int result;
	int remainder;
} TestData_BN_signed_mod_tests[] = {
	{  10,   3,   3,   1 },
	{ -10,   3,  -3,  -1 },
	{  10,  -3,  -3,   1 },
	{ -10,  -3,   3,  -1 },
};

static const char * TestData_BN_bn1strings[] = {
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000000000FFFFFFFF00",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"00000000000000000000000000000000000000000000000000FFFFFFFFFFFFFF",
	NULL
};

static const char * TestData_BN_bn2strings[] = {
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000000000FFFFFFFF0000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"0000000000000000000000000000000000000000000000000000000000000000",
	"000000000000000000000000000000000000000000FFFFFFFFFFFFFF00000000",
	NULL
};

static const MPITEST TestData_BN_kMPITests[] = {
	{"0", "\x00\x00\x00\x00", 4},
	{"1", "\x00\x00\x00\x01\x01", 5},
	{"-1", "\x00\x00\x00\x01\x81", 5},
	{"128", "\x00\x00\x00\x02\x00\x80", 6},
	{"256", "\x00\x00\x00\x02\x01\x00", 6},
	{"-256", "\x00\x00\x00\x02\x81\x00", 6},
};

typedef struct mod_exp_test_st {
	const char * base;
	const char * exp;
	const char * mod;
	const char * res;
} MOD_EXP_TEST;

static const MOD_EXP_TEST TestData_BN_ModExpTests[] = {
	// original test vectors for rsaz_512_sqr bug, by OSS-Fuzz
	{
		"1166180238001879113042182292626169621106255558914000595999312084"
		"4627946820899490684928760491249738643524880720584249698100907201"
		"002086675047927600340800371",
		"8000000000000000000000000000000000000000000000000000000000000000"
		"0000000000000000000000000000000000000000000000000000000000000000"
		"00000000",
		"1340780792684523720980737645613191762604395855615117867483316354"
		"3294276330515137663421134775482798690129946803802212663956180562"
		"088664022929883876655300863",
		"8243904058268085430037326628480645845409758077568738532059032482"
		"8294114415890603594730158120426756266457928475330450251339773498"
		"26758407619521544102068438"
	},
	{
		"4974270041410803822078866696159586946995877618987010219312844726"
		"0284386121835740784990869050050504348861513337232530490826340663"
		"197278031692737429054",
		"4974270041410803822078866696159586946995877428188754995041148539"
		"1663243362592271353668158565195557417149981094324650322556843202"
		"946445882670777892608",
		"1340780716511420227215592830971452482815377482627251725537099028"
		"4429769497230131760206012644403029349547320953206103351725462999"
		"947509743623340557059752191",
		"5296244594780707015616522701706118082963369547253192207884519362"
		"1767869984947542695665420219028522815539559194793619684334900442"
		"49304558011362360473525933"
	},
	/* test vectors for rsaz_512_srq bug, with rcx/rbx=1 */
	{ /* between first and second iteration */
		"5148719036160389201525610950887605325980251964889646556085286545"
		"3931548809178823413169359635978762036512397113080988070677858033"
		"36463909753993540214027190",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042158",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042159",
		"1"
	},
	{ /* between second and third iteration */
		"8908340854353752577419678771330460827942371434853054158622636544"
		"8151360109722890949471912566649465436296659601091730745087014189"
		"2672764191218875181826063",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042158",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042159",
		"1"
	},
	{ /* between third and fourth iteration */
		"3427446396505596330634350984901719674479522569002785244080234738"
		"4288743635435746136297299366444548736533053717416735379073185344"
		"26985272974404612945608761",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042158",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042159",
		"1"
	},
	{ /* between fourth and fifth iteration */
		"3472743044917564564078857826111874560045331237315597383869652985"
		"6919870028890895988478351133601517365908445058405433832718206902"
		"4088133164805266956353542",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042158",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042159",
		"1"
	},
	{ /* between fifth and sixth iteration */
		"3608632990153469264412378349742339216742409743898601587274768025"
		"0110772032985643555192767717344946174122842255204082586753499651"
		"14483434992887431333675068",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042158",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042159",
		"1"
	},
	{ /* between sixth and seventh iteration */
		"8455374370234070242910508226941981520235709767260723212165264877"
		"8689064388017521524568434328264431772644802567028663962962025746"
		"9283458217850119569539086",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042158",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042159",
		"1"
	},
	{ /* between seventh and eighth iteration */
		"5155371529688532178421209781159131443543419764974688878527112131"
		"7446518205609427412336183157918981038066636807317733319323257603"
		"04416292040754017461076359",
		"1005585594745694782468051874865438459560952436544429503329267108"
		"2791323022555160232601405723625177570767523893639864538140315412"
		"108959927459825236754563832",
		"1005585594745694782468051874865438459560952436544429503329267108"
		"2791323022555160232601405723625177570767523893639864538140315412"
		"108959927459825236754563833",
		"1"
	},
	/* test vectors for rsaz_512_srq bug, with rcx/rbx=2 */
	{ /* between first and second iteration */
		"3155666506033786929967309937640790361084670559125912405342594979"
		"4345142818528956285490897841406338022378565972533508820577760065"
		"58494345853302083699912572",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042158",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042159",
		"1"
	},
	{ /* between second and third iteration */
		"3789819583801342198190405714582958759005991915505282362397087750"
		"4213544724644823098843135685133927198668818185338794377239590049"
		"41019388529192775771488319",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042158",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042159",
		"1"
	},
	{ /* between third and forth iteration */
		"4695752552040706867080542538786056470322165281761525158189220280"
		"4025547447667484759200742764246905647644662050122968912279199065"
		"48065034299166336940507214",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042158",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042159",
		"1"
	},
	{ /* between forth and fifth iteration */
		"2159140240970485794188159431017382878636879856244045329971239574"
		"8919691133560661162828034323196457386059819832804593989740268964"
		"74502911811812651475927076",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042158",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042159",
		"1"
	},
	{ /* between fifth and sixth iteration */
		"5239312332984325668414624633307915097111691815000872662334695514"
		"5436533521392362443557163429336808208137221322444780490437871903"
		"99972784701334569424519255",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042158",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042159",
		"1"
	},
	{ /* between sixth and seventh iteration */
		"1977953647322612860406858017869125467496941904523063466791308891"
		"1172796739058531929470539758361774569875505293428856181093904091"
		"33788264851714311303725089",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042158",
		"6703903964971298549787012499102923063739682910296196688861780721"
		"8608820150367734884009371490834517138450159290932430254268769414"
		"05973284973216824503042159",
		"1"
	},
	{ /* between seventh and eighth iteration */
		"6456987954117763835533395796948878140715006860263624787492985786"
		"8514630216966738305923915688821526449499763719943997120302368211"
		"04813318117996225041943964",
		"1340780792994259709957402499820584612747936582059239337772356144"
		"3721764030073546976801874298166903427690031858186486050853753882"
		"811946551499689575296532556",
		"1340780792994259709957402499820584612747936582059239337772356144"
		"3721764030073546976801874298166903427690031858186486050853753882"
		"811946551499689575296532557",
		"1"
	}
};

class TestInnerBlock_BN {
public:
	// 
	// Look for |key| in the stanza and return it or NULL if not found.
	// 
	static const char * findattr(STANZA * s, const char * key)
	{
		int i = s->numpairs;
		PAIR * pp = s->pairs;
		for(; --i >= 0; pp++)
			if(strcasecmp(pp->key, key) == 0)
				return pp->value;
		return NULL;
	}
	// 
	// Parse BIGNUM from sparse hex-strings, return |BN_hex2bn| result.
	// 
	static int parse_bigBN(BIGNUM ** out, const char * bn_strings[])
	{
		char * bigstring = glue_strings(bn_strings, NULL);
		int ret = BN_hex2bn(out, bigstring);
		OPENSSL_free(bigstring);
		return ret;
	}
	// 
	// Parse BIGNUM, return number of bytes parsed.
	// 
	static int parseBN(BIGNUM ** out, const char * in)
	{
		*out = NULL;
		return BN_hex2bn(out, in);
	}

	static int parsedecBN(BIGNUM ** out, const char * in)
	{
		*out = NULL;
		return BN_dec2bn(out, in);
	}

	static BIGNUM * getBN(STANZA * s, const char * attribute)
	{
		const char * hex;
		BIGNUM * ret = NULL;
		if((hex = findattr(s, attribute)) == NULL) {
			TEST_error("%s:%d: Can't find %s", s->test_file, s->start, attribute);
			return NULL;
		}
		if(parseBN(&ret, hex) != (int)strlen(hex)) {
			TEST_error("Could not decode '%s'", hex);
			return NULL;
		}
		return ret;
	}

	static int getint(STANZA * s, int * out, const char * attribute)
	{
		BIGNUM * ret;
		BN_ULONG word;
		int st = 0;
		if(!TEST_ptr(ret = getBN(s, attribute)) || !TEST_ulong_le(word = BN_get_word(ret), INT_MAX))
			goto err;
		*out = (int)word;
		st = 1;
	err:
		BN_free(ret);
		return st;
	}

	static int equalBN(const char * op, const BIGNUM * expected, const BIGNUM * actual)
	{
		if(BN_cmp(expected, actual) == 0)
			return 1;
		TEST_error("unexpected %s value", op);
		TEST_BN_eq(expected, actual);
		return 0;
	}
	// 
	// Return a "random" flag for if a BN should be negated.
	// 
	static int rand_neg()
	{
		static uint neg = 0;
		static int sign[8] = { 0, 0, 0, 1, 1, 0, 1, 1 };
		return sign[(neg++) % 8];
	}
	static int test_swap()
	{
		BIGNUM * a = NULL, * b = NULL, * c = NULL, * d = NULL;
		int top, cond, st = 0;
		if(!TEST_ptr(a = BN_new()) || !TEST_ptr(b = BN_new()) || !TEST_ptr(c = BN_new()) || !TEST_ptr(d = BN_new()))
			goto err;
		if(!(TEST_true(BN_bntest_rand(a, 1024, 1, 0)) && TEST_true(BN_bntest_rand(b, 1024, 1, 0)) && TEST_ptr(BN_copy(c, a)) && TEST_ptr(BN_copy(d, b))))
			goto err;
		top = BN_num_bits(a) / BN_BITS2;
		// regular swap 
		BN_swap(a, b);
		if(!equalBN("swap", a, d) || !equalBN("swap", b, c))
			goto err;
		// conditional swap: true 
		cond = 1;
		BN_consttime_swap(cond, a, b, top);
		if(!equalBN("cswap true", a, c) || !equalBN("cswap true", b, d))
			goto err;
		// conditional swap: false 
		cond = 0;
		BN_consttime_swap(cond, a, b, top);
		if(!equalBN("cswap false", a, c) || !equalBN("cswap false", b, d))
			goto err;
		/* same tests but checking flag swap */
		BN_set_flags(a, BN_FLG_CONSTTIME);
		BN_swap(a, b);
		if(!equalBN("swap, flags", a, d) || !equalBN("swap, flags", b, c) || !TEST_true(BN_get_flags(b, BN_FLG_CONSTTIME)) || !TEST_false(BN_get_flags(a, BN_FLG_CONSTTIME)))
			goto err;
		cond = 1;
		BN_consttime_swap(cond, a, b, top);
		if(!equalBN("cswap true, flags", a, c) || !equalBN("cswap true, flags", b, d) || 
			!TEST_true(BN_get_flags(a, BN_FLG_CONSTTIME)) || !TEST_false(BN_get_flags(b, BN_FLG_CONSTTIME)))
			goto err;
		cond = 0;
		BN_consttime_swap(cond, a, b, top);
		if(!equalBN("cswap false, flags", a, c) || !equalBN("cswap false, flags", b, d) || 
			!TEST_true(BN_get_flags(a, BN_FLG_CONSTTIME)) || !TEST_false(BN_get_flags(b, BN_FLG_CONSTTIME)))
			goto err;
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(c);
		BN_free(d);
		return st;
	}

	static int test_sub()
	{
		BIGNUM * a = NULL, * b = NULL, * c = NULL;
		int i, st = 0;
		if(!TEST_ptr(a = BN_new()) || !TEST_ptr(b = BN_new()) || !TEST_ptr(c = BN_new()))
			goto err;
		for(i = 0; i < TestData_BN_NUM0 + TestData_BN_NUM1; i++) {
			if(i < TestData_BN_NUM1) {
				if(!(TEST_true(BN_bntest_rand(a, 512, 0, 0))) && TEST_ptr(BN_copy(b, a)) && TEST_int_ne(BN_set_bit(a, i), 0) && TEST_true(BN_add_word(b, i)))
					goto err;
			}
			else {
				if(!TEST_true(BN_bntest_rand(b, 400 + i - TestData_BN_NUM1, 0, 0)))
					goto err;
				BN_set_negative(a, rand_neg());
				BN_set_negative(b, rand_neg());
			}
			if(!(TEST_true(BN_sub(c, a, b)) && TEST_true(BN_add(c, c, b)) && TEST_true(BN_sub(c, c, a)) && TEST_BN_eq_zero(c)))
				goto err;
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(c);
		return st;
	}
	static int test_div_recip()
	{
		BIGNUM * a = NULL, * b = NULL, * c = NULL, * d = NULL, * e = NULL;
		BN_RECP_CTX * recp = NULL;
		int st = 0, i;
		if(!TEST_ptr(a = BN_new()) || !TEST_ptr(b = BN_new()) || !TEST_ptr(c = BN_new()) || !TEST_ptr(d = BN_new()) || !TEST_ptr(e = BN_new()) || !TEST_ptr(recp = BN_RECP_CTX_new()))
			goto err;
		for(i = 0; i < TestData_BN_NUM0 + TestData_BN_NUM1; i++) {
			if(i < TestData_BN_NUM1) {
				if(!(TEST_true(BN_bntest_rand(a, 400, 0, 0)) && TEST_ptr(BN_copy(b, a)) && TEST_true(BN_lshift(a, a, i)) && TEST_true(BN_add_word(a, i))))
					goto err;
			}
			else {
				if(!(TEST_true(BN_bntest_rand(b, 50 + 3 * (i - TestData_BN_NUM1), 0, 0))))
					goto err;
			}
			BN_set_negative(a, rand_neg());
			BN_set_negative(b, rand_neg());
			if(!(TEST_true(BN_RECP_CTX_set(recp, b, TestData_BN_ctx))
				&& TEST_true(BN_div_recp(d, c, a, recp, TestData_BN_ctx))
				&& TEST_true(BN_mul(e, d, b, TestData_BN_ctx))
				&& TEST_true(BN_add(d, e, c))
				&& TEST_true(BN_sub(d, d, a))
				&& TEST_BN_eq_zero(d)))
				goto err;
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(c);
		BN_free(d);
		BN_free(e);
		BN_RECP_CTX_free(recp);
		return st;
	}
	static BIGNUM * set_signed_bn(int value)
	{
		BIGNUM * bn = BN_new();
		if(bn == NULL)
			return NULL;
		if(!BN_set_word(bn, value < 0 ? -value : value)) {
			BN_free(bn);
			return NULL;
		}
		BN_set_negative(bn, value < 0);
		return bn;
	}
	static int test_signed_mod_replace_ab(int n)
	{
		BIGNUM * a = NULL, * b = NULL, * c = NULL, * d = NULL;
		int st = 0;
		if(!TEST_ptr(a = set_signed_bn(TestData_BN_signed_mod_tests[n].n))
			|| !TEST_ptr(b = set_signed_bn(TestData_BN_signed_mod_tests[n].divisor))
			|| !TEST_ptr(c = set_signed_bn(TestData_BN_signed_mod_tests[n].result))
			|| !TEST_ptr(d = set_signed_bn(TestData_BN_signed_mod_tests[n].remainder)))
			goto err;
		if(TEST_true(BN_div(a, b, a, b, TestData_BN_ctx)) && TEST_BN_eq(a, c) && TEST_BN_eq(b, d))
			st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(c);
		BN_free(d);
		return st;
	}

	static int test_signed_mod_replace_ba(int n)
	{
		BIGNUM * a = NULL, * b = NULL, * c = NULL, * d = NULL;
		int st = 0;
		if(!TEST_ptr(a = set_signed_bn(TestData_BN_signed_mod_tests[n].n))
			|| !TEST_ptr(b = set_signed_bn(TestData_BN_signed_mod_tests[n].divisor))
			|| !TEST_ptr(c = set_signed_bn(TestData_BN_signed_mod_tests[n].result))
			|| !TEST_ptr(d = set_signed_bn(TestData_BN_signed_mod_tests[n].remainder)))
			goto err;
		if(TEST_true(BN_div(b, a, a, b, TestData_BN_ctx)) && TEST_BN_eq(b, c) && TEST_BN_eq(a, d))
			st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(c);
		BN_free(d);
		return st;
	}
	static int test_mod()
	{
		BIGNUM * a = NULL, * b = NULL, * c = NULL, * d = NULL, * e = NULL;
		int st = 0, i;
		if(!TEST_ptr(a = BN_new())
			|| !TEST_ptr(b = BN_new())
			|| !TEST_ptr(c = BN_new())
			|| !TEST_ptr(d = BN_new())
			|| !TEST_ptr(e = BN_new()))
			goto err;
		if(!(TEST_true(BN_bntest_rand(a, 1024, 0, 0))))
			goto err;
		for(i = 0; i < TestData_BN_NUM0; i++) {
			if(!(TEST_true(BN_bntest_rand(b, 450 + i * 10, 0, 0))))
				goto err;
			BN_set_negative(a, rand_neg());
			BN_set_negative(b, rand_neg());
			if(!(TEST_true(BN_mod(c, a, b, TestData_BN_ctx))
				&& TEST_true(BN_div(d, e, a, b, TestData_BN_ctx))
				&& TEST_BN_eq(e, c)
				&& TEST_true(BN_mul(c, d, b, TestData_BN_ctx))
				&& TEST_true(BN_add(d, c, e))
				&& TEST_BN_eq(d, a)))
				goto err;
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(c);
		BN_free(d);
		BN_free(e);
		return st;
	}
	// 
	// Test constant-time modular exponentiation with 1024-bit inputs, which on
	// x86_64 cause a different code branch to be taken.
	// 
	static int test_modexp_mont5()
	{
		BIGNUM * a = NULL, * p = NULL, * m = NULL, * d = NULL, * e = NULL;
		BIGNUM * b = NULL, * n = NULL, * c = NULL;
		BN_MONT_CTX * mont = NULL;
		int st = 0;
		if(!TEST_ptr(a = BN_new())
			|| !TEST_ptr(p = BN_new())
			|| !TEST_ptr(m = BN_new())
			|| !TEST_ptr(d = BN_new())
			|| !TEST_ptr(e = BN_new())
			|| !TEST_ptr(b = BN_new())
			|| !TEST_ptr(n = BN_new())
			|| !TEST_ptr(c = BN_new())
			|| !TEST_ptr(mont = BN_MONT_CTX_new()))
			goto err;
		/* must be odd for montgomery */
		if(!(TEST_true(BN_bntest_rand(m, 1024, 0, 1)) /* Zero exponent */ && TEST_true(BN_bntest_rand(a, 1024, 0, 0))))
			goto err;
		BN_zero(p);
		if(!TEST_true(BN_mod_exp_mont_consttime(d, a, p, m, TestData_BN_ctx, NULL)))
			goto err;
		if(!TEST_BN_eq_one(d))
			goto err;
		/* Regression test for carry bug in mulx4x_mont */
		if(!(TEST_true(BN_hex2bn(&a,
			"7878787878787878787878787878787878787878787878787878787878787878"
			"7878787878787878787878787878787878787878787878787878787878787878"
			"7878787878787878787878787878787878787878787878787878787878787878"
			"7878787878787878787878787878787878787878787878787878787878787878"))
			&& TEST_true(BN_hex2bn(&b,
			"095D72C08C097BA488C5E439C655A192EAFB6380073D8C2664668EDDB4060744"
			"E16E57FB4EDB9AE10A0CEFCDC28A894F689A128379DB279D48A2E20849D68593"
			"9B7803BCF46CEBF5C533FB0DD35B080593DE5472E3FE5DB951B8BFF9B4CB8F03"
			"9CC638A5EE8CDD703719F8000E6A9F63BEED5F2FCD52FF293EA05A251BB4AB81"))
			&& TEST_true(BN_hex2bn(&n,
			"D78AF684E71DB0C39CFF4E64FB9DB567132CB9C50CC98009FEB820B26F2DED9B"
			"91B9B5E2B83AE0AE4EB4E0523CA726BFBE969B89FD754F674CE99118C3F2D1C5"
			"D81FDC7C54E02B60262B241D53C040E99E45826ECA37A804668E690E1AFC1CA4"
			"2C9A15D84D4954425F0B7642FC0BD9D7B24E2618D2DCC9B729D944BADACFDDAF"))))
			goto err;
		if(!(TEST_true(BN_MONT_CTX_set(mont, n, TestData_BN_ctx))
			&& TEST_true(BN_mod_mul_montgomery(c, a, b, mont, TestData_BN_ctx))
			&& TEST_true(BN_mod_mul_montgomery(d, b, a, mont, TestData_BN_ctx))
			&& TEST_BN_eq(c, d)))
			goto err;
		// Regression test for carry bug in sqr[x]8x_mont 
		if(!(TEST_true(parse_bigBN(&n, TestData_BN_bn1strings)) && TEST_true(parse_bigBN(&a, TestData_BN_bn2strings))))
			goto err;
		BN_free(b);
		if(!(TEST_ptr(b = BN_dup(a)) && TEST_true(BN_MONT_CTX_set(mont, n, TestData_BN_ctx))
			&& TEST_true(BN_mod_mul_montgomery(c, a, a, mont, TestData_BN_ctx))
			&& TEST_true(BN_mod_mul_montgomery(d, a, b, mont, TestData_BN_ctx))
			&& TEST_BN_eq(c, d)))
			goto err;
		// Regression test for carry bug in bn_sqrx8x_internal
		{
			static const char * ahex[] = {
				"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
				"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
				"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
				"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
				"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF8FFEADBCFC4DAE7FFF908E92820306B",
				"9544D954000000006C0000000000000000000000000000000000000000000000",
				"00000000000000000000FF030202FFFFF8FFEBDBCFC4DAE7FFF908E92820306B",
				"9544D954000000006C000000FF0302030000000000FFFFFFFFFFFFFFFFFFFFFF",
				"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF01FC00FF02FFFFFFFF",
				"00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FCFD",
				"FCFFFFFFFFFF000000000000000000FF0302030000000000FFFFFFFFFFFFFFFF",
				"FF00FCFDFDFF030202FF00000000FFFFFFFFFFFFFFFFFF00FCFDFCFFFFFFFFFF",
				NULL
			};
			static const char * nhex[] = {
				"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
				"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
				"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
				"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
				"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF8F8F8F8000000",
				"00000010000000006C0000000000000000000000000000000000000000000000",
				"00000000000000000000000000000000000000FFFFFFFFFFFFF8F8F8F8000000",
				"00000010000000006C000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF",
				"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
				"00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
				"FFFFFFFFFFFF000000000000000000000000000000000000FFFFFFFFFFFFFFFF",
				"FFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
				NULL
			};

			if(!(TEST_true(parse_bigBN(&a, ahex))
				&& TEST_true(parse_bigBN(&n, nhex))))
				goto err;
		}
		BN_free(b);
		if(!(TEST_ptr(b = BN_dup(a)) && TEST_true(BN_MONT_CTX_set(mont, n, TestData_BN_ctx))))
			goto err;
		if(!TEST_true(BN_mod_mul_montgomery(c, a, a, mont, TestData_BN_ctx)) || !TEST_true(BN_mod_mul_montgomery(d, a, b, mont, TestData_BN_ctx)) || !TEST_BN_eq(c, d))
			goto err;
		// Regression test for bug in BN_from_montgomery_word 
		if(!(TEST_true(BN_hex2bn(&a,
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"))
			&& TEST_true(BN_hex2bn(&n,
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"))
			&& TEST_true(BN_MONT_CTX_set(mont, n, TestData_BN_ctx))
			&& TEST_false(BN_mod_mul_montgomery(d, a, a, mont, TestData_BN_ctx))))
			goto err;
		// Regression test for bug in rsaz_1024_mul_avx2 
		if(!(TEST_true(BN_hex2bn(&a,
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF2020202020DF"))
			&& TEST_true(BN_hex2bn(&b,
			"2020202020202020202020202020202020202020202020202020202020202020"
			"2020202020202020202020202020202020202020202020202020202020202020"
			"20202020202020FF202020202020202020202020202020202020202020202020"
			"2020202020202020202020202020202020202020202020202020202020202020"))
			&& TEST_true(BN_hex2bn(&n,
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF2020202020FF"))
			&& TEST_true(BN_MONT_CTX_set(mont, n, TestData_BN_ctx))
			&& TEST_true(BN_mod_exp_mont_consttime(c, a, b, n, TestData_BN_ctx, mont))
			&& TEST_true(BN_mod_exp_mont(d, a, b, n, TestData_BN_ctx, mont))
			&& TEST_BN_eq(c, d)))
			goto err;
		//
		// rsaz_1024_mul_avx2 expects fully-reduced inputs.
		// BN_mod_exp_mont_consttime should reduce the input first.
		//
		if(!(TEST_true(BN_hex2bn(&a,
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF2020202020DF"))
			&& TEST_true(BN_hex2bn(&b,
			"1FA53F26F8811C58BE0357897AA5E165693230BC9DF5F01DFA6A2D59229EC69D"
			"9DE6A89C36E3B6957B22D6FAAD5A3C73AE587B710DBE92E83D3A9A3339A085CB"
			"B58F508CA4F837924BB52CC1698B7FDC2FD74362456A595A5B58E38E38E38E38"
			"E38E38E38E38E38E38E38E38E38E38E38E38E38E38E38E38E38E38E38E38E38E"))
			&& TEST_true(BN_hex2bn(&n,
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF2020202020DF"))
			&& TEST_true(BN_MONT_CTX_set(mont, n, TestData_BN_ctx))
			&& TEST_true(BN_mod_exp_mont_consttime(c, a, b, n, TestData_BN_ctx, mont))))
			goto err;
		BN_zero(d);
		if(!TEST_BN_eq(c, d))
			goto err;
		//
		// Regression test for overflow bug in bn_sqr_comba4/8 for
		// mips-linux-gnu and mipsel-linux-gnu 32bit targets.
		//
		{
			static const char * ehex[] = {
				"95564994a96c45954227b845a1e99cb939d5a1da99ee91acc962396ae999a9ee",
				"38603790448f2f7694c242a875f0cad0aae658eba085f312d2febbbd128dd2b5",
				"8f7d1149f03724215d704344d0d62c587ae3c5939cba4b9b5f3dc5e8e911ef9a",
				"5ce1a5a749a4989d0d8368f6e1f8cdf3a362a6c97fb02047ff152b480a4ad985",
				"2d45efdf0770542992afca6a0590d52930434bba96017afbc9f99e112950a8b1",
				"a359473ec376f329bdae6a19f503be6d4be7393c4e43468831234e27e3838680",
				"b949390d2e416a3f9759e5349ab4c253f6f29f819a6fe4cbfd27ada34903300e",
				"da021f62839f5878a36f1bc3085375b00fd5fa3e68d316c0fdace87a97558465",
				NULL
			};
			static const char * phex[] = {
				"f95dc0f980fbd22e90caa5a387cc4a369f3f830d50dd321c40db8c09a7e1a241",
				"a536e096622d3280c0c1ba849c1f4a79bf490f60006d081e8cf69960189f0d31",
				"2cd9e17073a3fba7881b21474a13b334116cb2f5dbf3189a6de3515d0840f053",
				"c776d3982d391b6d04d642dda5cc6d1640174c09875addb70595658f89efb439",
				"dc6fbd55f903aadd307982d3f659207f265e1ec6271b274521b7a5e28e8fd7a5",
				"5df089292820477802a43cf5b6b94e999e8c9944ddebb0d0e95a60f88cb7e813",
				"ba110d20e1024774107dd02949031864923b3cb8c3f7250d6d1287b0a40db6a4",
				"7bd5a469518eb65aa207ddc47d8c6e5fc8e0c105be8fc1d4b57b2e27540471d5",
				NULL
			};
			static const char * mhex[] = {
				"fef15d5ce4625f1bccfbba49fc8439c72bf8202af039a2259678941b60bb4a8f",
				"2987e965d58fd8cf86a856674d519763d0e1211cc9f8596971050d56d9b35db3",
				"785866cfbca17cfdbed6060be3629d894f924a89fdc1efc624f80d41a22f1900",
				"9503fcc3824ef62ccb9208430c26f2d8ceb2c63488ec4c07437aa4c96c43dd8b",
				"9289ed00a712ff66ee195dc71f5e4ead02172b63c543d69baf495f5fd63ba7bc",
				"c633bd309c016e37736da92129d0b053d4ab28d21ad7d8b6fab2a8bbdc8ee647",
				"d2fbcf2cf426cf892e6f5639e0252993965dfb73ccd277407014ea784aaa280c",
				"b7b03972bc8b0baa72360bdb44b82415b86b2f260f877791cd33ba8f2d65229b",
				NULL
			};

			if(!TEST_true(parse_bigBN(&e, ehex))
				|| !TEST_true(parse_bigBN(&p, phex))
				|| !TEST_true(parse_bigBN(&m, mhex))
				|| !TEST_true(BN_mod_exp_mont_consttime(d, e, p, m, TestData_BN_ctx, NULL))
				|| !TEST_true(BN_mod_exp_simple(a, e, p, m, TestData_BN_ctx))
				|| !TEST_BN_eq(a, d))
				goto err;
		}
		// Zero input 
		if(!TEST_true(BN_bntest_rand(p, 1024, 0, 0)))
			goto err;
		BN_zero(a);
		if(!TEST_true(BN_mod_exp_mont_consttime(d, a, p, m, TestData_BN_ctx, NULL)) || !TEST_BN_eq_zero(d))
			goto err;
		/*
		 * Craft an input whose Montgomery representation is 1, i.e., shorter
		 * than the modulus m, in order to test the const time precomputation
		 * scattering/gathering.
		 */
		if(!(TEST_true(BN_one(a)) && TEST_true(BN_MONT_CTX_set(mont, m, TestData_BN_ctx))))
			goto err;
		if(!TEST_true(BN_from_montgomery(e, a, mont, TestData_BN_ctx))
			|| !TEST_true(BN_mod_exp_mont_consttime(d, e, p, m, TestData_BN_ctx, NULL))
			|| !TEST_true(BN_mod_exp_simple(a, e, p, m, TestData_BN_ctx))
			|| !TEST_BN_eq(a, d))
			goto err;
		// Finally, some regular test vectors
		if(!(TEST_true(BN_bntest_rand(e, 1024, 0, 0))
			&& TEST_true(BN_mod_exp_mont_consttime(d, e, p, m, TestData_BN_ctx, NULL))
			&& TEST_true(BN_mod_exp_simple(a, e, p, m, TestData_BN_ctx))
			&& TEST_BN_eq(a, d)))
			goto err;
		st = 1;
	err:
		BN_MONT_CTX_free(mont);
		BN_free(a);
		BN_free(p);
		BN_free(m);
		BN_free(d);
		BN_free(e);
		BN_free(b);
		BN_free(n);
		BN_free(c);
		return st;
	}
	#ifndef OPENSSL_NO_EC2M
	static int test_gf2m_add()
	{
		BIGNUM * a = NULL, * b = NULL, * c = NULL;
		int i, st = 0;
		if(!TEST_ptr(a = BN_new()) || !TEST_ptr(b = BN_new()) || !TEST_ptr(c = BN_new()))
			goto err;
		for(i = 0; i < TestData_BN_NUM0; i++) {
			if(!(TEST_true(BN_rand(a, 512, 0, 0)) && TEST_ptr(BN_copy(b, BN_value_one()))))
				goto err;
			BN_set_negative(a, rand_neg());
			BN_set_negative(b, rand_neg());
			if(!(TEST_true(BN_GF2m_add(c, a, b))
				/* Test that two added values have the correct parity. */
				&& TEST_false((BN_is_odd(a) && BN_is_odd(c))
				|| (!BN_is_odd(a) && !BN_is_odd(c)))))
				goto err;
			if(!(TEST_true(BN_GF2m_add(c, c, c))
				/* Test that c + c = 0. */
				&& TEST_BN_eq_zero(c)))
				goto err;
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(c);
		return st;
	}
	static int test_gf2m_mod()
	{
		BIGNUM * a = NULL, * b[2] = {NULL, NULL}, * c = NULL, * d = NULL, * e = NULL;
		int i, j, st = 0;
		if(!TEST_ptr(a = BN_new())
			|| !TEST_ptr(b[0] = BN_new())
			|| !TEST_ptr(b[1] = BN_new())
			|| !TEST_ptr(c = BN_new())
			|| !TEST_ptr(d = BN_new())
			|| !TEST_ptr(e = BN_new()))
			goto err;
		if(!(TEST_true(BN_GF2m_arr2poly(TestData_BN_p0, b[0])) && TEST_true(BN_GF2m_arr2poly(TestData_BN_p1, b[1]))))
			goto err;
		for(i = 0; i < TestData_BN_NUM0; i++) {
			if(!TEST_true(BN_bntest_rand(a, 1024, 0, 0)))
				goto err;
			for(j = 0; j < 2; j++) {
				if(!(TEST_true(BN_GF2m_mod(c, a, b[j]))
					&& TEST_true(BN_GF2m_add(d, a, c))
					&& TEST_true(BN_GF2m_mod(e, d, b[j]))
					/* Test that a + (a mod p) mod p == 0. */
					&& TEST_BN_eq_zero(e)))
					goto err;
			}
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b[0]);
		BN_free(b[1]);
		BN_free(c);
		BN_free(d);
		BN_free(e);
		return st;
	}
	static int test_gf2m_mul()
	{
		BIGNUM * a, * b[2] = {NULL, NULL}, * c = NULL, * d = NULL;
		BIGNUM * e = NULL, * f = NULL, * g = NULL, * h = NULL;
		int i, j, st = 0;
		if(!TEST_ptr(a = BN_new())
			|| !TEST_ptr(b[0] = BN_new())
			|| !TEST_ptr(b[1] = BN_new())
			|| !TEST_ptr(c = BN_new())
			|| !TEST_ptr(d = BN_new())
			|| !TEST_ptr(e = BN_new())
			|| !TEST_ptr(f = BN_new())
			|| !TEST_ptr(g = BN_new())
			|| !TEST_ptr(h = BN_new()))
			goto err;
		if(!(TEST_true(BN_GF2m_arr2poly(TestData_BN_p0, b[0])) && TEST_true(BN_GF2m_arr2poly(TestData_BN_p1, b[1]))))
			goto err;
		for(i = 0; i < TestData_BN_NUM0; i++) {
			if(!(TEST_true(BN_bntest_rand(a, 1024, 0, 0)) && TEST_true(BN_bntest_rand(c, 1024, 0, 0)) && TEST_true(BN_bntest_rand(d, 1024, 0, 0))))
				goto err;
			for(j = 0; j < 2; j++) {
				if(!(TEST_true(BN_GF2m_mod_mul(e, a, c, b[j], TestData_BN_ctx))
					&& TEST_true(BN_GF2m_add(f, a, d))
					&& TEST_true(BN_GF2m_mod_mul(g, f, c, b[j], TestData_BN_ctx))
					&& TEST_true(BN_GF2m_mod_mul(h, d, c, b[j], TestData_BN_ctx))
					&& TEST_true(BN_GF2m_add(f, e, g))
					&& TEST_true(BN_GF2m_add(f, f, h))
					/* Test that (a+d)*c = a*c + d*c. */
					&& TEST_BN_eq_zero(f)))
					goto err;
			}
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b[0]);
		BN_free(b[1]);
		BN_free(c);
		BN_free(d);
		BN_free(e);
		BN_free(f);
		BN_free(g);
		BN_free(h);
		return st;
	}
	static int test_gf2m_sqr()
	{
		BIGNUM * a = NULL, * b[2] = {NULL, NULL}, * c = NULL, * d = NULL;
		int i, j, st = 0;
		if(!TEST_ptr(a = BN_new()) || !TEST_ptr(b[0] = BN_new()) || !TEST_ptr(b[1] = BN_new())
			|| !TEST_ptr(c = BN_new()) || !TEST_ptr(d = BN_new())) goto err;
		if(!(TEST_true(BN_GF2m_arr2poly(TestData_BN_p0, b[0])) && TEST_true(BN_GF2m_arr2poly(TestData_BN_p1, b[1]))))
			goto err;
		for(i = 0; i < TestData_BN_NUM0; i++) {
			if(!TEST_true(BN_bntest_rand(a, 1024, 0, 0)))
				goto err;
			for(j = 0; j < 2; j++) {
				if(!(TEST_true(BN_GF2m_mod_sqr(c, a, b[j], TestData_BN_ctx))
					&& TEST_true(BN_copy(d, a))
					&& TEST_true(BN_GF2m_mod_mul(d, a, d, b[j], TestData_BN_ctx))
					&& TEST_true(BN_GF2m_add(d, c, d))
					/* Test that a*a = a^2. */
					&& TEST_BN_eq_zero(d)))
					goto err;
			}
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b[0]);
		BN_free(b[1]);
		BN_free(c);
		BN_free(d);
		return st;
	}
	static int test_gf2m_modinv()
	{
		BIGNUM * a = NULL, * b[2] = {NULL, NULL}, * c = NULL, * d = NULL;
		int i, j, st = 0;
		if(!TEST_ptr(a = BN_new()) || !TEST_ptr(b[0] = BN_new()) || !TEST_ptr(b[1] = BN_new()) || !TEST_ptr(c = BN_new()) || !TEST_ptr(d = BN_new()))
			goto err;
		if(!(TEST_true(BN_GF2m_arr2poly(TestData_BN_p0, b[0])) && TEST_true(BN_GF2m_arr2poly(TestData_BN_p1, b[1]))))
			goto err;
		for(i = 0; i < TestData_BN_NUM0; i++) {
			if(!TEST_true(BN_bntest_rand(a, 512, 0, 0)))
				goto err;
			for(j = 0; j < 2; j++) {
				if(!(TEST_true(BN_GF2m_mod_inv(c, a, b[j], TestData_BN_ctx)) && TEST_true(BN_GF2m_mod_mul(d, a, c, b[j], TestData_BN_ctx)) /* Test that ((1/a)*a) = 1. */ && TEST_BN_eq_one(d)))
					goto err;
			}
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b[0]);
		BN_free(b[1]);
		BN_free(c);
		BN_free(d);
		return st;
	}
	static int test_gf2m_moddiv()
	{
		BIGNUM * a = NULL, * b[2] = {NULL, NULL}, * c = NULL, * d = NULL;
		BIGNUM * e = NULL, * f = NULL;
		int i, j, st = 0;
		if(!TEST_ptr(a = BN_new())
			|| !TEST_ptr(b[0] = BN_new())
			|| !TEST_ptr(b[1] = BN_new())
			|| !TEST_ptr(c = BN_new())
			|| !TEST_ptr(d = BN_new())
			|| !TEST_ptr(e = BN_new())
			|| !TEST_ptr(f = BN_new()))
			goto err;
		if(!(TEST_true(BN_GF2m_arr2poly(TestData_BN_p0, b[0])) && TEST_true(BN_GF2m_arr2poly(TestData_BN_p1, b[1]))))
			goto err;
		for(i = 0; i < TestData_BN_NUM0; i++) {
			if(!(TEST_true(BN_bntest_rand(a, 512, 0, 0)) && TEST_true(BN_bntest_rand(c, 512, 0, 0))))
				goto err;
			for(j = 0; j < 2; j++) {
				if(!(TEST_true(BN_GF2m_mod_div(d, a, c, b[j], TestData_BN_ctx))
					&& TEST_true(BN_GF2m_mod_mul(e, d, c, b[j], TestData_BN_ctx))
					&& TEST_true(BN_GF2m_mod_div(f, a, e, b[j], TestData_BN_ctx))
					/* Test that ((a/c)*c)/a = 1. */
					&& TEST_BN_eq_one(f)))
					goto err;
			}
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b[0]);
		BN_free(b[1]);
		BN_free(c);
		BN_free(d);
		BN_free(e);
		BN_free(f);
		return st;
	}
	static int test_gf2m_modexp()
	{
		BIGNUM * a = NULL, * b[2] = {NULL, NULL}, * c = NULL, * d = NULL;
		BIGNUM * e = NULL, * f = NULL;
		int i, j, st = 0;
		if(!TEST_ptr(a = BN_new())
			|| !TEST_ptr(b[0] = BN_new())
			|| !TEST_ptr(b[1] = BN_new())
			|| !TEST_ptr(c = BN_new())
			|| !TEST_ptr(d = BN_new())
			|| !TEST_ptr(e = BN_new())
			|| !TEST_ptr(f = BN_new()))
			goto err;
		if(!(TEST_true(BN_GF2m_arr2poly(TestData_BN_p0, b[0])) && TEST_true(BN_GF2m_arr2poly(TestData_BN_p1, b[1]))))
			goto err;
		for(i = 0; i < TestData_BN_NUM0; i++) {
			if(!(TEST_true(BN_bntest_rand(a, 512, 0, 0))
				&& TEST_true(BN_bntest_rand(c, 512, 0, 0))
				&& TEST_true(BN_bntest_rand(d, 512, 0, 0))))
				goto err;
			for(j = 0; j < 2; j++) {
				if(!(TEST_true(BN_GF2m_mod_exp(e, a, c, b[j], TestData_BN_ctx))
					&& TEST_true(BN_GF2m_mod_exp(f, a, d, b[j], TestData_BN_ctx))
					&& TEST_true(BN_GF2m_mod_mul(e, e, f, b[j], TestData_BN_ctx))
					&& TEST_true(BN_add(f, c, d))
					&& TEST_true(BN_GF2m_mod_exp(f, a, f, b[j], TestData_BN_ctx))
					&& TEST_true(BN_GF2m_add(f, e, f))
					/* Test that a^(c+d)=a^c*a^d. */
					&& TEST_BN_eq_zero(f)))
					goto err;
			}
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b[0]);
		BN_free(b[1]);
		BN_free(c);
		BN_free(d);
		BN_free(e);
		BN_free(f);
		return st;
	}
	static int test_gf2m_modsqrt()
	{
		BIGNUM * a = NULL, * b[2] = {NULL, NULL}, * c = NULL, * d = NULL;
		BIGNUM * e = NULL, * f = NULL;
		int i, j, st = 0;
		if(!TEST_ptr(a = BN_new())
			|| !TEST_ptr(b[0] = BN_new())
			|| !TEST_ptr(b[1] = BN_new())
			|| !TEST_ptr(c = BN_new())
			|| !TEST_ptr(d = BN_new())
			|| !TEST_ptr(e = BN_new())
			|| !TEST_ptr(f = BN_new()))
			goto err;
		if(!(TEST_true(BN_GF2m_arr2poly(TestData_BN_p0, b[0])) && TEST_true(BN_GF2m_arr2poly(TestData_BN_p1, b[1]))))
			goto err;
		for(i = 0; i < TestData_BN_NUM0; i++) {
			if(!TEST_true(BN_bntest_rand(a, 512, 0, 0)))
				goto err;
			for(j = 0; j < 2; j++) {
				if(!(TEST_true(BN_GF2m_mod(c, a, b[j]))
					&& TEST_true(BN_GF2m_mod_sqrt(d, a, b[j], TestData_BN_ctx))
					&& TEST_true(BN_GF2m_mod_sqr(e, d, b[j], TestData_BN_ctx))
					&& TEST_true(BN_GF2m_add(f, c, e))
					/* Test that d^2 = a, where d = sqrt(a). */
					&& TEST_BN_eq_zero(f)))
					goto err;
			}
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b[0]);
		BN_free(b[1]);
		BN_free(c);
		BN_free(d);
		BN_free(e);
		BN_free(f);
		return st;
	}
	static int test_gf2m_modsolvequad()
	{
		BIGNUM * a = NULL, * b[2] = {NULL, NULL}, * c = NULL, * d = NULL;
		BIGNUM * e = NULL;
		int i, j, s = 0, t, st = 0;
		if(!TEST_ptr(a = BN_new())
			|| !TEST_ptr(b[0] = BN_new())
			|| !TEST_ptr(b[1] = BN_new())
			|| !TEST_ptr(c = BN_new())
			|| !TEST_ptr(d = BN_new())
			|| !TEST_ptr(e = BN_new()))
			goto err;
		if(!(TEST_true(BN_GF2m_arr2poly(TestData_BN_p0, b[0])) && TEST_true(BN_GF2m_arr2poly(TestData_BN_p1, b[1]))))
			goto err;
		for(i = 0; i < TestData_BN_NUM0; i++) {
			if(!TEST_true(BN_bntest_rand(a, 512, 0, 0)))
				goto err;
			for(j = 0; j < 2; j++) {
				t = BN_GF2m_mod_solve_quad(c, a, b[j], TestData_BN_ctx);
				if(t) {
					s++;
					if(!(TEST_true(BN_GF2m_mod_sqr(d, c, b[j], TestData_BN_ctx))
						&& TEST_true(BN_GF2m_add(d, c, d))
						&& TEST_true(BN_GF2m_mod(e, a, b[j]))
						&& TEST_true(BN_GF2m_add(e, e, d))
						/*
						 * Test that solution of quadratic c satisfies c^2 + c = a.
						 */
						&& TEST_BN_eq_zero(e)))
						goto err;
				}
			}
		}
		if(!TEST_int_ge(s, 0)) {
			TEST_info("%d tests found no roots; probably an error", TestData_BN_NUM0);
			goto err;
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b[0]);
		BN_free(b[1]);
		BN_free(c);
		BN_free(d);
		BN_free(e);
		return st;
	}
	#endif
	static int test_kronecker()
	{
		BIGNUM * a = NULL, * b = NULL, * r = NULL, * t = NULL;
		int i, legendre, kronecker, st = 0;
		if(!TEST_ptr(a = BN_new()) || !TEST_ptr(b = BN_new()) || !TEST_ptr(r = BN_new()) || !TEST_ptr(t = BN_new()))
			goto err;
		/*
		 * We test BN_kronecker(a, b, ctx) just for b odd (Jacobi symbol). In
		 * this case we know that if b is prime, then BN_kronecker(a, b, ctx) is
		 * congruent to $a^{(b-1)/2}$, modulo $b$ (Legendre symbol). So we
		 * generate a random prime b and compare these values for a number of
		 * random a's.  (That is, we run the Solovay-Strassen primality test to
		 * confirm that b is prime, except that we don't want to test whether b
		 * is prime but whether BN_kronecker works.)
		 */
		if(!TEST_true(BN_generate_prime_ex(b, 512, 0, NULL, NULL, NULL)))
			goto err;
		BN_set_negative(b, rand_neg());
		for(i = 0; i < TestData_BN_NUM0; i++) {
			if(!TEST_true(BN_bntest_rand(a, 512, 0, 0)))
				goto err;
			BN_set_negative(a, rand_neg());
			/* t := (|b|-1)/2  (note that b is odd) */
			if(!TEST_true(BN_copy(t, b)))
				goto err;
			BN_set_negative(t, 0);
			if(!TEST_true(BN_sub_word(t, 1)))
				goto err;
			if(!TEST_true(BN_rshift1(t, t)))
				goto err;
			/* r := a^t mod b */
			BN_set_negative(b, 0);
			if(!TEST_true(BN_mod_exp_recp(r, a, t, b, TestData_BN_ctx)))
				goto err;
			BN_set_negative(b, 1);
			if(BN_is_word(r, 1))
				legendre = 1;
			else if(BN_is_zero(r))
				legendre = 0;
			else {
				if(!TEST_true(BN_add_word(r, 1)))
					goto err;
				if(!TEST_int_eq(BN_ucmp(r, b), 0)) {
					TEST_info("Legendre symbol computation failed");
					goto err;
				}
				legendre = -1;
			}
			if(!TEST_int_ge(kronecker = BN_kronecker(a, b, TestData_BN_ctx), -1))
				goto err;
			/* we actually need BN_kronecker(a, |b|) */
			if(BN_is_negative(a) && BN_is_negative(b))
				kronecker = -kronecker;
			if(!TEST_int_eq(legendre, kronecker))
				goto err;
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(r);
		BN_free(t);
		return st;
	}
	static int file_sum(STANZA * s)
	{
		BIGNUM * a = NULL, * b = NULL, * sum = NULL, * ret = NULL;
		BN_ULONG b_word;
		int st = 0;
		if(!TEST_ptr(a = getBN(s, "A"))
			|| !TEST_ptr(b = getBN(s, "B"))
			|| !TEST_ptr(sum = getBN(s, "Sum"))
			|| !TEST_ptr(ret = BN_new()))
			goto err;
		if(!TEST_true(BN_add(ret, a, b))
			|| !equalBN("A + B", sum, ret)
			|| !TEST_true(BN_sub(ret, sum, a))
			|| !equalBN("Sum - A", b, ret)
			|| !TEST_true(BN_sub(ret, sum, b))
			|| !equalBN("Sum - B", a, ret))
			goto err;
		/*
		 * Test that the functions work when |r| and |a| point to the same BIGNUM,
		 * or when |r| and |b| point to the same BIGNUM.
		 * There is no test for all of |r|, |a|, and |b| pointint to the same BIGNUM.
		 */
		if(!TEST_true(BN_copy(ret, a))
			|| !TEST_true(BN_add(ret, ret, b))
			|| !equalBN("A + B (r is a)", sum, ret)
			|| !TEST_true(BN_copy(ret, b))
			|| !TEST_true(BN_add(ret, a, ret))
			|| !equalBN("A + B (r is b)", sum, ret)
			|| !TEST_true(BN_copy(ret, sum))
			|| !TEST_true(BN_sub(ret, ret, a))
			|| !equalBN("Sum - A (r is a)", b, ret)
			|| !TEST_true(BN_copy(ret, a))
			|| !TEST_true(BN_sub(ret, sum, ret))
			|| !equalBN("Sum - A (r is b)", b, ret)
			|| !TEST_true(BN_copy(ret, sum))
			|| !TEST_true(BN_sub(ret, ret, b))
			|| !equalBN("Sum - B (r is a)", a, ret)
			|| !TEST_true(BN_copy(ret, b))
			|| !TEST_true(BN_sub(ret, sum, ret))
			|| !equalBN("Sum - B (r is b)", a, ret))
			goto err;
		/*
		 * Test BN_uadd() and BN_usub() with the prerequisites they are
		 * documented as having. Note that these functions are frequently used
		 * when the prerequisites don't hold. In those cases, they are supposed
		 * to work as if the prerequisite hold, but we don't test that yet.
		 */
		if(!BN_is_negative(a) && !BN_is_negative(b) && BN_cmp(a, b) >= 0) {
			if(!TEST_true(BN_uadd(ret, a, b))
				|| !equalBN("A +u B", sum, ret)
				|| !TEST_true(BN_usub(ret, sum, a))
				|| !equalBN("Sum -u A", b, ret)
				|| !TEST_true(BN_usub(ret, sum, b))
				|| !equalBN("Sum -u B", a, ret))
				goto err;
			/*
			 * Test that the functions work when |r| and |a| point to the same
			 * BIGNUM, or when |r| and |b| point to the same BIGNUM.
			 * There is no test for all of |r|, |a|, and |b| pointint to the same
			 * BIGNUM.
			 */
			if(!TEST_true(BN_copy(ret, a))
				|| !TEST_true(BN_uadd(ret, ret, b))
				|| !equalBN("A +u B (r is a)", sum, ret)
				|| !TEST_true(BN_copy(ret, b))
				|| !TEST_true(BN_uadd(ret, a, ret))
				|| !equalBN("A +u B (r is b)", sum, ret)
				|| !TEST_true(BN_copy(ret, sum))
				|| !TEST_true(BN_usub(ret, ret, a))
				|| !equalBN("Sum -u A (r is a)", b, ret)
				|| !TEST_true(BN_copy(ret, a))
				|| !TEST_true(BN_usub(ret, sum, ret))
				|| !equalBN("Sum -u A (r is b)", b, ret)
				|| !TEST_true(BN_copy(ret, sum))
				|| !TEST_true(BN_usub(ret, ret, b))
				|| !equalBN("Sum -u B (r is a)", a, ret)
				|| !TEST_true(BN_copy(ret, b))
				|| !TEST_true(BN_usub(ret, sum, ret))
				|| !equalBN("Sum -u B (r is b)", a, ret))
				goto err;
		}
		/*
		 * Test with BN_add_word() and BN_sub_word() if |b| is small enough.
		 */
		b_word = BN_get_word(b);
		if(!BN_is_negative(b) && b_word != (BN_ULONG)-1) {
			if(!TEST_true(BN_copy(ret, a))
				|| !TEST_true(BN_add_word(ret, b_word))
				|| !equalBN("A + B (word)", sum, ret)
				|| !TEST_true(BN_copy(ret, sum))
				|| !TEST_true(BN_sub_word(ret, b_word))
				|| !equalBN("Sum - B (word)", a, ret))
				goto err;
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(sum);
		BN_free(ret);
		return st;
	}
	static int file_lshift1(STANZA * s)
	{
		BIGNUM * a = NULL, * lshift1 = NULL, * zero = NULL, * ret = NULL;
		BIGNUM * two = NULL, * remainder = NULL;
		int st = 0;
		if(!TEST_ptr(a = getBN(s, "A"))
			|| !TEST_ptr(lshift1 = getBN(s, "LShift1"))
			|| !TEST_ptr(zero = BN_new())
			|| !TEST_ptr(ret = BN_new())
			|| !TEST_ptr(two = BN_new())
			|| !TEST_ptr(remainder = BN_new()))
			goto err;
		BN_zero(zero);
		if(!TEST_true(BN_set_word(two, 2))
			|| !TEST_true(BN_add(ret, a, a))
			|| !equalBN("A + A", lshift1, ret)
			|| !TEST_true(BN_mul(ret, a, two, TestData_BN_ctx))
			|| !equalBN("A * 2", lshift1, ret)
			|| !TEST_true(BN_div(ret, remainder, lshift1, two, TestData_BN_ctx))
			|| !equalBN("LShift1 / 2", a, ret)
			|| !equalBN("LShift1 % 2", zero, remainder)
			|| !TEST_true(BN_lshift1(ret, a))
			|| !equalBN("A << 1", lshift1, ret)
			|| !TEST_true(BN_rshift1(ret, lshift1))
			|| !equalBN("LShift >> 1", a, ret)
			|| !TEST_true(BN_rshift1(ret, lshift1))
			|| !equalBN("LShift >> 1", a, ret))
			goto err;
		/* Set the LSB to 1 and test rshift1 again. */
		if(!TEST_true(BN_set_bit(lshift1, 0))
			|| !TEST_true(BN_div(ret, NULL /* rem */, lshift1, two, TestData_BN_ctx))
			|| !equalBN("(LShift1 | 1) / 2", a, ret)
			|| !TEST_true(BN_rshift1(ret, lshift1))
			|| !equalBN("(LShift | 1) >> 1", a, ret))
			goto err;
		st = 1;
	err:
		BN_free(a);
		BN_free(lshift1);
		BN_free(zero);
		BN_free(ret);
		BN_free(two);
		BN_free(remainder);
		return st;
	}
	static int file_lshift(STANZA * s)
	{
		BIGNUM * a = NULL, * lshift = NULL, * ret = NULL;
		int n = 0, st = 0;
		if(!TEST_ptr(a = getBN(s, "A"))
			|| !TEST_ptr(lshift = getBN(s, "LShift"))
			|| !TEST_ptr(ret = BN_new())
			|| !getint(s, &n, "N"))
			goto err;
		if(!TEST_true(BN_lshift(ret, a, n))
			|| !equalBN("A << N", lshift, ret)
			|| !TEST_true(BN_rshift(ret, lshift, n))
			|| !equalBN("A >> N", a, ret))
			goto err;
		st = 1;
	err:
		BN_free(a);
		BN_free(lshift);
		BN_free(ret);
		return st;
	}
	static int file_rshift(STANZA * s)
	{
		BIGNUM * a = NULL, * rshift = NULL, * ret = NULL;
		int n = 0, st = 0;
		if(!TEST_ptr(a = getBN(s, "A"))
			|| !TEST_ptr(rshift = getBN(s, "RShift"))
			|| !TEST_ptr(ret = BN_new())
			|| !getint(s, &n, "N"))
			goto err;
		if(!TEST_true(BN_rshift(ret, a, n)) || !equalBN("A >> N", rshift, ret))
			goto err;
		// If N == 1, try with rshift1 as well 
		if(n == 1) {
			if(!TEST_true(BN_rshift1(ret, a)) || !equalBN("A >> 1 (rshift1)", rshift, ret))
				goto err;
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(rshift);
		BN_free(ret);
		return st;
	}
	static int file_square(STANZA * s)
	{
		BIGNUM * a = NULL, * square = NULL, * zero = NULL, * ret = NULL;
		BIGNUM * remainder = NULL, * tmp = NULL;
		int st = 0;
		if(!TEST_ptr(a = getBN(s, "A"))
			|| !TEST_ptr(square = getBN(s, "Square"))
			|| !TEST_ptr(zero = BN_new())
			|| !TEST_ptr(ret = BN_new())
			|| !TEST_ptr(remainder = BN_new()))
			goto err;
		BN_zero(zero);
		if(!TEST_true(BN_sqr(ret, a, TestData_BN_ctx))
			|| !equalBN("A^2", square, ret)
			|| !TEST_true(BN_mul(ret, a, a, TestData_BN_ctx))
			|| !equalBN("A * A", square, ret)
			|| !TEST_true(BN_div(ret, remainder, square, a, TestData_BN_ctx))
			|| !equalBN("Square / A", a, ret)
			|| !equalBN("Square % A", zero, remainder))
			goto err;
	#if HAVE_BN_SQRT
		BN_set_negative(a, 0);
		if(!TEST_true(BN_sqrt(ret, square, ctx)) || !equalBN("sqrt(Square)", a, ret))
			goto err;
		// BN_sqrt should fail on non-squares and negative numbers
		if(!TEST_BN_eq_zero(square)) {
			if(!TEST_ptr(tmp = BN_new()) || !TEST_true(BN_copy(tmp, square)))
				goto err;
			BN_set_negative(tmp, 1);
			if(!TEST_int_eq(BN_sqrt(ret, tmp, ctx), 0))
				goto err;
			ERR_clear_error();
			BN_set_negative(tmp, 0);
			if(BN_add(tmp, tmp, BN_value_one()))
				goto err;
			if(!TEST_int_eq(BN_sqrt(ret, tmp, ctx)))
				goto err;
			ERR_clear_error();
		}
	#endif
		st = 1;
	err:
		BN_free(a);
		BN_free(square);
		BN_free(zero);
		BN_free(ret);
		BN_free(remainder);
		BN_free(tmp);
		return st;
	}
	static int file_product(STANZA * s)
	{
		BIGNUM * a = NULL, * b = NULL, * product = NULL, * ret = NULL;
		BIGNUM * remainder = NULL, * zero = NULL;
		int st = 0;
		if(!TEST_ptr(a = getBN(s, "A"))
			|| !TEST_ptr(b = getBN(s, "B"))
			|| !TEST_ptr(product = getBN(s, "Product"))
			|| !TEST_ptr(ret = BN_new())
			|| !TEST_ptr(remainder = BN_new())
			|| !TEST_ptr(zero = BN_new()))
			goto err;
		BN_zero(zero);
		if(!TEST_true(BN_mul(ret, a, b, TestData_BN_ctx))
			|| !equalBN("A * B", product, ret)
			|| !TEST_true(BN_div(ret, remainder, product, a, TestData_BN_ctx))
			|| !equalBN("Product / A", b, ret)
			|| !equalBN("Product % A", zero, remainder)
			|| !TEST_true(BN_div(ret, remainder, product, b, TestData_BN_ctx))
			|| !equalBN("Product / B", a, ret)
			|| !equalBN("Product % B", zero, remainder))
			goto err;
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(product);
		BN_free(ret);
		BN_free(remainder);
		BN_free(zero);
		return st;
	}
	static int file_quotient(STANZA * s)
	{
		BIGNUM * a = NULL, * b = NULL, * quotient = NULL, * remainder = NULL;
		BIGNUM * ret = NULL, * ret2 = NULL, * nnmod = NULL;
		BN_ULONG b_word, ret_word;
		int st = 0;
		if(!TEST_ptr(a = getBN(s, "A"))
			|| !TEST_ptr(b = getBN(s, "B"))
			|| !TEST_ptr(quotient = getBN(s, "Quotient"))
			|| !TEST_ptr(remainder = getBN(s, "Remainder"))
			|| !TEST_ptr(ret = BN_new())
			|| !TEST_ptr(ret2 = BN_new())
			|| !TEST_ptr(nnmod = BN_new()))
			goto err;
		if(!TEST_true(BN_div(ret, ret2, a, b, TestData_BN_ctx))
			|| !equalBN("A / B", quotient, ret)
			|| !equalBN("A % B", remainder, ret2)
			|| !TEST_true(BN_mul(ret, quotient, b, TestData_BN_ctx))
			|| !TEST_true(BN_add(ret, ret, remainder))
			|| !equalBN("Quotient * B + Remainder", a, ret))
			goto err;
		//
		// Test with BN_mod_word() and BN_div_word() if the divisor is small enough.
		//
		b_word = BN_get_word(b);
		if(!BN_is_negative(b) && b_word != (BN_ULONG)-1) {
			BN_ULONG remainder_word = BN_get_word(remainder);
			assert(remainder_word != (BN_ULONG)-1);
			if(!TEST_ptr(BN_copy(ret, a)))
				goto err;
			ret_word = BN_div_word(ret, b_word);
			if(ret_word != remainder_word) {
	#ifdef BN_DEC_FMT1
				TEST_error("Got A %% B (word) = " BN_DEC_FMT1 ", wanted " BN_DEC_FMT1, ret_word, remainder_word);
	#else
				TEST_error("Got A %% B (word) mismatch");
	#endif
				goto err;
			}
			if(!equalBN("A / B (word)", quotient, ret))
				goto err;
			ret_word = BN_mod_word(a, b_word);
			if(ret_word != remainder_word) {
	#ifdef BN_DEC_FMT1
				TEST_error("Got A %% B (word) = " BN_DEC_FMT1 ", wanted " BN_DEC_FMT1 "", ret_word, remainder_word);
	#else
				TEST_error("Got A %% B (word) mismatch");
	#endif
				goto err;
			}
		}
		// Test BN_nnmod
		if(!BN_is_negative(b)) {
			if(!TEST_true(BN_copy(nnmod, remainder)) || (BN_is_negative(nnmod) && !TEST_true(BN_add(nnmod, nnmod, b))) || 
				!TEST_true(BN_nnmod(ret, a, b, TestData_BN_ctx)) || !equalBN("A % B (non-negative)", nnmod, ret))
				goto err;
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(quotient);
		BN_free(remainder);
		BN_free(ret);
		BN_free(ret2);
		BN_free(nnmod);
		return st;
	}
	static int file_modmul(STANZA * s)
	{
		BIGNUM * a = NULL, * b = NULL, * m = NULL, * mod_mul = NULL, * ret = NULL;
		int st = 0;
		if(!TEST_ptr(a = getBN(s, "A"))
			|| !TEST_ptr(b = getBN(s, "B"))
			|| !TEST_ptr(m = getBN(s, "M"))
			|| !TEST_ptr(mod_mul = getBN(s, "ModMul"))
			|| !TEST_ptr(ret = BN_new()))
			goto err;
		if(!TEST_true(BN_mod_mul(ret, a, b, m, TestData_BN_ctx)) || !equalBN("A * B (mod M)", mod_mul, ret))
			goto err;
		if(BN_is_odd(m)) {
			/* Reduce |a| and |b| and test the Montgomery version. */
			BN_MONT_CTX * mont = BN_MONT_CTX_new();
			BIGNUM * a_tmp = BN_new();
			BIGNUM * b_tmp = BN_new();
			if(mont == NULL || a_tmp == NULL || b_tmp == NULL
				|| !TEST_true(BN_MONT_CTX_set(mont, m, TestData_BN_ctx))
				|| !TEST_true(BN_nnmod(a_tmp, a, m, TestData_BN_ctx))
				|| !TEST_true(BN_nnmod(b_tmp, b, m, TestData_BN_ctx))
				|| !TEST_true(BN_to_montgomery(a_tmp, a_tmp, mont, TestData_BN_ctx))
				|| !TEST_true(BN_to_montgomery(b_tmp, b_tmp, mont, TestData_BN_ctx))
				|| !TEST_true(BN_mod_mul_montgomery(ret, a_tmp, b_tmp, mont, TestData_BN_ctx))
				|| !TEST_true(BN_from_montgomery(ret, ret, mont, TestData_BN_ctx))
				|| !equalBN("A * B (mod M) (mont)", mod_mul, ret))
				st = 0;
			else
				st = 1;
			BN_MONT_CTX_free(mont);
			BN_free(a_tmp);
			BN_free(b_tmp);
			if(st == 0)
				goto err;
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(m);
		BN_free(mod_mul);
		BN_free(ret);
		return st;
	}
	static int file_modexp(STANZA * s)
	{
		BIGNUM * a = NULL, * e = NULL, * m = NULL, * mod_exp = NULL, * ret = NULL;
		BIGNUM * b = NULL, * c = NULL, * d = NULL;
		int st = 0;
		if(!TEST_ptr(a = getBN(s, "A"))
			|| !TEST_ptr(e = getBN(s, "E"))
			|| !TEST_ptr(m = getBN(s, "M"))
			|| !TEST_ptr(mod_exp = getBN(s, "ModExp"))
			|| !TEST_ptr(ret = BN_new())
			|| !TEST_ptr(d = BN_new()))
			goto err;
		if(!TEST_true(BN_mod_exp(ret, a, e, m, TestData_BN_ctx)) || !equalBN("A ^ E (mod M)", mod_exp, ret))
			goto err;
		if(BN_is_odd(m)) {
			if(!TEST_true(BN_mod_exp_mont(ret, a, e, m, TestData_BN_ctx, NULL))
				|| !equalBN("A ^ E (mod M) (mont)", mod_exp, ret)
				|| !TEST_true(BN_mod_exp_mont_consttime(ret, a, e, m, TestData_BN_ctx, NULL))
				|| !equalBN("A ^ E (mod M) (mont const", mod_exp, ret))
				goto err;
		}
		// Regression test for carry propagation bug in sqr8x_reduction 
		BN_hex2bn(&a, "050505050505");
		BN_hex2bn(&b, "02");
		BN_hex2bn(&c,
			"4141414141414141414141274141414141414141414141414141414141414141"
			"4141414141414141414141414141414141414141414141414141414141414141"
			"4141414141414141414141800000000000000000000000000000000000000000"
			"0000000000000000000000000000000000000000000000000000000000000000"
			"0000000000000000000000000000000000000000000000000000000000000000"
			"0000000000000000000000000000000000000000000000000000000001");
		if(!TEST_true(BN_mod_exp(d, a, b, c, TestData_BN_ctx)) || !TEST_true(BN_mul(e, a, a, TestData_BN_ctx)) || !TEST_BN_eq(d, e))
			goto err;
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(c);
		BN_free(d);
		BN_free(e);
		BN_free(m);
		BN_free(mod_exp);
		BN_free(ret);
		return st;
	}
	static int file_exp(STANZA * s)
	{
		BIGNUM * a = NULL, * e = NULL, * exp = NULL, * ret = NULL;
		int st = 0;
		if(!TEST_ptr(a = getBN(s, "A")) || !TEST_ptr(e = getBN(s, "E")) || !TEST_ptr(exp = getBN(s, "Exp")) || !TEST_ptr(ret = BN_new()))
			goto err;
		if(!TEST_true(BN_exp(ret, a, e, TestData_BN_ctx)) || !equalBN("A ^ E", exp, ret))
			goto err;
		st = 1;
	err:
		BN_free(a);
		BN_free(e);
		BN_free(exp);
		BN_free(ret);
		return st;
	}
	static int file_modsqrt(STANZA * s)
	{
		BIGNUM * a = NULL, * p = NULL, * mod_sqrt = NULL, * ret = NULL, * ret2 = NULL;
		int st = 0;
		if(!TEST_ptr(a = getBN(s, "A")) || !TEST_ptr(p = getBN(s, "P")) || !TEST_ptr(mod_sqrt = getBN(s, "ModSqrt"))
			|| !TEST_ptr(ret = BN_new()) || !TEST_ptr(ret2 = BN_new()))
			goto err;
		if(BN_is_negative(mod_sqrt)) {
			// A negative testcase 
			if(!TEST_ptr_null(BN_mod_sqrt(ret, a, p, TestData_BN_ctx)))
				goto err;
			st = 1;
			goto err;
		}
		/* There are two possible answers. */
		if(!TEST_ptr(BN_mod_sqrt(ret, a, p, TestData_BN_ctx)) || !TEST_true(BN_sub(ret2, p, ret)))
			goto err;
		/* The first condition should NOT be a test. */
		if(BN_cmp(ret2, mod_sqrt) != 0 && !equalBN("sqrt(A) (mod P)", mod_sqrt, ret))
			goto err;
		st = 1;
	err:
		BN_free(a);
		BN_free(p);
		BN_free(mod_sqrt);
		BN_free(ret);
		BN_free(ret2);
		return st;
	}
	static int file_gcd(STANZA * s)
	{
		BIGNUM * a = NULL, * b = NULL, * gcd = NULL, * ret = NULL;
		int st = 0;
		if(!TEST_ptr(a = getBN(s, "A")) || !TEST_ptr(b = getBN(s, "B")) || !TEST_ptr(gcd = getBN(s, "GCD")) || !TEST_ptr(ret = BN_new()))
			goto err;
		if(!TEST_true(BN_gcd(ret, a, b, TestData_BN_ctx)) || !equalBN("gcd(A,B)", gcd, ret))
			goto err;
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(gcd);
		BN_free(ret);
		return st;
	}
	static int test_bn2padded()
	{
		uint8_t zeros[256], out[256], reference[128];
		size_t bytes;
		BIGNUM * n;
		int st = 0;
		/* Test edge case at 0. */
		if(!TEST_ptr((n = BN_new())))
			goto err;
		if(!TEST_int_eq(BN_bn2binpad(n, NULL, 0), 0))
			goto err;
		memset(out, -1, sizeof(out));
		if(!TEST_int_eq(BN_bn2binpad(n, out, sizeof(out)), sizeof(out)))
			goto err;
		memzero(zeros, sizeof(zeros));
		if(!TEST_mem_eq(zeros, sizeof(zeros), out, sizeof(out)))
			goto err;
		// Test a random numbers at various byte lengths
		for(bytes = 128 - 7; bytes <= 128; bytes++) {
	#define TOP_BIT_ON 0
	#define BOTTOM_BIT_NOTOUCH 0
			if(!TEST_true(BN_rand(n, bytes * 8, TOP_BIT_ON, BOTTOM_BIT_NOTOUCH)))
				goto err;
			if(!TEST_int_eq(BN_num_bytes(n), bytes) || !TEST_int_eq(BN_bn2bin(n, reference), bytes))
				goto err;
			// Empty buffer should fail
			if(!TEST_int_eq(BN_bn2binpad(n, NULL, 0), -1))
				goto err;
			// One byte short should fail
			if(!TEST_int_eq(BN_bn2binpad(n, out, bytes - 1), -1))
				goto err;
			// Exactly right size should encode
			if(!TEST_int_eq(BN_bn2binpad(n, out, bytes), bytes) || !TEST_mem_eq(out, bytes, reference, bytes))
				goto err;
			// Pad up one byte extra
			if(!TEST_int_eq(BN_bn2binpad(n, out, bytes + 1), bytes + 1)
				|| !TEST_mem_eq(out + 1, bytes, reference, bytes)
				|| !TEST_mem_eq(out, 1, zeros, 1))
				goto err;
			// Pad up to 256
			if(!TEST_int_eq(BN_bn2binpad(n, out, sizeof(out)), sizeof(out))
				|| !TEST_mem_eq(out + sizeof(out) - bytes, bytes, reference, bytes)
				|| !TEST_mem_eq(out, sizeof(out) - bytes, zeros, sizeof(out) - bytes))
				goto err;
		}
		st = 1;
	err:
		BN_free(n);
		return st;
	}
	static int test_dec2bn()
	{
		BIGNUM * bn = NULL;
		int st = 0;
		if(!TEST_int_eq(parsedecBN(&bn, "0"), 1)
			|| !TEST_BN_eq_word(bn, 0)
			|| !TEST_BN_eq_zero(bn)
			|| !TEST_BN_le_zero(bn)
			|| !TEST_BN_ge_zero(bn)
			|| !TEST_BN_even(bn))
			goto err;
		BN_free(bn);
		bn = NULL;
		if(!TEST_int_eq(parsedecBN(&bn, "256"), 3)
			|| !TEST_BN_eq_word(bn, 256)
			|| !TEST_BN_ge_zero(bn)
			|| !TEST_BN_gt_zero(bn)
			|| !TEST_BN_ne_zero(bn)
			|| !TEST_BN_even(bn))
			goto err;
		BN_free(bn);
		bn = NULL;
		if(!TEST_int_eq(parsedecBN(&bn, "-42"), 3)
			|| !TEST_BN_abs_eq_word(bn, 42)
			|| !TEST_BN_lt_zero(bn)
			|| !TEST_BN_le_zero(bn)
			|| !TEST_BN_ne_zero(bn)
			|| !TEST_BN_even(bn))
			goto err;
		BN_free(bn);
		bn = NULL;
		if(!TEST_int_eq(parsedecBN(&bn, "1"), 1)
			|| !TEST_BN_eq_word(bn, 1)
			|| !TEST_BN_ne_zero(bn)
			|| !TEST_BN_gt_zero(bn)
			|| !TEST_BN_ge_zero(bn)
			|| !TEST_BN_eq_one(bn)
			|| !TEST_BN_odd(bn))
			goto err;
		BN_free(bn);
		bn = NULL;
		if(!TEST_int_eq(parsedecBN(&bn, "-0"), 2)
			|| !TEST_BN_eq_zero(bn)
			|| !TEST_BN_ge_zero(bn)
			|| !TEST_BN_le_zero(bn)
			|| !TEST_BN_even(bn))
			goto err;
		BN_free(bn);
		bn = NULL;
		if(!TEST_int_eq(parsedecBN(&bn, "42trailing garbage is ignored"), 2)
			|| !TEST_BN_abs_eq_word(bn, 42)
			|| !TEST_BN_ge_zero(bn)
			|| !TEST_BN_gt_zero(bn)
			|| !TEST_BN_ne_zero(bn)
			|| !TEST_BN_even(bn))
			goto err;
		st = 1;
	err:
		BN_free(bn);
		return st;
	}
	static int test_hex2bn()
	{
		BIGNUM * bn = NULL;
		int st = 0;
		if(!TEST_int_eq(parseBN(&bn, "0"), 1) || !TEST_BN_eq_zero(bn) || !TEST_BN_ge_zero(bn) || !TEST_BN_even(bn))
			goto err;
		BN_free(bn);
		bn = NULL;
		if(!TEST_int_eq(parseBN(&bn, "256"), 3)
			|| !TEST_BN_eq_word(bn, 0x256)
			|| !TEST_BN_ge_zero(bn)
			|| !TEST_BN_gt_zero(bn)
			|| !TEST_BN_ne_zero(bn)
			|| !TEST_BN_even(bn))
			goto err;
		BN_free(bn);
		bn = NULL;
		if(!TEST_int_eq(parseBN(&bn, "-42"), 3)
			|| !TEST_BN_abs_eq_word(bn, 0x42)
			|| !TEST_BN_lt_zero(bn)
			|| !TEST_BN_le_zero(bn)
			|| !TEST_BN_ne_zero(bn)
			|| !TEST_BN_even(bn))
			goto err;
		BN_free(bn);
		bn = NULL;
		if(!TEST_int_eq(parseBN(&bn, "cb"), 2)
			|| !TEST_BN_eq_word(bn, 0xCB)
			|| !TEST_BN_ge_zero(bn)
			|| !TEST_BN_gt_zero(bn)
			|| !TEST_BN_ne_zero(bn)
			|| !TEST_BN_odd(bn))
			goto err;
		BN_free(bn);
		bn = NULL;
		if(!TEST_int_eq(parseBN(&bn, "-0"), 2)
			|| !TEST_BN_eq_zero(bn)
			|| !TEST_BN_ge_zero(bn)
			|| !TEST_BN_le_zero(bn)
			|| !TEST_BN_even(bn))
			goto err;
		BN_free(bn);
		bn = NULL;
		if(!TEST_int_eq(parseBN(&bn, "abctrailing garbage is ignored"), 3)
			|| !TEST_BN_eq_word(bn, 0xabc)
			|| !TEST_BN_ge_zero(bn)
			|| !TEST_BN_gt_zero(bn)
			|| !TEST_BN_ne_zero(bn)
			|| !TEST_BN_even(bn))
			goto err;
		st = 1;
	err:
		BN_free(bn);
		return st;
	}
	static int test_asc2bn()
	{
		BIGNUM * bn = NULL;
		int st = 0;
		if(!TEST_ptr(bn = BN_new()))
			goto err;
		if(!TEST_true(BN_asc2bn(&bn, "0")) || !TEST_BN_eq_zero(bn) || !TEST_BN_ge_zero(bn))
			goto err;
		if(!TEST_true(BN_asc2bn(&bn, "256")) || !TEST_BN_eq_word(bn, 256) || !TEST_BN_ge_zero(bn))
			goto err;
		if(!TEST_true(BN_asc2bn(&bn, "-42")) || !TEST_BN_abs_eq_word(bn, 42) || !TEST_BN_lt_zero(bn))
			goto err;
		if(!TEST_true(BN_asc2bn(&bn, "0x1234")) || !TEST_BN_eq_word(bn, 0x1234) || !TEST_BN_ge_zero(bn))
			goto err;
		if(!TEST_true(BN_asc2bn(&bn, "0X1234")) || !TEST_BN_eq_word(bn, 0x1234) || !TEST_BN_ge_zero(bn))
			goto err;
		if(!TEST_true(BN_asc2bn(&bn, "-0xabcd")) || !TEST_BN_abs_eq_word(bn, 0xabcd) || !TEST_BN_lt_zero(bn))
			goto err;
		if(!TEST_true(BN_asc2bn(&bn, "-0")) || !TEST_BN_eq_zero(bn) || !TEST_BN_ge_zero(bn))
			goto err;
		if(!TEST_true(BN_asc2bn(&bn, "123trailing garbage is ignored")) || !TEST_BN_eq_word(bn, 123) || !TEST_BN_ge_zero(bn))
			goto err;
		st = 1;
	err:
		BN_free(bn);
		return st;
	}
	static int test_mpi(int i)
	{
		uint8_t scratch[8];
		const MPITEST * test = &TestData_BN_kMPITests[i];
		size_t mpi_len, mpi_len2;
		BIGNUM * bn = NULL;
		BIGNUM * bn2 = NULL;
		int st = 0;
		if(!TEST_ptr(bn = BN_new()) || !TEST_true(BN_asc2bn(&bn, test->base10)))
			goto err;
		mpi_len = BN_bn2mpi(bn, NULL);
		if(!TEST_size_t_le(mpi_len, sizeof(scratch)))
			goto err;
		if(!TEST_size_t_eq(mpi_len2 = BN_bn2mpi(bn, scratch), mpi_len) || !TEST_mem_eq(test->mpi, test->mpi_len, scratch, mpi_len))
			goto err;
		if(!TEST_ptr(bn2 = BN_mpi2bn(scratch, mpi_len, NULL)))
			goto err;
		if(!TEST_BN_eq(bn, bn2)) {
			BN_free(bn2);
			goto err;
		}
		BN_free(bn2);
		st = 1;
	err:
		BN_free(bn);
		return st;
	}
	static int test_rand()
	{
		BIGNUM * bn = NULL;
		int st = 0;
		if(!TEST_ptr(bn = BN_new()))
			return 0;
		// Test BN_rand for degenerate cases with |top| and |bottom| parameters
		if(!TEST_false(BN_rand(bn, 0, 0 /* top */, 0 /* bottom */))
			|| !TEST_false(BN_rand(bn, 0, 1 /* top */, 1 /* bottom */))
			|| !TEST_true(BN_rand(bn, 1, 0 /* top */, 0 /* bottom */))
			|| !TEST_BN_eq_one(bn)
			|| !TEST_false(BN_rand(bn, 1, 1 /* top */, 0 /* bottom */))
			|| !TEST_true(BN_rand(bn, 1, -1 /* top */, 1 /* bottom */))
			|| !TEST_BN_eq_one(bn)
			|| !TEST_true(BN_rand(bn, 2, 1 /* top */, 0 /* bottom */))
			|| !TEST_BN_eq_word(bn, 3))
			goto err;
		st = 1;
	err:
		BN_free(bn);
		return st;
	}
	/*
	 * Run some statistical tests to provide a degree confidence that the
	 * BN_rand_range() function works as expected.  The test cases and
	 * critical values are generated by the bn_rand_range script.
	 *
	 * Each individual test is a Chi^2 goodness of fit for a specified number
	 * of samples and range.  The samples are assumed to be independent and
	 * that they are from a discrete uniform distribution.
	 *
	 * Some of these individual tests are expected to fail, the success/failure
	 * of each is an independent Bernoulli trial.  The number of such successes
	 * will form a binomial distribution.  The count of the successes is compared
	 * against a precomputed critical value to determine the overall outcome.
	 */
	struct rand_range_case {
		uint   range;
		uint   iterations;
		double critical;
	};
	static int test_rand_range_single(size_t n)
	{
		const uint range = rand_range_cases[n].range;
		const uint iterations = rand_range_cases[n].iterations;
		const double critical = rand_range_cases[n].critical;
		const double expected = iterations / (double)range;
		double sum = 0;
		BIGNUM * rng = NULL, * val = NULL;
		size_t * counts;
		uint i, v;
		int res = 0;
		if(!TEST_ptr(counts = (size_t *)OPENSSL_zalloc(sizeof(*counts) * range))
			|| !TEST_ptr(rng = BN_new())
			|| !TEST_ptr(val = BN_new())
			|| !TEST_true(BN_set_word(rng, range)))
			goto err;
		for(i = 0; i < iterations; i++) {
			if(!TEST_true(BN_rand_range(val, rng)) || !TEST_uint_lt(v = (uint)BN_get_word(val), range))
				goto err;
			counts[v]++;
		}
		for(i = 0; i < range; i++) {
			const double delta = counts[i] - expected;
			sum += delta * delta;
		}
		sum /= expected;
		if(sum > critical) {
			TEST_info("Chi^2 test negative %.4f > %4.f", sum, critical);
			TEST_note("test case %zu  range %u  iterations %u", n + 1, range, iterations);
			goto err;
		}
		res = 1;
	err:
		BN_free(rng);
		BN_free(val);
		OPENSSL_free(counts);
		return res;
	}
	static int test_rand_range()
	{
		int n_success = 0;
		for(size_t i = 0; i < SIZEOFARRAY(rand_range_cases); i++)
			n_success += test_rand_range_single(i);
		if(TEST_int_ge(n_success, binomial_critical))
			return 1;
		TEST_note("This test is expected to fail by chance 0.01%% of the time.");
		return 0;
	}
	static int test_negzero()
	{
		BIGNUM * a = NULL, * b = NULL, * c = NULL, * d = NULL;
		BIGNUM * numerator = NULL, * denominator = NULL;
		int consttime, st = 0;
		if(!TEST_ptr(a = BN_new())
			|| !TEST_ptr(b = BN_new())
			|| !TEST_ptr(c = BN_new())
			|| !TEST_ptr(d = BN_new()))
			goto err;
		/* Test that BN_mul never gives negative zero. */
		if(!TEST_true(BN_set_word(a, 1)))
			goto err;
		BN_set_negative(a, 1);
		BN_zero(b);
		if(!TEST_true(BN_mul(c, a, b, TestData_BN_ctx)))
			goto err;
		if(!TEST_BN_eq_zero(c) || !TEST_BN_ge_zero(c))
			goto err;
		for(consttime = 0; consttime < 2; consttime++) {
			if(!TEST_ptr(numerator = BN_new()) || !TEST_ptr(denominator = BN_new()))
				goto err;
			if(consttime) {
				BN_set_flags(numerator, BN_FLG_CONSTTIME);
				BN_set_flags(denominator, BN_FLG_CONSTTIME);
			}
			// Test that BN_div never gives negative zero in the quotient. 
			if(!TEST_true(BN_set_word(numerator, 1)) || !TEST_true(BN_set_word(denominator, 2)))
				goto err;
			BN_set_negative(numerator, 1);
			if(!TEST_true(BN_div(a, b, numerator, denominator, TestData_BN_ctx)) || !TEST_BN_eq_zero(a) || !TEST_BN_ge_zero(a))
				goto err;
			/* Test that BN_div never gives negative zero in the remainder. */
			if(!TEST_true(BN_set_word(denominator, 1))
				|| !TEST_true(BN_div(a, b, numerator, denominator, TestData_BN_ctx))
				|| !TEST_BN_eq_zero(b)
				|| !TEST_BN_ge_zero(b))
				goto err;
			BN_free(numerator);
			BN_free(denominator);
			numerator = denominator = NULL;
		}
		/* Test that BN_set_negative will not produce a negative zero. */
		BN_zero(a);
		BN_set_negative(a, 1);
		if(BN_is_negative(a))
			goto err;
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(c);
		BN_free(d);
		BN_free(numerator);
		BN_free(denominator);
		return st;
	}
	static int test_badmod()
	{
		BIGNUM * a = NULL, * b = NULL, * zero = NULL;
		BN_MONT_CTX * mont = NULL;
		int st = 0;
		if(!TEST_ptr(a = BN_new()) || !TEST_ptr(b = BN_new()) || !TEST_ptr(zero = BN_new()) || !TEST_ptr(mont = BN_MONT_CTX_new()))
			goto err;
		BN_zero(zero);
		if(!TEST_false(BN_div(a, b, BN_value_one(), zero, TestData_BN_ctx)))
			goto err;
		ERR_clear_error();
		if(!TEST_false(BN_mod_mul(a, BN_value_one(), BN_value_one(), zero, TestData_BN_ctx)))
			goto err;
		ERR_clear_error();
		if(!TEST_false(BN_mod_exp(a, BN_value_one(), BN_value_one(), zero, TestData_BN_ctx)))
			goto err;
		ERR_clear_error();
		if(!TEST_false(BN_mod_exp_mont(a, BN_value_one(), BN_value_one(), zero, TestData_BN_ctx, NULL)))
			goto err;
		ERR_clear_error();
		if(!TEST_false(BN_mod_exp_mont_consttime(a, BN_value_one(), BN_value_one(), zero, TestData_BN_ctx, NULL)))
			goto err;
		ERR_clear_error();
		if(!TEST_false(BN_MONT_CTX_set(mont, zero, TestData_BN_ctx)))
			goto err;
		ERR_clear_error();
		// Some operations also may not be used with an even modulus
		if(!TEST_true(BN_set_word(b, 16)))
			goto err;
		if(!TEST_false(BN_MONT_CTX_set(mont, b, TestData_BN_ctx)))
			goto err;
		ERR_clear_error();
		if(!TEST_false(BN_mod_exp_mont(a, BN_value_one(), BN_value_one(), b, TestData_BN_ctx, NULL)))
			goto err;
		ERR_clear_error();
		if(!TEST_false(BN_mod_exp_mont_consttime(a, BN_value_one(), BN_value_one(), b, TestData_BN_ctx, NULL)))
			goto err;
		ERR_clear_error();
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(zero);
		BN_MONT_CTX_free(mont);
		return st;
	}
	static int test_expmodzero()
	{
		BIGNUM * a = NULL, * r = NULL, * zero = NULL;
		int st = 0;
		if(!TEST_ptr(zero = BN_new()) || !TEST_ptr(a = BN_new()) || !TEST_ptr(r = BN_new()))
			goto err;
		BN_zero(zero);
		if(!TEST_true(BN_mod_exp(r, a, zero, BN_value_one(), NULL))
			|| !TEST_BN_eq_zero(r)
			|| !TEST_true(BN_mod_exp_mont(r, a, zero, BN_value_one(), NULL, NULL))
			|| !TEST_BN_eq_zero(r)
			|| !TEST_true(BN_mod_exp_mont_consttime(r, a, zero, BN_value_one(), NULL, NULL))
			|| !TEST_BN_eq_zero(r)
			|| !TEST_true(BN_mod_exp_mont_word(r, 42, zero, BN_value_one(), NULL, NULL))
			|| !TEST_BN_eq_zero(r))
			goto err;
		st = 1;
	err:
		BN_free(zero);
		BN_free(a);
		BN_free(r);
		return st;
	}
	static int test_expmodone()
	{
		int ret = 0, i;
		BIGNUM * r = BN_new();
		BIGNUM * a = BN_new();
		BIGNUM * p = BN_new();
		BIGNUM * m = BN_new();
		if(!TEST_ptr(r)
			|| !TEST_ptr(a)
			|| !TEST_ptr(p)
			|| !TEST_ptr(p)
			|| !TEST_ptr(m)
			|| !TEST_true(BN_set_word(a, 1))
			|| !TEST_true(BN_set_word(p, 0))
			|| !TEST_true(BN_set_word(m, 1)))
			goto err;
		/* Calculate r = 1 ^ 0 mod 1, and check the result is always 0 */
		for(i = 0; i < 2; i++) {
			if(!TEST_true(BN_mod_exp(r, a, p, m, NULL))
				|| !TEST_BN_eq_zero(r)
				|| !TEST_true(BN_mod_exp_mont(r, a, p, m, NULL, NULL))
				|| !TEST_BN_eq_zero(r)
				|| !TEST_true(BN_mod_exp_mont_consttime(r, a, p, m, NULL, NULL))
				|| !TEST_BN_eq_zero(r)
				|| !TEST_true(BN_mod_exp_mont_word(r, 1, p, m, NULL, NULL))
				|| !TEST_BN_eq_zero(r)
				|| !TEST_true(BN_mod_exp_simple(r, a, p, m, NULL))
				|| !TEST_BN_eq_zero(r)
				|| !TEST_true(BN_mod_exp_recp(r, a, p, m, NULL))
				|| !TEST_BN_eq_zero(r))
				goto err;
			/* Repeat for r = 1 ^ 0 mod -1 */
			if(i == 0)
				BN_set_negative(m, 1);
		}
		ret = 1;
	err:
		BN_free(r);
		BN_free(a);
		BN_free(p);
		BN_free(m);
		return ret;
	}
	static int test_smallprime(int kBits)
	{
		BIGNUM * r;
		int st = 0;
		if(!TEST_ptr(r = BN_new()))
			goto err;
		if(kBits <= 1) {
			if(!TEST_false(BN_generate_prime_ex(r, kBits, 0, NULL, NULL, NULL)))
				goto err;
		}
		else {
			if(!TEST_true(BN_generate_prime_ex(r, kBits, 0, NULL, NULL, NULL)) || !TEST_int_eq(BN_num_bits(r), kBits))
				goto err;
		}
		st = 1;
	err:
		BN_free(r);
		return st;
	}
	static int test_smallsafeprime(int kBits)
	{
		BIGNUM * r;
		int st = 0;
		if(!TEST_ptr(r = BN_new()))
			goto err;
		if(kBits <= 5 && kBits != 3) {
			if(!TEST_false(BN_generate_prime_ex(r, kBits, 1, NULL, NULL, NULL)))
				goto err;
		}
		else {
			if(!TEST_true(BN_generate_prime_ex(r, kBits, 1, NULL, NULL, NULL)) || !TEST_int_eq(BN_num_bits(r), kBits))
				goto err;
		}
		st = 1;
	err:
		BN_free(r);
		return st;
	}
	static int test_is_prime(int i)
	{
		int ret = 0;
		BIGNUM * r = NULL;
		int trial;
		if(!TEST_ptr(r = BN_new()))
			goto err;
		for(trial = 0; trial <= 1; ++trial) {
			if(!TEST_true(BN_set_word(r, TestData_BN_primes[i])) || !TEST_int_eq(BN_check_prime(r, TestData_BN_ctx, NULL), 1))
				goto err;
		}
		ret = 1;
	err:
		BN_free(r);
		return ret;
	}
	static int test_not_prime(int i)
	{
		int ret = 0;
		BIGNUM * r = NULL;
		int trial;
		if(!TEST_ptr(r = BN_new()))
			goto err;
		for(trial = 0; trial <= 1; ++trial) {
			if(!TEST_true(BN_set_word(r, TestData_BN_not_primes[i])) || !TEST_false(BN_check_prime(r, TestData_BN_ctx, NULL)))
				goto err;
		}
		ret = 1;
	err:
		BN_free(r);
		return ret;
	}
	static int test_ctx_set_ct_flag(BN_CTX * c)
	{
		int st = 0;
		BIGNUM * b[15];
		BN_CTX_start(c);
		for(size_t i = 0; i < SIZEOFARRAY(b); i++) {
			if(!TEST_ptr(b[i] = BN_CTX_get(c)))
				goto err;
			if(i % 2 == 1)
				BN_set_flags(b[i], BN_FLG_CONSTTIME);
		}
		st = 1;
	err:
		BN_CTX_end(c);
		return st;
	}
	static int test_ctx_check_ct_flag(BN_CTX * c)
	{
		int st = 0;
		BIGNUM * b[30];
		BN_CTX_start(c);
		for(size_t i = 0; i < SIZEOFARRAY(b); i++) {
			if(!TEST_ptr(b[i] = BN_CTX_get(c)))
				goto err;
			if(!TEST_false(BN_get_flags(b[i], BN_FLG_CONSTTIME)))
				goto err;
		}
		st = 1;
	err:
		BN_CTX_end(c);
		return st;
	}
	static int test_ctx_consttime_flag()
	{
		/*-
		 * The constant-time flag should not "leak" among BN_CTX frames:
		 *
		 * - test_ctx_set_ct_flag() starts a frame in the given BN_CTX and
		 *   sets the BN_FLG_CONSTTIME flag on some of the BIGNUMs obtained
		 *   from the frame before ending it.
		 * - test_ctx_check_ct_flag() then starts a new frame and gets a
		 *   number of BIGNUMs from it. In absence of leaks, none of the
		 *   BIGNUMs in the new frame should have BN_FLG_CONSTTIME set.
		 *
		 * In actual BN_CTX usage inside libcrypto the leak could happen at
		 * any depth level in the BN_CTX stack, with varying results
		 * depending on the patterns of sibling trees of nested function
		 * calls sharing the same BN_CTX object, and the effect of
		 * unintended BN_FLG_CONSTTIME on the called BN_* functions.
		 *
		 * This simple unit test abstracts away this complexity and verifies
		 * that the leak does not happen between two sibling functions
		 * sharing the same BN_CTX object at the same level of nesting.
		 *
		 */
		BN_CTX * nctx = NULL;
		BN_CTX * sctx = NULL;
		size_t i = 0;
		int st = 0;
		if(!TEST_ptr(nctx = BN_CTX_new()) || !TEST_ptr(sctx = BN_CTX_secure_new()))
			goto err;
		for(i = 0; i < 2; i++) {
			BN_CTX * c = i == 0 ? nctx : sctx;
			if(!TEST_true(test_ctx_set_ct_flag(c)) || !TEST_true(test_ctx_check_ct_flag(c)))
				goto err;
		}
		st = 1;
	err:
		BN_CTX_free(nctx);
		BN_CTX_free(sctx);
		return st;
	}
	static int test_gcd_prime()
	{
		BIGNUM * a = NULL, * b = NULL, * gcd = NULL;
		int i, st = 0;
		if(!TEST_ptr(a = BN_new()) || !TEST_ptr(b = BN_new()) || !TEST_ptr(gcd = BN_new()))
			goto err;
		if(!TEST_true(BN_generate_prime_ex(a, 1024, 0, NULL, NULL, NULL)))
			goto err;
		for(i = 0; i < TestData_BN_NUM0; i++) {
			if(!TEST_true(BN_generate_prime_ex(b, 1024, 0, NULL, NULL, NULL)) || !TEST_true(BN_gcd(gcd, a, b, TestData_BN_ctx)) || !TEST_true(BN_is_one(gcd)))
				goto err;
		}
		st = 1;
	err:
		BN_free(a);
		BN_free(b);
		BN_free(gcd);
		return st;
	}
	static int test_mod_exp(int i)
	{
		const MOD_EXP_TEST * test = &TestData_BN_ModExpTests[i];
		int res = 0;
		BIGNUM* result = NULL;
		BIGNUM * base = NULL, * exponent = NULL, * modulo = NULL;
		char * s = NULL;
		if(!TEST_ptr(result = BN_new()) || !TEST_true(BN_dec2bn(&base, test->base)) || !TEST_true(BN_dec2bn(&exponent, test->exp)) || !TEST_true(BN_dec2bn(&modulo, test->mod)))
			goto err;
		if(!TEST_int_eq(BN_mod_exp(result, base, exponent, modulo, TestData_BN_ctx), 1))
			goto err;
		if(!TEST_ptr(s = BN_bn2dec(result)))
			goto err;
		if(!TEST_mem_eq(s, strlen(s), test->res, strlen(test->res)))
			goto err;
		res = 1;
	err:
		OPENSSL_free(s);
		BN_free(result);
		BN_free(base);
		BN_free(exponent);
		BN_free(modulo);
		return res;
	}
	static int test_mod_exp_consttime(int i)
	{
		const MOD_EXP_TEST * test = &TestData_BN_ModExpTests[i];
		int res = 0;
		BIGNUM* result = NULL;
		BIGNUM * base = NULL, * exponent = NULL, * modulo = NULL;
		char * s = NULL;
		if(!TEST_ptr(result = BN_new()) || !TEST_true(BN_dec2bn(&base, test->base)) || !TEST_true(BN_dec2bn(&exponent, test->exp)) || !TEST_true(BN_dec2bn(&modulo, test->mod)))
			goto err;
		BN_set_flags(base, BN_FLG_CONSTTIME);
		BN_set_flags(exponent, BN_FLG_CONSTTIME);
		BN_set_flags(modulo, BN_FLG_CONSTTIME);
		if(!TEST_int_eq(BN_mod_exp(result, base, exponent, modulo, TestData_BN_ctx), 1))
			goto err;
		if(!TEST_ptr(s = BN_bn2dec(result)))
			goto err;
		if(!TEST_mem_eq(s, strlen(s), test->res, strlen(test->res)))
			goto err;
		res = 1;
	err:
		OPENSSL_free(s);
		BN_free(result);
		BN_free(base);
		BN_free(exponent);
		BN_free(modulo);
		return res;
	}
	// 
	// Regression test to ensure BN_mod_exp2_mont fails safely if argument m is zero.
	// 
	static int test_mod_exp2_mont()
	{
		int res = 0;
		BIGNUM * exp_result = NULL;
		BIGNUM * exp_a1 = NULL;
		BIGNUM * exp_p1 = NULL;
		BIGNUM * exp_a2 = NULL;
		BIGNUM * exp_p2 = NULL;
		BIGNUM * exp_m = NULL;
		if(!TEST_ptr(exp_result = BN_new()) || !TEST_ptr(exp_a1 = BN_new()) || !TEST_ptr(exp_p1 = BN_new()) || !TEST_ptr(exp_a2 = BN_new())
			|| !TEST_ptr(exp_p2 = BN_new()) || !TEST_ptr(exp_m = BN_new()))
			goto err;
		if(!TEST_true(BN_one(exp_a1)) || !TEST_true(BN_one(exp_p1)) || !TEST_true(BN_one(exp_a2)) || !TEST_true(BN_one(exp_p2)))
			goto err;
		BN_zero(exp_m);
		// input of 0 is even, so must fail 
		if(!TEST_int_eq(BN_mod_exp2_mont(exp_result, exp_a1, exp_p1, exp_a2, exp_p2, exp_m, TestData_BN_ctx, NULL), 0))
			goto err;
		res = 1;
	err:
		BN_free(exp_result);
		BN_free(exp_a1);
		BN_free(exp_p1);
		BN_free(exp_a2);
		BN_free(exp_p2);
		BN_free(exp_m);
		return res;
	}
	static int file_test_run(STANZA * s)
	{
		static const FILETEST filetests[] = {
			{"Sum", file_sum},
			{"LShift1", file_lshift1},
			{"LShift", file_lshift},
			{"RShift", file_rshift},
			{"Square", file_square},
			{"Product", file_product},
			{"Quotient", file_quotient},
			{"ModMul", file_modmul},
			{"ModExp", file_modexp},
			{"Exp", file_exp},
			{"ModSqrt", file_modsqrt},
			{"GCD", file_gcd},
		};
		int numtests = SIZEOFARRAY(filetests);
		const FILETEST * tp = filetests;
		for(; --numtests >= 0; tp++) {
			if(findattr(s, tp->name) != NULL) {
				if(!tp->func(s)) {
					TEST_info("%s:%d: Failed %s test", s->test_file, s->start, tp->name);
					return 0;
				}
				return 1;
			}
		}
		TEST_info("%s:%d: Unknown test", s->test_file, s->start);
		return 0;
	}
	static int run_file_tests(int i)
	{
		STANZA * s = NULL;
		char * testfile = test_get_argument(i);
		int c;
		if(!TEST_ptr(s = (STANZA *)OPENSSL_zalloc(sizeof(*s))))
			return 0;
		if(!test_start_file(s, testfile)) {
			OPENSSL_free(s);
			return 0;
		}
		// Read test file
		while(!BIO_eof(s->fp) && test_readstanza(s)) {
			if(s->numpairs == 0)
				continue;
			if(!file_test_run(s))
				s->errors++;
			s->numtests++;
			test_clearstanza(s);
		}
		test_end_file(s);
		c = s->errors;
		OPENSSL_free(s);
		return c == 0;
	}
};

typedef enum OPTION_choice {
	OPT_ERR = -1,
	OPT_EOF = 0,
	OPT_STOCHASTIC_TESTS,
	OPT_TEST_ENUM
} TestData_BN_OPTION_CHOICE;

const OPTIONS * test_get_options()
{
	static const OPTIONS test_options[] = {
		OPT_TEST_OPTIONS_WITH_EXTRA_USAGE("[file...]\n"),
		{ "stochastic", OPT_STOCHASTIC_TESTS, '-', "Run stochastic tests" },
		{ OPT_HELP_STR, 1, '-', "file\tFile to run tests on. Normal tests are not run\n" },
		{ NULL }
	};
	return test_options;
}
//
//
//
static uchar TestData_Asn1DsaInternal_t_dsa_sig[] = {
	0x30, 0x06,              /* SEQUENCE tag + length */
	0x02, 0x01, 0x01,        /* INTEGER tag + length + content */
	0x02, 0x01, 0x02         /* INTEGER tag + length + content */
};

static uchar TestData_Asn1DsaInternal_t_dsa_sig_extra[] = {
	0x30, 0x06,              /* SEQUENCE tag + length */
	0x02, 0x01, 0x01,        /* INTEGER tag + length + content */
	0x02, 0x01, 0x02,        /* INTEGER tag + length + content */
	0x05, 0x00               /* NULL tag + length */
};

static uchar TestData_Asn1DsaInternal_t_dsa_sig_msb[] = {
	0x30, 0x08,              /* SEQUENCE tag + length */
	0x02, 0x02, 0x00, 0x81,  /* INTEGER tag + length + content */
	0x02, 0x02, 0x00, 0x82   /* INTEGER tag + length + content */
};

static uchar TestData_Asn1DsaInternal_t_dsa_sig_two[] = {
	0x30, 0x08,              /* SEQUENCE tag + length */
	0x02, 0x02, 0x01, 0x00,  /* INTEGER tag + length + content */
	0x02, 0x02, 0x02, 0x00   /* INTEGER tag + length + content */
};
// 
// Badly coded ASN.1 INTEGER zero wrapped in a sequence along with another (valid) INTEGER.
// 
static uchar TestData_Asn1DsaInternal_t_invalid_int_zero[] = {
	0x30, 0x05,              /* SEQUENCE tag + length */
	0x02, 0x00,              /* INTEGER tag + length */
	0x02, 0x01, 0x2a         /* INTEGER tag + length */
};
// 
// Badly coded ASN.1 INTEGER (with leading zeros) wrapped in a sequence along with another (valid) INTEGER.
// 
static uchar TestData_Asn1DsaInternal_t_invalid_int[] = {
	0x30, 0x07,              /* SEQUENCE tag + length */
	0x02, 0x02, 0x00, 0x7f,  /* INTEGER tag + length */
	0x02, 0x01, 0x2a         /* INTEGER tag + length */
};
// 
// Negative ASN.1 INTEGER wrapped in a sequence along with another (valid) INTEGER.
// 
static uchar TestData_Asn1DsaInternal_t_neg_int[] = {
	0x30, 0x06,              /* SEQUENCE tag + length */
	0x02, 0x01, 0xaa,        /* INTEGER tag + length */
	0x02, 0x01, 0x2a         /* INTEGER tag + length */
};

static uchar TestData_Asn1DsaInternal_t_trunc_der[] = {
	0x30, 0x08,              /* SEQUENCE tag + length */
	0x02, 0x02, 0x00, 0x81,  /* INTEGER tag + length */
	0x02, 0x02, 0x00         /* INTEGER tag + length */
};

static uchar TestData_Asn1DsaInternal_t_trunc_seq[] = {
	0x30, 0x07,              /* SEQUENCE tag + length */
	0x02, 0x02, 0x00, 0x81,  /* INTEGER tag + length */
	0x02, 0x02, 0x00, 0x82   /* INTEGER tag + length */
};

class TestInnerBlock_Asn1DsaInternal {
public:
	static int test_decode()
	{
		int rv = 0;
		const uchar * pder;
		BIGNUM * r = BN_new();
		BIGNUM * s = BN_new();
		/* Positive tests */
		pder = TestData_Asn1DsaInternal_t_dsa_sig;
		if(ossl_decode_der_dsa_sig(r, s, &pder, sizeof(TestData_Asn1DsaInternal_t_dsa_sig)) == 0
			|| !TEST_ptr_eq(pder, (TestData_Asn1DsaInternal_t_dsa_sig + sizeof(TestData_Asn1DsaInternal_t_dsa_sig)))
			|| !TEST_BN_eq_word(r, 1) || !TEST_BN_eq_word(s, 2)) {
			TEST_info("asn1_dsa test_decode: t_dsa_sig failed");
			goto fail;
		}
		BN_clear(r);
		BN_clear(s);
		pder = TestData_Asn1DsaInternal_t_dsa_sig_extra;
		if(ossl_decode_der_dsa_sig(r, s, &pder, sizeof(TestData_Asn1DsaInternal_t_dsa_sig_extra)) == 0
			|| !TEST_ptr_eq(pder, (TestData_Asn1DsaInternal_t_dsa_sig_extra + sizeof(TestData_Asn1DsaInternal_t_dsa_sig_extra) - 2))
			|| !TEST_BN_eq_word(r, 1) || !TEST_BN_eq_word(s, 2)) {
			TEST_info("asn1_dsa test_decode: t_dsa_sig_extra failed");
			goto fail;
		}
		BN_clear(r);
		BN_clear(s);
		pder = TestData_Asn1DsaInternal_t_dsa_sig_msb;
		if(ossl_decode_der_dsa_sig(r, s, &pder, sizeof(TestData_Asn1DsaInternal_t_dsa_sig_msb)) == 0
			|| !TEST_ptr_eq(pder, (TestData_Asn1DsaInternal_t_dsa_sig_msb + sizeof(TestData_Asn1DsaInternal_t_dsa_sig_msb)))
			|| !TEST_BN_eq_word(r, 0x81) || !TEST_BN_eq_word(s, 0x82)) {
			TEST_info("asn1_dsa test_decode: t_dsa_sig_msb failed");
			goto fail;
		}
		BN_clear(r);
		BN_clear(s);
		pder = TestData_Asn1DsaInternal_t_dsa_sig_two;
		if(ossl_decode_der_dsa_sig(r, s, &pder, sizeof(TestData_Asn1DsaInternal_t_dsa_sig_two)) == 0
			|| !TEST_ptr_eq(pder, (TestData_Asn1DsaInternal_t_dsa_sig_two + sizeof(TestData_Asn1DsaInternal_t_dsa_sig_two)))
			|| !TEST_BN_eq_word(r, 0x100) || !TEST_BN_eq_word(s, 0x200)) {
			TEST_info("asn1_dsa test_decode: t_dsa_sig_two failed");
			goto fail;
		}
		/* Negative tests */
		pder = TestData_Asn1DsaInternal_t_invalid_int_zero;
		if(ossl_decode_der_dsa_sig(r, s, &pder, sizeof(TestData_Asn1DsaInternal_t_invalid_int_zero)) != 0) {
			TEST_info("asn1_dsa test_decode: Expected t_invalid_int_zero to fail");
			goto fail;
		}
		BN_clear(r);
		BN_clear(s);
		pder = TestData_Asn1DsaInternal_t_invalid_int;
		if(ossl_decode_der_dsa_sig(r, s, &pder, sizeof(TestData_Asn1DsaInternal_t_invalid_int)) != 0) {
			TEST_info("asn1_dsa test_decode: Expected t_invalid_int to fail");
			goto fail;
		}
		BN_clear(r);
		BN_clear(s);
		pder = TestData_Asn1DsaInternal_t_neg_int;
		if(ossl_decode_der_dsa_sig(r, s, &pder, sizeof(TestData_Asn1DsaInternal_t_neg_int)) != 0) {
			TEST_info("asn1_dsa test_decode: Expected t_neg_int to fail");
			goto fail;
		}
		BN_clear(r);
		BN_clear(s);
		pder = TestData_Asn1DsaInternal_t_trunc_der;
		if(ossl_decode_der_dsa_sig(r, s, &pder, sizeof(TestData_Asn1DsaInternal_t_trunc_der)) != 0) {
			TEST_info("asn1_dsa test_decode: Expected fail t_trunc_der");
			goto fail;
		}
		BN_clear(r);
		BN_clear(s);
		pder = TestData_Asn1DsaInternal_t_trunc_seq;
		if(ossl_decode_der_dsa_sig(r, s, &pder, sizeof(TestData_Asn1DsaInternal_t_trunc_seq)) != 0) {
			TEST_info("asn1_dsa test_decode: Expected fail t_trunc_seq");
			goto fail;
		}
		rv = 1;
	fail:
		BN_free(r);
		BN_free(s);
		return rv;
	}
};
//
//
//
#ifdef __GNUC__
	#pragma GCC diagnostic ignored "-Wunused-function"
	#pragma GCC diagnostic ignored "-Wformat"
#endif
#ifdef __clang__
	#pragma clang diagnostic ignored "-Wunused-function"
	#pragma clang diagnostic ignored "-Wformat"
#endif
//
// Custom test data
//
// We conduct tests with these arrays for every type we try out.
// You will find the expected results together with the test structures
// for each type, further down.
// 
static uchar TestData_Asn1Encode_t_zero[] = { 0x00 };
static uchar TestData_Asn1Encode_t_one[] = { 0x01 };
static uchar TestData_Asn1Encode_t_one_neg[] = { 0xff };
static uchar TestData_Asn1Encode_t_minus_256[] = { 0xff, 0x00 };
static uchar TestData_Asn1Encode_t_longundef[] = { 0x7f, 0xff, 0xff, 0xff };
static uchar TestData_Asn1Encode_t_9bytes_1[] = { 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static uchar TestData_Asn1Encode_t_8bytes_1[] = { 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uchar TestData_Asn1Encode_t_8bytes_2[] = { 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static uchar TestData_Asn1Encode_t_8bytes_3_pad[] = { 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static uchar TestData_Asn1Encode_t_8bytes_4_neg[] = { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uchar TestData_Asn1Encode_t_8bytes_5_negpad[] = { 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
/* 32-bit long */
static uchar TestData_Asn1Encode_t_5bytes_1[] = { 0x01, 0xff, 0xff, 0xff, 0xff };
static uchar TestData_Asn1Encode_t_4bytes_1[] = { 0x00, 0x80, 0x00, 0x00, 0x00 };
/* We make the last byte 0xfe to avoid a clash with ASN1_LONG_UNDEF */
static uchar TestData_Asn1Encode_t_4bytes_2[] = { 0x7f, 0xff, 0xff, 0xfe };
static uchar TestData_Asn1Encode_t_4bytes_3_pad[] = { 0x00, 0x7f, 0xff, 0xff, 0xfe };
static uchar TestData_Asn1Encode_t_4bytes_4_neg[] = { 0x80, 0x00, 0x00, 0x00 };
static uchar TestData_Asn1Encode_t_4bytes_5_negpad[] = { 0xff, 0x80, 0x00, 0x00, 0x00 };

typedef struct {
	uchar * bytes1;
	size_t nbytes1;
	uchar * bytes2;
	size_t nbytes2;
} TestData_Asn1Encode_TEST_CUSTOM_DATA;
#define TestData_Asn1Encode_CUSTOM_DATA(v) { v, sizeof(v), TestData_Asn1Encode_t_one, sizeof(TestData_Asn1Encode_t_one) }, { TestData_Asn1Encode_t_one, sizeof(TestData_Asn1Encode_t_one), v, sizeof(v) }

static TestData_Asn1Encode_TEST_CUSTOM_DATA TestData_Asn1Encode_test_custom_data[] = {
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_zero),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_longundef),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_one),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_one_neg),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_minus_256),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_9bytes_1),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_8bytes_1),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_8bytes_2),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_8bytes_3_pad),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_8bytes_4_neg),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_8bytes_5_negpad),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_5bytes_1),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_4bytes_1),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_4bytes_2),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_4bytes_3_pad),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_4bytes_4_neg),
	TestData_Asn1Encode_CUSTOM_DATA(TestData_Asn1Encode_t_4bytes_5_negpad),
};
//
// Type specific test data
// 
// First, a few utility things that all type specific data can use, or in some cases, MUST use.
//
// For easy creation of arrays of expected data.  These macros correspond to the uses of CUSTOM_DATA above.
//
#define TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(num, znum) { 0xff, num, 1 }, { 0xff, 1, znum }
#define TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE            { 0, 0, 0 }, { 0, 0, 0 }
// 
// A structure to collect all test information in.  There MUST be one instance of this for each test
//
typedef int i2d_fn(void * a, uchar ** pp);
typedef void * d2i_fn(void ** a, uchar ** pp, long length);
typedef void ifree_fn(void * a);

typedef struct {
	ASN1_ITEM_EXP * asn1_type;
	const char * name;
	int skip;                /* 1 if this package should be skipped */
	/* An array of structures to compare decoded custom data with */
	void * encode_expectations;
	size_t encode_expectations_size;
	size_t encode_expectations_elem_size;
	/*
	 * An array of structures that are encoded into a DER blob, which is
	 * then decoded, and result gets compared with the original.
	 */
	void * encdec_data;
	size_t encdec_data_size;
	size_t encdec_data_elem_size;
	i2d_fn * i2d; /* The i2d function to use with this type */
	d2i_fn * d2i; /* The d2i function to use with this type */
	ifree_fn * ifree; /* Function to free a decoded structure */
} TestData_Asn1Encode_TEST_PACKAGE;

/* To facilitate the creation of an encdec_data array */
#define TestData_Asn1Encode_ENCDEC_DATA(num, znum) { 0xff, num, 1 }, { 0xff, 1, znum }
#define TestData_Asn1Encode_ENCDEC_ARRAY(max, zmax, min, zmin)      \
	TestData_Asn1Encode_ENCDEC_DATA(max, zmax),                      \
	TestData_Asn1Encode_ENCDEC_DATA(min, zmin),                      \
	TestData_Asn1Encode_ENCDEC_DATA(1, 1),                          \
	TestData_Asn1Encode_ENCDEC_DATA(-1, -1),                        \
	TestData_Asn1Encode_ENCDEC_DATA(0, ASN1_LONG_UNDEF)

#ifndef OPENSSL_NO_DEPRECATED_3_0
//
// LONG
//
typedef struct {
	/* If decoding is expected to succeed, set this to 1, otherwise 0 */
	ASN1_BOOLEAN success;
	long test_long;
	long test_zlong;
} TestData_Asn1Encode_ASN1_LONG_DATA;

ASN1_SEQUENCE(TestData_Asn1Encode_ASN1_LONG_DATA) = {
	ASN1_SIMPLE(TestData_Asn1Encode_ASN1_LONG_DATA, success, ASN1_BOOLEAN),
	ASN1_SIMPLE(TestData_Asn1Encode_ASN1_LONG_DATA, test_long, LONG),
	ASN1_EXP_OPT(TestData_Asn1Encode_ASN1_LONG_DATA, test_zlong, ZLONG, 0)
} static_ASN1_SEQUENCE_END(TestData_Asn1Encode_ASN1_LONG_DATA)

IMPLEMENT_STATIC_ASN1_ENCODE_FUNCTIONS(TestData_Asn1Encode_ASN1_LONG_DATA)
IMPLEMENT_STATIC_ASN1_ALLOC_FUNCTIONS(TestData_Asn1Encode_ASN1_LONG_DATA)

static TestData_Asn1Encode_ASN1_LONG_DATA TestData_Asn1Encode_long_expected_32bit[] = {
	/* The following should fail on the second because it's the default */
	{ 0xff, 0, 1 }, { 0, 0, 0 }, /* t_zero */
	{ 0, 0, 0 }, { 0xff, 1, 0x7fffffff }, /* t_longundef */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(1, 1), /* t_one */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(-1, -1), /* t_one_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(-256, -256), /* t_minus_256 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_9bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_2 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_3_pad */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_4_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_5_negpad */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_5bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_1 (too large positive) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT32_MAX - 1, INT32_MAX -1), /* t_4bytes_2 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_3_pad (illegal padding) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT32_MIN, INT32_MIN), /* t_4bytes_4_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_5_negpad (illegal padding) */
};
static TestData_Asn1Encode_ASN1_LONG_DATA long_encdec_data_32bit[] = {
	TestData_Asn1Encode_ENCDEC_ARRAY(LONG_MAX - 1, LONG_MAX, LONG_MIN, LONG_MIN),
	/* Check that default numbers fail */
	{ 0, ASN1_LONG_UNDEF, 1 }, { 0, 1, 0 }
};

static TestData_Asn1Encode_TEST_PACKAGE TestData_Asn1Encode_long_test_package_32bit = {
	ASN1_ITEM_ref(TestData_Asn1Encode_ASN1_LONG_DATA), "LONG", sizeof(long) != 4,
	TestData_Asn1Encode_long_expected_32bit,
	sizeof(TestData_Asn1Encode_long_expected_32bit), sizeof(TestData_Asn1Encode_long_expected_32bit[0]),
	long_encdec_data_32bit,
	sizeof(long_encdec_data_32bit), sizeof(long_encdec_data_32bit[0]),
	(i2d_fn*)i2d_TestData_Asn1Encode_ASN1_LONG_DATA, (d2i_fn*)d2i_TestData_Asn1Encode_ASN1_LONG_DATA,
	(ifree_fn*)TestData_Asn1Encode_ASN1_LONG_DATA_free
};

static TestData_Asn1Encode_ASN1_LONG_DATA TestData_Asn1Encode_long_expected_64bit[] = {
	/* The following should fail on the second because it's the default */
	{ 0xff, 0, 1 }, { 0, 0, 0 }, /* t_zero */
	{ 0, 0, 0 }, { 0xff, 1, 0x7fffffff }, /* t_longundef */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(1, 1), /* t_one */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(-1, -1), /* t_one_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(-256, -256), /* t_minus_256 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_9bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(LONG_MAX, LONG_MAX), /* t_8bytes_2 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_3_pad (illegal padding) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(LONG_MIN, LONG_MIN), /* t_8bytes_4_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_5_negpad (illegal padding) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS((long)0x1ffffffff, (long)0x1ffffffff), /* t_5bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS((long)0x80000000, (long)0x80000000), /* t_4bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT32_MAX - 1, INT32_MAX -1), /* t_4bytes_2 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_3_pad (illegal padding) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT32_MIN, INT32_MIN), /* t_4bytes_4_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_5_negpad (illegal padding) */
};
static TestData_Asn1Encode_ASN1_LONG_DATA TestData_Asn1Encode_long_encdec_data_64bit[] = {
	TestData_Asn1Encode_ENCDEC_ARRAY(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN),
	/* Check that default numbers fail */
	{ 0, ASN1_LONG_UNDEF, 1 }, { 0, 1, 0 }
};

static TestData_Asn1Encode_TEST_PACKAGE TestData_Asn1Encode_long_test_package_64bit = {
	ASN1_ITEM_ref(TestData_Asn1Encode_ASN1_LONG_DATA), "LONG", sizeof(long) != 8,
	TestData_Asn1Encode_long_expected_64bit,
	sizeof(TestData_Asn1Encode_long_expected_64bit), sizeof(TestData_Asn1Encode_long_expected_64bit[0]),
	TestData_Asn1Encode_long_encdec_data_64bit,
	sizeof(TestData_Asn1Encode_long_encdec_data_64bit), sizeof(TestData_Asn1Encode_long_encdec_data_64bit[0]),
	(i2d_fn*)i2d_TestData_Asn1Encode_ASN1_LONG_DATA, (d2i_fn*)d2i_TestData_Asn1Encode_ASN1_LONG_DATA,
	(ifree_fn*)TestData_Asn1Encode_ASN1_LONG_DATA_free
};
#endif
//
// INT32
//
typedef struct {
	ASN1_BOOLEAN success;
	int32_t test_int32;
	int32_t test_zint32;
} TestData_Asn1Encode_ASN1_INT32_DATA;

ASN1_SEQUENCE(TestData_Asn1Encode_ASN1_INT32_DATA) = {
	ASN1_SIMPLE(TestData_Asn1Encode_ASN1_INT32_DATA, success, ASN1_BOOLEAN),
	ASN1_EMBED(TestData_Asn1Encode_ASN1_INT32_DATA, test_int32, INT32),
	ASN1_EXP_OPT_EMBED(TestData_Asn1Encode_ASN1_INT32_DATA, test_zint32, ZINT32, 0)
} static_ASN1_SEQUENCE_END(TestData_Asn1Encode_ASN1_INT32_DATA)

IMPLEMENT_STATIC_ASN1_ENCODE_FUNCTIONS(TestData_Asn1Encode_ASN1_INT32_DATA)
IMPLEMENT_STATIC_ASN1_ALLOC_FUNCTIONS(TestData_Asn1Encode_ASN1_INT32_DATA)

static TestData_Asn1Encode_ASN1_INT32_DATA TestData_Asn1Encode_int32_expected[] = {
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(0, 0), /* t_zero */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(ASN1_LONG_UNDEF, ASN1_LONG_UNDEF), /* t_zero */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(1, 1), /* t_one */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(-1, -1), /* t_one_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(-256, -256), /* t_minus_256 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_9bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_2 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_3_pad */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_4_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_5_negpad */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_5bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_1 (too large positive) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT32_MAX - 1, INT32_MAX -1), /* t_4bytes_2 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_3_pad (illegal padding) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT32_MIN, INT32_MIN), /* t_4bytes_4_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_5_negpad (illegal padding) */
};
static TestData_Asn1Encode_ASN1_INT32_DATA TestData_Asn1Encode_int32_encdec_data[] = {
	TestData_Asn1Encode_ENCDEC_ARRAY(INT32_MAX, INT32_MAX, INT32_MIN, INT32_MIN),
};

static TestData_Asn1Encode_TEST_PACKAGE TestData_Asn1Encode_int32_test_package = {
	ASN1_ITEM_ref(TestData_Asn1Encode_ASN1_INT32_DATA), "INT32", 0,
	TestData_Asn1Encode_int32_expected, sizeof(TestData_Asn1Encode_int32_expected), sizeof(TestData_Asn1Encode_int32_expected[0]),
	TestData_Asn1Encode_int32_encdec_data, sizeof(TestData_Asn1Encode_int32_encdec_data), sizeof(TestData_Asn1Encode_int32_encdec_data[0]),
	(i2d_fn*)i2d_TestData_Asn1Encode_ASN1_INT32_DATA, (d2i_fn*)d2i_TestData_Asn1Encode_ASN1_INT32_DATA,
	(ifree_fn*)TestData_Asn1Encode_ASN1_INT32_DATA_free
};
//
// UINT32
//
typedef struct {
	ASN1_BOOLEAN success;
	uint32_t test_uint32;
	uint32_t test_zuint32;
} TestData_Asn1Encode_ASN1_UINT32_DATA;

ASN1_SEQUENCE(TestData_Asn1Encode_ASN1_UINT32_DATA) = {
	ASN1_SIMPLE(TestData_Asn1Encode_ASN1_UINT32_DATA, success, ASN1_BOOLEAN),
	ASN1_EMBED(TestData_Asn1Encode_ASN1_UINT32_DATA, test_uint32, UINT32),
	ASN1_EXP_OPT_EMBED(TestData_Asn1Encode_ASN1_UINT32_DATA, test_zuint32, ZUINT32, 0)
} static_ASN1_SEQUENCE_END(TestData_Asn1Encode_ASN1_UINT32_DATA)

IMPLEMENT_STATIC_ASN1_ENCODE_FUNCTIONS(TestData_Asn1Encode_ASN1_UINT32_DATA)
IMPLEMENT_STATIC_ASN1_ALLOC_FUNCTIONS(TestData_Asn1Encode_ASN1_UINT32_DATA)

static TestData_Asn1Encode_ASN1_UINT32_DATA TestData_Asn1Encode_uint32_expected[] = {
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(0, 0), /* t_zero */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(ASN1_LONG_UNDEF, ASN1_LONG_UNDEF), /* t_zero */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(1, 1), /* t_one */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_one_neg (illegal negative value) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_minus_256 (illegal negative value) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_9bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_2 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_3_pad */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_4_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_5_negpad */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_5bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(0x80000000, 0x80000000), /* t_4bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT32_MAX - 1, INT32_MAX -1), /* t_4bytes_2 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_3_pad (illegal padding) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_4_neg (illegal negative value) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_5_negpad (illegal padding) */
};
static TestData_Asn1Encode_ASN1_UINT32_DATA TestData_Asn1Encode_uint32_encdec_data[] = {
	TestData_Asn1Encode_ENCDEC_ARRAY(UINT32_MAX, UINT32_MAX, 0, 0),
};

static TestData_Asn1Encode_TEST_PACKAGE TestData_Asn1Encode_uint32_test_package = {
	ASN1_ITEM_ref(TestData_Asn1Encode_ASN1_UINT32_DATA), "UINT32", 0,
	TestData_Asn1Encode_uint32_expected, sizeof(TestData_Asn1Encode_uint32_expected), sizeof(TestData_Asn1Encode_uint32_expected[0]),
	TestData_Asn1Encode_uint32_encdec_data, sizeof(TestData_Asn1Encode_uint32_encdec_data), sizeof(TestData_Asn1Encode_uint32_encdec_data[0]),
	(i2d_fn*)i2d_TestData_Asn1Encode_ASN1_UINT32_DATA, (d2i_fn*)d2i_TestData_Asn1Encode_ASN1_UINT32_DATA,
	(ifree_fn*)TestData_Asn1Encode_ASN1_UINT32_DATA_free
};
//
// INT64
//
typedef struct {
	ASN1_BOOLEAN success;
	int64_t test_int64;
	int64_t test_zint64;
} TestData_Asn1Encode_ASN1_INT64_DATA;

ASN1_SEQUENCE(TestData_Asn1Encode_ASN1_INT64_DATA) = {
	ASN1_SIMPLE(TestData_Asn1Encode_ASN1_INT64_DATA, success, ASN1_BOOLEAN),
	ASN1_EMBED(TestData_Asn1Encode_ASN1_INT64_DATA, test_int64, INT64),
	ASN1_EXP_OPT_EMBED(TestData_Asn1Encode_ASN1_INT64_DATA, test_zint64, ZINT64, 0)
} static_ASN1_SEQUENCE_END(TestData_Asn1Encode_ASN1_INT64_DATA)

IMPLEMENT_STATIC_ASN1_ENCODE_FUNCTIONS(TestData_Asn1Encode_ASN1_INT64_DATA)
IMPLEMENT_STATIC_ASN1_ALLOC_FUNCTIONS(TestData_Asn1Encode_ASN1_INT64_DATA)

static TestData_Asn1Encode_ASN1_INT64_DATA TestData_Asn1Encode_int64_expected[] = {
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(0, 0), /* t_zero */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(ASN1_LONG_UNDEF, ASN1_LONG_UNDEF), /* t_zero */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(1, 1), /* t_one */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(-1, -1), /* t_one_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(-256, -256), /* t_minus_256 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_9bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_1 (too large positive) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT64_MAX, INT64_MAX), /* t_8bytes_2 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_3_pad (illegal padding) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT64_MIN, INT64_MIN), /* t_8bytes_4_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_5_negpad (illegal padding) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(0x1ffffffffULL, 0x1ffffffffULL), /* t_5bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(0x80000000, 0x80000000), /* t_4bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT32_MAX - 1, INT32_MAX -1), /* t_4bytes_2 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_3_pad (illegal padding) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT32_MIN, INT32_MIN), /* t_4bytes_4_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_5_negpad (illegal padding) */
};
static TestData_Asn1Encode_ASN1_INT64_DATA TestData_Asn1Encode_int64_encdec_data[] = {
	TestData_Asn1Encode_ENCDEC_ARRAY(INT64_MAX, INT64_MAX, INT64_MIN, INT64_MIN),
	TestData_Asn1Encode_ENCDEC_ARRAY(INT32_MAX, INT32_MAX, INT32_MIN, INT32_MIN),
};

static TestData_Asn1Encode_TEST_PACKAGE TestData_Asn1Encode_int64_test_package = {
	ASN1_ITEM_ref(TestData_Asn1Encode_ASN1_INT64_DATA), "INT64", 0,
	TestData_Asn1Encode_int64_expected, sizeof(TestData_Asn1Encode_int64_expected), sizeof(TestData_Asn1Encode_int64_expected[0]),
	TestData_Asn1Encode_int64_encdec_data, sizeof(TestData_Asn1Encode_int64_encdec_data), sizeof(TestData_Asn1Encode_int64_encdec_data[0]),
	(i2d_fn*)i2d_TestData_Asn1Encode_ASN1_INT64_DATA, (d2i_fn*)d2i_TestData_Asn1Encode_ASN1_INT64_DATA,
	(ifree_fn*)TestData_Asn1Encode_ASN1_INT64_DATA_free
};
//
// UINT64
//
typedef struct {
	ASN1_BOOLEAN success;
	uint64_t test_uint64;
	uint64_t test_zuint64;
} TestData_Asn1Encode_ASN1_UINT64_DATA;

ASN1_SEQUENCE(TestData_Asn1Encode_ASN1_UINT64_DATA) = {
	ASN1_SIMPLE(TestData_Asn1Encode_ASN1_UINT64_DATA, success, ASN1_BOOLEAN),
	ASN1_EMBED(TestData_Asn1Encode_ASN1_UINT64_DATA, test_uint64, UINT64),
	ASN1_EXP_OPT_EMBED(TestData_Asn1Encode_ASN1_UINT64_DATA, test_zuint64, ZUINT64, 0)
} static_ASN1_SEQUENCE_END(TestData_Asn1Encode_ASN1_UINT64_DATA)

IMPLEMENT_STATIC_ASN1_ENCODE_FUNCTIONS(TestData_Asn1Encode_ASN1_UINT64_DATA)
IMPLEMENT_STATIC_ASN1_ALLOC_FUNCTIONS(TestData_Asn1Encode_ASN1_UINT64_DATA)

static TestData_Asn1Encode_ASN1_UINT64_DATA TestData_Asn1Encode_uint64_expected[] = {
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(0, 0), /* t_zero */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(ASN1_LONG_UNDEF, ASN1_LONG_UNDEF), /* t_zero */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(1, 1), /* t_one */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_one_neg (illegal negative value) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_minus_256 (illegal negative value) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_9bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS((uint64_t)INT64_MAX+1, (uint64_t)INT64_MAX+1),
	/* t_8bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT64_MAX, INT64_MAX), /* t_8bytes_2 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_3_pad */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_4_neg */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_8bytes_5_negpad */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(0x1ffffffffULL, 0x1ffffffffULL), /* t_5bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(0x80000000, 0x80000000), /* t_4bytes_1 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_SUCCESS(INT32_MAX - 1, INT32_MAX -1), /* t_4bytes_2 */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_3_pad (illegal padding) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_4_neg (illegal negative value) */
	TestData_Asn1Encode_CUSTOM_EXPECTED_FAILURE, /* t_4bytes_5_negpad (illegal padding) */
};
static TestData_Asn1Encode_ASN1_UINT64_DATA TestData_Asn1Encode_uint64_encdec_data[] = { TestData_Asn1Encode_ENCDEC_ARRAY(UINT64_MAX, UINT64_MAX, 0, 0), };

static TestData_Asn1Encode_TEST_PACKAGE TestData_Asn1Encode_uint64_test_package = {
	ASN1_ITEM_ref(TestData_Asn1Encode_ASN1_UINT64_DATA), "UINT64", 0,
	TestData_Asn1Encode_uint64_expected, sizeof(TestData_Asn1Encode_uint64_expected), sizeof(TestData_Asn1Encode_uint64_expected[0]),
	TestData_Asn1Encode_uint64_encdec_data, sizeof(TestData_Asn1Encode_uint64_encdec_data), sizeof(TestData_Asn1Encode_uint64_encdec_data[0]),
	(i2d_fn*)i2d_TestData_Asn1Encode_ASN1_UINT64_DATA, (d2i_fn*)d2i_TestData_Asn1Encode_ASN1_UINT64_DATA,
	(ifree_fn*)TestData_Asn1Encode_ASN1_UINT64_DATA_free
};
//
// General testing functions
//
// Template structure to map onto any test data structure 
//
typedef struct {
	ASN1_BOOLEAN success;
	uchar bytes[1];   /* In reality, there's more */
} EXPECTED;

typedef struct {
	ASN1_STRING * invalidDirString;
} INVALIDTEMPLATE;

ASN1_SEQUENCE(INVALIDTEMPLATE) = {
	// DirectoryString is a CHOICE type so it must use explicit tagging -
	// but we deliberately use implicit here, which makes this template invalid.
	ASN1_IMP(INVALIDTEMPLATE, invalidDirString, DIRECTORYSTRING, 12)
} static_ASN1_SEQUENCE_END(INVALIDTEMPLATE)

IMPLEMENT_STATIC_ASN1_ENCODE_FUNCTIONS(INVALIDTEMPLATE)
IMPLEMENT_STATIC_ASN1_ALLOC_FUNCTIONS(INVALIDTEMPLATE)

class TestInnerBlock_Asn1Encode {
public:
	// 
	// do_decode returns a tristate:
	//   -1      Couldn't decode
	//    0       decoded structure wasn't what was expected (failure)
	//    1       decoded structure was what was expected (success)
	// 
	static int do_decode(uchar * bytes, long nbytes, const EXPECTED * expected, size_t expected_size, const TestData_Asn1Encode_TEST_PACKAGE * package)
	{
		int ret = 0;
		const uchar * start = bytes;
		EXPECTED * enctst = (EXPECTED *)package->d2i(NULL, &bytes, nbytes);
		if(enctst == NULL) {
			if(expected->success == 0) {
				ret = 1;
				ERR_clear_error();
			}
			else {
				ret = -1;
			}
		}
		else {
			if(start + nbytes == bytes && memcmp(enctst, expected, expected_size) == 0)
				ret = 1;
			else
				ret = 0;
		}
		package->ifree(enctst);
		return ret;
	}
	// 
	// do_encode returns a tristate:
	// 
	//  -1 Couldn't encode
	//   0 encoded DER wasn't what was expected (failure)
	//   1 encoded DER was what was expected (success)
	//
	static int do_encode(EXPECTED * input, const uchar * expected, size_t expected_len, const TestData_Asn1Encode_TEST_PACKAGE * package)
	{
		uchar * data = NULL;
		int ret = 0;
		int len = package->i2d(input, &data);
		if(len < 0)
			return -1;
		if((size_t)len != expected_len || memcmp(data, expected, expected_len) != 0) {
			if(input->success == 0) {
				ret = 1;
				ERR_clear_error();
			}
			else {
				ret = 0;
			}
		}
		else {
			ret = 1;
		}
		OPENSSL_free(data);
		return ret;
	}
	//
	// Do an encode/decode round trip 
	//
	static int do_enc_dec(EXPECTED * bytes, long nbytes, const TestData_Asn1Encode_TEST_PACKAGE * package)
	{
		uchar * data = NULL;
		int ret = 0;
		void * p = bytes;
		int len = package->i2d(p, &data);
		if(len < 0)
			return -1;
		ret = do_decode(data, len, bytes, nbytes, package);
		OPENSSL_free(data);
		return ret;
	}
	static size_t der_encode_length(size_t len, uchar ** pp)
	{
		size_t lenbytes;
		OPENSSL_assert(len < 0x8000);
		if(len > 255)
			lenbytes = 3;
		else if(len > 127)
			lenbytes = 2;
		else
			lenbytes = 1;
		if(pp) {
			if(lenbytes == 1) {
				*(*pp)++ = (uchar)len;
			}
			else {
				*(*pp)++ = (uchar)(lenbytes - 1);
				if(lenbytes == 2) {
					*(*pp)++ = (uchar)(0x80 | len);
				}
				else {
					*(*pp)++ = (uchar)(0x80 | (len >> 8));
					*(*pp)++ = (uchar)(len);
				}
			}
		}
		return lenbytes;
	}

	static size_t make_custom_der(const TestData_Asn1Encode_TEST_CUSTOM_DATA * custom_data, uchar ** encoding, int explicit_default)
	{
		size_t firstbytes, secondbytes = 0, secondbytesinner = 0, seqbytes;
		const uchar t_true[] = { V_ASN1_BOOLEAN, 0x01, 0xff };
		uchar * p = NULL;
		size_t i;
		/*
		 * The first item is just an INTEGER tag, INTEGER length and INTEGER content
		 */
		firstbytes = 1 + der_encode_length(custom_data->nbytes1, NULL) + custom_data->nbytes1;
		for(i = custom_data->nbytes2; i > 0; i--) {
			if(custom_data->bytes2[i - 1] != '\0')
				break;
		}
		if(explicit_default || i > 0) {
			/*
			 * The second item is an explicit tag, content length, INTEGER tag,
			 * INTEGER length, INTEGER bytes
			 */
			secondbytesinner = 1 + der_encode_length(custom_data->nbytes2, NULL) + custom_data->nbytes2;
			secondbytes = 1 + der_encode_length(secondbytesinner, NULL) + secondbytesinner;
		}
		/*
		 * The whole sequence is the sequence tag, content length, BOOLEAN true
		 * (copied from t_true), the first (firstbytes) and second (secondbytes) items
		 */
		seqbytes = 1 + der_encode_length(sizeof(t_true) + firstbytes + secondbytes, NULL) + sizeof(t_true) + firstbytes + secondbytes;
		*encoding = p = (uchar *)OPENSSL_malloc(seqbytes);
		if(*encoding == NULL)
			return 0;
		// Sequence tag
		*p++ = 0x30;
		der_encode_length(sizeof(t_true) + firstbytes + secondbytes, &p);
		// ASN1_BOOLEAN TRUE
		memcpy(p, t_true, sizeof(t_true)); /* Marks decoding success */
		p += sizeof(t_true);
		// First INTEGER item (non-optional)
		*p++ = V_ASN1_INTEGER;
		der_encode_length(custom_data->nbytes1, &p);
		memcpy(p, custom_data->bytes1, custom_data->nbytes1);
		p += custom_data->nbytes1;
		if(secondbytes > 0) {
			// Second INTEGER item (optional) 
			// Start with the explicit optional tag 
			*p++ = 0xa0;
			der_encode_length(secondbytesinner, &p);
			*p++ = V_ASN1_INTEGER;
			der_encode_length(custom_data->nbytes2, &p);
			memcpy(p, custom_data->bytes2, custom_data->nbytes2);
			p += custom_data->nbytes2;
		}
		OPENSSL_assert(seqbytes == (size_t)(p - *encoding));
		return seqbytes;
	}
	//
	// Attempt to decode a custom encoding of the test structure 
	//
	static int do_decode_custom(const TestData_Asn1Encode_TEST_CUSTOM_DATA * custom_data, const EXPECTED * expected, size_t expected_size, const TestData_Asn1Encode_TEST_PACKAGE * package)
	{
		uchar * encoding = NULL;
		// 
		// We force the defaults to be explicitly encoded to make sure we test
		// for defaults that shouldn't be present (i.e. we check for failure)
		// 
		size_t encoding_length = make_custom_der(custom_data, &encoding, 1);
		int ret;
		if(encoding_length == 0)
			return -1;
		ret = do_decode(encoding, encoding_length, expected, expected_size, package);
		OPENSSL_free(encoding);
		return ret;
	}
	//
	// Attempt to encode the test structure and compare it to custom DER 
	//
	static int do_encode_custom(EXPECTED * input, const TestData_Asn1Encode_TEST_CUSTOM_DATA * custom_data, const TestData_Asn1Encode_TEST_PACKAGE * package)
	{
		uchar * expected = NULL;
		size_t expected_length = make_custom_der(custom_data, &expected, 0);
		int ret;
		if(expected_length == 0)
			return -1;
		ret = do_encode(input, expected, expected_length, package);
		OPENSSL_free(expected);
		return ret;
	}
	static int do_print_item(const TestData_Asn1Encode_TEST_PACKAGE * package)
	{
	#define DATA_BUF_SIZE 256
		const ASN1_ITEM * i = ASN1_ITEM_ptr(package->asn1_type);
		ASN1_VALUE * o;
		int ret;
		OPENSSL_assert(package->encode_expectations_elem_size <= DATA_BUF_SIZE);
		if((o = (ASN1_VALUE *)OPENSSL_malloc(DATA_BUF_SIZE)) == NULL)
			return 0;
		(void)RAND_bytes((uchar *)o, (int)package->encode_expectations_elem_size);
		ret = ASN1_item_print(bio_err, o, 0, i, NULL);
		OPENSSL_free(o);
		return ret;
	}
	static int test_intern(const TestData_Asn1Encode_TEST_PACKAGE * package)
	{
		uint i;
		size_t nelems;
		int fail = 0;
		if(package->skip)
			return 1;
		// Do decode_custom checks 
		nelems = package->encode_expectations_size / package->encode_expectations_elem_size;
		OPENSSL_assert(nelems == sizeof(TestData_Asn1Encode_test_custom_data) / sizeof(TestData_Asn1Encode_test_custom_data[0]));
		for(i = 0; i < nelems; i++) {
			size_t pos = i * package->encode_expectations_elem_size;
			EXPECTED * expected = (EXPECTED*)&((uchar *)package->encode_expectations)[pos];
			switch(do_encode_custom(expected, &TestData_Asn1Encode_test_custom_data[i], package)) {
				case -1:
					if(expected->success) {
						TEST_error("Failed custom encode round trip %u of %s", i, package->name);
						TEST_openssl_errors();
						fail++;
					}
					break;
				case 0:
					TEST_error("Custom encode round trip %u of %s mismatch", i, package->name);
					TEST_openssl_errors();
					fail++;
					break;
				case 1:
					break;
				default:
					OPENSSL_die("do_encode_custom() return unknown value", __FILE__, __LINE__);
			}
			switch(do_decode_custom(&TestData_Asn1Encode_test_custom_data[i], expected,
				package->encode_expectations_elem_size,
				package)) {
				case -1:
					if(expected->success) {
						TEST_error("Failed custom decode round trip %u of %s", i, package->name);
						TEST_openssl_errors();
						fail++;
					}
					break;
				case 0:
					TEST_error("Custom decode round trip %u of %s mismatch", i, package->name);
					TEST_openssl_errors();
					fail++;
					break;
				case 1:
					break;
				default:
					OPENSSL_die("do_decode_custom() return unknown value", __FILE__, __LINE__);
			}
		}
		// Do enc_dec checks 
		nelems = package->encdec_data_size / package->encdec_data_elem_size;
		for(i = 0; i < nelems; i++) {
			size_t pos = i * package->encdec_data_elem_size;
			EXPECTED * expected = (EXPECTED*)&((uchar *)package->encdec_data)[pos];
			switch(do_enc_dec(expected, package->encdec_data_elem_size, package)) {
				case -1:
					if(expected->success) {
						TEST_error("Failed encode/decode round trip %u of %s", i, package->name);
						TEST_openssl_errors();
						fail++;
					}
					break;
				case 0:
					TEST_error("Encode/decode round trip %u of %s mismatch", i, package->name);
					fail++;
					break;
				case 1:
					break;
				default:
					OPENSSL_die("do_enc_dec() return unknown value", __FILE__, __LINE__);
			}
		}
		if(!do_print_item(package)) {
			TEST_error("Printing of %s failed", package->name);
			TEST_openssl_errors();
			fail++;
		}
		return fail == 0;
	}

	#ifndef OPENSSL_NO_DEPRECATED_3_0
		static int test_long_32bit() { return test_intern(&TestData_Asn1Encode_long_test_package_32bit); }
		static int test_long_64bit() { return test_intern(&TestData_Asn1Encode_long_test_package_64bit); }
	#endif
	static int test_int32() { return test_intern(&TestData_Asn1Encode_int32_test_package); }
	static int test_uint32() { return test_intern(&TestData_Asn1Encode_uint32_test_package); }
	static int test_int64() { return test_intern(&TestData_Asn1Encode_int64_test_package); }
	static int test_uint64() { return test_intern(&TestData_Asn1Encode_uint64_test_package); }
	static int test_invalid_template()
	{
		INVALIDTEMPLATE * temp = INVALIDTEMPLATE_new();
		int ret;
		if(!TEST_ptr(temp))
			return 0;
		ret = i2d_INVALIDTEMPLATE(temp, NULL);
		INVALIDTEMPLATE_free(temp);
		// We expect the i2d operation to fail 
		return ret < 0;
	}
};
//
//
//
#ifdef __GNUC__
	#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#ifdef __clang__
	#pragma clang diagnostic ignored "-Wunused-function"
#endif

// Badly coded ASN.1 INTEGER zero wrapped in a sequence 
static uchar TestData_Asn1Decode_t_invalid_zero[] = {
	0x30, 0x02, /* SEQUENCE tag + length */
	0x02, 0x00 /* INTEGER tag + length */
};

//
// LONG case
//
typedef struct {
	long test_long;
} TestData_Asn1Decode_ASN1_LONG_DATA;

ASN1_SEQUENCE(TestData_Asn1Decode_ASN1_LONG_DATA) = {
	ASN1_EMBED(TestData_Asn1Decode_ASN1_LONG_DATA, test_long, LONG),
} static_ASN1_SEQUENCE_END(TestData_Asn1Decode_ASN1_LONG_DATA)

IMPLEMENT_STATIC_ASN1_ENCODE_FUNCTIONS(TestData_Asn1Decode_ASN1_LONG_DATA)
IMPLEMENT_STATIC_ASN1_ALLOC_FUNCTIONS(TestData_Asn1Decode_ASN1_LONG_DATA)
//
// INT32 case
//
typedef struct {
	int32_t test_int32;
} TestData_Asn1Decode_ASN1_INT32_DATA;

ASN1_SEQUENCE(TestData_Asn1Decode_ASN1_INT32_DATA) = {
	ASN1_EMBED(TestData_Asn1Decode_ASN1_INT32_DATA, test_int32, INT32),
} static_ASN1_SEQUENCE_END(TestData_Asn1Decode_ASN1_INT32_DATA)

IMPLEMENT_STATIC_ASN1_ENCODE_FUNCTIONS(TestData_Asn1Decode_ASN1_INT32_DATA)
IMPLEMENT_STATIC_ASN1_ALLOC_FUNCTIONS(TestData_Asn1Decode_ASN1_INT32_DATA)
//
// UINT32 case
//
typedef struct {
	uint32_t test_uint32;
} TestData_Asn1Decode_ASN1_UINT32_DATA;

ASN1_SEQUENCE(TestData_Asn1Decode_ASN1_UINT32_DATA) = {
	ASN1_EMBED(TestData_Asn1Decode_ASN1_UINT32_DATA, test_uint32, UINT32),
} static_ASN1_SEQUENCE_END(TestData_Asn1Decode_ASN1_UINT32_DATA)

IMPLEMENT_STATIC_ASN1_ENCODE_FUNCTIONS(TestData_Asn1Decode_ASN1_UINT32_DATA)
IMPLEMENT_STATIC_ASN1_ALLOC_FUNCTIONS(TestData_Asn1Decode_ASN1_UINT32_DATA)
//
// INT64 case
//
typedef struct {
	int64_t test_int64;
} TestData_Asn1Decode_ASN1_INT64_DATA;

ASN1_SEQUENCE(TestData_Asn1Decode_ASN1_INT64_DATA) = {
	ASN1_EMBED(TestData_Asn1Decode_ASN1_INT64_DATA, test_int64, INT64),
} static_ASN1_SEQUENCE_END(TestData_Asn1Decode_ASN1_INT64_DATA)

IMPLEMENT_STATIC_ASN1_ENCODE_FUNCTIONS(TestData_Asn1Decode_ASN1_INT64_DATA)
IMPLEMENT_STATIC_ASN1_ALLOC_FUNCTIONS(TestData_Asn1Decode_ASN1_INT64_DATA)
//
// UINT64 case
//
typedef struct {
	uint64_t test_uint64;
} TestData_Asn1Decode_ASN1_UINT64_DATA;

ASN1_SEQUENCE(TestData_Asn1Decode_ASN1_UINT64_DATA) = {
	ASN1_EMBED(TestData_Asn1Decode_ASN1_UINT64_DATA, test_uint64, UINT64),
} static_ASN1_SEQUENCE_END(TestData_Asn1Decode_ASN1_UINT64_DATA)

IMPLEMENT_STATIC_ASN1_ENCODE_FUNCTIONS(TestData_Asn1Decode_ASN1_UINT64_DATA)
IMPLEMENT_STATIC_ASN1_ALLOC_FUNCTIONS(TestData_Asn1Decode_ASN1_UINT64_DATA)

// Empty sequence for invalid template test
static uchar TestData_Asn1Decode_t_invalid_template[] = {
	0x30, 0x03, /* SEQUENCE tag + length */
	0x0c, 0x01, 0x41 /* UTF8String, length 1, "A" */
};

class TestInnerBlock_Asn1Decode {
public:
	#ifndef OPENSSL_NO_DEPRECATED_3_0
	static int test_long()
	{
		const uchar * p = TestData_Asn1Decode_t_invalid_zero;
		TestData_Asn1Decode_ASN1_LONG_DATA * dectst = d2i_TestData_Asn1Decode_ASN1_LONG_DATA(NULL, &p, sizeof(TestData_Asn1Decode_t_invalid_zero));
		if(dectst == NULL)
			return 0; // Fail
		TestData_Asn1Decode_ASN1_LONG_DATA_free(dectst);
		return 1;
	}
	#endif
	static int test_int32()
	{
		const uchar * p = TestData_Asn1Decode_t_invalid_zero;
		TestData_Asn1Decode_ASN1_INT32_DATA * dectst = d2i_TestData_Asn1Decode_ASN1_INT32_DATA(NULL, &p, sizeof(TestData_Asn1Decode_t_invalid_zero));
		if(dectst == NULL)
			return 0; // Fail
		TestData_Asn1Decode_ASN1_INT32_DATA_free(dectst);
		return 1;
	}
	static int test_uint32()
	{
		const uchar * p = TestData_Asn1Decode_t_invalid_zero;
		TestData_Asn1Decode_ASN1_UINT32_DATA * dectst = d2i_TestData_Asn1Decode_ASN1_UINT32_DATA(NULL, &p, sizeof(TestData_Asn1Decode_t_invalid_zero));
		if(dectst == NULL)
			return 0; // Fail
		TestData_Asn1Decode_ASN1_UINT32_DATA_free(dectst);
		return 1;
	}
	static int test_int64()
	{
		const uchar * p = TestData_Asn1Decode_t_invalid_zero;
		TestData_Asn1Decode_ASN1_INT64_DATA * dectst = d2i_TestData_Asn1Decode_ASN1_INT64_DATA(NULL, &p, sizeof(TestData_Asn1Decode_t_invalid_zero));
		if(dectst == NULL)
			return 0; // Fail
		TestData_Asn1Decode_ASN1_INT64_DATA_free(dectst);
		return 1;
	}
	static int test_uint64()
	{
		const uchar * p = TestData_Asn1Decode_t_invalid_zero;
		TestData_Asn1Decode_ASN1_UINT64_DATA * dectst = d2i_TestData_Asn1Decode_ASN1_UINT64_DATA(NULL, &p, sizeof(TestData_Asn1Decode_t_invalid_zero));
		if(dectst == NULL)
			return 0; // Fail
		TestData_Asn1Decode_ASN1_UINT64_DATA_free(dectst);
		return 1;
	}
	static int test_invalid_template()
	{
		const uchar * p = TestData_Asn1Decode_t_invalid_template;
		INVALIDTEMPLATE * tmp = d2i_INVALIDTEMPLATE(NULL, &p, sizeof(TestData_Asn1Decode_t_invalid_template));
		// We expect a NULL pointer return 
		if(TEST_ptr_null(tmp))
			return 1;
		INVALIDTEMPLATE_free(tmp);
		return 0;
	}
	static int test_reuse_asn1_object()
	{
		static uchar cn_der[] = { 0x06, 0x03, 0x55, 0x04, 0x06 };
		static uchar oid_der[] = { 0x06, 0x06, 0x2a, 0x03, 0x04, 0x05, 0x06, 0x07 };
		int ret = 0;
		ASN1_OBJECT * obj;
		uchar const * p = oid_der;
		/* Create an object that owns dynamically allocated 'sn' and 'ln' fields */
		if(!TEST_ptr(obj = ASN1_OBJECT_create(NID_undef, cn_der, sizeof(cn_der), "C", "countryName")))
			goto err;
		/* reuse obj - this should not leak sn and ln */
		if(!TEST_ptr(d2i_ASN1_OBJECT(&obj, &p, sizeof(oid_der))))
			goto err;
		ret = 1;
	err:
		ASN1_OBJECT_free(obj);
		return ret;
	}
};
//
//
//
#define TestData_BioCallback_MAXCOUNT 5
static int    TestData_BioCallback_my_param_count;
static BIO  * TestData_BioCallback_my_param_b[TestData_BioCallback_MAXCOUNT];
static int    TestData_BioCallback_my_param_oper[TestData_BioCallback_MAXCOUNT];
static const  char * TestData_BioCallback_my_param_argp[TestData_BioCallback_MAXCOUNT];
static int    TestData_BioCallback_my_param_argi[TestData_BioCallback_MAXCOUNT];
static long   TestData_BioCallback_my_param_argl[TestData_BioCallback_MAXCOUNT];
static long   TestData_BioCallback_my_param_ret[TestData_BioCallback_MAXCOUNT];
static size_t TestData_BioCallback_my_param_len[TestData_BioCallback_MAXCOUNT];
static size_t TestData_BioCallback_my_param_processed[TestData_BioCallback_MAXCOUNT];

class TestInnerBlock_BioCallback {
public:
	static long my_bio_cb_ex(BIO * b, int oper, const char * argp, size_t len, int argi, long argl, int ret, size_t * processed)
	{
		if(TestData_BioCallback_my_param_count >= TestData_BioCallback_MAXCOUNT)
			return -1;
		TestData_BioCallback_my_param_b[TestData_BioCallback_my_param_count]    = b;
		TestData_BioCallback_my_param_oper[TestData_BioCallback_my_param_count] = oper;
		TestData_BioCallback_my_param_argp[TestData_BioCallback_my_param_count] = argp;
		TestData_BioCallback_my_param_argi[TestData_BioCallback_my_param_count] = argi;
		TestData_BioCallback_my_param_argl[TestData_BioCallback_my_param_count] = argl;
		TestData_BioCallback_my_param_ret[TestData_BioCallback_my_param_count]  = ret;
		TestData_BioCallback_my_param_len[TestData_BioCallback_my_param_count]  = len;
		TestData_BioCallback_my_param_processed[TestData_BioCallback_my_param_count] = processed ? *processed : 0;
		TestData_BioCallback_my_param_count++;
		return ret;
	}
	static int test_bio_callback_ex()
	{
		int ok = 0;
		BIO * bio;
		int i;
		char test1[] = "test";
		const size_t test1len = sizeof(test1) - 1;
		char test2[] = "hello";
		const size_t test2len = sizeof(test2) - 1;
		char buf[16];
		TestData_BioCallback_my_param_count = 0;
		bio = BIO_new(BIO_s_mem());
		if(!bio)
			goto err;
		BIO_set_callback_ex(bio, my_bio_cb_ex);
		i = BIO_write(bio, test1, test1len);
		if(!TEST_int_eq(i, test1len)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 2)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_WRITE)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], test1)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_len[0], test1len)
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[0], 1)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[1], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[1], BIO_CB_WRITE | BIO_CB_RETURN)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[1], test1)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_len[1], test1len)
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[1], 0L)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_processed[1], test1len)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[1], 1))
			goto err;
		TestData_BioCallback_my_param_count = 0;
		i = BIO_read(bio, buf, sizeof(buf));
		if(!TEST_mem_eq(buf, i, test1, test1len)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 2)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_READ)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], buf)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_len[0], sizeof(buf))
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[0], 1)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[1], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[1], BIO_CB_READ | BIO_CB_RETURN)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[1], buf)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_len[1], sizeof(buf))
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[1], 0L)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_processed[1], test1len)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[1], 1))
			goto err;
		// By default a mem bio returns -1 if it has run out of data 
		TestData_BioCallback_my_param_count = 0;
		i = BIO_read(bio, buf, sizeof(buf));
		if(!TEST_int_eq(i, -1)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 2)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_READ)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], buf)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_len[0], sizeof(buf))
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[0], 1)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[1], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[1], BIO_CB_READ | BIO_CB_RETURN)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[1], buf)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_len[1], sizeof(buf))
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[1], 0L)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_processed[1], 0)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[1], -1))
			goto err;
		// Force the mem bio to return 0 if it has run out of data 
		TestData_BioCallback_my_param_count = 0;
		i = BIO_set_mem_eof_return(bio, 0);
		if(!TEST_int_eq(i, 1)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 2)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_CTRL)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], NULL)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[0], BIO_C_SET_BUF_MEM_EOF_RETURN)
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[0], 1)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[1], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[1], BIO_CB_CTRL | BIO_CB_RETURN)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[1], NULL)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[1], BIO_C_SET_BUF_MEM_EOF_RETURN)
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[1], 0L)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[1], 1))
			goto err;
		TestData_BioCallback_my_param_count = 0;
		i = BIO_read(bio, buf, sizeof(buf));
		if(!TEST_int_eq(i, 0)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 2)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_READ)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], buf)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_len[0], sizeof(buf))
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[0], 1)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[1], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[1], BIO_CB_READ | BIO_CB_RETURN)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[1], buf)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_len[1], sizeof(buf))
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[1], 0L)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_processed[1], 0)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[1], 0))
			goto err;
		TestData_BioCallback_my_param_count = 0;
		i = BIO_puts(bio, test2);
		if(!TEST_int_eq(i, 5)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 2)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_PUTS)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], test2)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[0], 0)
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[0], 1)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[1], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[1], BIO_CB_PUTS | BIO_CB_RETURN)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[1], test2)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[1], 0)
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[1], 0L)
			|| !TEST_size_t_eq(TestData_BioCallback_my_param_processed[1], test2len)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[1], 1))
			goto err;
		TestData_BioCallback_my_param_count = 0;
		i = BIO_free(bio);
		if(!TEST_int_eq(i, 1)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 1)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_FREE)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], NULL)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[0], 0)
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_int_eq((int)TestData_BioCallback_my_param_ret[0], 1))
			goto finish;
		ok = 1;
		goto finish;
	err:
		BIO_free(bio);
	finish:
		// This helps finding memory leaks with ASAN 
		memzero(TestData_BioCallback_my_param_b, sizeof(TestData_BioCallback_my_param_b));
		memzero(TestData_BioCallback_my_param_argp, sizeof(TestData_BioCallback_my_param_argp));
		return ok;
	}
	#ifndef OPENSSL_NO_DEPRECATED_3_0
	static long my_bio_callback(BIO * b, int oper, const char * argp, int argi, long argl, long ret)
	{
		if(TestData_BioCallback_my_param_count >= TestData_BioCallback_MAXCOUNT)
			return -1;
		TestData_BioCallback_my_param_b[TestData_BioCallback_my_param_count]    = b;
		TestData_BioCallback_my_param_oper[TestData_BioCallback_my_param_count] = oper;
		TestData_BioCallback_my_param_argp[TestData_BioCallback_my_param_count] = argp;
		TestData_BioCallback_my_param_argi[TestData_BioCallback_my_param_count] = argi;
		TestData_BioCallback_my_param_argl[TestData_BioCallback_my_param_count] = argl;
		TestData_BioCallback_my_param_ret[TestData_BioCallback_my_param_count]  = ret;
		TestData_BioCallback_my_param_count++;
		return ret;
	}
	static int test_bio_callback()
	{
		int ok = 0;
		BIO * bio;
		int i;
		char test1[] = "test";
		const int test1len = sizeof(test1) - 1;
		char test2[] = "hello";
		const int test2len = sizeof(test2) - 1;
		char buf[16];
		TestData_BioCallback_my_param_count = 0;
		bio = BIO_new(BIO_s_mem());
		if(!bio)
			goto err;
		BIO_set_callback(bio, my_bio_callback);
		i = BIO_write(bio, test1, test1len);
		if(!TEST_int_eq(i, test1len)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 2)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_WRITE)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], test1)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[0], test1len)
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_long_eq(TestData_BioCallback_my_param_ret[0], 1L)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[1], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[1], BIO_CB_WRITE | BIO_CB_RETURN)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[1], test1)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[1], test1len)
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[1], 0L)
			|| !TEST_long_eq(TestData_BioCallback_my_param_ret[1], (long)test1len))
			goto err;
		TestData_BioCallback_my_param_count = 0;
		i = BIO_read(bio, buf, sizeof(buf));
		if(!TEST_mem_eq(buf, i, test1, test1len)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 2)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_READ)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], buf)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[0], sizeof(buf))
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_long_eq(TestData_BioCallback_my_param_ret[0], 1L)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[1], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[1], BIO_CB_READ | BIO_CB_RETURN)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[1], buf)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[1], sizeof(buf))
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[1], 0L)
			|| !TEST_long_eq(TestData_BioCallback_my_param_ret[1], (long)test1len))
			goto err;
		// By default a mem bio returns -1 if it has run out of data 
		TestData_BioCallback_my_param_count = 0;
		i = BIO_read(bio, buf, sizeof(buf));
		if(!TEST_int_eq(i, -1)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 2)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_READ)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], buf)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[0], sizeof(buf))
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_long_eq(TestData_BioCallback_my_param_ret[0], 1L)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[1], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[1], BIO_CB_READ | BIO_CB_RETURN)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[1], buf)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[1], sizeof(buf))
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[1], 0L)
			|| !TEST_long_eq(TestData_BioCallback_my_param_ret[1], -1L))
			goto err;
		// Force the mem bio to return 0 if it has run out of data 
		BIO_set_mem_eof_return(bio, 0);
		TestData_BioCallback_my_param_count = 0;
		i = BIO_read(bio, buf, sizeof(buf));
		if(!TEST_int_eq(i, 0)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 2)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_READ)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], buf)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[0], sizeof(buf))
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_long_eq(TestData_BioCallback_my_param_ret[0], 1L)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[1], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[1], BIO_CB_READ | BIO_CB_RETURN)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[1], buf)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[1], sizeof(buf))
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[1], 0L)
			|| !TEST_long_eq(TestData_BioCallback_my_param_ret[1], 0L))
			goto err;
		TestData_BioCallback_my_param_count = 0;
		i = BIO_puts(bio, test2);
		if(!TEST_int_eq(i, 5)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 2)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_PUTS)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], test2)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[0], 0)
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_long_eq(TestData_BioCallback_my_param_ret[0], 1L)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[1], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[1], BIO_CB_PUTS | BIO_CB_RETURN)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[1], test2)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[1], 0)
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[1], 0L)
			|| !TEST_long_eq(TestData_BioCallback_my_param_ret[1], (long)test2len))
			goto err;
		TestData_BioCallback_my_param_count = 0;
		i = BIO_free(bio);
		if(!TEST_int_eq(i, 1)
			|| !TEST_int_eq(TestData_BioCallback_my_param_count, 1)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_b[0], bio)
			|| !TEST_int_eq(TestData_BioCallback_my_param_oper[0], BIO_CB_FREE)
			|| !TEST_ptr_eq(TestData_BioCallback_my_param_argp[0], NULL)
			|| !TEST_int_eq(TestData_BioCallback_my_param_argi[0], 0)
			|| !TEST_long_eq(TestData_BioCallback_my_param_argl[0], 0L)
			|| !TEST_long_eq(TestData_BioCallback_my_param_ret[0], 1L))
			goto finish;
		ok = 1;
		goto finish;
	err:
		BIO_free(bio);
	finish:
		// This helps finding memory leaks with ASAN 
		memzero(TestData_BioCallback_my_param_b, sizeof(TestData_BioCallback_my_param_b));
		memzero(TestData_BioCallback_my_param_argp, sizeof(TestData_BioCallback_my_param_argp));
		return ok;
	}
	#endif
};
//
//
//
struct ossl_core_bio_st {
	int dummy;
	BIO * bio;
};

class TestInnerBlock_BioCore {
public:
	static int tst_bio_core_read_ex(OSSL_CORE_BIO * bio, char * data, size_t data_len, size_t * bytes_read)
		{ return BIO_read_ex(bio->bio, data, data_len, bytes_read); }
	static int tst_bio_core_write_ex(OSSL_CORE_BIO * bio, const char * data, size_t data_len, size_t * written)
		{ return BIO_write_ex(bio->bio, data, data_len, written); }
	static int tst_bio_core_gets(OSSL_CORE_BIO * bio, char * buf, int size) { return BIO_gets(bio->bio, buf, size); }
	static int tst_bio_core_puts(OSSL_CORE_BIO * bio, const char * str) { return BIO_puts(bio->bio, str); }
	static long tst_bio_core_ctrl(OSSL_CORE_BIO * bio, int cmd, long num, void * ptr) { return BIO_ctrl(bio->bio, cmd, num, ptr); }
	static int tst_bio_core_up_ref(OSSL_CORE_BIO * bio) { return BIO_up_ref(bio->bio); }
	static int tst_bio_core_free(OSSL_CORE_BIO * bio) { return BIO_free(bio->bio); }
	static int TestInnerBlock_BioCore::test_bio_core();
};

static const OSSL_DISPATCH TestData_BioCore_biocbs[] = {
	{ OSSL_FUNC_BIO_READ_EX, (void (*)())TestInnerBlock_BioCore::tst_bio_core_read_ex },
	{ OSSL_FUNC_BIO_WRITE_EX, (void (*)())TestInnerBlock_BioCore::tst_bio_core_write_ex },
	{ OSSL_FUNC_BIO_GETS, (void (*)())TestInnerBlock_BioCore::tst_bio_core_gets },
	{ OSSL_FUNC_BIO_PUTS, (void (*)())TestInnerBlock_BioCore::tst_bio_core_puts },
	{ OSSL_FUNC_BIO_CTRL, (void (*)())TestInnerBlock_BioCore::tst_bio_core_ctrl },
	{ OSSL_FUNC_BIO_UP_REF, (void (*)())TestInnerBlock_BioCore::tst_bio_core_up_ref },
	{ OSSL_FUNC_BIO_FREE, (void (*)())TestInnerBlock_BioCore::tst_bio_core_free },
	{ 0, NULL }
};

/*static*/int TestInnerBlock_BioCore::test_bio_core()
{
	BIO * cbio = NULL;
	BIO * cbiobad = NULL;
	OSSL_LIB_CTX * libctx = OSSL_LIB_CTX_new_from_dispatch(NULL, TestData_BioCore_biocbs);
	int testresult = 0;
	OSSL_CORE_BIO corebio;
	const char * msg = "Hello world";
	char buf[80];
	corebio.bio = BIO_new(BIO_s_mem());
	if(!TEST_ptr(corebio.bio) || !TEST_ptr(libctx)
		// Attempting to create a corebio in a libctx that was not created via OSSL_LIB_CTX_new_from_dispatch() should fail.
		|| !TEST_ptr_null((cbiobad = BIO_new_from_core_bio(NULL, &corebio)))
		|| !TEST_ptr((cbio = BIO_new_from_core_bio(libctx, &corebio))))
		goto err;
	if(!TEST_int_gt(BIO_puts(corebio.bio, msg), 0)
		/* Test a ctrl via BIO_eof */
		|| !TEST_false(BIO_eof(cbio))
		|| !TEST_int_gt(BIO_gets(cbio, buf, sizeof(buf)), 0)
		|| !TEST_true(BIO_eof(cbio))
		|| !TEST_str_eq(buf, msg))
		goto err;
	buf[0] = '\0';
	if(!TEST_int_gt(BIO_write(cbio, msg, strlen(msg) + 1), 0) || !TEST_int_gt(BIO_read(cbio, buf, sizeof(buf)), 0) || !TEST_str_eq(buf, msg))
		goto err;
	testresult = 1;
err:
	BIO_free(cbiobad);
	BIO_free(cbio);
	BIO_free(corebio.bio);
	OSSL_LIB_CTX_free(libctx);
	return testresult;
}
//
//
//
#define TesData_BioEnc_ENCRYPT  1
#define TesData_BioEnc_DECRYPT  0
#define TesData_BioEnc_DATA_SIZE    1024
#define TesData_BioEnc_MAX_IV       32
#define TesData_BioEnc_BUF_SIZE     (TesData_BioEnc_DATA_SIZE + TesData_BioEnc_MAX_IV)

static const uchar TesData_BioEnc_KEY[] = {
	0x51, 0x50, 0xd1, 0x77, 0x2f, 0x50, 0x83, 0x4a,
	0x50, 0x3e, 0x06, 0x9a, 0x97, 0x3f, 0xbd, 0x7c,
	0xe6, 0x1c, 0x43, 0x2b, 0x72, 0x0b, 0x19, 0xd1,
	0x8e, 0xc8, 0xd8, 0x4b, 0xdc, 0x63, 0x15, 0x1b
};

static const uchar TesData_BioEnc_IV[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
};

class TestInnerBlock_BioEnc {
public:
	static int do_bio_cipher(const EVP_CIPHER* cipher, const uchar* key, const uchar* iv)
	{
		BIO * b, * mem;
		static uchar inp[TesData_BioEnc_BUF_SIZE] = { 0 };
		uchar out[TesData_BioEnc_BUF_SIZE];
		uchar ref[TesData_BioEnc_BUF_SIZE];
		int i, lref, len;
		// Fill buffer with non-zero data so that over steps can be detected 
		if(!TEST_int_gt(RAND_bytes(inp, TesData_BioEnc_DATA_SIZE), 0))
			return 0;
		//
		// Encrypt tests
		//
		// reference output for single-chunk operation
		b = BIO_new(BIO_f_cipher());
		if(!TEST_ptr(b))
			return 0;
		if(!TEST_true(BIO_set_cipher(b, cipher, key, iv, TesData_BioEnc_ENCRYPT)))
			goto err;
		mem = BIO_new_mem_buf(inp, TesData_BioEnc_DATA_SIZE);
		if(!TEST_ptr(mem))
			goto err;
		BIO_push(b, mem);
		lref = BIO_read(b, ref, sizeof(ref));
		BIO_free_all(b);
		// perform split operations and compare to reference 
		for(i = 1; i < lref; i++) {
			b = BIO_new(BIO_f_cipher());
			if(!TEST_ptr(b))
				return 0;
			if(!TEST_true(BIO_set_cipher(b, cipher, key, iv, TesData_BioEnc_ENCRYPT))) {
				TEST_info("Split encrypt failed @ operation %d", i);
				goto err;
			}
			mem = BIO_new_mem_buf(inp, TesData_BioEnc_DATA_SIZE);
			if(!TEST_ptr(mem))
				goto err;
			BIO_push(b, mem);
			memzero(out, sizeof(out));
			out[i] = ~ref[i];
			len = BIO_read(b, out, i);
			/* check for overstep */
			if(!TEST_uchar_eq(out[i], (uchar)~ref[i])) {
				TEST_info("Encrypt overstep check failed @ operation %d", i);
				goto err;
			}
			len += BIO_read(b, out + len, sizeof(out) - len);
			BIO_free_all(b);
			if(!TEST_mem_eq(out, len, ref, lref)) {
				TEST_info("Encrypt compare failed @ operation %d", i);
				return 0;
			}
		}
		// perform small-chunk operations and compare to reference 
		for(i = 1; i < lref / 2; i++) {
			int delta;
			b = BIO_new(BIO_f_cipher());
			if(!TEST_ptr(b))
				return 0;
			if(!TEST_true(BIO_set_cipher(b, cipher, key, iv, TesData_BioEnc_ENCRYPT))) {
				TEST_info("Small chunk encrypt failed @ operation %d", i);
				goto err;
			}
			mem = BIO_new_mem_buf(inp, TesData_BioEnc_DATA_SIZE);
			if(!TEST_ptr(mem))
				goto err;
			BIO_push(b, mem);
			memzero(out, sizeof(out));
			for(len = 0; (delta = BIO_read(b, out + len, i));) {
				len += delta;
			}
			BIO_free_all(b);
			if(!TEST_mem_eq(out, len, ref, lref)) {
				TEST_info("Small chunk encrypt compare failed @ operation %d", i);
				return 0;
			}
		}
		//
		// Decrypt tests 
		//
		// reference output for single-chunk operation 
		b = BIO_new(BIO_f_cipher());
		if(!TEST_ptr(b))
			return 0;
		if(!TEST_true(BIO_set_cipher(b, cipher, key, iv, TesData_BioEnc_DECRYPT)))
			goto err;
		// Use original reference output as input 
		mem = BIO_new_mem_buf(ref, lref);
		if(!TEST_ptr(mem))
			goto err;
		BIO_push(b, mem);
		(void)BIO_flush(b);
		memzero(out, sizeof(out));
		len = BIO_read(b, out, sizeof(out));
		BIO_free_all(b);
		if(!TEST_mem_eq(inp, TesData_BioEnc_DATA_SIZE, out, len))
			return 0;
		// perform split operations and compare to reference 
		for(i = 1; i < lref; i++) {
			b = BIO_new(BIO_f_cipher());
			if(!TEST_ptr(b))
				return 0;
			if(!TEST_true(BIO_set_cipher(b, cipher, key, iv, TesData_BioEnc_DECRYPT))) {
				TEST_info("Split decrypt failed @ operation %d", i);
				goto err;
			}
			mem = BIO_new_mem_buf(ref, lref);
			if(!TEST_ptr(mem))
				goto err;
			BIO_push(b, mem);
			memzero(out, sizeof(out));
			out[i] = ~ref[i];
			len = BIO_read(b, out, i);
			// check for overstep 
			if(!TEST_uchar_eq(out[i], (uchar)~ref[i])) {
				TEST_info("Decrypt overstep check failed @ operation %d", i);
				goto err;
			}
			len += BIO_read(b, out + len, sizeof(out) - len);
			BIO_free_all(b);
			if(!TEST_mem_eq(inp, TesData_BioEnc_DATA_SIZE, out, len)) {
				TEST_info("Decrypt compare failed @ operation %d", i);
				return 0;
			}
		}
		// perform small-chunk operations and compare to reference 
		for(i = 1; i < lref / 2; i++) {
			int delta;
			b = BIO_new(BIO_f_cipher());
			if(!TEST_ptr(b))
				return 0;
			if(!TEST_true(BIO_set_cipher(b, cipher, key, iv, TesData_BioEnc_DECRYPT))) {
				TEST_info("Small chunk decrypt failed @ operation %d", i);
				goto err;
			}
			mem = BIO_new_mem_buf(ref, lref);
			if(!TEST_ptr(mem))
				goto err;
			BIO_push(b, mem);
			memzero(out, sizeof(out));
			for(len = 0; (delta = BIO_read(b, out + len, i));) {
				len += delta;
			}
			BIO_free_all(b);
			if(!TEST_mem_eq(inp, TesData_BioEnc_DATA_SIZE, out, len)) {
				TEST_info("Small chunk decrypt compare failed @ operation %d", i);
				return 0;
			}
		}
		return 1;
	err:
		BIO_free_all(b);
		return 0;
	}
	static int do_test_bio_cipher(const EVP_CIPHER* cipher, int idx)
	{
		switch(idx) {
			case 0: return do_bio_cipher(cipher, TesData_BioEnc_KEY, NULL);
			case 1: return do_bio_cipher(cipher, TesData_BioEnc_KEY, TesData_BioEnc_IV);
		}
		return 0;
	}
	static int test_bio_enc_aes_128_cbc(int idx) { return do_test_bio_cipher(EVP_aes_128_cbc(), idx); }
	static int test_bio_enc_aes_128_ctr(int idx) { return do_test_bio_cipher(EVP_aes_128_ctr(), idx); }
	static int test_bio_enc_aes_256_cfb(int idx) { return do_test_bio_cipher(EVP_aes_256_cfb(), idx); }
	static int test_bio_enc_aes_256_ofb(int idx) { return do_test_bio_cipher(EVP_aes_256_ofb(), idx); }
	#ifndef OPENSSL_NO_CHACHA
		static int test_bio_enc_chacha20(int idx) { return do_test_bio_cipher(EVP_chacha20(), idx); }
		#ifndef OPENSSL_NO_POLY1305
			static int test_bio_enc_chacha20_poly1305(int idx) { return do_test_bio_cipher(EVP_chacha20_poly1305(), idx); }
		#endif
	#endif
};
//
// Internal tests for the mdc2 module 
// MDC2 low level APIs are deprecated for public use, but still ok for internal use.
//
typedef struct {
	const char * input;
	const uchar expected[MDC2_DIGEST_LENGTH];
} TestData_Mdc2Internal_TESTDATA;
//
// Test driver
//
static TestData_Mdc2Internal_TESTDATA TestData_Mdc2Internal_tests[] = { { "Now is the time for all ", { 0x42, 0xE5, 0x0C, 0xD2, 0x24, 0xBA, 0xCE, 0xBA, 0x76, 0x0B, 0xDD, 0x2B, 0xD4, 0x09, 0x28, 0x1A } } };

class TestInnerBlock_Mdc2Internal {
public:
	static int test_mdc2(int idx)
	{
		uchar md[MDC2_DIGEST_LENGTH];
		MDC2_CTX c;
		const TestData_Mdc2Internal_TESTDATA testdata = TestData_Mdc2Internal_tests[idx];
		MDC2_Init(&c);
		MDC2_Update(&c, (const uchar*)testdata.input, strlen(testdata.input));
		MDC2_Final(&(md[0]), &c);
		if(!TEST_mem_eq(testdata.expected, MDC2_DIGEST_LENGTH, md, MDC2_DIGEST_LENGTH)) {
			TEST_info("mdc2 test %d: unexpected output", idx);
			return 0;
		}
		return 1;
	}
};
// 
// MDC2 low level APIs are deprecated for public use, but still ok for internal use.
// 
#ifndef OPENSSL_NO_MDC2
static uchar TestData_Mdc2_pad1[16] = { 0x42, 0xE5, 0x0C, 0xD2, 0x24, 0xBA, 0xCE, 0xBA, 0x76, 0x0B, 0xDD, 0x2B, 0xD4, 0x09, 0x28, 0x1A };
static uchar TestData_Mdc2_pad2[16] = { 0x2E, 0x46, 0x79, 0xB5, 0xAD, 0xD9, 0xCA, 0x75, 0x35, 0xD8, 0x7A, 0xFE, 0xAB, 0x33, 0xBE, 0xE2 };

class TestInnerBlock_Mdc2 {
public:
	static int test_mdc2()
	{
		int testresult = 0;
		uint pad_type = 2;
		uchar md[MDC2_DIGEST_LENGTH];
		EVP_MD_CTX * c;
		static char text[] = "Now is the time for all ";
		size_t tlen = strlen(text), i = 0;
		OSSL_PROVIDER * prov = NULL;
		OSSL_PARAM params[2];
		params[i++] = OSSL_PARAM_construct_uint(OSSL_DIGEST_PARAM_PAD_TYPE, &pad_type),
		params[i++] = OSSL_PARAM_construct_end();
		prov = OSSL_PROVIDER_load(NULL, "legacy");
	#ifdef CHARSET_EBCDIC
		ebcdic2ascii(text, text, tlen);
	#endif
		c = EVP_MD_CTX_new();
		if(!TEST_ptr(c)
			|| !TEST_true(EVP_DigestInit_ex(c, EVP_mdc2(), NULL))
			|| !TEST_true(EVP_DigestUpdate(c, (uchar *)text, tlen))
			|| !TEST_true(EVP_DigestFinal_ex(c, &(md[0]), NULL))
			|| !TEST_mem_eq(md, MDC2_DIGEST_LENGTH, TestData_Mdc2_pad1, MDC2_DIGEST_LENGTH)
			|| !TEST_true(EVP_DigestInit_ex(c, EVP_mdc2(), NULL)))
			goto end;
		if(!TEST_int_gt(EVP_MD_CTX_set_params(c, params), 0)
			|| !TEST_true(EVP_DigestUpdate(c, (uchar *)text, tlen))
			|| !TEST_true(EVP_DigestFinal_ex(c, &(md[0]), NULL))
			|| !TEST_mem_eq(md, MDC2_DIGEST_LENGTH, TestData_Mdc2_pad2, MDC2_DIGEST_LENGTH))
			goto end;
		testresult = 1;
	end:
		EVP_MD_CTX_free(c);
		OSSL_PROVIDER_unload(prov);
		return testresult;
	}
};
#endif
//
//
//
#define TestData_UserProperty_MYPROPERTIES "foo.bar=yes"

static OSSL_FUNC_provider_query_operation_fn TestData_UserProperty_testprov_query;
static OSSL_FUNC_digest_get_params_fn TestData_UserProperty_tmpmd_get_params;
static OSSL_FUNC_digest_digest_fn TestData_UserProperty_tmpmd_digest;

class TestInnerBlock_UserProperty {
public:
	enum {
		DEFAULT_PROPS_FIRST = 0,
		DEFAULT_PROPS_AFTER_LOAD,
		DEFAULT_PROPS_AFTER_FETCH,
		DEFAULT_PROPS_FINAL
	};
	static int tmpmd_get_params(OSSL_PARAM params[])
	{
		OSSL_PARAM * p = OSSL_PARAM_locate(params, OSSL_DIGEST_PARAM_BLOCK_SIZE);
		if(p && !OSSL_PARAM_set_size_t(p, 1))
			return 0;
		p = OSSL_PARAM_locate(params, OSSL_DIGEST_PARAM_SIZE);
		if(p && !OSSL_PARAM_set_size_t(p, 1))
			return 0;
		return 1;
	}
	static int tmpmd_digest(void * provctx, const unsigned char * in, size_t inl, unsigned char * out, size_t * outl, size_t outsz)
	{
		return 0;
	}
	static const OSSL_ALGORITHM * testprov_query(void * provctx, int operation_id, int * no_cache)
	{
		static const OSSL_DISPATCH TestData_UserProperty_testprovmd_functions[] = {
			{ OSSL_FUNC_DIGEST_GET_PARAMS, (void (*)())TestInnerBlock_UserProperty::tmpmd_get_params },
			{ OSSL_FUNC_DIGEST_DIGEST, (void (*)())TestInnerBlock_UserProperty::tmpmd_digest },
			{ 0, NULL }
		};
		static const OSSL_ALGORITHM TestData_UserProperty_testprov_digests[] = {
			{ "testprovmd", TestData_UserProperty_MYPROPERTIES, TestData_UserProperty_testprovmd_functions },
			{ NULL, NULL, NULL }
		};
		*no_cache = 0;
		return operation_id == OSSL_OP_DIGEST ? TestData_UserProperty_testprov_digests : NULL;
	}
	static int testprov_provider_init(const OSSL_CORE_HANDLE * handle, const OSSL_DISPATCH * in, const OSSL_DISPATCH ** out, void ** provctx)
	{
		static const OSSL_DISPATCH TestData_UserProperty_testprov_dispatch_table[] = {
			{ OSSL_FUNC_PROVIDER_QUERY_OPERATION, (void (*)())TestInnerBlock_UserProperty::testprov_query },
			{ 0, NULL }
		};
		*provctx = (void*)handle;
		*out = TestData_UserProperty_testprov_dispatch_table;
		return 1;
	}
	static int test_default_props_and_providers(int propsorder)
	{
		OSSL_LIB_CTX * libctx;
		OSSL_PROVIDER * testprov = NULL;
		EVP_MD * testprovmd = NULL;
		int res = 0;
		if(!TEST_ptr(libctx = OSSL_LIB_CTX_new()) || !TEST_true(OSSL_PROVIDER_add_builtin(libctx, "testprov",
			testprov_provider_init)))
			goto err;
		if(propsorder == DEFAULT_PROPS_FIRST && !TEST_true(EVP_set_default_properties(libctx, TestData_UserProperty_MYPROPERTIES)))
			goto err;
		if(!TEST_ptr(testprov = OSSL_PROVIDER_load(libctx, "testprov")))
			goto err;
		if(propsorder == DEFAULT_PROPS_AFTER_LOAD && !TEST_true(EVP_set_default_properties(libctx, TestData_UserProperty_MYPROPERTIES)))
			goto err;
		if(!TEST_ptr(testprovmd = EVP_MD_fetch(libctx, "testprovmd", NULL)))
			goto err;
		if(propsorder == DEFAULT_PROPS_AFTER_FETCH) {
			if(!TEST_true(EVP_set_default_properties(libctx, TestData_UserProperty_MYPROPERTIES)))
				goto err;
			EVP_MD_free(testprovmd);
			if(!TEST_ptr(testprovmd = EVP_MD_fetch(libctx, "testprovmd", NULL)))
				goto err;
		}
		res = 1;
	err:
		EVP_MD_free(testprovmd);
		OSSL_PROVIDER_unload(testprov);
		OSSL_LIB_CTX_free(libctx);
		return res;
	}
};
//
//
//
typedef struct cipherlist_test_fixture {
	const char * test_case_name;
	SSL_CTX * server;
	SSL_CTX * client;
} CIPHERLIST_TEST_FIXTURE;
// 
// All ciphers in the DEFAULT cipherlist meet the default security level.
// However, default supported ciphers exclude SRP and PSK ciphersuites
// for which no callbacks have been set up.
// 
// Supported ciphers also exclude TLSv1.2 ciphers if TLSv1.2 is disabled,
// and individual disabled algorithms. However, NO_RSA, NO_AES and NO_SHA
// are currently broken and should be considered mission impossible in libssl.
// 
static const uint32_t TestData_CipherList_default_ciphers_in_order[] = {
#ifndef OPENSSL_NO_TLS1_3
	TLS1_3_CK_AES_256_GCM_SHA384,
	#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
		TLS1_3_CK_CHACHA20_POLY1305_SHA256,
	#endif
	TLS1_3_CK_AES_128_GCM_SHA256,
#endif
#ifndef OPENSSL_NO_TLS1_2
	#ifndef OPENSSL_NO_EC
		TLS1_CK_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
		TLS1_CK_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
	#endif
	#ifndef OPENSSL_NO_DH
		TLS1_CK_DHE_RSA_WITH_AES_256_GCM_SHA384,
	#endif
	#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
		#ifndef OPENSSL_NO_EC
			TLS1_CK_ECDHE_ECDSA_WITH_CHACHA20_POLY1305,
			TLS1_CK_ECDHE_RSA_WITH_CHACHA20_POLY1305,
		#endif
		#ifndef OPENSSL_NO_DH
			TLS1_CK_DHE_RSA_WITH_CHACHA20_POLY1305,
		#endif
	#endif  /* !OPENSSL_NO_CHACHA && !OPENSSL_NO_POLY1305 */
	#ifndef OPENSSL_NO_EC
		TLS1_CK_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
		TLS1_CK_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
	#endif
	#ifndef OPENSSL_NO_DH
		TLS1_CK_DHE_RSA_WITH_AES_128_GCM_SHA256,
	#endif
	#ifndef OPENSSL_NO_EC
		TLS1_CK_ECDHE_ECDSA_WITH_AES_256_SHA384,
		TLS1_CK_ECDHE_RSA_WITH_AES_256_SHA384,
	#endif
	#ifndef OPENSSL_NO_DH
		TLS1_CK_DHE_RSA_WITH_AES_256_SHA256,
	#endif
	#ifndef OPENSSL_NO_EC
		TLS1_CK_ECDHE_ECDSA_WITH_AES_128_SHA256,
		TLS1_CK_ECDHE_RSA_WITH_AES_128_SHA256,
	#endif
	#ifndef OPENSSL_NO_DH
		TLS1_CK_DHE_RSA_WITH_AES_128_SHA256,
	#endif
#endif  /* !OPENSSL_NO_TLS1_2 */
#if !defined(OPENSSL_NO_TLS1_2) || defined(OPENSSL_NO_TLS1_3)
	/* These won't be usable if TLSv1.3 is available but TLSv1.2 isn't */
#ifndef OPENSSL_NO_EC
	TLS1_CK_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
	TLS1_CK_ECDHE_RSA_WITH_AES_256_CBC_SHA,
#endif
	#ifndef OPENSSL_NO_DH
	TLS1_CK_DHE_RSA_WITH_AES_256_SHA,
#endif
#ifndef OPENSSL_NO_EC
	TLS1_CK_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
	TLS1_CK_ECDHE_RSA_WITH_AES_128_CBC_SHA,
#endif
#ifndef OPENSSL_NO_DH
	TLS1_CK_DHE_RSA_WITH_AES_128_SHA,
#endif
#endif /* !defined(OPENSSL_NO_TLS1_2) || defined(OPENSSL_NO_TLS1_3) */
#ifndef OPENSSL_NO_TLS1_2
	TLS1_CK_RSA_WITH_AES_256_GCM_SHA384,
	TLS1_CK_RSA_WITH_AES_128_GCM_SHA256,
#endif
#ifndef OPENSSL_NO_TLS1_2
	TLS1_CK_RSA_WITH_AES_256_SHA256,
	TLS1_CK_RSA_WITH_AES_128_SHA256,
#endif
#if !defined(OPENSSL_NO_TLS1_2) || defined(OPENSSL_NO_TLS1_3)
	// These won't be usable if TLSv1.3 is available but TLSv1.2 isn't 
	TLS1_CK_RSA_WITH_AES_256_SHA,
	TLS1_CK_RSA_WITH_AES_128_SHA,
#endif
};

#define SETUP_CIPHERLIST_TEST_FIXTURE() SETUP_TEST_FIXTURE(CIPHERLIST_TEST_FIXTURE, set_up)
#define EXECUTE_CIPHERLIST_TEST() EXECUTE_TEST(execute_test, tear_down)

class TestInnerBlock_CipherList {
public:
	static void tear_down(CIPHERLIST_TEST_FIXTURE * fixture)
	{
		if(fixture) {
			SSL_CTX_free(fixture->server);
			SSL_CTX_free(fixture->client);
			fixture->server = fixture->client = NULL;
			OPENSSL_free(fixture);
		}
	}
	static CIPHERLIST_TEST_FIXTURE * set_up(const char * const test_case_name)
	{
		CIPHERLIST_TEST_FIXTURE * fixture;
		if(!TEST_ptr(fixture = (CIPHERLIST_TEST_FIXTURE *)OPENSSL_zalloc(sizeof(*fixture))))
			return NULL;
		fixture->test_case_name = test_case_name;
		if(!TEST_ptr(fixture->server = SSL_CTX_new(TLS_server_method())) || !TEST_ptr(fixture->client = SSL_CTX_new(TLS_client_method()))) {
			tear_down(fixture);
			return NULL;
		}
		return fixture;
	}
	static int test_default_cipherlist(SSL_CTX * ctx)
	{
		STACK_OF(SSL_CIPHER) *ciphers = NULL;
		SSL * ssl = NULL;
		int i, ret = 0, num_expected_ciphers, num_ciphers;
		uint32_t expected_cipher_id, cipher_id;
		if(!ctx)
			return 0;
		if(!TEST_ptr(ssl = SSL_new(ctx)) || !TEST_ptr(ciphers = SSL_get1_supported_ciphers(ssl)))
			goto err;
		num_expected_ciphers = SIZEOFARRAY(TestData_CipherList_default_ciphers_in_order);
		num_ciphers = sk_SSL_CIPHER_num(ciphers);
		if(!TEST_int_eq(num_ciphers, num_expected_ciphers))
			goto err;
		for(i = 0; i < num_ciphers; i++) {
			expected_cipher_id = TestData_CipherList_default_ciphers_in_order[i];
			cipher_id = SSL_CIPHER_get_id(sk_SSL_CIPHER_value(ciphers, i));
			if(!TEST_int_eq(cipher_id, expected_cipher_id)) {
				TEST_info("Wrong cipher at position %d", i);
				goto err;
			}
		}
		ret = 1;
	err:
		sk_SSL_CIPHER_free(ciphers);
		SSL_free(ssl);
		return ret;
	}
	static int execute_test(CIPHERLIST_TEST_FIXTURE * fixture)
	{
		return (fixture && test_default_cipherlist(fixture->server) && test_default_cipherlist(fixture->client));
	}
	static int test_default_cipherlist_implicit()
	{
		SETUP_CIPHERLIST_TEST_FIXTURE();
		EXECUTE_CIPHERLIST_TEST();
		return result;
	}
	static int test_default_cipherlist_explicit()
	{
		SETUP_CIPHERLIST_TEST_FIXTURE();
		if(!TEST_true(SSL_CTX_set_cipher_list(fixture->server, "DEFAULT")) || !TEST_true(SSL_CTX_set_cipher_list(fixture->client, "DEFAULT"))) {
			tear_down(fixture);
			fixture = NULL;
		}
		EXECUTE_CIPHERLIST_TEST();
		return result;
	}
	// SSL_CTX_set_cipher_list() should fail if it clears all TLSv1.2 ciphers
	static int test_default_cipherlist_clear()
	{
		SSL * s = NULL;
		SETUP_CIPHERLIST_TEST_FIXTURE();
		if(!TEST_int_eq(SSL_CTX_set_cipher_list(fixture->server, "no-such"), 0))
			goto end;
		if(!TEST_int_eq(ERR_GET_REASON(ERR_get_error()), SSL_R_NO_CIPHER_MATCH))
			goto end;
		s = SSL_new(fixture->client);
		if(!TEST_ptr(s))
			goto end;
		if(!TEST_int_eq(SSL_set_cipher_list(s, "no-such"), 0))
			goto end;
		if(!TEST_int_eq(ERR_GET_REASON(ERR_get_error()),
			SSL_R_NO_CIPHER_MATCH))
			goto end;
		result = 1;
	end:
		SSL_free(s);
		tear_down(fixture);
		return result;
	}
};
//
// CMAC low level APIs are deprecated for public use, but still ok for internal use.
//
static const char TestData_CMAC_xtskey[32] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
	0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

static struct TestData_CMAC_test_st {
	const  char key[32];
	int    key_len;
	const  uchar data[64];
	int    data_len;
	const  char * mac;
} TestData_CMAC_test[3] = {
	{ { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f }, 16, "My test data", 12, "29cec977c48f63c200bd5c4a6881b224" },
	{ { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f }, 32, "My test data", 12, "db6493aa04e4761f473b2b453c031c9a" },
	{ { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
		0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f }, 32, "My test data again", 18, "65c11c75ecf590badd0a5e56cbb8af60" },
};

class TestInnerBlock_CMAC {
public:
	static char * pt(uchar * md, uint len)
	{
		static char buf[80];
		for(uint i = 0; i < len; i++)
			sprintf(&(buf[i * 2]), "%02x", md[i]);
		return buf;
	}
	static int test_cmac_bad()
	{
		int ret = 0;
		CMAC_CTX * ctx = CMAC_CTX_new();
		if(!TEST_ptr(ctx)
			|| !TEST_false(CMAC_Init(ctx, NULL, 0, NULL, NULL))
			|| !TEST_false(CMAC_Update(ctx, TestData_CMAC_test[0].data, TestData_CMAC_test[0].data_len))
			/* Should be able to pass cipher first, and then key */
			|| !TEST_true(CMAC_Init(ctx, NULL, 0, EVP_aes_128_cbc(), NULL))
			/* Must have a key */
			|| !TEST_false(CMAC_Update(ctx, TestData_CMAC_test[0].data, TestData_CMAC_test[0].data_len))
			/* Now supply the key */
			|| !TEST_true(CMAC_Init(ctx, TestData_CMAC_test[0].key, TestData_CMAC_test[0].key_len, NULL, NULL))
			/* Update should now work */
			|| !TEST_true(CMAC_Update(ctx, TestData_CMAC_test[0].data, TestData_CMAC_test[0].data_len))
			/* XTS is not a suitable cipher to use */
			|| !TEST_false(CMAC_Init(ctx, TestData_CMAC_xtskey, sizeof(TestData_CMAC_xtskey), EVP_aes_128_xts(),
			NULL))
			|| !TEST_false(CMAC_Update(ctx, TestData_CMAC_test[0].data, TestData_CMAC_test[0].data_len)))
			goto err;
		ret = 1;
	err:
		CMAC_CTX_free(ctx);
		return ret;
	}
	static int test_cmac_run()
	{
		char * p;
		uchar buf[AES_BLOCK_SIZE];
		size_t len;
		int ret = 0;
		CMAC_CTX * ctx = CMAC_CTX_new();
		if(!TEST_true(CMAC_Init(ctx, TestData_CMAC_test[0].key, TestData_CMAC_test[0].key_len, EVP_aes_128_cbc(), NULL))
			|| !TEST_true(CMAC_Update(ctx, TestData_CMAC_test[0].data, TestData_CMAC_test[0].data_len))
			|| !TEST_true(CMAC_Final(ctx, buf, &len)))
			goto err;
		p = pt(buf, len);
		if(!TEST_str_eq(p, TestData_CMAC_test[0].mac))
			goto err;
		if(!TEST_true(CMAC_Init(ctx, TestData_CMAC_test[1].key, TestData_CMAC_test[1].key_len, EVP_aes_256_cbc(), NULL))
			|| !TEST_true(CMAC_Update(ctx, TestData_CMAC_test[1].data, TestData_CMAC_test[1].data_len))
			|| !TEST_true(CMAC_Final(ctx, buf, &len)))
			goto err;
		p = pt(buf, len);
		if(!TEST_str_eq(p, TestData_CMAC_test[1].mac))
			goto err;
		if(!TEST_true(CMAC_Init(ctx, TestData_CMAC_test[2].key, TestData_CMAC_test[2].key_len, NULL, NULL))
			|| !TEST_true(CMAC_Update(ctx, TestData_CMAC_test[2].data, TestData_CMAC_test[2].data_len))
			|| !TEST_true(CMAC_Final(ctx, buf, &len)))
			goto err;
		p = pt(buf, len);
		if(!TEST_str_eq(p, TestData_CMAC_test[2].mac))
			goto err;
		/* Test reusing a key */
		if(!TEST_true(CMAC_Init(ctx, NULL, 0, NULL, NULL))
			|| !TEST_true(CMAC_Update(ctx, TestData_CMAC_test[2].data, TestData_CMAC_test[2].data_len))
			|| !TEST_true(CMAC_Final(ctx, buf, &len)))
			goto err;
		p = pt(buf, len);
		if(!TEST_str_eq(p, TestData_CMAC_test[2].mac))
			goto err;
		/* Test setting the cipher and key separately */
		if(!TEST_true(CMAC_Init(ctx, NULL, 0, EVP_aes_256_cbc(), NULL))
			|| !TEST_true(CMAC_Init(ctx, TestData_CMAC_test[2].key, TestData_CMAC_test[2].key_len, NULL, NULL))
			|| !TEST_true(CMAC_Update(ctx, TestData_CMAC_test[2].data, TestData_CMAC_test[2].data_len))
			|| !TEST_true(CMAC_Final(ctx, buf, &len)))
			goto err;
		p = pt(buf, len);
		if(!TEST_str_eq(p, TestData_CMAC_test[2].mac))
			goto err;
		ret = 1;
	err:
		CMAC_CTX_free(ctx);
		return ret;
	}
	static int test_cmac_copy()
	{
		char * p;
		uchar buf[AES_BLOCK_SIZE];
		size_t len;
		int ret = 0;
		CMAC_CTX * ctx = CMAC_CTX_new();
		CMAC_CTX * ctx2 = CMAC_CTX_new();
		if(!TEST_ptr(ctx) || !TEST_ptr(ctx2))
			goto err;
		if(!TEST_true(CMAC_Init(ctx, TestData_CMAC_test[0].key, TestData_CMAC_test[0].key_len, EVP_aes_128_cbc(), NULL))
			|| !TEST_true(CMAC_Update(ctx, TestData_CMAC_test[0].data, TestData_CMAC_test[0].data_len))
			|| !TEST_true(CMAC_CTX_copy(ctx2, ctx))
			|| !TEST_true(CMAC_Final(ctx2, buf, &len)))
			goto err;
		p = pt(buf, len);
		if(!TEST_str_eq(p, TestData_CMAC_test[0].mac))
			goto err;
		ret = 1;
	err:
		CMAC_CTX_free(ctx2);
		CMAC_CTX_free(ctx);
		return ret;
	}
};
//
// HMAC low level APIs are deprecated for public use, but still ok for internal use.
//
#ifndef OPENSSL_NO_MD5
static struct TestData_HMAC_test_st {
	const  char * key; // @sobolev key[16]--> * key
	int    key_len;
	const  uchar data[64];
	int    data_len;
	const  char * digest;
} TestData_HMAC_test[8] = {
	{ "", 0, "More text test vectors to stuff up EBCDIC machines :-)", 54, "e9139d1e6ee064ef8cf514fc7dc83e86", },
	{ "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b", 16, "Hi There", 8, "9294727a3638bb1c13f48ef8158bfc9d", },
	{ "Jefe", 4, "what do ya want for nothing?", 28, "750c783e6ab0b503eaa86e310a5db738", },
	{ "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa", 16, {
			0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
			0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
			0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
			0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
			0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd
		}, 50, "56be34521d144c88dbb8c733f0e8b3f6",
	},
	{ "", 0, "My test data", 12, "61afdecb95429ef494d61fdee15990cabf0826fc" },
	{ "", 0, "My test data", 12, "2274b195d90ce8e03406f4b526a47e0787a88a65479938f1a5baa3ce0f079776" },
	{ "123456", 6, "My test data", 12, "bab53058ae861a7f191abe2d0145cbb123776a6369ee3f9d79ce455667e411dd" },
	{ "12345", 5, "My test data again", 18, "a12396ceddd2a85f4c656bc1e0aa50c78cffde3e" }
};
#endif

class TestInnerBlock_HMAC {
public:
	#ifndef OPENSSL_NO_MD5
	static char * pt(uchar * md, uint len)
	{
		uint i;
		static char buf[80];
		if(!md)
			return NULL;
		for(i = 0; i < len; i++)
			sprintf(&(buf[i * 2]), "%02x", md[i]);
		return buf;
	}
	static int test_hmac_md5(int idx)
	{
		char * p;
	#ifdef CHARSET_EBCDIC
		ebcdic2ascii(test[0].data, test[0].data, test[0].data_len);
		ebcdic2ascii(test[1].data, test[1].data, test[1].data_len);
		ebcdic2ascii(test[2].key, test[2].key, test[2].key_len);
		ebcdic2ascii(test[2].data, test[2].data, test[2].data_len);
	#endif
		p = pt(HMAC(EVP_md5(), TestData_HMAC_test[idx].key, TestData_HMAC_test[idx].key_len, TestData_HMAC_test[idx].data, TestData_HMAC_test[idx].data_len, NULL, NULL), MD5_DIGEST_LENGTH);
		return TEST_ptr(p) && TEST_str_eq(p, TestData_HMAC_test[idx].digest);
	}
	#endif
	static int test_hmac_bad()
	{
		int ret = 0;
		HMAC_CTX * ctx = HMAC_CTX_new();
		if(!TEST_ptr(ctx)
			|| !TEST_ptr_null(HMAC_CTX_get_md(ctx))
			|| !TEST_false(HMAC_Init_ex(ctx, NULL, 0, NULL, NULL))
			|| !TEST_false(HMAC_Update(ctx, TestData_HMAC_test[4].data, TestData_HMAC_test[4].data_len))
			|| !TEST_false(HMAC_Init_ex(ctx, NULL, 0, EVP_sha1(), NULL))
			|| !TEST_false(HMAC_Update(ctx, TestData_HMAC_test[4].data, TestData_HMAC_test[4].data_len)))
			goto err;
		ret = 1;
	err:
		HMAC_CTX_free(ctx);
		return ret;
	}
	static int test_hmac_run()
	{
		char * p;
		HMAC_CTX * ctx = NULL;
		uchar buf[EVP_MAX_MD_SIZE];
		uint len;
		int ret = 0;
		if(!TEST_ptr(ctx = HMAC_CTX_new()))
			return 0;
		HMAC_CTX_reset(ctx);
		if(!TEST_ptr(ctx)
			|| !TEST_ptr_null(HMAC_CTX_get_md(ctx))
			|| !TEST_false(HMAC_Init_ex(ctx, NULL, 0, NULL, NULL))
			|| !TEST_false(HMAC_Update(ctx, TestData_HMAC_test[4].data, TestData_HMAC_test[4].data_len))
			|| !TEST_false(HMAC_Init_ex(ctx, TestData_HMAC_test[4].key, -1, EVP_sha1(), NULL)))
			goto err;
		if(!TEST_true(HMAC_Init_ex(ctx, TestData_HMAC_test[4].key, TestData_HMAC_test[4].key_len, EVP_sha1(), NULL))
			|| !TEST_true(HMAC_Update(ctx, TestData_HMAC_test[4].data, TestData_HMAC_test[4].data_len))
			|| !TEST_true(HMAC_Final(ctx, buf, &len)))
			goto err;
		p = pt(buf, len);
		if(!TEST_ptr(p) || !TEST_str_eq(p, TestData_HMAC_test[4].digest))
			goto err;
		if(!TEST_false(HMAC_Init_ex(ctx, NULL, 0, EVP_sha256(), NULL)))
			goto err;
		if(!TEST_true(HMAC_Init_ex(ctx, TestData_HMAC_test[5].key, TestData_HMAC_test[5].key_len, EVP_sha256(), NULL))
			|| !TEST_ptr_eq(HMAC_CTX_get_md(ctx), EVP_sha256())
			|| !TEST_true(HMAC_Update(ctx, TestData_HMAC_test[5].data, TestData_HMAC_test[5].data_len))
			|| !TEST_true(HMAC_Final(ctx, buf, &len)))
			goto err;
		p = pt(buf, len);
		if(!TEST_ptr(p) || !TEST_str_eq(p, TestData_HMAC_test[5].digest))
			goto err;
		if(!TEST_true(HMAC_Init_ex(ctx, TestData_HMAC_test[6].key, TestData_HMAC_test[6].key_len, NULL, NULL))
			|| !TEST_true(HMAC_Update(ctx, TestData_HMAC_test[6].data, TestData_HMAC_test[6].data_len))
			|| !TEST_true(HMAC_Final(ctx, buf, &len)))
			goto err;
		p = pt(buf, len);
		if(!TEST_ptr(p) || !TEST_str_eq(p, TestData_HMAC_test[6].digest))
			goto err;
		// Test reusing a key 
		if(!TEST_true(HMAC_Init_ex(ctx, NULL, 0, NULL, NULL))
			|| !TEST_true(HMAC_Update(ctx, TestData_HMAC_test[6].data, TestData_HMAC_test[6].data_len))
			|| !TEST_true(HMAC_Final(ctx, buf, &len)))
			goto err;
		p = pt(buf, len);
		if(!TEST_ptr(p) || !TEST_str_eq(p, TestData_HMAC_test[6].digest))
			goto err;
		//
		// Test reusing a key where the digest is provided again but is the same as last time
		//
		if(!TEST_true(HMAC_Init_ex(ctx, NULL, 0, EVP_sha256(), NULL))
			|| !TEST_true(HMAC_Update(ctx, TestData_HMAC_test[6].data, TestData_HMAC_test[6].data_len))
			|| !TEST_true(HMAC_Final(ctx, buf, &len)))
			goto err;
		p = pt(buf, len);
		if(!TEST_ptr(p) || !TEST_str_eq(p, TestData_HMAC_test[6].digest))
			goto err;
		ret = 1;
	err:
		HMAC_CTX_free(ctx);
		return ret;
	}
	static int test_hmac_single_shot()
	{
		// Test single-shot with NULL key
		char * p = pt(HMAC(EVP_sha1(), NULL, 0, TestData_HMAC_test[4].data, TestData_HMAC_test[4].data_len, NULL, NULL), SHA_DIGEST_LENGTH);
		if(!TEST_ptr(p) || !TEST_str_eq(p, TestData_HMAC_test[4].digest))
			return 0;
		return 1;
	}
	static int test_hmac_copy()
	{
		char * p;
		uchar buf[EVP_MAX_MD_SIZE];
		uint len;
		int ret = 0;
		HMAC_CTX * ctx = HMAC_CTX_new();
		HMAC_CTX * ctx2 = HMAC_CTX_new();
		if(!TEST_ptr(ctx) || !TEST_ptr(ctx2))
			goto err;
		if(!TEST_true(HMAC_Init_ex(ctx, TestData_HMAC_test[7].key, TestData_HMAC_test[7].key_len, EVP_sha1(), NULL))
			|| !TEST_true(HMAC_Update(ctx, TestData_HMAC_test[7].data, TestData_HMAC_test[7].data_len))
			|| !TEST_true(HMAC_CTX_copy(ctx2, ctx))
			|| !TEST_true(HMAC_Final(ctx2, buf, &len)))
			goto err;
		p = pt(buf, len);
		if(!TEST_ptr(p) || !TEST_str_eq(p, TestData_HMAC_test[7].digest))
			goto err;
		ret = 1;
	err:
		HMAC_CTX_free(ctx2);
		HMAC_CTX_free(ctx);
		return ret;
	}
	static int test_hmac_copy_uninited()
	{
		const uchar key[24] = {0};
		const uchar ct[166] = {0};
		EVP_PKEY * pkey = NULL;
		EVP_MD_CTX * ctx = NULL;
		EVP_MD_CTX * ctx_tmp = NULL;
		int res = 0;
		if(!TEST_ptr(ctx = EVP_MD_CTX_new())
			|| !TEST_ptr(pkey = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, key, sizeof(key)))
			|| !TEST_true(EVP_DigestSignInit(ctx, NULL, EVP_sha1(), NULL, pkey))
			|| !TEST_ptr(ctx_tmp = EVP_MD_CTX_new())
			|| !TEST_true(EVP_MD_CTX_copy(ctx_tmp, ctx)))
			goto err;
		EVP_MD_CTX_free(ctx);
		ctx = ctx_tmp;
		ctx_tmp = NULL;
		if(!TEST_true(EVP_DigestSignUpdate(ctx, ct, sizeof(ct))))
			goto err;
		res = 1;
	err:
		EVP_MD_CTX_free(ctx);
		EVP_MD_CTX_free(ctx_tmp);
		EVP_PKEY_free(pkey);
		return res;
	}
};
#ifndef OPENSSL_NO_IDEA
// 
// IDEA low level APIs are deprecated for public use, but still ok for internal
// use where we're using them to implement the higher level EVP interface, as is the case here.
// 
static const uchar TestData_Idea_k[16] = { 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x08 };
static const uchar TestData_Idea_in[8] = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03 };
static const uchar TestData_Idea_c[8] = { 0x11, 0xFB, 0xED, 0x2B, 0x01, 0x98, 0x6D, 0xE5 };
static uchar TestData_Idea_out[80];
static const uchar TestData_Idea_text[] = "Hello to all people out there";
static const uchar TestData_Idea_cfb_key[16] = { 0xe1, 0xf0, 0xc3, 0xd2, 0xa5, 0xb4, 0x87, 0x96, 0x69, 0x78, 0x4b, 0x5a, 0x2d, 0x3c, 0x0f, 0x1e, };
static const uchar TestData_Idea_cfb_iv[80] = { 0x34, 0x12, 0x78, 0x56, 0xab, 0x90, 0xef, 0xcd };
static uchar TestData_Idea_cfb_buf1[40];
static uchar TestData_Idea_cfb_buf2[40];
static uchar TestData_Idea_cfb_tmp[8];
#define CFB_TEST_SIZE 24
static const uchar TestData_Idea_plain[CFB_TEST_SIZE] = {
	0x4e, 0x6f, 0x77, 0x20, 0x69, 0x73,
	0x20, 0x74, 0x68, 0x65, 0x20, 0x74,
	0x69, 0x6d, 0x65, 0x20, 0x66, 0x6f,
	0x72, 0x20, 0x61, 0x6c, 0x6c, 0x20
};

static const uchar TestData_Idea_cfb_cipher64[CFB_TEST_SIZE] = {
	0x59, 0xD8, 0xE2, 0x65, 0x00, 0x58, 0x6C, 0x3F,
	0x2C, 0x17, 0x25, 0xD0, 0x1A, 0x38, 0xB7, 0x2A,
	0x39, 0x61, 0x37, 0xDC, 0x79, 0xFB, 0x9F, 0x45
/*- 0xF9,0x78,0x32,0xB5,0x42,0x1A,0x6B,0x38,
    0x9A,0x44,0xD6,0x04,0x19,0x43,0xC4,0xD9,
    0x3D,0x1E,0xAE,0x47,0xFC,0xCF,0x29,0x0B,*/
};

class TestInnerBlock_Idea {
public:
	static int test_idea_ecb()
	{
		IDEA_KEY_SCHEDULE key, dkey;
		IDEA_set_encrypt_key(TestData_Idea_k, &key);
		IDEA_ecb_encrypt(TestData_Idea_in, TestData_Idea_out, &key);
		if(!TEST_mem_eq(TestData_Idea_out, IDEA_BLOCK, TestData_Idea_c, sizeof(TestData_Idea_c)))
			return 0;
		IDEA_set_decrypt_key(&key, &dkey);
		IDEA_ecb_encrypt(TestData_Idea_c, TestData_Idea_out, &dkey);
		return TEST_mem_eq(TestData_Idea_out, IDEA_BLOCK, TestData_Idea_in, sizeof(TestData_Idea_in));
	}
	static int test_idea_cbc()
	{
		IDEA_KEY_SCHEDULE key, dkey;
		uchar iv[IDEA_BLOCK];
		const size_t text_len = sizeof(TestData_Idea_text);
		IDEA_set_encrypt_key(TestData_Idea_k, &key);
		IDEA_set_decrypt_key(&key, &dkey);
		memcpy(iv, TestData_Idea_k, sizeof(iv));
		IDEA_cbc_encrypt(TestData_Idea_text, TestData_Idea_out, text_len, &key, iv, 1);
		memcpy(iv, TestData_Idea_k, sizeof(iv));
		IDEA_cbc_encrypt(TestData_Idea_out, TestData_Idea_out, IDEA_BLOCK, &dkey, iv, 0);
		IDEA_cbc_encrypt(&TestData_Idea_out[8], &TestData_Idea_out[8], text_len - 8, &dkey, iv, 0);
		return TEST_mem_eq(TestData_Idea_text, text_len, TestData_Idea_out, text_len);
	}
	static int test_idea_cfb64()
	{
		IDEA_KEY_SCHEDULE eks, dks;
		int n;
		IDEA_set_encrypt_key(TestData_Idea_cfb_key, &eks);
		IDEA_set_decrypt_key(&eks, &dks);
		memcpy(TestData_Idea_cfb_tmp, TestData_Idea_cfb_iv, sizeof(TestData_Idea_cfb_tmp));
		n = 0;
		IDEA_cfb64_encrypt(TestData_Idea_plain, TestData_Idea_cfb_buf1, (long)12, &eks, TestData_Idea_cfb_tmp, &n, IDEA_ENCRYPT);
		IDEA_cfb64_encrypt(&TestData_Idea_plain[12], &TestData_Idea_cfb_buf1[12], (long)CFB_TEST_SIZE - 12, &eks, TestData_Idea_cfb_tmp, &n, IDEA_ENCRYPT);
		if(!TEST_mem_eq(TestData_Idea_cfb_cipher64, CFB_TEST_SIZE, TestData_Idea_cfb_buf1, CFB_TEST_SIZE))
			return 0;
		memcpy(TestData_Idea_cfb_tmp, TestData_Idea_cfb_iv, sizeof(TestData_Idea_cfb_tmp));
		n = 0;
		IDEA_cfb64_encrypt(TestData_Idea_cfb_buf1, TestData_Idea_cfb_buf2, (long)13, &eks, TestData_Idea_cfb_tmp, &n, IDEA_DECRYPT);
		IDEA_cfb64_encrypt(&TestData_Idea_cfb_buf1[13], &TestData_Idea_cfb_buf2[13], (long)CFB_TEST_SIZE - 13, &eks, TestData_Idea_cfb_tmp, &n, IDEA_DECRYPT);
		return TEST_mem_eq(TestData_Idea_plain, CFB_TEST_SIZE, TestData_Idea_cfb_buf2, CFB_TEST_SIZE);
	}
};
#endif
//
// The AES_ige_* functions are deprecated, so we suppress warnings about them 
//
#ifndef OPENSSL_NO_DEPRECATED_3_0

#define TestData_Ige_TEST_SIZE       128
#define TestData_Ige_BIG_TEST_SIZE 10240
#if TestData_Ige_BIG_TEST_SIZE < TestData_Ige_TEST_SIZE
	#error TestData_Ige_BIG_TEST_SIZE is smaller than TestData_Ige_TEST_SIZE
#endif

static uchar TestData_Ige_rkey[16];
static uchar TestData_Ige_rkey2[16];
static uchar TestData_Ige_plaintext[TestData_Ige_BIG_TEST_SIZE];
static uchar TestData_Ige_saved_iv[AES_BLOCK_SIZE * 4];

#define TestData_Ige_MAX_VECTOR_SIZE 64

struct ige_test {
	const uchar key[16];
	const uchar iv[32];
	const uchar in[TestData_Ige_MAX_VECTOR_SIZE];
	const uchar out[TestData_Ige_MAX_VECTOR_SIZE];
	const size_t length;
	const int encrypt;
};

static struct ige_test const TestData_Ige_ige_test_vectors[] = {
	{{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f}, /* key */
	 {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f}, /* iv */
	 {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* in */
	 {0x1a, 0x85, 0x19, 0xa6, 0x55, 0x7b, 0xe6, 0x52, 0xe9, 0xda, 0x8e, 0x43, 0xda, 0x4e, 0xf4, 0x45,
      0x3c, 0xf4, 0x56, 0xb4, 0xca, 0x48, 0x8a, 0xa3, 0x83, 0xc7, 0x9c, 0x98, 0xb3, 0x47, 0x97, 0xcb}, /* out */
	 32, AES_ENCRYPT},      /* test vector 0 */

	{{0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
	  0x61, 0x6e, 0x20, 0x69, 0x6d, 0x70, 0x6c, 0x65}, /* key */
	 {0x6d, 0x65, 0x6e, 0x74, 0x61, 0x74, 0x69, 0x6f,
      0x6e, 0x20, 0x6f, 0x66, 0x20, 0x49, 0x47, 0x45,
      0x20, 0x6d, 0x6f, 0x64, 0x65, 0x20, 0x66, 0x6f,
      0x72, 0x20, 0x4f, 0x70, 0x65, 0x6e, 0x53, 0x53}, /* iv */
	 {0x4c, 0x2e, 0x20, 0x4c, 0x65, 0x74, 0x27, 0x73,
      0x20, 0x68, 0x6f, 0x70, 0x65, 0x20, 0x42, 0x65,
      0x6e, 0x20, 0x67, 0x6f, 0x74, 0x20, 0x69, 0x74,
      0x20, 0x72, 0x69, 0x67, 0x68, 0x74, 0x21, 0x0a}, /* in */
	 {0x99, 0x70, 0x64, 0x87, 0xa1, 0xcd, 0xe6, 0x13,
      0xbc, 0x6d, 0xe0, 0xb6, 0xf2, 0x4b, 0x1c, 0x7a,
      0xa4, 0x48, 0xc8, 0xb9, 0xc3, 0x40, 0x3e, 0x34,
      0x67, 0xa8, 0xca, 0xd8, 0x93, 0x40, 0xf5, 0x3b}, /* out */
	 32, AES_DECRYPT},      /* test vector 1 */
};

struct bi_ige_test {
	const uchar key1[32];
	const uchar key2[32];
	const uchar iv[64];
	const uchar in[TestData_Ige_MAX_VECTOR_SIZE];
	const uchar out[TestData_Ige_MAX_VECTOR_SIZE];
	const size_t keysize;
	const size_t length;
	const int encrypt;
};

static struct bi_ige_test const TestData_Ige_bi_ige_test_vectors[] = {
	{{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f}, /* key1 */
	 {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f}, /* key2 */
	 {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
      0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
      0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f}, /* iv */
	 {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* in */
	 {0x14, 0x40, 0x6f, 0xae, 0xa2, 0x79, 0xf2, 0x56,
      0x1f, 0x86, 0xeb, 0x3b, 0x7d, 0xff, 0x53, 0xdc,
      0x4e, 0x27, 0x0c, 0x03, 0xde, 0x7c, 0xe5, 0x16,
      0x6a, 0x9c, 0x20, 0x33, 0x9d, 0x33, 0xfe, 0x12}, /* out */
	 16, 32, AES_ENCRYPT},  /* test vector 0 */
	{{0x58, 0x0a, 0x06, 0xe9, 0x97, 0x07, 0x59, 0x5c,
	  0x9e, 0x19, 0xd2, 0xa7, 0xbb, 0x40, 0x2b, 0x7a,
	  0xc7, 0xd8, 0x11, 0x9e, 0x4c, 0x51, 0x35, 0x75,
	  0x64, 0x28, 0x0f, 0x23, 0xad, 0x74, 0xac, 0x37}, /* key1 */
	 {0xd1, 0x80, 0xa0, 0x31, 0x47, 0xa3, 0x11, 0x13,
      0x86, 0x26, 0x9e, 0x6d, 0xff, 0xaf, 0x72, 0x74,
      0x5b, 0xa2, 0x35, 0x81, 0xd2, 0xa6, 0x3d, 0x21,
      0x67, 0x7b, 0x58, 0xa8, 0x18, 0xf9, 0x72, 0xe4}, /* key2 */
	 {0x80, 0x3d, 0xbd, 0x4c, 0xe6, 0x7b, 0x06, 0xa9,
      0x53, 0x35, 0xd5, 0x7e, 0x71, 0xc1, 0x70, 0x70,
      0x74, 0x9a, 0x00, 0x28, 0x0c, 0xbf, 0x6c, 0x42,
      0x9b, 0xa4, 0xdd, 0x65, 0x11, 0x77, 0x7c, 0x67,
      0xfe, 0x76, 0x0a, 0xf0, 0xd5, 0xc6, 0x6e, 0x6a,
      0xe7, 0x5e, 0x4c, 0xf2, 0x7e, 0x9e, 0xf9, 0x20,
      0x0e, 0x54, 0x6f, 0x2d, 0x8a, 0x8d, 0x7e, 0xbd,
      0x48, 0x79, 0x37, 0x99, 0xff, 0x27, 0x93, 0xa3}, /* iv */
	 {0xf1, 0x54, 0x3d, 0xca, 0xfe, 0xb5, 0xef, 0x1c,
      0x4f, 0xa6, 0x43, 0xf6, 0xe6, 0x48, 0x57, 0xf0,
      0xee, 0x15, 0x7f, 0xe3, 0xe7, 0x2f, 0xd0, 0x2f,
      0x11, 0x95, 0x7a, 0x17, 0x00, 0xab, 0xa7, 0x0b,
      0xbe, 0x44, 0x09, 0x9c, 0xcd, 0xac, 0xa8, 0x52,
      0xa1, 0x8e, 0x7b, 0x75, 0xbc, 0xa4, 0x92, 0x5a,
      0xab, 0x46, 0xd3, 0x3a, 0xa0, 0xd5, 0x35, 0x1c,
      0x55, 0xa4, 0xb3, 0xa8, 0x40, 0x81, 0xa5, 0x0b}, /* in */
	 {0x42, 0xe5, 0x28, 0x30, 0x31, 0xc2, 0xa0, 0x23,
      0x68, 0x49, 0x4e, 0xb3, 0x24, 0x59, 0x92, 0x79,
      0xc1, 0xa5, 0xcc, 0xe6, 0x76, 0x53, 0xb1, 0xcf,
      0x20, 0x86, 0x23, 0xe8, 0x72, 0x55, 0x99, 0x92,
      0x0d, 0x16, 0x1c, 0x5a, 0x2f, 0xce, 0xcb, 0x51,
      0xe2, 0x67, 0xfa, 0x10, 0xec, 0xcd, 0x3d, 0x67,
      0xa5, 0xe6, 0xf7, 0x31, 0x26, 0xb0, 0x0d, 0x76,
      0x5e, 0x28, 0xdc, 0x7f, 0x01, 0xc5, 0xa5, 0x4c}, /* out */
	 32, 64, AES_ENCRYPT},  /* test vector 1 */
};

class TestInnerBlock_Ige {
public:
	static int test_ige_vectors(int n)
	{
		const struct ige_test * const v = &TestData_Ige_ige_test_vectors[n];
		AES_KEY key;
		uchar buf[TestData_Ige_MAX_VECTOR_SIZE];
		uchar iv[AES_BLOCK_SIZE * 2];
		int testresult = 1;
		if(!TEST_int_le(v->length, TestData_Ige_MAX_VECTOR_SIZE))
			return 0;
		if(v->encrypt == AES_ENCRYPT)
			AES_set_encrypt_key(v->key, 8 * sizeof(v->key), &key);
		else
			AES_set_decrypt_key(v->key, 8 * sizeof(v->key), &key);
		memcpy(iv, v->iv, sizeof(iv));
		AES_ige_encrypt(v->in, buf, v->length, &key, iv, v->encrypt);
		if(!TEST_mem_eq(v->out, v->length, buf, v->length)) {
			TEST_info("IGE test vector %d failed", n);
			test_output_memory("key", v->key, sizeof(v->key));
			test_output_memory("iv", v->iv, sizeof(v->iv));
			test_output_memory("in", v->in, v->length);
			testresult = 0;
		}
		/* try with in == out */
		memcpy(iv, v->iv, sizeof(iv));
		memcpy(buf, v->in, v->length);
		AES_ige_encrypt(buf, buf, v->length, &key, iv, v->encrypt);
		if(!TEST_mem_eq(v->out, v->length, buf, v->length)) {
			TEST_info("IGE test vector %d failed (with in == out)", n);
			test_output_memory("key", v->key, sizeof(v->key));
			test_output_memory("iv", v->iv, sizeof(v->iv));
			test_output_memory("in", v->in, v->length);
			testresult = 0;
		}
		return testresult;
	}

	static int test_bi_ige_vectors(int n)
	{
		const struct bi_ige_test * const v = &TestData_Ige_bi_ige_test_vectors[n];
		AES_KEY key1;
		AES_KEY key2;
		uchar buf[TestData_Ige_MAX_VECTOR_SIZE];
		if(!TEST_int_le(v->length, TestData_Ige_MAX_VECTOR_SIZE))
			return 0;
		if(v->encrypt == AES_ENCRYPT) {
			AES_set_encrypt_key(v->key1, 8 * v->keysize, &key1);
			AES_set_encrypt_key(v->key2, 8 * v->keysize, &key2);
		}
		else {
			AES_set_decrypt_key(v->key1, 8 * v->keysize, &key1);
			AES_set_decrypt_key(v->key2, 8 * v->keysize, &key2);
		}
		AES_bi_ige_encrypt(v->in, buf, v->length, &key1, &key2, v->iv, v->encrypt);
		if(!TEST_mem_eq(v->out, v->length, buf, v->length)) {
			test_output_memory("key 1", v->key1, sizeof(v->key1));
			test_output_memory("key 2", v->key2, sizeof(v->key2));
			test_output_memory("iv", v->iv, sizeof(v->iv));
			test_output_memory("in", v->in, v->length);
			return 0;
		}
		return 1;
	}
	static int test_ige_enc_dec()
	{
		AES_KEY key;
		uchar iv[AES_BLOCK_SIZE * 4];
		uchar ciphertext[TestData_Ige_BIG_TEST_SIZE];
		uchar checktext[TestData_Ige_BIG_TEST_SIZE];
		memcpy(iv, TestData_Ige_saved_iv, sizeof(iv));
		AES_set_encrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		AES_ige_encrypt(TestData_Ige_plaintext, ciphertext, TestData_Ige_TEST_SIZE, &key, iv, AES_ENCRYPT);
		AES_set_decrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		memcpy(iv, TestData_Ige_saved_iv, sizeof(iv));
		AES_ige_encrypt(ciphertext, checktext, TestData_Ige_TEST_SIZE, &key, iv, AES_DECRYPT);
		return TEST_mem_eq(checktext, TestData_Ige_TEST_SIZE, TestData_Ige_plaintext, TestData_Ige_TEST_SIZE);
	}
	static int test_ige_enc_chaining()
	{
		AES_KEY key;
		uchar iv[AES_BLOCK_SIZE * 4];
		uchar ciphertext[TestData_Ige_BIG_TEST_SIZE];
		uchar checktext[TestData_Ige_BIG_TEST_SIZE];
		AES_set_encrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		memcpy(iv, TestData_Ige_saved_iv, sizeof(iv));
		AES_ige_encrypt(TestData_Ige_plaintext, ciphertext, TestData_Ige_TEST_SIZE / 2, &key, iv, AES_ENCRYPT);
		AES_ige_encrypt(TestData_Ige_plaintext + TestData_Ige_TEST_SIZE / 2, ciphertext + TestData_Ige_TEST_SIZE / 2, TestData_Ige_TEST_SIZE / 2, &key, iv, AES_ENCRYPT);
		AES_set_decrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		memcpy(iv, TestData_Ige_saved_iv, sizeof(iv));
		AES_ige_encrypt(ciphertext, checktext, TestData_Ige_TEST_SIZE, &key, iv, AES_DECRYPT);
		return TEST_mem_eq(checktext, TestData_Ige_TEST_SIZE, TestData_Ige_plaintext, TestData_Ige_TEST_SIZE);
	}
	static int test_ige_dec_chaining()
	{
		AES_KEY key;
		uchar iv[AES_BLOCK_SIZE * 4];
		uchar ciphertext[TestData_Ige_BIG_TEST_SIZE];
		uchar checktext[TestData_Ige_BIG_TEST_SIZE];
		AES_set_encrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		memcpy(iv, TestData_Ige_saved_iv, sizeof(iv));
		AES_ige_encrypt(TestData_Ige_plaintext, ciphertext, TestData_Ige_TEST_SIZE / 2, &key, iv, AES_ENCRYPT);
		AES_ige_encrypt(TestData_Ige_plaintext + TestData_Ige_TEST_SIZE / 2, ciphertext + TestData_Ige_TEST_SIZE / 2, TestData_Ige_TEST_SIZE / 2, &key, iv, AES_ENCRYPT);
		AES_set_decrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		memcpy(iv, TestData_Ige_saved_iv, sizeof(iv));
		AES_ige_encrypt(ciphertext, checktext, TestData_Ige_TEST_SIZE / 2, &key, iv, AES_DECRYPT);
		AES_ige_encrypt(ciphertext + TestData_Ige_TEST_SIZE / 2, checktext + TestData_Ige_TEST_SIZE / 2, TestData_Ige_TEST_SIZE / 2, &key, iv, AES_DECRYPT);
		return TEST_mem_eq(checktext, TestData_Ige_TEST_SIZE, TestData_Ige_plaintext, TestData_Ige_TEST_SIZE);
	}
	static int test_ige_garble_forwards()
	{
		AES_KEY key;
		uchar iv[AES_BLOCK_SIZE * 4];
		uchar ciphertext[TestData_Ige_BIG_TEST_SIZE];
		uchar checktext[TestData_Ige_BIG_TEST_SIZE];
		uint n;
		int testresult = 1;
		const size_t ctsize = sizeof(checktext);
		size_t matches;
		AES_set_encrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		memcpy(iv, TestData_Ige_saved_iv, sizeof(iv));
		AES_ige_encrypt(TestData_Ige_plaintext, ciphertext, sizeof(TestData_Ige_plaintext), &key, iv, AES_ENCRYPT);
		// corrupt halfway through 
		++ciphertext[sizeof(ciphertext) / 2];
		AES_set_decrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		memcpy(iv, TestData_Ige_saved_iv, sizeof(iv));
		AES_ige_encrypt(ciphertext, checktext, sizeof(checktext), &key, iv, AES_DECRYPT);
		matches = 0;
		for(n = 0; n < sizeof(checktext); ++n)
			if(checktext[n] == TestData_Ige_plaintext[n])
				++matches;
		/* Fail if there is more than 51% matching bytes */
		if(!TEST_size_t_le(matches, ctsize / 2 + ctsize / 100))
			testresult = 0;
		/* Fail if the garble goes backwards */
		if(!TEST_size_t_gt(matches, ctsize / 2))
			testresult = 0;
		return testresult;
	}
	static int test_bi_ige_enc_dec()
	{
		AES_KEY key, key2;
		uchar iv[AES_BLOCK_SIZE * 4];
		uchar ciphertext[TestData_Ige_BIG_TEST_SIZE];
		uchar checktext[TestData_Ige_BIG_TEST_SIZE];
		memcpy(iv, TestData_Ige_saved_iv, sizeof(iv));
		AES_set_encrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		AES_set_encrypt_key(TestData_Ige_rkey2, 8 * sizeof(TestData_Ige_rkey2), &key2);
		AES_bi_ige_encrypt(TestData_Ige_plaintext, ciphertext, TestData_Ige_TEST_SIZE, &key, &key2, iv, AES_ENCRYPT);
		AES_set_decrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		AES_set_decrypt_key(TestData_Ige_rkey2, 8 * sizeof(TestData_Ige_rkey2), &key2);
		AES_bi_ige_encrypt(ciphertext, checktext, TestData_Ige_TEST_SIZE, &key, &key2, iv, AES_DECRYPT);
		return TEST_mem_eq(checktext, TestData_Ige_TEST_SIZE, TestData_Ige_plaintext, TestData_Ige_TEST_SIZE);
	}
	static int test_bi_ige_garble1()
	{
		AES_KEY key, key2;
		uchar iv[AES_BLOCK_SIZE * 4];
		uchar ciphertext[TestData_Ige_BIG_TEST_SIZE];
		uchar checktext[TestData_Ige_BIG_TEST_SIZE];
		uint n;
		size_t matches;
		memcpy(iv, TestData_Ige_saved_iv, sizeof(iv));
		AES_set_encrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		AES_set_encrypt_key(TestData_Ige_rkey2, 8 * sizeof(TestData_Ige_rkey2), &key2);
		AES_ige_encrypt(TestData_Ige_plaintext, ciphertext, sizeof(TestData_Ige_plaintext), &key, iv, AES_ENCRYPT);
		/* corrupt halfway through */
		++ciphertext[sizeof(ciphertext) / 2];
		AES_set_decrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		AES_set_decrypt_key(TestData_Ige_rkey2, 8 * sizeof(TestData_Ige_rkey2), &key2);
		AES_ige_encrypt(ciphertext, checktext, sizeof(checktext), &key, iv, AES_DECRYPT);
		matches = 0;
		for(n = 0; n < sizeof(checktext); ++n)
			if(checktext[n] == TestData_Ige_plaintext[n])
				++matches;
		return TEST_size_t_le(matches, sizeof(checktext) / 100); // Fail if there is more than 1% matching bytes 
	}
	static int test_bi_ige_garble2()
	{
		AES_KEY key, key2;
		uchar iv[AES_BLOCK_SIZE * 4];
		uchar ciphertext[TestData_Ige_BIG_TEST_SIZE];
		uchar checktext[TestData_Ige_BIG_TEST_SIZE];
		uint n;
		size_t matches;
		memcpy(iv, TestData_Ige_saved_iv, sizeof(iv));
		AES_set_encrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		AES_set_encrypt_key(TestData_Ige_rkey2, 8 * sizeof(TestData_Ige_rkey2), &key2);
		AES_ige_encrypt(TestData_Ige_plaintext, ciphertext, sizeof(TestData_Ige_plaintext), &key, iv, AES_ENCRYPT);
		// corrupt right at the end 
		++ciphertext[sizeof(ciphertext) - 1];
		AES_set_decrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		AES_set_decrypt_key(TestData_Ige_rkey2, 8 * sizeof(TestData_Ige_rkey2), &key2);
		AES_ige_encrypt(ciphertext, checktext, sizeof(checktext), &key, iv, AES_DECRYPT);
		matches = 0;
		for(n = 0; n < sizeof(checktext); ++n)
			if(checktext[n] == TestData_Ige_plaintext[n])
				++matches;
		return TEST_size_t_le(matches, sizeof(checktext) / 100); // Fail if there is more than 1% matching bytes 
	}
	static int test_bi_ige_garble3()
	{
		AES_KEY key, key2;
		uchar iv[AES_BLOCK_SIZE * 4];
		uchar ciphertext[TestData_Ige_BIG_TEST_SIZE];
		uchar checktext[TestData_Ige_BIG_TEST_SIZE];
		uint n;
		size_t matches;
		memcpy(iv, TestData_Ige_saved_iv, sizeof(iv));
		AES_set_encrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		AES_set_encrypt_key(TestData_Ige_rkey2, 8 * sizeof(TestData_Ige_rkey2), &key2);
		AES_ige_encrypt(TestData_Ige_plaintext, ciphertext, sizeof(TestData_Ige_plaintext), &key, iv, AES_ENCRYPT);
		// corrupt right at the start 
		++ciphertext[0];
		AES_set_decrypt_key(TestData_Ige_rkey, 8 * sizeof(TestData_Ige_rkey), &key);
		AES_set_decrypt_key(TestData_Ige_rkey2, 8 * sizeof(TestData_Ige_rkey2), &key2);
		AES_ige_encrypt(ciphertext, checktext, sizeof(checktext), &key, iv, AES_DECRYPT);
		matches = 0;
		for(n = 0; n < sizeof(checktext); ++n)
			if(checktext[n] == TestData_Ige_plaintext[n])
				++matches;
		return TEST_size_t_le(matches, sizeof(checktext) / 100); // Fail if there is more than 1% matching bytes 
	}
};
#endif
//
//
//
static BUF_MEM * TestData_WPACKET_buf;
//
//
//
int setup_tests()
{
	OPENSSL_init_crypto(OPENSSL_INIT_ADD_ALL_CIPHERS|OPENSSL_INIT_ADD_ALL_DIGESTS|OPENSSL_INIT_ASYNC, NULL);
	{
		//
		// RAND STATUS
		// 
		// This needs to be in a test executable all by itself so that it can be
		// guaranteed to run before any generate calls have been made.
		// 
		class TestInnerBlock_RandStatus {
		public:
			static int test_rand_status() { return TEST_true(RAND_status()); }
		};
		ADD_TEST(TestInnerBlock_RandStatus::test_rand_status);
	}
	/* Пока не включаем: не понятно что делать с глобальным вызовом RAND_set_DRBG_type, который выполняется несколько раз и ломает некоторые тесты
	{
		class TestInnerBlock_Rand {
		public:
			static int test_rand()
			{
				EVP_RAND_CTX * privctx;
				OSSL_PARAM params[2], * p = params;
				uchar entropy1[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
				uchar entropy2[] = { 0xff, 0xfe, 0xfd };
				uchar outbuf[3];
				*p++ = OSSL_PARAM_construct_octet_string(OSSL_RAND_PARAM_TEST_ENTROPY, entropy1, sizeof(entropy1));
				*p = OSSL_PARAM_construct_end();
				if(!TEST_ptr(privctx = RAND_get0_private(NULL))
					|| !TEST_true(EVP_RAND_CTX_set_params(privctx, params))
					|| !TEST_int_gt(RAND_priv_bytes(outbuf, sizeof(outbuf)), 0)
					|| !TEST_mem_eq(outbuf, sizeof(outbuf), entropy1, sizeof(outbuf))
					|| !TEST_int_le(RAND_priv_bytes(outbuf, sizeof(outbuf) + 1), 0)
					|| !TEST_int_gt(RAND_priv_bytes(outbuf, sizeof(outbuf)), 0)
					|| !TEST_mem_eq(outbuf, sizeof(outbuf),
					entropy1 + sizeof(outbuf), sizeof(outbuf)))
					return 0;
				*params = OSSL_PARAM_construct_octet_string(OSSL_RAND_PARAM_TEST_ENTROPY, entropy2, sizeof(entropy2));
				if(!TEST_true(EVP_RAND_CTX_set_params(privctx, params))
					|| !TEST_int_gt(RAND_priv_bytes(outbuf, sizeof(outbuf)), 0)
					|| !TEST_mem_eq(outbuf, sizeof(outbuf), entropy2, sizeof(outbuf)))
					return 0;
				return 1;
			}
		};
		if(!TEST_true(RAND_set_DRBG_type(NULL, "TEST-RAND", NULL, NULL, NULL)))
			return 0;
		ADD_TEST(TestInnerBlock_Rand::test_rand);
	}*/
	{
		ADD_TEST(TestInnerBlock_AesGcm::kat_test);
		ADD_TEST(TestInnerBlock_AesGcm::badkeylen_test);
	#ifdef FIPS_MODULE
		ADD_TEST(TestInnerBlock_AesGcm::ivgen_test);
	#endif
	}
	{
		ADD_TEST(TestInnerBlock_BadDTLS::test_bad_dtls);
	}
	{
		ADD_ALL_TESTS(TestInnerBlock_Rsa::test_rsa_pkcs1, 3);
		ADD_ALL_TESTS(TestInnerBlock_Rsa::test_rsa_oaep, 3);
		ADD_ALL_TESTS(TestInnerBlock_Rsa::test_rsa_security_bit, SIZEOFARRAY(TestData_Rsa_rsa_security_bits_cases));
	}
	{
		ADD_ALL_TESTS(TestInnerBlock_RsaMp::test_rsa_mp, 2);
	}
	{
		ADD_TEST(TestInnerBlock_Rsa_Sp800_56b::test_check_public_exponent);
		ADD_TEST(TestInnerBlock_Rsa_Sp800_56b::test_check_prime_factor_range);
		ADD_TEST(TestInnerBlock_Rsa_Sp800_56b::test_check_prime_factor);
		ADD_TEST(TestInnerBlock_Rsa_Sp800_56b::test_check_private_exponent);
		ADD_TEST(TestInnerBlock_Rsa_Sp800_56b::test_check_crt_components);
		ADD_TEST(TestInnerBlock_Rsa_Sp800_56b::test_check_private_key);
		ADD_TEST(TestInnerBlock_Rsa_Sp800_56b::test_check_public_key);
		ADD_TEST(TestInnerBlock_Rsa_Sp800_56b::test_invalid_keypair);
		ADD_TEST(TestInnerBlock_Rsa_Sp800_56b::test_pq_diff);
		ADD_ALL_TESTS(TestInnerBlock_Rsa_Sp800_56b::test_sp80056b_keygen, (int)SIZEOFARRAY(TestData_Rsa_Sp800_56b_keygen_size));
	}
	{
		#ifdef OPENSSL_NO_SM2
			TEST_note("SM2 is disabled.");
		#else
			TestData_Sm2Internal_fake_rand = fake_rand_start(NULL);
			if(TestData_Sm2Internal_fake_rand == NULL)
				return 0;
			ADD_TEST(TestInnerBlock_Sm2Internal::sm2_crypt_test);
			ADD_TEST(TestInnerBlock_Sm2Internal::sm2_sig_test);
		#endif
	}
	{
		#ifndef OPENSSL_NO_SM3
			class TestInnerBlock_Sm3Internal {
			public:
				static int test_sm3()
				{
					static const uchar input1[] = { 0x61, 0x62, 0x63 };
					// 
					// This test vector comes from Example 1 (A.1) of GM/T 0004-2012
					// 
					static const uchar expected1[SM3_DIGEST_LENGTH] = {
						0x66, 0xc7, 0xf0, 0xf4, 0x62, 0xee, 0xed, 0xd9,
						0xd1, 0xf2, 0xd4, 0x6b, 0xdc, 0x10, 0xe4, 0xe2,
						0x41, 0x67, 0xc4, 0x87, 0x5c, 0xf2, 0xf7, 0xa2,
						0x29, 0x7d, 0xa0, 0x2b, 0x8f, 0x4b, 0xa8, 0xe0
					};
					static const uchar input2[] = {
						0x61, 0x62, 0x63, 0x64, 0x61, 0x62, 0x63, 0x64,
						0x61, 0x62, 0x63, 0x64, 0x61, 0x62, 0x63, 0x64,
						0x61, 0x62, 0x63, 0x64, 0x61, 0x62, 0x63, 0x64,
						0x61, 0x62, 0x63, 0x64, 0x61, 0x62, 0x63, 0x64,
						0x61, 0x62, 0x63, 0x64, 0x61, 0x62, 0x63, 0x64,
						0x61, 0x62, 0x63, 0x64, 0x61, 0x62, 0x63, 0x64,
						0x61, 0x62, 0x63, 0x64, 0x61, 0x62, 0x63, 0x64,
						0x61, 0x62, 0x63, 0x64, 0x61, 0x62, 0x63, 0x64
					};
					// 
					// This test vector comes from Example 2 (A.2) from GM/T 0004-2012
					// 
					static const uchar expected2[SM3_DIGEST_LENGTH] = {
						0xde, 0xbe, 0x9f, 0xf9, 0x22, 0x75, 0xb8, 0xa1,
						0x38, 0x60, 0x48, 0x89, 0xc1, 0x8e, 0x5a, 0x4d,
						0x6f, 0xdb, 0x70, 0xe5, 0x38, 0x7e, 0x57, 0x65,
						0x29, 0x3d, 0xcb, 0xa3, 0x9c, 0x0c, 0x57, 0x32
					};
					SM3_CTX ctx1, ctx2;
					uchar md1[SM3_DIGEST_LENGTH], md2[SM3_DIGEST_LENGTH];
					if(!TEST_true(ossl_sm3_init(&ctx1)) || !TEST_true(ossl_sm3_update(&ctx1, input1, sizeof(input1))) || 
						!TEST_true(ossl_sm3_final(md1, &ctx1)) || !TEST_mem_eq(md1, SM3_DIGEST_LENGTH, expected1, SM3_DIGEST_LENGTH))
						return 0;
					if(!TEST_true(ossl_sm3_init(&ctx2)) || !TEST_true(ossl_sm3_update(&ctx2, input2, sizeof(input2))) || 
						!TEST_true(ossl_sm3_final(md2, &ctx2)) || !TEST_mem_eq(md2, SM3_DIGEST_LENGTH, expected2, SM3_DIGEST_LENGTH))
						return 0;
					return 1;
				}
			};
			ADD_TEST(TestInnerBlock_Sm3Internal::test_sm3);
		#endif
	}
	{
		// 
		// Internal tests for the SM4 module.
		// 
		class TestInnerBlock_Sm4Internal {
		public:
			static int test_sm4_ecb()
			{
				static const uint8_t k[SM4_BLOCK_SIZE] = {
					0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
					0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
				};
				static const uint8_t input[SM4_BLOCK_SIZE] = {
					0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
					0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
				};
				// 
				// This test vector comes from Example 1 of GB/T 32907-2016,
				// and described in Internet Draft draft-ribose-cfrg-sm4-02.
				// 
				static const uint8_t expected[SM4_BLOCK_SIZE] = {
					0x68, 0x1e, 0xdf, 0x34, 0xd2, 0x06, 0x96, 0x5e,
					0x86, 0xb3, 0xe9, 0x4f, 0x53, 0x6e, 0x42, 0x46
				};
				// 
				// This test vector comes from Example 2 from GB/T 32907-2016,
				// and described in Internet Draft draft-ribose-cfrg-sm4-02.
				// After 1,000,000 iterations.
				// 
				static const uint8_t expected_iter[SM4_BLOCK_SIZE] = {
					0x59, 0x52, 0x98, 0xc7, 0xc6, 0xfd, 0x27, 0x1f,
					0x04, 0x02, 0xf8, 0x04, 0xc3, 0x3d, 0x3f, 0x66
				};
				int i;
				SM4_KEY key;
				uint8_t block[SM4_BLOCK_SIZE];
				ossl_sm4_set_key(k, &key);
				memcpy(block, input, SM4_BLOCK_SIZE);
				ossl_sm4_encrypt(block, block, &key);
				if(!TEST_mem_eq(block, SM4_BLOCK_SIZE, expected, SM4_BLOCK_SIZE))
					return 0;
				for(i = 0; i != 999999; ++i)
					ossl_sm4_encrypt(block, block, &key);
				if(!TEST_mem_eq(block, SM4_BLOCK_SIZE, expected_iter, SM4_BLOCK_SIZE))
					return 0;
				for(i = 0; i != 1000000; ++i)
					ossl_sm4_decrypt(block, block, &key);
				if(!TEST_mem_eq(block, SM4_BLOCK_SIZE, input, SM4_BLOCK_SIZE))
					return 0;
				return 1;
			}
		};
		#ifndef OPENSSL_NO_SM4
			ADD_TEST(TestInnerBlock_Sm4Internal::test_sm4_ecb);
		#endif
	}
	{
		ADD_TEST(TestInnerBlock_SparseArray::test_sparse_array);
		ADD_TEST(TestInnerBlock_SparseArray::test_sparse_array_num);
		ADD_TEST(TestInnerBlock_SparseArray::test_sparse_array_doall);
	}
	{
		ADD_ALL_TESTS(TestInnerBlock_Poly1305Internal::test_poly1305, SIZEOFARRAY(TestData_Poly1305Internal_tests));
	}
	{ // Этот тест не отрабатывает из-за отсутствия регистрации legacy-модуля PBKDF1. Я пока не разобрался как это исправить.
		#if !defined OPENSSL_NO_RC4 && !defined OPENSSL_NO_MD5
			ADD_TEST(TestInnerBlock_Pbe::test_pkcs5_pbe_rc4_md5);
		#endif
		#if !defined OPENSSL_NO_DES && !defined OPENSSL_NO_SHA1
			ADD_TEST(TestInnerBlock_Pbe::test_pkcs5_pbe_des_sha1);
		#endif
	}
	{
		#ifndef OPENSSL_NO_CAST
			ADD_ALL_TESTS(TestInnerBlock_Cast::cast_test_vector, SIZEOFARRAY(TestData_Cast_k_len));
			ADD_TEST(TestInnerBlock_Cast::cast_test_iterations);
		#endif
	}
	{
		#ifdef CPUID_OBJ
			OPENSSL_cpuid_setup();
		#endif
		ADD_ALL_TESTS(TestInnerBlock_ChachaInternal::test_cha_cha_internal, sizeof(TestData_ChachaInternal_ref));
	}
	{
		ADD_TEST(TestInnerBlock_Ciphername::test_cipher_name);
	}
	{
		ADD_TEST(TestInnerBlock_ConstantTime::test_sizeofs);
		ADD_ALL_TESTS(TestInnerBlock_ConstantTime::test_is_zero, SIZEOFARRAY(TestData_ConstantTime_test_values));
		ADD_ALL_TESTS(TestInnerBlock_ConstantTime::test_is_zero_8, SIZEOFARRAY(TestData_ConstantTime_test_values_8));
		ADD_ALL_TESTS(TestInnerBlock_ConstantTime::test_is_zero_32, SIZEOFARRAY(TestData_ConstantTime_test_values_32));
		ADD_ALL_TESTS(TestInnerBlock_ConstantTime::test_is_zero_s, SIZEOFARRAY(TestData_ConstantTime_test_values_s));
		ADD_ALL_TESTS(TestInnerBlock_ConstantTime::test_binops, SIZEOFARRAY(TestData_ConstantTime_test_values));
		ADD_ALL_TESTS(TestInnerBlock_ConstantTime::test_binops_8, SIZEOFARRAY(TestData_ConstantTime_test_values_8));
		ADD_ALL_TESTS(TestInnerBlock_ConstantTime::test_binops_s, SIZEOFARRAY(TestData_ConstantTime_test_values_s));
		ADD_ALL_TESTS(TestInnerBlock_ConstantTime::test_signed, SIZEOFARRAY(TestData_ConstantTime_signed_test_values));
		ADD_ALL_TESTS(TestInnerBlock_ConstantTime::test_8values, SIZEOFARRAY(TestData_ConstantTime_test_values_8));
		ADD_ALL_TESTS(TestInnerBlock_ConstantTime::test_32values, SIZEOFARRAY(TestData_ConstantTime_test_values_32));
		ADD_ALL_TESTS(TestInnerBlock_ConstantTime::test_64values, SIZEOFARRAY(TestData_ConstantTime_test_values_64));
	}
	{
		if(!TEST_ptr(TestData_BnInternal_ctx = BN_CTX_new()))
			return 0;
		ADD_TEST(TestInnerBlock_BnInternal::test_is_prime_enhanced);
		ADD_ALL_TESTS(TestInnerBlock_BnInternal::test_is_composite_enhanced, (int)SIZEOFARRAY(TestData_BnInternal_composites));
		ADD_TEST(TestInnerBlock_BnInternal::test_bn_small_factors);
	}
	{
		class TestInnerBlock_CipherOverhead {
		public:
			static int cipher_enabled(const SSL_CIPHER * ciph)
			{
				// 
				// ssl_cipher_get_overhead() actually works with AEAD ciphers even if the
				// underlying implementation is not present.
				// 
				if((ciph->algorithm_mac & SSL_AEAD) != 0)
					return 1;
				if(ciph->algorithm_enc != SSL_eNULL && EVP_get_cipherbynid(SSL_CIPHER_get_cipher_nid(ciph)) == NULL)
					return 0;
				if(EVP_get_digestbynid(SSL_CIPHER_get_digest_nid(ciph)) == NULL)
					return 0;
				return 1;
			}
			static int cipher_overhead()
			{
				int ret = 1;
				const int n = ssl3_num_ciphers();
				const SSL_CIPHER * ciph;
				size_t mac, in, blk, ex;
				for(int i = 0; i < n; i++) {
					ciph = ssl3_get_cipher(i);
					if(!ciph->min_dtls)
						continue;
					if(!cipher_enabled(ciph)) {
						TEST_skip("Skipping disabled cipher %s", ciph->name);
						continue;
					}
					if(!TEST_true(ssl_cipher_get_overhead(ciph, &mac, &in, &blk, &ex))) {
						TEST_info("Failed getting %s", ciph->name);
						ret = 0;
					}
					else {
						TEST_info("Cipher %s: %zu %zu %zu %zu", ciph->name, mac, in, blk, ex);
					}
				}
				return ret;
			}
		};
		ADD_TEST(TestInnerBlock_CipherOverhead::cipher_overhead);
	}
	{
		if(!TEST_ptr(TestData_CipherBytes_ctx = SSL_CTX_new(TLS_server_method())) || !TEST_ptr(TestData_CipherBytes_s = SSL_new(TestData_CipherBytes_ctx)))
			return 0;
		ADD_TEST(TestInnerBlock_CipherBytes::test_empty);
		ADD_TEST(TestInnerBlock_CipherBytes::test_unsupported);
		ADD_TEST(TestInnerBlock_CipherBytes::test_v2);
		ADD_TEST(TestInnerBlock_CipherBytes::test_v3);
	}
	{
		#ifndef OPENSSL_NO_DES
			ADD_ALL_TESTS(TestInnerBlock_DES::test_des_ecb, TestData_DES_NUM_TESTS);
			ADD_TEST(TestInnerBlock_DES::test_des_cbc);
			ADD_TEST(TestInnerBlock_DES::test_ede_cbc);
			ADD_ALL_TESTS(TestInnerBlock_DES::test_des_ede_ecb, TestData_DES_NUM_TESTS - 2);
			ADD_TEST(TestInnerBlock_DES::test_des_ede_cbc);
			ADD_TEST(TestInnerBlock_DES::test_des_pcbc);
			ADD_TEST(TestInnerBlock_DES::test_des_cfb8);
			ADD_TEST(TestInnerBlock_DES::test_des_cfb16);
			ADD_TEST(TestInnerBlock_DES::test_des_cfb32);
			ADD_TEST(TestInnerBlock_DES::test_des_cfb48);
			ADD_TEST(TestInnerBlock_DES::test_des_cfb64);
			ADD_TEST(TestInnerBlock_DES::test_des_ede_cfb64);
			ADD_TEST(TestInnerBlock_DES::test_des_ofb);
			ADD_TEST(TestInnerBlock_DES::test_des_ofb64);
			ADD_TEST(TestInnerBlock_DES::test_des_ede_ofb64);
			ADD_TEST(TestInnerBlock_DES::test_des_cbc_cksum);
			ADD_TEST(TestInnerBlock_DES::test_des_quad_cksum);
			ADD_TEST(TestInnerBlock_DES::test_des_crypt);
			ADD_ALL_TESTS(TestInnerBlock_DES::test_input_align, 4);
			ADD_ALL_TESTS(TestInnerBlock_DES::test_output_align, 4);
			ADD_ALL_TESTS(TestInnerBlock_DES::test_des_key_wrap, SIZEOFARRAY(TestData_DES_test_des_key_wrap_sizes));
			ADD_ALL_TESTS(TestInnerBlock_DES::test_des_weak_keys, SIZEOFARRAY(TestData_DES_weak_keys));
			ADD_ALL_TESTS(TestInnerBlock_DES::test_des_check_bad_parity, SIZEOFARRAY(TestData_DES_bad_parity_keys));
		#endif
	}
	{
		ADD_TEST(TestInnerBlock_Property::test_property_string);
		ADD_TEST(TestInnerBlock_Property::test_property_query_value_create);
		ADD_ALL_TESTS(TestInnerBlock_Property::test_property_parse, SIZEOFARRAY(TestData_Property_parser_tests));
		ADD_ALL_TESTS(TestInnerBlock_Property::test_property_parse_error, SIZEOFARRAY(TestData_Property_parse_error_tests));
		ADD_ALL_TESTS(TestInnerBlock_Property::test_property_merge, SIZEOFARRAY(TestData_Property_merge_tests));
		ADD_TEST(TestInnerBlock_Property::test_property_defn_cache);
		ADD_ALL_TESTS(TestInnerBlock_Property::test_definition_compares, SIZEOFARRAY(TestData_Property_definition_tests));
		ADD_TEST(TestInnerBlock_Property::test_register_deregister);
		ADD_TEST(TestInnerBlock_Property::test_property);
		ADD_TEST(TestInnerBlock_Property::test_query_cache_stochastic);
		ADD_TEST(TestInnerBlock_Property::test_fips_mode);
		ADD_ALL_TESTS(TestInnerBlock_Property::test_property_list_to_string, SIZEOFARRAY(TestData_Property_to_string_tests));
	}
	{
		for(uint i = 1; i <= TestData_Packet_BUF_LEN; i++)
			TestData_Packet_smbuf[i - 1] = (i * 2) & 0xff;
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_buf_init);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_null_init);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_remaining);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_end);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_equal);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_get_1);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_get_4);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_get_net_2);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_get_net_3);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_get_net_4);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_get_sub_packet);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_get_bytes);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_copy_bytes);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_copy_all);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_memdup);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_strndup);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_contains_zero_byte);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_forward);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_get_length_prefixed_1);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_get_length_prefixed_2);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_get_length_prefixed_3);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_as_length_prefixed_1);
		ADD_TEST(TestInnerBlock_Packet::test_PACKET_as_length_prefixed_2);
	}
	{
		ADD_TEST(TestInnerBlock_NameMapInternal::test_namemap_empty);
		ADD_TEST(TestInnerBlock_NameMapInternal::test_namemap_independent);
		ADD_TEST(TestInnerBlock_NameMapInternal::test_namemap_stored);
		ADD_TEST(TestInnerBlock_NameMapInternal::test_digestbyname);
		ADD_TEST(TestInnerBlock_NameMapInternal::test_cipherbyname);
		ADD_TEST(TestInnerBlock_NameMapInternal::test_digest_is_a);
		ADD_TEST(TestInnerBlock_NameMapInternal::test_cipher_is_a);
	}
	{
		class TestInnerBlock_Asn1Internal {
		public:
			// 
			// Internal tests for the asn1 module */
			// 
			// RSA low level APIs are deprecated for public use, but still ok for internal use.
			// 
			// 
			// Test of a_strnid's tbl_standard
			// 
			static int test_tbl_standard()
			{
				const ASN1_STRING_TABLE * tmp;
				int last_nid = -1;
				size_t i;
				for(tmp = tbl_standard, i = 0; i < SIZEOFARRAY(tbl_standard); i++, tmp++) {
					if(tmp->nid < last_nid) {
						last_nid = 0;
						break;
					}
					last_nid = tmp->nid;
				}
				if(TEST_int_ne(last_nid, 0)) {
					TEST_info("asn1 tbl_standard: Table order OK");
					return 1;
				}
				TEST_info("asn1 tbl_standard: out of order");
				for(tmp = tbl_standard, i = 0; i < SIZEOFARRAY(tbl_standard); i++, tmp++)
					TEST_note("asn1 tbl_standard: Index %zu, NID %d, Name=%s", i, tmp->nid, OBJ_nid2ln(tmp->nid));
				return 0;
			}
			// 
			// Test of ameth_lib's standard_methods
			// 
			static int test_standard_methods()
			{
				const EVP_PKEY_ASN1_METHOD ** tmp;
				int last_pkey_id = -1;
				size_t i;
				int ok = 1;
				for(tmp = standard_methods, i = 0; i < SIZEOFARRAY(standard_methods); i++, tmp++) {
					if((*tmp)->pkey_id < last_pkey_id) {
						last_pkey_id = 0;
						break;
					}
					last_pkey_id = (*tmp)->pkey_id;
					/*
					 * One of the following must be true:
					 *
					 * pem_str == NULL AND ASN1_PKEY_ALIAS is set
					 * pem_str != NULL AND ASN1_PKEY_ALIAS is clear
					 *
					 * Anything else is an error and may lead to a corrupt ASN1 method table
					 */
					if(!TEST_true(((*tmp)->pem_str == NULL && ((*tmp)->pkey_flags & ASN1_PKEY_ALIAS) != 0)
						|| ((*tmp)->pem_str && ((*tmp)->pkey_flags & ASN1_PKEY_ALIAS) == 0))) {
						TEST_note("asn1 standard methods: Index %zu, pkey ID %d, Name=%s", i, (*tmp)->pkey_id, OBJ_nid2sn((*tmp)->pkey_id));
						ok = 0;
					}
				}
				if(TEST_int_ne(last_pkey_id, 0)) {
					TEST_info("asn1 standard methods: Table order OK");
					return ok;
				}
				TEST_note("asn1 standard methods: out of order");
				for(tmp = standard_methods, i = 0; i < SIZEOFARRAY(standard_methods); i++, tmp++)
					TEST_note("asn1 standard methods: Index %zu, pkey ID %d, Name=%s", i, (*tmp)->pkey_id, OBJ_nid2sn((*tmp)->pkey_id));
				return 0;
			}
			// 
			// Test of that i2d fail on non-existing non-optional items
			// 
			static int test_empty_nonoptional_content()
			{
				RSA * rsa = NULL;
				BIGNUM * n = NULL;
				BIGNUM * e = NULL;
				int ok = 0;
				if(!TEST_ptr(rsa = RSA_new()) || !TEST_ptr(n = BN_new()) || !TEST_ptr(e = BN_new()) || !TEST_true(RSA_set0_key(rsa, n, e, NULL)))
					goto end;
				n = e = NULL; /* They are now "owned" by |rsa| */
				/*
				 * This SHOULD fail, as we're trying to encode a public key as a private
				 * key.  The private key bits MUST be present for a proper RSAPrivateKey.
				 */
				if(TEST_int_le(i2d_RSAPrivateKey(rsa, NULL), 0))
					ok = 1;
			end:
				RSA_free(rsa);
				BN_free(n);
				BN_free(e);
				return ok;
			}
			// 
			// Tests of the Unicode code point range
			// 
			static int test_unicode(const uchar * univ, size_t len, int expected)
			{
				const uchar * end = univ + len;
				int ok = 1;
				for(; univ < end; univ += 4) {
					if(!TEST_int_eq(ASN1_mbstring_copy(NULL, univ, 4, MBSTRING_UNIV, B_ASN1_UTF8STRING), expected))
						ok = 0;
				}
				return ok;
			}
			static int test_unicode_range()
			{
				const uchar univ_ok[] = "\0\0\0\0"
					"\0\0\xd7\xff"
					"\0\0\xe0\x00"
					"\0\x10\xff\xff";
				const uchar univ_bad[] = "\0\0\xd8\x00"
					"\0\0\xdf\xff"
					"\0\x11\x00\x00"
					"\x80\x00\x00\x00"
					"\xff\xff\xff\xff";
				int ok = 1;
				if(!test_unicode(univ_ok, sizeof univ_ok - 1, V_ASN1_UTF8STRING))
					ok = 0;
				if(!test_unicode(univ_bad, sizeof univ_bad - 1, -1))
					ok = 0;
				return ok;
			}
		};

		ADD_TEST(TestInnerBlock_Asn1Internal::test_tbl_standard);
		ADD_TEST(TestInnerBlock_Asn1Internal::test_standard_methods);
		ADD_TEST(TestInnerBlock_Asn1Internal::test_empty_nonoptional_content);
		ADD_TEST(TestInnerBlock_Asn1Internal::test_unicode_range);
	}
	{
		//
		// Tests for the ASN1_STRING_TABLE_* functions
		//
		class TestInnerBlock_Asn1StringTable {
		public:
			static int test_string_tbl()
			{
				int nid = 12345678;
				int nid2 = 87654321;
				int rv = 0;
				int ret = 0;
				const ASN1_STRING_TABLE * tmp = ASN1_STRING_TABLE_get(nid);
				if(!TEST_ptr_null(tmp)) {
					TEST_info("asn1 string table: ASN1_STRING_TABLE_get non-exist nid");
					goto out;
				}
				ret = ASN1_STRING_TABLE_add(nid, -1, -1, MBSTRING_ASC, 0);
				if(!TEST_true(ret)) {
					TEST_info("asn1 string table: add NID(%d) failed", nid);
					goto out;
				}
				ret = ASN1_STRING_TABLE_add(nid2, -1, -1, MBSTRING_ASC, 0);
				if(!TEST_true(ret)) {
					TEST_info("asn1 string table: add NID(%d) failed", nid2);
					goto out;
				}
				tmp = ASN1_STRING_TABLE_get(nid);
				if(!TEST_ptr(tmp)) {
					TEST_info("asn1 string table: get NID(%d) failed", nid);
					goto out;
				}
				tmp = ASN1_STRING_TABLE_get(nid2);
				if(!TEST_ptr(tmp)) {
					TEST_info("asn1 string table: get NID(%d) failed", nid2);
					goto out;
				}
				ASN1_STRING_TABLE_cleanup();
				/* check if all newly added NIDs are cleaned up */
				tmp = ASN1_STRING_TABLE_get(nid);
				if(!TEST_ptr_null(tmp)) {
					TEST_info("asn1 string table: get NID(%d) failed", nid);
					goto out;
				}
				tmp = ASN1_STRING_TABLE_get(nid2);
				if(!TEST_ptr_null(tmp)) {
					TEST_info("asn1 string table: get NID(%d) failed", nid2);
					goto out;
				}
				rv = 1;
			out:
				return rv;
			}
		};
		
		ADD_TEST(TestInnerBlock_Asn1StringTable::test_string_tbl);
	}
	{
		// 
		// On platforms where |time_t| is an unsigned integer, t will be a positive number.
		// 
		// We check if we're on a platform with a signed |time_t| with '!(t > 0)'
		// because some compilers are picky if you do 't < 0', or even 't <= 0' if |t| is unsigned.
		// 
		time_t t = -1;
		// 
		// On some platforms, |time_t| is signed, but a negative value is an
		// error, and using it with gmtime() or localtime() generates a NULL.
		// If that is the case, we can't perform tests on negative values.
		// 
		struct tm * ptm = localtime(&t);
		ADD_ALL_TESTS(TestInnerBlock_Asn1Time::test_table_pos, SIZEOFARRAY(TestData_Asn1Time_tbl_testdata_pos));
		if(!(t > 0) && ptm) {
			TEST_info("Adding negative-sign time_t tests");
			ADD_ALL_TESTS(TestInnerBlock_Asn1Time::test_table_neg, SIZEOFARRAY(TestData_Asn1Time_tbl_testdata_neg));
		}
		if(sizeof(time_t) > sizeof(uint32_t)) {
			TEST_info("Adding 64-bit time_t tests");
			ADD_ALL_TESTS(TestInnerBlock_Asn1Time::test_table_pos_64bit, SIZEOFARRAY(TestData_Asn1Time_tbl_testdata_pos_64bit));
	#ifndef __hpux
			if(!(t > 0) && ptm) {
				TEST_info("Adding negative-sign 64-bit time_t tests");
				ADD_ALL_TESTS(TestInnerBlock_Asn1Time::test_table_neg_64bit, SIZEOFARRAY(TestData_Asn1Time_tbl_testdata_neg_64bit));
			}
	#endif
		}
		ADD_ALL_TESTS(TestInnerBlock_Asn1Time::test_table_compare, SIZEOFARRAY(TestData_Asn1Time_tbl_compare_testdata));
		ADD_TEST(TestInnerBlock_Asn1Time::test_time_dup);
	}
	{
		// @todo Вероятно, надо как-то учесть скрипт bntests.pl 
		TestData_BN_OPTION_CHOICE o;
		int n, stochastic = 0;
		while((o = (TestData_BN_OPTION_CHOICE)opt_next()) != OPT_EOF) {
			switch(o) {
				case OPT_STOCHASTIC_TESTS:
					stochastic = 1;
					break;
				case OPT_TEST_CASES:
					break;
				default:
				case OPT_ERR:
					return 0;
			}
		}
		n  = test_get_argument_count();
		if(!TEST_ptr(TestData_BN_ctx = BN_CTX_new()))
			return 0;
		if(n == 0) {
			ADD_TEST(TestInnerBlock_BN::test_sub);
			ADD_TEST(TestInnerBlock_BN::test_div_recip);
			ADD_ALL_TESTS(TestInnerBlock_BN::test_signed_mod_replace_ab, SIZEOFARRAY(TestData_BN_signed_mod_tests));
			ADD_ALL_TESTS(TestInnerBlock_BN::test_signed_mod_replace_ba, SIZEOFARRAY(TestData_BN_signed_mod_tests));
			ADD_TEST(TestInnerBlock_BN::test_mod);
			ADD_TEST(TestInnerBlock_BN::test_modexp_mont5);
			ADD_TEST(TestInnerBlock_BN::test_kronecker);
			ADD_TEST(TestInnerBlock_BN::test_rand);
			ADD_TEST(TestInnerBlock_BN::test_bn2padded);
			ADD_TEST(TestInnerBlock_BN::test_dec2bn);
			ADD_TEST(TestInnerBlock_BN::test_hex2bn);
			ADD_TEST(TestInnerBlock_BN::test_asc2bn);
			ADD_ALL_TESTS(TestInnerBlock_BN::test_mpi, (int)SIZEOFARRAY(TestData_BN_kMPITests));
			ADD_TEST(TestInnerBlock_BN::test_negzero);
			ADD_TEST(TestInnerBlock_BN::test_badmod);
			ADD_TEST(TestInnerBlock_BN::test_expmodzero);
			ADD_TEST(TestInnerBlock_BN::test_expmodone);
			ADD_ALL_TESTS(TestInnerBlock_BN::test_smallprime, 16);
			ADD_ALL_TESTS(TestInnerBlock_BN::test_smallsafeprime, 16);
			ADD_TEST(TestInnerBlock_BN::test_swap);
			ADD_TEST(TestInnerBlock_BN::test_ctx_consttime_flag);
	#ifndef OPENSSL_NO_EC2M
			ADD_TEST(TestInnerBlock_BN::test_gf2m_add);
			ADD_TEST(TestInnerBlock_BN::test_gf2m_mod);
			ADD_TEST(TestInnerBlock_BN::test_gf2m_mul);
			ADD_TEST(TestInnerBlock_BN::test_gf2m_sqr);
			ADD_TEST(TestInnerBlock_BN::test_gf2m_modinv);
			ADD_TEST(TestInnerBlock_BN::test_gf2m_moddiv);
			ADD_TEST(TestInnerBlock_BN::test_gf2m_modexp);
			ADD_TEST(TestInnerBlock_BN::test_gf2m_modsqrt);
			ADD_TEST(TestInnerBlock_BN::test_gf2m_modsolvequad);
	#endif
			ADD_ALL_TESTS(TestInnerBlock_BN::test_is_prime, (int)SIZEOFARRAY(TestData_BN_primes));
			ADD_ALL_TESTS(TestInnerBlock_BN::test_not_prime, (int)SIZEOFARRAY(TestData_BN_not_primes));
			ADD_TEST(TestInnerBlock_BN::test_gcd_prime);
			ADD_ALL_TESTS(TestInnerBlock_BN::test_mod_exp, (int)SIZEOFARRAY(TestData_BN_ModExpTests));
			ADD_ALL_TESTS(TestInnerBlock_BN::test_mod_exp_consttime, (int)SIZEOFARRAY(TestData_BN_ModExpTests));
			ADD_TEST(TestInnerBlock_BN::test_mod_exp2_mont);
			if(stochastic)
				ADD_TEST(TestInnerBlock_BN::test_rand_range);
		}
		else {
			ADD_ALL_TESTS(TestInnerBlock_BN::run_file_tests, n);
		}
	}
	{
		class TestInnerBlock_SecMem {
		public:
			static int test_sec_mem()
			{
			#ifndef OPENSSL_NO_SECURE_MEMORY
				int testresult = 0;
				char * p = NULL, * q = NULL, * r = NULL, * s = NULL;
				TEST_info("Secure memory is implemented.");
				s = (char*)OPENSSL_secure_malloc(20);
				/* s = non-secure 20 */
				if(!TEST_ptr(s) || !TEST_false(CRYPTO_secure_allocated(s)))
					goto end;
				r = (char *)OPENSSL_secure_malloc(20);
				/* r = non-secure 20, s = non-secure 20 */
				if(!TEST_ptr(r) || !TEST_true(CRYPTO_secure_malloc_init(4096, 32)) || !TEST_false(CRYPTO_secure_allocated(r)))
					goto end;
				p = (char *)OPENSSL_secure_malloc(20);
				if(!TEST_ptr(p)
					/* r = non-secure 20, p = secure 20, s = non-secure 20 */
					|| !TEST_true(CRYPTO_secure_allocated(p))
					/* 20 secure -> 32-byte minimum allocation unit */
					|| !TEST_size_t_eq(CRYPTO_secure_used(), 32))
					goto end;
				q = (char *)OPENSSL_malloc(20);
				if(!TEST_ptr(q))
					goto end;
				/* r = non-secure 20, p = secure 20, q = non-secure 20, s = non-secure 20 */
				if(!TEST_false(CRYPTO_secure_allocated(q)))
					goto end;
				OPENSSL_secure_clear_free(s, 20);
				s = (char *)OPENSSL_secure_malloc(20);
				if(!TEST_ptr(s)
					/* r = non-secure 20, p = secure 20, q = non-secure 20, s = secure 20 */
					|| !TEST_true(CRYPTO_secure_allocated(s))
					/* 2 * 20 secure -> 64 bytes allocated */
					|| !TEST_size_t_eq(CRYPTO_secure_used(), 64))
					goto end;
				OPENSSL_secure_clear_free(p, 20);
				p = NULL;
				/* 20 secure -> 32 bytes allocated */
				if(!TEST_size_t_eq(CRYPTO_secure_used(), 32))
					goto end;
				OPENSSL_free(q);
				q = NULL;
				/* should not complete, as secure memory is still allocated */
				if(!TEST_false(CRYPTO_secure_malloc_done())
					|| !TEST_true(CRYPTO_secure_malloc_initialized()))
					goto end;
				OPENSSL_secure_free(s);
				s = NULL;
				/* secure memory should now be 0, so done should complete */
				if(!TEST_size_t_eq(CRYPTO_secure_used(), 0)
					|| !TEST_true(CRYPTO_secure_malloc_done())
					|| !TEST_false(CRYPTO_secure_malloc_initialized()))
					goto end;
				TEST_info("Possible infinite loop: allocate more than available");
				if(!TEST_true(CRYPTO_secure_malloc_init(32768, 16)))
					goto end;
				TEST_ptr_null(OPENSSL_secure_malloc((size_t)-1));
				TEST_true(CRYPTO_secure_malloc_done());
				/*
				 * If init fails, then initialized should be false, if not, this
				 * could cause an infinite loop secure_malloc, but we don't test it
				 */
				if(TEST_false(CRYPTO_secure_malloc_init(16, 16)) && !TEST_false(CRYPTO_secure_malloc_initialized())) {
					TEST_true(CRYPTO_secure_malloc_done());
					goto end;
				}
				/*-
				 * There was also a possible infinite loop when the number of
				 * elements was 1<<31, as |int i| was set to that, which is a
				 * negative number. However, it requires minimum input values:
				 *
				 * CRYPTO_secure_malloc_init((size_t)1<<34, 1<<4);
				 *
				 * Which really only works on 64-bit systems, since it took 16 GB
				 * secure memory arena to trigger the problem. It naturally takes
				 * corresponding amount of available virtual and physical memory
				 * for test to be feasible/representative. Since we can't assume
				 * that every system is equipped with that much memory, the test
				 * remains disabled. If the reader of this comment really wants
				 * to make sure that infinite loop is fixed, they can enable the
				 * code below.
				 */
			#if 0
				/*-
				 * On Linux and BSD this test has a chance to complete in minimal
				 * time and with minimum side effects, because mlock is likely to
				 * fail because of RLIMIT_MEMLOCK, which is customarily [much]
				 * smaller than 16GB. In other words Linux and BSD users can be
				 * limited by virtual space alone...
				 */
				if(sizeof(size_t) > 4) {
					TEST_info("Possible infinite loop: 1<<31 limit");
					if(TEST_true(CRYPTO_secure_malloc_init((size_t)1<<34, 1<<4) != 0))
						TEST_true(CRYPTO_secure_malloc_done());
				}
			#endif
				// this can complete - it was not really secure 
				testresult = 1;
			end:
				OPENSSL_secure_free(p);
				OPENSSL_free(q);
				OPENSSL_secure_free(r);
				OPENSSL_secure_free(s);
				return testresult;
			#else
				TEST_info("Secure memory is *not* implemented.");
				// Should fail
				return TEST_false(CRYPTO_secure_malloc_init(4096, 32));
			#endif
			}

			static int test_sec_mem_clear()
			{
			#ifndef OPENSSL_NO_SECURE_MEMORY
				const int size = 64;
				uchar * p = NULL;
				int i, res = 0;
				if(!TEST_true(CRYPTO_secure_malloc_init(4096, 32)) || !TEST_ptr(p = (uchar *)OPENSSL_secure_malloc(size)))
					goto err;
				for(i = 0; i < size; i++)
					if(!TEST_uchar_eq(p[i], 0))
						goto err;
				for(i = 0; i < size; i++)
					p[i] = (uchar)(i + ' ' + 1);
				OPENSSL_secure_free(p);
				/*
				 * A deliberate use after free here to verify that the memory has been
				 * cleared properly.  Since secure free doesn't return the memory to
				 * libc's memory pool, it technically isn't freed.  However, the header
				 * bytes have to be skipped and these consist of two pointers in the
				 * current implementation.
				 */
				for(i = sizeof(void *) * 2; i < size; i++)
					if(!TEST_uchar_eq(p[i], 0))
						return 0;
				res = 1;
				p = NULL;
			err:
				OPENSSL_secure_free(p);
				CRYPTO_secure_malloc_done();
				return res;
			#else
				return 1;
			#endif
			}
		};
		ADD_TEST(TestInnerBlock_SecMem::test_sec_mem);
		ADD_TEST(TestInnerBlock_SecMem::test_sec_mem_clear);
	}
	{
		ADD_TEST(TestInnerBlock_Asn1DsaInternal::test_decode);
	}
	{
		#ifndef OPENSSL_NO_DEPRECATED_3_0
			ADD_TEST(TestInnerBlock_Asn1Encode::test_long_32bit);
			ADD_TEST(TestInnerBlock_Asn1Encode::test_long_64bit);
		#endif
			ADD_TEST(TestInnerBlock_Asn1Encode::test_int32);
			ADD_TEST(TestInnerBlock_Asn1Encode::test_uint32);
			ADD_TEST(TestInnerBlock_Asn1Encode::test_int64);
			ADD_TEST(TestInnerBlock_Asn1Encode::test_uint64);
			ADD_TEST(TestInnerBlock_Asn1Encode::test_invalid_template);
	}
	{
		#ifndef OPENSSL_NO_DEPRECATED_3_0
			ADD_TEST(TestInnerBlock_Asn1Decode::test_long);
		#endif
			ADD_TEST(TestInnerBlock_Asn1Decode::test_int32);
			ADD_TEST(TestInnerBlock_Asn1Decode::test_uint32);
			ADD_TEST(TestInnerBlock_Asn1Decode::test_int64);
			ADD_TEST(TestInnerBlock_Asn1Decode::test_uint64);
			ADD_TEST(TestInnerBlock_Asn1Decode::test_invalid_template);
			ADD_TEST(TestInnerBlock_Asn1Decode::test_reuse_asn1_object);
	}
	{
			ADD_TEST(TestInnerBlock_BioCallback::test_bio_callback_ex);
		#ifndef OPENSSL_NO_DEPRECATED_3_0
			ADD_TEST(TestInnerBlock_BioCallback::test_bio_callback);
		#endif
	}
	{
		if(!test_skip_common_options()) {
			TEST_error("Error parsing test options\n");
			return 0;
		}
		ADD_TEST(TestInnerBlock_BioCore::test_bio_core);
	}
	{
			ADD_ALL_TESTS(TestInnerBlock_BioEnc::test_bio_enc_aes_128_cbc, 2);
			ADD_ALL_TESTS(TestInnerBlock_BioEnc::test_bio_enc_aes_128_ctr, 2);
			ADD_ALL_TESTS(TestInnerBlock_BioEnc::test_bio_enc_aes_256_cfb, 2);
			ADD_ALL_TESTS(TestInnerBlock_BioEnc::test_bio_enc_aes_256_ofb, 2);
		#ifndef OPENSSL_NO_CHACHA
			ADD_ALL_TESTS(TestInnerBlock_BioEnc::test_bio_enc_chacha20, 2);
		#ifndef OPENSSL_NO_POLY1305
			ADD_ALL_TESTS(TestInnerBlock_BioEnc::test_bio_enc_chacha20_poly1305, 2);
		#endif
		#endif
	}
	{
		ADD_ALL_TESTS(TestInnerBlock_Mdc2Internal::test_mdc2, SIZEOFARRAY(TestData_Mdc2Internal_tests));
	}
	{
		#ifndef OPENSSL_NO_MDC2
			ADD_TEST(TestInnerBlock_Mdc2::test_mdc2);
		#endif
	}
	{
		ADD_ALL_TESTS(TestInnerBlock_UserProperty::test_default_props_and_providers, TestInnerBlock_UserProperty::DEFAULT_PROPS_FINAL);
	}
	{
		ADD_TEST(TestInnerBlock_CipherList::test_default_cipherlist_implicit);
		ADD_TEST(TestInnerBlock_CipherList::test_default_cipherlist_explicit);
		ADD_TEST(TestInnerBlock_CipherList::test_default_cipherlist_clear);
	}
	{
		ADD_TEST(TestInnerBlock_CMAC::test_cmac_bad);
		ADD_TEST(TestInnerBlock_CMAC::test_cmac_run);
		ADD_TEST(TestInnerBlock_CMAC::test_cmac_copy);
	}
	{
		ADD_ALL_TESTS(TestInnerBlock_HMAC::test_hmac_md5, 4);
		ADD_TEST(TestInnerBlock_HMAC::test_hmac_single_shot);
		ADD_TEST(TestInnerBlock_HMAC::test_hmac_bad);
		ADD_TEST(TestInnerBlock_HMAC::test_hmac_run);
		ADD_TEST(TestInnerBlock_HMAC::test_hmac_copy);
		ADD_TEST(TestInnerBlock_HMAC::test_hmac_copy_uninited);
	}
	{
		#ifndef OPENSSL_NO_IDEA
			ADD_TEST(TestInnerBlock_Idea::test_idea_ecb);
			ADD_TEST(TestInnerBlock_Idea::test_idea_cbc);
			ADD_TEST(TestInnerBlock_Idea::test_idea_cfb64);
		#endif
	}
	{
		#ifndef OPENSSL_NO_DEPRECATED_3_0
			RAND_bytes(TestData_Ige_rkey, sizeof(TestData_Ige_rkey));
			RAND_bytes(TestData_Ige_rkey2, sizeof(TestData_Ige_rkey2));
			RAND_bytes(TestData_Ige_plaintext, sizeof(TestData_Ige_plaintext));
			RAND_bytes(TestData_Ige_saved_iv, sizeof(TestData_Ige_saved_iv));
			ADD_TEST(TestInnerBlock_Ige::test_ige_enc_dec);
			ADD_TEST(TestInnerBlock_Ige::test_ige_enc_chaining);
			ADD_TEST(TestInnerBlock_Ige::test_ige_dec_chaining);
			ADD_TEST(TestInnerBlock_Ige::test_ige_garble_forwards);
			ADD_TEST(TestInnerBlock_Ige::test_bi_ige_enc_dec);
			ADD_TEST(TestInnerBlock_Ige::test_bi_ige_garble1);
			ADD_TEST(TestInnerBlock_Ige::test_bi_ige_garble2);
			ADD_TEST(TestInnerBlock_Ige::test_bi_ige_garble3);
			ADD_ALL_TESTS(TestInnerBlock_Ige::test_ige_vectors, SIZEOFARRAY(TestData_Ige_ige_test_vectors));
			ADD_ALL_TESTS(TestInnerBlock_Ige::test_bi_ige_vectors, SIZEOFARRAY(TestData_Ige_bi_ige_test_vectors));
		#endif
	}
	{
		// 
		// RC2 low level APIs are deprecated for public use, but still ok for internal use.
		// 
		#ifndef OPENSSL_NO_RC2
			static const uchar RC2key[4][16] = {
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
			};

			class TestInnerBlock_RC2 {
			public:
				static int test_rc2(const int n)
				{
					static const uchar RC2plain[4][8] = {
						{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
						{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
						{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
						{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
					};
					static const uchar RC2cipher[4][8] = {
						{0x1C, 0x19, 0x8A, 0x83, 0x8D, 0xF0, 0x28, 0xB7},
						{0x21, 0x82, 0x9C, 0x78, 0xA9, 0xF9, 0xC0, 0x74},
						{0x13, 0xDB, 0x35, 0x17, 0xD3, 0x21, 0x86, 0x9E},
						{0x50, 0xDC, 0x01, 0x62, 0xBD, 0x75, 0x7F, 0x31},
					};
					int testresult = 1;
					RC2_KEY key;
					uchar buf[8], buf2[8];
					RC2_set_key(&key, 16, &(RC2key[n][0]), 0 /* or 1024 */);
					RC2_ecb_encrypt(&RC2plain[n][0], buf, &key, RC2_ENCRYPT);
					if(!TEST_mem_eq(&RC2cipher[n][0], 8, buf, 8))
						testresult = 0;
					RC2_ecb_encrypt(buf, buf2, &key, RC2_DECRYPT);
					if(!TEST_mem_eq(&RC2plain[n][0], 8, buf2, 8))
						testresult = 0;
					return testresult;
				}
			};
			ADD_ALL_TESTS(TestInnerBlock_RC2::test_rc2, SIZEOFARRAY(RC2key));
		#endif
	}
	{
		#ifndef OPENSSL_NO_RC4
		// 
		// RC4 and SHA-1 low level APIs are deprecated for public use, but still ok for internal use.
		// 
		static const uchar data_len[6] = { 8, 8, 8, 20, 28, 10 };
		static const uchar keys[6][30] = {
			{8, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef},
			{8, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef},
			{8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
			{4, 0xef, 0x01, 0x23, 0x45},
			{8, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef},
			{4, 0xef, 0x01, 0x23, 0x45},
		};
		static const uchar data[6][30] = {
			{0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xff},
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0xff},
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
		};
		static const uchar output[6][30] = {
			{0x75, 0xb7, 0x87, 0x80, 0x99, 0xe0, 0xc5, 0x96, 0x00},
			{0x74, 0x94, 0xc2, 0xe7, 0x10, 0x4b, 0x08, 0x79, 0x00},
			{0xde, 0x18, 0x89, 0x41, 0xa3, 0x37, 0x5d, 0x3a, 0x00},
			{0xd6, 0xa1, 0x41, 0xa7, 0xec, 0x3c, 0x38, 0xdf, 0xbd, 0x61, 0x5a, 0x11, 0x62, 0xe1, 0xc7, 0xba, 0x36, 0xb6, 0x78, 0x58, 0x00},
			{0x66, 0xa0, 0x94, 0x9f, 0x8a, 0xf7, 0xd6, 0x89, 0x1f, 0x7f, 0x83, 0x2b, 0xa8, 0x33, 0xc0, 0x0c, 0x89, 0x2e, 0xbe, 0x30, 0x14, 0x3c, 0xe2, 0x87, 0x40, 0x01, 0x1e, 0xcf, 0x00},
			{0xd6, 0xa1, 0x41, 0xa7, 0xec, 0x3c, 0x38, 0xdf, 0xbd, 0x61, 0x00},
		};
		class TestInnerBlock_RC4 {
		public:
			static int test_rc4_encrypt(const int i)
			{
				uchar obuf[512];
				RC4_KEY key;
				RC4_set_key(&key, keys[i][0], &(keys[i][1]));
				memzero(obuf, sizeof(obuf));
				RC4(&key, data_len[i], &(data[i][0]), obuf);
				return TEST_mem_eq(obuf, data_len[i] + 1, output[i], data_len[i] + 1);
			}
			static int test_rc4_end_processing(const int i)
			{
				uchar obuf[512];
				RC4_KEY key;
				RC4_set_key(&key, keys[3][0], &(keys[3][1]));
				memzero(obuf, sizeof(obuf));
				RC4(&key, i, &(data[3][0]), obuf);
				if(!TEST_mem_eq(obuf, i, output[3], i))
					return 0;
				return TEST_uchar_eq(obuf[i], 0);
			}
			static int test_rc4_multi_call(const int i)
			{
				uchar obuf[512];
				RC4_KEY key;
				RC4_set_key(&key, keys[3][0], &(keys[3][1]));
				memzero(obuf, sizeof(obuf));
				RC4(&key, i, &(data[3][0]), obuf);
				RC4(&key, data_len[3] - i, &(data[3][i]), &(obuf[i]));
				return TEST_mem_eq(obuf, data_len[3] + 1, output[3], data_len[3] + 1);
			}
			static int test_rc_bulk()
			{
				RC4_KEY key;
				uchar buf[513];
				SHA_CTX c;
				uchar md[SHA_DIGEST_LENGTH];
				int i;
				static uchar expected[] = { 0xa4, 0x7b, 0xcc, 0x00, 0x3d, 0xd0, 0xbd, 0xe1, 0xac, 0x5f, 0x12, 0x1e, 0x45, 0xbc, 0xfb, 0x1a, 0xa1, 0xf2, 0x7f, 0xc5 };
				RC4_set_key(&key, keys[0][0], &(keys[3][1]));
				memzero(buf, sizeof(buf));
				SHA1_Init(&c);
				for(i = 0; i < 2571; i++) {
					RC4(&key, sizeof(buf), buf, buf);
					SHA1_Update(&c, buf, sizeof(buf));
				}
				SHA1_Final(md, &c);
				return TEST_mem_eq(md, sizeof(md), expected, sizeof(expected));
			}
		};
		#endif
		#ifndef OPENSSL_NO_RC4
			ADD_ALL_TESTS(TestInnerBlock_RC4::test_rc4_encrypt, SIZEOFARRAY(data_len));
			ADD_ALL_TESTS(TestInnerBlock_RC4::test_rc4_end_processing, data_len[3]);
			ADD_ALL_TESTS(TestInnerBlock_RC4::test_rc4_multi_call, data_len[3]);
			ADD_TEST(TestInnerBlock_RC4::test_rc_bulk);
		#endif
	}
	{
		#ifndef OPENSSL_NO_RC5
			// 
			// RC5 low level APIs are deprecated for public use, but still ok for internal use.
			// 
			static const uchar RC5key[5][16] = {
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x91, 0x5f, 0x46, 0x19, 0xbe, 0x41, 0xb2, 0x51, 0x63, 0x55, 0xa5, 0x01, 0x10, 0xa9, 0xce, 0x91},
				{0x78, 0x33, 0x48, 0xe7, 0x5a, 0xeb, 0x0f, 0x2f, 0xd7, 0xb1, 0x69, 0xbb, 0x8d, 0xc1, 0x67, 0x87},
				{0xdc, 0x49, 0xdb, 0x13, 0x75, 0xa5, 0x58, 0x4f, 0x64, 0x85, 0xb4, 0x13, 0xb5, 0xf1, 0x2b, 0xaf},
				{0x52, 0x69, 0xf1, 0x49, 0xd4, 0x1b, 0xa0, 0x15, 0x24, 0x97, 0x57, 0x4d, 0x7f, 0x15, 0x31, 0x25},
			};

			static const uchar RC5plain[5][8] = {
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x21, 0xA5, 0xDB, 0xEE, 0x15, 0x4B, 0x8F, 0x6D},
				{0xF7, 0xC0, 0x13, 0xAC, 0x5B, 0x2B, 0x89, 0x52},
				{0x2F, 0x42, 0xB3, 0xB7, 0x03, 0x69, 0xFC, 0x92},
				{0x65, 0xC1, 0x78, 0xB2, 0x84, 0xD1, 0x97, 0xCC},
			};

			static const uchar RC5cipher[5][8] = {
				{0x21, 0xA5, 0xDB, 0xEE, 0x15, 0x4B, 0x8F, 0x6D},
				{0xF7, 0xC0, 0x13, 0xAC, 0x5B, 0x2B, 0x89, 0x52},
				{0x2F, 0x42, 0xB3, 0xB7, 0x03, 0x69, 0xFC, 0x92},
				{0x65, 0xC1, 0x78, 0xB2, 0x84, 0xD1, 0x97, 0xCC},
				{0xEB, 0x44, 0xE4, 0x15, 0xDA, 0x31, 0x98, 0x24},
			};

			#define RC5_CBC_NUM 27
			static const uchar rc5_cbc_cipher[RC5_CBC_NUM][8] = {
				{0x7a, 0x7b, 0xba, 0x4d, 0x79, 0x11, 0x1d, 0x1e},
				{0x79, 0x7b, 0xba, 0x4d, 0x78, 0x11, 0x1d, 0x1e},
				{0x7a, 0x7b, 0xba, 0x4d, 0x79, 0x11, 0x1d, 0x1f},
				{0x7a, 0x7b, 0xba, 0x4d, 0x79, 0x11, 0x1d, 0x1f},
				{0x8b, 0x9d, 0xed, 0x91, 0xce, 0x77, 0x94, 0xa6},
				{0x2f, 0x75, 0x9f, 0xe7, 0xad, 0x86, 0xa3, 0x78},
				{0xdc, 0xa2, 0x69, 0x4b, 0xf4, 0x0e, 0x07, 0x88},
				{0xdc, 0xa2, 0x69, 0x4b, 0xf4, 0x0e, 0x07, 0x88},
				{0xdc, 0xfe, 0x09, 0x85, 0x77, 0xec, 0xa5, 0xff},
				{0x96, 0x46, 0xfb, 0x77, 0x63, 0x8f, 0x9c, 0xa8},
				{0xb2, 0xb3, 0x20, 0x9d, 0xb6, 0x59, 0x4d, 0xa4},
				{0x54, 0x5f, 0x7f, 0x32, 0xa5, 0xfc, 0x38, 0x36},
				{0x82, 0x85, 0xe7, 0xc1, 0xb5, 0xbc, 0x74, 0x02},
				{0xfc, 0x58, 0x6f, 0x92, 0xf7, 0x08, 0x09, 0x34},
				{0xcf, 0x27, 0x0e, 0xf9, 0x71, 0x7f, 0xf7, 0xc4},
				{0xe4, 0x93, 0xf1, 0xc1, 0xbb, 0x4d, 0x6e, 0x8c},
				{0x5c, 0x4c, 0x04, 0x1e, 0x0f, 0x21, 0x7a, 0xc3},
				{0x92, 0x1f, 0x12, 0x48, 0x53, 0x73, 0xb4, 0xf7},
				{0x5b, 0xa0, 0xca, 0x6b, 0xbe, 0x7f, 0x5f, 0xad},
				{0xc5, 0x33, 0x77, 0x1c, 0xd0, 0x11, 0x0e, 0x63},
				{0x29, 0x4d, 0xdb, 0x46, 0xb3, 0x27, 0x8d, 0x60},
				{0xda, 0xd6, 0xbd, 0xa9, 0xdf, 0xe8, 0xf7, 0xe8},
				{0x97, 0xe0, 0x78, 0x78, 0x37, 0xed, 0x31, 0x7f},
				{0x78, 0x75, 0xdb, 0xf6, 0x73, 0x8c, 0x64, 0x78},
				{0x8f, 0x34, 0xc3, 0xc6, 0x81, 0xc9, 0x96, 0x95},
				{0x7c, 0xb3, 0xf1, 0xdf, 0x34, 0xf9, 0x48, 0x11},
				{0x7f, 0xd1, 0xa0, 0x23, 0xa5, 0xbb, 0xa2, 0x17},
			};
			static const uchar rc5_cbc_key[RC5_CBC_NUM][17] = {
				{1, 0x00},
				{1, 0x00},
				{1, 0x00},
				{1, 0x00},
				{1, 0x00},
				{1, 0x11},
				{1, 0x00},
				{4, 0x00, 0x00, 0x00, 0x00},
				{1, 0x00},
				{1, 0x00},
				{1, 0x00},
				{1, 0x00},
				{4, 0x01, 0x02, 0x03, 0x04},
				{4, 0x01, 0x02, 0x03, 0x04},
				{4, 0x01, 0x02, 0x03, 0x04},
				{8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{16, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{16, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{16, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{5, 0x01, 0x02, 0x03, 0x04, 0x05},
				{5, 0x01, 0x02, 0x03, 0x04, 0x05},
				{5, 0x01, 0x02, 0x03, 0x04, 0x05},
				{5, 0x01, 0x02, 0x03, 0x04, 0x05},
				{5, 0x01, 0x02, 0x03, 0x04, 0x05},
			};
			static const uchar rc5_cbc_plain[RC5_CBC_NUM][8] = {
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
				{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
				{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
				{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
				{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
				{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
				{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
				{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
				{0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x01},
			};
			static const int rc5_cbc_rounds[RC5_CBC_NUM] = {
				0, 0, 0, 0, 0, 1, 2, 2,
				8, 8, 12, 16, 8, 12, 16, 12,
				8, 12, 16, 8, 12, 16, 12, 8,
				8, 8, 8,
			};
			static const uchar rc5_cbc_iv[RC5_CBC_NUM][8] = {
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x78, 0x75, 0xdb, 0xf6, 0x73, 0x8c, 0x64, 0x78},
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
				{0x7c, 0xb3, 0xf1, 0xdf, 0x34, 0xf9, 0x48, 0x11},
			};
			class TestInnerBlock_RC5 {
			public:
				static int test_rc5_ecb(int n)
				{
					int testresult = 1;
					RC5_32_KEY key;
					uchar buf[8], buf2[8];
					if(!TEST_true(RC5_32_set_key(&key, 16, &RC5key[n][0], 12)))
						return 0;
					RC5_32_ecb_encrypt(&RC5plain[n][0], buf, &key, RC5_ENCRYPT);
					if(!TEST_mem_eq(&RC5cipher[n][0], sizeof(RC5cipher[0]), buf, sizeof(buf)))
						testresult = 0;
					RC5_32_ecb_encrypt(buf, buf2, &key, RC5_DECRYPT);
					if(!TEST_mem_eq(&RC5plain[n][0], sizeof(RC5cipher[0]), buf2, sizeof(buf2)))
						testresult = 0;
					return testresult;
				}
				static int test_rc5_cbc(int n)
				{
					int testresult = 1;
					RC5_32_KEY key;
					uchar buf[8], buf2[8], ivb[8];
					int i = rc5_cbc_rounds[n];
					if(i >= 8) {
						if(!TEST_true(RC5_32_set_key(&key, rc5_cbc_key[n][0], &rc5_cbc_key[n][1], i)))
							return 0;
						memcpy(ivb, &rc5_cbc_iv[n][0], 8);
						RC5_32_cbc_encrypt(&rc5_cbc_plain[n][0], buf, 8, &key, &ivb[0], RC5_ENCRYPT);
						if(!TEST_mem_eq(&rc5_cbc_cipher[n][0], sizeof(rc5_cbc_cipher[0]), buf, sizeof(buf)))
							testresult = 0;
						memcpy(ivb, &rc5_cbc_iv[n][0], 8);
						RC5_32_cbc_encrypt(buf, buf2, 8, &key, &ivb[0], RC5_DECRYPT);
						if(!TEST_mem_eq(&rc5_cbc_plain[n][0], sizeof(rc5_cbc_plain[0]), buf2, sizeof(buf2)))
							testresult = 0;
					}
					return testresult;
				}
			};

			ADD_ALL_TESTS(TestInnerBlock_RC5::test_rc5_ecb, SIZEOFARRAY(RC5key));
			ADD_ALL_TESTS(TestInnerBlock_RC5::test_rc5_cbc, RC5_CBC_NUM);
			#undef RC5_CBC_NUM
		#endif
	}
	{
		static const uchar simple1[] = { 0xff };
		static const uchar simple2[] = { 0x01, 0xff };
		static const uchar simple3[] = { 0x00, 0x00, 0x00, 0x01, 0xff };
		static const uchar nestedsub[] = { 0x03, 0xff, 0x01, 0xff };
		static const uchar seqsub[] = { 0x01, 0xff, 0x01, 0xff };
		static const uchar empty[] = { 0x00 };
		static const uchar alloc[] = { 0x02, 0xfe, 0xff };
		static const uchar submem[] = { 0x03, 0x02, 0xfe, 0xff };
		static const uchar fixed[] = { 0xff, 0xff, 0xff };
		static const uchar simpleder[] = { 0xfc, 0x04, 0x00, 0x01, 0x02, 0x03, 0xff, 0xfe, 0xfd };

		class TestInnerBlock_WPACKET {
		public:
			static int cleanup(WPACKET * pkt)
			{
				WPACKET_cleanup(pkt);
				return 0;
			}
			static int test_WPACKET_init()
			{
				WPACKET pkt;
				int i;
				size_t written;
				uchar sbuf[3];
				if(!TEST_true(WPACKET_init(&pkt, TestData_WPACKET_buf))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					/* Closing a top level WPACKET should fail */
					|| !TEST_false(WPACKET_close(&pkt))
					/* Finishing a top level WPACKET should succeed */
					|| !TEST_true(WPACKET_finish(&pkt))
					/*
					 * Can't call close or finish on a WPACKET that's already finished.
					 */
					|| !TEST_false(WPACKET_close(&pkt))
					|| !TEST_false(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, simple1, sizeof(simple1)))
					return cleanup(&pkt);
				/* Now try with a one byte length prefix */
				if(!TEST_true(WPACKET_init_len(&pkt, TestData_WPACKET_buf, 1))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, simple2, sizeof(simple2)))
					return cleanup(&pkt);
				/* And a longer length prefix */
				if(!TEST_true(WPACKET_init_len(&pkt, TestData_WPACKET_buf, 4))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, simple3, sizeof(simple3)))
					return cleanup(&pkt);
				if(!TEST_true(WPACKET_init_len(&pkt, TestData_WPACKET_buf, 1)))
					return cleanup(&pkt);
				for(i = 1; i < 257; i++) {
					/*
					 * Putting more bytes in than fit for the size of the length prefix should fail
					 */
					if(!TEST_int_eq(WPACKET_put_bytes_u8(&pkt, 0xff), i < 256))
						return cleanup(&pkt);
				}
				if(!TEST_true(WPACKET_finish(&pkt)))
					return cleanup(&pkt);
				/* Test initialising from a fixed size buffer */
				if(!TEST_true(WPACKET_init_static_len(&pkt, sbuf, sizeof(sbuf), 0))
					/* Adding 3 bytes should succeed */
					|| !TEST_true(WPACKET_put_bytes_u24(&pkt, 0xffffff))
					/* Adding 1 more byte should fail */
					|| !TEST_false(WPACKET_put_bytes_u8(&pkt, 0xff))
					/* Finishing the top level WPACKET should succeed */
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(sbuf, written, fixed, sizeof(sbuf))
					/* Initialise with 1 len byte */
					|| !TEST_true(WPACKET_init_static_len(&pkt, sbuf, sizeof(sbuf), 1))
					/* Adding 2 bytes should succeed */
					|| !TEST_true(WPACKET_put_bytes_u16(&pkt, 0xfeff))
					/* Adding 1 more byte should fail */
					|| !TEST_false(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(sbuf, written, alloc, sizeof(alloc)))
					return cleanup(&pkt);
				return 1;
			}
			static int test_WPACKET_set_max_size()
			{
				WPACKET pkt;
				size_t written;
				if(!TEST_true(WPACKET_init(&pkt, TestData_WPACKET_buf))
					/*
					 * No previous lenbytes set so we should be ok to set the max possible max size
					 */
					|| !TEST_true(WPACKET_set_max_size(&pkt, SIZE_MAX))
					/* We should be able to set it smaller too */
					|| !TEST_true(WPACKET_set_max_size(&pkt, SIZE_MAX -1))
					/* And setting it bigger again should be ok */
					|| !TEST_true(WPACKET_set_max_size(&pkt, SIZE_MAX))
					|| !TEST_true(WPACKET_finish(&pkt)))
					return cleanup(&pkt);
				if(!TEST_true(WPACKET_init_len(&pkt, TestData_WPACKET_buf, 1))
					/*
					 * Should fail because we already consumed 1 byte with the length
					 */
					|| !TEST_false(WPACKET_set_max_size(&pkt, 0))
					/*
					 * Max size can't be bigger than biggest that will fit in lenbytes
					 */
					|| !TEST_false(WPACKET_set_max_size(&pkt, 0x0101))
					/* It can be the same as the maximum possible size */
					|| !TEST_true(WPACKET_set_max_size(&pkt, 0x0100))
					/* Or it can be less */
					|| !TEST_true(WPACKET_set_max_size(&pkt, 0x01))
					/* Should fail because packet is already filled */
					|| !TEST_false(WPACKET_put_bytes_u8(&pkt, 0xff))
					/* You can't put in more bytes than max size */
					|| !TEST_true(WPACKET_set_max_size(&pkt, 0x02))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_false(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, simple2, sizeof(simple2)))
					return cleanup(&pkt);
				return 1;
			}
			static int test_WPACKET_start_sub_packet()
			{
				WPACKET pkt;
				size_t written;
				size_t len;
				if(!TEST_true(WPACKET_init(&pkt, TestData_WPACKET_buf))
					|| !TEST_true(WPACKET_start_sub_packet(&pkt))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					/* Can't finish because we have a sub packet */
					|| !TEST_false(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_close(&pkt))
					/* Sub packet is closed so can't close again */
					|| !TEST_false(WPACKET_close(&pkt))
					/* Now a top level so finish should succeed */
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, simple1, sizeof(simple1)))
					return cleanup(&pkt);
				/* Single sub-packet with length prefix */
				if(!TEST_true(WPACKET_init(&pkt, TestData_WPACKET_buf))
					|| !TEST_true(WPACKET_start_sub_packet_u8(&pkt))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, simple2, sizeof(simple2)))
					return cleanup(&pkt);
				/* Nested sub-packets with length prefixes */
				if(!TEST_true(WPACKET_init(&pkt, TestData_WPACKET_buf))
					|| !TEST_true(WPACKET_start_sub_packet_u8(&pkt))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_start_sub_packet_u8(&pkt))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_get_length(&pkt, &len))
					|| !TEST_size_t_eq(len, 1)
					|| !TEST_true(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_get_length(&pkt, &len))
					|| !TEST_size_t_eq(len, 3)
					|| !TEST_true(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, nestedsub, sizeof(nestedsub)))
					return cleanup(&pkt);
				/* Sequential sub-packets with length prefixes */
				if(!TEST_true(WPACKET_init(&pkt, TestData_WPACKET_buf))
					|| !TEST_true(WPACKET_start_sub_packet_u8(&pkt))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_start_sub_packet_u8(&pkt))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, seqsub, sizeof(seqsub)))
					return cleanup(&pkt);
				/* Nested sub-packets with lengths filled before finish */
				if(!TEST_true(WPACKET_init(&pkt, TestData_WPACKET_buf))
					|| !TEST_true(WPACKET_start_sub_packet_u8(&pkt))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_start_sub_packet_u8(&pkt))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_get_length(&pkt, &len))
					|| !TEST_size_t_eq(len, 1)
					|| !TEST_true(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_get_length(&pkt, &len))
					|| !TEST_size_t_eq(len, 3)
					|| !TEST_true(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_fill_lengths(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, nestedsub, sizeof(nestedsub))
					|| !TEST_true(WPACKET_finish(&pkt)))
					return cleanup(&pkt);
				return 1;
			}
			static int test_WPACKET_set_flags()
			{
				WPACKET pkt;
				size_t written;
				/* Set packet to be non-zero length */
				if(!TEST_true(WPACKET_init(&pkt, TestData_WPACKET_buf))
					|| !TEST_true(WPACKET_set_flags(&pkt, WPACKET_FLAGS_NON_ZERO_LENGTH))
					/* Should fail because of zero length */
					|| !TEST_false(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, simple1, sizeof(simple1)))
					return cleanup(&pkt);
				/* Repeat above test in a sub-packet */
				if(!TEST_true(WPACKET_init(&pkt, TestData_WPACKET_buf))
					|| !TEST_true(WPACKET_start_sub_packet(&pkt))
					|| !TEST_true(WPACKET_set_flags(&pkt, WPACKET_FLAGS_NON_ZERO_LENGTH))
					/* Should fail because of zero length */
					|| !TEST_false(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, simple1, sizeof(simple1)))
					return cleanup(&pkt);
				/* Set packet to abandon non-zero length */
				if(!TEST_true(WPACKET_init_len(&pkt, TestData_WPACKET_buf, 1))
					|| !TEST_true(WPACKET_set_flags(&pkt, WPACKET_FLAGS_ABANDON_ON_ZERO_LENGTH))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_size_t_eq(written, 0))
					return cleanup(&pkt);
				/* Repeat above test but only abandon a sub-packet */
				if(!TEST_true(WPACKET_init_len(&pkt, TestData_WPACKET_buf, 1))
					|| !TEST_true(WPACKET_start_sub_packet_u8(&pkt))
					|| !TEST_true(WPACKET_set_flags(&pkt, WPACKET_FLAGS_ABANDON_ON_ZERO_LENGTH))
					|| !TEST_true(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, empty, sizeof(empty)))
					return cleanup(&pkt);
				/* And repeat with a non empty sub-packet */
				if(!TEST_true(WPACKET_init(&pkt, TestData_WPACKET_buf))
					|| !TEST_true(WPACKET_start_sub_packet_u8(&pkt))
					|| !TEST_true(WPACKET_set_flags(&pkt, WPACKET_FLAGS_ABANDON_ON_ZERO_LENGTH))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xff))
					|| !TEST_true(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, simple2, sizeof(simple2)))
					return cleanup(&pkt);
				return 1;
			}
			static int test_WPACKET_allocate_bytes()
			{
				WPACKET pkt;
				size_t written;
				uchar * bytes;
				if(!TEST_true(WPACKET_init_len(&pkt, TestData_WPACKET_buf, 1)) || !TEST_true(WPACKET_allocate_bytes(&pkt, 2, &bytes)))
					return cleanup(&pkt);
				bytes[0] = 0xfe;
				bytes[1] = 0xff;
				if(!TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, alloc, sizeof(alloc)))
					return cleanup(&pkt);
				/* Repeat with WPACKET_sub_allocate_bytes */
				if(!TEST_true(WPACKET_init_len(&pkt, TestData_WPACKET_buf, 1)) || !TEST_true(WPACKET_sub_allocate_bytes_u8(&pkt, 2, &bytes)))
					return cleanup(&pkt);
				bytes[0] = 0xfe;
				bytes[1] = 0xff;
				if(!TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, submem, sizeof(submem)))
					return cleanup(&pkt);
				return 1;
			}
			static int test_WPACKET_memcpy()
			{
				WPACKET pkt;
				size_t written;
				const uchar bytes[] = { 0xfe, 0xff };
				if(!TEST_true(WPACKET_init_len(&pkt, TestData_WPACKET_buf, 1))
					|| !TEST_true(WPACKET_memcpy(&pkt, bytes, sizeof(bytes)))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, alloc, sizeof(alloc)))
					return cleanup(&pkt);
				/* Repeat with WPACKET_sub_memcpy() */
				if(!TEST_true(WPACKET_init_len(&pkt, TestData_WPACKET_buf, 1))
					|| !TEST_true(WPACKET_sub_memcpy_u8(&pkt, bytes, sizeof(bytes)))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written))
					|| !TEST_mem_eq(TestData_WPACKET_buf->data, written, submem, sizeof(submem)))
					return cleanup(&pkt);
				return 1;
			}
			static int test_WPACKET_init_der()
			{
				WPACKET pkt;
				uchar sbuf[1024];
				uchar testdata[] = { 0x00, 0x01, 0x02, 0x03 };
				uchar testdata2[259]  = { 0x82, 0x01, 0x00 };
				size_t written[2];
				size_t size1, size2;
				int flags = WPACKET_FLAGS_ABANDON_ON_ZERO_LENGTH;
				int i;
				/* Test initialising for writing DER */
				if(!TEST_true(WPACKET_init_der(&pkt, sbuf, sizeof(sbuf)))
					|| !TEST_true(WPACKET_put_bytes_u24(&pkt, 0xfffefd))
					/* Test writing data in a length prefixed sub-packet */
					|| !TEST_true(WPACKET_start_sub_packet(&pkt))
					|| !TEST_true(WPACKET_memcpy(&pkt, testdata, sizeof(testdata)))
					|| !TEST_true(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_put_bytes_u8(&pkt, 0xfc))
					/* this sub-packet is empty, and should render zero bytes */
					|| (!TEST_true(WPACKET_start_sub_packet(&pkt))
					|| !TEST_true(WPACKET_set_flags(&pkt, flags))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &size1))
					|| !TEST_true(WPACKET_close(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &size2))
					|| !TEST_size_t_eq(size1, size2))
					|| !TEST_true(WPACKET_finish(&pkt))
					|| !TEST_true(WPACKET_get_total_written(&pkt, &written[0]))
					|| !TEST_mem_eq(WPACKET_get_curr(&pkt), written[0], simpleder, sizeof(simpleder)))
					return cleanup(&pkt);
				/* Generate random packet data for test */
				if(!TEST_true(RAND_bytes(&testdata2[3], sizeof(testdata2) - 3)))
					return 0;
				/*
				 * Test with a sub-packet that has 2 length bytes. We do 2 passes - first
				 * with a NULL buffer, just to calculate lengths, and a second pass with a
				 * real buffer to actually generate a packet
				 */
				for(i = 0; i < 2; i++) {
					if(i == 0) {
						if(!TEST_true(WPACKET_init_null_der(&pkt)))
							return 0;
					}
					else {
						if(!TEST_true(WPACKET_init_der(&pkt, sbuf, sizeof(sbuf))))
							return 0;
					}
					if(!TEST_true(WPACKET_start_sub_packet(&pkt)) || !TEST_true(WPACKET_memcpy(&pkt, &testdata2[3], sizeof(testdata2) - 3))
						|| !TEST_true(WPACKET_close(&pkt))
						|| !TEST_true(WPACKET_finish(&pkt))
						|| !TEST_true(WPACKET_get_total_written(&pkt, &written[i])))
						return cleanup(&pkt);
				}
				/*
				 * Check that the size calculated in the first pass equals the size of the
				 * packet actually generated in the second pass. Also check the generated
				 * packet looks as we expect it to.
				 */
				if(!TEST_size_t_eq(written[0], written[1]) || !TEST_mem_eq(WPACKET_get_curr(&pkt), written[1], testdata2, sizeof(testdata2)))
					return 0;
				return 1;
			}
		};

		if(!TEST_ptr(TestData_WPACKET_buf = BUF_MEM_new()))
			return 0;
		ADD_TEST(TestInnerBlock_WPACKET::test_WPACKET_init);
		ADD_TEST(TestInnerBlock_WPACKET::test_WPACKET_set_max_size);
		ADD_TEST(TestInnerBlock_WPACKET::test_WPACKET_start_sub_packet);
		ADD_TEST(TestInnerBlock_WPACKET::test_WPACKET_set_flags);
		ADD_TEST(TestInnerBlock_WPACKET::test_WPACKET_allocate_bytes);
		ADD_TEST(TestInnerBlock_WPACKET::test_WPACKET_memcpy);
		ADD_TEST(TestInnerBlock_WPACKET::test_WPACKET_init_der);
	}
	{
		static int error_callback_fired;

		class TestInnerBlock_BioMemLeak {
		public:
			static int test_bio_memleak()
			{
				int ok = 0;
				BUF_MEM bufmem;
				static const char str[] = "BIO test\n";
				char buf[100];
				BIO * bio = BIO_new(BIO_s_mem());
				if(!TEST_ptr(bio))
					goto finish;
				bufmem.length = sizeof(str);
				bufmem.data = (char*)str;
				bufmem.max = bufmem.length;
				BIO_set_mem_buf(bio, &bufmem, BIO_NOCLOSE);
				BIO_set_flags(bio, BIO_FLAGS_MEM_RDONLY);
				if(!TEST_int_eq(BIO_read(bio, buf, sizeof(buf)), sizeof(str)))
					goto finish;
				if(!TEST_mem_eq(buf, sizeof(str), str, sizeof(str)))
					goto finish;
				ok = 1;
			finish:
				BIO_free(bio);
				return ok;
			}
			static int test_bio_get_mem()
			{
				int ok = 0;
				BUF_MEM * bufmem = NULL;
				BIO * bio = BIO_new(BIO_s_mem());
				if(!TEST_ptr(bio))
					goto finish;
				if(!TEST_int_eq(BIO_puts(bio, "Hello World\n"), 12))
					goto finish;
				BIO_get_mem_ptr(bio, &bufmem);
				if(!TEST_ptr(bufmem))
					goto finish;
				if(!TEST_int_gt(BIO_set_close(bio, BIO_NOCLOSE), 0))
					goto finish;
				BIO_free(bio);
				bio = NULL;
				if(!TEST_mem_eq(bufmem->data, bufmem->length, "Hello World\n", 12))
					goto finish;
				ok = 1;
			finish:
				BIO_free(bio);
				BUF_MEM_free(bufmem);
				return ok;
			}
			static int test_bio_new_mem_buf()
			{
				int ok = 0;
				BUF_MEM * bufmem;
				char data[16];
				BIO * bio = BIO_new_mem_buf("Hello World\n", 12);
				if(!TEST_ptr(bio))
					goto finish;
				if(!TEST_int_eq(BIO_read(bio, data, 5), 5))
					goto finish;
				if(!TEST_mem_eq(data, 5, "Hello", 5))
					goto finish;
				if(!TEST_int_gt(BIO_get_mem_ptr(bio, &bufmem), 0))
					goto finish;
				if(!TEST_int_lt(BIO_write(bio, "test", 4), 0))
					goto finish;
				if(!TEST_int_eq(BIO_read(bio, data, 16), 7))
					goto finish;
				if(!TEST_mem_eq(data, 7, " World\n", 7))
					goto finish;
				if(!TEST_int_gt(BIO_reset(bio), 0))
					goto finish;
				if(!TEST_int_eq(BIO_read(bio, data, 16), 12))
					goto finish;
				if(!TEST_mem_eq(data, 12, "Hello World\n", 12))
					goto finish;
				ok = 1;
			finish:
				BIO_free(bio);
				return ok;
			}
			static int test_bio_rdonly_mem_buf()
			{
				int ok = 0;
				BIO * bio2 = NULL;
				BUF_MEM * bufmem;
				char data[16];
				BIO * bio = BIO_new_mem_buf("Hello World\n", 12);
				if(!TEST_ptr(bio))
					goto finish;
				if(!TEST_int_eq(BIO_read(bio, data, 5), 5))
					goto finish;
				if(!TEST_mem_eq(data, 5, "Hello", 5))
					goto finish;
				if(!TEST_int_gt(BIO_get_mem_ptr(bio, &bufmem), 0))
					goto finish;
				(void)BIO_set_close(bio, BIO_NOCLOSE);
				bio2 = BIO_new(BIO_s_mem());
				if(!TEST_ptr(bio2))
					goto finish;
				BIO_set_mem_buf(bio2, bufmem, BIO_CLOSE);
				BIO_set_flags(bio2, BIO_FLAGS_MEM_RDONLY);
				if(!TEST_int_eq(BIO_read(bio2, data, 16), 7))
					goto finish;
				if(!TEST_mem_eq(data, 7, " World\n", 7))
					goto finish;
				if(!TEST_int_gt(BIO_reset(bio2), 0))
					goto finish;
				if(!TEST_int_eq(BIO_read(bio2, data, 16), 7))
					goto finish;
				if(!TEST_mem_eq(data, 7, " World\n", 7))
					goto finish;
				ok = 1;
			finish:
				BIO_free(bio);
				BIO_free(bio2);
				return ok;
			}
			static int test_bio_rdwr_rdonly()
			{
				int ok = 0;
				char data[16];
				BIO * bio = BIO_new(BIO_s_mem());
				if(!TEST_ptr(bio))
					goto finish;
				if(!TEST_int_eq(BIO_puts(bio, "Hello World\n"), 12))
					goto finish;
				BIO_set_flags(bio, BIO_FLAGS_MEM_RDONLY);
				if(!TEST_int_eq(BIO_read(bio, data, 16), 12))
					goto finish;
				if(!TEST_mem_eq(data, 12, "Hello World\n", 12))
					goto finish;
				if(!TEST_int_gt(BIO_reset(bio), 0))
					goto finish;
				BIO_clear_flags(bio, BIO_FLAGS_MEM_RDONLY);
				if(!TEST_int_eq(BIO_puts(bio, "Hi!\n"), 4))
					goto finish;
				if(!TEST_int_eq(BIO_read(bio, data, 16), 16))
					goto finish;
				if(!TEST_mem_eq(data, 16, "Hello World\nHi!\n", 16))
					goto finish;
				ok = 1;
			finish:
				BIO_free(bio);
				return ok;
			}
			static int test_bio_nonclear_rst()
			{
				int ok = 0;
				char data[16];
				BIO * bio = BIO_new(BIO_s_mem());
				if(!TEST_ptr(bio))
					goto finish;
				if(!TEST_int_eq(BIO_puts(bio, "Hello World\n"), 12))
					goto finish;
				BIO_set_flags(bio, BIO_FLAGS_NONCLEAR_RST);
				if(!TEST_int_eq(BIO_read(bio, data, 16), 12))
					goto finish;
				if(!TEST_mem_eq(data, 12, "Hello World\n", 12))
					goto finish;
				if(!TEST_int_gt(BIO_reset(bio), 0))
					goto finish;
				if(!TEST_int_eq(BIO_read(bio, data, 16), 12))
					goto finish;
				if(!TEST_mem_eq(data, 12, "Hello World\n", 12))
					goto finish;
				BIO_clear_flags(bio, BIO_FLAGS_NONCLEAR_RST);
				if(!TEST_int_gt(BIO_reset(bio), 0))
					goto finish;
				if(!TEST_int_lt(BIO_read(bio, data, 16), 1))
					goto finish;
				ok = 1;
			finish:
				BIO_free(bio);
				return ok;
			}
			static long BIO_error_callback(BIO * bio, int cmd, const char * argp, size_t len, int argi, long argl, int ret, size_t * processed)
			{
				if((cmd & (BIO_CB_READ | BIO_CB_RETURN)) != 0) {
					error_callback_fired = 1;
					ret = 0; /* fail for read operations to simulate error in input BIO */
				}
				return ret;
			}
			// Checks i2d_ASN1_bio_stream() is freeing all memory when input BIO ends unexpectedly.
			static int test_bio_i2d_ASN1_mime()
			{
				int ok = 0;
				BIO * bio = NULL, * out = NULL;
				BUF_MEM bufmem;
				static const char str[] = "BIO mime test\n";
				PKCS7 * p7 = NULL;
				if(!TEST_ptr(bio = BIO_new(BIO_s_mem())))
					goto finish;
				bufmem.length = sizeof(str);
				bufmem.data = (char*)str;
				bufmem.max = bufmem.length;
				BIO_set_mem_buf(bio, &bufmem, BIO_NOCLOSE);
				BIO_set_flags(bio, BIO_FLAGS_MEM_RDONLY);
				BIO_set_callback_ex(bio, BIO_error_callback);
				if(!TEST_ptr(out = BIO_new(BIO_s_mem())))
					goto finish;
				if(!TEST_ptr(p7 = PKCS7_new()))
					goto finish;
				if(!TEST_true(PKCS7_set_type(p7, NID_pkcs7_data)))
					goto finish;
				error_callback_fired = 0;
				/*
				 * The call succeeds even if the input stream ends unexpectedly as
				 * there is no handling for this case in SMIME_crlf_copy().
				 */
				if(!TEST_true(i2d_ASN1_bio_stream(out, (ASN1_VALUE*)p7, bio, SMIME_STREAM | SMIME_BINARY, ASN1_ITEM_rptr(PKCS7))))
					goto finish;
				if(!TEST_int_eq(error_callback_fired, 1))
					goto finish;
				ok = 1;
			finish:
				BIO_free(bio);
				BIO_free(out);
				PKCS7_free(p7);
				return ok;
			}
		};
		ADD_TEST(TestInnerBlock_BioMemLeak::test_bio_memleak);
		ADD_TEST(TestInnerBlock_BioMemLeak::test_bio_get_mem);
		ADD_TEST(TestInnerBlock_BioMemLeak::test_bio_new_mem_buf);
		ADD_TEST(TestInnerBlock_BioMemLeak::test_bio_rdonly_mem_buf);
		ADD_TEST(TestInnerBlock_BioMemLeak::test_bio_rdwr_rdonly);
		ADD_TEST(TestInnerBlock_BioMemLeak::test_bio_nonclear_rst);
		ADD_TEST(TestInnerBlock_BioMemLeak::test_bio_i2d_ASN1_mime);
	}
	{
		static uchar TestData_CmpAsn_rand_data[OSSL_CMP_TRANSACTIONID_LENGTH];

		typedef struct test_fixture {
			const char * test_case_name;
			int expected;
			ASN1_OCTET_STRING * src_string;
			ASN1_OCTET_STRING * tgt_string;
		} CMP_ASN_TEST_FIXTURE;

		class TestInnerBlock_CmpAsn {
		public:
			static CMP_ASN_TEST_FIXTURE * set_up(const char * const test_case_name)
			{
				CMP_ASN_TEST_FIXTURE * fixture;
				if(!TEST_ptr(fixture = (CMP_ASN_TEST_FIXTURE*)OPENSSL_zalloc(sizeof(*fixture))))
					return NULL;
				fixture->test_case_name = test_case_name;
				return fixture;
			}
			static void tear_down(CMP_ASN_TEST_FIXTURE * fixture)
			{
				ASN1_OCTET_STRING_free(fixture->src_string);
				if(fixture->tgt_string != fixture->src_string)
					ASN1_OCTET_STRING_free(fixture->tgt_string);
				OPENSSL_free(fixture);
			}
			static int execute_cmp_asn1_get_int_test(CMP_ASN_TEST_FIXTURE * fixture)
			{
				int res;
				ASN1_INTEGER * asn1integer = ASN1_INTEGER_new();
				if(!TEST_ptr(asn1integer))
					return 0;
				ASN1_INTEGER_set(asn1integer, 77);
				res = TEST_int_eq(77, ossl_cmp_asn1_get_int(asn1integer));
				ASN1_INTEGER_free(asn1integer);
				return res;
			}
			static int test_cmp_asn1_get_int()
			{
				SETUP_TEST_FIXTURE(CMP_ASN_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				EXECUTE_TEST(execute_cmp_asn1_get_int_test, tear_down);
				return result;
			}
			static int execute_CMP_ASN1_OCTET_STRING_set1_test(CMP_ASN_TEST_FIXTURE * fixture)
			{
				if(!TEST_int_eq(fixture->expected, ossl_cmp_asn1_octet_string_set1(&fixture->tgt_string, fixture->src_string)))
					return 0;
				if(fixture->expected != 0)
					return TEST_int_eq(0, ASN1_OCTET_STRING_cmp(fixture->tgt_string, fixture->src_string));
				return 1;
			}
			static int test_ASN1_OCTET_STRING_set()
			{
				SETUP_TEST_FIXTURE(CMP_ASN_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				if(!TEST_ptr(fixture->tgt_string = ASN1_OCTET_STRING_new())
					|| !TEST_ptr(fixture->src_string = ASN1_OCTET_STRING_new())
					|| !TEST_true(ASN1_OCTET_STRING_set(fixture->src_string, TestData_CmpAsn_rand_data, sizeof(TestData_CmpAsn_rand_data)))) {
					tear_down(fixture);
					fixture = NULL;
				}
				EXECUTE_TEST(execute_CMP_ASN1_OCTET_STRING_set1_test, tear_down);
				return result;
			}
			static int test_ASN1_OCTET_STRING_set_tgt_is_src()
			{
				SETUP_TEST_FIXTURE(CMP_ASN_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				if(!TEST_ptr(fixture->src_string = ASN1_OCTET_STRING_new())
					|| !(fixture->tgt_string = fixture->src_string)
					|| !TEST_true(ASN1_OCTET_STRING_set(fixture->src_string, TestData_CmpAsn_rand_data, sizeof(TestData_CmpAsn_rand_data)))) {
					tear_down(fixture);
					fixture = NULL;
				}
				EXECUTE_TEST(execute_CMP_ASN1_OCTET_STRING_set1_test, tear_down);
				return result;
			}
		};
		RAND_bytes(TestData_CmpAsn_rand_data, OSSL_CMP_TRANSACTIONID_LENGTH);
		// ASN.1 related tests 
		ADD_TEST(TestInnerBlock_CmpAsn::test_cmp_asn1_get_int);
		ADD_TEST(TestInnerBlock_CmpAsn::test_ASN1_OCTET_STRING_set);
		ADD_TEST(TestInnerBlock_CmpAsn::test_ASN1_OCTET_STRING_set_tgt_is_src);
	}
	{
		static uchar TestData_CmpHdr_rand_data[OSSL_CMP_TRANSACTIONID_LENGTH];

		typedef struct test_fixture {
			const char * test_case_name;
			int expected;
			OSSL_CMP_CTX * cmp_ctx;
			OSSL_CMP_PKIHEADER * hdr;
		} CMP_HDR_TEST_FIXTURE;

		class TestInnerBlock_CmpHdr {
		public:
			static void tear_down(CMP_HDR_TEST_FIXTURE * fixture)
			{
				OSSL_CMP_PKIHEADER_free(fixture->hdr);
				OSSL_CMP_CTX_free(fixture->cmp_ctx);
				OPENSSL_free(fixture);
			}
			static CMP_HDR_TEST_FIXTURE * set_up(const char * const test_case_name)
			{
				CMP_HDR_TEST_FIXTURE * fixture;
				if(!TEST_ptr(fixture = (CMP_HDR_TEST_FIXTURE*)OPENSSL_zalloc(sizeof(*fixture))))
					return NULL;
				fixture->test_case_name = test_case_name;
				if(!TEST_ptr(fixture->cmp_ctx = OSSL_CMP_CTX_new(NULL, NULL)))
					goto err;
				if(!TEST_ptr(fixture->hdr = OSSL_CMP_PKIHEADER_new()))
					goto err;
				return fixture;
			err:
				tear_down(fixture);
				return NULL;
			}
			static int execute_HDR_set_get_pvno_test(CMP_HDR_TEST_FIXTURE * fixture)
			{
				int pvno = 77;
				if(!TEST_int_eq(ossl_cmp_hdr_set_pvno(fixture->hdr, pvno), 1))
					return 0;
				if(!TEST_int_eq(ossl_cmp_hdr_get_pvno(fixture->hdr), pvno))
					return 0;
				return 1;
			}
			static int test_HDR_set_get_pvno()
			{
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				EXECUTE_TEST(execute_HDR_set_get_pvno_test, tear_down);
				return result;
			}

			#define X509_NAME_ADD(n, rd, s) X509_NAME_add_entry_by_txt((n), (rd), MBSTRING_ASC, (uchar *)(s), -1, -1, 0)

			static int execute_HDR_get0_senderNonce_test(CMP_HDR_TEST_FIXTURE * fixture)
			{
				X509_NAME * sender = X509_NAME_new();
				ASN1_OCTET_STRING * sn;
				if(!TEST_ptr(sender))
					return 0;
				X509_NAME_ADD(sender, "CN", "A common sender name");
				if(!TEST_int_eq(OSSL_CMP_CTX_set1_subjectName(fixture->cmp_ctx, sender), 1))
					return 0;
				if(!TEST_int_eq(ossl_cmp_hdr_init(fixture->cmp_ctx, fixture->hdr), 1))
					return 0;
				sn = ossl_cmp_hdr_get0_senderNonce(fixture->hdr);
				if(!TEST_int_eq(ASN1_OCTET_STRING_cmp(fixture->cmp_ctx->senderNonce, sn), 0))
					return 0;
				X509_NAME_free(sender);
				return 1;
			}
			static int test_HDR_get0_senderNonce()
			{
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				EXECUTE_TEST(execute_HDR_get0_senderNonce_test, tear_down);
				return result;
			}
			static int execute_HDR_set1_sender_test(CMP_HDR_TEST_FIXTURE * fixture)
			{
				X509_NAME * x509name = X509_NAME_new();
				if(!TEST_ptr(x509name))
					return 0;
				X509_NAME_ADD(x509name, "CN", "A common sender name");
				if(!TEST_int_eq(ossl_cmp_hdr_set1_sender(fixture->hdr, x509name), 1))
					return 0;
				if(!TEST_int_eq(fixture->hdr->sender->type, GEN_DIRNAME))
					return 0;
				if(!TEST_int_eq(X509_NAME_cmp(fixture->hdr->sender->d.directoryName, x509name), 0))
					return 0;
				X509_NAME_free(x509name);
				return 1;
			}
			static int test_HDR_set1_sender()
			{
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				EXECUTE_TEST(execute_HDR_set1_sender_test, tear_down);
				return result;
			}
			static int execute_HDR_set1_recipient_test(CMP_HDR_TEST_FIXTURE * fixture)
			{
				X509_NAME * x509name = X509_NAME_new();
				if(!TEST_ptr(x509name))
					return 0;
				X509_NAME_ADD(x509name, "CN", "A common recipient name");
				if(!TEST_int_eq(ossl_cmp_hdr_set1_recipient(fixture->hdr, x509name), 1))
					return 0;
				if(!TEST_int_eq(fixture->hdr->recipient->type, GEN_DIRNAME))
					return 0;
				if(!TEST_int_eq(X509_NAME_cmp(fixture->hdr->recipient->d.directoryName, x509name), 0))
					return 0;
				X509_NAME_free(x509name);
				return 1;
			}
			static int test_HDR_set1_recipient()
			{
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				EXECUTE_TEST(execute_HDR_set1_recipient_test, tear_down);
				return result;
			}
			static int execute_HDR_update_messageTime_test(CMP_HDR_TEST_FIXTURE * fixture)
			{
				struct tm hdrtm;
				time_t hdrtime, after;
				time_t now = time(NULL);
				/*
				 * Trial and error reveals that passing the return value from gmtime
				 * directly to mktime in a mingw 32 bit build gives unexpected results. To
				 * work around this we take a copy of the return value first.
				 */
				struct tm tmptm = *gmtime(&now);
				time_t before = mktime(&tmptm);
				if(!TEST_true(ossl_cmp_hdr_update_messageTime(fixture->hdr)))
					return 0;
				if(!TEST_true(ASN1_TIME_to_tm(fixture->hdr->messageTime, &hdrtm)))
					return 0;
				hdrtime = mktime(&hdrtm);
				if(!TEST_time_t_le(before, hdrtime))
					return 0;
				now = time(NULL);
				tmptm = *gmtime(&now);
				after = mktime(&tmptm);
				return TEST_time_t_le(hdrtime, after);
			}
			static int test_HDR_update_messageTime()
			{
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				EXECUTE_TEST(execute_HDR_update_messageTime_test, tear_down);
				return result;
			}
			static int execute_HDR_set1_senderKID_test(CMP_HDR_TEST_FIXTURE * fixture)
			{
				ASN1_OCTET_STRING * senderKID = ASN1_OCTET_STRING_new();
				int res = 0;
				if(!TEST_ptr(senderKID))
					return 0;
				if(!TEST_int_eq(ASN1_OCTET_STRING_set(senderKID, TestData_CmpHdr_rand_data, sizeof(TestData_CmpHdr_rand_data)), 1))
					goto err;
				if(!TEST_int_eq(ossl_cmp_hdr_set1_senderKID(fixture->hdr, senderKID), 1))
					goto err;
				if(!TEST_int_eq(ASN1_OCTET_STRING_cmp(fixture->hdr->senderKID, senderKID), 0))
					goto err;
				res = 1;
			err:
				ASN1_OCTET_STRING_free(senderKID);
				return res;
			}
			static int test_HDR_set1_senderKID()
			{
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				EXECUTE_TEST(execute_HDR_set1_senderKID_test, tear_down);
				return result;
			}
			static int execute_HDR_push0_freeText_test(CMP_HDR_TEST_FIXTURE * fixture)
			{
				ASN1_UTF8STRING * text = ASN1_UTF8STRING_new();
				if(!TEST_ptr(text))
					return 0;
				if(!ASN1_STRING_set(text, "A free text", -1))
					goto err;
				if(!TEST_int_eq(ossl_cmp_hdr_push0_freeText(fixture->hdr, text), 1))
					goto err;
				if(!TEST_true(text == sk_ASN1_UTF8STRING_value(fixture->hdr->freeText, 0)))
					goto err;
				return 1;
			err:
				ASN1_UTF8STRING_free(text);
				return 0;
			}
			static int test_HDR_push0_freeText()
			{
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				EXECUTE_TEST(execute_HDR_push0_freeText_test, tear_down);
				return result;
			}
			static int execute_HDR_push1_freeText_test(CMP_HDR_TEST_FIXTURE * fixture)
			{
				ASN1_UTF8STRING * text = ASN1_UTF8STRING_new();
				ASN1_UTF8STRING * pushed_text;
				int res = 0;
				if(!TEST_ptr(text))
					return 0;
				if(!ASN1_STRING_set(text, "A free text", -1))
					goto err;
				if(!TEST_int_eq(ossl_cmp_hdr_push1_freeText(fixture->hdr, text), 1))
					goto err;
				pushed_text = sk_ASN1_UTF8STRING_value(fixture->hdr->freeText, 0);
				if(!TEST_int_eq(ASN1_STRING_cmp(text, pushed_text), 0))
					goto err;
				res = 1;
			err:
				ASN1_UTF8STRING_free(text);
				return res;
			}
			static int test_HDR_push1_freeText()
			{
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				EXECUTE_TEST(execute_HDR_push1_freeText_test, tear_down);
				return result;
			}
			static int execute_HDR_generalInfo_push0_item_test(CMP_HDR_TEST_FIXTURE * fixture)
			{
				OSSL_CMP_ITAV * itav = OSSL_CMP_ITAV_new();
				if(!TEST_ptr(itav))
					return 0;
				if(!TEST_int_eq(ossl_cmp_hdr_generalInfo_push0_item(fixture->hdr, itav), 1))
					return 0;
				if(!TEST_true(itav == sk_OSSL_CMP_ITAV_value(fixture->hdr->generalInfo, 0)))
					return 0;
				return 1;
			}
			static int test_HDR_generalInfo_push0_item()
			{
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				EXECUTE_TEST(execute_HDR_generalInfo_push0_item_test, tear_down);
				return result;
			}
			static int execute_HDR_generalInfo_push1_items_test(CMP_HDR_TEST_FIXTURE * fixture)
			{
				const char oid[] = "1.2.3.4";
				char buf[20];
				OSSL_CMP_ITAV * itav, * pushed_itav;
				STACK_OF(OSSL_CMP_ITAV) *itavs = NULL, *ginfo;
				ASN1_INTEGER * asn1int = ASN1_INTEGER_new();
				ASN1_TYPE * val = ASN1_TYPE_new();
				ASN1_TYPE * pushed_val;
				int res = 0;
				if(!TEST_ptr(asn1int))
					return 0;
				if(!TEST_ptr(val)) {
					ASN1_INTEGER_free(asn1int);
					return 0;
				}
				ASN1_INTEGER_set(asn1int, 88);
				ASN1_TYPE_set(val, V_ASN1_INTEGER, asn1int);
				if(!TEST_ptr(itav = OSSL_CMP_ITAV_create(OBJ_txt2obj(oid, 1), val))) {
					ASN1_TYPE_free(val);
					return 0;
				}
				if(!TEST_true(OSSL_CMP_ITAV_push0_stack_item(&itavs, itav))) {
					OSSL_CMP_ITAV_free(itav);
					return 0;
				}
				if(!TEST_int_eq(ossl_cmp_hdr_generalInfo_push1_items(fixture->hdr, itavs), 1))
					goto err;
				ginfo = fixture->hdr->generalInfo;
				pushed_itav = sk_OSSL_CMP_ITAV_value(ginfo, 0);
				OBJ_obj2txt(buf, sizeof(buf), OSSL_CMP_ITAV_get0_type(pushed_itav), 0);
				if(!TEST_int_eq(memcmp(oid, buf, sizeof(oid)), 0))
					goto err;
				pushed_val = OSSL_CMP_ITAV_get0_value(sk_OSSL_CMP_ITAV_value(ginfo, 0));
				if(!TEST_int_eq(ASN1_TYPE_cmp(itav->infoValue.other, pushed_val), 0))
					goto err;
				res = 1;
			err:
				sk_OSSL_CMP_ITAV_pop_free(itavs, OSSL_CMP_ITAV_free);
				return res;
			}
			static int test_HDR_generalInfo_push1_items()
			{
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				EXECUTE_TEST(execute_HDR_generalInfo_push1_items_test, tear_down);
				return result;
			}
			static int execute_HDR_set_and_check_implicitConfirm_test(CMP_HDR_TEST_FIXTURE * fixture)
			{
				return TEST_false(ossl_cmp_hdr_has_implicitConfirm(fixture->hdr)) && TEST_true(ossl_cmp_hdr_set_implicitConfirm(fixture->hdr))
					   && TEST_true(ossl_cmp_hdr_has_implicitConfirm(fixture->hdr));
			}
			static int test_HDR_set_and_check_implicit_confirm()
			{
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				EXECUTE_TEST(execute_HDR_set_and_check_implicitConfirm_test, tear_down);
				return result;
			}
			static int execute_HDR_init_test(CMP_HDR_TEST_FIXTURE * fixture)
			{
				ASN1_OCTET_STRING * header_nonce, * header_transactionID;
				ASN1_OCTET_STRING * ctx_nonce;
				if(!TEST_int_eq(fixture->expected, ossl_cmp_hdr_init(fixture->cmp_ctx, fixture->hdr)))
					return 0;
				if(fixture->expected == 0)
					return 1;
				if(!TEST_int_eq(ossl_cmp_hdr_get_pvno(fixture->hdr), OSSL_CMP_PVNO))
					return 0;
				header_nonce = ossl_cmp_hdr_get0_senderNonce(fixture->hdr);
				if(!TEST_int_eq(0, ASN1_OCTET_STRING_cmp(header_nonce, fixture->cmp_ctx->senderNonce)))
					return 0;
				header_transactionID = OSSL_CMP_HDR_get0_transactionID(fixture->hdr);
				if(!TEST_true(0 == ASN1_OCTET_STRING_cmp(header_transactionID, fixture->cmp_ctx->transactionID)))
					return 0;
				header_nonce = OSSL_CMP_HDR_get0_recipNonce(fixture->hdr);
				ctx_nonce = fixture->cmp_ctx->recipNonce;
				if(ctx_nonce && (!TEST_ptr(header_nonce) || !TEST_int_eq(0, ASN1_OCTET_STRING_cmp(header_nonce, ctx_nonce))))
					return 0;
				return 1;
			}
			static int test_HDR_init_with_ref()
			{
				uchar ref[CMP_TEST_REFVALUE_LENGTH];
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				if(!TEST_int_eq(1, RAND_bytes(ref, sizeof(ref))) || !TEST_true(OSSL_CMP_CTX_set1_referenceValue(fixture->cmp_ctx, ref, sizeof(ref)))) {
					tear_down(fixture);
					fixture = NULL;
				}
				EXECUTE_TEST(execute_HDR_init_test, tear_down);
				return result;
			}
			static int test_HDR_init_with_subject()
			{
				X509_NAME * subject = NULL;
				SETUP_TEST_FIXTURE(CMP_HDR_TEST_FIXTURE, set_up);
				fixture->expected = 1;
				if(!TEST_ptr(subject = X509_NAME_new()) || !TEST_true(X509_NAME_ADD(subject, "CN", "Common Name"))
					|| !TEST_true(OSSL_CMP_CTX_set1_subjectName(fixture->cmp_ctx, subject))) {
					tear_down(fixture);
					fixture = NULL;
				}
				X509_NAME_free(subject);
				EXECUTE_TEST(execute_HDR_init_test, tear_down);
				return result;
			}
		};
		#undef X509_NAME_ADD

		RAND_bytes(TestData_CmpHdr_rand_data, OSSL_CMP_TRANSACTIONID_LENGTH);
		// Message header tests 
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_set_get_pvno);
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_get0_senderNonce);
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_set1_sender);
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_set1_recipient);
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_update_messageTime);
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_set1_senderKID);
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_push0_freeText);
		// indirectly tests ossl_cmp_pkifreetext_push_str():
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_push1_freeText);
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_generalInfo_push0_item);
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_generalInfo_push1_items);
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_set_and_check_implicit_confirm);
		// also tests public function OSSL_CMP_HDR_get0_transactionID(): 
		// also tests public function OSSL_CMP_HDR_get0_recipNonce():
		// also tests internal function ossl_cmp_hdr_get_pvno():
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_init_with_ref);
		ADD_TEST(TestInnerBlock_CmpHdr::test_HDR_init_with_subject);
	}
	{
		#define TEST(expected, test) test_case((expected), #test, (test))

		static const char * bn_output_tests[] = { NULL, "0", "-12345678",
			"12345678901234567890123456789012345678901234567890121234567890123456789012345678901234567890123456789013987657"
		};

		class TestInnerBlock_Test {
		public:
			static int test_case(int expected, const char * test, int result)
			{
				if(result != expected) {
					slfprintf_stderr("# FATAL: %s != %d\n", test, expected);
					return 0;
				}
				return 1;
			}
			static int test_int()
			{
				if(!TEST(1, TEST_int_eq(1, 1))
					| !TEST(0, TEST_int_eq(1, -1))
					| !TEST(1, TEST_int_ne(1, 2))
					| !TEST(0, TEST_int_ne(3, 3))
					| !TEST(1, TEST_int_lt(4, 9))
					| !TEST(0, TEST_int_lt(9, 4))
					| !TEST(1, TEST_int_le(4, 9))
					| !TEST(1, TEST_int_le(5, 5))
					| !TEST(0, TEST_int_le(9, 4))
					| !TEST(1, TEST_int_gt(8, 5))
					| !TEST(0, TEST_int_gt(5, 8))
					| !TEST(1, TEST_int_ge(8, 5))
					| !TEST(1, TEST_int_ge(6, 6))
					| !TEST(0, TEST_int_ge(5, 8)))
					goto err;
				return 1;
			err:
				return 0;
			}
			static int test_uint()
			{
				if(!TEST(1, TEST_uint_eq(3u, 3u))
					| !TEST(0, TEST_uint_eq(3u, 5u))
					| !TEST(1, TEST_uint_ne(4u, 2u))
					| !TEST(0, TEST_uint_ne(6u, 6u))
					| !TEST(1, TEST_uint_lt(5u, 9u))
					| !TEST(0, TEST_uint_lt(9u, 5u))
					| !TEST(1, TEST_uint_le(5u, 9u))
					| !TEST(1, TEST_uint_le(7u, 7u))
					| !TEST(0, TEST_uint_le(9u, 5u))
					| !TEST(1, TEST_uint_gt(11u, 1u))
					| !TEST(0, TEST_uint_gt(1u, 11u))
					| !TEST(1, TEST_uint_ge(11u, 1u))
					| !TEST(1, TEST_uint_ge(6u, 6u))
					| !TEST(0, TEST_uint_ge(1u, 11u)))
					goto err;
				return 1;
			err:
				return 0;
			}
			static int test_char()
			{
				if(!TEST(1, TEST_char_eq('a', 'a'))
					| !TEST(0, TEST_char_eq('a', 'A'))
					| !TEST(1, TEST_char_ne('a', 'c'))
					| !TEST(0, TEST_char_ne('e', 'e'))
					| !TEST(1, TEST_char_lt('i', 'x'))
					| !TEST(0, TEST_char_lt('x', 'i'))
					| !TEST(1, TEST_char_le('i', 'x'))
					| !TEST(1, TEST_char_le('n', 'n'))
					| !TEST(0, TEST_char_le('x', 'i'))
					| !TEST(1, TEST_char_gt('w', 'n'))
					| !TEST(0, TEST_char_gt('n', 'w'))
					| !TEST(1, TEST_char_ge('w', 'n'))
					| !TEST(1, TEST_char_ge('p', 'p'))
					| !TEST(0, TEST_char_ge('n', 'w')))
					goto err;
				return 1;
			err:
				return 0;
			}
			static int test_uchar()
			{
				if(!TEST(1, TEST_uchar_eq(49, 49))
					| !TEST(0, TEST_uchar_eq(49, 60))
					| !TEST(1, TEST_uchar_ne(50, 2))
					| !TEST(0, TEST_uchar_ne(66, 66))
					| !TEST(1, TEST_uchar_lt(60, 80))
					| !TEST(0, TEST_uchar_lt(80, 60))
					| !TEST(1, TEST_uchar_le(60, 80))
					| !TEST(1, TEST_uchar_le(78, 78))
					| !TEST(0, TEST_uchar_le(80, 60))
					| !TEST(1, TEST_uchar_gt(88, 37))
					| !TEST(0, TEST_uchar_gt(37, 88))
					| !TEST(1, TEST_uchar_ge(88, 37))
					| !TEST(1, TEST_uchar_ge(66, 66))
					| !TEST(0, TEST_uchar_ge(37, 88)))
					goto err;
				return 1;
			err:
				return 0;
			}
			static int test_long()
			{
				if(!TEST(1, TEST_long_eq(123l, 123l))
					| !TEST(0, TEST_long_eq(123l, -123l))
					| !TEST(1, TEST_long_ne(123l, 500l))
					| !TEST(0, TEST_long_ne(1000l, 1000l))
					| !TEST(1, TEST_long_lt(-8923l, 102934563l))
					| !TEST(0, TEST_long_lt(102934563l, -8923l))
					| !TEST(1, TEST_long_le(-8923l, 102934563l))
					| !TEST(1, TEST_long_le(12345l, 12345l))
					| !TEST(0, TEST_long_le(102934563l, -8923l))
					| !TEST(1, TEST_long_gt(84325677l, 12345l))
					| !TEST(0, TEST_long_gt(12345l, 84325677l))
					| !TEST(1, TEST_long_ge(84325677l, 12345l))
					| !TEST(1, TEST_long_ge(465869l, 465869l))
					| !TEST(0, TEST_long_ge(12345l, 84325677l)))
					goto err;
				return 1;
			err:
				return 0;
			}
			static int test_ulong()
			{
				if(!TEST(1, TEST_ulong_eq(919ul, 919ul))
					| !TEST(0, TEST_ulong_eq(919ul, 10234ul))
					| !TEST(1, TEST_ulong_ne(8190ul, 66ul))
					| !TEST(0, TEST_ulong_ne(10555ul, 10555ul))
					| !TEST(1, TEST_ulong_lt(10234ul, 1000000ul))
					| !TEST(0, TEST_ulong_lt(1000000ul, 10234ul))
					| !TEST(1, TEST_ulong_le(10234ul, 1000000ul))
					| !TEST(1, TEST_ulong_le(100000ul, 100000ul))
					| !TEST(0, TEST_ulong_le(1000000ul, 10234ul))
					| !TEST(1, TEST_ulong_gt(100000000ul, 22ul))
					| !TEST(0, TEST_ulong_gt(22ul, 100000000ul))
					| !TEST(1, TEST_ulong_ge(100000000ul, 22ul))
					| !TEST(1, TEST_ulong_ge(10555ul, 10555ul))
					| !TEST(0, TEST_ulong_ge(22ul, 100000000ul)))
					goto err;
				return 1;
			err:
				return 0;
			}
			static int test_size_t()
			{
				if(!TEST(1, TEST_size_t_eq((size_t)10, (size_t)10))
					| !TEST(0, TEST_size_t_eq((size_t)10, (size_t)12))
					| !TEST(1, TEST_size_t_ne((size_t)10, (size_t)12))
					| !TEST(0, TEST_size_t_ne((size_t)24, (size_t)24))
					| !TEST(1, TEST_size_t_lt((size_t)30, (size_t)88))
					| !TEST(0, TEST_size_t_lt((size_t)88, (size_t)30))
					| !TEST(1, TEST_size_t_le((size_t)30, (size_t)88))
					| !TEST(1, TEST_size_t_le((size_t)33, (size_t)33))
					| !TEST(0, TEST_size_t_le((size_t)88, (size_t)30))
					| !TEST(1, TEST_size_t_gt((size_t)52, (size_t)33))
					| !TEST(0, TEST_size_t_gt((size_t)33, (size_t)52))
					| !TEST(1, TEST_size_t_ge((size_t)52, (size_t)33))
					| !TEST(1, TEST_size_t_ge((size_t)38, (size_t)38))
					| !TEST(0, TEST_size_t_ge((size_t)33, (size_t)52)))
					goto err;
				return 1;
			err:
				return 0;
			}
			static int test_time_t()
			{
				if(!TEST(1, TEST_time_t_eq((time_t)10, (time_t)10))
					| !TEST(0, TEST_time_t_eq((time_t)10, (time_t)12))
					| !TEST(1, TEST_time_t_ne((time_t)10, (time_t)12))
					| !TEST(0, TEST_time_t_ne((time_t)24, (time_t)24))
					| !TEST(1, TEST_time_t_lt((time_t)30, (time_t)88))
					| !TEST(0, TEST_time_t_lt((time_t)88, (time_t)30))
					| !TEST(1, TEST_time_t_le((time_t)30, (time_t)88))
					| !TEST(1, TEST_time_t_le((time_t)33, (time_t)33))
					| !TEST(0, TEST_time_t_le((time_t)88, (time_t)30))
					| !TEST(1, TEST_time_t_gt((time_t)52, (time_t)33))
					| !TEST(0, TEST_time_t_gt((time_t)33, (time_t)52))
					| !TEST(1, TEST_time_t_ge((time_t)52, (time_t)33))
					| !TEST(1, TEST_time_t_ge((time_t)38, (time_t)38))
					| !TEST(0, TEST_time_t_ge((time_t)33, (time_t)52)))
					goto err;
				return 1;
			err:
				return 0;
			}
			static int test_pointer()
			{
				int x = 0;
				char y = 1;
				if(!TEST(1, TEST_ptr(&y))
					| !TEST(0, TEST_ptr(NULL))
					| !TEST(0, TEST_ptr_null(&y))
					| !TEST(1, TEST_ptr_null(NULL))
					| !TEST(1, TEST_ptr_eq(NULL, NULL))
					| !TEST(0, TEST_ptr_eq(NULL, &y))
					| !TEST(0, TEST_ptr_eq(&y, NULL))
					| !TEST(0, TEST_ptr_eq(&y, &x))
					| !TEST(1, TEST_ptr_eq(&x, &x))
					| !TEST(0, TEST_ptr_ne(NULL, NULL))
					| !TEST(1, TEST_ptr_ne(NULL, &y))
					| !TEST(1, TEST_ptr_ne(&y, NULL))
					| !TEST(1, TEST_ptr_ne(&y, &x))
					| !TEST(0, TEST_ptr_ne(&x, &x)))
					goto err;
				return 1;
			err:
				return 0;
			}
			static int test_bool()
			{
				if(!TEST(0, TEST_true(0)) | !TEST(1, TEST_true(1)) | !TEST(1, TEST_false(0)) | !TEST(0, TEST_false(1)))
					goto err;
				return 1;
			err:
				return 0;
			}
			static int test_string()
			{
				static char buf[] = "abc";
				if(!TEST(1, TEST_str_eq(NULL, NULL))
					| !TEST(1, TEST_str_eq("abc", buf))
					| !TEST(0, TEST_str_eq("abc", NULL))
					| !TEST(0, TEST_str_eq("abc", ""))
					| !TEST(0, TEST_str_eq(NULL, buf))
					| !TEST(0, TEST_str_ne(NULL, NULL))
					| !TEST(0, TEST_str_eq("", NULL))
					| !TEST(0, TEST_str_eq(NULL, ""))
					| !TEST(0, TEST_str_ne("", ""))
					| !TEST(0, TEST_str_eq("\1\2\3\4\5", "\1x\3\6\5"))
					| !TEST(0, TEST_str_ne("abc", buf))
					| !TEST(1, TEST_str_ne("abc", NULL))
					| !TEST(1, TEST_str_ne(NULL, buf))
					| !TEST(0, TEST_str_eq("abcdef", "abcdefghijk")))
					goto err;
				return 1;
			err:
				return 0;
			}
			static int test_memory()
			{
				static char buf[] = "xyz";
				if(!TEST(1, TEST_mem_eq(NULL, 0, NULL, 0))
					| !TEST(1, TEST_mem_eq(NULL, 1, NULL, 2))
					| !TEST(0, TEST_mem_eq(NULL, 0, "xyz", 3))
					| !TEST(0, TEST_mem_eq(NULL, 7, "abc", 3))
					| !TEST(0, TEST_mem_ne(NULL, 0, NULL, 0))
					| !TEST(0, TEST_mem_eq(NULL, 0, "", 0))
					| !TEST(0, TEST_mem_eq("", 0, NULL, 0))
					| !TEST(0, TEST_mem_ne("", 0, "", 0))
					| !TEST(0, TEST_mem_eq("xyz", 3, NULL, 0))
					| !TEST(0, TEST_mem_eq("xyz", 3, buf, sizeof(buf)))
					| !TEST(1, TEST_mem_eq("xyz", 4, buf, sizeof(buf))))
					goto err;
				return 1;
			err:
				return 0;
			}
			static int test_memory_overflow()
			{
				/* Verify that the memory printing overflows without walking the stack */
				const char * p = "1234567890123456789012345678901234567890123456789012";
				const char * q = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
				return TEST(0, TEST_mem_eq(p, strlen(p), q, strlen(q)));
			}
			static int test_bignum()
			{
				BIGNUM * a = NULL, * b = NULL, * c = NULL;
				int r = 0;
				if(!TEST(1, TEST_int_eq(BN_dec2bn(&a, "0"), 1))
					| !TEST(1, TEST_BN_eq_word(a, 0))
					| !TEST(0, TEST_BN_eq_word(a, 30))
					| !TEST(1, TEST_BN_abs_eq_word(a, 0))
					| !TEST(0, TEST_BN_eq_one(a))
					| !TEST(1, TEST_BN_eq_zero(a))
					| !TEST(0, TEST_BN_ne_zero(a))
					| !TEST(1, TEST_BN_le_zero(a))
					| !TEST(0, TEST_BN_lt_zero(a))
					| !TEST(1, TEST_BN_ge_zero(a))
					| !TEST(0, TEST_BN_gt_zero(a))
					| !TEST(1, TEST_BN_even(a))
					| !TEST(0, TEST_BN_odd(a))
					| !TEST(1, TEST_BN_eq(b, c))
					| !TEST(0, TEST_BN_eq(a, b))
					| !TEST(0, TEST_BN_ne(NULL, c))
					| !TEST(1, TEST_int_eq(BN_dec2bn(&b, "1"), 1))
					| !TEST(1, TEST_BN_eq_word(b, 1))
					| !TEST(1, TEST_BN_eq_one(b))
					| !TEST(0, TEST_BN_abs_eq_word(b, 0))
					| !TEST(1, TEST_BN_abs_eq_word(b, 1))
					| !TEST(0, TEST_BN_eq_zero(b))
					| !TEST(1, TEST_BN_ne_zero(b))
					| !TEST(0, TEST_BN_le_zero(b))
					| !TEST(0, TEST_BN_lt_zero(b))
					| !TEST(1, TEST_BN_ge_zero(b))
					| !TEST(1, TEST_BN_gt_zero(b))
					| !TEST(0, TEST_BN_even(b))
					| !TEST(1, TEST_BN_odd(b))
					| !TEST(1, TEST_int_eq(BN_dec2bn(&c, "-334739439"), 10))
					| !TEST(0, TEST_BN_eq_word(c, 334739439))
					| !TEST(1, TEST_BN_abs_eq_word(c, 334739439))
					| !TEST(0, TEST_BN_eq_zero(c))
					| !TEST(1, TEST_BN_ne_zero(c))
					| !TEST(1, TEST_BN_le_zero(c))
					| !TEST(1, TEST_BN_lt_zero(c))
					| !TEST(0, TEST_BN_ge_zero(c))
					| !TEST(0, TEST_BN_gt_zero(c))
					| !TEST(0, TEST_BN_even(c))
					| !TEST(1, TEST_BN_odd(c))
					| !TEST(1, TEST_BN_eq(a, a))
					| !TEST(0, TEST_BN_ne(a, a))
					| !TEST(0, TEST_BN_eq(a, b))
					| !TEST(1, TEST_BN_ne(a, b))
					| !TEST(0, TEST_BN_lt(a, c))
					| !TEST(1, TEST_BN_lt(c, b))
					| !TEST(0, TEST_BN_lt(b, c))
					| !TEST(0, TEST_BN_le(a, c))
					| !TEST(1, TEST_BN_le(c, b))
					| !TEST(0, TEST_BN_le(b, c))
					| !TEST(1, TEST_BN_gt(a, c))
					| !TEST(0, TEST_BN_gt(c, b))
					| !TEST(1, TEST_BN_gt(b, c))
					| !TEST(1, TEST_BN_ge(a, c))
					| !TEST(0, TEST_BN_ge(c, b))
					| !TEST(1, TEST_BN_ge(b, c)))
					goto err;
				r = 1;
			err:
				BN_free(a);
				BN_free(b);
				BN_free(c);
				return r;
			}
			static int test_long_output()
			{
				const char * p = "1234567890123456789012345678901234567890123456789012";
				const char * q = "1234567890klmnopqrs01234567890EFGHIJKLM0123456789XYZ";
				const char * r = "1234567890123456789012345678901234567890123456789012"
					"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXY+"
					"12345678901234567890123ABC78901234567890123456789012";
				const char * s = "1234567890123456789012345678901234567890123456789012"
					"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXY-"
					"1234567890123456789012345678901234567890123456789012"
					"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
				return TEST(0, TEST_str_eq(p,  q)) & TEST(0, TEST_str_eq(q, r)) & TEST(0, TEST_str_eq(r, s)) & TEST(0, TEST_mem_eq(r, strlen(r), s, strlen(s)));
			}
			static int test_long_bignum()
			{
				int r;
				BIGNUM * a = NULL, * b = NULL, * c = NULL, * d = NULL;
				const char as[] = "1234567890123456789012345678901234567890123456789012"
					"1234567890123456789012345678901234567890123456789012"
					"1234567890123456789012345678901234567890123456789012"
					"1234567890123456789012345678901234567890123456789012"
					"1234567890123456789012345678901234567890123456789012"
					"1234567890123456789012345678901234567890123456789012"
					"FFFFFF";
				const char bs[] = "1234567890123456789012345678901234567890123456789012"
					"1234567890123456789012345678901234567890123456789013"
					"987657";
				const char cs[] = "-"    /* 64 characters plus sign */
					"123456789012345678901234567890"
					"123456789012345678901234567890"
					"ABCD";
				const char ds[] = "-"    /* 63 characters plus sign */
					"23456789A123456789B123456789C"
					"123456789D123456789E123456789F"
					"ABCD";
				r = TEST_true(BN_hex2bn(&a, as)) && TEST_true(BN_hex2bn(&b, bs)) && TEST_true(BN_hex2bn(&c, cs)) && 
					TEST_true(BN_hex2bn(&d, ds)) && (TEST(0, TEST_BN_eq(a, b)) & TEST(0, TEST_BN_eq(b, a)) & TEST(0, TEST_BN_eq(b, NULL)) & 
					TEST(0, TEST_BN_eq(NULL, a)) & TEST(1, TEST_BN_ne(a, NULL)) & TEST(0, TEST_BN_eq(c, d)));
				BN_free(a);
				BN_free(b);
				BN_free(c);
				BN_free(d);
				return r;
			}
			static int test_messages()
			{
				TEST_info("This is an %s message.", "info");
				TEST_error("This is an %s message.", "error");
				return 1;
			}
			static int test_single_eval()
			{
				int i = 4;
				long l = -9000;
				char c = 'd';
				unsigned char uc = 22;
				unsigned long ul = 500;
				size_t st = 1234;
				char buf[4] = { 0 };
				const char * p = buf;
				/* int */
				return TEST_int_eq(i++, 4)
					   && TEST_int_eq(i, 5)
					   && TEST_int_gt(++i, 5)
					   && TEST_int_le(5, i++)
					   && TEST_int_ne(--i, 5)
					   && TEST_int_eq(12, i *= 2)
					   /* Long */
					   && TEST_long_eq(l--, -9000L)
					   && TEST_long_eq(++l, -9000L)
					   && TEST_long_ne(-9000L, l /= 2)
					   && TEST_long_lt(--l, -4500L)
					   /* char */
					   && TEST_char_eq(++c, 'e')
					   && TEST_char_eq('e', c--)
					   && TEST_char_ne('d', --c)
					   && TEST_char_le('b', --c)
					   && TEST_char_lt(c++, 'c')
					   /* unsigned char */
					   && TEST_uchar_eq(22, uc++)
					   && TEST_uchar_eq(uc /= 2, 11)
					   && TEST_ulong_eq(ul ^= 1, 501)
					   && TEST_ulong_eq(502, ul ^= 3)
					   && TEST_ulong_eq(ul = ul * 3 - 6, 1500)
					   /* size_t */
					   && TEST_size_t_eq((--i, st++), 1234)
					   && TEST_size_t_eq(st, 1235)
					   && TEST_int_eq(11, i)
					   /* pointers */
					   && TEST_ptr_eq(p++, buf)
					   && TEST_ptr_eq(buf + 2, ++p)
					   && TEST_ptr_eq(buf, p -= 2)
					   && TEST_ptr(++p)
					   && TEST_ptr_eq(p, buf + 1)
					   && TEST_ptr_null(p = NULL)
					   /* strings */
					   && TEST_str_eq(p = &("123456"[1]), "23456")
					   && TEST_str_eq("3456", ++p)
					   && TEST_str_ne(p++, "456")
					   /* memory */
					   && TEST_mem_eq(--p, sizeof("3456"), "3456", sizeof("3456"))
					   && TEST_mem_ne(p++, sizeof("456"), "456", sizeof("456"))
					   && TEST_mem_eq(p--, sizeof("456"), "456", sizeof("456"));
			}
			static int test_output()
			{
				const char s[] = "1234567890123456789012345678901234567890123456789012abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
				test_output_string("test", s, sizeof(s) - 1);
				test_output_memory("test", (const unsigned char*)s, sizeof(s));
				return 1;
			}
			static int test_bn_output(int n)
			{
				BIGNUM * b = NULL;
				if(bn_output_tests[n] != NULL && !TEST_true(BN_hex2bn(&b, bn_output_tests[n])))
					return 0;
				test_output_bignum(bn_output_tests[n], b);
				BN_free(b);
				return 1;
			}
			static int test_skip_one() { return TEST_skip("skip test"); }
			static int test_skip_many(int n) { return TEST_skip("skip tests: %d", n); }
			static int test_skip_null()
			{
				// This is not a recommended way of skipping a test, a reason or description should be included.
				return TEST_skip(NULL);
			}
		};
		#undef TEST

		ADD_TEST(TestInnerBlock_Test::test_int);
		ADD_TEST(TestInnerBlock_Test::test_uint);
		ADD_TEST(TestInnerBlock_Test::test_char);
		ADD_TEST(TestInnerBlock_Test::test_uchar);
		ADD_TEST(TestInnerBlock_Test::test_long);
		ADD_TEST(TestInnerBlock_Test::test_ulong);
		ADD_TEST(TestInnerBlock_Test::test_size_t);
		ADD_TEST(TestInnerBlock_Test::test_time_t);
		ADD_TEST(TestInnerBlock_Test::test_pointer);
		ADD_TEST(TestInnerBlock_Test::test_bool);
		ADD_TEST(TestInnerBlock_Test::test_string);
		ADD_TEST(TestInnerBlock_Test::test_memory);
		ADD_TEST(TestInnerBlock_Test::test_memory_overflow);
		ADD_TEST(TestInnerBlock_Test::test_bignum);
		ADD_TEST(TestInnerBlock_Test::test_long_bignum);
		ADD_TEST(TestInnerBlock_Test::test_long_output);
		ADD_TEST(TestInnerBlock_Test::test_messages);
		ADD_TEST(TestInnerBlock_Test::test_single_eval);
		ADD_TEST(TestInnerBlock_Test::test_output);
		ADD_ALL_TESTS(TestInnerBlock_Test::test_bn_output, SIZEOFARRAY(bn_output_tests));
		ADD_TEST(TestInnerBlock_Test::test_skip_one);
		ADD_TEST(TestInnerBlock_Test::test_skip_null);
		ADD_ALL_TESTS(TestInnerBlock_Test::test_skip_many, 3);
	}
	{
		// 
		// DH low level APIs are deprecated for public use, but still ok for internal use.
		// 
		#ifndef OPENSSL_NO_DH
			//
			// Test data from RFC 5114 
			//
			static const uchar dhtest_1024_160_xA[] = {
				0xB9, 0xA3, 0xB3, 0xAE, 0x8F, 0xEF, 0xC1, 0xA2, 0x93, 0x04, 0x96, 0x50,
				0x70, 0x86, 0xF8, 0x45, 0x5D, 0x48, 0x94, 0x3E
			};
			static const uchar dhtest_1024_160_yA[] = {
				0x2A, 0x85, 0x3B, 0x3D, 0x92, 0x19, 0x75, 0x01, 0xB9, 0x01, 0x5B, 0x2D,
				0xEB, 0x3E, 0xD8, 0x4F, 0x5E, 0x02, 0x1D, 0xCC, 0x3E, 0x52, 0xF1, 0x09,
				0xD3, 0x27, 0x3D, 0x2B, 0x75, 0x21, 0x28, 0x1C, 0xBA, 0xBE, 0x0E, 0x76,
				0xFF, 0x57, 0x27, 0xFA, 0x8A, 0xCC, 0xE2, 0x69, 0x56, 0xBA, 0x9A, 0x1F,
				0xCA, 0x26, 0xF2, 0x02, 0x28, 0xD8, 0x69, 0x3F, 0xEB, 0x10, 0x84, 0x1D,
				0x84, 0xA7, 0x36, 0x00, 0x54, 0xEC, 0xE5, 0xA7, 0xF5, 0xB7, 0xA6, 0x1A,
				0xD3, 0xDF, 0xB3, 0xC6, 0x0D, 0x2E, 0x43, 0x10, 0x6D, 0x87, 0x27, 0xDA,
				0x37, 0xDF, 0x9C, 0xCE, 0x95, 0xB4, 0x78, 0x75, 0x5D, 0x06, 0xBC, 0xEA,
				0x8F, 0x9D, 0x45, 0x96, 0x5F, 0x75, 0xA5, 0xF3, 0xD1, 0xDF, 0x37, 0x01,
				0x16, 0x5F, 0xC9, 0xE5, 0x0C, 0x42, 0x79, 0xCE, 0xB0, 0x7F, 0x98, 0x95,
				0x40, 0xAE, 0x96, 0xD5, 0xD8, 0x8E, 0xD7, 0x76
			};
			static const uchar dhtest_1024_160_xB[] = {
				0x93, 0x92, 0xC9, 0xF9, 0xEB, 0x6A, 0x7A, 0x6A, 0x90, 0x22, 0xF7, 0xD8,
				0x3E, 0x72, 0x23, 0xC6, 0x83, 0x5B, 0xBD, 0xDA
			};
			static const uchar dhtest_1024_160_yB[] = {
				0x71, 0x7A, 0x6C, 0xB0, 0x53, 0x37, 0x1F, 0xF4, 0xA3, 0xB9, 0x32, 0x94,
				0x1C, 0x1E, 0x56, 0x63, 0xF8, 0x61, 0xA1, 0xD6, 0xAD, 0x34, 0xAE, 0x66,
				0x57, 0x6D, 0xFB, 0x98, 0xF6, 0xC6, 0xCB, 0xF9, 0xDD, 0xD5, 0xA5, 0x6C,
				0x78, 0x33, 0xF6, 0xBC, 0xFD, 0xFF, 0x09, 0x55, 0x82, 0xAD, 0x86, 0x8E,
				0x44, 0x0E, 0x8D, 0x09, 0xFD, 0x76, 0x9E, 0x3C, 0xEC, 0xCD, 0xC3, 0xD3,
				0xB1, 0xE4, 0xCF, 0xA0, 0x57, 0x77, 0x6C, 0xAA, 0xF9, 0x73, 0x9B, 0x6A,
				0x9F, 0xEE, 0x8E, 0x74, 0x11, 0xF8, 0xD6, 0xDA, 0xC0, 0x9D, 0x6A, 0x4E,
				0xDB, 0x46, 0xCC, 0x2B, 0x5D, 0x52, 0x03, 0x09, 0x0E, 0xAE, 0x61, 0x26,
				0x31, 0x1E, 0x53, 0xFD, 0x2C, 0x14, 0xB5, 0x74, 0xE6, 0xA3, 0x10, 0x9A,
				0x3D, 0xA1, 0xBE, 0x41, 0xBD, 0xCE, 0xAA, 0x18, 0x6F, 0x5C, 0xE0, 0x67,
				0x16, 0xA2, 0xB6, 0xA0, 0x7B, 0x3C, 0x33, 0xFE
			};
			static const uchar dhtest_1024_160_Z[] = {
				0x5C, 0x80, 0x4F, 0x45, 0x4D, 0x30, 0xD9, 0xC4, 0xDF, 0x85, 0x27, 0x1F,
				0x93, 0x52, 0x8C, 0x91, 0xDF, 0x6B, 0x48, 0xAB, 0x5F, 0x80, 0xB3, 0xB5,
				0x9C, 0xAA, 0xC1, 0xB2, 0x8F, 0x8A, 0xCB, 0xA9, 0xCD, 0x3E, 0x39, 0xF3,
				0xCB, 0x61, 0x45, 0x25, 0xD9, 0x52, 0x1D, 0x2E, 0x64, 0x4C, 0x53, 0xB8,
				0x07, 0xB8, 0x10, 0xF3, 0x40, 0x06, 0x2F, 0x25, 0x7D, 0x7D, 0x6F, 0xBF,
				0xE8, 0xD5, 0xE8, 0xF0, 0x72, 0xE9, 0xB6, 0xE9, 0xAF, 0xDA, 0x94, 0x13,
				0xEA, 0xFB, 0x2E, 0x8B, 0x06, 0x99, 0xB1, 0xFB, 0x5A, 0x0C, 0xAC, 0xED,
				0xDE, 0xAE, 0xAD, 0x7E, 0x9C, 0xFB, 0xB3, 0x6A, 0xE2, 0xB4, 0x20, 0x83,
				0x5B, 0xD8, 0x3A, 0x19, 0xFB, 0x0B, 0x5E, 0x96, 0xBF, 0x8F, 0xA4, 0xD0,
				0x9E, 0x34, 0x55, 0x25, 0x16, 0x7E, 0xCD, 0x91, 0x55, 0x41, 0x6F, 0x46,
				0xF4, 0x08, 0xED, 0x31, 0xB6, 0x3C, 0x6E, 0x6D
			};
			static const uchar dhtest_2048_224_xA[] = {
				0x22, 0xE6, 0x26, 0x01, 0xDB, 0xFF, 0xD0, 0x67, 0x08, 0xA6, 0x80, 0xF7,
				0x47, 0xF3, 0x61, 0xF7, 0x6D, 0x8F, 0x4F, 0x72, 0x1A, 0x05, 0x48, 0xE4,
				0x83, 0x29, 0x4B, 0x0C
			};
			static const uchar dhtest_2048_224_yA[] = {
				0x1B, 0x3A, 0x63, 0x45, 0x1B, 0xD8, 0x86, 0xE6, 0x99, 0xE6, 0x7B, 0x49,
				0x4E, 0x28, 0x8B, 0xD7, 0xF8, 0xE0, 0xD3, 0x70, 0xBA, 0xDD, 0xA7, 0xA0,
				0xEF, 0xD2, 0xFD, 0xE7, 0xD8, 0xF6, 0x61, 0x45, 0xCC, 0x9F, 0x28, 0x04,
				0x19, 0x97, 0x5E, 0xB8, 0x08, 0x87, 0x7C, 0x8A, 0x4C, 0x0C, 0x8E, 0x0B,
				0xD4, 0x8D, 0x4A, 0x54, 0x01, 0xEB, 0x1E, 0x87, 0x76, 0xBF, 0xEE, 0xE1,
				0x34, 0xC0, 0x38, 0x31, 0xAC, 0x27, 0x3C, 0xD9, 0xD6, 0x35, 0xAB, 0x0C,
				0xE0, 0x06, 0xA4, 0x2A, 0x88, 0x7E, 0x3F, 0x52, 0xFB, 0x87, 0x66, 0xB6,
				0x50, 0xF3, 0x80, 0x78, 0xBC, 0x8E, 0xE8, 0x58, 0x0C, 0xEF, 0xE2, 0x43,
				0x96, 0x8C, 0xFC, 0x4F, 0x8D, 0xC3, 0xDB, 0x08, 0x45, 0x54, 0x17, 0x1D,
				0x41, 0xBF, 0x2E, 0x86, 0x1B, 0x7B, 0xB4, 0xD6, 0x9D, 0xD0, 0xE0, 0x1E,
				0xA3, 0x87, 0xCB, 0xAA, 0x5C, 0xA6, 0x72, 0xAF, 0xCB, 0xE8, 0xBD, 0xB9,
				0xD6, 0x2D, 0x4C, 0xE1, 0x5F, 0x17, 0xDD, 0x36, 0xF9, 0x1E, 0xD1, 0xEE,
				0xDD, 0x65, 0xCA, 0x4A, 0x06, 0x45, 0x5C, 0xB9, 0x4C, 0xD4, 0x0A, 0x52,
				0xEC, 0x36, 0x0E, 0x84, 0xB3, 0xC9, 0x26, 0xE2, 0x2C, 0x43, 0x80, 0xA3,
				0xBF, 0x30, 0x9D, 0x56, 0x84, 0x97, 0x68, 0xB7, 0xF5, 0x2C, 0xFD, 0xF6,
				0x55, 0xFD, 0x05, 0x3A, 0x7E, 0xF7, 0x06, 0x97, 0x9E, 0x7E, 0x58, 0x06,
				0xB1, 0x7D, 0xFA, 0xE5, 0x3A, 0xD2, 0xA5, 0xBC, 0x56, 0x8E, 0xBB, 0x52,
				0x9A, 0x7A, 0x61, 0xD6, 0x8D, 0x25, 0x6F, 0x8F, 0xC9, 0x7C, 0x07, 0x4A,
				0x86, 0x1D, 0x82, 0x7E, 0x2E, 0xBC, 0x8C, 0x61, 0x34, 0x55, 0x31, 0x15,
				0xB7, 0x0E, 0x71, 0x03, 0x92, 0x0A, 0xA1, 0x6D, 0x85, 0xE5, 0x2B, 0xCB,
				0xAB, 0x8D, 0x78, 0x6A, 0x68, 0x17, 0x8F, 0xA8, 0xFF, 0x7C, 0x2F, 0x5C,
				0x71, 0x64, 0x8D, 0x6F
			};
			static const uchar dhtest_2048_224_xB[] = {
				0x4F, 0xF3, 0xBC, 0x96, 0xC7, 0xFC, 0x6A, 0x6D, 0x71, 0xD3, 0xB3, 0x63,
				0x80, 0x0A, 0x7C, 0xDF, 0xEF, 0x6F, 0xC4, 0x1B, 0x44, 0x17, 0xEA, 0x15,
				0x35, 0x3B, 0x75, 0x90
			};
			static const uchar dhtest_2048_224_yB[] = {
				0x4D, 0xCE, 0xE9, 0x92, 0xA9, 0x76, 0x2A, 0x13, 0xF2, 0xF8, 0x38, 0x44,
				0xAD, 0x3D, 0x77, 0xEE, 0x0E, 0x31, 0xC9, 0x71, 0x8B, 0x3D, 0xB6, 0xC2,
				0x03, 0x5D, 0x39, 0x61, 0x18, 0x2C, 0x3E, 0x0B, 0xA2, 0x47, 0xEC, 0x41,
				0x82, 0xD7, 0x60, 0xCD, 0x48, 0xD9, 0x95, 0x99, 0x97, 0x06, 0x22, 0xA1,
				0x88, 0x1B, 0xBA, 0x2D, 0xC8, 0x22, 0x93, 0x9C, 0x78, 0xC3, 0x91, 0x2C,
				0x66, 0x61, 0xFA, 0x54, 0x38, 0xB2, 0x07, 0x66, 0x22, 0x2B, 0x75, 0xE2,
				0x4C, 0x2E, 0x3A, 0xD0, 0xC7, 0x28, 0x72, 0x36, 0x12, 0x95, 0x25, 0xEE,
				0x15, 0xB5, 0xDD, 0x79, 0x98, 0xAA, 0x04, 0xC4, 0xA9, 0x69, 0x6C, 0xAC,
				0xD7, 0x17, 0x20, 0x83, 0xA9, 0x7A, 0x81, 0x66, 0x4E, 0xAD, 0x2C, 0x47,
				0x9E, 0x44, 0x4E, 0x4C, 0x06, 0x54, 0xCC, 0x19, 0xE2, 0x8D, 0x77, 0x03,
				0xCE, 0xE8, 0xDA, 0xCD, 0x61, 0x26, 0xF5, 0xD6, 0x65, 0xEC, 0x52, 0xC6,
				0x72, 0x55, 0xDB, 0x92, 0x01, 0x4B, 0x03, 0x7E, 0xB6, 0x21, 0xA2, 0xAC,
				0x8E, 0x36, 0x5D, 0xE0, 0x71, 0xFF, 0xC1, 0x40, 0x0A, 0xCF, 0x07, 0x7A,
				0x12, 0x91, 0x3D, 0xD8, 0xDE, 0x89, 0x47, 0x34, 0x37, 0xAB, 0x7B, 0xA3,
				0x46, 0x74, 0x3C, 0x1B, 0x21, 0x5D, 0xD9, 0xC1, 0x21, 0x64, 0xA7, 0xE4,
				0x05, 0x31, 0x18, 0xD1, 0x99, 0xBE, 0xC8, 0xEF, 0x6F, 0xC5, 0x61, 0x17,
				0x0C, 0x84, 0xC8, 0x7D, 0x10, 0xEE, 0x9A, 0x67, 0x4A, 0x1F, 0xA8, 0xFF,
				0xE1, 0x3B, 0xDF, 0xBA, 0x1D, 0x44, 0xDE, 0x48, 0x94, 0x6D, 0x68, 0xDC,
				0x0C, 0xDD, 0x77, 0x76, 0x35, 0xA7, 0xAB, 0x5B, 0xFB, 0x1E, 0x4B, 0xB7,
				0xB8, 0x56, 0xF9, 0x68, 0x27, 0x73, 0x4C, 0x18, 0x41, 0x38, 0xE9, 0x15,
				0xD9, 0xC3, 0x00, 0x2E, 0xBC, 0xE5, 0x31, 0x20, 0x54, 0x6A, 0x7E, 0x20,
				0x02, 0x14, 0x2B, 0x6C
			};
			static const uchar dhtest_2048_224_Z[] = {
				0x34, 0xD9, 0xBD, 0xDC, 0x1B, 0x42, 0x17, 0x6C, 0x31, 0x3F, 0xEA, 0x03,
				0x4C, 0x21, 0x03, 0x4D, 0x07, 0x4A, 0x63, 0x13, 0xBB, 0x4E, 0xCD, 0xB3,
				0x70, 0x3F, 0xFF, 0x42, 0x45, 0x67, 0xA4, 0x6B, 0xDF, 0x75, 0x53, 0x0E,
				0xDE, 0x0A, 0x9D, 0xA5, 0x22, 0x9D, 0xE7, 0xD7, 0x67, 0x32, 0x28, 0x6C,
				0xBC, 0x0F, 0x91, 0xDA, 0x4C, 0x3C, 0x85, 0x2F, 0xC0, 0x99, 0xC6, 0x79,
				0x53, 0x1D, 0x94, 0xC7, 0x8A, 0xB0, 0x3D, 0x9D, 0xEC, 0xB0, 0xA4, 0xE4,
				0xCA, 0x8B, 0x2B, 0xB4, 0x59, 0x1C, 0x40, 0x21, 0xCF, 0x8C, 0xE3, 0xA2,
				0x0A, 0x54, 0x1D, 0x33, 0x99, 0x40, 0x17, 0xD0, 0x20, 0x0A, 0xE2, 0xC9,
				0x51, 0x6E, 0x2F, 0xF5, 0x14, 0x57, 0x79, 0x26, 0x9E, 0x86, 0x2B, 0x0F,
				0xB4, 0x74, 0xA2, 0xD5, 0x6D, 0xC3, 0x1E, 0xD5, 0x69, 0xA7, 0x70, 0x0B,
				0x4C, 0x4A, 0xB1, 0x6B, 0x22, 0xA4, 0x55, 0x13, 0x53, 0x1E, 0xF5, 0x23,
				0xD7, 0x12, 0x12, 0x07, 0x7B, 0x5A, 0x16, 0x9B, 0xDE, 0xFF, 0xAD, 0x7A,
				0xD9, 0x60, 0x82, 0x84, 0xC7, 0x79, 0x5B, 0x6D, 0x5A, 0x51, 0x83, 0xB8,
				0x70, 0x66, 0xDE, 0x17, 0xD8, 0xD6, 0x71, 0xC9, 0xEB, 0xD8, 0xEC, 0x89,
				0x54, 0x4D, 0x45, 0xEC, 0x06, 0x15, 0x93, 0xD4, 0x42, 0xC6, 0x2A, 0xB9,
				0xCE, 0x3B, 0x1C, 0xB9, 0x94, 0x3A, 0x1D, 0x23, 0xA5, 0xEA, 0x3B, 0xCF,
				0x21, 0xA0, 0x14, 0x71, 0xE6, 0x7E, 0x00, 0x3E, 0x7F, 0x8A, 0x69, 0xC7,
				0x28, 0xBE, 0x49, 0x0B, 0x2F, 0xC8, 0x8C, 0xFE, 0xB9, 0x2D, 0xB6, 0xA2,
				0x15, 0xE5, 0xD0, 0x3C, 0x17, 0xC4, 0x64, 0xC9, 0xAC, 0x1A, 0x46, 0xE2,
				0x03, 0xE1, 0x3F, 0x95, 0x29, 0x95, 0xFB, 0x03, 0xC6, 0x9D, 0x3C, 0xC4,
				0x7F, 0xCB, 0x51, 0x0B, 0x69, 0x98, 0xFF, 0xD3, 0xAA, 0x6D, 0xE7, 0x3C,
				0xF9, 0xF6, 0x38, 0x69
			};
			static const uchar dhtest_2048_256_xA[] = {
				0x08, 0x81, 0x38, 0x2C, 0xDB, 0x87, 0x66, 0x0C, 0x6D, 0xC1, 0x3E, 0x61,
				0x49, 0x38, 0xD5, 0xB9, 0xC8, 0xB2, 0xF2, 0x48, 0x58, 0x1C, 0xC5, 0xE3,
				0x1B, 0x35, 0x45, 0x43, 0x97, 0xFC, 0xE5, 0x0E
			};
			static const uchar dhtest_2048_256_yA[] = {
				0x2E, 0x93, 0x80, 0xC8, 0x32, 0x3A, 0xF9, 0x75, 0x45, 0xBC, 0x49, 0x41,
				0xDE, 0xB0, 0xEC, 0x37, 0x42, 0xC6, 0x2F, 0xE0, 0xEC, 0xE8, 0x24, 0xA6,
				0xAB, 0xDB, 0xE6, 0x6C, 0x59, 0xBE, 0xE0, 0x24, 0x29, 0x11, 0xBF, 0xB9,
				0x67, 0x23, 0x5C, 0xEB, 0xA3, 0x5A, 0xE1, 0x3E, 0x4E, 0xC7, 0x52, 0xBE,
				0x63, 0x0B, 0x92, 0xDC, 0x4B, 0xDE, 0x28, 0x47, 0xA9, 0xC6, 0x2C, 0xB8,
				0x15, 0x27, 0x45, 0x42, 0x1F, 0xB7, 0xEB, 0x60, 0xA6, 0x3C, 0x0F, 0xE9,
				0x15, 0x9F, 0xCC, 0xE7, 0x26, 0xCE, 0x7C, 0xD8, 0x52, 0x3D, 0x74, 0x50,
				0x66, 0x7E, 0xF8, 0x40, 0xE4, 0x91, 0x91, 0x21, 0xEB, 0x5F, 0x01, 0xC8,
				0xC9, 0xB0, 0xD3, 0xD6, 0x48, 0xA9, 0x3B, 0xFB, 0x75, 0x68, 0x9E, 0x82,
				0x44, 0xAC, 0x13, 0x4A, 0xF5, 0x44, 0x71, 0x1C, 0xE7, 0x9A, 0x02, 0xDC,
				0xC3, 0x42, 0x26, 0x68, 0x47, 0x80, 0xDD, 0xDC, 0xB4, 0x98, 0x59, 0x41,
				0x06, 0xC3, 0x7F, 0x5B, 0xC7, 0x98, 0x56, 0x48, 0x7A, 0xF5, 0xAB, 0x02,
				0x2A, 0x2E, 0x5E, 0x42, 0xF0, 0x98, 0x97, 0xC1, 0xA8, 0x5A, 0x11, 0xEA,
				0x02, 0x12, 0xAF, 0x04, 0xD9, 0xB4, 0xCE, 0xBC, 0x93, 0x7C, 0x3C, 0x1A,
				0x3E, 0x15, 0xA8, 0xA0, 0x34, 0x2E, 0x33, 0x76, 0x15, 0xC8, 0x4E, 0x7F,
				0xE3, 0xB8, 0xB9, 0xB8, 0x7F, 0xB1, 0xE7, 0x3A, 0x15, 0xAF, 0x12, 0xA3,
				0x0D, 0x74, 0x6E, 0x06, 0xDF, 0xC3, 0x4F, 0x29, 0x0D, 0x79, 0x7C, 0xE5,
				0x1A, 0xA1, 0x3A, 0xA7, 0x85, 0xBF, 0x66, 0x58, 0xAF, 0xF5, 0xE4, 0xB0,
				0x93, 0x00, 0x3C, 0xBE, 0xAF, 0x66, 0x5B, 0x3C, 0x2E, 0x11, 0x3A, 0x3A,
				0x4E, 0x90, 0x52, 0x69, 0x34, 0x1D, 0xC0, 0x71, 0x14, 0x26, 0x68, 0x5F,
				0x4E, 0xF3, 0x7E, 0x86, 0x8A, 0x81, 0x26, 0xFF, 0x3F, 0x22, 0x79, 0xB5,
				0x7C, 0xA6, 0x7E, 0x29
			};
			static const uchar dhtest_2048_256_xB[] = {
				0x7D, 0x62, 0xA7, 0xE3, 0xEF, 0x36, 0xDE, 0x61, 0x7B, 0x13, 0xD1, 0xAF,
				0xB8, 0x2C, 0x78, 0x0D, 0x83, 0xA2, 0x3B, 0xD4, 0xEE, 0x67, 0x05, 0x64,
				0x51, 0x21, 0xF3, 0x71, 0xF5, 0x46, 0xA5, 0x3D
			};
			static const uchar dhtest_2048_256_yB[] = {
				0x57, 0x5F, 0x03, 0x51, 0xBD, 0x2B, 0x1B, 0x81, 0x74, 0x48, 0xBD, 0xF8,
				0x7A, 0x6C, 0x36, 0x2C, 0x1E, 0x28, 0x9D, 0x39, 0x03, 0xA3, 0x0B, 0x98,
				0x32, 0xC5, 0x74, 0x1F, 0xA2, 0x50, 0x36, 0x3E, 0x7A, 0xCB, 0xC7, 0xF7,
				0x7F, 0x3D, 0xAC, 0xBC, 0x1F, 0x13, 0x1A, 0xDD, 0x8E, 0x03, 0x36, 0x7E,
				0xFF, 0x8F, 0xBB, 0xB3, 0xE1, 0xC5, 0x78, 0x44, 0x24, 0x80, 0x9B, 0x25,
				0xAF, 0xE4, 0xD2, 0x26, 0x2A, 0x1A, 0x6F, 0xD2, 0xFA, 0xB6, 0x41, 0x05,
				0xCA, 0x30, 0xA6, 0x74, 0xE0, 0x7F, 0x78, 0x09, 0x85, 0x20, 0x88, 0x63,
				0x2F, 0xC0, 0x49, 0x23, 0x37, 0x91, 0xAD, 0x4E, 0xDD, 0x08, 0x3A, 0x97,
				0x8B, 0x88, 0x3E, 0xE6, 0x18, 0xBC, 0x5E, 0x0D, 0xD0, 0x47, 0x41, 0x5F,
				0x2D, 0x95, 0xE6, 0x83, 0xCF, 0x14, 0x82, 0x6B, 0x5F, 0xBE, 0x10, 0xD3,
				0xCE, 0x41, 0xC6, 0xC1, 0x20, 0xC7, 0x8A, 0xB2, 0x00, 0x08, 0xC6, 0x98,
				0xBF, 0x7F, 0x0B, 0xCA, 0xB9, 0xD7, 0xF4, 0x07, 0xBE, 0xD0, 0xF4, 0x3A,
				0xFB, 0x29, 0x70, 0xF5, 0x7F, 0x8D, 0x12, 0x04, 0x39, 0x63, 0xE6, 0x6D,
				0xDD, 0x32, 0x0D, 0x59, 0x9A, 0xD9, 0x93, 0x6C, 0x8F, 0x44, 0x13, 0x7C,
				0x08, 0xB1, 0x80, 0xEC, 0x5E, 0x98, 0x5C, 0xEB, 0xE1, 0x86, 0xF3, 0xD5,
				0x49, 0x67, 0x7E, 0x80, 0x60, 0x73, 0x31, 0xEE, 0x17, 0xAF, 0x33, 0x80,
				0xA7, 0x25, 0xB0, 0x78, 0x23, 0x17, 0xD7, 0xDD, 0x43, 0xF5, 0x9D, 0x7A,
				0xF9, 0x56, 0x8A, 0x9B, 0xB6, 0x3A, 0x84, 0xD3, 0x65, 0xF9, 0x22, 0x44,
				0xED, 0x12, 0x09, 0x88, 0x21, 0x93, 0x02, 0xF4, 0x29, 0x24, 0xC7, 0xCA,
				0x90, 0xB8, 0x9D, 0x24, 0xF7, 0x1B, 0x0A, 0xB6, 0x97, 0x82, 0x3D, 0x7D,
				0xEB, 0x1A, 0xFF, 0x5B, 0x0E, 0x8E, 0x4A, 0x45, 0xD4, 0x9F, 0x7F, 0x53,
				0x75, 0x7E, 0x19, 0x13
			};
			static const uchar dhtest_2048_256_Z[] = {
				0x86, 0xC7, 0x0B, 0xF8, 0xD0, 0xBB, 0x81, 0xBB, 0x01, 0x07, 0x8A, 0x17,
				0x21, 0x9C, 0xB7, 0xD2, 0x72, 0x03, 0xDB, 0x2A, 0x19, 0xC8, 0x77, 0xF1,
				0xD1, 0xF1, 0x9F, 0xD7, 0xD7, 0x7E, 0xF2, 0x25, 0x46, 0xA6, 0x8F, 0x00,
				0x5A, 0xD5, 0x2D, 0xC8, 0x45, 0x53, 0xB7, 0x8F, 0xC6, 0x03, 0x30, 0xBE,
				0x51, 0xEA, 0x7C, 0x06, 0x72, 0xCA, 0xC1, 0x51, 0x5E, 0x4B, 0x35, 0xC0,
				0x47, 0xB9, 0xA5, 0x51, 0xB8, 0x8F, 0x39, 0xDC, 0x26, 0xDA, 0x14, 0xA0,
				0x9E, 0xF7, 0x47, 0x74, 0xD4, 0x7C, 0x76, 0x2D, 0xD1, 0x77, 0xF9, 0xED,
				0x5B, 0xC2, 0xF1, 0x1E, 0x52, 0xC8, 0x79, 0xBD, 0x95, 0x09, 0x85, 0x04,
				0xCD, 0x9E, 0xEC, 0xD8, 0xA8, 0xF9, 0xB3, 0xEF, 0xBD, 0x1F, 0x00, 0x8A,
				0xC5, 0x85, 0x30, 0x97, 0xD9, 0xD1, 0x83, 0x7F, 0x2B, 0x18, 0xF7, 0x7C,
				0xD7, 0xBE, 0x01, 0xAF, 0x80, 0xA7, 0xC7, 0xB5, 0xEA, 0x3C, 0xA5, 0x4C,
				0xC0, 0x2D, 0x0C, 0x11, 0x6F, 0xEE, 0x3F, 0x95, 0xBB, 0x87, 0x39, 0x93,
				0x85, 0x87, 0x5D, 0x7E, 0x86, 0x74, 0x7E, 0x67, 0x6E, 0x72, 0x89, 0x38,
				0xAC, 0xBF, 0xF7, 0x09, 0x8E, 0x05, 0xBE, 0x4D, 0xCF, 0xB2, 0x40, 0x52,
				0xB8, 0x3A, 0xEF, 0xFB, 0x14, 0x78, 0x3F, 0x02, 0x9A, 0xDB, 0xDE, 0x7F,
				0x53, 0xFA, 0xE9, 0x20, 0x84, 0x22, 0x40, 0x90, 0xE0, 0x07, 0xCE, 0xE9,
				0x4D, 0x4B, 0xF2, 0xBA, 0xCE, 0x9F, 0xFD, 0x4B, 0x57, 0xD2, 0xAF, 0x7C,
				0x72, 0x4D, 0x0C, 0xAA, 0x19, 0xBF, 0x05, 0x01, 0xF6, 0xF1, 0x7B, 0x4A,
				0xA1, 0x0F, 0x42, 0x5E, 0x3E, 0xA7, 0x60, 0x80, 0xB4, 0xB9, 0xD6, 0xB3,
				0xCE, 0xFE, 0xA1, 0x15, 0xB2, 0xCE, 0xB8, 0x78, 0x9B, 0xB8, 0xA3, 0xB0,
				0xEA, 0x87, 0xFE, 0xBE, 0x63, 0xB6, 0xC8, 0xF8, 0x46, 0xEC, 0x6D, 0xB0,
				0xC2, 0x6C, 0x5D, 0x7C
			};

			typedef struct {
				DH *(*get_param)();
				const uchar * xA;
				size_t xA_len;
				const uchar * yA;
				size_t yA_len;
				const uchar * xB;
				size_t xB_len;
				const uchar * yB;
				size_t yB_len;
				const uchar * Z;
				size_t Z_len;
			} rfc5114_td;

			#define make_rfc5114_td(pre) { \
					DH_get_ ## pre, \
					dhtest_ ## pre ## _xA, sizeof(dhtest_ ## pre ## _xA), \
					dhtest_ ## pre ## _yA, sizeof(dhtest_ ## pre ## _yA), \
					dhtest_ ## pre ## _xB, sizeof(dhtest_ ## pre ## _xB), \
					dhtest_ ## pre ## _yB, sizeof(dhtest_ ## pre ## _yB), \
					dhtest_ ## pre ## _Z, sizeof(dhtest_ ## pre ## _Z) \
			}
			static const rfc5114_td rfctd[] = {
				make_rfc5114_td(1024_160),
				make_rfc5114_td(2048_224),
				make_rfc5114_td(2048_256)
			};
			#undef make_rfc5114_td

			static int prime_groups[] = {
				NID_ffdhe2048,
				NID_ffdhe3072,
				NID_ffdhe4096,
				NID_ffdhe6144,
				NID_ffdhe8192,
				NID_modp_2048,
				NID_modp_3072,
				NID_modp_4096,
				NID_modp_6144,
			};
			static const uchar dh_pub_der[] = {
				0x30, 0x82, 0x02, 0x28, 0x30, 0x82, 0x01, 0x1b, 0x06, 0x09, 0x2a, 0x86,
				0x48, 0x86, 0xf7, 0x0d, 0x01, 0x03, 0x01, 0x30, 0x82, 0x01, 0x0c, 0x02,
				0x82, 0x01, 0x01, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xc9, 0x0f, 0xda, 0xa2, 0x21, 0x68, 0xc2, 0x34, 0xc4, 0xc6, 0x62, 0x8b,
				0x80, 0xdc, 0x1c, 0xd1, 0x29, 0x02, 0x4e, 0x08, 0x8a, 0x67, 0xcc, 0x74,
				0x02, 0x0b, 0xbe, 0xa6, 0x3b, 0x13, 0x9b, 0x22, 0x51, 0x4a, 0x08, 0x79,
				0x8e, 0x34, 0x04, 0xdd, 0xef, 0x95, 0x19, 0xb3, 0xcd, 0x3a, 0x43, 0x1b,
				0x30, 0x2b, 0x0a, 0x6d, 0xf2, 0x5f, 0x14, 0x37, 0x4f, 0xe1, 0x35, 0x6d,
				0x6d, 0x51, 0xc2, 0x45, 0xe4, 0x85, 0xb5, 0x76, 0x62, 0x5e, 0x7e, 0xc6,
				0xf4, 0x4c, 0x42, 0xe9, 0xa6, 0x37, 0xed, 0x6b, 0x0b, 0xff, 0x5c, 0xb6,
				0xf4, 0x06, 0xb7, 0xed, 0xee, 0x38, 0x6b, 0xfb, 0x5a, 0x89, 0x9f, 0xa5,
				0xae, 0x9f, 0x24, 0x11, 0x7c, 0x4b, 0x1f, 0xe6, 0x49, 0x28, 0x66, 0x51,
				0xec, 0xe4, 0x5b, 0x3d, 0xc2, 0x00, 0x7c, 0xb8, 0xa1, 0x63, 0xbf, 0x05,
				0x98, 0xda, 0x48, 0x36, 0x1c, 0x55, 0xd3, 0x9a, 0x69, 0x16, 0x3f, 0xa8,
				0xfd, 0x24, 0xcf, 0x5f, 0x83, 0x65, 0x5d, 0x23, 0xdc, 0xa3, 0xad, 0x96,
				0x1c, 0x62, 0xf3, 0x56, 0x20, 0x85, 0x52, 0xbb, 0x9e, 0xd5, 0x29, 0x07,
				0x70, 0x96, 0x96, 0x6d, 0x67, 0x0c, 0x35, 0x4e, 0x4a, 0xbc, 0x98, 0x04,
				0xf1, 0x74, 0x6c, 0x08, 0xca, 0x18, 0x21, 0x7c, 0x32, 0x90, 0x5e, 0x46,
				0x2e, 0x36, 0xce, 0x3b, 0xe3, 0x9e, 0x77, 0x2c, 0x18, 0x0e, 0x86, 0x03,
				0x9b, 0x27, 0x83, 0xa2, 0xec, 0x07, 0xa2, 0x8f, 0xb5, 0xc5, 0x5d, 0xf0,
				0x6f, 0x4c, 0x52, 0xc9, 0xde, 0x2b, 0xcb, 0xf6, 0x95, 0x58, 0x17, 0x18,
				0x39, 0x95, 0x49, 0x7c, 0xea, 0x95, 0x6a, 0xe5, 0x15, 0xd2, 0x26, 0x18,
				0x98, 0xfa, 0x05, 0x10, 0x15, 0x72, 0x8e, 0x5a, 0x8a, 0xac, 0xaa, 0x68,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x02, 0x01, 0x02, 0x02,
				0x02, 0x04, 0x00, 0x03, 0x82, 0x01, 0x05, 0x00, 0x02, 0x82, 0x01, 0x00,
				0x08, 0x87, 0x8a, 0x5f, 0x4f, 0x3b, 0xef, 0xe1, 0x77, 0x13, 0x3b, 0xd7,
				0x58, 0x76, 0xc9, 0xeb, 0x7e, 0x2d, 0xcc, 0x7e, 0xed, 0xc5, 0xee, 0xf9,
				0x2d, 0x55, 0xb0, 0xe2, 0x37, 0x8c, 0x51, 0x87, 0x6a, 0x8e, 0x0d, 0xb2,
				0x08, 0xed, 0x4f, 0x88, 0x9b, 0x63, 0x19, 0x7a, 0x67, 0xa1, 0x61, 0xd8,
				0x17, 0xa0, 0x2c, 0xdb, 0xc2, 0xfa, 0xb3, 0x4f, 0xe7, 0xcb, 0x16, 0xf2,
				0xe7, 0xd0, 0x2c, 0xf8, 0xcc, 0x97, 0xd3, 0xe7, 0xae, 0xc2, 0x71, 0xd8,
				0x2b, 0x12, 0x83, 0xe9, 0x5a, 0x45, 0xfe, 0x66, 0x5c, 0xa2, 0xb6, 0xce,
				0x2f, 0x04, 0x05, 0xe7, 0xa7, 0xbc, 0xe5, 0x63, 0x1a, 0x93, 0x3d, 0x4d,
				0xf4, 0x77, 0xdd, 0x2a, 0xc9, 0x51, 0x7b, 0xf5, 0x54, 0xa2, 0xab, 0x26,
				0xee, 0x16, 0xd3, 0x83, 0x92, 0x85, 0x40, 0x67, 0xa3, 0xa9, 0x31, 0x16,
				0x64, 0x45, 0x5a, 0x2a, 0x9d, 0xa8, 0x1a, 0x84, 0x2f, 0x59, 0x57, 0x6b,
				0xbb, 0x51, 0x28, 0xbd, 0x91, 0x60, 0xd9, 0x8f, 0x54, 0x6a, 0xa0, 0x6b,
				0xb2, 0xf6, 0x78, 0x79, 0xd2, 0x3a, 0x8f, 0xa6, 0x24, 0x7e, 0xe9, 0x6e,
				0x66, 0x30, 0xed, 0xbf, 0x55, 0x71, 0x9c, 0x89, 0x81, 0xf0, 0xa7, 0xe7,
				0x05, 0x87, 0x51, 0xc1, 0xff, 0xe5, 0xcf, 0x1f, 0x19, 0xe4, 0xeb, 0x7c,
				0x1c, 0x1a, 0x58, 0xd5, 0x22, 0x3d, 0x31, 0x22, 0xc7, 0x8b, 0x60, 0xf5,
				0xe8, 0x95, 0x73, 0xe0, 0x20, 0xe2, 0x4f, 0x03, 0x9e, 0x89, 0x34, 0x91,
				0x5e, 0xda, 0x4f, 0x60, 0xff, 0xc9, 0x4f, 0x5a, 0x37, 0x1e, 0xb0, 0xed,
				0x26, 0x4c, 0xa4, 0xc6, 0x26, 0xc9, 0xcc, 0xab, 0xd2, 0x1a, 0x3a, 0x82,
				0x68, 0x03, 0x49, 0x8f, 0xb0, 0xb9, 0xc8, 0x48, 0x9d, 0xc7, 0xdf, 0x8b,
				0x1c, 0xbf, 0xda, 0x89, 0x78, 0x6f, 0xd3, 0x62, 0xad, 0x35, 0xb9, 0xd3,
				0x9b, 0xd0, 0x25, 0x65
			};

			class TestInnerBlock_DH {
			public:
				static int cb(int p, int n, BN_GENCB * arg)
				{
					return 1;
				}
				static int dh_test()
				{
					DH * dh = NULL;
					BIGNUM * p = NULL, * q = NULL, * g = NULL;
					const BIGNUM * p2, * q2, * g2;
					BIGNUM * priv_key = NULL;
					const BIGNUM * pub_key2, * priv_key2;
					BN_GENCB * _cb = NULL;
					DH * a = NULL;
					DH * b = NULL;
					DH * c = NULL;
					const BIGNUM * ap = NULL, * ag = NULL, * apub_key = NULL;
					const BIGNUM * bpub_key = NULL, * bpriv_key = NULL;
					BIGNUM * bp = NULL, * bg = NULL, * cpriv_key = NULL;
					uchar * abuf = NULL;
					uchar * bbuf = NULL;
					uchar * cbuf = NULL;
					int i, alen, blen, clen, aout, bout, cout;
					int ret = 0;
					if(!TEST_ptr(dh = DH_new())
						|| !TEST_ptr(p = BN_new())
						|| !TEST_ptr(q = BN_new())
						|| !TEST_ptr(g = BN_new())
						|| !TEST_ptr(priv_key = BN_new()))
						goto err1;
					/*
					 * I) basic tests
					 */
					/* using a small predefined Sophie Germain DH group with generator 3 */
					if(!TEST_true(BN_set_word(p, 4079L)) || !TEST_true(BN_set_word(q, 2039L)) || !TEST_true(BN_set_word(g, 3L)) || !TEST_true(DH_set0_pqg(dh, p, q, g)))
						goto err1;
					/* check fails, because p is way too small */
					if(!DH_check(dh, &i))
						goto err2;
					i ^= DH_MODULUS_TOO_SMALL;
					if(!TEST_false(i & DH_CHECK_P_NOT_PRIME)
						|| !TEST_false(i & DH_CHECK_P_NOT_SAFE_PRIME)
						|| !TEST_false(i & DH_UNABLE_TO_CHECK_GENERATOR)
						|| !TEST_false(i & DH_NOT_SUITABLE_GENERATOR)
						|| !TEST_false(i & DH_CHECK_Q_NOT_PRIME)
						|| !TEST_false(i & DH_CHECK_INVALID_Q_VALUE)
						|| !TEST_false(i & DH_CHECK_INVALID_J_VALUE)
						|| !TEST_false(i & DH_MODULUS_TOO_SMALL)
						|| !TEST_false(i & DH_MODULUS_TOO_LARGE)
						|| !TEST_false(i))
						goto err2;
					/* test the combined getter for p, q, and g */
					DH_get0_pqg(dh, &p2, &q2, &g2);
					if(!TEST_ptr_eq(p2, p) || !TEST_ptr_eq(q2, q) || !TEST_ptr_eq(g2, g))
						goto err2;
					/* test the simple getters for p, q, and g */
					if(!TEST_ptr_eq(DH_get0_p(dh), p2) || !TEST_ptr_eq(DH_get0_q(dh), q2) || !TEST_ptr_eq(DH_get0_g(dh), g2))
						goto err2;
					/* set the private key only*/
					if(!TEST_true(BN_set_word(priv_key, 1234L)) || !TEST_true(DH_set0_key(dh, NULL, priv_key)))
						goto err2;
					/* test the combined getter for pub_key and priv_key */
					DH_get0_key(dh, &pub_key2, &priv_key2);
					if(!TEST_ptr_eq(pub_key2, NULL) || !TEST_ptr_eq(priv_key2, priv_key))
						goto err3;
					/* test the simple getters for pub_key and priv_key */
					if(!TEST_ptr_eq(DH_get0_pub_key(dh), pub_key2) || !TEST_ptr_eq(DH_get0_priv_key(dh), priv_key2))
						goto err3;
					/* now generate a key pair (expect failure since modulus is too small) */
					if(!TEST_false(DH_generate_key(dh)))
						goto err3;
					/* We'll have a stale error on the queue from the above test so clear it */
					ERR_clear_error();
					/*
					 * II) key generation
					 */
					/* generate a DH group ... */
					if(!TEST_ptr(_cb = BN_GENCB_new()))
						goto err3;
					BN_GENCB_set(_cb, &cb, NULL);
					if(!TEST_ptr(a = DH_new()) || !TEST_true(DH_generate_parameters_ex(a, 512, DH_GENERATOR_5, _cb)))
						goto err3;
					/* ... and check whether it is valid */
					if(!DH_check(a, &i))
						goto err3;
					if(!TEST_false(i & DH_CHECK_P_NOT_PRIME)
						|| !TEST_false(i & DH_CHECK_P_NOT_SAFE_PRIME)
						|| !TEST_false(i & DH_UNABLE_TO_CHECK_GENERATOR)
						|| !TEST_false(i & DH_NOT_SUITABLE_GENERATOR)
						|| !TEST_false(i & DH_CHECK_Q_NOT_PRIME)
						|| !TEST_false(i & DH_CHECK_INVALID_Q_VALUE)
						|| !TEST_false(i & DH_CHECK_INVALID_J_VALUE)
						|| !TEST_false(i & DH_MODULUS_TOO_SMALL)
						|| !TEST_false(i & DH_MODULUS_TOO_LARGE)
						|| !TEST_false(i))
						goto err3;
					DH_get0_pqg(a, &ap, NULL, &ag);
					/* now create another copy of the DH group for the peer */
					if(!TEST_ptr(b = DH_new()))
						goto err3;
					if(!TEST_ptr(bp = BN_dup(ap)) || !TEST_ptr(bg = BN_dup(ag)) || !TEST_true(DH_set0_pqg(b, bp, NULL, bg)))
						goto err3;
					bp = bg = NULL;
					/*
					 * III) simulate a key exchange
					 */
					if(!DH_generate_key(a))
						goto err3;
					DH_get0_key(a, &apub_key, NULL);
					if(!DH_generate_key(b))
						goto err3;
					DH_get0_key(b, &bpub_key, &bpriv_key);
					/* Also test with a private-key-only copy of |b|. */
					if(!TEST_ptr(c = DHparams_dup(b)) || !TEST_ptr(cpriv_key = BN_dup(bpriv_key)) || !TEST_true(DH_set0_key(c, NULL, cpriv_key)))
						goto err3;
					cpriv_key = NULL;
					alen = DH_size(a);
					if(!TEST_ptr(abuf = (uchar *)OPENSSL_malloc(alen)) || !TEST_true((aout = DH_compute_key(abuf, bpub_key, a)) != -1))
						goto err3;
					blen = DH_size(b);
					if(!TEST_ptr(bbuf = (uchar *)OPENSSL_malloc(blen)) || !TEST_true((bout = DH_compute_key(bbuf, apub_key, b)) != -1))
						goto err3;
					clen = DH_size(c);
					if(!TEST_ptr(cbuf = (uchar *)OPENSSL_malloc(clen))
						|| !TEST_true((cout = DH_compute_key(cbuf, apub_key, c)) != -1))
						goto err3;
					if(!TEST_true(aout >= 20) || !TEST_mem_eq(abuf, aout, bbuf, bout) || !TEST_mem_eq(abuf, aout, cbuf, cout))
						goto err3;
					ret = 1;
					goto success;
				err1:
					/* an error occurred before p,q,g were assigned to dh */
					BN_free(p);
					BN_free(q);
					BN_free(g);
				err2:
					/* an error occurred before priv_key was assigned to dh */
					BN_free(priv_key);
				err3:
				success:
					OPENSSL_free(abuf);
					OPENSSL_free(bbuf);
					OPENSSL_free(cbuf);
					DH_free(b);
					DH_free(a);
					DH_free(c);
					BN_free(bp);
					BN_free(bg);
					BN_free(cpriv_key);
					BN_GENCB_free(_cb);
					DH_free(dh);
					return ret;
				}
				static int dh_computekey_range_test()
				{
					int ret = 0, sz;
					DH * dh = NULL;
					BIGNUM * p = NULL, * q = NULL, * g = NULL, * pub = NULL, * priv = NULL;
					uchar * buf = NULL;
					if(!TEST_ptr(p = BN_dup(&ossl_bignum_ffdhe2048_p))
						|| !TEST_ptr(q = BN_dup(&ossl_bignum_ffdhe2048_q))
						|| !TEST_ptr(g = BN_dup(&ossl_bignum_const_2))
						|| !TEST_ptr(dh = DH_new())
						|| !TEST_true(DH_set0_pqg(dh, p, q, g)))
						goto err;
					p = q = g = NULL;
					if(!TEST_int_gt(sz = DH_size(dh), 0) || !TEST_ptr(buf = (uchar *)OPENSSL_malloc(sz)) || !TEST_ptr(pub = BN_new()) || !TEST_ptr(priv = BN_new()))
						goto err;
					if(!TEST_true(BN_set_word(priv, 1)) || !TEST_true(DH_set0_key(dh, NULL, priv)))
						goto err;
					priv = NULL;
					if(!TEST_true(BN_set_word(pub, 1)))
						goto err;
					/* Given z = pub ^ priv mod p */
					/* Test that z == 1 fails */
					if(!TEST_int_le(ossl_dh_compute_key(buf, pub, dh), 0))
						goto err;
					/* Test that z == 0 fails */
					if(!TEST_ptr(BN_copy(pub, DH_get0_p(dh))) || !TEST_int_le(ossl_dh_compute_key(buf, pub, dh), 0))
						goto err;
					/* Test that z == p - 1 fails */
					if(!TEST_true(BN_sub_word(pub, 1)) || !TEST_int_le(ossl_dh_compute_key(buf, pub, dh), 0))
						goto err;
					/* Test that z == p - 2 passes */
					if(!TEST_true(BN_sub_word(pub, 1)) || !TEST_int_eq(ossl_dh_compute_key(buf, pub, dh), sz))
						goto err;
					ret = 1;
				err:
					OPENSSL_free(buf);
					BN_free(priv);
					BN_free(pub);
					BN_free(g);
					BN_free(q);
					BN_free(p);
					DH_free(dh);
					return ret;
				}
				static int rfc5114_test()
				{
					int i;
					DH * dhA = NULL;
					DH * dhB = NULL;
					uchar * Z1 = NULL;
					uchar * Z2 = NULL;
					int szA, szB;
					const rfc5114_td * td = NULL;
					BIGNUM * priv_key = NULL, * pub_key = NULL;
					const BIGNUM * pub_key_tmp;
					for(i = 0; i < (int)SIZEOFARRAY(rfctd); i++) {
						td = rfctd + i;
						/* Set up DH structures setting key components */
						if(!TEST_ptr(dhA = td->get_param()) || !TEST_ptr(dhB = td->get_param()))
							goto bad_err;
						if(!TEST_ptr(priv_key = BN_bin2bn(td->xA, td->xA_len, NULL))
							|| !TEST_ptr(pub_key = BN_bin2bn(td->yA, td->yA_len, NULL))
							|| !TEST_true(DH_set0_key(dhA, pub_key, priv_key)))
							goto bad_err;
						if(!TEST_ptr(priv_key = BN_bin2bn(td->xB, td->xB_len, NULL))
							|| !TEST_ptr(pub_key = BN_bin2bn(td->yB, td->yB_len, NULL))
							|| !TEST_true(DH_set0_key(dhB, pub_key, priv_key)))
							goto bad_err;
						priv_key = pub_key = NULL;
						if(!TEST_int_gt(szA = DH_size(dhA), 0) || !TEST_int_gt(szB = DH_size(dhB), 0)
							|| !TEST_size_t_eq(td->Z_len, (size_t)szA)
							|| !TEST_size_t_eq(td->Z_len, (size_t)szB))
							goto err;
						if(!TEST_ptr(Z1 = (uchar *)OPENSSL_malloc((size_t)szA)) || !TEST_ptr(Z2 = (uchar *)OPENSSL_malloc((size_t)szB)))
							goto bad_err;
						/*
						 * Work out shared secrets using both sides and compare with expected values.
						 */
						DH_get0_key(dhB, &pub_key_tmp, NULL);
						if(!TEST_int_ne(DH_compute_key(Z1, pub_key_tmp, dhA), -1))
							goto bad_err;
						DH_get0_key(dhA, &pub_key_tmp, NULL);
						if(!TEST_int_ne(DH_compute_key(Z2, pub_key_tmp, dhB), -1))
							goto bad_err;
						if(!TEST_mem_eq(Z1, td->Z_len, td->Z, td->Z_len) || !TEST_mem_eq(Z2, td->Z_len, td->Z, td->Z_len))
							goto err;
						DH_free(dhA);
						dhA = NULL;
						DH_free(dhB);
						dhB = NULL;
						OPENSSL_free(Z1);
						Z1 = NULL;
						OPENSSL_free(Z2);
						Z2 = NULL;
					}
					return 1;
				bad_err:
					DH_free(dhA);
					DH_free(dhB);
					BN_free(pub_key);
					BN_free(priv_key);
					OPENSSL_free(Z1);
					OPENSSL_free(Z2);
					TEST_error("Initialisation error RFC5114 set %d\n", i + 1);
					return 0;
				err:
					DH_free(dhA);
					DH_free(dhB);
					OPENSSL_free(Z1);
					OPENSSL_free(Z2);
					TEST_error("Test failed RFC5114 set %d\n", i + 1);
					return 0;
				}
				static int rfc7919_test()
				{
					DH * a = NULL, * b = NULL;
					const BIGNUM * apub_key = NULL, * bpub_key = NULL;
					uchar * abuf = NULL;
					uchar * bbuf = NULL;
					int i, alen, blen, aout, bout;
					int ret = 0;
					if(!TEST_ptr(a = DH_new_by_nid(NID_ffdhe2048)))
						goto err;
					if(!DH_check(a, &i))
						goto err;
					if(!TEST_false(i & DH_CHECK_P_NOT_PRIME)
						|| !TEST_false(i & DH_CHECK_P_NOT_SAFE_PRIME)
						|| !TEST_false(i & DH_UNABLE_TO_CHECK_GENERATOR)
						|| !TEST_false(i & DH_NOT_SUITABLE_GENERATOR)
						|| !TEST_false(i))
						goto err;
					if(!DH_generate_key(a))
						goto err;
					DH_get0_key(a, &apub_key, NULL);
					/* now create another copy of the DH group for the peer */
					if(!TEST_ptr(b = DH_new_by_nid(NID_ffdhe2048)))
						goto err;
					if(!DH_generate_key(b))
						goto err;
					DH_get0_key(b, &bpub_key, NULL);
					alen = DH_size(a);
					if(!TEST_int_gt(alen, 0) || !TEST_ptr(abuf = (uchar *)OPENSSL_malloc(alen)) || !TEST_true((aout = DH_compute_key(abuf, bpub_key, a)) != -1))
						goto err;
					blen = DH_size(b);
					if(!TEST_int_gt(blen, 0) || !TEST_ptr(bbuf = (uchar *)OPENSSL_malloc(blen)) || !TEST_true((bout = DH_compute_key(bbuf, apub_key, b)) != -1))
						goto err;
					if(!TEST_true(aout >= 20) || !TEST_mem_eq(abuf, aout, bbuf, bout))
						goto err;
					ret = 1;
				err:
					OPENSSL_free(abuf);
					OPENSSL_free(bbuf);
					DH_free(a);
					DH_free(b);
					return ret;
				}
				static int dh_test_prime_groups(int index)
				{
					int ok = 0;
					DH * dh = NULL;
					const BIGNUM * p, * q, * g;
					if(!TEST_ptr(dh = DH_new_by_nid(prime_groups[index])))
						goto err;
					DH_get0_pqg(dh, &p, &q, &g);
					if(!TEST_ptr(p) || !TEST_ptr(q) || !TEST_ptr(g))
						goto err;
					if(!TEST_int_eq(DH_get_nid(dh), prime_groups[index]))
						goto err;
					// Since q is set there is no need for the private length to be set 
					if(!TEST_int_eq((int)DH_get_length(dh), 0))
						goto err;
					ok = 1;
				err:
					DH_free(dh);
					return ok;
				}
				static int dh_rfc5114_fix_nid_test()
				{
					int ok = 0;
					/* Run the test. Success is any time the test does not cause a SIGSEGV interrupt */
					EVP_PKEY_CTX * paramgen_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_DHX, 0);
					if(!TEST_ptr(paramgen_ctx))
						goto err;
					if(!TEST_int_eq(EVP_PKEY_paramgen_init(paramgen_ctx), 1))
						goto err;
					/* Tested function is called here */
					if(!TEST_int_eq(EVP_PKEY_CTX_set_dhx_rfc5114(paramgen_ctx, 3), 1))
						goto err;
					/* Negative test */
					if(!TEST_int_eq(EVP_PKEY_CTX_set_dhx_rfc5114(paramgen_ctx, 99), 0))
						goto err;
					/* If we're still running then the test passed. */
					ok = 1;
				err:
					EVP_PKEY_CTX_free(paramgen_ctx);
					return ok;
				}
				static int dh_set_dh_nid_test()
				{
					int ok = 0;
					/* Run the test. Success is any time the test does not cause a SIGSEGV interrupt */
					EVP_PKEY_CTX * paramgen_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_DH, 0);
					if(!TEST_ptr(paramgen_ctx))
						goto err;
					if(!TEST_int_eq(EVP_PKEY_paramgen_init(paramgen_ctx), 1))
						goto err;
					/* Tested function is called here */
					if(!TEST_int_eq(EVP_PKEY_CTX_set_dh_nid(paramgen_ctx, NID_ffdhe2048), 1))
						goto err;
					/* Negative test */
					if(!TEST_int_eq(EVP_PKEY_CTX_set_dh_nid(paramgen_ctx, NID_secp521r1), 0))
						goto err;
					/* If we're still running then the test passed. */
					ok = 1;
				err:
					EVP_PKEY_CTX_free(paramgen_ctx);
					return ok;
				}
				static int dh_get_nid()
				{
					int ok = 0;
					const BIGNUM * p, * q, * g;
					BIGNUM * pcpy = NULL, * gcpy = NULL, * qcpy = NULL;
					DH * dh1 = DH_new_by_nid(NID_ffdhe2048);
					DH * dh2 = DH_new();
					if(!TEST_ptr(dh1) || !TEST_ptr(dh2))
						goto err;
					// Set new DH parameters manually using a existing named group's p & g 
					DH_get0_pqg(dh1, &p, &q, &g);
					if(!TEST_ptr(p) || !TEST_ptr(q) || !TEST_ptr(g) || !TEST_ptr(pcpy = BN_dup(p)) || !TEST_ptr(gcpy = BN_dup(g)))
						goto err;
					if(!TEST_true(DH_set0_pqg(dh2, pcpy, NULL, gcpy)))
						goto err;
					pcpy = gcpy = NULL;
					/* Test q is set if p and g are provided */
					if(!TEST_ptr(DH_get0_q(dh2)))
						goto err;
					/* Test that setting p & g manually returns that it is a named group */
					if(!TEST_int_eq(DH_get_nid(dh2), NID_ffdhe2048))
						goto err;
					/* Test that after changing g it is no longer a named group */
					if(!TEST_ptr(gcpy = BN_dup(BN_value_one())))
						goto err;
					if(!TEST_true(DH_set0_pqg(dh2, NULL, NULL, gcpy)))
						goto err;
					gcpy = NULL;
					if(!TEST_int_eq(DH_get_nid(dh2), NID_undef))
						goto err;
					/* Test that setting an incorrect q results in this not being a named group */
					if(!TEST_ptr(pcpy = BN_dup(p))
						|| !TEST_ptr(qcpy = BN_dup(q))
						|| !TEST_ptr(gcpy = BN_dup(g))
						|| !TEST_int_eq(BN_add_word(qcpy, 2), 1)
						|| !TEST_true(DH_set0_pqg(dh2, pcpy, qcpy, gcpy)))
						goto err;
					pcpy = qcpy = gcpy = NULL;
					if(!TEST_int_eq(DH_get_nid(dh2), NID_undef))
						goto err;
					ok = 1;
				err:
					BN_free(pcpy);
					BN_free(qcpy);
					BN_free(gcpy);
					DH_free(dh2);
					DH_free(dh1);
					return ok;
				}
				//
				// Load PKCS3 DH Parameters that contain an optional private value length.
				// Loading a named group should not overwrite the private value length field.
				//
				static int dh_load_pkcs3_namedgroup_privlen_test()
				{
					int privlen = 0;
					EVP_PKEY * pkey = NULL;
					const uchar * p = dh_pub_der;
					int ret = TEST_ptr(pkey = d2i_PUBKEY_ex(NULL, &p, sizeof(dh_pub_der), NULL, NULL))
						&& TEST_true(EVP_PKEY_get_int_param(pkey, OSSL_PKEY_PARAM_DH_PRIV_LEN, &privlen))
						&& TEST_int_eq(privlen, 1024);
					EVP_PKEY_free(pkey);
					return ret;
				}
			};
			ADD_TEST(TestInnerBlock_DH::dh_test);
			ADD_TEST(TestInnerBlock_DH::dh_computekey_range_test);
			ADD_TEST(TestInnerBlock_DH::rfc5114_test);
			ADD_TEST(TestInnerBlock_DH::rfc7919_test);
			ADD_ALL_TESTS(TestInnerBlock_DH::dh_test_prime_groups, SIZEOFARRAY(prime_groups));
			ADD_TEST(TestInnerBlock_DH::dh_get_nid);
			ADD_TEST(TestInnerBlock_DH::dh_load_pkcs3_namedgroup_privlen_test);
			ADD_TEST(TestInnerBlock_DH::dh_rfc5114_fix_nid_test);
			ADD_TEST(TestInnerBlock_DH::dh_set_dh_nid_test);
		#else
			TEST_note("No DH support");
		#endif
	}
	{
		class TestInnerBlock_ProvFetch {
		public:
			static int dummy_decoder_decode(void * ctx, OSSL_CORE_BIO * cin, int selection, OSSL_CALLBACK * object_cb, void * object_cbarg, OSSL_PASSPHRASE_CALLBACK * pw_cb, void * pw_cbarg)
				{ return 0; }
			static int dummy_encoder_encode(void * ctx, OSSL_CORE_BIO * out, const void * obj_raw, const OSSL_PARAM obj_abstract[], int selection, OSSL_PASSPHRASE_CALLBACK * cb, void * cbarg)
				{ return 0; }
			static void * dummy_store_open(void * provctx, const char * uri)
				{ return NULL; }
			static int dummy_store_load(void * loaderctx,  OSSL_CALLBACK * object_cb, void * object_cbarg, OSSL_PASSPHRASE_CALLBACK * pw_cb, void * pw_cbarg)
				{ return 0; }
			static int dumm_store_eof(void * loaderctx)
				{ return 0; }
			static int dummy_store_close(void * loaderctx)
				{ return 0; }
			static void * dummy_rand_newctx(void * provctx, void * parent, const OSSL_DISPATCH * parent_calls)
				{ return provctx; }
			static void dummy_rand_freectx(void * vctx)
				{}
			static int dummy_rand_instantiate(void * vdrbg, uint strength, int prediction_resistance, const uchar * pstr, size_t pstr_len, const OSSL_PARAM params[])
				{ return 1; }
			static int dummy_rand_uninstantiate(void * vdrbg)
				{ return 1; }
			static int dummy_rand_generate(void * vctx, uchar * out, size_t outlen, uint strength, int prediction_resistance, const uchar * addin, size_t addin_len)
			{
				for(size_t i = 0; i < outlen; i++)
					out[i] = (uchar)(i & 0xff);
				return 1;
			}
			static const OSSL_PARAM * dummy_rand_gettable_ctx_params(void * vctx, void * provctx)
			{
				static const OSSL_PARAM known_gettable_ctx_params[] = {
					OSSL_PARAM_size_t(OSSL_RAND_PARAM_MAX_REQUEST, NULL),
					OSSL_PARAM_END
				};
				return known_gettable_ctx_params;
			}
			static int dummy_rand_get_ctx_params(void * vctx, OSSL_PARAM params[])
			{
				OSSL_PARAM * p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_MAX_REQUEST);
				if(p && !OSSL_PARAM_set_size_t(p, INT_MAX))
					return 0;
				return 1;
			}
			static int dummy_rand_enable_locking(void * vtest)
				{ return 1; }
			static int dummy_rand_lock(void * vtest)
				{ return 1; }
			static void dummy_rand_unlock(void * vtest)
				{}
			static const OSSL_ALGORITHM * dummy_query(void * provctx, int operation_id, int * no_cache)
			{
				static const OSSL_DISPATCH dummy_decoder_functions[] = {
					{ OSSL_FUNC_DECODER_DECODE, (void (*)())dummy_decoder_decode },
					{ 0, NULL }
				};
				static const OSSL_ALGORITHM dummy_decoders[] = {
					{ "DUMMY", "provider=dummy,input=pem", dummy_decoder_functions },
					{ NULL, NULL, NULL }
				};
				static const OSSL_DISPATCH dummy_encoder_functions[] = {
					{ OSSL_FUNC_DECODER_DECODE, (void (*)())dummy_encoder_encode },
					{ 0, NULL }
				};
				static const OSSL_ALGORITHM dummy_encoders[] = {
					{ "DUMMY", "provider=dummy,output=pem", dummy_encoder_functions },
					{ NULL, NULL, NULL }
				};
				static const OSSL_DISPATCH dummy_store_functions[] = {
					{ OSSL_FUNC_STORE_OPEN, (void (*)())dummy_store_open },
					{ OSSL_FUNC_STORE_LOAD, (void (*)())dummy_store_load },
					{ OSSL_FUNC_STORE_EOF, (void (*)())dumm_store_eof },
					{ OSSL_FUNC_STORE_CLOSE, (void (*)())dummy_store_close },
					{ 0, NULL }
				};
				static const OSSL_ALGORITHM dummy_store[] = {
					{ "DUMMY", "provider=dummy", dummy_store_functions },
					{ NULL, NULL, NULL }
				};
				static const OSSL_DISPATCH dummy_rand_functions[] = {
					{ OSSL_FUNC_RAND_NEWCTX, (void (*)())dummy_rand_newctx },
					{ OSSL_FUNC_RAND_FREECTX, (void (*)())dummy_rand_freectx },
					{ OSSL_FUNC_RAND_INSTANTIATE, (void (*)())dummy_rand_instantiate },
					{ OSSL_FUNC_RAND_UNINSTANTIATE, (void (*)())dummy_rand_uninstantiate },
					{ OSSL_FUNC_RAND_GENERATE, (void (*)())dummy_rand_generate },
					{ OSSL_FUNC_RAND_GETTABLE_CTX_PARAMS, (void (*)())dummy_rand_gettable_ctx_params },
					{ OSSL_FUNC_RAND_GET_CTX_PARAMS, (void (*)())dummy_rand_get_ctx_params },
					{ OSSL_FUNC_RAND_ENABLE_LOCKING, (void (*)())dummy_rand_enable_locking },
					{ OSSL_FUNC_RAND_LOCK, (void (*)())dummy_rand_lock },
					{ OSSL_FUNC_RAND_UNLOCK, (void (*)())dummy_rand_unlock },
					{ 0, NULL }
				};
				static const OSSL_ALGORITHM dummy_rand[] = {
					{ "DUMMY", "provider=dummy", dummy_rand_functions },
					{ NULL, NULL, NULL }
				};
				*no_cache = 0;
				switch(operation_id) {
					case OSSL_OP_DECODER: return dummy_decoders;
					case OSSL_OP_ENCODER: return dummy_encoders;
					case OSSL_OP_STORE: return dummy_store;
					case OSSL_OP_RAND: return dummy_rand;
				}
				return NULL;
			}
			static int dummy_provider_init(const OSSL_CORE_HANDLE * handle, const OSSL_DISPATCH * in, const OSSL_DISPATCH ** out, void ** provctx)
			{
				static const OSSL_DISPATCH dummy_dispatch_table[] = {
					{ OSSL_FUNC_PROVIDER_QUERY_OPERATION, (void (*)())dummy_query },
					{ OSSL_FUNC_PROVIDER_TEARDOWN, (void (*)())OSSL_LIB_CTX_free },
					{ 0, NULL }
				};
				OSSL_LIB_CTX * libctx = OSSL_LIB_CTX_new_child(handle, in);
				uchar buf[32];
				*provctx = (void*)libctx;
				*out = dummy_dispatch_table;
				/*
				 * Do some work using the child libctx, to make sure this is possible from
				 * inside the init function.
				 */
				if(RAND_bytes_ex(libctx, buf, sizeof(buf), 0) <= 0)
					return 0;
				return 1;
			}
			/*
			 * Try fetching and freeing various things.
			 * Test 0: Decoder
			 * Test 1: Encoder
			 * Test 2: Store loader
			 * Test 3: EVP_RAND
			 * Test 4-7: As above, but additionally with a query string
			 */
			static int fetch_test(int tst)
			{
				OSSL_LIB_CTX * libctx = OSSL_LIB_CTX_new();
				OSSL_PROVIDER * dummyprov = NULL;
				OSSL_PROVIDER * nullprov = NULL;
				OSSL_DECODER * decoder = NULL;
				OSSL_ENCODER * encoder = NULL;
				OSSL_STORE_LOADER * loader = NULL;
				int testresult = 0;
				uchar buf[32];
				int query = tst > 3;
				if(!TEST_ptr(libctx))
					goto err;
				if(!TEST_true(OSSL_PROVIDER_add_builtin(libctx, "dummy-prov", dummy_provider_init)) || 
					!TEST_ptr(nullprov = OSSL_PROVIDER_load(libctx, "default")) || !TEST_ptr(dummyprov = OSSL_PROVIDER_load(libctx, "dummy-prov")))
					goto err;
				switch(tst % 4) {
					case 0:
						decoder = OSSL_DECODER_fetch(libctx, "DUMMY", query ? "provider=dummy" : NULL);
						if(!TEST_ptr(decoder))
							goto err;
						break;
					case 1:
						encoder = OSSL_ENCODER_fetch(libctx, "DUMMY", query ? "provider=dummy" : NULL);
						if(!TEST_ptr(encoder))
							goto err;
						break;
					case 2:
						loader = OSSL_STORE_LOADER_fetch(libctx, "DUMMY", query ? "provider=dummy" : NULL);
						if(!TEST_ptr(loader))
							goto err;
						break;
					case 3:
						if(!TEST_true(RAND_set_DRBG_type(libctx, "DUMMY", query ? "provider=dummy" : NULL, NULL, NULL)) || !TEST_int_ge(RAND_bytes_ex(libctx, buf, sizeof(buf), 0), 1))
							goto err;
						break;
					default:
						goto err;
				}
				testresult = 1;
			err:
				OSSL_DECODER_free(decoder);
				OSSL_ENCODER_free(encoder);
				OSSL_STORE_LOADER_free(loader);
				OSSL_PROVIDER_unload(dummyprov);
				OSSL_PROVIDER_unload(nullprov);
				OSSL_LIB_CTX_free(libctx);
				return testresult;
			}
		};
		
		ADD_ALL_TESTS(TestInnerBlock_ProvFetch::fetch_test, 8);
	}
	{
		typedef struct test_fixture {
			const char * test_case_name;
			int pkistatus;
			const char * str; /* Not freed by tear_down */
			const char * text; /* Not freed by tear_down */
			int pkifailure;
		} CMP_STATUS_TEST_FIXTURE;

		class TestInnerBlock_CmpStatus {
		public:
			static CMP_STATUS_TEST_FIXTURE * set_up(const char * const test_case_name)
			{
				CMP_STATUS_TEST_FIXTURE * fixture;
				if(!TEST_ptr(fixture = (CMP_STATUS_TEST_FIXTURE*)OPENSSL_zalloc(sizeof(*fixture))))
					return NULL;
				fixture->test_case_name = test_case_name;
				return fixture;
			}
			static void tear_down(CMP_STATUS_TEST_FIXTURE * fixture)
			{
				OPENSSL_free(fixture);
			}
			/*
			 * Tests PKIStatusInfo creation and get-functions
			 */
			static int execute_PKISI_test(CMP_STATUS_TEST_FIXTURE * fixture)
			{
				OSSL_CMP_PKISI * si = NULL;
				int status;
				ASN1_UTF8STRING * statusString = NULL;
				int res = 0, i;
				if(!TEST_ptr(si = OSSL_CMP_STATUSINFO_new(fixture->pkistatus, fixture->pkifailure, fixture->text)))
					goto end;
				status = ossl_cmp_pkisi_get_status(si);
				if(!TEST_int_eq(fixture->pkistatus, status) || !TEST_str_eq(fixture->str, ossl_cmp_PKIStatus_to_string(status)))
					goto end;
				if(!TEST_ptr(statusString = sk_ASN1_UTF8STRING_value(ossl_cmp_pkisi_get0_statusString(si), 0)) || !TEST_mem_eq(fixture->text, strlen(fixture->text),
					(char*)statusString->data, statusString->length))
					goto end;
				if(!TEST_int_eq(fixture->pkifailure, ossl_cmp_pkisi_get_pkifailureinfo(si)))
					goto end;
				for(i = 0; i <= OSSL_CMP_PKIFAILUREINFO_MAX; i++)
					if(!TEST_int_eq((fixture->pkifailure >> i) & 1, ossl_cmp_pkisi_check_pkifailureinfo(si, i)))
						goto end;
				res = 1;
			end:
				OSSL_CMP_PKISI_free(si);
				return res;
			}
			static int test_PKISI(void)
			{
				SETUP_TEST_FIXTURE(CMP_STATUS_TEST_FIXTURE, set_up);
				fixture->pkistatus = OSSL_CMP_PKISTATUS_revocationNotification;
				fixture->str = "PKIStatus: revocation notification - a revocation of the cert has occurred";
				fixture->text = "this is an additional text describing the failure";
				fixture->pkifailure = OSSL_CMP_CTX_FAILINFO_unsupportedVersion | OSSL_CMP_CTX_FAILINFO_badDataFormat;
				EXECUTE_TEST(execute_PKISI_test, tear_down);
				return result;
			}
		};
		/*-
		 * this tests all of:
		 * OSSL_CMP_STATUSINFO_new()
		 * ossl_cmp_pkisi_get_status()
		 * ossl_cmp_PKIStatus_to_string()
		 * ossl_cmp_pkisi_get0_statusString()
		 * ossl_cmp_pkisi_get_pkifailureinfo()
		 * ossl_cmp_pkisi_check_pkifailureinfo()
		 */
		ADD_TEST(TestInnerBlock_CmpStatus::test_PKISI);
	}
	{
		class TestInnerBlock_Err {
		public:
			#ifndef OPENSSL_NO_DEPRECATED_3_0
				//#define IS_HEX(ch) ((ch >= '0' && ch <='9') || (ch >= 'A' && ch <='F'))
				static int test_print_error_format(void)
				{
					/* Variables used to construct an error line */
					char * lib;
					const char * func = OPENSSL_FUNC;
					char * reason;
				#ifdef OPENSSL_NO_ERR
					char reasonbuf[255];
				#endif
				#ifndef OPENSSL_NO_FILENAMES
					const char * file = OPENSSL_FILE;
					const int line = OPENSSL_LINE;
				#else
					const char * file = "";
					const int line = 0;
				#endif
					/* The format for OpenSSL error lines */
					const char * expected_format = ":error:%08lX:%s:%s:%s:%s:%d";
					/*-
					 *                                          ^^ ^^ ^^ ^^ ^^
					 * "library" name --------------------------++ || || || ||
					 * function name ------------------------------++ || || ||
					 * reason string (system error string) -----------++ || ||
					 * file name ----------------------------------------++ ||
					 * line number -----------------------------------------++
					 */
					char expected[512];

					char * out = NULL, * p = NULL;
					int ret = 0, len;
					BIO * bio = NULL;
					const int syserr = EPERM;
					unsigned long errorcode;
					unsigned long reasoncode;
					/*
					 * We set a mark here so we can clear the system error that we generate
					 * with ERR_PUT_error().  That is, after all, just a simulation to verify
					 * ERR_print_errors() output, not a real error.
					 */
					ERR_set_mark();
					ERR_PUT_error(ERR_LIB_SYS, 0, syserr, file, line);
					errorcode = ERR_peek_error();
					reasoncode = ERR_GET_REASON(errorcode);
					if(!TEST_int_eq(reasoncode, syserr)) {
						ERR_pop_to_mark();
						goto err;
					}
				#if !defined(OPENSSL_NO_ERR)
				#if defined(OPENSSL_NO_AUTOERRINIT)
					lib = "lib(2)";
				#else
					lib = "system library";
				#endif
					reason = strerror(syserr);
				#else
					lib = "lib(2)";
					BIO_snprintf(reasonbuf, sizeof(reasonbuf), "reason(%lu)", reasoncode);
					reason = reasonbuf;
				#endif
					BIO_snprintf(expected, sizeof(expected), expected_format, errorcode, lib, func, reason, file, line);
					if(!TEST_ptr(bio = BIO_new(BIO_s_mem())))
						goto err;
					ERR_print_errors(bio);
					if(!TEST_int_gt(len = BIO_get_mem_data(bio, &out), 0))
						goto err;
					/* Skip over the variable thread id at the start of the string */
					for(p = out; *p != ':' && *p != 0; ++p) {
						if(!TEST_true(ishex(*p)))
							goto err;
					}
					if(!TEST_true(*p != 0) || !TEST_strn_eq(expected, p, strlen(expected)))
						goto err;
					ret = 1;
				err:
					BIO_free(bio);
					return ret;
				}
			#endif
			/* Test that querying the error queue preserves the OS error. */
			static int preserves_system_error(void)
			{
			#if defined(OPENSSL_SYS_WINDOWS)
				SetLastError(ERROR_INVALID_FUNCTION);
				ERR_get_error();
				return TEST_int_eq(GetLastError(), ERROR_INVALID_FUNCTION);
			#else
				errno = EINVAL;
				ERR_get_error();
				return TEST_int_eq(errno, EINVAL);
			#endif
			}
			/* Test that calls to ERR_add_error_[v]data append */
			static int vdata_appends(void)
			{
				const char * data;
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_MALLOC_FAILURE);
				ERR_add_error_data(1, "hello ");
				ERR_add_error_data(1, "world");
				ERR_peek_error_data(&data, NULL);
				return TEST_str_eq(data, "hello world");
			}
			static int raised_error(void)
			{
				const char * f, * data;
				int l;
				unsigned long e;
				/*
				 * When OPENSSL_NO_ERR or OPENSSL_NO_FILENAMES, no file name or line
				 * number is saved, so no point checking them.
				 */
			#if !defined(OPENSSL_NO_FILENAMES) && !defined(OPENSSL_NO_ERR)
				const char * file = __FILE__;
				int line = __LINE__ + 2; /* The error is generated on the ERR_raise_data line */
			#endif
				ERR_raise_data(ERR_LIB_NONE, ERR_R_INTERNAL_ERROR, "calling exit()");
				if(!TEST_ulong_ne(e = ERR_get_error_all(&f, &l, NULL, &data, NULL), 0) || !TEST_int_eq(ERR_GET_REASON(e), ERR_R_INTERNAL_ERROR)
			#if !defined(OPENSSL_NO_FILENAMES) && !defined(OPENSSL_NO_ERR)
					|| !TEST_int_eq(l, line) || !TEST_str_eq(f, file)
			#endif
					|| !TEST_str_eq(data, "calling exit()"))
					return 0;
				return 1;
			}
			static int test_marks(void)
			{
				unsigned long mallocfail, shouldnot;
				/* Set an initial error */
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_MALLOC_FAILURE);
				mallocfail = ERR_peek_last_error();
				if(!TEST_ulong_gt(mallocfail, 0))
					return 0;
				/* Setting and clearing a mark should not affect the error */
				if(!TEST_true(ERR_set_mark())
					|| !TEST_true(ERR_pop_to_mark())
					|| !TEST_ulong_eq(mallocfail, ERR_peek_last_error())
					|| !TEST_true(ERR_set_mark())
					|| !TEST_true(ERR_clear_last_mark())
					|| !TEST_ulong_eq(mallocfail, ERR_peek_last_error()))
					return 0;

				/* Test popping errors */
				if(!TEST_true(ERR_set_mark()))
					return 0;
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_INTERNAL_ERROR);
				if(!TEST_ulong_ne(mallocfail, ERR_peek_last_error())
					|| !TEST_true(ERR_pop_to_mark())
					|| !TEST_ulong_eq(mallocfail, ERR_peek_last_error()))
					return 0;

				/* Nested marks should also work */
				if(!TEST_true(ERR_set_mark())
					|| !TEST_true(ERR_set_mark()))
					return 0;
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_INTERNAL_ERROR);
				if(!TEST_ulong_ne(mallocfail, ERR_peek_last_error())
					|| !TEST_true(ERR_pop_to_mark())
					|| !TEST_true(ERR_pop_to_mark())
					|| !TEST_ulong_eq(mallocfail, ERR_peek_last_error()))
					return 0;

				if(!TEST_true(ERR_set_mark()))
					return 0;
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
				shouldnot = ERR_peek_last_error();
				if(!TEST_ulong_ne(mallocfail, shouldnot)
					|| !TEST_true(ERR_set_mark()))
					return 0;
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_INTERNAL_ERROR);
				if(!TEST_ulong_ne(shouldnot, ERR_peek_last_error())
					|| !TEST_true(ERR_pop_to_mark())
					|| !TEST_ulong_eq(shouldnot, ERR_peek_last_error())
					|| !TEST_true(ERR_pop_to_mark())
					|| !TEST_ulong_eq(mallocfail, ERR_peek_last_error()))
					return 0;

				/* Setting and clearing a mark should not affect the errors on the stack */
				if(!TEST_true(ERR_set_mark()))
					return 0;
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
				if(!TEST_true(ERR_clear_last_mark()) || !TEST_ulong_eq(shouldnot, ERR_peek_last_error()))
					return 0;
				/*
				 * Popping where no mark has been set should pop everything - but return
				 * a failure result
				 */
				if(!TEST_false(ERR_pop_to_mark()) || !TEST_ulong_eq(0, ERR_peek_last_error()))
					return 0;
				/* Clearing where there is no mark should fail */
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_MALLOC_FAILURE);
				if(!TEST_false(ERR_clear_last_mark())
					/* "get" the last error to remove it */
					|| !TEST_ulong_eq(mallocfail, ERR_get_error()) || !TEST_ulong_eq(0, ERR_peek_last_error()))
					return 0;
				/*
				 * Setting a mark where there are no errors in the stack should fail.
				 * NOTE: This is somewhat surprising behaviour but is historically how this
				 * function behaves. In practice we typically set marks without first
				 * checking whether there is anything on the stack - but we also don't
				 * tend to check the success of this function. It turns out to work anyway
				 * because although setting a mark with no errors fails, a subsequent call
				 * to ERR_pop_to_mark() or ERR_clear_last_mark() will do the right thing
				 * anyway (even though they will report a failure result).
				 */
				if(!TEST_false(ERR_set_mark()))
					return 0;
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_MALLOC_FAILURE);
				if(!TEST_true(ERR_set_mark()))
					return 0;
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_INTERNAL_ERROR);
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
				/* Should be able to "pop" past 2 errors */
				if(!TEST_true(ERR_pop_to_mark()) || !TEST_ulong_eq(mallocfail, ERR_peek_last_error()))
					return 0;
				if(!TEST_true(ERR_set_mark()))
					return 0;
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_INTERNAL_ERROR);
				ERR_raise(ERR_LIB_CRYPTO, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
				/* Should be able to "clear" past 2 errors */
				if(!TEST_true(ERR_clear_last_mark()) || !TEST_ulong_eq(shouldnot, ERR_peek_last_error()))
					return 0;
				/* Clear remaining errors from last test */
				ERR_clear_error();
				return 1;
			}
			static int test_clear_error(void)
			{
				int flags = -1;
				const char * data = NULL;
				int res = 0;
				/* Raise an error with data and clear it */
				ERR_raise_data(0, 0, "hello %s", "world");
				ERR_peek_error_data(&data, &flags);
				if(!TEST_str_eq(data, "hello world") || !TEST_int_eq(flags, ERR_TXT_STRING | ERR_TXT_MALLOCED))
					goto err;
				ERR_clear_error();
				/* Raise a new error without data */
				ERR_raise(0, 0);
				ERR_peek_error_data(&data, &flags);
				if(!TEST_str_eq(data, "") || !TEST_int_eq(flags, ERR_TXT_MALLOCED))
					goto err;
				ERR_clear_error();
				/* Raise a new error with data */
				ERR_raise_data(0, 0, "goodbye %s world", "cruel");
				ERR_peek_error_data(&data, &flags);
				if(!TEST_str_eq(data, "goodbye cruel world") || !TEST_int_eq(flags, ERR_TXT_STRING | ERR_TXT_MALLOCED))
					goto err;
				ERR_clear_error();
				/*
				 * Raise a new error without data to check that the malloced storage is freed properly
				 */
				ERR_raise(0, 0);
				ERR_peek_error_data(&data, &flags);
				if(!TEST_str_eq(data, "") || !TEST_int_eq(flags, ERR_TXT_MALLOCED))
					goto err;
				ERR_clear_error();
				res = 1;
			err:
				ERR_clear_error();
				return res;
			}
		};

		ADD_TEST(TestInnerBlock_Err::preserves_system_error);
		ADD_TEST(TestInnerBlock_Err::vdata_appends);
		ADD_TEST(TestInnerBlock_Err::raised_error);
		#ifndef OPENSSL_NO_DEPRECATED_3_0
			ADD_TEST(TestInnerBlock_Err::test_print_error_format);
		#endif
		ADD_TEST(TestInnerBlock_Err::test_marks);
		ADD_TEST(TestInnerBlock_Err::test_clear_error);
	}
	return 1;
}

void cleanup_tests()
{
	{
		#ifndef OPENSSL_NO_SM2
			fake_rand_finish(TestData_Sm2Internal_fake_rand);
		#endif
	}
	{
		BN_CTX_free(TestData_BnInternal_ctx);
	}
	{
		SSL_free(TestData_CipherBytes_s);
		SSL_CTX_free(TestData_CipherBytes_ctx);
	}
	{
		BN_CTX_free(TestData_BN_ctx);
	}
	{
		BUF_MEM_free(TestData_WPACKET_buf);
	}
}
