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

    /// <summary>
    /// Override the Release() method
    /// </summary>
    /// <returns></returns>
    ULONG Release() override;

    /// <summary>
    /// Thread-safe initialization of the application collection.
    /// If the internal application array has not been initialized, this method retrieves
    /// all COM+ applications and populates the array. Uses InterlockedCompareExchangePointer
    /// to ensure only one thread performs the initialization, and cleans up any redundant
    /// allocations if another thread wins the race. Returns S_OK if already initialized
    /// or if initialization succeeds; otherwise, returns an error HRESULT.
    /// </summary>
    HRESULT Initialize();

    /// <summary>
    /// Provides access to an Application object at the specified index.
    /// </summary>
    /// <param name="index">The zero-based index of the Application to retrieve.</param>
    /// <param name="ppApplication">Pointer to receive the Application object at the specified index.</param>
    /// <returns>HRESULT indicating success or failure of the operation.</returns>
    HRESULT GetItem(size_t index, Application** ppApplication);

    /// <summary>
    /// Searches for an application in the collection by its name.
    /// Iterates through all applications, calling GetName on each and comparing the result to the specified name.
    /// If a match is found, clones the application into ppApplication and returns S_OK.
    /// Returns HRESULT_FROM_WIN32(ERROR_NOT_FOUND) if no match is found, or an error HRESULT on failure.
    /// Logs errors using the event logger.
    /// </summary>
    HRESULT QueryApplicationByName(LPCWSTR bstrName, Application** ppApplication);

    friend class BigDriveClientTest::ApplicationCollectionTests;

private:

    /// <summary>
    /// Retrieves all Application objects in the collection and returns them as an array of Application pointers.
    /// </summary>
    /// <param name="pppApplications">Address of a pointer that receives the array of Application pointers.</param>
    /// <param name="lSize">Reference to a LONG that receives the number of applications in the array.</param>
    /// <returns>HRESULT indicating success or failure of the operation.</returns>

    HRESULT GetApplications(Application*** pppApplications, LONG& lSize);
};
