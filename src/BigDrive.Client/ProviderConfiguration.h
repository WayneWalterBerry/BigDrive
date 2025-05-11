// <copyright file="ProviderConfiguration.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <comutil.h> 

#include "GuidUtil.h"

using namespace BigDriveClient;

/// <summary>
/// Represents the configuration for a provider, including its unique identifier, name
/// Provides functionality to parse configuration data from a JSON string.
/// </summary>
class ProviderConfiguration
{
public:

    /// <summary>
    /// The unique identifier (CLSID) of the provider.
    /// </summary>
    CLSID clsid;

    /// <summary>
    /// The name of the drive.
    /// </summary>
    BSTR name;

    /// <summary>
    /// Initializes a new instance of the <see cref="ProviderConfiguration"/> class with default values.
    /// </summary>
    ProviderConfiguration()
        : clsid(GUID_NULL), name(nullptr)
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="ProviderConfiguration"/> class by parsing a JSON string.
    /// </summary>
    /// <param name="jsonString">The JSON string containing the drive configuration.</param>
    ProviderConfiguration(LPCWSTR jsonString)
        : clsid(GUID_NULL), name(nullptr)
    {
        ParseJson(jsonString);
    }

    /// <summary>
    /// Destructor to clean up allocated memory for the drive name.
    /// </summary>
    ~ProviderConfiguration()
    {
        if (name)
        {
            ::SysFreeString(name);
        }
    }

    /// <summary>
    /// Parses a JSON string to populate the provider configuration properties.
    /// </summary>
    /// <param name="jsonString">The JSON string containing the provider configuration.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT ParseJson(LPCWSTR jsonString);
};
