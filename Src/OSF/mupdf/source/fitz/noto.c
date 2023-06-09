//
//
#include "mupdf/fitz.h"
#pragma hdrstop
#include "mupdf/ucdn.h"
/*
        Base 14 PDF fonts from URW.
        Noto fonts from Google.
        Source Han Serif from Adobe for CJK.
        DroidSansFallback from Android for CJK.
        Charis SIL from SIL.

        Define TOFU to only include the Base14 and CJK fonts.

        Define TOFU_CJK_LANG to skip Source Han Serif per-language fonts.
        Define TOFU_CJK_EXT to skip DroidSansFallbackFull (and the above).
        Define TOFU_CJK to skip DroidSansFallback (and the above).

        Define TOFU_NOTO to skip ALL non-CJK noto fonts.
        Define TOFU_SYMBOL to skip symbol fonts.
        Define TOFU_EMOJI to skip emoji/extended symbol font.

        Define TOFU_SIL to skip the SIL fonts (warning: makes EPUB documents ugly).
        Define TOFU_BASE14 to skip the Base 14 fonts (warning: makes PDF unusable).
 */
#ifdef NOTO_SMALL
	#define TOFU_CJK_EXT
	#define TOFU_SYMBOL
	#define TOFU_EMOJI
	#define TOFU_SIL
#endif
#ifdef NO_CJK
	#define TOFU_CJK
#endif
#ifdef TOFU
	#define TOFU_NOTO
	#define TOFU_SIL
#endif
#ifdef TOFU_NOTO
	#define TOFU_SYMBOL
	#define TOFU_EMOJI
#endif
/* This historic script has an unusually large font (2MB), so we skip it by default. */
#ifndef NOTO_TANGUT
	#define NOTO_TANGUT 0
#endif

#define DECLARE(NAME) \
	extern "C" uchar _binary_ ## NAME[];\
	extern "C" uint _binary_ ## NAME ## _size;

#ifdef HAVE_OBJCOPY
#define RETURN(FORGE, NAME) \
	do { \
		extern uchar _binary_resources_fonts_ ## FORGE ## _ ## NAME ## _start; \
		extern uchar _binary_resources_fonts_ ## FORGE ## _ ## NAME ## _end; \
		return * size = \
			   &_binary_resources_fonts_ ## FORGE ## _ ## NAME ## _end - \
			   &_binary_resources_fonts_ ## FORGE ## _ ## NAME ## _start, \
		       &_binary_resources_fonts_ ## FORGE ## _ ## NAME ## _start; \
	} while(0)
#else
#define RETURN(FORGE, NAME) \
	/*extern "C" uchar _binary_ ## NAME[];*/\
	/*extern "C" uint _binary_ ## NAME ## _size;*/\
	do { \
		/*extern uchar _binary_ ## NAME[]; */\
		/*extern uint _binary_ ## NAME ## _size; */\
		return * size = _binary_ ## NAME ## _size, _binary_ ## NAME; \
	} while(0)
#endif

