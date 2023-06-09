// Scintilla source code edit control
/** @file ContractionState.cxx
** Manages visibility of lines for folding and wrapping.
**/
// Copyright 1998-2007 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <Platform.h>
#include <Scintilla.h>
#include <scintilla-internal.h>
#pragma hdrstop

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

ContractionState::ContractionState() : visible(0), expanded(0), heights(0), foldDisplayTexts(0), displayLines(0), linesInDocument(1)
{
	//InsertLine(0);
}

ContractionState::~ContractionState()
{
	Clear();
}

void ContractionState::EnsureData()
{
	if(OneToOne()) {
		visible = new RunStyles();
		expanded = new RunStyles();
		heights = new RunStyles();
		foldDisplayTexts = new SparseVector<const char *>();
		displayLines = new Partitioning(4);
		InsertLines(0, linesInDocument);
	}
}

void ContractionState::Clear()
{
	ZDELETE(visible);
	ZDELETE(expanded);
	ZDELETE(heights);
	ZDELETE(foldDisplayTexts);
	ZDELETE(displayLines);
	linesInDocument = 1;
}

int ContractionState::LinesInDoc() const
{
	return OneToOne() ? linesInDocument : (displayLines->Partitions() - 1);
}

int ContractionState::LinesDisplayed() const
{
	return OneToOne() ? linesInDocument : displayLines->PositionFromPartition(LinesInDoc());
}

int ContractionState::DisplayFromDoc(int lineDoc) const
{
	if(OneToOne()) {
		return (lineDoc <= linesInDocument) ? lineDoc : linesInDocument;
	}
	else {
		SETMIN(lineDoc, displayLines->Partitions());
		return displayLines->PositionFromPartition(lineDoc);
	}
}

int ContractionState::DisplayLastFromDoc(int lineDoc) const
{
	return DisplayFromDoc(lineDoc) + GetHeight(lineDoc) - 1;
}

int ContractionState::DocFromDisplay(int lineDisplay) const
{
	if(OneToOne()) {
		return lineDisplay;
	}
	else if(lineDisplay <= 0) {
		return 0;
	}
	else if(lineDisplay > LinesDisplayed()) {
		return displayLines->PartitionFromPosition(LinesDisplayed());
	}
	else {
		int lineDoc = displayLines->PartitionFromPosition(lineDisplay);
		PLATFORM_ASSERT(GetVisible(lineDoc));
		return lineDoc;
	}
}

void ContractionState::InsertLine(int lineDoc)
{
	if(OneToOne()) {
		linesInDocument++;
	}
	else {
		visible->InsertSpace(lineDoc, 1);
		visible->SetValueAt(lineDoc, 1);
		expanded->InsertSpace(lineDoc, 1);
		expanded->SetValueAt(lineDoc, 1);
		heights->InsertSpace(lineDoc, 1);
		heights->SetValueAt(lineDoc, 1);
		foldDisplayTexts->InsertSpace(lineDoc, 1);
		foldDisplayTexts->SetValueAt(lineDoc, 0);
		int lineDisplay = DisplayFromDoc(lineDoc);
		displayLines->InsertPartition(lineDoc, lineDisplay);
		displayLines->InsertText(lineDoc, 1);
	}
}

void ContractionState::InsertLines(int lineDoc, int lineCount)
{
	for(int l = 0; l < lineCount; l++) {
		InsertLine(lineDoc + l);
	}
	Check();
}

void ContractionState::DeleteLine(int lineDoc)
{
	if(OneToOne()) {
		linesInDocument--;
	}
	else {
		if(GetVisible(lineDoc)) {
			displayLines->InsertText(lineDoc, -heights->ValueAt(lineDoc));
		}
		displayLines->RemovePartition(lineDoc);
		visible->DeleteRange(lineDoc, 1);
		expanded->DeleteRange(lineDoc, 1);
		heights->DeleteRange(lineDoc, 1);
		foldDisplayTexts->DeletePosition(lineDoc);
	}
}

void ContractionState::DeleteLines(int lineDoc, int lineCount)
{
	for(int l = 0; l < lineCount; l++) {
		DeleteLine(lineDoc);
	}
	Check();
}

bool FASTCALL ContractionState::GetVisible(int lineDoc) const
{
	return OneToOne() ? true : ((lineDoc >= visible->Length()) ? true : (visible->ValueAt(lineDoc) == 1));
}

