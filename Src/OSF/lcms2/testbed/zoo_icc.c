//
//  Little Color Management System
//  Copyright (c) 1998-2020 Marti Maria Saguer
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software
// is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
#include "lcms2_internal.h"
#pragma hdrstop
#include "testcms2.h"
//
// ZOO checks
//
#ifdef CMS_IS_WINDOWS_

static char ZOOfolder[cmsMAX_PATH] = "c:\\colormaps\\";
static char ZOOwrite[cmsMAX_PATH]  = "c:\\colormaps\\write\\";
static char ZOORawWrite[cmsMAX_PATH]  = "c:\\colormaps\\rawwrite\\";

// Read all tags on a profile given by its handle
static void ReadAllTags(cmsHPROFILE h)
{
	int32 i;
	cmsTagSignature sig;
	int32 n = cmsGetTagCount(h);
	for(i = 0; i < n; i++) {
		sig = cmsGetTagSignature(h, i);
		if(cmsReadTag(h, sig) == NULL) return;
	}
}

// Read all tags on a profile given by its handle
static void ReadAllRAWTags(cmsHPROFILE h)
{
	cmsTagSignature sig;
	int32 len;
	int32 n = cmsGetTagCount(h);
	for(int32 i = 0; i < n; i++) {
		sig = cmsGetTagSignature(h, i);
		len = cmsReadRawTag(h, sig, NULL, 0);
	}
}

static void PrintInfo(cmsHPROFILE h, cmsInfoType Info)
{
	const int32 len = cmsGetProfileInfo(h, Info, "en", "US", NULL, 0);
	if(len > 0) {
		cmsContext id = 0;
		wchar_t * text = (wchar_t *)_cmsMalloc(id, len);
		cmsGetProfileInfo(h, Info, "en", "US", text, len);
		wprintf(L"%s\n", text);
		_cmsFree(id, text);
	}
}

static void PrintAllInfos(cmsHPROFILE h)
{
	PrintInfo(h, cmsInfoDescription);
	PrintInfo(h, cmsInfoManufacturer);
	PrintInfo(h, cmsInfoModel);
	PrintInfo(h, cmsInfoCopyright);
	printf("\n\n");
}

static void ReadAllLUTS(cmsHPROFILE h)
{
	cmsCIEXYZ Black;
	cmsPipeline * a = _cmsReadInputLUT(h, INTENT_PERCEPTUAL);
	cmsPipelineFree(a);
	a = _cmsReadInputLUT(h, INTENT_RELATIVE_COLORIMETRIC);
	cmsPipelineFree(a);
	a = _cmsReadInputLUT(h, INTENT_SATURATION);
	cmsPipelineFree(a);
	a = _cmsReadInputLUT(h, INTENT_ABSOLUTE_COLORIMETRIC);
	cmsPipelineFree(a);
	a = _cmsReadOutputLUT(h, INTENT_PERCEPTUAL);
	cmsPipelineFree(a);
	a = _cmsReadOutputLUT(h, INTENT_RELATIVE_COLORIMETRIC);
	cmsPipelineFree(a);
	a = _cmsReadOutputLUT(h, INTENT_SATURATION);
	cmsPipelineFree(a);
	a = _cmsReadOutputLUT(h, INTENT_ABSOLUTE_COLORIMETRIC);
	cmsPipelineFree(a);
	a = _cmsReadDevicelinkLUT(h, INTENT_PERCEPTUAL);
	cmsPipelineFree(a);
	a = _cmsReadDevicelinkLUT(h, INTENT_RELATIVE_COLORIMETRIC);
	cmsPipelineFree(a);
	a = _cmsReadDevicelinkLUT(h, INTENT_SATURATION);
	cmsPipelineFree(a);
	a = _cmsReadDevicelinkLUT(h, INTENT_ABSOLUTE_COLORIMETRIC);
	cmsPipelineFree(a);
	cmsDetectDestinationBlackPoint(&Black, h, INTENT_PERCEPTUAL, 0);
	cmsDetectDestinationBlackPoint(&Black, h, INTENT_RELATIVE_COLORIMETRIC, 0);
	cmsDetectDestinationBlackPoint(&Black, h, INTENT_SATURATION, 0);
	cmsDetectDestinationBlackPoint(&Black, h, INTENT_ABSOLUTE_COLORIMETRIC, 0);
	cmsDetectTAC(h);
}

