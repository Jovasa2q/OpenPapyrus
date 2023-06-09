/* Generated by Snowball 2.0.0 - https://snowballstem.org/ */


#include <xapian-internal.h>
#pragma hdrstop
#include "indonesian.sbl.h"

static int tr_VOWEL(Xapian::StemImplementation * this_ptr) { return (static_cast<Xapian::InternalStemIndonesian *>(this_ptr))->r_VOWEL(); }

static int tr_SUFFIX_I_OK(Xapian::StemImplementation * this_ptr) { return (static_cast<Xapian::InternalStemIndonesian *>(this_ptr))->r_SUFFIX_I_OK(); }

static int tr_SUFFIX_AN_OK(Xapian::StemImplementation * this_ptr) { return (static_cast<Xapian::InternalStemIndonesian *>(this_ptr))->r_SUFFIX_AN_OK(); }

static int tr_SUFFIX_KAN_OK(Xapian::StemImplementation * this_ptr) { return (static_cast<Xapian::InternalStemIndonesian *>(this_ptr))->r_SUFFIX_KAN_OK(); }

static int tr_KER(Xapian::StemImplementation * this_ptr) { return (static_cast<Xapian::InternalStemIndonesian *>(this_ptr))->r_KER(); }

static const among_function af[5] =
{
/*  1 */ tr_VOWEL,
/*  2 */ tr_SUFFIX_I_OK,
/*  3 */ tr_SUFFIX_AN_OK,
/*  4 */ tr_SUFFIX_KAN_OK,
/*  5 */ tr_KER
};

static const symbol s_pool[] = {
#define s_0_0 0
'k', 'a', 'h',
#define s_0_1 3
'l', 'a', 'h',
#define s_0_2 6
'p', 'u', 'n',
#define s_1_0 9
'n', 'y', 'a',
#define s_1_1 12
'k', 'u',
#define s_1_2 14
'm', 'u',
#define s_2_0 16
'i',
#define s_2_1 (s_2_2 + 1)
#define s_2_2 17
'k', 'a', 'n',
#define s_3_0 20
'd', 'i',
#define s_3_1 22
'k', 'e',
#define s_3_2 s_3_3
#define s_3_3 24
'm', 'e', 'm',
#define s_3_4 s_3_5
#define s_3_5 27
'm', 'e', 'n', 'g',
#define s_3_6 31
'm', 'e', 'n', 'y',
#define s_3_7 35
'p', 'e', 'm',
#define s_3_8 s_3_9
#define s_3_9 38
'p', 'e', 'n', 'g',
#define s_3_10 42
'p', 'e', 'n', 'y',
#define s_3_11 46
't', 'e', 'r',
#define s_4_0 s_4_1
#define s_4_1 49
'b', 'e', 'l', 'a', 'j', 'a', 'r',
#define s_4_2 56
'b', 'e', 'r',
#define s_4_3 s_4_4
#define s_4_4 59
'p', 'e', 'l', 'a', 'j', 'a', 'r',
#define s_4_5 66
'p', 'e', 'r',
};


static const Among a_0[3] = {
/*  0 */ { 3, s_0_0, -1, 1},
/*  1 */ { 3, s_0_1, -1, 1},
/*  2 */ { 3, s_0_2, -1, 1}
};


static const Among a_1[3] = {
/*  0 */ { 3, s_1_0, -1, 1},
/*  1 */ { 2, s_1_1, -1, 1},
/*  2 */ { 2, s_1_2, -1, 1}
};


static const Among a_2[3] = {
/*  0 */ { 1, s_2_0, -1, 1},
/*  1 */ { 2, s_2_1, -1, 1},
/*  2 */ { 3, s_2_2, 1, 1}
};

static const uchar af_2[3] = {
/*  0 */ 2 /* tr_SUFFIX_I_OK */,
/*  1 */ 3 /* tr_SUFFIX_AN_OK */,
/*  2 */ 4 /* tr_SUFFIX_KAN_OK */
};


static const Among a_3[12] = {
/*  0 */ { 2, s_3_0, -1, 1},
/*  1 */ { 2, s_3_1, -1, 2},
/*  2 */ { 2, s_3_2, -1, 1},
/*  3 */ { 3, s_3_3, 2, 5},
/*  4 */ { 3, s_3_4, 2, 1},
/*  5 */ { 4, s_3_5, 4, 1},
/*  6 */ { 4, s_3_6, 4, 3},
/*  7 */ { 3, s_3_7, -1, 6},
/*  8 */ { 3, s_3_8, -1, 2},
/*  9 */ { 4, s_3_9, 8, 2},
/* 10 */ { 4, s_3_10, 8, 4},
/* 11 */ { 3, s_3_11, -1, 1}
};

