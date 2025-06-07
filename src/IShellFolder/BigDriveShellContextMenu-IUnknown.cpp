// <copyright file="BigDriveShellContextMenu-IUnknown.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveShellContextMenu.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"

#include <shellapi.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <shlobj_core.h>
#include <shobjidl.h>

/// <summary>
/// Retrieves a pointer to the requested interface.
/// </summary>
/// <summary>
/// Retrieves a pointer to the requested interface.
/// </summary>
HRESULT __stdcall BigDriveShellContextMenu::QueryInterface(REFIID riid, void** ppv)
{
	HRESULT hr = S_OK;

	m_traceLogger.LogEnter(__FUNCTION__, riid);

	// Validate output parameter
	if (!ppv)
	{
		hr = E_POINTER;
		goto End;
	}

	// Initialize output to null
	*ppv = nullptr;

	// Check for requested interfaces
	if (IsEqualIID(riid, IID_IUnknown))
	{
		// Return IUnknown
		*ppv = static_cast<IUnknown*>(static_cast<IContextMenu*>(this));
	}
	else if (IsEqualIID(riid, IID_IContextMenu))
	{
		// Return IContextMenu
		*ppv = static_cast<IContextMenu*>(this);
	}
	else if (IsEqualIID(riid, IID_IContextMenu2))
	{
		// Return IContextMenu2
		*ppv = static_cast<IContextMenu2*>(this);
	}
	else if (IsEqualIID(riid, IID_IContextMenu3))
	{
		// Return IContextMenu3
		*ppv = static_cast<IContextMenu3*>(this);
	}
	else
	{
		// Interface not supported
		hr = E_NOINTERFACE;
		goto End;
	}

	// AddRef the interface
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

End:

	m_traceLogger.LogEnter(__FUNCTION__, riid);

	return hr;
}

/// <summary>
/// Adds a reference to this object.
/// </summary>
ULONG __stdcall BigDriveShellContextMenu::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

/// <summary>
/// Releases a reference to this object.
/// </summary>
ULONG __stdcall BigDriveShellContextMenu::Release()
{
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (!cRef)
	{
		delete this;
	}
	return cRef;
}