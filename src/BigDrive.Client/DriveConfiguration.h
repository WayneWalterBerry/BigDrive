// <copyright file="DriveConfiguration.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <comutil.h> 

#include "GuidUtil.h"

using namespace BigDriveClient;

/// <summary>
/// Represents the configuration for a drive, including its unique identifier, name, and CLSID.
/// Provides functionality to parse configuration data from a JSON string.
/// </summary>
class DriveConfiguration
{
public:

    /// <summary>
    /// The unique identifier (GUID) of the drive.
    /// </summary>
    GUID id;

    /// <summary>
    /// The name of the drive.
    /// </summary>
    BSTR name;

    /// <summary>
    /// The CLSID (Class Identifier) associated with the drive.
    /// </summary>
    GUID clsid;

    /// <summary>
    /// Initializes a new instance of the <see cref="DriveConfiguration"/> class with default values.
    /// </summary>
    DriveConfiguration()
        : id(GUID_NULL), name(nullptr), clsid(GUID_NULL)
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="DriveConfiguration"/> class by parsing a JSON string.
    /// </summary>
    /// <param name="jsonString">The JSON string containing the drive configuration.</param>
    DriveConfiguration(LPCWSTR jsonString)
        : id(GUID_NULL), name(nullptr), clsid(GUID_NULL)
    {
        ParseJson(jsonString);
    }

    /// <summary>
    /// Destructor to clean up allocated memory for the drive name.
    /// </summary>
    ~DriveConfiguration()
    {
        if (name)
        {
            ::SysFreeString(name);
        }
    }

    /// <summary>
    /// Parses a JSON string to populate the drive configuration properties.
    /// </summary>
    /// <param name="jsonString">The JSON string containing the drive configuration.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT ParseJson(LPCWSTR jsonString);
};
