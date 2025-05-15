// <copyright file="ComponentCollection.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comdef.h>

// Local
#include "CatalogCollection.h"
#include "Component.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\ApplicationTests.h"

class ComponentCollection : public CatalogCollection
{

    /// <summary>
    /// Array of Components
    /// </summary>
    Component** m_ppComponents = nullptr;
    LONG       m_lSize = 0;

public:

    ComponentCollection(LPDISPATCH pDispatch)
        : CatalogCollection(pDispatch), m_ppComponents(nullptr)
    {
    }

    ~ComponentCollection()
    {
    }

    HRESULT GetName(BSTR& bstrString);

    /// <summary>
    /// Thread-safe initialization of the component collection.
    /// If the internal component array has not been initialized, this method retrieves
    /// all COM+ components and populates the array. Uses InterlockedCompareExchangePointer
    /// to ensure only one thread performs the initialization, and cleans up any redundant
    /// allocations if another thread wins the race. Returns S_OK if already initialized
    /// or if initialization succeeds; otherwise, returns an error HRESULT.
    /// </summary>
    HRESULT Initialize();

    /// <summary>
    /// Provides access to an Component object at the specified index.
    /// </summary>
    /// <param name="index">The zero-based index of the Component to retrieve.</param>
    /// <param name="ppComponent">Pointer to receive the Component object at the specified index.</param>
    /// <returns>HRESULT indicating success or failure of the operation.</returns>
    HRESULT GetItem(size_t index, Component** ppComponent);

    friend class BigDriveClientTest::ApplicationTests;

private:

    HRESULT GetComponents(Component*** pppComponents, LONG& lSize);
};
