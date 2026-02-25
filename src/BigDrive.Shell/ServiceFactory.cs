// <copyright file="ServiceFactory.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Runtime.InteropServices;

    using BigDrive.Service.Interfaces;

    /// <summary>
    /// Factory for activating BigDriveService via COM+ out-of-process.
    /// Uses CoCreateInstance with CLSCTX_LOCAL_SERVER to ensure the service
    /// always runs in dllhost.exe, never in the Shell process.
    /// </summary>
    /// <remarks>
    /// The Shell must never reference BigDrive.Service directly. This factory
    /// activates the service out-of-process via COM+ and casts to the
    /// <see cref="IBigDriveProvision"/> interface defined in BigDrive.Service.Interfaces.
    ///
    /// Why CLSCTX_LOCAL_SERVER:
    ///   The Shell must never load COM objects in-process. All COM+ activation
    ///   from the Shell uses CLSCTX_LOCAL_SERVER so the component runs in
    ///   dllhost.exe. This is the same approach used by ProviderFactory.
    ///
    /// Why not Type.GetTypeFromCLSID + Activator.CreateInstance:
    ///   The .NET CLR short-circuits COM interop for managed-to-managed calls,
    ///   loading the ServicedComponent in-process. This causes TypeInitializationException
    ///   because the Shell process doesn't have the Service's NuGet dependencies.
    /// </remarks>
    public static class ServiceFactory
    {
        /// <summary>
        /// CLSCTX_LOCAL_SERVER - Activates the COM object out-of-process in dllhost.exe.
        /// The Shell must never use CLSCTX_INPROC_SERVER; all COM activation is remote.
        /// </summary>
        private const uint CLSCTX_LOCAL_SERVER = 0x4;

        /// <summary>
        /// The CLSID of BigDriveService (from BigDrive.Service assembly).
        /// Must match the [Guid] attribute on the BigDriveService class.
        /// </summary>
        private static readonly Guid BigDriveServiceCLSID = new Guid("E6F5A1B2-4C6E-4F8A-9D3E-1A2B3C4D5E7F");

        /// <summary>
        /// P/Invoke declaration for CoCreateInstance.
        /// </summary>
        /// <param name="rclsid">The CLSID of the COM class.</param>
        /// <param name="pUnkOuter">Outer unknown for aggregation (null for no aggregation).</param>
        /// <param name="dwClsContext">The context in which the code runs.</param>
        /// <param name="riid">The IID of the interface to retrieve.</param>
        /// <param name="ppv">The interface pointer.</param>
        /// <returns>HRESULT indicating success or failure.</returns>
        [DllImport("ole32.dll")]
        private static extern int CoCreateInstance(
            [In] ref Guid rclsid,
            [MarshalAs(UnmanagedType.IUnknown)] object pUnkOuter,
            uint dwClsContext,
            [In] ref Guid riid,
            [MarshalAs(UnmanagedType.IUnknown)] out object ppv);

        /// <summary>
        /// Creates an IBigDriveProvision instance via COM+ out-of-process activation.
        /// </summary>
        /// <returns>The <see cref="IBigDriveProvision"/> interface for managing drive provisioning.</returns>
        /// <exception cref="InvalidOperationException">Thrown if the service cannot be activated.</exception>
        public static IBigDriveProvision GetProvisionService()
        {
            ShellTrace.Enter("ServiceFactory", "GetProvisionService");

            object service = CreateServiceInstance();

            ShellTrace.Verbose("Casting to IBigDriveProvision");
            IBigDriveProvision provision = service as IBigDriveProvision;
            if (provision == null)
            {
                ShellTrace.Error("BigDriveService does not implement IBigDriveProvision. Type: {0}",
                    service != null ? service.GetType().FullName : "null");
                throw new InvalidOperationException("BigDriveService does not implement IBigDriveProvision.");
            }

            ShellTrace.Exit("ServiceFactory", "GetProvisionService", "success");
            return provision;
        }

        /// <summary>
        /// Creates a BigDriveService instance via COM+ CoCreateInstance (CLSCTX_LOCAL_SERVER).
        /// The service runs out-of-process in dllhost.exe under the BigDriveInstaller identity.
        /// </summary>
        /// <returns>The COM object instance.</returns>
        /// <exception cref="InvalidOperationException">Thrown if CoCreateInstance fails.</exception>
        private static object CreateServiceInstance()
        {
            Guid clsid = BigDriveServiceCLSID;
            Guid iidUnknown = new Guid("00000000-0000-0000-C000-000000000046"); // IUnknown

            ShellTrace.Info("CoCreateInstance: BigDriveService CLSID={0}, Context=CLSCTX_LOCAL_SERVER (out-of-process in dllhost.exe)", clsid);

            object service;
            int hr = CoCreateInstance(ref clsid, null, CLSCTX_LOCAL_SERVER, ref iidUnknown, out service);

            ShellTrace.ComResult("CoCreateInstance", "BigDriveService", hr,
                service != null ? "service created" : "service is null");

            if (hr < 0)
            {
                string errorMessage = string.Format(
                    "Failed to create BigDriveService via COM+. CLSID: {0}, HRESULT: 0x{1:X8}. " +
                    "Ensure BigDrive.Service is registered with regsvcs.exe.",
                    clsid, hr);
                ShellTrace.Error(errorMessage);
                throw new InvalidOperationException(errorMessage, Marshal.GetExceptionForHR(hr));
            }

            if (service == null)
            {
                ShellTrace.Error("BigDriveService is null after successful CoCreateInstance");
                throw new InvalidOperationException("Failed to create BigDriveService instance.");
            }

            ShellTrace.Info("BigDriveService activated successfully via COM+");
            return service;
        }
    }
}
