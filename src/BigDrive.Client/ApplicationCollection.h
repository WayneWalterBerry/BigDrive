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
#include "Dispatch.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\ApplicationCollectionTests.h"

/// <summary>
/// Collection of COM+ applications.
/// </summary>
class ApplicationCollection : Dispatch
{
private:

    /// <summary>
    /// Static instance of EventLogger for logging events.
    /// </summary>
    static EventLogger s_eventLogger;

    /// <summary>
    /// Array of Applications
    /// </summary>
    Application **m_ppApplications = nullptr;
    DWORD       m_dwSize = 0;

public:

    ApplicationCollection(LPDISPATCH pDispatch)
        : Dispatch(pDispatch), m_ppApplications(nullptr)
    {
    }

    ~ApplicationCollection()
    {
        if (m_ppApplications != nullptr)
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

    /// <summary>
    /// Returns The Count Of Applications In The Collection 
    /// </summary>
    HRESULT GetCount(LONG& lCount)
    {
        HRESULT hrReturn = Populate(m_pIDispatch);
        if (FAILED(hrReturn))
        {
            s_eventLogger.WriteErrorFormmated(L"GetCount: Failed to populate Applications collection. HRESULT: 0x%08X", hrReturn);
            return hrReturn;
        }

        return GetLongProperty(L"Count", lCount);
    }

    HRESULT GetName(BSTR& bstrName)
    {
        HRESULT hrReturn = Populate(m_pIDispatch);
        if (FAILED(hrReturn))
        {
            s_eventLogger.WriteErrorFormmated(L"GetCount: Failed to populate Applications collection. HRESULT: 0x%08X", hrReturn);
            return hrReturn;
        }

        return GetStringProperty(L"Name", bstrName);
    }

    friend class BigDriveClientTest::ApplicationCollectionTests;

private :

    /// <summary>
    /// Populates the specified COM+ collection by invoking the "Populate" method.
    /// </summary>
    /// <returns>HRESULT indicating success or failure of the operation.</returns>
    static HRESULT Populate(LPDISPATCH pDispatch);

    HRESULT GetApplications(Application*** pppApplications, DWORD& dwSize);
};
