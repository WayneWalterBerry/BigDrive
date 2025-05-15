// <copyright file="IBigDriveRegistration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Interface for COM+ Registration
    /// </summary>
    /// <remarks>
    /// Register() / UnRegister() are called by the BigDrive IShellFolder implementation when the shell folder dll 
    /// is registered as a COM object. This is not the registration of the Provider as a COM object, but the secondary
    /// registration that allows the provider to "setup" when the shell folder is registered.
    /// 
    /// Both Register() and UnRegister() are called with elevated permissions.
    /// 
    /// At the minimum the implementation of Register() should create a subkey in Software\BigDrive\Providers of the registry
    /// which contains the CLSID of the COM object.  This is used by the BigDrive shell folder implementation to determine which 
    /// providers are available.  This can be accomplished using the ProviderManager.RegisterProvider() method.
    /// 
    /// </remarks>
    [ComVisible(true)]
    [Guid("FF0FA03A-5DC1-464F-AFCE-5C60ECAA3912")] // Randomly generated GUID
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveRegistration
    {
        /// <summary>
        /// Called By Installation To Tell The COM+ Application (Provider) To Register
        /// </summary>
        void Register();

        /// <summary>
        ///  Called By Installation To Tell The COM+ Application (Provider) To UnRegister
        /// </summary>
        void Unregister();
    }
}
