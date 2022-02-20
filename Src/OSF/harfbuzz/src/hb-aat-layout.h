/*
 * Copyright © 2018  Ebrahim Byagowi
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 */
#ifndef HB_AAT_H_IN
#error "Include <hb-aat.h> instead."
#endif

#ifndef HB_AAT_LAYOUT_H
#define HB_AAT_LAYOUT_H

#include "hb.h"
#include "hb-ot.h"

HB_BEGIN_DECLS

/**
 * hb_aat_layout_feature_type_t:
 *
 *
 * Since: 2.2.0
 */
typedef enum {
	HB_AAT_LAYOUT_FEATURE_TYPE_INVALID                    = 0xFFFF,

	HB_AAT_LAYOUT_FEATURE_TYPE_ALL_TYPOGRAPHIC            = 0,
	HB_AAT_LAYOUT_FEATURE_TYPE_LIGATURES                  = 1,
	HB_AAT_LAYOUT_FEATURE_TYPE_CURISVE_CONNECTION         = 2,
	HB_AAT_LAYOUT_FEATURE_TYPE_LETTER_CASE                = 3,
	HB_AAT_LAYOUT_FEATURE_TYPE_VERTICAL_SUBSTITUTION      = 4,
	HB_AAT_LAYOUT_FEATURE_TYPE_LINGUISTIC_REARRANGEMENT   = 5,
	HB_AAT_LAYOUT_FEATURE_TYPE_NUMBER_SPACING             = 6,
	HB_AAT_LAYOUT_FEATURE_TYPE_SMART_SWASH_TYPE           = 8,
	HB_AAT_LAYOUT_FEATURE_TYPE_DIACRITICS_TYPE            = 9,
	HB_AAT_LAYOUT_FEATURE_TYPE_VERTICAL_POSITION          = 10,
	HB_AAT_LAYOUT_FEATURE_TYPE_FRACTIONS                  = 11,
	HB_AAT_LAYOUT_FEATURE_TYPE_OVERLAPPING_CHARACTERS_TYPE        = 13,
	HB_AAT_LAYOUT_FEATURE_TYPE_TYPOGRAPHIC_EXTRAS         = 14,
	HB_AAT_LAYOUT_FEATURE_TYPE_MATHEMATICAL_EXTRAS        = 15,
	HB_AAT_LAYOUT_FEATURE_TYPE_ORNAMENT_SETS_TYPE         = 16,
	HB_AAT_LAYOUT_FEATURE_TYPE_CHARACTER_ALTERNATIVES     = 17,
	HB_AAT_LAYOUT_FEATURE_TYPE_DESIGN_COMPLEXITY_TYPE     = 18,
	HB_AAT_LAYOUT_FEATURE_TYPE_STYLE_OPTIONS              = 19,
	HB_AAT_LAYOUT_FEATURE_TYPE_CHARACTER_SHAPE            = 20,
	HB_AAT_LAYOUT_FEATURE_TYPE_NUMBER_CASE                = 21,
	HB_AAT_LAYOUT_FEATURE_TYPE_TEXT_SPACING               = 22,
	HB_AAT_LAYOUT_FEATURE_TYPE_TRANSLITERATION            = 23,
	HB_AAT_LAYOUT_FEATURE_TYPE_ANNOTATION_TYPE            = 24,
	HB_AAT_LAYOUT_FEATURE_TYPE_KANA_SPACING_TYPE          = 25,
	HB_AAT_LAYOUT_FEATURE_TYPE_IDEOGRAPHIC_SPACING_TYPE   = 26,
	HB_AAT_LAYOUT_FEATURE_TYPE_UNICODE_DECOMPOSITION_TYPE = 27,
	HB_AAT_LAYOUT_FEATURE_TYPE_RUBY_KANA                  = 28,
	HB_AAT_LAYOUT_FEATURE_TYPE_CJK_SYMBOL_ALTERNATIVES_TYPE       = 29,
	HB_AAT_LAYOUT_FEATURE_TYPE_IDEOGRAPHIC_ALTERNATIVES_TYPE      = 30,
	HB_AAT_LAYOUT_FEATURE_TYPE_CJK_VERTICAL_ROMAN_PLACEMENT_TYPE  = 31,
	HB_AAT_LAYOUT_FEATURE_TYPE_ITALIC_CJK_ROMAN           = 32,
	HB_AAT_LAYOUT_FEATURE_TYPE_CASE_SENSITIVE_LAYOUT      = 33,
	HB_AAT_LAYOUT_FEATURE_TYPE_ALTERNATE_KANA             = 34,
	HB_AAT_LAYOUT_FEATURE_TYPE_STYLISTIC_ALTERNATIVES     = 35,
	HB_AAT_LAYOUT_FEATURE_TYPE_CONTEXTUAL_ALTERNATIVES    = 36,
	HB_AAT_LAYOUT_FEATURE_TYPE_LOWER_CASE                 = 37,
	HB_AAT_LAYOUT_FEATURE_TYPE_UPPER_CASE                 = 38,
	HB_AAT_LAYOUT_FEATURE_TYPE_LANGUAGE_TAG_TYPE          = 39,
	HB_AAT_LAYOUT_FEATURE_TYPE_CJK_ROMAN_SPACING_TYPE     = 103,

	_HB_AAT_LAYOUT_FEATURE_TYPE_MAX_VALUE = HB_TAG_MAX_SIGNED /*< skip >*/
} hb_aat_layout_feature_type_t;

