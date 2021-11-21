// SPIP.CPP
// Copyright (c) A.Sobolev 2007, 2008, 2009, 2010, 2012, 2015, 2016, 2017, 2018, 2019, 2020, 2021
// Program Interface Paradigm
//
#include <slib-internal.h>
#pragma hdrstop

//uuid(52D5E7CA-F613-4333-A04E-125DE29D715F)
//uuid(52D5E7CAF6134333A04E125DE29D715F)

const S_GUID ZEROGUID; // @v10.9.3

S_GUID_Base & FASTCALL S_GUID_Base::Init(REFIID rIID)
{
	memcpy(Data, &rIID, sizeof(Data));
	return *this;
}

S_GUID_Base::operator GUID & () { return *(GUID *)Data; }
int FASTCALL S_GUID_Base::operator == (const S_GUID_Base & s) const { return BIN(memcmp(Data, s.Data, sizeof(Data)) == 0); }
int FASTCALL S_GUID_Base::operator != (const S_GUID_Base & s) const { return BIN(memcmp(Data, s.Data, sizeof(Data)) != 0); }
bool S_GUID_Base::IsZero() const { return (Data[0] == 0 && Data[1] == 0 && Data[2] == 0 && Data[3] == 0); }

S_GUID_Base & S_GUID_Base::Z()
{
	memzero(Data, sizeof(Data));
	return *this;
}

static char * FASTCALL format_uuid_part(const uint8 * pData, size_t numBytes, int useLower, char * pBuf)
{
	if(useLower)
		switch(numBytes) {
			case 2: sprintf(pBuf, "%04x", *reinterpret_cast<const uint16 *>(pData)); break;
			case 4: sprintf(pBuf, "%08x", *reinterpret_cast<const uint32 *>(pData)); break;
			case 1: sprintf(pBuf, "%02x", *pData); break;
		}
	else
		switch(numBytes) {
			case 2:	sprintf(pBuf, "%04X", *reinterpret_cast<const uint16 *>(pData)); break;
			case 4: sprintf(pBuf, "%08X", *reinterpret_cast<const uint32 *>(pData)); break;
			case 1: sprintf(pBuf, "%02X", *pData); break;
		}
	return pBuf;
}

/* @construction
static void GUID_to_InnerSUID27(const S_GUID_Base & rS, SString & rBuf)
{
	rBuf.Z();
	const char * p_base = "ABCDEFGHIJKLMNOPQRSTUV0123456789";
	const char * p_tail_size_base = "WXYZ";
	char   temp_buf[64];
	uint32 c;
	uint i = 0;
	for(; i < (sizeof(rS) * 8) / 5; i++) {
		c = getbits(&rS, sizeof(rS), (i*5), 5); assert(c < 32); temp_buf[i] = p_base[c];
	}
	const uint tail_size = ((sizeof(rS) * 8) % 5);
	assert(tail_size == 3);
	c = getbits(&rS, sizeof(rS), (i*5), tail_size); assert(c < 32); 
	temp_buf[i++] = p_tail_size_base[tail_size-1];
	temp_buf[i++] = p_base[c];
	temp_buf[i] = 0;
	assert(i == 27);
	assert(i == sstrlen(temp_buf));
	//
	S_GUID debug_guid;
	{
		// @debug
		uint32 debug_buf[64];
		memzero(debug_buf, sizeof(debug_buf));
		uint j = 0;
		for(; j < 25; j++) {
			uint v = temp_buf[j];
			assert((v >= 'A' && v <= 'V') || (v >= '0' && v <= '9'));
			c = (v >= 'A' && v <= 'V') ? (v - 'A') : (v - '0' + ('V'-'A'+1));
			assert(c < 32);
			if(c & 0x01)
				setbit32(debug_buf, sizeof(debug_buf), (j * 5)+0);
			if(c & 0x02)
				setbit32(debug_buf, sizeof(debug_buf), (j * 5)+1);
			if(c & 0x04)
				setbit32(debug_buf, sizeof(debug_buf), (j * 5)+2);
			if(c & 0x08)
				setbit32(debug_buf, sizeof(debug_buf), (j * 5)+3);
			if(c & 0x10)
				setbit32(debug_buf, sizeof(debug_buf), (j * 5)+4);
		}
		assert(temp_buf[j++] == 'Y');
		{
			uint v = temp_buf[j];
			assert((v >= 'A' && v <= 'V') || (v >= '0' && v <= '9'));
			c = (v >= 'A' && v <= 'V') ? (v - 'A') : (v - '0' + ('V'-'A'+1));
			assert(c < 32);
			if(c & 0x01)
				setbit32(debug_buf, sizeof(debug_buf), (j * 5)+0);
			if(c & 0x02)
				setbit32(debug_buf, sizeof(debug_buf), (j * 5)+1);
			if(c & 0x04)
				setbit32(debug_buf, sizeof(debug_buf), (j * 5)+2);
		}
		memcpy(&debug_guid, debug_buf, sizeof(debug_guid));
	}
	rBuf = temp_buf;
}*/