DECLARE(NimbusMonoPS_Regular_cff);
DECLARE(NimbusMonoPS_Italic_cff);
DECLARE(NimbusMonoPS_Bold_cff);
DECLARE(NimbusMonoPS_BoldItalic_cff);
DECLARE(NimbusSans_Regular_cff);
DECLARE(NimbusSans_Italic_cff);
DECLARE(NimbusSans_Bold_cff);
DECLARE(NimbusSans_BoldItalic_cff);
DECLARE(NimbusRoman_Regular_cff);
DECLARE(NimbusRoman_Italic_cff);
DECLARE(NimbusRoman_Bold_cff);
DECLARE(NimbusRoman_BoldItalic_cff);
DECLARE(StandardSymbolsPS_cff);
DECLARE(Dingbats_cff);
DECLARE(SourceHanSerif_Regular_ttc);
DECLARE(SourceHanSerif_Regular_ttc);
DECLARE(SourceHanSerif_Regular_ttc);
DECLARE(SourceHanSerif_Regular_ttc);
DECLARE(NotoSansAdlam_Regular_otf);
DECLARE(NotoSerifAhom_Regular_otf);
DECLARE(NotoSansAnatolianHieroglyphs_Regular_otf);
DECLARE(NotoSerifArmenian_Regular_otf);
DECLARE(NotoSansAvestan_Regular_otf);
DECLARE(NotoSerifBalinese_Regular_otf);
DECLARE(NotoSansBamum_Regular_otf);
DECLARE(NotoSansBassaVah_Regular_otf);
DECLARE(NotoSansBatak_Regular_otf);
DECLARE(NotoSerifBengali_Regular_ttf);
DECLARE(NotoSansBhaiksuki_Regular_otf);
DECLARE(NotoSansBrahmi_Regular_otf);
DECLARE(NotoSansBuginese_Regular_otf);
DECLARE(NotoSansBuhid_Regular_otf);
DECLARE(NotoSansCanadianAboriginal_Regular_otf);
DECLARE(NotoSansCarian_Regular_otf);
DECLARE(NotoSansCaucasianAlbanian_Regular_otf);
DECLARE(NotoSansChakma_Regular_otf);
DECLARE(NotoSansCham_Regular_otf);
DECLARE(NotoSansCherokee_Regular_otf);
DECLARE(NotoSansCoptic_Regular_otf);
DECLARE(NotoSansCuneiform_Regular_otf);
DECLARE(NotoSansCypriot_Regular_otf);
DECLARE(NotoSansDeseret_Regular_otf);
DECLARE(NotoSerifDevanagari_Regular_ttf);
DECLARE(NotoSansDuployan_Regular_otf);
DECLARE(NotoSansEgyptianHieroglyphs_Regular_otf);
DECLARE(NotoSansElbasan_Regular_otf);
DECLARE(NotoSerifEthiopic_Regular_otf);
DECLARE(NotoSerifGeorgian_Regular_otf);
DECLARE(NotoSansGlagolitic_Regular_otf);
DECLARE(NotoSansGothic_Regular_otf);
DECLARE(NotoSansGrantha_Regular_otf);
DECLARE(NotoSerifGujarati_Regular_otf);
DECLARE(NotoSerifGurmukhi_Regular_otf);
DECLARE(NotoSansHanunoo_Regular_otf);
DECLARE(NotoSansHatran_Regular_otf);
DECLARE(NotoSerifHebrew_Regular_otf);
DECLARE(NotoSansImperialAramaic_Regular_otf);
DECLARE(NotoSansInscriptionalPahlavi_Regular_otf);
DECLARE(NotoSansInscriptionalParthian_Regular_otf);
DECLARE(NotoSansJavanese_Regular_otf);
DECLARE(NotoSansKaithi_Regular_otf);
DECLARE(NotoSerifKannada_Regular_otf);
DECLARE(NotoSansKayahLi_Regular_otf);
DECLARE(NotoSansKharoshthi_Regular_otf);
DECLARE(NotoSerifKhmer_Regular_otf);
DECLARE(NotoSansKhojki_Regular_otf);;
DECLARE(NotoSansKhudawadi_Regular_otf);
DECLARE(NotoSerifLao_Regular_otf);
DECLARE(NotoSansLepcha_Regular_otf);
DECLARE(NotoSansLimbu_Regular_otf);
DECLARE(NotoSansLinearA_Regular_otf);
DECLARE(NotoSansLinearB_Regular_otf);
DECLARE(NotoSansLisu_Regular_otf);
DECLARE(NotoSansLycian_Regular_otf);
DECLARE(NotoSansLydian_Regular_otf);
DECLARE(NotoSansMahajani_Regular_otf);
DECLARE(NotoSerifMalayalam_Regular_ttf);
DECLARE(NotoSansMandaic_Regular_otf);
DECLARE(NotoSansManichaean_Regular_otf);
DECLARE(NotoSansMarchen_Regular_otf);
DECLARE(NotoSansMeeteiMayek_Regular_otf);
DECLARE(NotoSansMendeKikakui_Regular_otf);
DECLARE(NotoSansMeroitic_Regular_otf);
DECLARE(NotoSansMeroitic_Regular_otf);
DECLARE(NotoSansMiao_Regular_otf);
DECLARE(NotoSansModi_Regular_otf);
DECLARE(NotoSansMongolian_Regular_otf);
DECLARE(NotoSansMro_Regular_otf);
DECLARE(NotoSansMultani_Regular_otf);
DECLARE(NotoSerifMyanmar_Regular_otf);
DECLARE(NotoSansNabataean_Regular_otf);
DECLARE(NotoSansNewa_Regular_otf);
DECLARE(NotoSansNewTaiLue_Regular_otf);
DECLARE(NotoSansNKo_Regular_otf);
DECLARE(NotoSansOgham_Regular_otf);
DECLARE(NotoSansOldHungarian_Regular_otf);
DECLARE(NotoSansOldItalic_Regular_otf);
DECLARE(NotoSansOldNorthArabian_Regular_otf);
DECLARE(NotoSansOldPermic_Regular_otf);
DECLARE(NotoSansOldPersian_Regular_otf);
DECLARE(NotoSansOldSouthArabian_Regular_otf);
DECLARE(NotoSansOldTurkic_Regular_otf);
DECLARE(NotoSansOlChiki_Regular_otf);
DECLARE(NotoSansOriya_Regular_ttf);
DECLARE(NotoSansOsage_Regular_otf);
DECLARE(NotoSansOsmanya_Regular_otf);
DECLARE(NotoSansPahawhHmong_Regular_otf);
DECLARE(NotoSansPalmyrene_Regular_otf);
DECLARE(NotoSansPauCinHau_Regular_otf);
DECLARE(NotoSansPhagsPa_Regular_otf);
DECLARE(NotoSansPhoenician_Regular_otf);
DECLARE(NotoSansPsalterPahlavi_Regular_otf);
DECLARE(NotoSansRejang_Regular_otf);
DECLARE(NotoSansRunic_Regular_otf);
DECLARE(NotoSansSamaritan_Regular_otf);
DECLARE(NotoSansSaurashtra_Regular_otf);
DECLARE(NotoSansSharada_Regular_otf);
DECLARE(NotoSansShavian_Regular_otf);
DECLARE(NotoSansSiddham_Regular_otf);
DECLARE(NotoSerifSinhala_Regular_otf);
DECLARE(NotoSansSoraSompeng_Regular_otf);
DECLARE(NotoSansSundanese_Regular_otf);
DECLARE(NotoSansSylotiNagri_Regular_otf);
DECLARE(NotoSansSyriac_Regular_otf);
DECLARE(NotoSansTagalog_Regular_otf);
DECLARE(NotoSansTagbanwa_Regular_otf);
DECLARE(NotoSansTaiLe_Regular_otf);
DECLARE(NotoSansTaiTham_Regular_ttf);
DECLARE(NotoSansTaiViet_Regular_otf);
DECLARE(NotoSansTakri_Regular_otf);
DECLARE(NotoSerifTamil_Regular_otf);
DECLARE(NotoSerifTelugu_Regular_ttf);
DECLARE(NotoSansThaana_Regular_otf);
DECLARE(NotoSerifThai_Regular_otf);
DECLARE(NotoSerifTibetan_Regular_otf);
DECLARE(NotoSansTifinagh_Regular_otf);
DECLARE(NotoSansTirhuta_Regular_otf);
DECLARE(NotoSansUgaritic_Regular_otf);
DECLARE(NotoSansVai_Regular_otf);
DECLARE(NotoSansWarangCiti_Regular_otf);
DECLARE(NotoSansYi_Regular_otf);
DECLARE(NotoSerif_Regular_otf);
DECLARE(CharisSIL_cff);
DECLARE(CharisSIL_Italic_cff);
DECLARE(CharisSIL_Bold_cff);
DECLARE(CharisSIL_BoldItalic_cff);
DECLARE(NotoSansMath_Regular_otf);
DECLARE(NotoMusic_Regular_otf);
DECLARE(NotoSansSymbols_Regular_otf);
DECLARE(NotoSansSymbols2_Regular_otf);
DECLARE(NotoEmoji_Regular_ttf);
DECLARE(NotoNastaliqUrdu_Regular_otf);
DECLARE(NotoNaskhArabic_Regular_ttf);

