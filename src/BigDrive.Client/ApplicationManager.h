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

    /// <summary>
    /// Registers all COM+ applications and their components that support the IBigDriveRegistration interface.
    /// This method enumerates applications and their components using the COMAdminCatalog, queries for the
    /// IBigDriveRegistration interface, and invokes the Register method on each supported component.
    /// Returns S_OK if registration succeeds for all applicable components, or an error HRESULT if any step fails.
    /// </summary>
    static HRESULT RegisterApplications();

};