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
    /// Initializes a new instance of the <see cref="BigDriveInterfaceProvider"/> class with the specified CLSID.
    /// </summary>
    /// <param name="clsid">The CLSID of the COM+ class.</param>
    BigDriveInterfaceProvider(const CLSID& clsid);

    /// <summary>
    /// Retrieves all interface IDs (IIDs) supported by the stored CLSID.
    /// </summary>
    /// <param name="interfaceIDs">A vector to store the supported interface IDs.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT GetSupportedInterfaceIDs(std::vector<IID>& interfaceIDs) const;

private:
    /// <summary>
    /// The CLSID of the COM+ class.
    /// </summary>
    CLSID m_clsid;
};