static const uchar af_3[12] = {
/*  0 */ 0,
/*  1 */ 0,
/*  2 */ 0,
/*  3 */ 0,
/*  4 */ 0,
/*  5 */ 0,
/*  6 */ 1 /* tr_VOWEL */,
/*  7 */ 0,
/*  8 */ 0,
/*  9 */ 0,
/* 10 */ 1 /* tr_VOWEL */,
/* 11 */ 0
};


static const Among a_4[6] = {
/*  0 */ { 2, s_4_0, -1, 3},
/*  1 */ { 7, s_4_1, 0, 4},
/*  2 */ { 3, s_4_2, 0, 3},
/*  3 */ { 2, s_4_3, -1, 1},
/*  4 */ { 7, s_4_4, 3, 2},
/*  5 */ { 3, s_4_5, 3, 1}
};

static const uchar af_4[6] = {
/*  0 */ 5 /* tr_KER */,
/*  1 */ 0,
/*  2 */ 0,
/*  3 */ 0,
/*  4 */ 0,
/*  5 */ 0
};

static const uchar g_vowel[] = { 17, 65, 16 };

static const symbol s_0[] = { 'e', 'r' };
static const symbol s_1[] = { 's' };
static const symbol s_2[] = { 's' };
static const symbol s_3[] = { 'p' };
static const symbol s_4[] = { 'p' };
static const symbol s_5[] = { 'a', 'j', 'a', 'r' };
static const symbol s_6[] = { 'a', 'j', 'a', 'r' };

int Xapian::InternalStemIndonesian::r_remove_particle() {
	ket = c;
	if(c - 2 <= lb || (p[c-1] != 104 && p[c-1] != 110)) return 0;
	if(!(find_among_b(s_pool, a_0, 3, 0, 0))) return 0;
	bra = c;
	{    int ret = slice_del();
		if(ret < 0) return ret;
	}
	I_measure -= 1;
	return 1;
}

int Xapian::InternalStemIndonesian::r_remove_possessive_pronoun() {
	ket = c;
	if(c - 1 <= lb || (p[c-1] != 97 && p[c-1] != 117)) return 0;
	if(!(find_among_b(s_pool, a_1, 3, 0, 0))) return 0;
	bra = c;
	{    int ret = slice_del();
		if(ret < 0) return ret;
	}
	I_measure -= 1;
	return 1;
}

int Xapian::InternalStemIndonesian::r_SUFFIX_KAN_OK() {
	
	if(!(I_prefix != 3)) return 0;
	if(!(I_prefix != 2)) return 0;
	return 1;
}

int Xapian::InternalStemIndonesian::r_SUFFIX_AN_OK() {
	if(!(I_prefix != 1)) return 0;
	return 1;
}

int Xapian::InternalStemIndonesian::r_SUFFIX_I_OK() {
	if(!(I_prefix <= 2)) return 0;
	{   int m1 = l - c; (void)m1;
		if(c <= lb || p[c - 1] != 's') goto lab0;
		c--;
		return 0;
	lab0:
		c = l - m1;
	}
	return 1;
}

int Xapian::InternalStemIndonesian::r_remove_suffix() {
	ket = c;
	if(c <= lb || (p[c-1] != 105 && p[c-1] != 110)) return 0;
	if(!(find_among_b(s_pool, a_2, 3, af_2, af))) return 0;
	bra = c;
	{    int ret = slice_del();
		if(ret < 0) return ret;
	}
	I_measure -= 1;
	return 1;
}

int Xapian::InternalStemIndonesian::r_VOWEL() {
	if(in_grouping_U(g_vowel, 97, 117, 0)) return 0;
	return 1;
}

int Xapian::InternalStemIndonesian::r_KER() {
	if(out_grouping_U(g_vowel, 97, 117, 0)) return 0;
	if(!(eq_s(2, s_0))) return 0;
	return 1;
}

