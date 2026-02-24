// <copyright file="Provider.IBigDriveFileOperations.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Flickr
{
    using System;
    using System.IO;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveFileOperations"/> for the Flickr provider.
    /// Handles file operations for Flickr photos.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Copies a local file to Flickr (uploads a photo).
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="localFilePath">The local file path to copy from.</param>
        /// <param name="bigDriveTargetPath">The destination path in BigDrive (photoset).</param>
        public void CopyFileToBigDrive(Guid driveGuid, string localFilePath, string bigDriveTargetPath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"CopyFileToBigDrive: localFilePath={localFilePath}, targetPath={bigDriveTargetPath}");

                FlickrClientWrapper flickrClient = GetFlickrClient(driveGuid);

                if (!File.Exists(localFilePath))
                {
                    throw new FileNotFoundException("Local file not found.", localFilePath);
                }

                string photosetName = GetPhotosetNameFromPath(bigDriveTargetPath);
                string photoTitle = Path.GetFileNameWithoutExtension(bigDriveTargetPath);

                flickrClient.UploadPhoto(localFilePath, photoTitle, photosetName);
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"CopyFileToBigDrive failed: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Copies a Flickr photo to local storage (downloads a photo).
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveFilePath">The BigDrive file path to copy from.</param>
        /// <param name="localTargetPath">The destination path in local storage.</param>
        public void CopyFileFromBigDrive(Guid driveGuid, string bigDriveFilePath, string localTargetPath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"CopyFileFromBigDrive: bigDriveFilePath={bigDriveFilePath}, localTargetPath={localTargetPath}");

                FlickrClientWrapper flickrClient = GetFlickrClient(driveGuid);

                string photosetName = GetPhotosetNameFromPath(bigDriveFilePath);
                string photoName = GetPhotoNameFromPath(bigDriveFilePath);

                if (string.IsNullOrEmpty(photosetName) || string.IsNullOrEmpty(photoName))
                {
                    throw new FileNotFoundException("Photo not found.", bigDriveFilePath);
                }

                flickrClient.DownloadPhoto(photosetName, photoName, localTargetPath);
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"CopyFileFromBigDrive failed: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Deletes a photo from Flickr.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveFilePath">The file path to delete.</param>
        public void DeleteFile(Guid driveGuid, string bigDriveFilePath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"DeleteFile: bigDriveFilePath={bigDriveFilePath}");

                FlickrClientWrapper flickrClient = GetFlickrClient(driveGuid);

                string photosetName = GetPhotosetNameFromPath(bigDriveFilePath);
                string photoName = GetPhotoNameFromPath(bigDriveFilePath);

                if (string.IsNullOrEmpty(photoName))
                {
                    // Deleting a photoset
                    flickrClient.DeletePhotoset(photosetName);
                }
                else
                {
                    // Deleting a photo
                    flickrClient.DeletePhoto(photosetName, photoName);
                }
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"DeleteFile failed: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Creates a new photoset (album) in Flickr.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveDirectoryPath">The directory path to create.</param>
        public void CreateDirectory(Guid driveGuid, string bigDriveDirectoryPath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"CreateDirectory: bigDriveDirectoryPath={bigDriveDirectoryPath}");

                FlickrClientWrapper flickrClient = GetFlickrClient(driveGuid);

                string photosetName = GetPhotosetNameFromPath(bigDriveDirectoryPath);
                if (string.IsNullOrEmpty(photosetName))
                {
                    throw new ArgumentException("Invalid photoset name.", nameof(bigDriveDirectoryPath));
                }

                flickrClient.CreatePhotoset(photosetName);
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"CreateDirectory failed: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Opens a Flickr photo with the default application.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveFilePath">The file path to open.</param>
        /// <param name="hwndParent">Parent window handle for any UI.</param>
        public void OpenFile(Guid driveGuid, string bigDriveFilePath, IntPtr hwndParent)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"OpenFile: bigDriveFilePath={bigDriveFilePath}");

                FlickrClientWrapper flickrClient = GetFlickrClient(driveGuid);

                string photosetName = GetPhotosetNameFromPath(bigDriveFilePath);
                string photoName = GetPhotoNameFromPath(bigDriveFilePath);

                if (string.IsNullOrEmpty(photosetName) || string.IsNullOrEmpty(photoName))
                {
                    throw new FileNotFoundException("Photo not found.", bigDriveFilePath);
                }

                // Get the photo URL and open in browser
                string photoUrl = flickrClient.GetPhotoUrl(photosetName, photoName);
                if (!string.IsNullOrEmpty(photoUrl))
                {
                    System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo
                    {
                        FileName = photoUrl,
                        UseShellExecute = true
                    });
                }
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"OpenFile failed: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Moves a photo within Flickr (between photosets).
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="sourcePath">The source file path.</param>
        /// <param name="destinationPath">The destination file path.</param>
        public void MoveFile(Guid driveGuid, string sourcePath, string destinationPath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"MoveFile: sourcePath={sourcePath}, destinationPath={destinationPath}");

                FlickrClientWrapper flickrClient = GetFlickrClient(driveGuid);

                string sourcePhotosetName = GetPhotosetNameFromPath(sourcePath);
                string sourcePhotoName = GetPhotoNameFromPath(sourcePath);
                string destPhotosetName = GetPhotosetNameFromPath(destinationPath);

                if (string.IsNullOrEmpty(sourcePhotosetName) || string.IsNullOrEmpty(sourcePhotoName) || string.IsNullOrEmpty(destPhotosetName))
                {
                    throw new ArgumentException("Invalid source or destination path.");
                }

                flickrClient.MovePhoto(sourcePhotosetName, sourcePhotoName, destPhotosetName);
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"MoveFile failed: {ex.Message}");
                throw;
            }
        }
    }
}