const uchar * fz_lookup_base14_font(fz_context * ctx, const char * name, int * size)
{
#ifndef TOFU_BASE14
	if(sstreq(name, "Courier")) RETURN(urw, NimbusMonoPS_Regular_cff);
	if(sstreq(name, "Courier-Oblique")) RETURN(urw, NimbusMonoPS_Italic_cff);
	if(sstreq(name, "Courier-Bold")) RETURN(urw, NimbusMonoPS_Bold_cff);
	if(sstreq(name, "Courier-BoldOblique")) RETURN(urw, NimbusMonoPS_BoldItalic_cff);
	if(sstreq(name, "Helvetica")) RETURN(urw, NimbusSans_Regular_cff);
	if(sstreq(name, "Helvetica-Oblique")) RETURN(urw, NimbusSans_Italic_cff);
	if(sstreq(name, "Helvetica-Bold")) RETURN(urw, NimbusSans_Bold_cff);
	if(sstreq(name, "Helvetica-BoldOblique")) RETURN(urw, NimbusSans_BoldItalic_cff);
	if(sstreq(name, "Times-Roman")) RETURN(urw, NimbusRoman_Regular_cff);
	if(sstreq(name, "Times-Italic")) RETURN(urw, NimbusRoman_Italic_cff);
	if(sstreq(name, "Times-Bold")) RETURN(urw, NimbusRoman_Bold_cff);
	if(sstreq(name, "Times-BoldItalic")) RETURN(urw, NimbusRoman_BoldItalic_cff);
	if(sstreq(name, "Symbol")) RETURN(urw, StandardSymbolsPS_cff);
	if(sstreq(name, "ZapfDingbats")) RETURN(urw, Dingbats_cff);
#endif
	*size = 0;
	return NULL;
}

#define FAMILY(F, R, I, B, BI) \
	if(!is_bold) { \
		if(!is_italic) RETURN(F, R); else RETURN(F, I); \
	} else { \
		if(!is_italic) RETURN(F, B); else RETURN(F, BI); \
	}

