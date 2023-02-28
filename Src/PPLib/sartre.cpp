// SARTR.CPP
// Copyright (c) A.Sobolev 2011, 2012, 2013, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023
// @codepage windows-1251
//
#include <pp.h>
#pragma hdrstop
#if(_MSC_VER >= 1900)
	#include <cmath>
	#include <unicode\uclean.h>
	#include <unicode\brkiter.h>
	#include <unicode\measunit.h>
	#include <unicode\measfmt.h>
	#include <unicode\unumberformatter.h>
#endif
#include <sartre.h>
#include <locale.h>
//
//
/*
// @stub
int CallbackCompress(long a, long b, const char * c, int stop) { return 1; }
*/
//
//
//
SrSList::SrSList(int type) : Type(type), Len(0)
{
	SBaseBuffer::Init();
	Alloc(32);
	ASSIGN_PTR(P_Buf, 0);
}

SrSList::~SrSList()
{
	SBaseBuffer::Destroy();
}

int FASTCALL SrSList::Copy(const SrSList & rS)
{
	SBaseBuffer::Copy(rS);
	Type = rS.Type;
	Len = rS.Len;
	return 1;
}

/*virtual*/void SrSList::Clear()
{
	Len = 0;
	ASSIGN_PTR(P_Buf, 0);
}

/*virtual*/int SrSList::IsEq(const SrSList & rS) const
{
	if(Type != rS.Type || Len != rS.Len)
		return 0;
	else if(Len && memcmp(P_Buf, rS.P_Buf, Len) != 0)
		return 0;
	else
		return 1;
}

/*virtual*/int SrSList::Validate() const
{
	return 1;
}

int SrSList::Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx)
{
	int    ok = 1;
	if(dir < 0)
		Clear();
	THROW_SL(pCtx->Serialize(dir, Type, rBuf));
	THROW_SL(pCtx->Serialize(dir, Len, rBuf));
	if(dir < 0) {
		THROW_SL(Alloc(Len));
	}
	THROW_SL(pCtx->SerializeBlock(dir, Len, P_Buf, rBuf, 0));
	CATCHZOK
	return ok;
}
//
//
//
SrWordForm::SrWordForm() : SrSList(SRGRAMTYP_WORDFORM)
{
}

SrWordForm::SrWordForm(const SrWordForm & rS) : SrSList(SRGRAMTYP_WORDFORM)
{
	Copy(rS);
}

SrWordForm & FASTCALL SrWordForm::operator = (const SrWordForm & rS)
{
	return Copy(rS);
}

SrWordForm & FASTCALL SrWordForm::Copy(const SrWordForm & rS)
{
	SrSList::Copy(rS);
	return *this;
}

SrWordForm & SrWordForm::Merge_(const SrWordForm & rBase, const SrWordForm & rVar, int mode)
{
	SrSList::Copy(rBase);
	for(size_t p = 0; rVar.P_Buf[p]; p = rVar.Step(p))
		SetTag(rVar.Tag(p), rVar.Get(p), mode);
	Normalize();
	return *this;
}

int SrWordForm::Normalize()
{
	int    ok = 1;
	if(P_Buf) {
		LAssocArray list;
		for(size_t p = 0; P_Buf[p]; p = Step(p)) {
			list.Add(Tag(p), Get(p), 0);
		}
		list.Sort();
		Clear();
		for(uint i = 0; i < list.getCount(); i++) {
			SetTag(list.at(i).Key, list.at(i).Val);
		}
	}
	return ok;
}

int FASTCALL SrWordForm::IsSubsetOf(const SrWordForm & rS) const
{
	int    r = 0;
	if(Len <= rS.Len) {
		LAssocArray list2;
		size_t p;
		for(p = 0; rS.P_Buf[p]; p = rS.Step(p))
			list2.Add(rS.Tag(p), rS.Get(p), 0);
		r = 1;
		for(p = 0; r && P_Buf[p]; p = Step(p)) {
			if(!list2.SearchPair(Tag(p), Get(p), 0))
				r = 0;
		}
	}
	return r;
}

int FASTCALL SrWordForm::IsEq(const SrWordForm & rS) const
{
	int    r = 0;
	if(Len == rS.Len) {
		LAssocArray list1, list2;
		size_t p;
		for(p = 0; P_Buf[p]; p = Step(p)) {
			list1.Add(Tag(p), Get(p), 0);
		}
		list1.Sort();

		for(p = 0; rS.P_Buf[p]; p = rS.Step(p)) {
			list2.Add(rS.Tag(p), rS.Get(p), 0);
		}
		list2.Sort();
		r = BIN(list1 == list2);
	}
	return r;
}

double FASTCALL SrWordForm::MatchScore(const SrWordForm & rS) const
{
	double score = 0.0;
	for(size_t p = 0; P_Buf[p]; p = Step(p)) {
		const int tag = Tag(p);
		const int s_val = rS.GetTag(tag);
		if(s_val) {
			const int this_val = Get(p);
			if(s_val == this_val)
				score += 1.0;
			else
				score += 0.01;
		}
		else
			score += 0.0;
	}
	return score;
}

int FASTCALL SrWordForm::RemoveTag(int tag)
{
	int    ok = -1;
	if(P_Buf) {
		for(size_t p = 0; P_Buf[p];) {
			size_t next = Step(p);
			if(Tag(p) == tag) {
				memmove(P_Buf+p, P_Buf+next, Len-next);
				Len -= (next - p);
				ok = 1;
			}
			else
				p = next;
		}
	}
	CalcLength(); // @debug
	return ok;
}

int SrWordForm::SetTag(int tag, int val, int mode)
{
	int    ok = -1;
	int    found = 0;
	size_t p = 0;
	if(P_Buf) {
		for(; P_Buf[p]; p = Step(p)) {
			if(Tag(p) == tag) {
				if(oneof2(mode, 0, 1)) {
					Set(p, val);
					ok = 1;
				}
				found = 1;
			}
		}
	}
	if(!found && oneof2(mode, 0, 2)) {
		size_t new_size = p + (oneof2(tag, SRWG_LANGUAGE, SRWG_CLASS) ? 3 : 2) + 1;
		Alloc(new_size);
		PTR8(P_Buf+p)[0] = (uint8)tag;
		p += Set(p, val);
		PTR8(P_Buf+p)[0] = 0;
		Len = new_size;
		ok = 2;
	}
	CalcLength(); // @debug
	return ok;
}

uint SrWordForm::GetTagCount() const
{
	uint   c = 0;
	if(P_Buf)
		for(size_t p = 0; P_Buf[p]; p = Step(p))
			c++;
	return c;
}

int FASTCALL SrWordForm::GetTagByIdx(uint idx, int * pTag, int * pVal) const
{
	int    ok = 0;
	int    tag = 0;
	int    val = 0;
	if(P_Buf) {
		uint   c = 0;
		for(size_t p = 0; !ok && P_Buf[p]; p = Step(p)) {
			if(c++ == idx) {
				tag = Tag(p);
				val = Get(p);
				ok = 1;
			}
		}
	}
	ASSIGN_PTR(pTag, tag);
	ASSIGN_PTR(pVal, val);
	return ok;
}

int FASTCALL SrWordForm::GetTag(int tag) const
{
	int    ret = 0;
	if(P_Buf) {
		for(size_t p = 0; !ret && P_Buf[p]; p = Step(p)) {
			if(Tag(p) == tag)
				ret = Get(p);
		}
	}
	return ret;
}

size_t SrWordForm::CalcLength() const
{
	size_t p = 0;
	if(P_Buf) {
		while(P_Buf[p])
			p = Step(p);
		p++;
	}
	assert(p == Len);
	return p;
}

void SrWordForm::CatTok(SString & rBuf, const char * pTok) const
{
	rBuf.CatDivIfNotEmpty(0, 1).Cat(pTok);
}

size_t SrWordForm::Set(size_t pos, int val)
{
	size_t inc = sizeof(uint8);
	if(oneof2(Tag(pos), SRWG_LANGUAGE, SRWG_CLASS)) {
		PTR16(P_Buf+pos+1)[0] = (uint16)val;
		inc += sizeof(uint16);
	}
	else {
		PTR8(P_Buf+pos+1)[0] = (uint8)val;
		inc += sizeof(uint8);
	}
	return inc;
}

int FASTCALL SrWordForm::Tag(size_t pos) const
	{ return (int)(PTR8(P_Buf+pos)[0]); }
int FASTCALL SrWordForm::Get(size_t pos) const
	{ return oneof2(Tag(pos), SRWG_LANGUAGE, SRWG_CLASS) ? (int)PTR16(P_Buf+pos+1)[0] : (int)PTR8(P_Buf+pos+1)[0]; }
size_t FASTCALL SrWordForm::Step(size_t pos) const
	{ return pos + sizeof(uint8) + (oneof2(Tag(pos), SRWG_LANGUAGE, SRWG_CLASS) ? sizeof(uint16) : sizeof(uint8)); }

struct SrWfToken {
	int16  Tag;
	int16  Val;
	const char * Tok;
};

