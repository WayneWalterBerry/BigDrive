// <copyright file="IsoClientWrapper.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Iso
{
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using DiscUtils.Iso9660;

    /// <summary>
    /// Wrapper for reading ISO 9660 and UDF disc image contents using DiscUtils library.
    /// Supports browsing and extracting files from ISO images.
    /// </summary>
    internal class IsoClientWrapper
    {
        /// <summary>
        /// Cache of ISO clients per drive GUID.
        /// </summary>
        private static readonly ConcurrentDictionary<Guid, IsoClientWrapper> DriveClients =
            new ConcurrentDictionary<Guid, IsoClientWrapper>();

        /// <summary>
        /// The drive GUID this client is configured for.
        /// </summary>
        private readonly Guid m_driveGuid;

        /// <summary>
        /// The path to the ISO file on the local file system.
        /// </summary>
        private readonly string m_isoFilePath;

        /// <summary>
        /// The drive property name for the ISO file path.
        /// </summary>
        private const string IsoFilePathProperty = "IsoFilePath";

        /// <summary>
        /// Initializes a new instance of the <see cref="IsoClientWrapper"/> class
        /// configured for a specific drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID to configure for.</param>
        private IsoClientWrapper(Guid driveGuid)
        {
            m_driveGuid = driveGuid;
            m_isoFilePath = DriveManager.ReadDriveProperty(driveGuid, IsoFilePathProperty, CancellationToken.None);
        }

        /// <summary>
        /// Gets or creates an IsoClientWrapper for the specified drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>An <see cref="IsoClientWrapper"/> configured for the drive.</returns>
        public static IsoClientWrapper GetForDrive(Guid driveGuid)
        {
            return DriveClients.GetOrAdd(driveGuid, guid => new IsoClientWrapper(guid));
        }

        /// <summary>
        /// Gets the folder names at the specified path within the ISO image.
        /// </summary>
        /// <param name="normalizedPath">The normalized path (forward slashes, no leading/trailing separators). Empty string for root.</param>
        /// <returns>Array of folder names at the specified path.</returns>
        public string[] GetFolders(string normalizedPath)
        {
            if (string.IsNullOrEmpty(m_isoFilePath) || !File.Exists(m_isoFilePath))
            {
                return Array.Empty<string>();
            }

            try
            {
                using (FileStream isoStream = File.OpenRead(m_isoFilePath))
                using (CDReader reader = new CDReader(isoStream, true))
                {
                    string isoPath = ConvertToIsoPath(normalizedPath);
                    
                    if (!reader.DirectoryExists(isoPath))
                    {
                        return Array.Empty<string>();
                    }

                    string[] directories = reader.GetDirectories(isoPath);
                    List<string> folderNames = new List<string>();

                    foreach (string directory in directories)
                    {
                        string folderName = Path.GetFileName(directory.TrimEnd('\\', '/'));
                        if (!string.IsNullOrEmpty(folderName))
                        {
                            folderNames.Add(SanitizeName(folderName));
                        }
                    }

                    return folderNames.ToArray();
                }
            }
            catch (Exception ex)
            {
                BigDriveTraceSource.Instance.TraceError($"IsoClientWrapper.GetFolders: Error reading ISO: {ex.Message}");
                return Array.Empty<string>();
            }
        }

        /// <summary>
        /// Gets the file names at the specified path within the ISO image.
        /// </summary>
        /// <param name="normalizedPath">The normalized path (forward slashes, no leading/trailing separators). Empty string for root.</param>
        /// <returns>Array of file names at the specified path.</returns>
        public string[] GetFiles(string normalizedPath)
        {
            if (string.IsNullOrEmpty(m_isoFilePath) || !File.Exists(m_isoFilePath))
            {
                return Array.Empty<string>();
            }

            try
            {
                using (FileStream isoStream = File.OpenRead(m_isoFilePath))
                using (CDReader reader = new CDReader(isoStream, true))
                {
                    string isoPath = ConvertToIsoPath(normalizedPath);
                    
                    if (!reader.DirectoryExists(isoPath))
                    {
                        return Array.Empty<string>();
                    }

                    string[] files = reader.GetFiles(isoPath);
                    List<string> fileNames = new List<string>();

                    foreach (string file in files)
                    {
                        string fileName = Path.GetFileName(file);
                        if (!string.IsNullOrEmpty(fileName))
                        {
                            fileNames.Add(SanitizeName(fileName));
                        }
                    }

                    return fileNames.ToArray();
                }
            }
            catch (Exception ex)
            {
                BigDriveTraceSource.Instance.TraceError($"IsoClientWrapper.GetFiles: Error reading ISO: {ex.Message}");
                return Array.Empty<string>();
            }
        }

        /// <summary>
        /// Gets the last modified time for the specified file in the ISO image.
        /// </summary>
        /// <param name="normalizedPath">The normalized file path within the ISO image.</param>
        /// <returns>The last modified <see cref="DateTime"/>, or <see cref="DateTime.MinValue"/> if not found.</returns>
        public DateTime GetLastModifiedTime(string normalizedPath)
        {
            if (string.IsNullOrEmpty(m_isoFilePath) || !File.Exists(m_isoFilePath) || string.IsNullOrEmpty(normalizedPath))
            {
                return DateTime.MinValue;
            }

            try
            {
                using (FileStream isoStream = File.OpenRead(m_isoFilePath))
                using (CDReader reader = new CDReader(isoStream, true))
                {
                    string isoPath = ConvertToIsoPath(normalizedPath);
                    
                    if (!reader.FileExists(isoPath))
                    {
                        return DateTime.MinValue;
                    }

                    return reader.GetLastWriteTimeUtc(isoPath);
                }
            }
            catch (Exception ex)
            {
                BigDriveTraceSource.Instance.TraceError($"IsoClientWrapper.GetLastModifiedTime: Error reading ISO: {ex.Message}");
                return DateTime.MinValue;
            }
        }

        /// <summary>
        /// Gets the file size for the specified file in the ISO image.
        /// </summary>
        /// <param name="normalizedPath">The normalized file path within the ISO image.</param>
        /// <returns>The file size in bytes, or 0 if not found.</returns>
        public long GetFileSize(string normalizedPath)
        {
            if (string.IsNullOrEmpty(m_isoFilePath) || !File.Exists(m_isoFilePath) || string.IsNullOrEmpty(normalizedPath))
            {
                return 0;
            }

            try
            {
                using (FileStream isoStream = File.OpenRead(m_isoFilePath))
                using (CDReader reader = new CDReader(isoStream, true))
                {
                    string isoPath = ConvertToIsoPath(normalizedPath);
                    
                    if (!reader.FileExists(isoPath))
                    {
                        return 0;
                    }

                    DiscUtils.DiscFileInfo fileInfo = reader.GetFileInfo(isoPath);
                    return fileInfo.Length;
                }
            }
            catch (Exception ex)
            {
                BigDriveTraceSource.Instance.TraceError($"IsoClientWrapper.GetFileSize: Error reading ISO: {ex.Message}");
                return 0;
            }
        }

        /// <summary>
        /// Opens a file from the ISO image for reading.
        /// </summary>
        /// <param name="normalizedPath">The normalized file path within the ISO image.</param>
        /// <returns>A <see cref="Stream"/> for reading the file contents.</returns>
        public Stream OpenFile(string normalizedPath)
        {
            if (string.IsNullOrEmpty(m_isoFilePath) || !File.Exists(m_isoFilePath) || string.IsNullOrEmpty(normalizedPath))
            {
                return null;
            }

            try
            {
                FileStream isoStream = File.OpenRead(m_isoFilePath);
                CDReader reader = new CDReader(isoStream, true);
                
                string isoPath = ConvertToIsoPath(normalizedPath);
                
                if (!reader.FileExists(isoPath))
                {
                    reader.Dispose();
                    isoStream.Dispose();
                    return null;
                }

                Stream fileStream = reader.OpenFile(isoPath, FileMode.Open, FileAccess.Read);
                
                // Copy to memory stream so we can dispose the reader and ISO stream
                MemoryStream memoryStream = new MemoryStream();
                fileStream.CopyTo(memoryStream);
                memoryStream.Position = 0;
                
                fileStream.Dispose();
                reader.Dispose();
                isoStream.Dispose();
                
                return memoryStream;
            }
            catch (Exception ex)
            {
                BigDriveTraceSource.Instance.TraceError($"IsoClientWrapper.OpenFile: Error opening file from ISO: {ex.Message}");
                return null;
            }
        }

        /// <summary>
        /// Converts a normalized path (forward slashes, no leading slash) to ISO path format.
        /// </summary>
        /// <param name="normalizedPath">The normalized path.</param>
        /// <returns>The ISO path with backslashes and leading backslash.</returns>
        private static string ConvertToIsoPath(string normalizedPath)
        {
            if (string.IsNullOrEmpty(normalizedPath))
            {
                return "\\";
            }

            return "\\" + normalizedPath.Replace('/', '\\');
        }

        /// <summary>
        /// Sanitizes a file or folder name by removing invalid characters.
        /// ISO 9660 has specific naming constraints that may need cleanup.
        /// </summary>
        /// <param name="name">The name to sanitize.</param>
        /// <returns>The sanitized name.</returns>
        private static string SanitizeName(string name)
        {
            if (string.IsNullOrEmpty(name))
            {
                return name;
            }

            // Remove version suffix that ISO 9660 adds (e.g., "file.txt;1" becomes "file.txt")
            int semicolonIndex = name.IndexOf(';');
            if (semicolonIndex > 0)
            {
                name = name.Substring(0, semicolonIndex);
            }

            return name;
        }
    }
}
