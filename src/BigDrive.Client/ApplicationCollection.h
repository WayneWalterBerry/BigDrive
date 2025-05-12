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

    LPDISPATCH  m_pDispatch
        ;
    Application** m_ppApplications;

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
    }

    /// <summary>
    /// Populates the specified COM+ collection by invoking the "Populate" method.
    /// </summary>
    /// <returns>HRESULT indicating success or failure of the operation.</returns>
    HRESULT Populate();

    /// <summary>
    /// Returns The Count Of Applications In The Collection 
    /// </summary>
    HRESULT GetCount(LONG& lCount)
    {
        HRESULT hrReturn = Populate();
        if (FAILED(hrReturn))
        {
            s_eventLogger.WriteErrorFormmated(L"GetCount: Failed to populate Applications collection. HRESULT: 0x%08X", hrReturn);
            return hrReturn;
        }

        return COM::GetLongProperty(m_pDispatch, L"Count", lCount);
    }

    HRESULT GetName(BSTR& bstrName)
    {
        HRESULT hrReturn = Populate();
        if (FAILED(hrReturn))
        {
            s_eventLogger.WriteErrorFormmated(L"GetCount: Failed to populate Applications collection. HRESULT: 0x%08X", hrReturn);
            return hrReturn;
        }

        return COM::GetStringProperty(m_pDispatch, L"Name", bstrName);
    }

    HRESULT GetApplications(Application*** pppApplications, DWORD& dwSize);
};