static SrWfToken __Tokens[] = {
	{ SRWG_LANGUAGE, slangMeta,          "META" },
	{ SRWG_LANGUAGE, slangLA,            "LAT"  },
	{ SRWG_LANGUAGE, slangEN,            "ENG"  },
	{ SRWG_LANGUAGE, slangRU,            "RUS"  },
	{ SRWG_LANGUAGE, slangDE,            "GER"  },
	{ SRWG_LANGUAGE, slangES,            "SPA"  },
	{ SRWG_LANGUAGE, slangJA,            "JPN"  },
	{ SRWG_LANGUAGE, slangZH,            "CHI"  },
	{ SRWG_LANGUAGE, slangKO,            "KOR"  },

	{ SRWG_CLASS,  SRWC_ALPHA,           "ALPHA" },
	{ SRWG_CLASS,  SRWC_DIGIT,           "DIGIT" },
	{ SRWG_CLASS,  SRWC_NOUN,            "NOUN" },
	{ SRWG_CLASS,  SRWC_NUMERAL,         "NUM" },
	{ SRWG_CLASS,  SRWC_NUMERALORD,      "ORD" },
	{ SRWG_CLASS,  SRWC_ADJECTIVE,       "ADJ" },
	{ SRWG_CLASS,  SRWC_VERB,            "VERB" },
	// @v9.2.0 { SRWG_CLASS,  SRWC_VERBMODAL,       "VERBMODAL" },
	{ SRWG_CLASS,  SRWC_PRONOUN,         "PRONOUN" },
	{ SRWG_CLASS,  SRWC_PRONOUNPOSS,     "PRONOUNPOSS" },
	{ SRWG_CLASS,  SRWC_PRAEDICPRO,      "PRAEDICPRO"  },
	{ SRWG_CLASS,  SRWC_PRAEDIC,         "PRAEDIC" },
	{ SRWG_CLASS,  SRWC_ADVERB,          "ADVERB" },
	{ SRWG_CLASS,  SRWC_PREPOSITION,     "PREPOS" },
	{ SRWG_CLASS,  SRWC_POSTPOSITION,    "POSTPOS" },
	{ SRWG_CLASS,  SRWC_CONJUNCTION,     "CONJ" },
	{ SRWG_CLASS,  SRWC_INTERJECTION,    "INTERJ" },
	{ SRWG_CLASS,  SRWC_PARENTH,         "PARENTH" },
	{ SRWG_CLASS,  SRWC_ARTICLE,         "ART" },
	{ SRWG_CLASS,  SRWC_PARTICLE,        "PART" },
	{ SRWG_CLASS,  SRWC_PARTICIPLE,         "PARTCP" },
	{ SRWG_CLASS,  SRWC_GERUND,             "GERUND" },
	{ SRWG_CLASS,  SRWC_GPARTICIPLE,        "GPARTCP" },
	{ SRWG_CLASS,  SRWC_PHRAS,              "PHRAS" },
	{ SRWG_CLASS,  SRWC_PREFIX,             "PREFIX" },
	{ SRWG_CLASS,  SRWC_AFFIX,              "AFFIX" },
	{ SRWG_CLASS,  SRWC_SKELETON,           "SKEL" },
	// { SRWG_CLASS,  SRWC_PRONOUNREFL,        "PRONOUNREFL" }, // @v9.2.0 ���������� ����������� (reflexive pronoun)
	{ SRWG_CLASS,  SRWC_POSSESSIVEGROUP,    "POSSGRP" }, // @v9.2.0 possessive group (english)

	{ SRWG_PROPERNAME, SRPROPN_PERSONNAME,  "PERSN" },
	{ SRWG_PROPERNAME, SRPROPN_FAMILYNAME,  "FAMN"  },
	{ SRWG_PROPERNAME, SRPROPN_PATRONYMIC,  "PATRN" },
	{ SRWG_PROPERNAME, SRPROPN_ZOONAME,     "ZOON"  },
	{ SRWG_PROPERNAME, SRPROPN_ORG,         "ORG"   },
	{ SRWG_PROPERNAME, SRPROPN_GEO,         "GEO"   }, // @v9.2.0

	{ SRWG_ABBR,       SRABBR_ABBR,         "ABBR"  },
	{ SRWG_ABBR,       SRABBR_NARROW,       "NARR"  },

	{ SRWG_USAGE,   SRWU_LITERARY,          "LIT" },
	{ SRWG_USAGE,   SRWU_PRO,               "PRO" },
	{ SRWG_USAGE,   SRWU_ARCHAIC,           "ARC" },
	{ SRWG_USAGE,   SRWU_SPOKEN,            "SPK" },
	{ SRWG_USAGE,   SRWU_VULGAR,            "VUL" },

	{ SRWG_GENDER,  SRGENDER_MASCULINE,     "MASC" },
	{ SRWG_GENDER,  SRGENDER_FEMININE,      "FEM"  },
	{ SRWG_GENDER,  SRGENDER_NEUTER,        "NEU"  },
	{ SRWG_GENDER,  SRGENDER_COMMON,        "GCOM" },

	{ SRWG_TANTUM,  SRTANTUM_SINGULAR,      "SINGT" },
	{ SRWG_TANTUM,  SRTANTUM_PLURAL,        "PLURT" },

	{ SRWG_COUNT,   SRCNT_SINGULAR,         "SING" },
	{ SRWG_COUNT,   SRCNT_PLURAL,           "PLUR" },

	{ SRWG_CASE,    SRCASE_NOMINATIVE,      "NOM"  },
	{ SRWG_CASE,    SRCASE_GENITIVE,        "GEN"  },
	{ SRWG_CASE,    SRCASE_GENITIVE2,       "GEN2" },
	{ SRWG_CASE,    SRCASE_DATIVE,          "DAT"  },
	{ SRWG_CASE,    SRCASE_DATIVE2,         "DAT2" },
	{ SRWG_CASE,    SRCASE_ACCUSATIVE,      "ACC"  },
	{ SRWG_CASE,    SRCASE_ACCUSATIVE2,     "ACC2" },
	{ SRWG_CASE,    SRCASE_INSTRUMENT,      "INS"  },
	{ SRWG_CASE,    SRCASE_PREPOSITIONAL,   "PREP" },
	{ SRWG_CASE,    SRCASE_PREPOSITIONAL2,  "PREP2" },
	{ SRWG_CASE,    SRCASE_VOCATIVE,        "VOC"   },
	{ SRWG_CASE,    SRCASE_ADNUM,           "ADNUM" },
	{ SRWG_CASE,    SRCASE_OBJECTIVE,       "OBJCTV" }, // @v9.2.0 ��������� ����� (� ����������)

	{ SRWG_TENSE,   SRTENSE_PRESENT,        "PRES" },
	{ SRWG_TENSE,   SRTENSE_PAST,           "PAST" },
	{ SRWG_TENSE,   SRTENSE_FUTURE,         "FUTU" },
	{ SRWG_TENSE,   SRTENSE_PASTPARTICIPLE, "PASTPART" }, // @v9.2.0 ��������� ����� �������������� ����� (English: Past Participle)

	{ SRWG_PERSON,  SRPERSON_FIRST,         "P1" },
	{ SRWG_PERSON,  SRPERSON_SECOND,        "P2" },
	{ SRWG_PERSON,  SRPERSON_THIRD,         "P3" },

	{ SRWG_ANIMATE, SRANIM_ANIMATE,         "ANIM"   },
	{ SRWG_ANIMATE, SRANIM_INANIMATE,       "INANIM" },

	{ SRWG_VALENCY, SRVALENCY_AVALENT,      "AVALENT" },
	{ SRWG_VALENCY, SRVALENCY_INTRANSITIVE, "INTRANS" },
	{ SRWG_VALENCY, SRVALENCY_TRANSITIVE,   "TRANS"   },
	{ SRWG_VALENCY, SRVALENCY_DITRANSITIVE, "DITRANS" },

	{ SRWG_ASPECT,  SRASPECT_INFINITIVE,    "INF"     },
	{ SRWG_ASPECT,  SRASPECT_PERFECTIVE,    "PERFV"   },
	{ SRWG_ASPECT,  SRASPECT_IMPERFECTIVE,  "IMPERFV" },
	{ SRWG_ASPECT,  SRASPECT_HABITUAL,      "HABIT"   },
	{ SRWG_ASPECT,  SRASPECT_CONTINUOUS,    "CONTS"   },
	{ SRWG_ASPECT,  SRASPECT_STATIVE,       "CSTATV"  },
	{ SRWG_ASPECT,  SRASPECT_PROGRESSIVE,   "CPROGV"  },
	{ SRWG_ASPECT,  SRASPECT_PERFECT,       "PERF"    },

	{ SRWG_MOOD,    SRMOOD_INDICATIVE,      "INDCTV" },
	{ SRWG_MOOD,    SRMOOD_SUBJUNCTIVE,     "SUBJUNCTV" },
	{ SRWG_MOOD,    SRMOOD_CONJUNCTIVE,     "CONJUNCTV" },
	{ SRWG_MOOD,    SRMOOD_OPTATIVE,        "OPTV" },
	{ SRWG_MOOD,    SRMOOD_JUSSIVE,         "JUSSIV" },
	{ SRWG_MOOD,    SRMOOD_POTENTIAL,       "POTENT" },
	{ SRWG_MOOD,    SRMOOD_PROHIBITIVE,     "PROHIBV" },
	{ SRWG_MOOD,    SRMOOD_IMPERATIVE,      "IMPERV" },
	{ SRWG_MOOD,    SRMOOD_INTERROGATIVE,   "INTERRV" },

	{ SRWG_VOICE,   SRVOICE_ACTIVE,         "ACT" },
	{ SRWG_VOICE,   SRVOICE_PASSIVE,        "PASS" },

	{ SRWG_ADJCAT,  SRADJCAT_QUALIT,        "ADJQUAL" },
	{ SRWG_ADJCAT,  SRADJCAT_RELATIVE,      "ADJREL"  },
	{ SRWG_ADJCAT,  SRADJCAT_POSSESSIVE,    "ADJPOSS" },
	{ SRWG_ADJCAT,  SRADJCAT_NATION,        "ADJNAT"  }, // @v9.1.12

	{ SRWG_ADJCMP,  SRADJCMP_COMPARATIVE,   "COMP" },
	{ SRWG_ADJCMP,  SRADJCMP_SUPERLATIVE,   "SUPR" },
	{ SRWG_ADJCMP,  SRADJCMP_COMPARATIVE2,  "COMP2" },

	{ SRWG_SHORT,   SRSHORT_BREV,           "BREV" },
	{ SRWG_SHORT,   SRSHORT_PLEN,           "PLEN" },

	{ SRWG_INVARIABLE,  1,                  "INVAR" },
	{ SRWG_LOCAT,       1,                  "LOC"   },
	{ SRWG_ERROR,       1,                  "ERR"   },
	{ SRWG_TOBE,        1,                  "TOBE"  }, // @v9.2.0
	{ SRWG_QUEST,       1,                  "QUEST" }, // @v9.2.0 (�� ���� � ����� ��������� �������) �������������� ����� �����
	{ SRWG_MODAL,       1,                  "MODAL" }, // @v9.2.0 (�� ���� � ����� ��������� �������) ��������� ������

	{ SRWG_POSSESSIVE,  1,                  "POSSTAG"  }, // @v9.2.0
	{ SRWG_PREDICATIVE, 1,                  "PREDICAT" }, // @v9.2.0

	{ SRWG_PRONOUN, SRPRON_REFL,            "PNREFL" }, // @v9.2.0 ���������� ����������� (reflexive pronoun) [self]
	{ SRWG_PRONOUN, SRPRON_DEMONSTR,        "PNDEM"  }, // @v9.2.0 ������������ ����������� (pronomina demonstrativa) [one]
	{ SRWG_PRONOUN, SRPRON_PERSONAL,        "PNPERS" }, // @v9.2.0 ������������ �����������

	{ SRWG_COUNTAB, SRCTB_UNCOUNTABLE,      "NUNCNT" }, // @v9.2.0 ������������� ���������������;
		// (������������� ��������������� ���������� �������, ������� ������ ������ �����������. ������������� ���������������
		// �� ����� ����� �������������� �����. �� �� ����� �������������� �������, �������� ...an area of outstanding natural beauty.)
	{ SRWG_COUNTAB, SRCTB_MASS,             "NMASS"  }, // @v9.2.0 mass-��������������� ��������� � ���� ��������� ����������� � �������������
		// ���������������. ��� ������������ ��� �������������, ����� ���������� ����������, � ��� ����������� - ����� ���������� ����� ��� ���:
		// Rinse in cold water to remove any remaining detergent...Wash it in hot water with a good detergent ... We used several different
		// detergents in our stain-removal tests. Other examples of mass nouns are: shampoo, butter, bleach. pl - ������������� �����;
	{ SRWG_ADVERBCAT, SRADVCAT_INTERROGATIVE, "ADVINTR" },
	{ SRWG_ADVERBCAT, SRADVCAT_RELATIVE,      "ADVREL"  },
	{ SRWG_ADVERBCAT, SRADVCAT_POINTING,      "ADVPNT"  }

};

