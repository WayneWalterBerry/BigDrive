// <copyright file="Provider.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Archive
{
    using System;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;

    using BigDrive.Interfaces;

    /// <summary>
    /// BigDrive provider for browsing archive file contents as a virtual file system.
    /// Supports multiple archive formats: ZIP, TAR, TAR.GZ, TAR.BZ2, 7z, RAR (read-only).
    /// Uses SharpCompress library for multi-format archive support.
    /// </summary>
    /// <remarks>
    /// This provider supports drive-specific configuration. Each drive points to a
    /// different archive file on the local file system via the ArchiveFilePath drive property.
    ///
    /// Drive-specific properties (stored in registry under each drive):
    /// - ArchiveFilePath: The full path to the local archive file to browse.
    ///
    /// Supported archive formats:
    /// - ZIP (.zip)
    /// - TAR (.tar)
    /// - TAR.GZ (.tar.gz, .tgz)
    /// - TAR.BZ2 (.tar.bz2, .tbz, .tbz2)
    /// - 7-Zip (.7z)
    /// - RAR (.rar) - read-only
    /// - GZIP (.gz) - single file
    /// - BZIP2 (.bz2) - single file
    ///
    /// Write operations (copy to archive) only supported for writable formats (ZIP, TAR).
    /// </remarks>
    [Guid("A9B8C7D6-5E4F-3A2B-1C0D-9E8F7A6B5C4D")]
    [ClassInterface(ClassInterfaceType.None)]
    [ComVisible(true)]
    public partial class Provider : ServicedComponent,
        IProcessInitializer,
        IBigDriveRegistration,
        IBigDriveEnumerate,
        IBigDriveFileInfo,
        IBigDriveFileData,
        IBigDriveFileOperations,
        IBigDriveDriveInfo,
        IBigDriveCapabilities
    {
        /// <summary>
        /// The trace source for logging.
        /// </summary>
        private static readonly BigDriveTraceSource DefaultTraceSource = BigDriveTraceSource.Instance;

        /// <summary>
        /// Gets the CLSID of this provider.
        /// </summary>
        public static Guid CLSID
        {
            get
            {
                Type providerType = typeof(Provider);
                GuidAttribute guidAttribute = (GuidAttribute)Attribute.GetCustomAttribute(providerType, typeof(GuidAttribute));
                return Guid.Parse(guidAttribute.Value);
            }
        }

        /// <summary>
        /// Gets the provider configuration for registry registration.
        /// </summary>
        private static BigDrive.ConfigProvider.Model.ProviderConfiguration ProviderConfig
        {
            get
            {
                return ProviderConfigurationFactory.Create();
            }
        }

        /// <summary>
        /// Called automatically by regsvcs.exe during COM registration.
        /// Sets the COM+ application identity to Interactive User and registers the provider.
        /// </summary>
        /// <param name="type">The type being registered.</param>
        [ComRegisterFunction]
        public static void ComRegister(Type type)
        {
            DefaultTraceSource.TraceInformation($"ComRegister: Registering provider {type.FullName}");

            // Set COM+ application identity to Interactive User
            SetApplicationIdentityToInteractiveUser("BigDrive.Provider.Archive");

            // Create an instance and call Register()
            Provider provider = new Provider();
            provider.Register();

            DefaultTraceSource.TraceInformation("ComRegister: Provider registration completed.");
        }

        /// <summary>
        /// Called automatically by regsvcs.exe during COM unregistration.
        /// </summary>
        /// <param name="type">The type being unregistered.</param>
        [ComUnregisterFunction]
        public static void ComUnregister(Type type)
        {
            DefaultTraceSource.TraceInformation($"ComUnregister: Unregistering provider {type.FullName}");

            // Create an instance and call Unregister()
            Provider provider = new Provider();
            provider.Unregister();
        }

        /// <summary>
        /// Sets the identity of a COM+ application to "Interactive User".
        /// This allows the provider to run as the logged-in user and access local files.
        /// </summary>
        /// <param name="applicationName">The name of the COM+ application.</param>
        private static void SetApplicationIdentityToInteractiveUser(string applicationName)
        {
            DefaultTraceSource.TraceInformation($"Setting COM+ application '{applicationName}' identity to Interactive User");

            Type comAdminType = Type.GetTypeFromProgID("COMAdmin.COMAdminCatalog");
            if (comAdminType == null)
            {
                DefaultTraceSource.TraceError("COMAdminCatalog is not available on this system.");
                return;
            }

            dynamic comAdmin = Activator.CreateInstance(comAdminType);
            try
            {
                dynamic applications = comAdmin.GetCollection("Applications");
                applications.Populate();

                foreach (dynamic app in applications)
                {
                    if (string.Equals((string)app.Name, applicationName, StringComparison.OrdinalIgnoreCase))
                    {
                        app.Value["Identity"] = "Interactive User";
                        app.Value["Password"] = "";
                        applications.SaveChanges();

                        DefaultTraceSource.TraceInformation($"COM+ application '{applicationName}' identity set to 'Interactive User'.");
                        return;
                    }
                }

                DefaultTraceSource.TraceInformation($"COM+ application '{applicationName}' not found.");
            }
            finally
            {
                if (comAdmin != null && Marshal.IsComObject(comAdmin))
                {
                    Marshal.ReleaseComObject(comAdmin);
                }
            }
        }

        /// <summary>
        /// Gets the Archive client wrapper for a specific drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>An ArchiveClientWrapper configured for the drive.</returns>
        private static ArchiveClientWrapper GetArchiveClient(Guid driveGuid)
        {
            return ArchiveClientWrapper.GetForDrive(driveGuid);
        }
    }
}
