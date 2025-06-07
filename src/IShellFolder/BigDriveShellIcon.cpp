// <copyright file="BigDriveShellIcon.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>


#include "pch.h"

// Header
#include "BigDriveShellIcon.h"

// For IShellFolder and related interfaces
#include <shlobj.h> 

BigDriveShellIcon::BigDriveShellIcon(const CLSID& driveGuid, BigDriveShellFolder* pParentFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl)
	: m_refCount(1), m_driveGuid(driveGuid), m_pParentFolder(pParentFolder), m_cidl(cidl), m_apidl(nullptr)
{
	m_traceLogger.Initialize(driveGuid);

	if (cidl > 0 && apidl != nullptr) 
	{
		m_apidl = new (std::nothrow) PCUITEMID_CHILD[cidl];

		if (m_apidl)
		{
			for (UINT i = 0; i < cidl; ++i)
			{
				m_apidl[i] = apidl[i];
			}
		}
		else
		{
			m_cidl = 0;
		}
	}
}

BigDriveShellIcon::~BigDriveShellIcon()
{
	if (m_apidl) 
	{
		delete[] m_apidl;
		m_apidl = nullptr;
	}

	m_traceLogger.Uninitialize();
}

/// <summary>
/// Factory method to create an instance of BigDriveShellIcon.
/// </summary>
HRESULT BigDriveShellIcon::CreateInstance(
	const CLSID& driveGuid,
	BigDriveShellFolder* pFolder,
	UINT cidl,
	PCUITEMID_CHILD_ARRAY apidl,
	REFIID riid,
	void** ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}

	*ppv = nullptr;

	BigDriveShellIcon* pShellIcon = new(std::nothrow) BigDriveShellIcon(driveGuid, pFolder, cidl, apidl);
	if (!pShellIcon)
		return E_OUTOFMEMORY;

	HRESULT hr = pShellIcon->QueryInterface(riid, ppv);
	pShellIcon->Release();

	return hr;
}