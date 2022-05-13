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

// Auxiliary: append a Lab identity after the given sequence of profiles
// and return the transform. Lab profile is closed, rest of profiles are kept open.
cmsHTRANSFORM _cmsChain2Lab(cmsContext ContextID, uint32 nProfiles, uint32 InputFormat, uint32 OutputFormat,
    const uint32 Intents[], const cmsHPROFILE hProfiles[], const boolint BPC[], const double AdaptationStates[], uint32 dwFlags)
{
	cmsHTRANSFORM xform;
	cmsHPROFILE hLab;
	cmsHPROFILE ProfileList[256];
	boolint BPCList[256];
	double AdaptationList[256];
	uint32 IntentList[256];
	uint32 i;
	// This is a rather big number and there is no need of dynamic memory
	// since we are adding a profile, 254 + 1 = 255 and this is the limit
	if(nProfiles > 254) 
		return NULL;
	// The output space
	hLab = cmsCreateLab4ProfileTHR(ContextID, NULL);
	if(hLab == NULL) return NULL;

	// Create a copy of parameters
	for(i = 0; i < nProfiles; i++) {
		ProfileList[i]    = hProfiles[i];
		BPCList[i]        = BPC[i];
		AdaptationList[i] = AdaptationStates[i];
		IntentList[i]     = Intents[i];
	}

	// Place Lab identity at chain's end.
	ProfileList[nProfiles]    = hLab;
	BPCList[nProfiles]        = 0;
	AdaptationList[nProfiles] = 1.0;
	IntentList[nProfiles]     = INTENT_RELATIVE_COLORIMETRIC;

	// Create the transform
	xform = cmsCreateExtendedTransform(ContextID, nProfiles + 1, ProfileList,
		BPCList, IntentList, AdaptationList, NULL, 0, InputFormat, OutputFormat, dwFlags);
	cmsCloseProfile(hLab);
	return xform;
}

// Compute K -> L* relationship. Flags may include black point compensation. In this case,
// the relationship is assumed from the profile with BPC to a black point zero.
static cmsToneCurve * ComputeKToLstar(cmsContext ContextID, uint32 nPoints, uint32 nProfiles,
    const uint32 Intents[], const cmsHPROFILE hProfiles[], const boolint BPC[], const double AdaptationStates[], uint32 dwFlags)
{
	cmsToneCurve * out = NULL;
	uint32 i;
	cmsHTRANSFORM xform;
	cmsCIELab Lab;
	float cmyk[4];
	float* SampledPoints;
	xform = _cmsChain2Lab(ContextID, nProfiles, TYPE_CMYK_FLT, TYPE_Lab_DBL, Intents, hProfiles, BPC, AdaptationStates, dwFlags);
	if(xform == NULL) return NULL;
	SampledPoints = (float *)_cmsCalloc(ContextID, nPoints, sizeof(float));
	if(SampledPoints  == NULL) goto Error;
	for(i = 0; i < nPoints; i++) {
		cmyk[0] = 0;
		cmyk[1] = 0;
		cmyk[2] = 0;
		cmyk[3] = (float)((i * 100.0) / (nPoints-1));
		cmsDoTransform(xform, cmyk, &Lab, 1);
		SampledPoints[i] = (float)(1.0 - Lab.L / 100.0); // Negate K for easier operation
	}
	out = cmsBuildTabulatedToneCurveFloat(ContextID, nPoints, SampledPoints);
Error:
	cmsDeleteTransform(xform);
	_cmsFree(ContextID, SampledPoints);
	return out;
}

// Compute Black tone curve on a CMYK -> CMYK transform. This is done by
// using the proof direction on both profiles to find K->L* relationship
// then joining both curves. dwFlags may include black point compensation.
cmsToneCurve * _cmsBuildKToneCurve(cmsContext ContextID,
    uint32 nPoints,
    uint32 nProfiles,
    const uint32 Intents[],
    const cmsHPROFILE hProfiles[],
    const boolint BPC[],
    const double AdaptationStates[],
    uint32 dwFlags)
{
	cmsToneCurve * in, * out, * KTone;

	// Make sure CMYK -> CMYK
	if(cmsGetColorSpace(hProfiles[0]) != cmsSigCmykData ||
	    cmsGetColorSpace(hProfiles[nProfiles-1])!= cmsSigCmykData) return NULL;

	// Make sure last is an output profile
	if(cmsGetDeviceClass(hProfiles[nProfiles - 1]) != cmsSigOutputClass) return NULL;

	// Create individual curves. BPC works also as each K to L* is
	// computed as a BPC to zero black point in case of L*
	in  = ComputeKToLstar(ContextID, nPoints, nProfiles - 1, Intents, hProfiles, BPC, AdaptationStates, dwFlags);
	if(in == NULL) return NULL;

	out = ComputeKToLstar(ContextID, nPoints, 1,
		Intents + (nProfiles - 1),
		&hProfiles [nProfiles - 1],
		BPC + (nProfiles - 1),
		AdaptationStates + (nProfiles - 1),
		dwFlags);
	if(out == NULL) {
		cmsFreeToneCurve(in);
		return NULL;
	}

	// Build the relationship. This effectively limits the maximum accuracy to 16 bits, but
	// since this is used on black-preserving LUTs, we are not losing  accuracy in any case
	KTone = cmsJoinToneCurve(ContextID, in, out, nPoints);

	// Get rid of components
	cmsFreeToneCurve(in); cmsFreeToneCurve(out);

	// Something went wrong...
	if(KTone == NULL) return NULL;

	// Make sure it is monotonic
	if(!cmsIsToneCurveMonotonic(KTone)) {
		cmsFreeToneCurve(KTone);
		return NULL;
	}

	return KTone;
}