int FASTCALL SrWordForm::ToStr(SString & rBuf) const
{
	rBuf.Z();
	if(P_Buf) {
		for(size_t p = 0; P_Buf[p] != 0; p = Step(p)) {
			const int tag = Tag(p);
			const int val = Get(p);
			if(val) {
				for(uint i = 0; i < SIZEOFARRAY(__Tokens); i++) {
					if(__Tokens[i].Tag == tag && __Tokens[i].Val == val)
						CatTok(rBuf, __Tokens[i].Tok);
				}
			}
		}
	}
	return 1;
}

/*static*/int FASTCALL SrWordForm::StrToToken(const char * pStr, int * pVal)
{
	for(uint i = 0; i < SIZEOFARRAY(__Tokens); i++) {
		if(sstreqi_ascii(pStr, __Tokens[i].Tok)) {
			ASSIGN_PTR(pVal, __Tokens[i].Val);
			return __Tokens[i].Tag;
		}
	}
	ASSIGN_PTR(pVal, 0);
	return PPSetError(PPERR_SR_INVMORPHTOKEN, pStr);
}

int FASTCALL SrWordForm::FromStr(const char * pStr)
{
	int    ok = 1;
	Clear();
	if(pStr) {
		SString temp_buf;
		SStrScan scan(pStr);
		while(scan.Skip().GetIdent(temp_buf)) {
			int    _val = 0;
			int    _tag = SrWordForm::StrToToken(temp_buf, &_val);
			if(_tag)
				SetTag(_tag, _val);
			/*
			temp_buf.ToUpper();
			for(uint i = 0; i < SIZEOFARRAY(__Tokens); i++) {
				if(temp_buf == __Tokens[i].Tok) {
					SetTag(__Tokens[i].Tag, __Tokens[i].Val);
				}
			}
			*/
		}
	}
	return ok;
}
//
//
//
SrFlexiaModel::Item::Item() : AffixID(0), PrefixID(0), WordFormID(0)
{
}

SrFlexiaModel::SrFlexiaModel() : SrSList(SRGRAMTYP_FLEXIAMODEL)
{
}

int SrFlexiaModel::Normalize()
{
	int    ok = 1;
	Item item;
	SArray list(sizeof(Item));
	{
		for(size_t i = 0; GetNext(&i, item) > 0;) {
			THROW(list.insert(&item));
		}
	}
	{
		list.sort(PTR_CMPFUNC(_2long));
		Clear();
		for(uint i = 0; i < list.getCount(); i++) {
			THROW(Add(*(Item *)list.at(i)));
		}
	}
	CATCHZOK
	return ok;
}

int FASTCALL SrFlexiaModel::IsEq(const SrFlexiaModel & rS) const
{
	int    ok = 0;
	Item item;
	SArray list1(sizeof(Item)), list2(sizeof(Item));
	{
		for(size_t i = 0; GetNext(&i, item) > 0;) {
			THROW(list1.insert(&item));
		}
	}
	{
		for(size_t i = 0; rS.GetNext(&i, item) > 0;) {
			THROW(list2.insert(&item));
		}
	}
	if(list1.IsEq(list2))
		ok = 1;
	CATCHZOK
	return ok;

}

int SrFlexiaModel::Search(LEXID afxID, LEXID pfxID, LongArray & rWordFormList) const
{
	int    ok = -1;
	Item item;
	for(size_t p = 0; GetNext(&p, item) > 0;) {
		if(item.AffixID == afxID && item.PrefixID == pfxID) {
			rWordFormList.addUnique(item.WordFormID);
			ok = 1;
		}
	}
	return ok;
}

int SrFlexiaModel::GetNext(size_t * pPos, SrFlexiaModel::Item & rItem) const
{
	int    ok = 1;
	MEMSZERO(rItem);
	if(pPos) {
		size_t p = *pPos;
		assert(p <= Len);
		if(p == Len) {
			ok = -1;
		}
		else {
			uint8  ind = *PTR8(P_Buf+p);
			p += sizeof(uint8);
			if(ind & fmiWf16) {
				rItem.WordFormID = (int32)PTR16(P_Buf+p)[0];
				p += sizeof(int16);
			}
			else {
				rItem.WordFormID = (int32)PTR32(P_Buf+p)[0];
				p += sizeof(int32);
			}
			if(!(ind & fmiZeroAffix)) {
				rItem.AffixID = (int32)PTR32(P_Buf+p)[0];
				p += sizeof(int32);
			}
			if(!(ind & fmiZeroPrefix)) {
				rItem.PrefixID = (int32)PTR32(P_Buf+p)[0];
				p += sizeof(int32);
			}
			*pPos = p;
		}
	}
	else
		ok = 0;
	return ok;
}

int FASTCALL SrFlexiaModel::Add(const SrFlexiaModel::Item & rItem)
{
	int    ok = 1;
	size_t sz = sizeof(SrFlexiaModel::Item);
	uint8  ind = 0;
	if(rItem.WordFormID < 0x7fff) {
		ind |= fmiWf16;
		sz -= sizeof(int16);
	}
	if(rItem.AffixID == 0) {
		ind |= fmiZeroAffix;
		sz -= sizeof(rItem.AffixID);
	}
	if(rItem.PrefixID == 0) {
		ind |= fmiZeroPrefix;
		sz -= sizeof(rItem.PrefixID);
	}
	sz += sizeof(uint8);
	THROW(Alloc(Len+sz));
	{
		size_t p = Len;
		*PTR8(P_Buf+p) = ind;
		p += sizeof(uint8);
		if(ind & fmiWf16) {
			*PTR16(P_Buf+p) = (uint16)rItem.WordFormID;
			p += sizeof(uint16);
		}
		else {
			*PTR32(P_Buf+p) = (uint32)rItem.WordFormID;
			p += sizeof(uint32);
		}
		if(!(ind & fmiZeroAffix)) {
			*PTR32(P_Buf+p) = (uint32)rItem.AffixID;
			p += sizeof(uint32);
		}
		if(!(ind & fmiZeroPrefix)) {
			*PTR32(P_Buf+p) = (uint32)rItem.PrefixID;
			p += sizeof(uint32);
		}
		assert((p-Len) == sz);
		Len = p;
	}
	CATCHZOK
	return ok;
}
//
//
//
SrNGram::SrNGram() : ID(0), Ver(0)
{
}

SrNGram & SrNGram::Z()
{
	ID = 0;
	Ver = 0;
	WordIdList.clear();
	return *this;
}

static IMPL_CMPCFUNC(SrNGram_ByLength, p1, p2)
{
	const SrNGram * p_ng1 = static_cast<const SrNGram *>(p1);
	const SrNGram * p_ng2 = static_cast<const SrNGram *>(p2);
	return CMPSIGN(p_ng1->WordIdList.getCount(), p_ng2->WordIdList.getCount());
}

SrNGramCollection::SrNGramCollection() : TSCollection <SrNGram>()
{
}

void SrNGramCollection::SortByLength()
{
	sort(PTR_CMPCFUNC(SrNGram_ByLength));
}
//
//
//
SrCPropDecl::SrCPropDecl() : PropID(0), SymbID(0)
{
	Tail.Init();
}

