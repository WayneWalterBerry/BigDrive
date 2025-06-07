// <copyright file="BigDriveShellIcon.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>


#include "pch.h"

// Header
#include "BigDriveShellIcon.h"

#include <shlobj.h> 

BigDriveShellIcon::BigDriveShellIcon(BigDriveShellFolder* pFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl)
	: m_refCount(1), m_pFolder(pFolder), m_cidl(cidl), m_apidl(nullptr)
{
	if (pFolder)
	{
		pFolder->AddRef();
		m_traceLogger.Initialize(pFolder->GetDriveGuid());
	}

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

	if (m_pFolder)
	{
		m_pFolder->Release();
		m_pFolder = nullptr;
	}

	m_traceLogger.Uninitialize();
}

/// <summary>
/// Factory method to create an instance of BigDriveShellIcon.
/// </summary>
HRESULT BigDriveShellIcon::CreateInstance(
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

	BigDriveShellIcon* pShellIcon = new BigDriveShellIcon(pFolder, cidl, apidl);
	if (!pShellIcon)
	{
		return E_OUTOFMEMORY;
	}

	HRESULT hr = pShellIcon->QueryInterface(riid, ppv);
	if (FAILED(hr))
	{
		goto End;
	}

	End:

	if (pShellIcon)
	{
		pShellIcon->Release();
		pShellIcon = nullptr;
	}

	return hr;
}