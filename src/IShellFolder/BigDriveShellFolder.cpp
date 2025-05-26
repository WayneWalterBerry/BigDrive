// <copyright file="BigDriveShellFolder-IDispatch.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "BigDriveShellFolder.h"

// Local
#include "LaunchDebugger.h"

HRESULT BigDriveShellFolder::GetProviderCLSID(CLSID& clsidProvider) const
{
	return S_OK;
}

HRESULT BigDriveShellFolder::GetPath(BSTR& bstrPath)
{
	bstrPath = ::SysAllocString(L"\\");
	return S_OK;
}