/**
 * hb_aat_layout_feature_selector_t:
 *
 *
 * Since: 2.2.0
 */
typedef enum {
	HB_AAT_LAYOUT_FEATURE_SELECTOR_INVALID                = 0xFFFF,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_ALL_TYPOGRAPHIC */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ALL_TYPE_FEATURES_ON   = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ALL_TYPE_FEATURES_OFF  = 1,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_LIGATURES */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_REQUIRED_LIGATURES_ON  = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_REQUIRED_LIGATURES_OFF = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_COMMON_LIGATURES_ON    = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_COMMON_LIGATURES_OFF   = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_RARE_LIGATURES_ON      = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_RARE_LIGATURES_OFF     = 5,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_LOGOS_ON               = 6,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_LOGOS_OFF              = 7,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_REBUS_PICTURES_ON      = 8,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_REBUS_PICTURES_OFF     = 9,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DIPHTHONG_LIGATURES_ON = 10,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DIPHTHONG_LIGATURES_OFF        = 11,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SQUARED_LIGATURES_ON   = 12,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SQUARED_LIGATURES_OFF  = 13,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ABBREV_SQUARED_LIGATURES_ON    = 14,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ABBREV_SQUARED_LIGATURES_OFF   = 15,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SYMBOL_LIGATURES_ON    = 16,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SYMBOL_LIGATURES_OFF   = 17,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CONTEXTUAL_LIGATURES_ON        = 18,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CONTEXTUAL_LIGATURES_OFF       = 19,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HISTORICAL_LIGATURES_ON        = 20,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HISTORICAL_LIGATURES_OFF       = 21,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_LIGATURES */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_UNCONNECTED            = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PARTIALLY_CONNECTED    = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CURSIVE                = 2,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_LETTER_CASE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_UPPER_AND_LOWER_CASE   = 0,/* deprecated */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ALL_CAPS               = 1,/* deprecated */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ALL_LOWER_CASE         = 2,/* deprecated */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SMALL_CAPS             = 3,/* deprecated */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_INITIAL_CAPS           = 4,/* deprecated */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_INITIAL_CAPS_AND_SMALL_CAPS    = 5,/* deprecated */

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_VERTICAL_SUBSTITUTION */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SUBSTITUTE_VERTICAL_FORMS_ON   = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SUBSTITUTE_VERTICAL_FORMS_OFF  = 1,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_LINGUISTIC_REARRANGEMENT */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_LINGUISTIC_REARRANGEMENT_ON    = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_LINGUISTIC_REARRANGEMENT_OFF   = 1,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_NUMBER_SPACING */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_MONOSPACED_NUMBERS     = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PROPORTIONAL_NUMBERS   = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_THIRD_WIDTH_NUMBERS    = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_QUARTER_WIDTH_NUMBERS  = 3,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_SMART_SWASH_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_WORD_INITIAL_SWASHES_ON        = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_WORD_INITIAL_SWASHES_OFF       = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_WORD_FINAL_SWASHES_ON  = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_WORD_FINAL_SWASHES_OFF = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_LINE_INITIAL_SWASHES_ON        = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_LINE_INITIAL_SWASHES_OFF       = 5,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_LINE_FINAL_SWASHES_ON  = 6,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_LINE_FINAL_SWASHES_OFF = 7,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NON_FINAL_SWASHES_ON   = 8,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NON_FINAL_SWASHES_OFF  = 9,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_DIACRITICS_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SHOW_DIACRITICS        = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HIDE_DIACRITICS        = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DECOMPOSE_DIACRITICS   = 2,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_VERTICAL_POSITION */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NORMAL_POSITION        = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SUPERIORS              = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_INFERIORS              = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ORDINALS               = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SCIENTIFIC_INFERIORS   = 4,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_FRACTIONS */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NO_FRACTIONS           = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_VERTICAL_FRACTIONS     = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DIAGONAL_FRACTIONS     = 2,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_OVERLAPPING_CHARACTERS_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PREVENT_OVERLAP_ON     = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PREVENT_OVERLAP_OFF    = 1,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_TYPOGRAPHIC_EXTRAS */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HYPHENS_TO_EM_DASH_ON  = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HYPHENS_TO_EM_DASH_OFF = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HYPHEN_TO_EN_DASH_ON   = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HYPHEN_TO_EN_DASH_OFF  = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SLASHED_ZERO_ON        = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SLASHED_ZERO_OFF       = 5,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_FORM_INTERROBANG_ON    = 6,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_FORM_INTERROBANG_OFF   = 7,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SMART_QUOTES_ON        = 8,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SMART_QUOTES_OFF       = 9,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PERIODS_TO_ELLIPSIS_ON = 10,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PERIODS_TO_ELLIPSIS_OFF        = 11,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_MATHEMATICAL_EXTRAS */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HYPHEN_TO_MINUS_ON     = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HYPHEN_TO_MINUS_OFF    = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ASTERISK_TO_MULTIPLY_ON        = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ASTERISK_TO_MULTIPLY_OFF       = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SLASH_TO_DIVIDE_ON     = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SLASH_TO_DIVIDE_OFF    = 5,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_INEQUALITY_LIGATURES_ON        = 6,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_INEQUALITY_LIGATURES_OFF       = 7,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_EXPONENTS_ON           = 8,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_EXPONENTS_OFF          = 9,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_MATHEMATICAL_GREEK_ON  = 10,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_MATHEMATICAL_GREEK_OFF = 11,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_ORNAMENT_SETS_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NO_ORNAMENTS           = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DINGBATS               = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PI_CHARACTERS          = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_FLEURONS               = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DECORATIVE_BORDERS     = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_INTERNATIONAL_SYMBOLS  = 5,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_MATH_SYMBOLS           = 6,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_CHARACTER_ALTERNATIVES */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NO_ALTERNATES          = 0,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_DESIGN_COMPLEXITY_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DESIGN_LEVEL1          = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DESIGN_LEVEL2          = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DESIGN_LEVEL3          = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DESIGN_LEVEL4          = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DESIGN_LEVEL5          = 4,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_STYLE_OPTIONS */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NO_STYLE_OPTIONS       = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DISPLAY_TEXT           = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ENGRAVED_TEXT          = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ILLUMINATED_CAPS       = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_TITLING_CAPS           = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_TALL_CAPS              = 5,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_CHARACTER_SHAPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_TRADITIONAL_CHARACTERS = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SIMPLIFIED_CHARACTERS  = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_JIS1978_CHARACTERS     = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_JIS1983_CHARACTERS     = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_JIS1990_CHARACTERS     = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_TRADITIONAL_ALT_ONE    = 5,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_TRADITIONAL_ALT_TWO    = 6,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_TRADITIONAL_ALT_THREE  = 7,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_TRADITIONAL_ALT_FOUR   = 8,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_TRADITIONAL_ALT_FIVE   = 9,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_EXPERT_CHARACTERS      = 10,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_JIS2004_CHARACTERS     = 11,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HOJO_CHARACTERS        = 12,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NLCCHARACTERS          = 13,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_TRADITIONAL_NAMES_CHARACTERS   = 14,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_NUMBER_CASE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_LOWER_CASE_NUMBERS     = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_UPPER_CASE_NUMBERS     = 1,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_TEXT_SPACING */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PROPORTIONAL_TEXT      = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_MONOSPACED_TEXT        = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HALF_WIDTH_TEXT        = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_THIRD_WIDTH_TEXT       = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_QUARTER_WIDTH_TEXT     = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ALT_PROPORTIONAL_TEXT  = 5,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ALT_HALF_WIDTH_TEXT    = 6,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_TRANSLITERATION */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NO_TRANSLITERATION     = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HANJA_TO_HANGUL        = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HIRAGANA_TO_KATAKANA   = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_KATAKANA_TO_HIRAGANA   = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_KANA_TO_ROMANIZATION   = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ROMANIZATION_TO_HIRAGANA       = 5,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ROMANIZATION_TO_KATAKANA       = 6,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HANJA_TO_HANGUL_ALT_ONE        = 7,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HANJA_TO_HANGUL_ALT_TWO        = 8,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HANJA_TO_HANGUL_ALT_THREE      = 9,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_ANNOTATION_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NO_ANNOTATION          = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_BOX_ANNOTATION         = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ROUNDED_BOX_ANNOTATION = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CIRCLE_ANNOTATION      = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_INVERTED_CIRCLE_ANNOTATION     = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PARENTHESIS_ANNOTATION = 5,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PERIOD_ANNOTATION      = 6,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ROMAN_NUMERAL_ANNOTATION       = 7,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DIAMOND_ANNOTATION     = 8,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_INVERTED_BOX_ANNOTATION        = 9,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_INVERTED_ROUNDED_BOX_ANNOTATION = 10,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_KANA_SPACING_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_FULL_WIDTH_KANA        = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PROPORTIONAL_KANA      = 1,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_IDEOGRAPHIC_SPACING_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_FULL_WIDTH_IDEOGRAPHS  = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PROPORTIONAL_IDEOGRAPHS        = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HALF_WIDTH_IDEOGRAPHS  = 2,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_UNICODE_DECOMPOSITION_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CANONICAL_COMPOSITION_ON       = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CANONICAL_COMPOSITION_OFF      = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_COMPATIBILITY_COMPOSITION_ON   = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_COMPATIBILITY_COMPOSITION_OFF  = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_TRANSCODING_COMPOSITION_ON     = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_TRANSCODING_COMPOSITION_OFF    = 5,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_RUBY_KANA */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NO_RUBY_KANA           = 0,/* deprecated - use HB_AAT_LAYOUT_FEATURE_SELECTOR_RUBY_KANA_OFF instead */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_RUBY_KANA              = 1,/* deprecated - use HB_AAT_LAYOUT_FEATURE_SELECTOR_RUBY_KANA_ON instead */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_RUBY_KANA_ON           = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_RUBY_KANA_OFF          = 3,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_CJK_SYMBOL_ALTERNATIVES_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NO_CJK_SYMBOL_ALTERNATIVES     = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CJK_SYMBOL_ALT_ONE     = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CJK_SYMBOL_ALT_TWO     = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CJK_SYMBOL_ALT_THREE   = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CJK_SYMBOL_ALT_FOUR    = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CJK_SYMBOL_ALT_FIVE    = 5,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_IDEOGRAPHIC_ALTERNATIVES_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NO_IDEOGRAPHIC_ALTERNATIVES    = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_IDEOGRAPHIC_ALT_ONE    = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_IDEOGRAPHIC_ALT_TWO    = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_IDEOGRAPHIC_ALT_THREE  = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_IDEOGRAPHIC_ALT_FOUR   = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_IDEOGRAPHIC_ALT_FIVE   = 5,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_CJK_VERTICAL_ROMAN_PLACEMENT_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CJK_VERTICAL_ROMAN_CENTERED    = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CJK_VERTICAL_ROMAN_HBASELINE   = 1,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_ITALIC_CJK_ROMAN */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NO_CJK_ITALIC_ROMAN    = 0,/* deprecated - use HB_AAT_LAYOUT_FEATURE_SELECTOR_CJK_ITALIC_ROMAN_OFF instead */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CJK_ITALIC_ROMAN       = 1,/* deprecated - use HB_AAT_LAYOUT_FEATURE_SELECTOR_CJK_ITALIC_ROMAN_ON instead */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CJK_ITALIC_ROMAN_ON    = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CJK_ITALIC_ROMAN_OFF   = 3,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_CASE_SENSITIVE_LAYOUT */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CASE_SENSITIVE_LAYOUT_ON       = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CASE_SENSITIVE_LAYOUT_OFF      = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CASE_SENSITIVE_SPACING_ON      = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CASE_SENSITIVE_SPACING_OFF     = 3,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_ALTERNATE_KANA */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ALTERNATE_HORIZ_KANA_ON        = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ALTERNATE_HORIZ_KANA_OFF       = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ALTERNATE_VERT_KANA_ON = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_ALTERNATE_VERT_KANA_OFF        = 3,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_STYLISTIC_ALTERNATIVES */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_NO_STYLISTIC_ALTERNATES        = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_ONE_ON   = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_ONE_OFF  = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_TWO_ON   = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_TWO_OFF  = 5,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_THREE_ON = 6,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_THREE_OFF        = 7,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_FOUR_ON  = 8,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_FOUR_OFF = 9,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_FIVE_ON  = 10,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_FIVE_OFF = 11,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_SIX_ON   = 12,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_SIX_OFF  = 13,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_SEVEN_ON = 14,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_SEVEN_OFF        = 15,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_EIGHT_ON = 16,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_EIGHT_OFF        = 17,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_NINE_ON  = 18,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_NINE_OFF = 19,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_TEN_ON   = 20,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_TEN_OFF  = 21,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_ELEVEN_ON        = 22,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_ELEVEN_OFF       = 23,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_TWELVE_ON        = 24,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_TWELVE_OFF       = 25,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_THIRTEEN_ON      = 26,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_THIRTEEN_OFF     = 27,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_FOURTEEN_ON      = 28,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_FOURTEEN_OFF     = 29,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_FIFTEEN_ON       = 30,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_FIFTEEN_OFF      = 31,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_SIXTEEN_ON       = 32,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_SIXTEEN_OFF      = 33,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_SEVENTEEN_ON     = 34,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_SEVENTEEN_OFF    = 35,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_EIGHTEEN_ON      = 36,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_EIGHTEEN_OFF     = 37,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_NINETEEN_ON      = 38,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_NINETEEN_OFF     = 39,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_TWENTY_ON        = 40,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_STYLISTIC_ALT_TWENTY_OFF       = 41,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_CONTEXTUAL_ALTERNATIVES */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CONTEXTUAL_ALTERNATES_ON       = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CONTEXTUAL_ALTERNATES_OFF      = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SWASH_ALTERNATES_ON    = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_SWASH_ALTERNATES_OFF   = 3,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CONTEXTUAL_SWASH_ALTERNATES_ON = 4,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_CONTEXTUAL_SWASH_ALTERNATES_OFF = 5,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_LOWER_CASE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DEFAULT_LOWER_CASE     = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_LOWER_CASE_SMALL_CAPS  = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_LOWER_CASE_PETITE_CAPS = 2,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_UPPER_CASE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DEFAULT_UPPER_CASE     = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_UPPER_CASE_SMALL_CAPS  = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_UPPER_CASE_PETITE_CAPS = 2,

	/* Selectors for #HB_AAT_LAYOUT_FEATURE_TYPE_CJK_ROMAN_SPACING_TYPE */
	HB_AAT_LAYOUT_FEATURE_SELECTOR_HALF_WIDTH_CJK_ROMAN   = 0,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_PROPORTIONAL_CJK_ROMAN = 1,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_DEFAULT_CJK_ROMAN      = 2,
	HB_AAT_LAYOUT_FEATURE_SELECTOR_FULL_WIDTH_CJK_ROMAN   = 3,

	_HB_AAT_LAYOUT_FEATURE_SELECTOR_MAX_VALUE = HB_TAG_MAX_SIGNED /*< skip >*/
} hb_aat_layout_feature_selector_t;