// Gamut LUT Creation -----------------------------------------------------------------------------------------

// Used by gamut & softproofing

typedef struct {
	cmsHTRANSFORM hInput;           // From whatever input color space. 16 bits to DBL
	cmsHTRANSFORM hForward, hReverse; // Transforms going from Lab to colorant and back
	double Thereshold;    // The thereshold after which is considered out of gamut
} GAMUTCHAIN;

// This sampler does compute gamut boundaries by comparing original
// values with a transform going back and forth. Values above ERR_THERESHOLD
// of maximum are considered out of gamut.

#define ERR_THERESHOLD      5

static boolint GamutSampler(const uint16 In[], uint16 Out[], void * Cargo)
{
	GAMUTCHAIN*  t = (GAMUTCHAIN*)Cargo;
	cmsCIELab LabIn1, LabOut1;
	cmsCIELab LabIn2, LabOut2;
	uint16 Proof[cmsMAXCHANNELS], Proof2[cmsMAXCHANNELS];
	double dE1, dE2, ErrorRatio;
	// Assume in-gamut by default.
	ErrorRatio = 1.0;
	// Convert input to Lab
	cmsDoTransform(t->hInput, In, &LabIn1, 1);
	// converts from PCS to colorant. This always
	// does return in-gamut values,
	cmsDoTransform(t->hForward, &LabIn1, Proof, 1);
	// Now, do the inverse, from colorant to PCS.
	cmsDoTransform(t->hReverse, Proof, &LabOut1, 1);
	memmove(&LabIn2, &LabOut1, sizeof(cmsCIELab));
	// Try again, but this time taking Check as input
	cmsDoTransform(t->hForward, &LabOut1, Proof2, 1);
	cmsDoTransform(t->hReverse, Proof2, &LabOut2, 1);
	// Take difference of direct value
	dE1 = cmsDeltaE(&LabIn1, &LabOut1);
	// Take difference of converted value
	dE2 = cmsDeltaE(&LabIn2, &LabOut2);
	// if dE1 is small and dE2 is small, value is likely to be in gamut
	if(dE1 < t->Thereshold && dE2 < t->Thereshold)
		Out[0] = 0;
	else {
		// if dE1 is small and dE2 is big, undefined. Assume in gamut
		if(dE1 < t->Thereshold && dE2 > t->Thereshold)
			Out[0] = 0;
		else
		// dE1 is big and dE2 is small, clearly out of gamut
		if(dE1 > t->Thereshold && dE2 < t->Thereshold)
			Out[0] = (uint16)_cmsQuickFloor((dE1 - t->Thereshold) + .5);
		else {
			// dE1 is big and dE2 is also big, could be due to perceptual mapping
			// so take error ratio
			if(dE2 == 0.0)
				ErrorRatio = dE1;
			else
				ErrorRatio = dE1 / dE2;

			if(ErrorRatio > t->Thereshold)
				Out[0] = (uint16)_cmsQuickFloor((ErrorRatio - t->Thereshold) + .5);
			else
				Out[0] = 0;
		}
	}
	return TRUE;
}

