// <copyright file="BigDriveInterfaceProvider.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <windows.h>
#include <comdef.h>

// Local
#include "BigDriveClientEventLogger.h"
#include "Interfaces/IBigDriveEnumerate.h"
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

    /// <summary>
    /// Retrieves the requested interface from the COM+ class associated with this provider.
    /// </summary>
    /// <param name="iid">The interface ID (IID) of the interface to retrieve.</param>
    /// <param name="ppv">Address of a pointer that receives the interface pointer on success. Set to nullptr on failure.</param>
    /// <returns>
    /// S_OK if the interface was successfully retrieved; otherwise, an HRESULT error code.
    /// </returns>
    HRESULT GetInterface(const IID& iid, IUnknown** ppv);

    /// <summary>
    /// Retrieves the IBigDriveConfiguration interface from the COM+ class associated with this provider.
    /// </summary>
    /// <param name="ppBigDriveConfiguration">Address of a pointer that receives the IBigDriveConfiguration interface pointer on success. Set to nullptr on failure.</param>
    /// <returns>
    /// S_OK if the interface was successfully retrieved; otherwise, an HRESULT error code.
    /// </returns>
    HRESULT GetIBigDriveConfiguration(IBigDriveConfiguration** ppBigDriveConfiguration);

    /// <summary>
    /// Retrieves the IBigDriveEnumerate interface from the COM+ class associated with this provider.
    /// </summary>
    /// <param name="ppBigDriveEnumerate">Address of a pointer that receives the IBigDriveEnumerate interface pointer on success. Set to nullptr on failure.</param>
    /// <returns>
    /// S_OK if the interface was successfully retrieved; otherwise, an HRESULT error code.
    /// </returns>
    HRESULT GetIBigDriveEnumerate(IBigDriveEnumerate** ppBigDriveEnumerate);

private:

    /// <summary>
    /// Writes an error message to the event log.
    /// </summary>
    /// <param name="message">The error message to log.</param>
    /// <returns>
    /// S_OK if the message was successfully logged; otherwise, an HRESULT error code.
    /// </returns>
    HRESULT WriteError(LPCWSTR message);

    /// <summary>
    /// Writes a formatted error message to the event log.
    /// </summary>
    /// <param name="formatter">The format string for the error message.</param>
    /// <param name="...">Additional arguments for formatting.</param>
    /// <returns>
    /// S_OK if the message was successfully logged; otherwise, an HRESULT error code.
    /// </returns>
    HRESULT WriteErrorFormmated(LPCWSTR formatter, ...);
};
