// SASLDB_INIT.C
//
#include <sasl-internal.h>
#pragma hdrstop
#ifndef macintosh
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <saslplug.h>
#include <saslutil.h>
#include "plugin_common.h"

#ifdef WIN32
	BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
	{
		switch(ul_reason_for_call) {
			case DLL_PROCESS_ATTACH:
			case DLL_THREAD_ATTACH:
			case DLL_THREAD_DETACH:
			case DLL_PROCESS_DETACH:
				break;
		}
		return TRUE;
	}
#endif

SASL_AUXPROP_PLUG_INIT(sasldb)
