// <copyright file="ProviderFactory.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Runtime.InteropServices;

    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;
    using BigDrive.Interfaces;

    /// <summary>
    /// Factory for creating provider instances from drive configurations.
    /// Uses COM+ out-of-process activation (CLSCTX_LOCAL_SERVER) to ensure
    /// providers run in their own dllhost.exe process.
    /// </summary>
    public static class ProviderFactory
    {
        /// <summary>
        /// CLSCTX_LOCAL_SERVER - Activates the COM object out-of-process.
        /// </summary>
        private const uint CLSCTX_LOCAL_SERVER = 0x4;

        /// <summary>
        /// P/Invoke declaration for CoCreateInstance.
        /// </summary>
        /// <param name="rclsid">The CLSID of the COM class.</param>
        /// <param name="pUnkOuter">Outer unknown for aggregation (null for no aggregation).</param>
        /// <param name="dwClsContext">The context in which the code runs (CLSCTX_LOCAL_SERVER for out-of-process).</param>
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
        /// Creates an IBigDriveEnumerate instance for the specified drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The IBigDriveEnumerate interface, or null if not available.</returns>
        public static IBigDriveEnumerate GetEnumerateProvider(Guid driveGuid)
        {
            ShellTrace.Verbose("GetEnumerateProvider(driveGuid={0})", driveGuid);
            object provider = GetProviderInstance(driveGuid);
            IBigDriveEnumerate enumerate = provider as IBigDriveEnumerate;
            ShellTrace.Verbose("IBigDriveEnumerate: {0}", enumerate != null ? "available" : "not supported");
            return enumerate;
        }

        /// <summary>
        /// Creates an IBigDriveFileOperations instance for the specified drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The IBigDriveFileOperations interface, or null if not available.</returns>
        public static IBigDriveFileOperations GetFileOperationsProvider(Guid driveGuid)
        {
            ShellTrace.Verbose("GetFileOperationsProvider(driveGuid={0})", driveGuid);
            object provider = GetProviderInstance(driveGuid);
            IBigDriveFileOperations fileOps = provider as IBigDriveFileOperations;
            ShellTrace.Verbose("IBigDriveFileOperations: {0}", fileOps != null ? "available" : "not supported");
            return fileOps;
        }

        /// <summary>
        /// Creates an IBigDriveFileData instance for the specified drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The IBigDriveFileData interface, or null if not available.</returns>
        public static IBigDriveFileData GetFileDataProvider(Guid driveGuid)
        {
            ShellTrace.Verbose("GetFileDataProvider(driveGuid={0})", driveGuid);
            object provider = GetProviderInstance(driveGuid);
            IBigDriveFileData fileData = provider as IBigDriveFileData;
            ShellTrace.Verbose("IBigDriveFileData: {0}", fileData != null ? "available" : "not supported");
            return fileData;
        }

        /// <summary>
        /// Creates an IBigDriveAuthentication instance for the specified drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The IBigDriveAuthentication interface, or null if not supported by the provider.</returns>
        /// <remarks>
        /// Returns null if the provider does not implement IBigDriveAuthentication.
        /// This is expected - providers that don't require authentication (like Sample provider)
        /// will not implement this interface. The shell should assume such providers don't need auth.
        /// </remarks>
        public static IBigDriveAuthentication GetAuthenticationProvider(Guid driveGuid)
        {
            try
            {
                object provider = GetProviderInstance(driveGuid);
                return provider as IBigDriveAuthentication;
            }
            catch
            {
                // Provider creation failed - return null to indicate no auth support
                return null;
            }
        }

        /// <summary>
        /// Gets the raw provider instance for a drive using out-of-process COM+ activation.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The provider COM object running in dllhost.exe.</returns>
        /// <remarks>
        /// Uses CoCreateInstance with CLSCTX_LOCAL_SERVER to ensure the provider
        /// runs out-of-process in dllhost.exe. This prevents the Shell from loading
        /// the provider DLL directly and allows providers to access user credentials
        /// from Windows Credential Manager.
        /// </remarks>
        public static object GetProviderInstance(Guid driveGuid)
        {
            ShellTrace.Enter("ProviderFactory", "GetProviderInstance", string.Format("driveGuid={0}", driveGuid));

            DriveConfiguration config = DriveManager.ReadConfiguration(driveGuid, System.Threading.CancellationToken.None);
            if (config == null)
            {
                ShellTrace.Error("Drive configuration not found for GUID: {0}", driveGuid);
                throw new InvalidOperationException("Drive configuration not found for GUID: " + driveGuid);
            }

            ShellTrace.Verbose("Drive config: Name=\"{0}\", CLSID={1}", config.Name, config.CLSID);

            // IUnknown IID
            Guid iidUnknown = new Guid("00000000-0000-0000-C000-000000000046");
            Guid clsid = config.CLSID;

            ShellTrace.Info("CoCreateInstance: CLSID={0}, Context=CLSCTX_LOCAL_SERVER", clsid);

            object provider;
            int hr = CoCreateInstance(ref clsid, null, CLSCTX_LOCAL_SERVER, ref iidUnknown, out provider);

            ShellTrace.ComResult("CoCreateInstance", "CreateProvider", hr, 
                provider != null ? "provider created" : "provider is null");

            if (hr < 0)
            {
                // Provide a more helpful error message
                string errorMessage = string.Format(
                    "Failed to create provider instance via COM+. CLSID: {0}, HRESULT: 0x{1:X8}. " +
                    "Ensure the provider is registered with regsvcs.exe and COM+ is configured for activation.",
                    clsid, hr);
                ShellTrace.Error(errorMessage);
                throw new InvalidOperationException(errorMessage, Marshal.GetExceptionForHR(hr));
            }

            if (provider == null)
            {
                ShellTrace.Error("Provider is null after successful CoCreateInstance");
                throw new InvalidOperationException("Failed to create provider instance for CLSID: " + config.CLSID);
            }

            ShellTrace.Exit("ProviderFactory", "GetProviderInstance", "success");
            return provider;
        }
    }
}
