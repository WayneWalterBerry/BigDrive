// <copyright file="BigDriveInterfaceProviderFactory.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveInterfaceProviderFactory.h"

/// <summary>
/// Retrieves the singleton instance of the factory.
/// </summary>
/// <returns>The singleton instance of <see cref="BigDriveInterfaceProviderFactory"/>.</returns>
BigDriveInterfaceProviderFactory& BigDriveInterfaceProviderFactory::GetInstance()
{
    static BigDriveInterfaceProviderFactory instance;
    return instance;
}

/// <summary>
/// Creates a new instance of <see cref="BigDriveInterfaceProvider"/> using the provided <see cref="DriveConfiguration"/>.
/// </summary>
/// <param name="config">The <see cref="DriveConfiguration"/> containing the CLSID.</param>
/// <returns>A new instance of <see cref="BigDriveInterfaceProvider"/>.</returns>
BigDriveInterfaceProvider BigDriveInterfaceProviderFactory::Create(const DriveConfiguration& config)
{
    // Extract the CLSID from the DriveConfiguration
    CLSID clsid = config.clsid;

    // Create and return a BigDriveInterfaceProvider instance
    return BigDriveInterfaceProvider(clsid);
}