bool ContractionState::SetVisible(int lineDocStart, int lineDocEnd, bool isVisible)
{
	if(OneToOne() && isVisible) {
		return false;
	}
	else {
		EnsureData();
		int delta = 0;
		Check();
		if((lineDocStart <= lineDocEnd) && (lineDocStart >= 0) && (lineDocEnd < LinesInDoc())) {
			for(int line = lineDocStart; line <= lineDocEnd; line++) {
				if(GetVisible(line) != isVisible) {
					int difference = isVisible ? heights->ValueAt(line) : -heights->ValueAt(line);
					visible->SetValueAt(line, isVisible ? 1 : 0);
					displayLines->InsertText(line, difference);
					delta += difference;
				}
			}
		}
		else {
			return false;
		}
		Check();
		return delta != 0;
	}
}

bool ContractionState::HiddenLines() const
{
	return OneToOne() ? false : !visible->AllSameAs(1);
}

const char * ContractionState::GetFoldDisplayText(int lineDoc) const
{
	Check();
	return foldDisplayTexts->ValueAt(lineDoc);
}

bool ContractionState::SetFoldDisplayText(int lineDoc, const char * text)
{
	EnsureData();
	const char * foldText = foldDisplayTexts->ValueAt(lineDoc);
	if(!foldText || strcmp(text, foldText) != 0) {
		foldDisplayTexts->SetValueAt(lineDoc, text);
		Check();
		return true;
	}
	else {
		Check();
		return false;
	}
}

bool FASTCALL ContractionState::GetExpanded(int lineDoc) const
{
	if(OneToOne())
		return true;
	else {
		Check();
		return expanded->ValueAt(lineDoc) == 1;
	}
}

bool ContractionState::SetExpanded(int lineDoc, bool isExpanded)
{
	if(OneToOne() && isExpanded)
		return false;
	else {
		EnsureData();
		if(isExpanded != (expanded->ValueAt(lineDoc) == 1)) {
			expanded->SetValueAt(lineDoc, isExpanded ? 1 : 0);
			Check();
			return true;
		}
		else {
			Check();
			return false;
		}
	}
}

bool ContractionState::GetFoldDisplayTextShown(int lineDoc) const
{
	return !GetExpanded(lineDoc) && GetFoldDisplayText(lineDoc);
}

int ContractionState::ContractedNext(int lineDocStart) const
{
	if(OneToOne())
		return -1;
	else {
		Check();
		if(!expanded->ValueAt(lineDocStart))
			return lineDocStart;
		else {
			int lineDocNextChange = expanded->EndRun(lineDocStart);
			return (lineDocNextChange < LinesInDoc()) ? lineDocNextChange : -1;
		}
	}
}

int ContractionState::GetHeight(int lineDoc) const
{
	return OneToOne() ? 1 : heights->ValueAt(lineDoc);
}

// Set the number of display lines needed for this line.
// Return true if this is a change.
bool ContractionState::SetHeight(int lineDoc, int height)
{
	if(OneToOne() && (height == 1)) {
		return false;
	}
	else if(lineDoc < LinesInDoc()) {
		EnsureData();
		if(GetHeight(lineDoc) != height) {
			if(GetVisible(lineDoc)) {
				displayLines->InsertText(lineDoc, height - GetHeight(lineDoc));
			}
			heights->SetValueAt(lineDoc, height);
			Check();
			return true;
		}
		else {
			Check();
			return false;
		}
	}
	else {
		return false;
	}
}

void ContractionState::ShowAll()
{
	int lines = LinesInDoc();
	Clear();
	linesInDocument = lines;
}

// Debugging checks

void ContractionState::Check() const
{
#ifdef CHECK_CORRECTNESS
	for(int vline = 0; vline < LinesDisplayed(); vline++) {
		const int lineDoc = DocFromDisplay(vline);
		PLATFORM_ASSERT(GetVisible(lineDoc));
	}
	for(int lineDoc = 0; lineDoc < LinesInDoc(); lineDoc++) {
		const int displayThis = DisplayFromDoc(lineDoc);
		const int displayNext = DisplayFromDoc(lineDoc + 1);
		const int height = displayNext - displayThis;
		PLATFORM_ASSERT(height >= 0);
		if(GetVisible(lineDoc)) {
			PLATFORM_ASSERT(GetHeight(lineDoc) == height);
		}
		else {
			PLATFORM_ASSERT(0 == height);
		}
	}
#endif
}

