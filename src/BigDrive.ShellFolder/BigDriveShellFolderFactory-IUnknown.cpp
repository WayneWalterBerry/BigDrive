// <copyright file="BigDriveShellFolderFactory-IClassFactory.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveShellFolderFactory.h"
#include "Logging\BigDriveTraceLogger.h"

HRESULT __stdcall BigDriveShellFolderFactory::QueryInterface(REFIID riid, void** ppvObject)
{
	HRESULT hr = E_NOINTERFACE;

	BigDriveTraceLogger::LogEnter(__FUNCTION__, riid);

	if (riid == IID_IUnknown || riid == IID_IClassFactory)
	{
		*ppvObject = static_cast<IClassFactory*>(this);
		AddRef();
		hr = S_OK;
		goto End;
	}

	*ppvObject = nullptr;

End:

	BigDriveTraceLogger::LogExit(__FUNCTION__, hr);

	return hr;
}

ULONG __stdcall BigDriveShellFolderFactory::AddRef()
{
	return InterlockedIncrement(&m_refCount);
}

ULONG __stdcall BigDriveShellFolderFactory::Release()
{
	LONG ref = InterlockedDecrement(&m_refCount);

	if (ref == 0)
	{
		delete this;
	}
	return ref;
}