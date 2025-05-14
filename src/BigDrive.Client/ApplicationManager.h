// <copyright file="ApplicationManager.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <wtypes.h>

// Shared
#include "..\Shared\EventLogger.h"

/// <summary>
/// Provides functionality to interact with the BigDrive client configuration,
/// including retrieving drive GUIDs.
/// </summary>
class ApplicationManager
{
private:

    /// <summary>
    /// Static instance of EventLogger for logging events.
    /// </summary>
    static EventLogger s_eventLogger;

public:

    static HRESULT StartApplication(CLSID clsidProvider);

    static HRESULT GetCOMAdminCatalog(IDispatch** ppIDispatch);

    /// <summary>
    /// Retrieves the Applications collection from the COM+ catalog.
    /// </summary>
    /// <param name="ppIDispatchApplicationsCollection">Pointer to an IDispatch pointer that will receive the Applications collection.</param>
    /// <returns>HRESULT indicating success or failure of the operation.</returns>
    static HRESULT GetApplicationsCollection(IDispatch** ppIDispatchApplicationCollection);
};