int Xapian::InternalStemIndonesian::r_remove_first_order_prefix() {
	int among_var;
	bra = c;
	if(c + 1 >= l || (p[c + 1] != 105 && p[c + 1] != 101)) return 0;
	among_var = find_among(s_pool, a_3, 12, af_3, af);
	if(!(among_var)) return 0;
	ket = c;
	switch(among_var) {
		case 1:
			{    int ret = slice_del();
				if(ret < 0) return ret;
			}
			I_prefix = 1;
			I_measure -= 1;
			break;
		case 2:
			{    int ret = slice_del();
				if(ret < 0) return ret;
			}
			I_prefix = 3;
			I_measure -= 1;
			break;
		case 3:
			I_prefix = 1;
			{    int ret = slice_from_s(1, s_1);
				if(ret < 0) return ret;
			}
			I_measure -= 1;
			break;
		case 4:
			I_prefix = 3;
			{    int ret = slice_from_s(1, s_2);
				if(ret < 0) return ret;
			}
			I_measure -= 1;
			break;
		case 5:
			I_prefix = 1;
			I_measure -= 1;
			{   int c1 = c;
				{   int c2 = c;
					if(in_grouping_U(g_vowel, 97, 117, 0)) goto lab1;
					c = c2;
					{    int ret = slice_from_s(1, s_3);
						if(ret < 0) return ret;
					}
				}
				goto lab0;
			lab1:
				c = c1;
				{    int ret = slice_del();
					if(ret < 0) return ret;
				}
			}
		lab0:
			break;
		case 6:
			I_prefix = 3;
			I_measure -= 1;
			{   int c3 = c;
				{   int c4 = c;
					if(in_grouping_U(g_vowel, 97, 117, 0)) goto lab3;
					c = c4;
					{    int ret = slice_from_s(1, s_4);
						if(ret < 0) return ret;
					}
				}
				goto lab2;
			lab3:
				c = c3;
				{    int ret = slice_del();
					if(ret < 0) return ret;
				}
			}
		lab2:
			break;
	}
	return 1;
}

int Xapian::InternalStemIndonesian::r_remove_second_order_prefix() {
	int among_var;
	bra = c;
	if(c + 1 >= l || p[c + 1] != 101) return 0;
	among_var = find_among(s_pool, a_4, 6, af_4, af);
	if(!(among_var)) return 0;
	ket = c;
	switch(among_var) {
		case 1:
			{    int ret = slice_del();
				if(ret < 0) return ret;
			}
			I_prefix = 2;
			I_measure -= 1;
			break;
		case 2:
			{    int ret = slice_from_s(4, s_5);
				if(ret < 0) return ret;
			}
			I_measure -= 1;
			break;
		case 3:
			{    int ret = slice_del();
				if(ret < 0) return ret;
			}
			I_prefix = 4;
			I_measure -= 1;
			break;
		case 4:
			{    int ret = slice_from_s(4, s_6);
				if(ret < 0) return ret;
			}
			I_prefix = 4;
			I_measure -= 1;
			break;
	}
	return 1;
}

int Xapian::InternalStemIndonesian::stem() {
	I_measure = 0;
	{   int c1 = c;
		while(1) {
			int c2 = c;
			{   
				int ret = out_grouping_U(g_vowel, 97, 117, 1);
				if(ret < 0) goto lab1;
				c += ret;
			}
			I_measure += 1;
			continue;
		lab1:
			c = c2;
			break;
		}
		c = c1;
	}
	if(!(I_measure > 2)) return 0;
	I_prefix = 0;
	lb = c; c = l;

	{   int m3 = l - c; (void)m3;
		{   int ret = r_remove_particle();
			if(ret < 0) return ret;
		}
		c = l - m3;
	}
	if(!(I_measure > 2)) return 0;
	{   int m4 = l - c; (void)m4;
		{   int ret = r_remove_possessive_pronoun();
			if(ret < 0) return ret;
		}
		c = l - m4;
	}
	c = lb;
	if(!(I_measure > 2)) return 0;
	{   int c5 = c;
		{   int c_test6 = c;
			{    int ret = r_remove_first_order_prefix();
				if(ret == 0) goto lab3;
				if(ret < 0) return ret;
			}
			{   int c7 = c;
				{   int c_test8 = c;
					if(!(I_measure > 2)) goto lab4;
					lb = c; c = l;

					{    int ret = r_remove_suffix();
						if(ret == 0) goto lab4;
						if(ret < 0) return ret;
					}
					c = lb;
					c = c_test8;
				}
				if(!(I_measure > 2)) goto lab4;
				{    int ret = r_remove_second_order_prefix();
					if(ret == 0) goto lab4;
					if(ret < 0) return ret;
				}
			lab4:
				c = c7;
			}
			c = c_test6;
		}
		goto lab2;
	lab3:
		c = c5;
		{   int c9 = c;
			{   int ret = r_remove_second_order_prefix();
				if(ret < 0) return ret;
			}
			c = c9;
		}
		{   int c10 = c;
			if(!(I_measure > 2)) goto lab5;
			lb = c; c = l;

			{    int ret = r_remove_suffix();
				if(ret == 0) goto lab5;
				if(ret < 0) return ret;
			}
			c = lb;
		lab5:
			c = c10;
		}
	}
lab2:
	return 1;
}

Xapian::InternalStemIndonesian::InternalStemIndonesian()
    : I_prefix(0), I_measure(0)
{
}

Xapian::InternalStemIndonesian::~InternalStemIndonesian()
{
}

std::string Xapian::InternalStemIndonesian::get_description() const { return "indonesian.sbl"; }