// Check one specimen in the ZOO

static int32 CheckSingleSpecimen(const char * Profile)
{
	char buff_src[256];
	char buff_dst[256];
	cmsHPROFILE h;
	sprintf(buff_src, "%s%s", ZOOfolder, Profile);
	sprintf(buff_dst, "%s%s", ZOOwrite,  Profile);
	h = cmsOpenProfileFromFile(buff_src, "r");
	if(h == NULL) return 0;
	printf("%s\n", Profile);
	PrintAllInfos(h);
	ReadAllTags(h);
	ReadAllLUTS(h);
	// ReadAllRAWTags(h);
	cmsSaveProfileToFile(h, buff_dst);
	cmsCloseProfile(h);
	h = cmsOpenProfileFromFile(buff_dst, "r");
	if(h == NULL) 
		return 0;
	ReadAllTags(h);
	cmsCloseProfile(h);
	return 1;
}

static int32 CheckRAWSpecimen(const char * Profile)
{
	char buff_src[256];
	char buff_dst[256];
	cmsHPROFILE h;
	sprintf(buff_src, "%s%s", ZOOfolder, Profile);
	sprintf(buff_dst, "%s%s", ZOORawWrite,  Profile);
	h = cmsOpenProfileFromFile(buff_src, "r");
	if(h == NULL) return 0;
	ReadAllTags(h);
	ReadAllRAWTags(h);
	cmsSaveProfileToFile(h, buff_dst);
	cmsCloseProfile(h);
	h = cmsOpenProfileFromFile(buff_dst, "r");
	if(h == NULL) return 0;
	ReadAllTags(h);
	cmsCloseProfile(h);
	return 1;
}

static int input = 0;
static int disp = 0;
static int output = 0;
static int link = 0;
static int abst = 0;
static int color = 0;
static int named = 0;

static int rgb = 0;
static int cmyk = 0;
static int gray = 0;
static int other = 0;

static int count_stats(const char * Profile)
{
	char buff_src[256];
	cmsHPROFILE h;
	cmsCIEXYZ Black;
	sprintf(buff_src, "%s%s", ZOOfolder, Profile);
	h = cmsOpenProfileFromFile(buff_src, "r");
	if(h == NULL) return 0;
	switch(cmsGetDeviceClass(h)) {
		case cmsSigInputClass: input++; break;
		case cmsSigDisplayClass: disp++; break;
		case cmsSigOutputClass: output++; break;
		case cmsSigLinkClass: link++;  break;
		case cmsSigAbstractClass: abst++; break;
		case cmsSigColorSpaceClass: color++; break;
		case cmsSigNamedColorClass: named++; break;
	}
	switch(cmsGetColorSpace(h)) {
		case cmsSigRgbData: rgb++; break;
		case cmsSigCmykData: cmyk++; break;
		case cmsSigGrayData: gray++; break;
		default: other++;
	}
	cmsDetectDestinationBlackPoint(&Black, h, INTENT_PERCEPTUAL, 0);
	cmsDetectDestinationBlackPoint(&Black, h, INTENT_RELATIVE_COLORIMETRIC, 0);
	cmsDetectDestinationBlackPoint(&Black, h, INTENT_SATURATION, 0);
	cmsCloseProfile(h);
	return 1;
}

void CheckProfileZOO(FILE * fOut)
{
	struct _finddata_t c_file;
	intptr_t hFile;
	cmsSetLogErrorHandler(NULL);
	if((hFile = _findfirst(/*"c:\\colormaps\\*.*"*/"/Papyrus/Src/PPTEST/DATA/colormaps/*.*", &c_file)) == -1L)
		fprintf(fOut, "No files in current directory");
	else {
		do {
			if(!sstreq(c_file.name, ".") && !sstreq(c_file.name, "..")) {
				CheckSingleSpecimen(c_file.name);
				CheckRAWSpecimen(c_file.name);
				count_stats(c_file.name);
				TestMemoryLeaks(fOut, FALSE);
			}
		} while(_findnext(hFile, &c_file) == 0);
		_findclose(hFile);
	}
	ResetFatalError();
}

#endif