const uchar * fz_lookup_builtin_font(fz_context * ctx, const char * name, int is_bold, int is_italic, int * size)
{
#ifndef TOFU_BASE14
	if(sstreq(name, "Courier")) {
		FAMILY(urw, NimbusMonoPS_Regular_cff, NimbusMonoPS_Italic_cff, NimbusMonoPS_Bold_cff, NimbusMonoPS_BoldItalic_cff)
	}
	if(sstreq(name, "Helvetica") || sstreq(name, "Arial")) {
		FAMILY(urw, NimbusSans_Regular_cff, NimbusSans_Italic_cff, NimbusSans_Bold_cff, NimbusSans_BoldItalic_cff)
	}
	if(sstreq(name, "Times") || sstreq(name, "Times Roman") || sstreq(name, "Times New Roman")) {
		FAMILY(urw, NimbusRoman_Regular_cff, NimbusRoman_Italic_cff, NimbusRoman_Bold_cff, NimbusRoman_BoldItalic_cff)
	}
	if(sstreq(name, "Dingbats") || sstreq(name, "Zapf Dingbats")) {
		RETURN(urw, Dingbats_cff);
	}
	if(sstreq(name, "Symbol")) {
		RETURN(urw, StandardSymbolsPS_cff);
	}
#endif
#ifndef TOFU_SIL
	if(sstreq(name, "Charis SIL")) {
		FAMILY(sil, CharisSIL_cff, CharisSIL_Italic_cff, CharisSIL_Bold_cff, CharisSIL_BoldItalic_cff)
	}
#endif
#ifndef TOFU_NOTO
	if(sstreq(name, "Noto Serif")) {
		RETURN(noto, NotoSerif_Regular_otf);
	}
#endif
	*size = 0;
	return NULL;
}

const uchar * fz_lookup_cjk_font(fz_context * ctx, int ordering, int * size, int * subfont)
{
	*subfont = 0;
#ifndef TOFU_CJK
#ifndef TOFU_CJK_EXT
#ifndef TOFU_CJK_LANG
	switch(ordering) {
		case FZ_ADOBE_JAPAN: *subfont = 0; RETURN(han, SourceHanSerif_Regular_ttc);
		case FZ_ADOBE_KOREA: *subfont = 1; RETURN(han, SourceHanSerif_Regular_ttc);
		case FZ_ADOBE_GB: *subfont = 2; RETURN(han, SourceHanSerif_Regular_ttc);
		default:
		case FZ_ADOBE_CNS: *subfont = 3; RETURN(han, SourceHanSerif_Regular_ttc);
	}
#else
	RETURN(droid, DroidSansFallbackFull_ttf);
#endif
#else
	RETURN(droid, DroidSansFallback_ttf);
#endif
#else
	return *size = 0, NULL;
#endif
}

int fz_lookup_cjk_ordering_by_language(const char * name)
{
	if(sstreq(name, "zh-Hant")) return FZ_ADOBE_CNS;
	if(sstreq(name, "zh-TW")) return FZ_ADOBE_CNS;
	if(sstreq(name, "zh-HK")) return FZ_ADOBE_CNS;
	if(sstreq(name, "zh-Hans")) return FZ_ADOBE_GB;
	if(sstreq(name, "zh-CN")) return FZ_ADOBE_GB;
	if(sstreq(name, "ja")) return FZ_ADOBE_JAPAN;
	if(sstreq(name, "ko")) return FZ_ADOBE_KOREA;
	return -1;
}

const uchar * fz_lookup_cjk_font_by_language(fz_context * ctx, const char * lang, int * size, int * subfont)
{
	int ordering = fz_lookup_cjk_ordering_by_language(lang);
	if(ordering >= 0)
		return fz_lookup_cjk_font(ctx, ordering, size, subfont);
	*size = 0;
	*subfont = 0;
	return NULL;
}

