// <copyright file="RegisterClipboardFormats.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>

// Clipboard format names
#define CFSTR_SHELLIDLIST      TEXT("Shell IDList Array")
#define CFSTR_FILECONTENTS     TEXT("FileContents")
#define CFSTR_SHELLIDLIST	   TEXT("Shell IDList Array")

#ifndef CFSTR_FILEDESCRIPTOR
	#define CFSTR_FILEDESCRIPTOR TEXT("FileGroupDescriptor")
#endif

extern CLIPFORMAT g_cfDropDescription;
extern CLIPFORMAT g_cfFileContents;
extern CLIPFORMAT g_cfFileNameW;
extern CLIPFORMAT g_cfShellIdList;
extern CLIPFORMAT g_cfFileDescriptor;
extern CLIPFORMAT g_cfHDrop;
