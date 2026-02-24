// <copyright file="Provider.IBigDriveFileData.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Flickr
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveFileData"/> for the Flickr provider.
    /// Provides photo data as a COM IStream.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the photo data as a stream for the file at the specified path.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The file path (e.g., "\Photoset\Photo.jpg").</param>
        /// <param name="stream">When this method returns, contains the IStream with the photo data.</param>
        /// <returns>HRESULT indicating success or failure.</returns>
        public int GetFileData(Guid driveGuid, string path, [MarshalAs(UnmanagedType.Interface)] out IStream stream)
        {
            stream = null;

            try
            {
                DefaultTraceSource.TraceInformation($"GetFileData: driveGuid={driveGuid}, path={path}");

                string photosetName = GetPhotosetNameFromPath(path);
                string photoName = GetPhotoNameFromPath(path);

                if (string.IsNullOrEmpty(photosetName) || string.IsNullOrEmpty(photoName))
                {
                    // E_FILENOTFOUND
                    return unchecked((int)0x80070002);
                }

                byte[] photoData = FlickrClient.GetPhotoData(photosetName, photoName);
                if (photoData != null && photoData.Length > 0)
                {
                    stream = new ComStream(new MemoryStream(photoData));
                    return 0; // S_OK
                }

                // E_FILENOTFOUND
                return unchecked((int)0x80070002);
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"GetFileData failed: {ex.Message}");
                // E_FAIL
                return unchecked((int)0x80004005);
            }
        }
    }
}
