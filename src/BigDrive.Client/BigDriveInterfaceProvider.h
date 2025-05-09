// <copyright file="BigDriveInterfaceProvider.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <windows.h>
#include <comdef.h>

// Shared
#include "..\Shared\EventLogger.h"

// Local
#include "Interfaces/IBigDriveConfiguration.h"

/// <summary>
/// Provides functionality to retrieve all interface IDs (IIDs) supported by a given COM+ class ID (CLSID).
/// </summary>
class BigDriveInterfaceProvider
{
private:

    /// <summary>
    /// Static instance of EventLogger for logging events.
    /// </summary>
    static EventLogger s_eventLogger;

public:

    /// <summary>
    /// Initializes a new instance of the <see cref="BigDriveInterfaceProvider"/> class with the specified CLSID.
    /// </summary>
    /// <param name="clsid">The CLSID of the COM+ class.</param>
    BigDriveInterfaceProvider(const CLSID& clsid);

    HRESULT GetInterface(const IID& iid, IUnknown** ppv);

    HRESULT GetIBigDriveConfiguration(IBigDriveConfiguration** ppBigDriveConfiguration);

private:

    /// <summary>
    /// The CLSID of the COM+ class.
    /// </summary>
    CLSID m_clsid;
};