HB_EXTERN uint hb_aat_layout_get_feature_types(hb_face_t * face,
    uint start_offset,
    uint * feature_count,                             /*IN/OUT May be NULL*/
    hb_aat_layout_feature_type_t * features /*OUT May be NULL*/);
HB_EXTERN hb_ot_name_id_t hb_aat_layout_feature_type_get_name_id(hb_face_t * face, hb_aat_layout_feature_type_t feature_type);

typedef struct hb_aat_layout_feature_selector_info_t {
	hb_ot_name_id_t name_id;
	hb_aat_layout_feature_selector_t enable;
	hb_aat_layout_feature_selector_t disable;
	/*< private >*/
	uint reserved;
} hb_aat_layout_feature_selector_info_t;

#define HB_AAT_LAYOUT_NO_SELECTOR_INDEX         0xFFFFu

HB_EXTERN uint hb_aat_layout_feature_type_get_selector_infos(hb_face_t * face,
    hb_aat_layout_feature_type_t feature_type, uint start_offset,
    uint * selector_count/*IN/OUT May be NULL*/,
    hb_aat_layout_feature_selector_info_t * selectors/*OUT May be NULL*/, uint * default_index /*OUT May be NULL*/);
/*
 * morx/mort
 */
HB_EXTERN hb_bool_t hb_aat_layout_has_substitution(hb_face_t * face);
/*
 * kerx
 */

HB_EXTERN hb_bool_t hb_aat_layout_has_positioning(hb_face_t * face);

/*
 * trak
 */

HB_EXTERN hb_bool_t hb_aat_layout_has_tracking(hb_face_t * face);

HB_END_DECLS

#endif /* HB_AAT_LAYOUT_H */