SrCPropDecl::~SrCPropDecl()
{
	Tail.Destroy();
}

int FASTCALL SrCPropDecl::IsEq(const SrCPropDecl & rS) const
{
	if(PropID != rS.PropID)
		return 0;
	else if(SymbID != rS.SymbID)
		return 0;
	else if(!Tail.IsEq(rS.Tail))
		return 0;
	else
		return 1;
}
//
//
//
SrCPropDeclList::Item::Item() : PropID(0), SymbID(0), TailS(0), TailP(0)
{
}

SrCPropDeclList::SrCPropDeclList() : PoolP(4)
{
	Pool.Init();
	Pool.Alloc(32);
}

SrCPropDeclList::~SrCPropDeclList()
{
	Pool.Destroy();
}

SrCPropDeclList::SrCPropDeclList(const SrCPropDeclList & rS)
{
	Pool.Init();
	Copy(rS);
}

SrCPropDeclList & FASTCALL SrCPropDeclList::operator = (const SrCPropDeclList & rS)
{
	return Copy(rS);
}

SrCPropDeclList & FASTCALL SrCPropDeclList::Copy(const SrCPropDeclList & rS)
{
	D = rS.D;
	PoolP = rS.PoolP;
	Pool.Copy(rS.Pool);
	return *this;
}

SrCPropDeclList & SrCPropDeclList::Z()
{
	D.clear();
	PoolP = 4;
	return *this;
}

int FASTCALL SrCPropDeclList::Add(const SrCPropDecl & rP)
{
	int    ok = 1;
	Item   item;
	item.PropID = rP.PropID;
	item.SymbID = rP.SymbID;
	const size_t added_size = rP.Tail.Size;
	if(added_size) {
		Pool.Alloc(ALIGNSIZE(PoolP + added_size, 4));
		memcpy(Pool.P_Buf+PoolP, rP.Tail.P_Buf, added_size);
		item.TailP = PoolP;
		item.TailS = added_size;
		PoolP += added_size;
	}
	D.insert(&item);
	return ok;
}

int SrCPropDeclList::Replace(uint pos, const SrCPropDecl & rP)
{
	int    ok = 1;
	if(pos < D.getCount()) {
		D.atFree(pos);
		Item item;
		item.PropID = rP.PropID;
		item.SymbID = rP.SymbID;
		const size_t added_size = rP.Tail.Size;
		if(added_size) {
			Pool.Alloc(ALIGNSIZE(PoolP + added_size, 4));
			memcpy(Pool.P_Buf+PoolP, rP.Tail.P_Buf, added_size);
			item.TailP = PoolP;
			item.TailS = added_size;
			PoolP += added_size;
		}
		D.atInsert(pos, &item);
	}
	else
		ok = 0;
	return ok;
}

int SrCPropDeclList::Get(uint idx, SrCPropDecl & rP) const
{
	int    ok = 1;
	if(idx < D.getCount()) {
		const Item & r_item = D.at(idx);
		rP.PropID = r_item.PropID;
		rP.SymbID = r_item.SymbID;
		rP.Tail.Destroy();
		if(r_item.TailS) {
			rP.Tail.Alloc(r_item.TailS);
			memcpy(rP.Tail.P_Buf, Pool.P_Buf+r_item.TailP, r_item.TailS);
		}
	}
	else
		ok = 0;
	return ok;
}

int SrCPropDeclList::GetBySymbID(LEXID id, SrCPropDecl & rP) const
{
	int    ok = 0;
	for(uint i = 0; i < D.getCount(); i++) {
		const Item & r_item = D.at(i);
		if(r_item.SymbID == id) {
			rP.PropID = r_item.PropID;
			rP.SymbID = r_item.SymbID;
			rP.Tail.Destroy();
			if(r_item.TailS) {
				rP.Tail.Alloc(r_item.TailS);
				memcpy(rP.Tail.P_Buf, Pool.P_Buf+r_item.TailP, r_item.TailS);
			}
			ok = 1;
		}
	}
	return ok;
}

int SrCPropDeclList::Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx)
{
	int    ok = 1;
	THROW_SL(pCtx->Serialize(dir, &D, rBuf));
	THROW_SL(pCtx->Serialize(dir, PoolP, rBuf));
	if(dir < 0) {
		THROW_SL(Pool.Alloc(ALIGNSIZE(PoolP, 4)));
	}
	THROW_SL(pCtx->SerializeBlock(dir, PoolP, Pool.P_Buf, rBuf, 1));
	CATCHZOK
	return ok;
}

int FASTCALL SrCPropDeclList::IsEq(const SrCPropDeclList & rS) const
{
	if(D.getCount() != rS.D.getCount())
		return 0;
	else {
		const uint c = D.getCount();
		for(uint i = 0; i < c; i++) {
			const Item & ri = D.at(i);
			const Item & rsi = rS.D.at(i);
			if(ri.PropID != rsi.PropID)
				return 0;
			else if(ri.SymbID != rsi.SymbID)
				return 0;
			else if(ri.TailS != rsi.TailS)
				return 0;
			else if(ri.TailS) {
				if(memcmp(Pool.P_Buf+ri.TailP, rS.Pool.P_Buf+rsi.TailP, ri.TailS) != 0)
					return 0;
			}
		}
		return 1;
	}
}

int FASTCALL SrCPropDeclList::Merge(const SrCPropDeclList & rS)
{
	int    ok = -1;
	if(!IsEq(rS)) { // ��������� ������� ������� ���������� ���������� �������� ��� ��������� ��������� ���� �� ��������� ������
		const uint sc = rS.D.getCount();
		for(uint i = 0; i < sc; i++) {
			const Item & rsi = rS.D.at(i);
			SrCPropDecl temp_pd;
			rS.Get(i, temp_pd);
			int    do_add = 1;
			const uint c = D.getCount();
			for(uint j = 0; j < c; j++) {
				const Item & ri = D.at(j);
				if(ri.SymbID == rsi.SymbID) {
					if(ri.PropID == rsi.PropID) {
						//
						// ���� {Prop; Symb} ������������. �������� ������� ������� ���,
						// ������� ��������� � rS (������� ��������������� ����� ������
						// ���������� ����� �������.
						//
						THROW(Replace(j, temp_pd));
						do_add = 0;
						ok = 1;
					}
					else {
						CALLEXCEPT();
					}
				}
			}
			if(do_add) {
				THROW(Add(temp_pd));
				ok = 1;
			}
		}
	}
	CATCHZOK
	return ok;
}
//
//
//
/*struct SrConcept_SurrogatePrefix {
	int    Surrsymbpfx;
	const char * P_Prefix;
};*/

static const /*SrConcept_SurrogatePrefix*/SIntToSymbTabEntry SrConcept_SurrogatePrefix_List[] = {
	{ SrConcept::surrsymbsrcFIAS, "fias" },
	{ SrConcept::surrsymbsrcGBIF, "gbif" },
	{ SrConcept::surrsymbsrcTICKER, "tckr" }
};

/*static const char * Get_SrConcept_SurrogatePrefix(int surrsymbpfx)
{
	for(uint i = 0; i < SIZEOFARRAY(SrConcept_SurrogatePrefix_List); i++) {
		if(SrConcept_SurrogatePrefix_List[i].Surrsymbpfx == surrsymbpfx) {
			return SrConcept_SurrogatePrefix_List[i].P_Prefix;
		}
	}
	return 0;
}*/

/*static*/int SrConcept::MakeSurrogateSymb(int surrsymbpfx, const void * pData, uint dataSize, SString & rSymb)
{
	int    ok = 0;
	rSymb.Z();
	//const char * p_prefix = Get_SrConcept_SurrogatePrefix(surrsymbpfx);
	const char * p_prefix = SIntToSymbTab_GetSymbPtr(SrConcept_SurrogatePrefix_List, SIZEOFARRAY(SrConcept_SurrogatePrefix_List), surrsymbpfx);
	assert(!isempty(p_prefix));
	if(surrsymbpfx == surrsymbsrcFIAS) {
		if(pData && dataSize == sizeof(S_GUID)) {
			SString & r_temp_buf = SLS.AcquireRvlStr();
			r_temp_buf.EncodeMime64(pData, dataSize);
			rSymb.Cat(p_prefix).Cat(r_temp_buf);
			ok = 1;
		}
	}
	else if(surrsymbpfx == surrsymbsrcGBIF) {
		if(pData && dataSize == sizeof(uint32)) {
			rSymb.Cat(p_prefix).Cat(*(uint32 *)pData);
		}
	}
	else if(surrsymbpfx == surrsymbsrcTICKER) {
		if(pData && dataSize >= 2 && dataSize <= 64) {
			rSymb.Cat(p_prefix).CatN((const char *)pData, dataSize);
			assert(rSymb.Len() == (sstrlen(p_prefix) + dataSize));
		}
	}
	return ok;
}

/*static*/int SrConcept::IsSurrogateSymb(const char * pSymb, void * pData, uint * pDataSize)
{
	int    surrsymbpfx = surrsymbsrcUndef;
	if(!isempty(pSymb)) {
		SString temp_buf;
		//const char * p_prefix = Get_SrConcept_SurrogatePrefix(surrsymbpfx);
		const char * p_prefix = SIntToSymbTab_GetSymbPtr(SrConcept_SurrogatePrefix_List, SIZEOFARRAY(SrConcept_SurrogatePrefix_List), surrsymbpfx);
		for(uint i = 0; i < SIZEOFARRAY(SrConcept_SurrogatePrefix_List); i++) {
			const char * p_prefix = SrConcept_SurrogatePrefix_List[i].P_Symb/*P_Prefix*/;
			const size_t prefix_len = sstrlen(p_prefix);
			if(strncmp(pSymb, p_prefix, prefix_len) == 0) {
				temp_buf = pSymb+prefix_len;
				if(SrConcept_SurrogatePrefix_List[i].Id/*Surrsymbpfx*/ == surrsymbsrcFIAS) {
					S_GUID uuid;
					size_t real_len = 0;
					temp_buf.DecodeMime64(&uuid, sizeof(uuid), &real_len);
					if(real_len == sizeof(uuid)) {
						surrsymbpfx = SrConcept_SurrogatePrefix_List[i].Id/*Surrsymbpfx*/;
						if(pData && pDataSize) {
							if(*pDataSize >= sizeof(uuid))
								memcpy(pData, &uuid, sizeof(uuid));
							else
								surrsymbpfx = surrsymbsrcUndef;
						}
						break;
					}
				}
			}
		}
	}
	return surrsymbpfx;
}