const uchar * fz_lookup_noto_font(fz_context * ctx, int script, int language, int * size, int * subfont)
{
	*subfont = 0;
	switch(script) {
		default:
		case UCDN_SCRIPT_COMMON:
		case UCDN_SCRIPT_INHERITED:
		case UCDN_SCRIPT_UNKNOWN:
#ifndef TOFU_NOTO
		    RETURN(noto, NotoSerif_Regular_otf);
#endif
		    break;

		case UCDN_SCRIPT_HANGUL:
		    return fz_lookup_cjk_font(ctx, FZ_ADOBE_KOREA, size, subfont);
		case UCDN_SCRIPT_HIRAGANA:
		case UCDN_SCRIPT_KATAKANA:
		    return fz_lookup_cjk_font(ctx, FZ_ADOBE_JAPAN, size, subfont);
		case UCDN_SCRIPT_BOPOMOFO:
		    return fz_lookup_cjk_font(ctx, FZ_ADOBE_CNS, size, subfont);
		case UCDN_SCRIPT_HAN:
		    switch(language)
		    {
			    case FZ_LANG_ja: return fz_lookup_cjk_font(ctx, FZ_ADOBE_JAPAN, size, subfont);
			    case FZ_LANG_ko: return fz_lookup_cjk_font(ctx, FZ_ADOBE_KOREA, size, subfont);
			    case FZ_LANG_zh_Hans: return fz_lookup_cjk_font(ctx, FZ_ADOBE_GB, size, subfont);
			    default:
			    case FZ_LANG_zh_Hant: return fz_lookup_cjk_font(ctx, FZ_ADOBE_CNS, size, subfont);
		    }

		case UCDN_SCRIPT_BRAILLE: break; /* no dedicated font; fallback to NotoSansSymbols will cover this */

#ifndef TOFU_NOTO
		case UCDN_SCRIPT_LATIN:
		case UCDN_SCRIPT_GREEK:
		case UCDN_SCRIPT_CYRILLIC:
		    RETURN(noto, NotoSerif_Regular_otf);

		case UCDN_SCRIPT_ARABIC:
		    if(language == FZ_LANG_ur || language == FZ_LANG_urd)
			    RETURN(noto, NotoNastaliqUrdu_Regular_otf);
		    RETURN(noto, NotoNaskhArabic_Regular_ttf);

		case UCDN_SCRIPT_ADLAM: RETURN(noto, NotoSansAdlam_Regular_otf);
		case UCDN_SCRIPT_AHOM: RETURN(noto, NotoSerifAhom_Regular_otf);
		case UCDN_SCRIPT_ANATOLIAN_HIEROGLYPHS: RETURN(noto, NotoSansAnatolianHieroglyphs_Regular_otf);
		case UCDN_SCRIPT_ARMENIAN: RETURN(noto, NotoSerifArmenian_Regular_otf);
		case UCDN_SCRIPT_AVESTAN: RETURN(noto, NotoSansAvestan_Regular_otf);
		case UCDN_SCRIPT_BALINESE: RETURN(noto, NotoSerifBalinese_Regular_otf);
		case UCDN_SCRIPT_BAMUM: RETURN(noto, NotoSansBamum_Regular_otf);
		case UCDN_SCRIPT_BASSA_VAH: RETURN(noto, NotoSansBassaVah_Regular_otf);
		case UCDN_SCRIPT_BATAK: RETURN(noto, NotoSansBatak_Regular_otf);
		case UCDN_SCRIPT_BENGALI: RETURN(noto, NotoSerifBengali_Regular_ttf);
		case UCDN_SCRIPT_BHAIKSUKI: RETURN(noto, NotoSansBhaiksuki_Regular_otf);
		case UCDN_SCRIPT_BRAHMI: RETURN(noto, NotoSansBrahmi_Regular_otf);
		case UCDN_SCRIPT_BUGINESE: RETURN(noto, NotoSansBuginese_Regular_otf);
		case UCDN_SCRIPT_BUHID: RETURN(noto, NotoSansBuhid_Regular_otf);
		case UCDN_SCRIPT_CANADIAN_ABORIGINAL: RETURN(noto, NotoSansCanadianAboriginal_Regular_otf);
		case UCDN_SCRIPT_CARIAN: RETURN(noto, NotoSansCarian_Regular_otf);
		case UCDN_SCRIPT_CAUCASIAN_ALBANIAN: RETURN(noto, NotoSansCaucasianAlbanian_Regular_otf);
		case UCDN_SCRIPT_CHAKMA: RETURN(noto, NotoSansChakma_Regular_otf);
		case UCDN_SCRIPT_CHAM: RETURN(noto, NotoSansCham_Regular_otf);
		case UCDN_SCRIPT_CHEROKEE: RETURN(noto, NotoSansCherokee_Regular_otf);
		case UCDN_SCRIPT_COPTIC: RETURN(noto, NotoSansCoptic_Regular_otf);
		case UCDN_SCRIPT_CUNEIFORM: RETURN(noto, NotoSansCuneiform_Regular_otf);
		case UCDN_SCRIPT_CYPRIOT: RETURN(noto, NotoSansCypriot_Regular_otf);
		case UCDN_SCRIPT_DESERET: RETURN(noto, NotoSansDeseret_Regular_otf);
		case UCDN_SCRIPT_DEVANAGARI: RETURN(noto, NotoSerifDevanagari_Regular_ttf);
		case UCDN_SCRIPT_DOGRA: break;
		case UCDN_SCRIPT_DUPLOYAN: RETURN(noto, NotoSansDuployan_Regular_otf);
		case UCDN_SCRIPT_EGYPTIAN_HIEROGLYPHS: RETURN(noto, NotoSansEgyptianHieroglyphs_Regular_otf);
		case UCDN_SCRIPT_ELBASAN: RETURN(noto, NotoSansElbasan_Regular_otf);
		case UCDN_SCRIPT_ELYMAIC: break;
		case UCDN_SCRIPT_ETHIOPIC: RETURN(noto, NotoSerifEthiopic_Regular_otf);
		case UCDN_SCRIPT_GEORGIAN: RETURN(noto, NotoSerifGeorgian_Regular_otf);
		case UCDN_SCRIPT_GLAGOLITIC: RETURN(noto, NotoSansGlagolitic_Regular_otf);
		case UCDN_SCRIPT_GOTHIC: RETURN(noto, NotoSansGothic_Regular_otf);
		case UCDN_SCRIPT_GRANTHA: RETURN(noto, NotoSansGrantha_Regular_otf);
		case UCDN_SCRIPT_GUJARATI: RETURN(noto, NotoSerifGujarati_Regular_otf);
		case UCDN_SCRIPT_GUNJALA_GONDI: break;
		case UCDN_SCRIPT_GURMUKHI: RETURN(noto, NotoSerifGurmukhi_Regular_otf);
		case UCDN_SCRIPT_HANIFI_ROHINGYA: break;
		case UCDN_SCRIPT_HANUNOO: RETURN(noto, NotoSansHanunoo_Regular_otf);
		case UCDN_SCRIPT_HATRAN: RETURN(noto, NotoSansHatran_Regular_otf);
		case UCDN_SCRIPT_HEBREW: RETURN(noto, NotoSerifHebrew_Regular_otf);
		case UCDN_SCRIPT_IMPERIAL_ARAMAIC: RETURN(noto, NotoSansImperialAramaic_Regular_otf);
		case UCDN_SCRIPT_INSCRIPTIONAL_PAHLAVI: RETURN(noto, NotoSansInscriptionalPahlavi_Regular_otf);
		case UCDN_SCRIPT_INSCRIPTIONAL_PARTHIAN: RETURN(noto, NotoSansInscriptionalParthian_Regular_otf);
		case UCDN_SCRIPT_JAVANESE: RETURN(noto, NotoSansJavanese_Regular_otf);
		case UCDN_SCRIPT_KAITHI: RETURN(noto, NotoSansKaithi_Regular_otf);
		case UCDN_SCRIPT_KANNADA: RETURN(noto, NotoSerifKannada_Regular_otf);
		case UCDN_SCRIPT_KAYAH_LI: RETURN(noto, NotoSansKayahLi_Regular_otf);
		case UCDN_SCRIPT_KHAROSHTHI: RETURN(noto, NotoSansKharoshthi_Regular_otf);
		case UCDN_SCRIPT_KHMER: RETURN(noto, NotoSerifKhmer_Regular_otf);
		case UCDN_SCRIPT_KHOJKI: RETURN(noto, NotoSansKhojki_Regular_otf);;
		case UCDN_SCRIPT_KHUDAWADI: RETURN(noto, NotoSansKhudawadi_Regular_otf);
		case UCDN_SCRIPT_LAO: RETURN(noto, NotoSerifLao_Regular_otf);
		case UCDN_SCRIPT_LEPCHA: RETURN(noto, NotoSansLepcha_Regular_otf);
		case UCDN_SCRIPT_LIMBU: RETURN(noto, NotoSansLimbu_Regular_otf);
		case UCDN_SCRIPT_LINEAR_A: RETURN(noto, NotoSansLinearA_Regular_otf);
		case UCDN_SCRIPT_LINEAR_B: RETURN(noto, NotoSansLinearB_Regular_otf);
		case UCDN_SCRIPT_LISU: RETURN(noto, NotoSansLisu_Regular_otf);
		case UCDN_SCRIPT_LYCIAN: RETURN(noto, NotoSansLycian_Regular_otf);
		case UCDN_SCRIPT_LYDIAN: RETURN(noto, NotoSansLydian_Regular_otf);
		case UCDN_SCRIPT_MAHAJANI: RETURN(noto, NotoSansMahajani_Regular_otf);
		case UCDN_SCRIPT_MAKASAR: break;
		case UCDN_SCRIPT_MALAYALAM: RETURN(noto, NotoSerifMalayalam_Regular_ttf);
		case UCDN_SCRIPT_MANDAIC: RETURN(noto, NotoSansMandaic_Regular_otf);
		case UCDN_SCRIPT_MANICHAEAN: RETURN(noto, NotoSansManichaean_Regular_otf);
		case UCDN_SCRIPT_MARCHEN: RETURN(noto, NotoSansMarchen_Regular_otf);
		case UCDN_SCRIPT_MASARAM_GONDI: break;
		case UCDN_SCRIPT_MEDEFAIDRIN: break;
		case UCDN_SCRIPT_MEETEI_MAYEK: RETURN(noto, NotoSansMeeteiMayek_Regular_otf);
		case UCDN_SCRIPT_MENDE_KIKAKUI: RETURN(noto, NotoSansMendeKikakui_Regular_otf);
		case UCDN_SCRIPT_MEROITIC_CURSIVE: RETURN(noto, NotoSansMeroitic_Regular_otf);
		case UCDN_SCRIPT_MEROITIC_HIEROGLYPHS: RETURN(noto, NotoSansMeroitic_Regular_otf);
		case UCDN_SCRIPT_MIAO: RETURN(noto, NotoSansMiao_Regular_otf);
		case UCDN_SCRIPT_MODI: RETURN(noto, NotoSansModi_Regular_otf);
		case UCDN_SCRIPT_MONGOLIAN: RETURN(noto, NotoSansMongolian_Regular_otf);
		case UCDN_SCRIPT_MRO: RETURN(noto, NotoSansMro_Regular_otf);
		case UCDN_SCRIPT_MULTANI: RETURN(noto, NotoSansMultani_Regular_otf);
		case UCDN_SCRIPT_MYANMAR: RETURN(noto, NotoSerifMyanmar_Regular_otf);
		case UCDN_SCRIPT_NABATAEAN: RETURN(noto, NotoSansNabataean_Regular_otf);
		case UCDN_SCRIPT_NANDINAGARI: break;
		case UCDN_SCRIPT_NEWA: RETURN(noto, NotoSansNewa_Regular_otf);
		case UCDN_SCRIPT_NEW_TAI_LUE: RETURN(noto, NotoSansNewTaiLue_Regular_otf);
		case UCDN_SCRIPT_NKO: RETURN(noto, NotoSansNKo_Regular_otf);
		case UCDN_SCRIPT_NUSHU: break;
		case UCDN_SCRIPT_NYIAKENG_PUACHUE_HMONG: break;
		case UCDN_SCRIPT_OGHAM: RETURN(noto, NotoSansOgham_Regular_otf);
		case UCDN_SCRIPT_OLD_HUNGARIAN: RETURN(noto, NotoSansOldHungarian_Regular_otf);
		case UCDN_SCRIPT_OLD_ITALIC: RETURN(noto, NotoSansOldItalic_Regular_otf);
		case UCDN_SCRIPT_OLD_NORTH_ARABIAN: RETURN(noto, NotoSansOldNorthArabian_Regular_otf);
		case UCDN_SCRIPT_OLD_PERMIC: RETURN(noto, NotoSansOldPermic_Regular_otf);
		case UCDN_SCRIPT_OLD_PERSIAN: RETURN(noto, NotoSansOldPersian_Regular_otf);
		case UCDN_SCRIPT_OLD_SOGDIAN: break;
		case UCDN_SCRIPT_OLD_SOUTH_ARABIAN: RETURN(noto, NotoSansOldSouthArabian_Regular_otf);
		case UCDN_SCRIPT_OLD_TURKIC: RETURN(noto, NotoSansOldTurkic_Regular_otf);
		case UCDN_SCRIPT_OL_CHIKI: RETURN(noto, NotoSansOlChiki_Regular_otf);
		case UCDN_SCRIPT_ORIYA: RETURN(noto, NotoSansOriya_Regular_ttf);
		case UCDN_SCRIPT_OSAGE: RETURN(noto, NotoSansOsage_Regular_otf);
		case UCDN_SCRIPT_OSMANYA: RETURN(noto, NotoSansOsmanya_Regular_otf);
		case UCDN_SCRIPT_PAHAWH_HMONG: RETURN(noto, NotoSansPahawhHmong_Regular_otf);
		case UCDN_SCRIPT_PALMYRENE: RETURN(noto, NotoSansPalmyrene_Regular_otf);
		case UCDN_SCRIPT_PAU_CIN_HAU: RETURN(noto, NotoSansPauCinHau_Regular_otf);
		case UCDN_SCRIPT_PHAGS_PA: RETURN(noto, NotoSansPhagsPa_Regular_otf);
		case UCDN_SCRIPT_PHOENICIAN: RETURN(noto, NotoSansPhoenician_Regular_otf);
		case UCDN_SCRIPT_PSALTER_PAHLAVI: RETURN(noto, NotoSansPsalterPahlavi_Regular_otf);
		case UCDN_SCRIPT_REJANG: RETURN(noto, NotoSansRejang_Regular_otf);
		case UCDN_SCRIPT_RUNIC: RETURN(noto, NotoSansRunic_Regular_otf);
		case UCDN_SCRIPT_SAMARITAN: RETURN(noto, NotoSansSamaritan_Regular_otf);
		case UCDN_SCRIPT_SAURASHTRA: RETURN(noto, NotoSansSaurashtra_Regular_otf);
		case UCDN_SCRIPT_SHARADA: RETURN(noto, NotoSansSharada_Regular_otf);
		case UCDN_SCRIPT_SHAVIAN: RETURN(noto, NotoSansShavian_Regular_otf);
		case UCDN_SCRIPT_SIDDHAM: RETURN(noto, NotoSansSiddham_Regular_otf);
		case UCDN_SCRIPT_SIGNWRITING: break;
		case UCDN_SCRIPT_SINHALA: RETURN(noto, NotoSerifSinhala_Regular_otf);
		case UCDN_SCRIPT_SOGDIAN: break;
		case UCDN_SCRIPT_SORA_SOMPENG: RETURN(noto, NotoSansSoraSompeng_Regular_otf);
		case UCDN_SCRIPT_SOYOMBO: break;
		case UCDN_SCRIPT_SUNDANESE: RETURN(noto, NotoSansSundanese_Regular_otf);
		case UCDN_SCRIPT_SYLOTI_NAGRI: RETURN(noto, NotoSansSylotiNagri_Regular_otf);
		case UCDN_SCRIPT_SYRIAC: RETURN(noto, NotoSansSyriac_Regular_otf);
		case UCDN_SCRIPT_TAGALOG: RETURN(noto, NotoSansTagalog_Regular_otf);
		case UCDN_SCRIPT_TAGBANWA: RETURN(noto, NotoSansTagbanwa_Regular_otf);
		case UCDN_SCRIPT_TAI_LE: RETURN(noto, NotoSansTaiLe_Regular_otf);
		case UCDN_SCRIPT_TAI_THAM: RETURN(noto, NotoSansTaiTham_Regular_ttf);
		case UCDN_SCRIPT_TAI_VIET: RETURN(noto, NotoSansTaiViet_Regular_otf);
		case UCDN_SCRIPT_TAKRI: RETURN(noto, NotoSansTakri_Regular_otf);
		case UCDN_SCRIPT_TAMIL: RETURN(noto, NotoSerifTamil_Regular_otf);
		case UCDN_SCRIPT_TELUGU: RETURN(noto, NotoSerifTelugu_Regular_ttf);
		case UCDN_SCRIPT_THAANA: RETURN(noto, NotoSansThaana_Regular_otf);
		case UCDN_SCRIPT_THAI: RETURN(noto, NotoSerifThai_Regular_otf);
		case UCDN_SCRIPT_TIBETAN: RETURN(noto, NotoSerifTibetan_Regular_otf);
		case UCDN_SCRIPT_TIFINAGH: RETURN(noto, NotoSansTifinagh_Regular_otf);
		case UCDN_SCRIPT_TIRHUTA: RETURN(noto, NotoSansTirhuta_Regular_otf);
		case UCDN_SCRIPT_UGARITIC: RETURN(noto, NotoSansUgaritic_Regular_otf);
		case UCDN_SCRIPT_VAI: RETURN(noto, NotoSansVai_Regular_otf);
		case UCDN_SCRIPT_WANCHO: break;
		case UCDN_SCRIPT_WARANG_CITI: RETURN(noto, NotoSansWarangCiti_Regular_otf);
		case UCDN_SCRIPT_YI: RETURN(noto, NotoSansYi_Regular_otf);
		case UCDN_SCRIPT_ZANABAZAR_SQUARE: break;

#if NOTO_TANGUT
		case UCDN_SCRIPT_TANGUT: RETURN(noto, NotoSerifTangut_Regular_otf);
#endif

#endif /* TOFU_NOTO */
	}
	*size = 0;
	return NULL;
}

const uchar * fz_lookup_noto_math_font(fz_context * ctx, int * size)
{
#ifndef TOFU_SYMBOL
	RETURN(noto, NotoSansMath_Regular_otf);
#else
	return *size = 0, NULL;
#endif
}

const uchar * fz_lookup_noto_music_font(fz_context * ctx, int * size)
{
#ifndef TOFU_SYMBOL
	RETURN(noto, NotoMusic_Regular_otf);
#else
	return *size = 0, NULL;
#endif
}

const uchar * fz_lookup_noto_symbol1_font(fz_context * ctx, int * size)
{
#ifndef TOFU_SYMBOL
	RETURN(noto, NotoSansSymbols_Regular_otf);
#else
	return *size = 0, NULL;
#endif
}

const uchar * fz_lookup_noto_symbol2_font(fz_context * ctx, int * size)
{
#ifndef TOFU_SYMBOL
	RETURN(noto, NotoSansSymbols2_Regular_otf);
#else
	return *size = 0, NULL;
#endif
}

const uchar * fz_lookup_noto_emoji_font(fz_context * ctx, int * size)
{
#ifndef TOFU_EMOJI
	RETURN(noto, NotoEmoji_Regular_ttf);
#else
	return *size = 0, NULL;
#endif
}
