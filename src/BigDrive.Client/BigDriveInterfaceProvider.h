// <copyright file="BigDriveInterfaceProvider.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <vector>

/// <summary>
/// Provides functionality to retrieve all interface IDs (IIDs) supported by a given COM+ class ID (CLSID).
/// </summary>
class BigDriveInterfaceProvider
{
public:

    /// <summary>
    /// Retrieves all interface IDs (IIDs) supported by a given CLSID.
    /// </summary>
    /// <param name="clsid">The CLSID of the COM+ class.</param>
    /// <param name="interfaceIDs">A vector to store the supported interface IDs.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    static HRESULT GetSupportedInterfaceIDs(const CLSID& clsid, std::vector<IID>& interfaceIDs);
};
