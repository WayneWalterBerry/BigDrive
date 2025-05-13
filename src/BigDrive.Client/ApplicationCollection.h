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
#include "CatalogCollection.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\ApplicationCollectionTests.h"

/// <summary>
/// Collection of COM+ applications.
/// </summary>
class ApplicationCollection : public CatalogCollection
{
private:

    /// <summary>
    /// Array of Applications
    /// </summary>
    Application** m_ppApplications = nullptr;
    LONG       m_lSize = 0;

public:

    ApplicationCollection(LPDISPATCH pDispatch)
        : CatalogCollection(pDispatch), m_ppApplications(nullptr)
    {
    }

    ~ApplicationCollection()
    {
        if (m_ppApplications != nullptr)
        {
            for (LONG j = 0; j < m_lSize; j++)
            {
                if (m_ppApplications[j] != nullptr)
                {
                    delete (m_ppApplications)[j];
                }
            }
            ::CoTaskMemFree(m_ppApplications);
            m_ppApplications = nullptr;
            m_lSize = 0;
        }
    }

    HRESULT Initialize();

    /// <summary>
    /// Provides access to an Application object at the specified index.
    /// </summary>
    /// <param name="index">The zero-based index of the Application to retrieve.</param>
    /// <param name="ppApplication">Pointer to receive the Application object at the specified index.</param>
    /// <returns>HRESULT indicating success or failure of the operation.</returns>
    HRESULT GetItem(size_t index, Application** ppApplication) const;

    friend class BigDriveClientTest::ApplicationCollectionTests;

private:

    HRESULT GetApplications(Application*** pppApplications, LONG& lSize);
};
