/// \file       lzma_encoder_private.h
/// \brief      Private definitions for LZMA encoder
//  Authors:    Igor Pavlov
//              Lasse Collin
//  This file has been put into the public domain. You can do whatever you want with this file.
//
#ifndef LZMA_LZMA_ENCODER_PRIVATE_H
#define LZMA_LZMA_ENCODER_PRIVATE_H

#include "range_encoder.h"

// Macro to compare if the first two bytes in two buffers differ. This is
// needed in lzma_lzma_optimum_*() to test if the match is at least
// MATCH_LEN_MIN bytes. Unaligned access gives tiny gain so there's no
// reason to not use it when it is supported.
#ifdef TUKLIB_FAST_UNALIGNED_ACCESS
	#define not_equal_16(a, b) (read16ne(a) != read16ne(b))
#else
	#define not_equal_16(a, b) ((a)[0] != (b)[0] || (a)[1] != (b)[1])
#endif

// Optimal - Number of entries in the optimum array.
#define OPTS (1 << 12)

struct lzma_length_encoder {
	probability choice;
	probability choice2;
	probability low[POS_STATES_MAX][LEN_LOW_SYMBOLS];
	probability mid[POS_STATES_MAX][LEN_MID_SYMBOLS];
	probability high[LEN_HIGH_SYMBOLS];
	uint32 prices[POS_STATES_MAX][LEN_SYMBOLS];
	uint32 table_size;
	uint32 counters[POS_STATES_MAX];
};

struct lzma_optimal {
	lzma_lzma_state state;
	bool prev_1_is_literal;
	bool prev_2;
	uint32 pos_prev_2;
	uint32 back_prev_2;
	uint32 price;
	uint32 pos_prev;  // pos_next;
	uint32 back_prev;
	uint32 backs[REPS];
};

struct lzma_lzma1_encoder_s {
	lzma_range_encoder rc; /// Range encoder
	lzma_lzma_state state; /// State
	uint32 reps[REPS]; /// The four most recent match distances
	lzma_match matches[MATCH_LEN_MAX + 1]; /// Array of match candidates
	uint32 matches_count; /// Number of match candidates in matches[]
	uint32 longest_match_length; /// Variable to hold the length of the longest match between calls to lzma_lzma_optimum_*().
	bool fast_mode; /// True if using getoptimumfast
	bool is_initialized; /// True if the encoder has been initialized by encoding the first byte as a literal.
	bool is_flushed; /// True if the range encoder has been flushed, but not all bytes have been written to the output buffer yet.
	uint32 pos_mask;         ///< (1 << pos_bits) - 1
	uint32 literal_context_bits;
	uint32 literal_pos_mask;
	// These are the same as in lzma_decoder.c. See comments there.
	probability literal[LITERAL_CODERS_MAX][LITERAL_CODER_SIZE];
	probability is_match[STATES][POS_STATES_MAX];
	probability is_rep[STATES];
	probability is_rep0[STATES];
	probability is_rep1[STATES];
	probability is_rep2[STATES];
	probability is_rep0_long[STATES][POS_STATES_MAX];
	probability dist_slot[DIST_STATES][DIST_SLOTS];
	probability dist_special[FULL_DISTANCES - DIST_MODEL_END];
	probability dist_align[ALIGN_SIZE];
	// These are the same as in lzma_decoder.c except that the encoders include also price tables.
	lzma_length_encoder match_len_encoder;
	lzma_length_encoder rep_len_encoder;
	// Price tables
	uint32 dist_slot_prices[DIST_STATES][DIST_SLOTS];
	uint32 dist_prices[DIST_STATES][FULL_DISTANCES];
	uint32 dist_table_size;
	uint32 match_price_count;
	uint32 align_prices[ALIGN_SIZE];
	uint32 align_price_count;
	// Optimal
	uint32 opts_end_index;
	uint32 opts_current_index;
	lzma_optimal opts[OPTS];
};

extern void lzma_lzma_optimum_fast(lzma_lzma1_encoder * coder, lzma_mf * mf, uint32 * back_res, uint32 * len_res);
extern void lzma_lzma_optimum_normal(lzma_lzma1_encoder * coder, lzma_mf * mf, uint32 * back_res, uint32 * len_res, uint32 position);

#endif