// Does compute a gamut LUT going back and forth across pcs -> relativ. colorimetric intent -> pcs
// the dE obtained is then annotated on the LUT. Values truly out of gamut are clipped to dE = 0xFFFE
// and values changed are supposed to be handled by any gamut remapping, so, are out of gamut as well.
//
// **WARNING: This algorithm does assume that gamut remapping algorithms does NOT move in-gamut colors,
// of course, many perceptual and saturation intents does not work in such way, but relativ. ones should.
cmsPipeline * _cmsCreateGamutCheckPipeline(cmsContext ContextID, cmsHPROFILE hProfiles[], boolint BPC[], uint32 Intents[],
    double AdaptationStates[], uint32 nGamutPCSposition, cmsHPROFILE hGamut)
{
	cmsHPROFILE hLab;
	cmsPipeline * Gamut;
	cmsStage * CLUT;
	uint32 dwFormat;
	GAMUTCHAIN Chain;
	uint32 nChannels, nGridpoints;
	cmsColorSpaceSignature ColorSpace;
	uint32 i;
	cmsHPROFILE ProfileList[256];
	boolint BPCList[256];
	double AdaptationList[256];
	uint32 IntentList[256];
	memzero(&Chain, sizeof(GAMUTCHAIN));
	if(nGamutPCSposition <= 0 || nGamutPCSposition > 255) {
		cmsSignalError(ContextID, cmsERROR_RANGE, "Wrong position of PCS. 1..255 expected, %d found.", nGamutPCSposition);
		return NULL;
	}

	hLab = cmsCreateLab4ProfileTHR(ContextID, NULL);
	if(hLab == NULL) return NULL;

	// The figure of merit. On matrix-shaper profiles, should be almost zero as
	// the conversion is pretty exact. On LUT based profiles, different resolutions
	// of input and output CLUT may result in differences.

	if(cmsIsMatrixShaper(hGamut)) {
		Chain.Thereshold = 1.0;
	}
	else {
		Chain.Thereshold = ERR_THERESHOLD;
	}

	// Create a copy of parameters
	for(i = 0; i < nGamutPCSposition; i++) {
		ProfileList[i]    = hProfiles[i];
		BPCList[i]        = BPC[i];
		AdaptationList[i] = AdaptationStates[i];
		IntentList[i]     = Intents[i];
	}

	// Fill Lab identity
	ProfileList[nGamutPCSposition] = hLab;
	BPCList[nGamutPCSposition] = 0;
	AdaptationList[nGamutPCSposition] = 1.0;
	IntentList[nGamutPCSposition] = INTENT_RELATIVE_COLORIMETRIC;

	ColorSpace  = cmsGetColorSpace(hGamut);

	nChannels   = cmsChannelsOf(ColorSpace);
	nGridpoints = _cmsReasonableGridpointsByColorspace(ColorSpace, cmsFLAGS_HIGHRESPRECALC);
	dwFormat    = (CHANNELS_SH(nChannels)|BYTES_SH(2));

	// 16 bits to Lab double
	Chain.hInput = cmsCreateExtendedTransform(ContextID,
		nGamutPCSposition + 1,
		ProfileList,
		BPCList,
		IntentList,
		AdaptationList,
		NULL, 0,
		dwFormat, TYPE_Lab_DBL,
		cmsFLAGS_NOCACHE);

	// Does create the forward step. Lab double to device
	dwFormat    = (CHANNELS_SH(nChannels)|BYTES_SH(2));
	Chain.hForward = cmsCreateTransformTHR(ContextID,
		hLab, TYPE_Lab_DBL,
		hGamut, dwFormat,
		INTENT_RELATIVE_COLORIMETRIC,
		cmsFLAGS_NOCACHE);

	// Does create the backwards step
	Chain.hReverse = cmsCreateTransformTHR(ContextID, hGamut, dwFormat,
		hLab, TYPE_Lab_DBL,
		INTENT_RELATIVE_COLORIMETRIC,
		cmsFLAGS_NOCACHE);

	// All ok?
	if(Chain.hInput && Chain.hForward && Chain.hReverse) {
		// Go on, try to compute gamut LUT from PCS. This consist on a single channel containing
		// dE when doing a transform back and forth on the colorimetric intent.

		Gamut = cmsPipelineAlloc(ContextID, 3, 1);
		if(Gamut != NULL) {
			CLUT = cmsStageAllocCLut16bit(ContextID, nGridpoints, nChannels, 1, NULL);
			if(!cmsPipelineInsertStage(Gamut, cmsAT_BEGIN, CLUT)) {
				cmsPipelineFree(Gamut);
				Gamut = NULL;
			}
			else {
				cmsStageSampleCLut16bit(CLUT, GamutSampler, (void *)&Chain, 0);
			}
		}
	}
	else
		Gamut = NULL; // Didn't work...
	// Free all needed stuff.
	if(Chain.hInput) cmsDeleteTransform(Chain.hInput);
	if(Chain.hForward) cmsDeleteTransform(Chain.hForward);
	if(Chain.hReverse) cmsDeleteTransform(Chain.hReverse);
	cmsCloseProfile(hLab);
	// And return computed hull
	return Gamut;
}

// Total Area Coverage estimation ----------------------------------------------------------------

typedef struct {
	uint32 nOutputChans;
	cmsHTRANSFORM hRoundTrip;
	float MaxTAC;
	float MaxInput[cmsMAXCHANNELS];
} cmsTACestimator;

