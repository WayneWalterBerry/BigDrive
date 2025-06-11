// <copyright file="BigDriveDataObject.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveTransferSource.h"

#include "Logging\BigDriveShellFolderTraceLogger.h"

#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

/// <summary>
/// Constructor for BigDriveDataObject.
/// </summary>
/// <param name="pFolder">Pointer to the parent shell folder.</param>
/// <param name="cidl">Count of items.</param>
/// <param name="apidl">Array of item IDs.</param>
BigDriveTransferSource::BigDriveTransferSource(BigDriveShellFolder* pFolder)
	: m_cRef(1), m_pFolder(pFolder)
{
	m_traceLogger.Initialize(pFolder->GetDriveGuid());

	// AddRef the folder object
	if (m_pFolder)
	{
		m_pFolder->AddRef();
	}

	m_driveGuid = pFolder->GetDriveGuid();
}

/// <summary>
/// Destructor for BigDriveTransferSource.
/// </summary>
BigDriveTransferSource::~BigDriveTransferSource()
{
	// Release the folder object
	if (m_pFolder)
	{
		m_pFolder->Release();
		m_pFolder = nullptr;
	}

	m_traceLogger.Uninitialize();
}

HRESULT BigDriveTransferSource::PropertyFailure(IShellItem* psi, REFPROPERTYKEY key, HRESULT hr)
{
	// Implementation for handling property failures
	return hr;
}