// HelpUtils.cpp

//#include "StdAfx.h"
#include <7z-internal.h>
#include "HelpUtils.h"

#if defined(UNDER_CE) || !defined(_WIN32)

void ShowHelpWindow(LPCSTR)
{
}

#else

#include <HtmlHelp.h>

#define kHelpFileName "7-zip.chm::/"

void ShowHelpWindow(LPCSTR topicFile)
{
  FString path = NWindows::NDLL::GetModuleDirPrefix();
  path += kHelpFileName;
  path += topicFile;
  // HWND hwnd = NULL;
  HtmlHelp(NULL, GetSystemString(fs2us(path)), HH_DISPLAY_TOPIC, 0);
}

#endif
