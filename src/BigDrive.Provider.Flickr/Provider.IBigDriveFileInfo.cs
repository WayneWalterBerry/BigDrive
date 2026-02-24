// <copyright file="Provider.IBigDriveFileInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Flickr
{
    using System;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveFileInfo"/> for the Flickr provider.
    /// Provides file metadata for Flickr photos.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the last modified time for the photo at the specified path.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The file path (e.g., "\Photoset\Photo.jpg").</param>
        /// <returns>The last modified <see cref="DateTime"/> of the photo, or <see cref="DateTime.MinValue"/> if not found.</returns>
        public DateTime LastModifiedTime(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"LastModifiedTime: driveGuid={driveGuid}, path={path}");

                FlickrClientWrapper flickrClient = GetFlickrClient(driveGuid);

                string photosetName = GetPhotosetNameFromPath(path);
                string photoName = GetPhotoNameFromPath(path);

                if (string.IsNullOrEmpty(photosetName) || string.IsNullOrEmpty(photoName))
                {
                    return DateTime.MinValue;
                }

                var photoInfo = flickrClient.GetPhotoInfo(photosetName, photoName);
                if (photoInfo != null)
                {
                    return photoInfo.DateUploaded;
                }

                return DateTime.MinValue;
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"LastModifiedTime failed: {ex.Message}");
                return DateTime.MinValue;
            }
        }

        /// <summary>
        /// Gets the file size for the photo at the specified path.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The file path (e.g., "\Photoset\Photo.jpg").</param>
        /// <returns>The file size in bytes, or 0 if not found.</returns>
        public ulong GetFileSize(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"GetFileSize: driveGuid={driveGuid}, path={path}");

                FlickrClientWrapper flickrClient = GetFlickrClient(driveGuid);

                string photosetName = GetPhotosetNameFromPath(path);
                string photoName = GetPhotoNameFromPath(path);

                if (string.IsNullOrEmpty(photosetName) || string.IsNullOrEmpty(photoName))
                {
                    return 0;
                }

                var photoInfo = flickrClient.GetPhotoInfo(photosetName, photoName);
                if (photoInfo != null)
                {
                    return photoInfo.FileSize;
                }

                return 0;
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"GetFileSize failed: {ex.Message}");
                return 0;
            }
        }
    }
}
