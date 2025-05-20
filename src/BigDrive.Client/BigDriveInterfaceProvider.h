// <copyright file="BigDriveInterfaceProvider.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <windows.h>
#include <comdef.h>

// Local
#include "BigDriveClientEventLogger.h"
#include "Interfaces/IBigDriveRoot.h"
#include "Interfaces/IBigDriveConfiguration.h"
#include "DriveConfiguration.h"

/// <summary>
/// Provides functionality to retrieve all interface IDs (IIDs) supported by a given COM+ class ID (CLSID).
/// </summary>
class BigDriveInterfaceProvider
{
private:

    /// <summary>
    /// Static instance of EventLogger for logging events.
    /// </summary>
    static BigDriveClientEventLogger s_eventLogger;

    /// <summary>
    /// The CLSID of the COM+ class.
    /// </summary>
    CLSID m_clsid;

public:

    /// <summary>
    /// Initializes a new instance of the <see cref="BigDriveInterfaceProvider"/> class with the specified CLSID.
    /// </summary>
    /// <param name="clsid">The CLSID of the COM+ class.</param>
    BigDriveInterfaceProvider(const CLSID& clsid);

    /// <summary>
    /// Initializes a new instance of the <see cref="BigDriveInterfaceProvider"/> class with the specified DriveConfiguration.
    /// </summary>
    /// <param name="driveConfiguration">Drive Configuration</param>
    BigDriveInterfaceProvider(DriveConfiguration& driveConfiguration);

    HRESULT GetInterface(const IID& iid, IUnknown** ppv);

    HRESULT GetIBigDriveConfiguration(IBigDriveConfiguration** ppBigDriveConfiguration);
    HRESULT GetIBigDriveRoot(IBigDriveRoot** ppBigDriveRoot);

private: 

    HRESULT WriteError(LPCWSTR message);
    HRESULT WriteErrorFormmated(LPCWSTR formatter, ...);
};
