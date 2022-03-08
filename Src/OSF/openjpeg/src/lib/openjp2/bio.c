/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2002-2014, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2014, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux
 * Copyright (c) 2003-2014, Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 */
 #include "opj_includes.h"
#pragma hdrstop

/** @defgroup BIO BIO - Individual bit input-output stream */
/*@{*/

/** @name Local static functions */
/*@{*/
/*@}*/
/*@}*/
/*
   ==========================================================
   local functions
   ==========================================================
 */
/**
   Write a byte
   @param bio BIO handle
   @return Returns TRUE if successful, returns FALSE otherwise
 */
static boolint FASTCALL opj_bio_byteout(opj_bio_t * bio)
{
	bio->buf = (bio->buf << 8) & 0xffff;
	bio->ct = bio->buf == 0xff00 ? 7 : 8;
	if((OPJ_SIZE_T)bio->bp >= (OPJ_SIZE_T)bio->end) {
		return FALSE;
	}
	*bio->bp++ = (uint8)(bio->buf >> 8);
	return TRUE;
}
/**
   Read a byte
   @param bio BIO handle
   @return Returns TRUE if successful, returns FALSE otherwise
 */
static boolint FASTCALL opj_bio_bytein(opj_bio_t * bio)
{
	bio->buf = (bio->buf << 8) & 0xffff;
	bio->ct = bio->buf == 0xff00 ? 7 : 8;
	if((OPJ_SIZE_T)bio->bp >= (OPJ_SIZE_T)bio->end) {
		return FALSE;
	}
	bio->buf |= *bio->bp++;
	return TRUE;
}
/**
   Write a bit
   @param bio BIO handle
   @param b Bit to write (0 or 1)
 */
static void FASTCALL opj_bio_putbit(opj_bio_t * bio, OPJ_UINT32 b)
{
	if(bio->ct == 0) {
		opj_bio_byteout(bio); /* MSD: why not check the return value of this function ? */
	}
	bio->ct--;
	bio->buf |= b << bio->ct;
}
/**
   Read a bit
   @param bio BIO handle
   @return Returns the read bit
 */
static OPJ_UINT32 FASTCALL opj_bio_getbit(opj_bio_t * bio)
{
	if(bio->ct == 0) {
		opj_bio_bytein(bio); /* MSD: why not check the return value of this function ? */
	}
	bio->ct--;
	return (bio->buf >> bio->ct) & 1;
}
/*
   ==========================================================
   Bit Input/Output interface
   ==========================================================
 */

opj_bio_t* opj_bio_create(void)
{
	opj_bio_t * bio = (opj_bio_t*)SAlloc::M(sizeof(opj_bio_t));
	return bio;
}

void opj_bio_destroy(opj_bio_t * bio)
{
	SAlloc::F(bio);
}

ptrdiff_t opj_bio_numbytes(opj_bio_t * bio)
{
	return (bio->bp - bio->start);
}

void opj_bio_init_enc(opj_bio_t * bio, uint8 * bp, OPJ_UINT32 len)
{
	bio->start = bp;
	bio->end = bp + len;
	bio->bp = bp;
	bio->buf = 0;
	bio->ct = 8;
}

void opj_bio_init_dec(opj_bio_t * bio, uint8 * bp, OPJ_UINT32 len)
{
	bio->start = bp;
	bio->end = bp + len;
	bio->bp = bp;
	bio->buf = 0;
	bio->ct = 0;
}

void FASTCALL opj_bio_write(opj_bio_t * bio, OPJ_UINT32 v, OPJ_UINT32 n)
{
	assert((n > 0U) && (n <= 32U));
	for(OPJ_INT32 i = (OPJ_INT32)n - 1; i >= 0; i--) {
		opj_bio_putbit(bio, (v >> i) & 1);
	}
}

OPJ_UINT32 FASTCALL opj_bio_read(opj_bio_t * bio, OPJ_UINT32 n)
{
	OPJ_INT32 i;
	OPJ_UINT32 v;
	assert((n > 0U) /* && (n <= 32U)*/);
#ifdef OPJ_UBSAN_BUILD
	/* This assert fails for some corrupted images which are gracefully rejected */
	/* Add this assert only for ubsan build. */
	/* This is the condition for overflow not to occur below which is needed because of OPJ_NOSANITIZE */
	assert(n <= 32U);
#endif
	v = 0U;
	for(i = (OPJ_INT32)n - 1; i >= 0; i--) {
		v |= opj_bio_getbit(bio) << i; /* can't overflow, opj_bio_getbit returns 0 or 1 */
	}
	return v;
}

boolint opj_bio_flush(opj_bio_t * bio)
{
	if(!opj_bio_byteout(bio)) {
		return FALSE;
	}
	if(bio->ct == 7) {
		if(!opj_bio_byteout(bio)) {
			return FALSE;
		}
	}
	return TRUE;
}

boolint opj_bio_inalign(opj_bio_t * bio)
{
	if((bio->buf & 0xff) == 0xff) {
		if(!opj_bio_bytein(bio)) {
			return FALSE;
		}
	}
	bio->ct = 0;
	return TRUE;
}
