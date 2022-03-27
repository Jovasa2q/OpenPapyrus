// SLIB-INTERNAL.H
// Copyright (c) A.Sobolev 2020, 2022
//
#ifndef __SLIB_INTERNAL_H
#define __SLIB_INTERNAL_H
	#include <slib.h>
	#include <tv.h>

FORCEINLINE uint implement_iseol(const char * pIn, SEOLFormat eolf)
{
	const char c = pIn[0];
	if(oneof2(c, '\x0A', '\x0D')) {
		switch(eolf) {
			case eolAny: // @v11.3.6 ����� ���������� ('\x0D'; '\x0A'; '\x0D\x0A'; '\x0A\x0D') ���������� ��� ����� ������
				return ((c == '\x0D' && pIn[1] == '\x0A') || (c == '\x0A' && pIn[1] == '\x0D')) ? 2 : 1;
			case eolUnix: return BIN(c == '\x0A');
			case eolMac: return BIN(c == '\x0D');
			case eolWindows: return (c == '\x0D' && (pIn[1] == '\x0A')) ? 2 : 0;
			case eolUndef: return (c == '\x0D' && (pIn[1] == '\x0A')) ? 2 : 1;
			case eolSpcICalendar: // @v11.0.3 (���� ����� \xD\xA ���� ������, �� ��� - ����������� ������� ������, �� ����������� �������� caller
				return (c == '\x0D' && (pIn[1] == '\x0A')) ? 2 : 0;
		}
	}
	return 0;
}

#endif // __SLIB_INTERNAL_H

