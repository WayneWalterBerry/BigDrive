// <copyright file="BigDriveShellFolder-IDispatch.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "BigDriveShellFolder.h"

// Local
#include "LaunchDebugger.h"

// Shared
#include "..\Shared\EventLogger.h"

EventLogger BigDriveShellFolder::s_eventLogger(L"BigDrive.ShellFolder");

HRESULT BigDriveShellFolder::GetProviderCLSID(CLSID& clsidProvider) const
{
    return S_OK;
}