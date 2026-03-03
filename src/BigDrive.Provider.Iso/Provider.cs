// <copyright file="Provider.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Iso
{
    using System;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;

    using BigDrive.Interfaces;

    /// <summary>
    /// BigDrive provider for browsing ISO 9660 and UDF disc image contents as a virtual file system.
    /// Exposes the directory structure and files inside a local ISO image file.
    /// </summary>
    /// <remarks>
    /// This provider supports drive-specific configuration. Each drive points to a
    /// different ISO file on the local file system via the IsoFilePath drive property.
    ///
    /// Drive-specific properties (stored in registry under each drive):
    /// - IsoFilePath: The full path to the local ISO file to browse.
    ///
    /// Supported formats:
    /// - ISO 9660 (CD-ROM standard)
    /// - Joliet extensions (long filenames, Unicode)
    /// - Rock Ridge extensions (POSIX attributes)
    /// - UDF (Universal Disk Format for DVDs)
    /// </remarks>
    [Guid("F8E7D6C5-B4A3-9281-7605-4C3B2A1D0E9F")]
    [ClassInterface(ClassInterfaceType.None)]
    [ComVisible(true)]
    public partial class Provider : ServicedComponent,
        IProcessInitializer,
        IBigDriveRegistration,
        IBigDriveDriveInfo,
        IBigDriveCapabilities,
        IBigDriveEnumerate,
        IBigDriveFileInfo,
        IBigDriveFileData
    {
        /// <summary>
        /// The trace source for logging.
        /// </summary>
        private static readonly BigDriveTraceSource DefaultTraceSource = BigDriveTraceSource.Instance;

        /// <summary>
        /// Static constructor to ensure AssemblyResolver is initialized early.
        /// </summary>
        static Provider()
        {
            // Force AssemblyResolver static constructor to run
            AssemblyResolver.Initialize();
        }

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
            SetApplicationIdentityToInteractiveUser("BigDrive.Provider.Iso");

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
        /// Gets the ISO client wrapper for a specific drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>An IsoClientWrapper configured for the drive.</returns>
        private static IsoClientWrapper GetIsoClient(Guid driveGuid)
        {
            return IsoClientWrapper.GetForDrive(driveGuid);
        }

        /// <summary>
        /// Normalizes a path by trimming leading/trailing separators and converting to forward slashes.
        /// Returns an empty string for root paths.
        /// </summary>
        /// <param name="path">The path to normalize.</param>
        /// <returns>The normalized path, or empty string for root.</returns>
        private static string NormalizePath(string path)
        {
            if (string.IsNullOrEmpty(path) || path == "\\" || path == "/" || path == "//")
            {
                return string.Empty;
            }

            return path.Trim('\\', '/').Replace('\\', '/');
        }
    }
}
