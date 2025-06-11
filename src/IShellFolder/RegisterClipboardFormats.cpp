// <copyright file="RegisterClipboardFormats.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "RegisterClipboardFormats.h"

// Initialize the clipboard formats
CLIPFORMAT g_cfDropDescription = ::RegisterClipboardFormat(TEXT("DropDescription"));
CLIPFORMAT g_cfFileContents = ::RegisterClipboardFormat(CFSTR_FILECONTENTS);
CLIPFORMAT g_cfFileNameW = ::RegisterClipboardFormat(TEXT("FileName"));
CLIPFORMAT g_cfShellIdList = ::RegisterClipboardFormat(CFSTR_SHELLIDLIST);
CLIPFORMAT g_cfFileDescriptor = ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
CLIPFORMAT g_cfHDrop = CF_HDROP;  // Standard clipboard format