// <copyright file="Provider.IBigDriveEnumerate.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Flickr
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveEnumerate"/> for the Flickr provider.
    /// Enumerates Flickr photosets as folders and photos as files.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Enumerates the folder names at the specified path.
        /// At the root level, returns Flickr photosets as folders.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The path to enumerate folders under.</param>
        /// <returns>Array of folder names at the specified path.</returns>
        public string[] EnumerateFolders(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"EnumerateFolders: driveGuid={driveGuid}, path={path}");

                FlickrClientWrapper flickrClient = GetFlickrClient(driveGuid);

                // At root level, return photosets as folders
                if (IsRootPath(path))
                {
                    var photosets = flickrClient.GetPhotosets();
                    return photosets.Select(ps => SanitizeFolderName(ps.Title)).ToArray();
                }

                // Photosets don't have subfolders
                return Array.Empty<string>();
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"EnumerateFolders failed: {ex.Message}");
                return Array.Empty<string>();
            }
        }

        /// <summary>
        /// Enumerates the file names at the specified path.
        /// Returns photos within a photoset as files.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The path to enumerate files under.</param>
        /// <returns>Array of file names at the specified path.</returns>
        public string[] EnumerateFiles(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"EnumerateFiles: driveGuid={driveGuid}, path={path}");

                FlickrClientWrapper flickrClient = GetFlickrClient(driveGuid);

                // Root level has no files, only photoset folders
                if (IsRootPath(path))
                {
                    return Array.Empty<string>();
                }

                // Get photoset name from path
                string photosetName = GetPhotosetNameFromPath(path);
                if (string.IsNullOrEmpty(photosetName))
                {
                    return Array.Empty<string>();
                }

                var photos = flickrClient.GetPhotosInPhotoset(photosetName);
                return photos.Select(p => SanitizeFileName(p.Title) + ".jpg").ToArray();
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"EnumerateFiles failed: {ex.Message}");
                return Array.Empty<string>();
            }
        }

        /// <summary>
        /// Determines if the path represents the root.
        /// </summary>
        /// <param name="path">The path to check.</param>
        /// <returns>True if the path is the root.</returns>
        private static bool IsRootPath(string path)
        {
            return string.IsNullOrEmpty(path) || path == "\\" || path == "//" || path == "/";
        }

        /// <summary>
        /// Extracts the photoset name from a path.
        /// </summary>
        /// <param name="path">The path (e.g., "\MyPhotoset").</param>
        /// <returns>The photoset name.</returns>
        private static string GetPhotosetNameFromPath(string path)
        {
            if (string.IsNullOrEmpty(path))
            {
                return null;
            }

            var segments = path.Trim('\\', '/').Split(new[] { '\\', '/' }, StringSplitOptions.RemoveEmptyEntries);
            return segments.Length > 0 ? segments[0] : null;
        }

        /// <summary>
        /// Extracts the photo name from a path.
        /// </summary>
        /// <param name="path">The path (e.g., "\MyPhotoset\Photo.jpg").</param>
        /// <returns>The photo name without extension.</returns>
        private static string GetPhotoNameFromPath(string path)
        {
            if (string.IsNullOrEmpty(path))
            {
                return null;
            }

            var segments = path.Trim('\\', '/').Split(new[] { '\\', '/' }, StringSplitOptions.RemoveEmptyEntries);
            if (segments.Length >= 2)
            {
                string fileName = segments[segments.Length - 1];
                // Remove .jpg extension if present
                if (fileName.EndsWith(".jpg", StringComparison.OrdinalIgnoreCase))
                {
                    return fileName.Substring(0, fileName.Length - 4);
                }
                return fileName;
            }
            return null;
        }

        /// <summary>
        /// Sanitizes a folder name by removing invalid characters.
        /// </summary>
        /// <param name="name">The original name.</param>
        /// <returns>A sanitized folder name.</returns>
        private static string SanitizeFolderName(string name)
        {
            if (string.IsNullOrEmpty(name))
            {
                return "Untitled";
            }

            char[] invalidChars = System.IO.Path.GetInvalidFileNameChars();
            foreach (char c in invalidChars)
            {
                name = name.Replace(c, '_');
            }
            return name;
        }

        /// <summary>
        /// Sanitizes a file name by removing invalid characters.
        /// </summary>
        /// <param name="name">The original name.</param>
        /// <returns>A sanitized file name.</returns>
        private static string SanitizeFileName(string name)
        {
            return SanitizeFolderName(name);
        }
    }
}