SString & S_GUID_Base::ToStr(long fmt__, SString & rBuf) const
{
	char   temp_buf[64];
	uint   i;
	const  int use_lower = BIN(fmt__ & fmtLower);
	const uint8 * p_data = reinterpret_cast<const uint8 *>(Data);
	rBuf.Z();
	switch(fmt__ & ~fmtLower) {
		case fmtIDL:
			rBuf.Cat(format_uuid_part(p_data, 4, use_lower, temp_buf)).CatChar('-');
			p_data += 4;
			rBuf.Cat(format_uuid_part(p_data, 2, use_lower, temp_buf)).CatChar('-');
			p_data += 2;
			rBuf.Cat(format_uuid_part(p_data, 2, use_lower, temp_buf)).CatChar('-');
			p_data += 2;
			for(i = 0; i < 2; i++) {
				rBuf.Cat(format_uuid_part(p_data, 1, use_lower, temp_buf));
				p_data++;
			}
			rBuf.CatChar('-');
			for(i = 0; i < 6; i++) {
				rBuf.Cat(format_uuid_part(p_data, 1, use_lower, temp_buf));
				p_data++;
			}
			break;
		case fmtPlain:
			rBuf.Cat(format_uuid_part(p_data, 4, use_lower, temp_buf));
			p_data += 4;
			rBuf.Cat(format_uuid_part(p_data, 2, use_lower, temp_buf));
			p_data += 2;
			rBuf.Cat(format_uuid_part(p_data, 2, use_lower, temp_buf));
			p_data += 2;
			for(i = 0; i < 2; i++) {
				rBuf.Cat(format_uuid_part(p_data, 1, use_lower, temp_buf));
				p_data++;
			}
			for(i = 0; i < 6; i++) {
				rBuf.Cat(format_uuid_part(p_data, 1, use_lower, temp_buf));
				p_data++;
			}
			break;
		case fmtC:
			{
				const char * ox = "0x";
				rBuf.Cat(ox).Cat(format_uuid_part(p_data, 4, use_lower, temp_buf)).Comma();
				p_data += 4;
				rBuf.Cat(ox).Cat(format_uuid_part(p_data, 2, use_lower, temp_buf)).Comma();
				p_data += 2;
				rBuf.Cat(ox).Cat(format_uuid_part(p_data, 2, use_lower, temp_buf)).Comma();
				p_data += 2;
				for(uint i = 0; i < 8; i++) {
					rBuf.Cat(ox).Cat(format_uuid_part(p_data, 1, use_lower, temp_buf));
					if(i < 7)
						rBuf.Comma();
					p_data++;
				}
			}
			break;
		// @construction case fmtSUID27: GUID_to_InnerSUID27(*this, rBuf); break;
	}
	return rBuf;
}

int FASTCALL S_GUID_Base::FromStr(const char * pBuf)
{
	int    ok = 1;
	const  char * p = pBuf;
	uint   t = 0;
	char   temp_buf[128];
	if(p) {
		int    start_ch = 0;
		while(*p) {
			char   c = *p;
			if(!oneof2(c, ' ', '\t')) {
				if(oneof2(c, '{', '(')) {
					p++;
					c = *p;
					start_ch = c;
				}
				int    is_valid_ch = 0;
				// @v10.9.8 if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
				if(ishex(c)) { // @v10.9.8 
					temp_buf[t++] = c;
					is_valid_ch = 1;
				}
				else if(t == 32)
					break;
				else if(oneof2(c, '-', ' ')) // ��������� ��� ������������� ����� ���� �������
					is_valid_ch = 1;
				else if(c == '}' && start_ch == '{')
					break;
				else if(c == ')' && start_ch == '(')
					break;
				if(t > (sizeof(temp_buf)-4))
					break;
			}
			p++;
		}
	}
	if(t == 32) {
		size_t i;
		uint8 * p_data = reinterpret_cast<uint8 *>(Data);
		for(i = 0; i < 16; i++)
			p_data[i] = hextobyte(temp_buf + (i << 1));
		for(i = 0; i < 4; i++)
			*reinterpret_cast<uint16 *>(p_data+i*2) = swapw(*PTR16(p_data+i*2));
		*reinterpret_cast<uint32 *>(p_data) = swapdw(*PTR32(p_data));
	}
	else {
		memzero(Data, sizeof(Data));
		ok = SLS.SetError(SLERR_INVGUIDSTR, pBuf);
	}
	return ok;
}