SrConcept::SrConcept() : ID(0), SymbID(0), Ver(0)
{
}

int FASTCALL SrConcept::IsEq(const SrConcept & rS) const
{
	int    ok = 1;
	if(ID != rS.ID)
		ok = 0;
	else if(SymbID != rS.SymbID)
		ok = 0;
	else if(Ver != rS.Ver)
		ok = 0;
	else if(!Pdl.IsEq(rS.Pdl))
		ok = 0;
	return ok;
}

SrConcept & SrConcept::Z()
{
	ID = 0;
	SymbID = 0;
	Ver = 0;
	Pdl.Z();
	return *this;
}
//
//
//
SrCProp::SrCProp() : CID(0), PropID(0)
{
}

SrCProp::SrCProp(CONCEPTID cID, CONCEPTID propID) : CID(cID), PropID(propID)
{
}

int FASTCALL SrCProp::IsEq(const SrCProp & rS) const
{
	int    ok = 1;
	if(CID != rS.CID)
		ok = 0;
	else if(PropID != rS.PropID)
		ok = 0;
	else if(!Value.IsEq(rS.Value))
		ok = 0;
	return ok;
}

SrCProp & SrCProp::Z()
{
	CID = 0;
	PropID = 0;
	Value.Z();
	return *this;
}

SrCProp & FASTCALL SrCProp::operator = (int val)
{
	int64 v64 = val;
	Value.Write(v64);
	return *this;
}

SrCProp & FASTCALL SrCProp::operator = (int64 val)
{
	Value.Write(val);
	return *this;
}

SrCProp & FASTCALL SrCProp::operator = (double val)
{
	Value.Write(val);
	return *this;
}

SrCProp & FASTCALL SrCProp::operator = (const SUniTime & rUtVal)
{
	Value.Write(&rUtVal, sizeof(rUtVal));
	return *this;
}

int FASTCALL SrCProp::Get(int64 & rIntVal) const
{
	size_t s = Value.ReadStatic(&rIntVal, sizeof(rIntVal));
	return BIN(s == sizeof(rIntVal));
}

int FASTCALL SrCProp::Get(double & rRealVal) const
{
	size_t s = Value.ReadStatic(&rRealVal, sizeof(rRealVal));
	return BIN(s == sizeof(rRealVal));
}

int FASTCALL SrCProp::Get(SUniTime & rUtVal) const
{
	size_t s = Value.ReadStatic(&rUtVal, sizeof(rUtVal));
	return BIN(s == sizeof(rUtVal));
}
//
//
//
SrCPropList::SrCPropList()
{
	uint32 zero = 0;
	D.Write(zero); // zero offset
}

SrCPropList & SrCPropList::Z()
{
	L.clear();
	D.Z();
	uint32 zero = 0;
	D.Write(zero); // zero offset
	return *this;
}

uint SrCPropList::GetCount() const { return L.getCount(); }
size_t FASTCALL SrCPropList::GetDataLen(uint pos) const { return (pos < L.getCount()) ? L.at(pos).S : 0; }

int SrCPropList::Search(CONCEPTID cID, CONCEPTID propID, uint * pPos) const
{
	int    ok = 0;
	uint   pos = 0;
	const uint c = L.getCount();
	for(uint i = 0; !ok && i < c; i++) {
		const Item & r_item = L.at(i);
		if(r_item.CID == cID && r_item.PropID == propID) {
			pos = i;
			ok = 1;
		}
	}
	ASSIGN_PTR(pPos, pos);
	return ok;
}

int SrCPropList::Get(CONCEPTID cID, CONCEPTID propID, SrCProp & rProp) const
{
	rProp.Z();
	int    ok = 0;
	const uint c = L.getCount();
	for(uint i = 0; !ok && i < c; i++) {
		const Item & r_item = L.at(i);
		if(r_item.CID == cID && r_item.PropID == propID) {
			rProp.CID = r_item.CID;
			rProp.PropID = r_item.PropID;
			if(r_item.P && r_item.S) {
				const void * p_data = D.GetBuf(r_item.P);
				if(p_data)
					rProp.Value.Write(p_data, r_item.S);
			}
			ok = 1;
		}
	}
	return ok;
}

int SrCPropList::GetByPos(uint pos, SrCProp & rProp) const
{
	int    ok = 1;
	rProp.Z();
	if(pos < L.getCount()) {
		const Item & r_item = L.at(pos);
		rProp.CID = r_item.CID;
		rProp.PropID = r_item.PropID;
		if(r_item.P && r_item.S) {
			const void * p_data = D.GetBuf(r_item.P);
			if(p_data)
				rProp.Value.Write(p_data, r_item.S);
		}
	}
	else
		ok = 0;
	return ok;
}

const void * SrCPropList::GetDataPtr(uint pos, size_t * pDataLen) const
{
	const void * p = 0;
	size_t sz = 0;
	if(pos < L.getCount()) {
		const Item & r_item = L.at(pos);
		if(r_item.P && r_item.S) {
			sz = r_item.S;
			p = D.GetBuf(r_item.P);
		}
	}
	ASSIGN_PTR(pDataLen, sz);
	return p;
}

size_t SrCPropList::GetData(uint pos, void * pData, size_t bufLen) const
{
	size_t sz = 0;
	const void * p = GetDataPtr(pos, &sz);
	if(p && sz) {
		sz = MIN(sz, bufLen);
		memcpy(pData, p, sz);
	}
	return sz;
}

int SrCPropList::SetData(uint pos, const void * pData, size_t dataLen)
{
	int    ok = 1;
	THROW(pos < L.getCount());
	{
		Item & r_item = L.at(pos);
		if(pData && dataLen) {
			uint32 offs = static_cast<uint32>(D.GetWrOffs());
			THROW(D.Write(pData, dataLen));
			r_item.P = offs;
			r_item.S = dataLen;
		}
		else {
			r_item.P = 0;
			r_item.S = 0;
		}
	}
	CATCHZOK
	return ok;
}

int SrCPropList::Set(CONCEPTID cID, CONCEPTID propID, const void * pData, size_t dataLen)
{
	int    ok = 1;
	uint pos = 0;
	if(Search(cID, propID, &pos)) {
		size_t data_len;
		const void * p_data = GetDataPtr(pos, &data_len);
		if(p_data) {
			if(data_len == dataLen && memcmp(p_data, pData, data_len) == 0) {
				ok = -1;
			}
			else {
				THROW(SetData(pos, pData, dataLen));
			}
		}
	}
	else {
		Item new_item;
		new_item.CID = cID;
		new_item.PropID = propID;
		uint   pos = L.getCount();
		THROW(L.insert(&new_item));
		THROW(SetData(pos, pData, dataLen));
	}
	CATCHZOK
	return ok;
}
//
//
//
SrWordInfo::SrWordInfo()
{
	Z();
}

SrWordInfo & SrWordInfo::Z()
{
	BaseID = 0;
	PrefixID = 0;
	AffixID = 0;
	BaseFormID = 0;
	FormID = 0;
	WaID = 0;
	AbbrExpID = 0;
	Score = 0.0;
	return *this;
}
//
//
//
SrWordAssoc::SrWordAssoc() : ID(0), WordID(0), Flags(0), BaseFormID(0), FlexiaModelID(0), AccentModelID(0),
	PrefixID(0), AffixModelID(0), AbbrExpID(0)
{
}

int FASTCALL SrWordAssoc::IsEq(const SrWordAssoc & rS) const
{
#define CMPFLD(f) if(f != rS.f) return 0
	CMPFLD(ID);
	CMPFLD(WordID);
	CMPFLD(Flags);
	CMPFLD(BaseFormID);
	CMPFLD(FlexiaModelID);
	CMPFLD(AccentModelID);
	CMPFLD(AffixModelID);
	CMPFLD(AbbrExpID);
#undef CMPFLD
	return 1;
}

SrWordAssoc & SrWordAssoc::Normalize()
{
	//Flags = 0;
	SETFLAG(Flags, fHasFlexiaModel, FlexiaModelID);
	SETFLAG(Flags, fHasAccentModel, AccentModelID);
	SETFLAG(Flags, fHasPrefix, PrefixID);
	SETFLAG(Flags, fHasAffixModel, AffixModelID);
	SETFLAG(Flags, fHasAbbrExp, AbbrExpID);
	return *this;
}

SString & FASTCALL SrWordAssoc::ToStr(SString & rBuf) const
{
	return rBuf.Z().CatChar('[').Cat(ID).CatDiv(',', 2).Cat(WordID).CatDiv(',', 2).Cat("0x").CatHex(Flags).CatDiv(',', 2).
		Cat(BaseFormID).CatDiv(',', 2).Cat(FlexiaModelID).CatDiv(',', 2).Cat(AccentModelID).CatDiv(',', 2).
		Cat(PrefixID).CatDiv(',', 2).Cat(AffixModelID).CatDiv(',', 2).Cat(AbbrExpID).CatChar(']');
}
//
//
//
SrImportParam::SrImportParam() : InputKind(0), LangID(0), CpID(0), Flags(0)
{
}

