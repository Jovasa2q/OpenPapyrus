// Scintilla source code edit control
/** @file Style.cxx
** Defines the font and colour style for a class of text.
**/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <Platform.h>
#include <Scintilla.h>
#include <scintilla-internal.h>
#pragma hdrstop

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

FontAlias::FontAlias()
{
}

FontAlias::FontAlias(const FontAlias &other) : SciFont()
{
	SetID(other.fid);
}

FontAlias::~FontAlias()
{
	SetID(0);
	// ~SciFont will not release the actual font resource since it is now 0
}

void FontAlias::MakeAlias(SciFont &fontOrigin)
{
	SetID(fontOrigin.GetID());
}

void FontAlias::ClearFont()
{
	SetID(0);
}

FontSpecification::FontSpecification() : fontName(0), weight(SC_WEIGHT_NORMAL), italic(false), size(10 * SC_FONT_SIZE_MULTIPLIER), characterSet(0), extraFontFlag(0) 
{
}

bool FASTCALL FontSpecification::operator == (const FontSpecification &other) const
{
	return fontName == other.fontName && weight == other.weight && italic == other.italic &&
	       size == other.size && characterSet == other.characterSet && extraFontFlag == other.extraFontFlag;
}

bool FASTCALL FontSpecification::operator < (const FontSpecification &other) const
{
	if(fontName != other.fontName)
		return fontName < other.fontName;
	else if(weight != other.weight)
		return weight < other.weight;
	else if(italic != other.italic)
		return italic == false;
	else if(size != other.size)
		return size < other.size;
	else if(characterSet != other.characterSet)
		return characterSet < other.characterSet;
	else if(extraFontFlag != other.extraFontFlag)
		return extraFontFlag < other.extraFontFlag;
	else
		return false;
}

FontMeasurements::FontMeasurements()
{
	Clear();
}

void FontMeasurements::Clear()
{
	ascent = 1;
	descent = 1;
	aveCharWidth = 1;
	spaceWidth = 1;
	sizeZoomed = 2;
}

SciStyle::SciStyle() : FontSpecification()
{
	//Clear(ColourDesired(0, 0, 0), ColourDesired(0xff, 0xff, 0xff), Platform::DefaultFontSize() * SC_FONT_SIZE_MULTIPLIER, 0, SC_CHARSET_DEFAULT,
	//    SC_WEIGHT_NORMAL, false, false, false, caseMixed, true, true, false);
		//void Clear(ColourDesired fore_, ColourDesired back_, int size_, const char *fontName_, int characterSet_,
		//	int weight_, bool italic_, bool eolFilled_, bool underline_, ecaseForced caseForce_, bool visible_, bool changeable_, bool hotspot_);
	Clear(ColourDesired(0, 0, 0), ColourDesired(0xff, 0xff, 0xff), Platform::DefaultFontSize() * SC_FONT_SIZE_MULTIPLIER, 0, SC_CHARSET_DEFAULT,
		SC_WEIGHT_NORMAL, false, caseMixed, fVisible|fChangeable);
}

SciStyle::SciStyle(const SciStyle & source) : FontSpecification(), FontMeasurements()
{
	//Clear(ColourDesired(0, 0, 0), ColourDesired(0xff, 0xff, 0xff), 0, 0, 0, SC_WEIGHT_NORMAL, false, false, false, caseMixed, true, true, false);
	Clear(ColourDesired(0, 0, 0), ColourDesired(0xff, 0xff, 0xff), 0, 0, 0, SC_WEIGHT_NORMAL, false, caseMixed, fVisible|fChangeable);
	fore = source.fore;
	back = source.back;
	characterSet = source.characterSet;
	weight = source.weight;
	italic = source.italic;
	size = source.size;
	fontName = source.fontName;
	Flags = source.Flags;
	//eolFilled = source.eolFilled;
	//underline = source.underline;
	caseForce = source.caseForce;
	//visible = source.visible;
	//changeable = source.changeable;
	//hotspot = source.hotspot;
}

SciStyle::~SciStyle()
{
}

SciStyle & SciStyle::operator = (const SciStyle & source)
{
	if(this == &source)
		return *this;
	//Clear(ColourDesired(0, 0, 0), ColourDesired(0xff, 0xff, 0xff), 0, 0, SC_CHARSET_DEFAULT, SC_WEIGHT_NORMAL, false, false, false, caseMixed, true, true, false);
	Clear(ColourDesired(0, 0, 0), ColourDesired(0xff, 0xff, 0xff), 0, 0, SC_CHARSET_DEFAULT, SC_WEIGHT_NORMAL, false, caseMixed, fVisible|fChangeable);
	fore = source.fore;
	back = source.back;
	characterSet = source.characterSet;
	weight = source.weight;
	italic = source.italic;
	size = source.size;
	fontName = source.fontName;
	//eolFilled = source.eolFilled;
	//underline = source.underline;
	caseForce = source.caseForce;
	//visible = source.visible;
	//changeable = source.changeable;
	// ? fHotspot
	Flags = source.Flags;
	return *this;
}

//void SciStyle::Clear(ColourDesired fore_, ColourDesired back_, int size_, const char * fontName_, int characterSet_,
//    int weight_, bool italic_, bool eolFilled_, bool underline_, ecaseForced caseForce_, bool visible_, bool changeable_, bool hotspot_)
void SciStyle::Clear(ColourDesired fore_, ColourDesired back_, int size_, const char * fontName_, int characterSet_,
    int weight_, bool italic_, ecaseForced caseForce_, long flags)
{
	fore = fore_;
	back = back_;
	characterSet = characterSet_;
	weight = weight_;
	italic = italic_;
	size = size_;
	fontName = fontName_;
	//eolFilled = eolFilled_;
	//underline = underline_;
	caseForce = caseForce_;
	//visible = visible_;
	//changeable = changeable_;
	//hotspot = hotspot_;
	Flags = flags;
	font.ClearFont();
	FontMeasurements::Clear();
}

void SciStyle::ClearTo(const SciStyle & source)
{
	//Clear(source.fore, source.back, source.size, source.fontName, source.characterSet, source.weight, 
	//	source.italic, source.eolFilled, source.underline, source.caseForce, source.visible, source.changeable, source.hotspot);
	Clear(source.fore, source.back, source.size, source.fontName, source.characterSet, source.weight, 
		source.italic, source.caseForce, source.Flags);
}

void SciStyle::Copy(SciFont &font_, const FontMeasurements &fm_)
{
	font.MakeAlias(font_);
	(FontMeasurements &)(*this) = fm_;
}
