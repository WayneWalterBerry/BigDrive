// <copyright file="ApplicationCollection.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comdef.h>

// Shared
#include "..\Shared\EventLogger.h"

// Local
#include "Application.h"
#include "COM.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\ApplicationCollectionTests.h"

/// <summary>
/// Collection of COM+ applications.
/// </summary>
class ApplicationCollection : COM
{
private:

    /// <summary>
    /// Static instance of EventLogger for logging events.
    /// </summary>
    static EventLogger s_eventLogger;

    LPDISPATCH  m_pDispatch;

    /// <summary>
    /// Array of Applications
    /// </summary>
    Application **m_ppApplications;
    DWORD       m_dwSize = 0;

public:

    ApplicationCollection(LPDISPATCH pDispatch)
        : m_pDispatch(pDispatch), m_ppApplications(nullptr)
    {
        if (m_pDispatch)
        {
            m_pDispatch->AddRef();
        }
    }

    ~ApplicationCollection()
    {
        if (m_pDispatch)
        {
            m_pDispatch->Release();
            m_pDispatch = nullptr;
        }

        if (*m_ppApplications != nullptr)
        {
            for (DWORD j = 0; j < m_dwSize; j++)
            {
                if (m_ppApplications[j] != nullptr)
                {
                    delete (m_ppApplications)[j];
                }
            }
            ::CoTaskMemFree(m_ppApplications);
            m_ppApplications = nullptr;
            m_dwSize = 0;
        }
    }

    HRESULT Initialize();

    friend class BigDriveClientTest::ApplicationCollectionTests;

private :

    /// <summary>
    /// Populates the specified COM+ collection by invoking the "Populate" method.
    /// </summary>
    /// <returns>HRESULT indicating success or failure of the operation.</returns>
    static HRESULT Populate(LPDISPATCH pDispatch);

    /// <summary>
    /// Returns The Count Of Applications In The Collection 
    /// </summary>
    HRESULT GetCount(LONG& lCount)
    {
        HRESULT hrReturn = Populate(m_pDispatch);
        if (FAILED(hrReturn))
        {
            s_eventLogger.WriteErrorFormmated(L"GetCount: Failed to populate Applications collection. HRESULT: 0x%08X", hrReturn);
            return hrReturn;
        }

        return COM::GetLongProperty(m_pDispatch, L"Count", lCount);
    }

    HRESULT GetName(BSTR& bstrName)
    {
        HRESULT hrReturn = Populate(m_pDispatch);
        if (FAILED(hrReturn))
        {
            s_eventLogger.WriteErrorFormmated(L"GetCount: Failed to populate Applications collection. HRESULT: 0x%08X", hrReturn);
            return hrReturn;
        }

        return COM::GetStringProperty(m_pDispatch, L"Name", bstrName);
    }

    HRESULT GetApplications(Application*** pppApplications, DWORD& dwSize);
};