void SrImportParam::SetField(int fld, const char * pVal) { StrItems.Add(fld, pVal); }
int  SrImportParam::GetField(int fld, SString & rVal) const { return StrItems.GetText(fld, rVal); }
//
//
#if(_MSC_VER >= 1900) // {
// @construction {
#include <..\OSF\abseil\absl\numeric\int128.h>

static const uint64 __ued_planarangle = 0x90ULL;
static const uint64 __ued_geoloc      = 0x93ULL;

/*static*/uint64 UED::ConvertGeoLoc(const SGeoPosLL & rGeoPos)
{
	uint64 result = 0;
	if(rGeoPos.IsValid()) {
		const uint64 ued_width = ((1 << 28) - 1); // 28 bits
		uint64 _lat = static_cast<uint64>((rGeoPos.Lat + 90.0)  * 1000000.0);
		uint64 _lon = static_cast<uint64>((rGeoPos.Lon + 180.0) * 1000000.0);
		absl::uint128 _lat128 = (_lat * ued_width);
		_lat128 /= absl::uint128(180ULL * 1000000ULL);
		assert(_lat128 <= ued_width);
		absl::uint128 _lon128 = (_lon * ued_width);
		_lon128 /= absl::uint128(360ULL * 1000000ULL);
		assert(_lon128 <= ued_width);
		result = (__ued_geoloc << 56) | (static_cast<uint64>(_lat128) << 28) | (static_cast<uint64>(_lon128));
		//
		// test straighten
		//
		SGeoPosLL test_geopos;
		absl::uint128 _lat128_ = ((result >> 28) & 0xfffffffULL) * absl::uint128(180ULL * 1000000ULL);
		absl::uint128 _lon128_ = (result & 0xfffffffULL) * absl::uint128(360ULL * 1000000ULL);
		_lat128_ /= ued_width;
		_lon128_ /= ued_width;
		test_geopos.Lat = (static_cast<double>(_lat128_) / 1000000.0) - 90.0;
		test_geopos.Lon = (static_cast<double>(_lon128_) / 1000000.0) - 180.0;
	}
	return result;
}

/*static*/uint64 UED::StraightenGeoLoc(uint64 ued, SGeoPosLL & rGeoPos)
{
	uint64 result = 0;
	if((ued >> 56) == __ued_geoloc) {
		const uint64 ued_width = ((1 << 28) - 1); // 28 bits
		absl::uint128 _lat128 = ((ued >> 28) & 0xfffffffULL) * absl::uint128(180ULL * 1000000ULL);
		absl::uint128 _lon128 = (ued & 0xfffffffULL) * absl::uint128(360ULL * 1000000ULL);
		_lat128 /= ued_width;
		_lon128 /= ued_width;
		rGeoPos.Lat = (static_cast<double>(_lat128) / 1000000.0) - 90.0;
		rGeoPos.Lon = (static_cast<double>(_lon128) / 1000000.0) - 180.0;
		result = 1;
	}
	return result;
}

static uint64 UedPlanarAngleMask     = 0x007fffffffffffULL;
static uint64 UedPlanarAngleSignMask = 0x00800000000000ULL;

/*static*/uint64 UED::ConvertPlanarAngle_Deg(double deg)
{
	uint64 result = 0;
	const uint64 _divisor = 360ULL * 3600ULL;
	const bool  minus = (deg < 0);
	const uint64 maxv = (UedPlanarAngleMask / _divisor) * _divisor;
	assert((maxv % 360ULL) == 0ULL);
	double _deg = fmod(deg, 360.0);
	assert(_deg > -360.0 && _deg < 360.0);
	_deg = fabs(_deg);
	if(minus)
		result = (__ued_planarangle << 56) | (uint64)(_deg * (maxv / 360ULL)) | UedPlanarAngleSignMask;
	else
		result = (__ued_planarangle << 56) | (uint64)(_deg * (maxv / 360ULL));
	assert((result & ~(UedPlanarAngleMask | UedPlanarAngleSignMask)) == (__ued_planarangle << 56));
	return result;
}

/*static*/uint64 UED::StraightenPlanarAngle_Deg(uint64 ued, double & rDeg)
{
	uint64 result = 0;
	if((ued >> 56) == __ued_planarangle) {
		const uint64 _divisor = 360ULL * 3600ULL;
		const uint64 maxv = (UedPlanarAngleMask / _divisor) * _divisor;
		rDeg = static_cast<double>(ued & UedPlanarAngleMask) / (maxv / 360ULL);
		assert(rDeg >= 0.0 && rDeg < 360.0);
		if(ued & UedPlanarAngleSignMask)
			rDeg = -rDeg;
		result = 1;
	}
	return result;
}
// } @construction
#endif // } (_MSC_VER >= 1900)
//
//
//
static IMPL_CMPFUNC(SrUedContainer_TextEntry, p1, p2)
{
	const SrUedContainer::TextEntry * p_e1 = static_cast<const SrUedContainer::TextEntry *>(p1);
	const SrUedContainer::TextEntry * p_e2 = static_cast<const SrUedContainer::TextEntry *>(p2);
	RET_CMPCASCADE2(p_e1, p_e2, Locale, Id);
}

SrUedContainer::SrUedContainer() : LinguaLocusMeta(0), Ht(1024*32, 0), LastSymbHashId(0)
{
}
	
SrUedContainer::~SrUedContainer()
{
}

uint64 SrUedContainer::SearchBaseIdBySymbId(uint symbId, uint64 meta) const
{
	uint64 id = 0;
	for(uint i = 0; !id && i < BL.getCount(); i++) {
		if(BL.at(i).SymbHashId == symbId) {
			uint64 temp_id = BL.at(i).Id;
			if(UED::BelongToMeta(temp_id, meta))
				id = temp_id;
		}
	}
	return id;
}

uint64 SrUedContainer::SearchBaseSymb(const char * pSymb, uint64 meta) const
{
	uint64 id = 0;
	if(!meta || UED::IsMetaId(meta)) {
		uint symb_hash_id = 0;
		if(Ht.Search(pSymb, &symb_hash_id, 0)) {
			id = SearchBaseIdBySymbId(symb_hash_id, meta); 
		}
	}
	return id;
}

bool   SrUedContainer::SearchBaseId(uint64 id, SString & rSymb) const
{
	rSymb.Z();
	bool   ok = false;
	for(uint i = 0; !ok && i < BL.getCount(); i++) {
		if(BL.at(i).Id == id) {
			if(Ht.GetByAssoc(BL.at(i).SymbHashId, rSymb))
				ok = true;
		}
	}
	return ok;
}

int SrUedContainer::ReplaceSurrogateLocaleIds(const SymbHashTable & rT)
{
	int    ok = 1;
	SString temp_buf;
	THROW(LinguaLocusMeta);
	for(uint i = 0; i < TL.getCount(); i++) {
		TextEntry & r_e = TL.at(i);
		if(r_e.Locale) {
			THROW(rT.GetByAssoc(r_e.Locale, temp_buf));
			{
				uint64 locale_id = SearchBaseSymb(temp_buf, LinguaLocusMeta);
				THROW(locale_id);
				THROW(UED::BelongToMeta(locale_id, LinguaLocusMeta));
				r_e.Locale = UED::MakeShort(locale_id, LinguaLocusMeta);
				assert(r_e.Locale);
			}
		}
	}
	CATCHZOK
	return ok;
}
	
int SrUedContainer::ReadSource(const char * pFileName)
{
	int    ok = 1;
	SString line_buf;
	SString temp_buf;
	SString lang_buf;
	SString text_buf;
	SString log_buf;
	StringSet ss;
	uint   last_linglocus_temp_id = 0;
	SymbHashTable temporary_linglocus_tab(512);
	SStrScan scan;
	SFile f_in(pFileName, SFile::mRead);
	THROW(f_in.IsValid());
	while(f_in.ReadLine(line_buf, SFile::rlfChomp|SFile::rlfStrip)) {
		if(line_buf.HasPrefix("//")) {
			; // comment
		}
		else if(line_buf.IsEmpty()) {
			; // empty line
		}
		else {
			size_t comment_pos = 0;
			if(line_buf.Search("//", 0, 0, &comment_pos)) {
				line_buf.Trim(comment_pos).Strip();
			}
			if(line_buf.NotEmpty()) {
				//line_buf.Tokenize(" \t", ss.Z());
				ss.Z();
				bool scan_ok = true;
				{
					scan.Set(line_buf, 0);
					if(scan.GetXDigits(temp_buf.Z())) {
						ss.add(temp_buf);
						scan.Skip();
						if(scan.GetQuotedString(temp_buf) || scan.GetIdent(temp_buf)) {
							ss.add(temp_buf);
							scan.Skip();
							if(!scan.IsEnd()) {
								if(scan.GetQuotedString(temp_buf)) {
									ss.add(temp_buf);
								}
								else
									scan_ok = false;
							}
						}
						else
							scan_ok = false;
					}
					else
						scan_ok = false;
				}
				uint ssc = ss.getCount();
				if(scan_ok && oneof2(ssc, 2, 3)) {
					uint64 id = 0;
					int   lang_id = 0;
					text_buf.Z();
					lang_buf.Z();
					uint   token_n = 0;
					for(uint ssp = 0; ss.get(&ssp, temp_buf);) {
						token_n++;
						temp_buf.Utf8ToLower();
						if(token_n == 1) {
							id = sxtou64(temp_buf);
						}
						else if(token_n == 2) {
							if(ssc == 2) {
								text_buf = temp_buf.StripQuotes();
							}
							else {
								assert(ssc == 3);
								lang_buf = temp_buf;
								uint llid = 0;
								if(!temporary_linglocus_tab.Search(lang_buf, &llid, 0)) {
									llid = ++last_linglocus_temp_id;
									temporary_linglocus_tab.Add(lang_buf, llid, 0);
								}
								lang_id = llid;
								//lang_id = RecognizeLinguaSymb(lang_buf, 1);
							}
						}
						else if(token_n == 3) {
							assert(ssc == 3);
							text_buf = temp_buf.StripQuotes();
						}
					}
					{
						log_buf.Z().Cat(id).Tab().Cat(lang_buf).Tab().Cat(text_buf);
						//PPLogMessage("ued-import.log", log_buf, LOGMSGF_TIME);
					}
					if(id) {
						if(ssc == 2) {
							if(text_buf.IsEqiAscii("lingualocus")) {
								if(!LinguaLocusMeta)
									LinguaLocusMeta = id;
								else {
									; // @error
								}
							}
							BaseEntry new_entry;
							new_entry.Id = id;
							uint   symb_hash_id = 0;
							if(!Ht.Search(text_buf, &symb_hash_id, 0)) {
								symb_hash_id = ++LastSymbHashId;
								Ht.Add(text_buf, symb_hash_id);
							}
							new_entry.SymbHashId = symb_hash_id;
							//AddS(text_buf, &new_entry.SymbP);
							BL.insert(&new_entry);
						}
						else if(ssc == 3) {
							if(lang_id) {
								TextEntry new_entry;
								new_entry.Id = id;
								new_entry.Locale = lang_id;
								AddS(text_buf, &new_entry.TextP);
								TL.insert(&new_entry);
							}
						}
					}
				}
				else {
					// invalid line
				}
			}
		}
	}
	Ht.BuildAssoc();
	THROW_SL(temporary_linglocus_tab.BuildAssoc());
	THROW(ReplaceSurrogateLocaleIds(temporary_linglocus_tab));
	CATCHZOK
	return ok;
}

