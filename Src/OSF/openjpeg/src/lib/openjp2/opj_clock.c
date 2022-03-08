/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
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
#ifdef _WIN32
#else
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/times.h>
#endif /* _WIN32 */

double opj_clock(void)
{
#ifdef _WIN32
	/* _WIN32: use QueryPerformance (very accurate) */
	LARGE_INTEGER freq, t;
	/* freq is the clock speed of the CPU */
	QueryPerformanceFrequency(&freq);
	/* cout << "freq = " << ((double) freq.QuadPart) << endl; */
	/* t is the high resolution performance counter (see MSDN) */
	QueryPerformanceCounter(&t);
	return ((double)t.QuadPart / (double)freq.QuadPart);
#else
	/* Unix or Linux: use resource usage */
	struct rusage t;
	double procTime;
	/* (1) Get the rusage data structure at this moment (man getrusage) */
	getrusage(0, &t);
	/* (2) What is the elapsed time ? - CPU time = User time + System time */
	/* (2a) Get the seconds */
	procTime = (double)(t.ru_utime.tv_sec + t.ru_stime.tv_sec);
	/* (2b) More precisely! Get the microseconds part ! */
	return (procTime + (double)(t.ru_utime.tv_usec + t.ru_stime.tv_usec) * 1e-6);
#endif
}
