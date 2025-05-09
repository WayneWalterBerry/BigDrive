// <copyright file="Provider.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using BigDrive.Interfaces;
    using System;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;

    [Guid("F8FE2E5A-E8B8-4207-BC04-EA4BCD4C4361")] // Unique GUID for the COM class
    [ClassInterface(ClassInterfaceType.None)] // No automatic interface generation
    [ComVisible(true)] // Make the class visible to COM
    public partial class Provider : ServicedComponent,
        IBigDriveRoot
    {
    }
}