/*static*/void SrUedContainer::MakeUedCanonicalName(SString & rResult, long ver)
{
	rResult.Z();
	rResult.Cat("ued-id");
	if(ver > 0)
		rResult.CatChar('-').CatLongZ(ver, 4);
	else if(ver == 0)
		rResult.CatChar('-').Cat("????");
	else { // ver < 0 - file-name for programming interface source
		//
	}
}

/*static*/long SrUedContainer::SearchLastCanonicalFile(const char * pPath, SString & rFileName)
{
	long   result = 0; // version
	long   max_ver = 0;
	SString max_ver_filename;
	SString temp_buf;
	SString fn_dat;
	SString fn_hash;
	SString fn_wc;
	StringSet ss;
	SDirEntry de;
	SPathStruc ps;
	MakeUedCanonicalName(fn_wc, 0);
	(temp_buf = pPath).Strip().SetLastSlash().Cat(fn_wc).Dot().Cat("dat");
	for(SDirec sd(temp_buf, 0); sd.Next(&de) > 0;) {
		if(de.IsFile()) {
			ss.Z();
			de.GetNameA(temp_buf);
			ps.Split(temp_buf);
			ps.Nam.Tokenize("-", ss);
			if(ss.getCount() == 3 && ss.getByIdx(2, temp_buf)) { // ued;id;version ("ued-id-0004")
				long iter_ver = temp_buf.ToLong();
				if(iter_ver > 0 && iter_ver > max_ver) {
					//
					// ���������� ��������� ��� ����� � �������� ������ ������������ ���� � ����������� ������ (ext .sha256)
					//
					de.GetNameA(pPath, temp_buf);
					ps.Split(temp_buf);
					ps.Ext = "sha256";
					ps.Merge(temp_buf);
					if(fileExists(temp_buf)) {
						max_ver = iter_ver;
						de.GetNameA(pPath, max_ver_filename);
					}
				}
			}
		}
	}
	if(max_ver > 0) {
		assert(max_ver_filename.NotEmpty());
		assert(fileExists(max_ver_filename));
		rFileName = max_ver_filename;
		result = max_ver;
	}
	return result;
}
	
int SrUedContainer::WriteSource(const char * pFileName)
{
	int    ok = 1;
	SString line_buf;
	SString temp_buf;
	THROW(!isempty(pFileName)); // @err
	THROW(LinguaLocusMeta); // @err
	{
		SFile f_out(pFileName, SFile::mWrite|SFile::mBinary);
		THROW_SL(f_out.IsValid());
		BL.sort(PTR_CMPFUNC(int64));
		TL.sort(PTR_CMPFUNC(SrUedContainer_TextEntry));
		{
			for(uint i = 0; i < BL.getCount(); i++) {
				const BaseEntry & r_e = BL.at(i);
				SearchBaseId(r_e.Id, temp_buf);
				assert(temp_buf.NotEmpty());
				line_buf.Z().CatHex(r_e.Id).Space().CatQStr(temp_buf).CRB();
				f_out.WriteLine(line_buf);
			}
		}
		{
			for(uint i = 0; i < TL.getCount(); i++) {
				const TextEntry & r_e = TL.at(i);
				line_buf.Z().CatHex(r_e.Id).Space();
				assert(r_e.Locale);
				uint64 ued_locus = UED::MakeCanonical(r_e.Locale, LinguaLocusMeta);
				assert(ued_locus);
				SearchBaseId(ued_locus, temp_buf);
				assert(temp_buf.NotEmpty());
				line_buf.Cat(temp_buf).Space(); // locale without quotes!
				GetS(r_e.TextP, temp_buf);
				assert(temp_buf.NotEmpty());
				line_buf.CatQStr(temp_buf);
				line_buf.CRB();
				f_out.WriteLine(line_buf);
			}
		}
	}
	{
		SPathStruc ps(pFileName);
		ps.Ext = "sha256";
		ps.Merge(temp_buf);
		SFile f_in(pFileName, SFile::mRead|SFile::mBinary);
		SFile f_hash(temp_buf, SFile::mWrite);
		SBinaryChunk bc_hash;
		THROW_SL(f_in.IsValid());
		THROW_SL(f_hash.IsValid());
		THROW_SL(f_in.CalcHash(0, SHASHF_SHA256, bc_hash));
		bc_hash.Hex(temp_buf);
		f_hash.WriteLine(temp_buf);
	}
	CATCHZOK
	return ok;
}

bool SrUedContainer::GenerateSourceDecl_C(const char * pFileName)
{
	bool   ok = true;
	SString temp_buf;
	SString def_symb;
	SString def_value;
	SString meta_symb;
	SString h_sentinel_def;
	Generator_CPP gen(pFileName);
	const  SPathStruc ps(pFileName);
	uint   max_symb_len = 0;
	{
		for(uint i = 0; i < BL.getCount(); i++) {
			const BaseEntry & r_be = BL.at(i);
			if(UED::IsMetaId(r_be.Id)) {
				Ht.GetByAssoc(r_be.SymbHashId, meta_symb);
				assert(meta_symb.NotEmpty());
				meta_symb.ToUpper();
				(def_symb = "UED").CatChar('_').Cat("META").CatChar('_').Cat(meta_symb);
				SETMAX(max_symb_len, def_symb.Len());
				if(r_be.Id != 0x0000000100000001ULL) { // super-meta wich identifies other meta's
					for(uint j = 0; j < BL.getCount(); j++) {
						const BaseEntry & r_be_inner = BL.at(j);
						if(UED::BelongToMeta(r_be_inner.Id, r_be.Id)) {
							Ht.GetByAssoc(r_be_inner.SymbHashId, temp_buf);
							assert(temp_buf.NotEmpty());
							temp_buf.ToUpper();
							(def_symb = "UED").CatChar('_').Cat(meta_symb).CatChar('_').Cat(temp_buf);
							SETMAX(max_symb_len, def_symb.Len());
						}
					}
					gen.IndentDec();
				}
			}
		}
	}
	h_sentinel_def.Z().Cat("__").Cat(ps.Nam).CatChar('_').Cat("h").ToUpper();
	h_sentinel_def.ReplaceChar('-', '_');
	gen.Wr_IfDef(h_sentinel_def, 1);
	gen.Wr_Define(h_sentinel_def, 0);
	gen.WriteBlancLine();
	{
		for(uint i = 0; i < BL.getCount(); i++) {
			const BaseEntry & r_be = BL.at(i);
			if(UED::IsMetaId(r_be.Id)) {
				Ht.GetByAssoc(r_be.SymbHashId, meta_symb);
				assert(meta_symb.NotEmpty());
				meta_symb.ToUpper();
				(def_symb = "UED").CatChar('_').Cat("META").CatChar('_').Cat(meta_symb);
				assert(max_symb_len >= def_symb.Len());
				def_symb.CatCharN(' ', (max_symb_len+1)-def_symb.Len());
				def_value.Z().Cat("0x").CatHex(r_be.Id).Cat("ULL");
				gen.Wr_Define(def_symb, def_value);
				if(r_be.Id != 0x0000000100000001ULL) { // super-meta wich identifies other meta's
					gen.IndentInc();
					for(uint j = 0; j < BL.getCount(); j++) {
						const BaseEntry & r_be_inner = BL.at(j);
						if(UED::BelongToMeta(r_be_inner.Id, r_be.Id)) {
							Ht.GetByAssoc(r_be_inner.SymbHashId, temp_buf);
							assert(temp_buf.NotEmpty());
							temp_buf.ToUpper();
							(def_symb = "UED").CatChar('_').Cat(meta_symb).CatChar('_').Cat(temp_buf);
							assert(max_symb_len >= def_symb.Len());
							def_symb.CatCharN(' ', (max_symb_len+1)-def_symb.Len());
							def_value.Z().Cat("0x").CatHex(r_be_inner.Id).Cat("ULL");
							gen.Wr_Indent();
							gen.Wr_Define(def_symb, def_value);						
						}
					}
					gen.IndentDec();
				}
			}
		}
	}
	gen.WriteBlancLine();
	gen.Wr_EndIf(h_sentinel_def);
	return ok;
}

