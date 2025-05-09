// <copyright file="BigDriveInterfaceProviderFactory.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "BigDriveInterfaceProvider.h"

#include "..\Shared\EventLogger.h"

#include "DriveConfiguration.h"

/// <summary>
/// A factory class for creating instances of <see cref="BigDriveInterfaceProvider"/>.
/// </summary>
class BigDriveInterfaceProviderFactory
{
public:

    /// <summary>
    /// Retrieves the singleton instance of the factory.
    /// </summary>
    /// <returns>The singleton instance of <see cref="BigDriveInterfaceProviderFactory"/>.</returns>
    static BigDriveInterfaceProviderFactory& GetInstance();

    /// <summary>
    /// Creates a new instance of <see cref="BigDriveInterfaceProvider"/> using the provided <see cref="DriveConfiguration"/>.
    /// </summary>
    /// <param name="config">The <see cref="DriveConfiguration"/> containing the CLSID.</param>
    /// <returns>A new instance of <see cref="BigDriveInterfaceProvider"/>.</returns>
    BigDriveInterfaceProvider Create(const DriveConfiguration& config);

private:
    /// <summary>
    /// Private constructor to enforce the singleton pattern.
    /// </summary>
    BigDriveInterfaceProviderFactory() = default;

    /// <summary>
    /// Deleted copy constructor to prevent copying.
    /// </summary>
    BigDriveInterfaceProviderFactory(const BigDriveInterfaceProviderFactory&) = delete;

    /// <summary>
    /// Deleted assignment operator to prevent assignment.
    /// </summary>
    BigDriveInterfaceProviderFactory& operator=(const BigDriveInterfaceProviderFactory&) = delete;
};