int S_GUID_Base::Generate()
{
#ifdef _DEBUG
	GUID guid;
	if(CoCreateGuid(&guid) == S_OK) {
		Init(guid);
		S_GUID_Base test;
		SString s_buf;
		ToStr(fmtIDL, s_buf);
		test.FromStr(s_buf);
		assert(test == *this);
		return 1;
	}
	else
		return 0;
#else
	return BIN(CoCreateGuid((GUID *)Data) == S_OK);
#endif
}

S_GUID::S_GUID()
{
	Z();
}

S_GUID::S_GUID(SCtrGenerate g)
{
	Generate();
}

S_GUID::S_GUID(const S_GUID_Base & rS)
{
	memcpy(Data, rS.Data, sizeof(Data));
}

S_GUID::S_GUID(const S_GUID & rS)
{
	memcpy(Data, rS.Data, sizeof(Data));
}

S_GUID::S_GUID(const char * pStr)
{
	FromStr(pStr);
}

S_GUID & FASTCALL S_GUID::operator = (const S_GUID_Base & rS)
{
	memcpy(Data, rS.Data, sizeof(Data));
	return *this;
}
//
//
//
SVerT::SVerT(int j, int n, int r)
{
	Set(j, n, r);
}

SVerT::operator uint32() const
{
	return ((static_cast<uint32>(V) << 16) | R);
}

void SVerT::Set(uint32 n)
{
	V = static_cast<uint16>(n >> 16);
	R = static_cast<uint16>(n & 0x0000ffff);
}

int SVerT::GetMajor() const { return static_cast<int>(V >> 8); }
int SVerT::GetMinor() const { return static_cast<int>(V & 0x00ff); }
int SVerT::GetRevision() const { return static_cast<int>(R); }

int SVerT::Get(int * pJ, int * pN, int * pR) const
{
	int j = static_cast<int>(V >> 8);
	int n = static_cast<int>(V & 0x00ff);
	int r = static_cast<int>(R);
	ASSIGN_PTR(pJ, j);
	ASSIGN_PTR(pN, n);
	ASSIGN_PTR(pR, r);
	return 1;
}

void SVerT::Set(int j, int n, int r)
{
	V = (((uint16)j) << 8) | ((uint16)n);
	R = (uint16)r;
}

bool SVerT::IsLt(int j, int n, int r) const
{
	SVerT v2(j, n, r);
	return (V < v2.V || (V == v2.V && R < v2.R));
}

bool SVerT::IsGt(int j, int n, int r) const
{
	SVerT v2(j, n, r);
	return ((V > v2.V) || (V == v2.V && R > v2.R));
}

bool SVerT::IsGe(int j, int n, int r) const
{
	SVerT v2(j, n, r);
	return ((V > v2.V) || (V == v2.V && R >= v2.R));
}

bool SVerT::IsEq(int j, int n, int r) const
{
	SVerT v2(j, n, r);
	return (V == v2.V && R == v2.R);
}

int FASTCALL SVerT::Cmp(const SVerT * pVer) const
{
	int ok = 0;
	if(pVer) {
		if(V > pVer->V)
			ok = 1;
		else if(V < pVer->V)
			ok = -1;
		else if(R > pVer->R)
			ok = 1;
		else if(R < pVer->R)
			ok = -1;
		else
			ok = 0;
	}
	return ok;
}

SString FASTCALL SVerT::ToStr(SString & rBuf) const
{
	int    j, n, r;
	Get(&j, &n, &r);
	return rBuf.Z().CatDotTriplet(j, n, r);
}

int FASTCALL SVerT::FromStr(const char * pStr)
{
	int    ok = 0;
	int    j = 0, n = 0, r = 0;
	SString temp_buf;
	SStrScan scan(pStr);
	Set(0, 0, 0);
	if(scan.Skip().GetDigits(temp_buf)) {
		j = (int)temp_buf.ToLong();
		if(scan.Skip()[0] == '.') {
			scan.Incr();
			if(scan.Skip().GetDigits(temp_buf)) {
				n = (int)temp_buf.ToLong();
				if(scan.Skip()[0] == '.') {
					scan.Incr();
					if(scan.Skip().GetDigits(temp_buf)) {
						r = (int)temp_buf.ToLong();
						Set(j, n, r);
						ok = 3;
					}
				}
				else {
					Set(j, n, 0);
					ok = 2;
				}
			}
		}
	}
	return ok;
}

int SVerT::Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx)
{
	int    ok = 1;
	THROW(pCtx->Serialize(dir, V, rBuf));
	THROW(pCtx->Serialize(dir, R, rBuf));
	CATCHZOK
	return ok;
}