bool SrUedContainer::GenerateSourceDecl_Java(const char * pFileName)
{
	bool   ok = true;
	SString temp_buf;
	SString def_symb;
	SString def_value;
	SString meta_symb;
	SString h_sentinel_def;
	SFile  genf(pFileName, SFile::mWrite);
	const  SPathStruc ps(pFileName);
	uint   max_symb_len = 0;
	{
		for(uint i = 0; i < BL.getCount(); i++) {
			const BaseEntry & r_be = BL.at(i);
			if(UED::IsMetaId(r_be.Id)) {
				Ht.GetByAssoc(r_be.SymbHashId, meta_symb);
				assert(meta_symb.NotEmpty());
				meta_symb.ToUpper();
				(def_symb = "UED").CatChar('_').Cat("META").CatChar('_').Cat(meta_symb);
				SETMAX(max_symb_len, def_symb.Len());
				if(r_be.Id != 0x0000000100000001ULL) { // super-meta wich identifies other meta's
					for(uint j = 0; j < BL.getCount(); j++) {
						const BaseEntry & r_be_inner = BL.at(j);
						if(UED::BelongToMeta(r_be_inner.Id, r_be.Id)) {
							Ht.GetByAssoc(r_be_inner.SymbHashId, temp_buf);
							assert(temp_buf.NotEmpty());
							temp_buf.ToUpper();
							(def_symb = "UED").CatChar('_').Cat(meta_symb).CatChar('_').Cat(temp_buf);
							SETMAX(max_symb_len, def_symb.Len());
						}
					}
				}
			}
		}
	}
	genf.WriteBlancLine();
	temp_buf.Z().Cat("class").Space().Cat("UED_ID").Space().CatChar('{').CR();
	genf.WriteLine(temp_buf);
	{
		for(uint i = 0; i < BL.getCount(); i++) {
			const BaseEntry & r_be = BL.at(i);
			if(UED::IsMetaId(r_be.Id)) {
				Ht.GetByAssoc(r_be.SymbHashId, meta_symb);
				assert(meta_symb.NotEmpty());
				meta_symb.ToUpper();
				(def_symb = "UED").CatChar('_').Cat("META").CatChar('_').Cat(meta_symb);
				assert(max_symb_len >= def_symb.Len());
				def_symb.CatCharN(' ', (max_symb_len+1)-def_symb.Len());
				def_value.Z().Cat("0x").CatHex(r_be.Id).Cat("L");
				temp_buf.Z().Tab().Cat("public static final long").Space().Cat(def_symb).CatChar('=').Space().Cat(def_value).Semicol().CR();
				genf.WriteLine(temp_buf);
				if(r_be.Id != 0x0000000100000001ULL) { // super-meta wich identifies other meta's
					//gen.IndentInc();
					for(uint j = 0; j < BL.getCount(); j++) {
						const BaseEntry & r_be_inner = BL.at(j);
						if(UED::BelongToMeta(r_be_inner.Id, r_be.Id)) {
							Ht.GetByAssoc(r_be_inner.SymbHashId, temp_buf);
							assert(temp_buf.NotEmpty());
							temp_buf.ToUpper();
							(def_symb = "UED").CatChar('_').Cat(meta_symb).CatChar('_').Cat(temp_buf);
							assert(max_symb_len >= def_symb.Len());
							def_symb.CatCharN(' ', (max_symb_len+1)-def_symb.Len());
							def_value.Z().Cat("0x").CatHex(r_be_inner.Id).Cat("L");
							temp_buf.Z().Tab(2).Cat("public static final long").Space().Cat(def_symb).CatChar('=').Space().Cat(def_value).Semicol().CR();
							genf.WriteLine(temp_buf);
						}
					}
					//gen.IndentDec();
				}
			}
		}
	}
	temp_buf.Z().CatChar('}').CR();
	genf.WriteLine(temp_buf);
	genf.WriteBlancLine();
	//gen.Wr_EndIf(h_sentinel_def);
	return ok;
}
	
int SrUedContainer::Verify(const char * pPath, long ver)
{
	int    ok = 1;
	SString temp_buf;
	THROW(ver > 0);
	MakeUedCanonicalName(temp_buf, ver);
	{
		SString file_path;
		SString hash_file_path;
		binary256 hash;
		(file_path = pPath).SetLastSlash().Cat(temp_buf).Dot().Cat("dat");
		THROW_SL(fileExists(file_path));
		(hash_file_path = pPath).SetLastSlash().Cat(temp_buf).Dot().Cat("sha256");
		THROW_SL(fileExists(hash_file_path));
		{
			SFile f_in(file_path, SFile::mRead|SFile::mBinary);
			SFile f_hash(hash_file_path, SFile::mRead);
			SBinaryChunk bc_hash_stored;
			SBinaryChunk bc_hash_evaluated;
			THROW_SL(f_in.IsValid());
			THROW_SL(f_hash.IsValid());
			THROW_SL(f_in.CalcHash(0, SHASHF_SHA256, bc_hash_evaluated));
			THROW_SL(f_hash.ReadLine(temp_buf));
			THROW_SL(bc_hash_stored.FromHex(temp_buf.Chomp().Strip()));
			THROW(bc_hash_stored == bc_hash_evaluated);
		}
	}
	CATCHZOK
	return ok;
}

#if(_MSC_VER >= 1900)
	using namespace U_ICU_NAMESPACE;
#endif

int Test_ReadUed(const char * pFileName)
{
	int    ok = 1;
	SString temp_buf;
	SStringU temp_buf_u;
#if(_MSC_VER >= 1900)
	{
		SGeoPosLL gp(40.67241045687091, -74.24130029528962);
		uint64 ued_gp = UED::ConvertGeoLoc(gp);
		SGeoPosLL gp_;
		UED::StraightenGeoLoc(ued_gp, gp_);
	}
	{
		const double angle_list[] = { -11.9, -45.0, -180.1, 10.5, 0.0, 30.0, 45.0, 60.0, 180.0, 10.0, 270.25, 359.9 };
		for(uint i = 0; i < SIZEOFARRAY(angle_list); i++) {
			uint64 ued_a = UED::ConvertPlanarAngle_Deg(angle_list[i]);
			double angle_;
			UED::StraightenPlanarAngle_Deg(ued_a, angle_);
			assert(feqeps(angle_, angle_list[i], 1E-6));
		}
		{
			const double src_angle = SMathConst::Pi / 4;
			uint64 ued_a = UED::ConvertPlanarAngle_Deg(src_angle * 180.0/SMathConst::Pi);
			double angle_;
			UED::StraightenPlanarAngle_Deg(ued_a, angle_);
			assert(feqeps(angle_, 45.0, 1E-6));
		}
		{
			const double src_angle = SMathConst::Pi / 6;
			uint64 ued_a = UED::ConvertPlanarAngle_Deg(src_angle * 180.0/SMathConst::Pi);
			double angle_;
			UED::StraightenPlanarAngle_Deg(ued_a, angle_);
			assert(feqeps(angle_, 30.0, 1E-6));
		}
	}
	{
		UErrorCode icu_st = U_ZERO_ERROR;
		/*{
			PPGetPath(PPPATH_BIN, temp_buf);
			u_setDataDirectory(temp_buf);
			u_init(&icu_st);
		}*/
		//
		MeasureUnit avl_units[2048];
		icu_st = U_ZERO_ERROR;
		Locale lcl("ru_RU");
		MeasureFormat mf(lcl, UMEASFMT_WIDTH_WIDE, icu_st);
		int avl_count = MeasureUnit::getAvailable(avl_units, SIZEOFARRAY(avl_units), icu_st);
		//UnicodeString measure_data_buf_list[24]; // really 11 needed
		//SStringU measure_text_list_u[24];
		//MeasureUnit mu("kilogram");
		for(int i = 0; i < avl_count; i++) {
			UnicodeString udn = mf.getUnitDisplayName(avl_units[i], icu_st);
			//getMeasureData(lcl, avl_units[i], UNUM_UNIT_WIDTH_FULL_NAME, ""/*unitDisplayCase*/, measure_data_buf_list, icu_st);
			//for(uint j = 0; j < 11; j++) {
			//	measure_text_list_u[j].Z().CatN(reinterpret_cast<const wchar_t *>(measure_data_buf_list[j].getBuffer()), measure_data_buf_list[j].length());
			//}
			temp_buf_u.Z().CatN(reinterpret_cast<const wchar_t *>(udn.getBuffer()), udn.length());
		}
	}
#endif
	SrUedContainer uedc;
	SString last_file_name;
	long   new_version = 0;
	SPathStruc ps(pFileName);
	ps.Merge(SPathStruc::fDrv|SPathStruc::fDir, temp_buf);
	long   prev_version = SrUedContainer::SearchLastCanonicalFile(temp_buf.RmvLastSlash(), last_file_name);
	if(prev_version > 0)
		new_version = prev_version+1;
	THROW(uedc.ReadSource(pFileName));
	{
		SETIFZQ(new_version, 1);
		SrUedContainer::MakeUedCanonicalName(ps.Nam, new_version);
		ps.Ext = "dat";
		ps.Merge(temp_buf);
		THROW(uedc.WriteSource(temp_buf));
		//
		ps.Merge(SPathStruc::fDir|SPathStruc::fDrv, temp_buf);
		THROW(uedc.Verify(temp_buf, new_version));
	}
	{
		SrUedContainer::MakeUedCanonicalName(ps.Nam, -1);
		ps.Ext = "h";
		ps.Merge(temp_buf);
		THROW(uedc.GenerateSourceDecl_C(temp_buf));
	}
	{
		SrUedContainer::MakeUedCanonicalName(ps.Nam, -1);
		ps.Ext = "java";
		ps.Merge(temp_buf);
		THROW(uedc.GenerateSourceDecl_Java(temp_buf));
	}
	CATCHZOK
	return ok;
}
