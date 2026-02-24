// <copyright file="Provider.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Flickr
{
    using BigDrive.Interfaces;
    using System;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;

    /// <summary>
    /// BigDrive provider for Flickr photo integration.
    /// Exposes Flickr photosets and photos as a virtual file system in Windows Explorer.
    /// </summary>
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
        /// The Flickr client wrapper for API calls.
        /// </summary>
        private static readonly FlickrClientWrapper FlickrClient = new FlickrClientWrapper();

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
    }
}