// This callback just accounts the maximum ink dropped in the given node. It does not populate any
// memory, as the destination table is NULL. Its only purpose it to know the global maximum.
static boolint EstimateTAC(const uint16 In[], uint16 Out[], void * Cargo)
{
	cmsTACestimator* bp = (cmsTACestimator*)Cargo;
	float RoundTrip[cmsMAXCHANNELS];
	uint32 i;
	float Sum;
	// Evaluate the xform
	cmsDoTransform(bp->hRoundTrip, In, RoundTrip, 1);
	// All all amounts of ink
	for(Sum = 0, i = 0; i < bp->nOutputChans; i++)
		Sum += RoundTrip[i];
	// If above maximum, keep track of input values
	if(Sum > bp->MaxTAC) {
		bp->MaxTAC = Sum;
		for(i = 0; i < bp->nOutputChans; i++) {
			bp->MaxInput[i] = In[i];
		}
	}
	return TRUE;
	CXX_UNUSED(Out);
}

// Detect Total area coverage of the profile
double CMSEXPORT cmsDetectTAC(cmsHPROFILE hProfile)
{
	cmsTACestimator bp;
	uint32 dwFormatter;
	uint32 GridPoints[MAX_INPUT_DIMENSIONS];
	cmsHPROFILE hLab;
	cmsContext ContextID = cmsGetProfileContextID(hProfile);

	// TAC only works on output profiles
	if(cmsGetDeviceClass(hProfile) != cmsSigOutputClass) {
		return 0;
	}

	// Create a fake formatter for result
	dwFormatter = cmsFormatterForColorspaceOfProfile(hProfile, 4, TRUE);

	bp.nOutputChans = T_CHANNELS(dwFormatter);
	bp.MaxTAC = 0; // Initial TAC is 0

	//  for safety
	if(bp.nOutputChans >= cmsMAXCHANNELS) return 0;

	hLab = cmsCreateLab4ProfileTHR(ContextID, NULL);
	if(hLab == NULL) return 0;
	// Setup a roundtrip on perceptual intent in output profile for TAC estimation
	bp.hRoundTrip = cmsCreateTransformTHR(ContextID, hLab, TYPE_Lab_16,
		hProfile, dwFormatter, INTENT_PERCEPTUAL, cmsFLAGS_NOOPTIMIZE|cmsFLAGS_NOCACHE);
	cmsCloseProfile(hLab);
	if(bp.hRoundTrip == NULL) 
		return 0;
	// For L* we only need black and white. For C* we need many points
	GridPoints[0] = 6;
	GridPoints[1] = 74;
	GridPoints[2] = 74;
	if(!cmsSliceSpace16(3, GridPoints, EstimateTAC, &bp)) {
		bp.MaxTAC = 0;
	}
	cmsDeleteTransform(bp.hRoundTrip);
	// Results in %
	return bp.MaxTAC;
}

// Carefully,  clamp on CIELab space.

boolint CMSEXPORT cmsDesaturateLab(cmsCIELab* Lab, double amax, double amin, double bmax, double bmin)
{
	// Whole Luma surface to zero
	if(Lab->L < 0) {
		Lab->L = Lab->a = Lab->b = 0.0;
		return FALSE;
	}
	// Clamp white, DISCARD HIGHLIGHTS. This is done
	// in such way because icc spec doesn't allow the
	// use of L>100 as a highlight means.
	if(Lab->L > 100)
		Lab->L = 100;
	// Check out gamut prism, on a, b faces

	if(Lab->a < amin || Lab->a > amax||
	    Lab->b < bmin || Lab->b > bmax) {
		cmsCIELCh LCh;
		double h, slope;

		// Falls outside a, b limits. Transports to LCh space,
		// and then do the clipping
		if(Lab->a == 0.0) { // Is hue exactly 90?
			// atan will not work, so clamp here
			Lab->b = Lab->b < 0 ? bmin : bmax;
			return TRUE;
		}
		cmsLab2LCh(&LCh, Lab);
		slope = Lab->b / Lab->a;
		h = LCh.h;
		// There are 4 zones
		if((h >= 0. && h < 45.) || (h >= 315 && h <= 360.)) {
			// clip by amax
			Lab->a = amax;
			Lab->b = amax * slope;
		}
		else if(h >= 45. && h < 135.) {
			// clip by bmax
			Lab->b = bmax;
			Lab->a = bmax / slope;
		}
		else if(h >= 135. && h < 225.) {
			// clip by amin
			Lab->a = amin;
			Lab->b = amin * slope;
		}
		else if(h >= 225. && h < 315.) {
			// clip by bmin
			Lab->b = bmin;
			Lab->a = bmin / slope;
		}
		else {
			cmsSignalError(0, cmsERROR_RANGE, "Invalid angle");
			return FALSE;
		}
	}
	return TRUE;
}
