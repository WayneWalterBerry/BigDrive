// <copyright file="Provider.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Flickr
{
    using System;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;

    using BigDrive.Interfaces;

    /// <summary>
    /// BigDrive provider for Flickr photo integration.
    /// Exposes Flickr photosets and photos as a virtual file system in Windows Explorer.
    /// </summary>
    /// <remarks>
    /// This provider supports drive-specific configuration. Each drive can have its own
    /// Flickr API credentials and OAuth tokens, allowing multiple Flickr accounts.
    /// 
    /// Drive-specific properties (stored in registry under each drive):
    /// - FlickrApiKey: The Flickr API key
    /// - FlickrApiSecret: The Flickr API secret
    /// - FlickrOAuthToken: OAuth access token
    /// - FlickrOAuthSecret: OAuth access token secret
    /// - FlickrUserId: The Flickr user ID (optional)
    /// </remarks>
    [Guid("B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B")]
    [ClassInterface(ClassInterfaceType.None)]
    [ComVisible(true)]
    public partial class Provider : ServicedComponent,
        IProcessInitializer,
        IBigDriveRegistration,
        IBigDriveEnumerate,
        IBigDriveFileInfo,
        IBigDriveFileOperations,
        IBigDriveFileData
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
        /// Called automatically by regsvcs.exe during COM registration.
        /// Sets the COM+ application identity to Interactive User and registers the provider.
        /// </summary>
        /// <param name="type">The type being registered.</param>
        [ComRegisterFunction]
        public static void ComRegister(Type type)
        {
            DefaultTraceSource.TraceInformation($"ComRegister: Registering provider {type.FullName}");

            // Set COM+ application identity to Interactive User
            SetApplicationIdentityToInteractiveUser("BigDrive.Provider.Flickr");

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
        /// This allows the provider to run as the logged-in user and access Credential Manager.
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
        /// Gets the Flickr client for a specific drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>A FlickrClientWrapper configured for the drive.</returns>
        private static FlickrClientWrapper GetFlickrClient(Guid driveGuid)
        {
            return FlickrClientWrapper.GetForDrive(driveGuid);
        }
    }
}
