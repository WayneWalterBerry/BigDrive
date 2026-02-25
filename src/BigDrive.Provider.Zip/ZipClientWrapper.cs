// <copyright file="ZipClientWrapper.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Zip
{
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.IO;
    using System.IO.Compression;
    using System.Linq;
    using System.Threading;

    using BigDrive.ConfigProvider;

    /// <summary>
    /// Wrapper for reading ZIP archive contents.
    /// Supports drive-specific configuration for different ZIP files.
    /// </summary>
    internal class ZipClientWrapper
    {
        /// <summary>
        /// Cache of Zip clients per drive GUID.
        /// </summary>
        private static readonly ConcurrentDictionary<Guid, ZipClientWrapper> DriveClients =
            new ConcurrentDictionary<Guid, ZipClientWrapper>();

        /// <summary>
        /// The drive GUID this client is configured for.
        /// </summary>
        private readonly Guid _driveGuid;

        /// <summary>
        /// The path to the ZIP file on the local file system.
        /// </summary>
        private readonly string _zipFilePath;

        /// <summary>
        /// The drive property name for the ZIP file path.
        /// </summary>
        private const string ZipFilePathProperty = "ZipFilePath";

        /// <summary>
        /// Initializes a new instance of the <see cref="ZipClientWrapper"/> class
        /// configured for a specific drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID to configure for.</param>
        private ZipClientWrapper(Guid driveGuid)
        {
            _driveGuid = driveGuid;
            _zipFilePath = DriveManager.ReadDriveProperty(driveGuid, ZipFilePathProperty, CancellationToken.None);
            EnsureZipFileExists();
        }

        /// <summary>
        /// Ensures the ZIP file exists, creating an empty archive if it doesn't.
        /// This allows mounting new ZIP files that will be populated later.
        /// </summary>
        private void EnsureZipFileExists()
        {
            if (string.IsNullOrEmpty(_zipFilePath))
            {
                return;
            }

            if (File.Exists(_zipFilePath))
            {
                return;
            }

            string directory = Path.GetDirectoryName(_zipFilePath);
            if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
            }

            using (ZipArchive archive = ZipFile.Open(_zipFilePath, ZipArchiveMode.Create))
            {
            }
        }

        /// <summary>
        /// Gets or creates a ZipClientWrapper for the specified drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>A <see cref="ZipClientWrapper"/> configured for the drive.</returns>
        public static ZipClientWrapper GetForDrive(Guid driveGuid)
        {
            return DriveClients.GetOrAdd(driveGuid, guid => new ZipClientWrapper(guid));
        }

        /// <summary>
        /// Gets the folder names at the specified path within the ZIP archive.
        /// </summary>
        /// <param name="normalizedPath">The normalized path (forward slashes, no leading/trailing separators). Empty string for root.</param>
        /// <returns>Array of folder names at the specified path.</returns>
        public string[] GetFolders(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_zipFilePath) || !File.Exists(_zipFilePath))
            {
                return Array.Empty<string>();
            }

            string prefix = string.IsNullOrEmpty(normalizedPath) ? "" : normalizedPath + "/";
            HashSet<string> folders = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            using (ZipArchive archive = ZipFile.OpenRead(_zipFilePath))
            {
                foreach (ZipArchiveEntry entry in archive.Entries)
                {
                    string fullName = entry.FullName.Replace('\\', '/');

                    // Skip entries that are not under the current path
                    if (!string.IsNullOrEmpty(prefix) && !fullName.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
                    {
                        continue;
                    }

                    // Get the relative path after the prefix
                    string relativePath = string.IsNullOrEmpty(prefix) ? fullName : fullName.Substring(prefix.Length);

                    // If relative path contains a slash, the first segment is a folder at this level
                    int slashIndex = relativePath.IndexOf('/');
                    if (slashIndex > 0)
                    {
                        string folderName = relativePath.Substring(0, slashIndex);
                        if (!string.IsNullOrEmpty(folderName))
                        {
                            folders.Add(SanitizeName(folderName));
                        }
                    }
                }
            }

            return folders.ToArray();
        }

        /// <summary>
        /// Gets the file names at the specified path within the ZIP archive.
        /// </summary>
        /// <param name="normalizedPath">The normalized path (forward slashes, no leading/trailing separators). Empty string for root.</param>
        /// <returns>Array of file names at the specified path.</returns>
        public string[] GetFiles(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_zipFilePath) || !File.Exists(_zipFilePath))
            {
                return Array.Empty<string>();
            }

            string prefix = string.IsNullOrEmpty(normalizedPath) ? "" : normalizedPath + "/";
            List<string> files = new List<string>();

            using (ZipArchive archive = ZipFile.OpenRead(_zipFilePath))
            {
                foreach (ZipArchiveEntry entry in archive.Entries)
                {
                    string fullName = entry.FullName.Replace('\\', '/');

                    // Skip directory entries (they end with /)
                    if (fullName.EndsWith("/"))
                    {
                        continue;
                    }

                    // Skip entries that are not under the current path
                    if (!string.IsNullOrEmpty(prefix) && !fullName.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
                    {
                        continue;
                    }

                    // Get the relative path after the prefix
                    string relativePath = string.IsNullOrEmpty(prefix) ? fullName : fullName.Substring(prefix.Length);

                    // Only include direct children (no slashes in relative path)
                    if (!relativePath.Contains("/") && !string.IsNullOrEmpty(relativePath))
                    {
                        files.Add(SanitizeName(relativePath));
                    }
                }
            }

            return files.ToArray();
        }

        /// <summary>
        /// Gets the last modified time for the specified file entry in the ZIP archive.
        /// </summary>
        /// <param name="normalizedPath">The normalized file path within the archive.</param>
        /// <returns>The last modified <see cref="DateTime"/>, or <see cref="DateTime.MinValue"/> if not found.</returns>
        public DateTime GetLastModifiedTime(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_zipFilePath) || !File.Exists(_zipFilePath) || string.IsNullOrEmpty(normalizedPath))
            {
                return DateTime.MinValue;
            }

            using (ZipArchive archive = ZipFile.OpenRead(_zipFilePath))
            {
                ZipArchiveEntry entry = FindEntry(archive, normalizedPath);
                if (entry != null)
                {
                    return entry.LastWriteTime.DateTime;
                }
            }

            return DateTime.MinValue;
        }

        /// <summary>
        /// Gets the uncompressed file size for the specified file entry in the ZIP archive.
        /// </summary>
        /// <param name="normalizedPath">The normalized file path within the archive.</param>
        /// <returns>The uncompressed file size in bytes, or 0 if not found.</returns>
        public ulong GetFileSize(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_zipFilePath) || !File.Exists(_zipFilePath) || string.IsNullOrEmpty(normalizedPath))
            {
                return 0;
            }

            using (ZipArchive archive = ZipFile.OpenRead(_zipFilePath))
            {
                ZipArchiveEntry entry = FindEntry(archive, normalizedPath);
                if (entry != null)
                {
                    return (ulong)entry.Length;
                }
            }

            return 0;
        }

        /// <summary>
        /// Gets the file data for the specified file entry in the ZIP archive.
        /// </summary>
        /// <param name="normalizedPath">The normalized file path within the archive.</param>
        /// <returns>The file data as a byte array, or null if not found.</returns>
        public byte[] GetFileData(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_zipFilePath) || !File.Exists(_zipFilePath) || string.IsNullOrEmpty(normalizedPath))
            {
                return null;
            }

            using (ZipArchive archive = ZipFile.OpenRead(_zipFilePath))
            {
                ZipArchiveEntry entry = FindEntry(archive, normalizedPath);
                if (entry == null)
                {
                    return null;
                }

                using (Stream entryStream = entry.Open())
                using (MemoryStream memoryStream = new MemoryStream())
                {
                    entryStream.CopyTo(memoryStream);
                    return memoryStream.ToArray();
                }
            }
        }

        /// <summary>
        /// Finds a ZIP archive entry by its normalized path within an open archive.
        /// </summary>
        /// <param name="archive">The open <see cref="ZipArchive"/> to search.</param>
        /// <param name="normalizedPath">The normalized file path within the archive (forward slashes).</param>
        /// <returns>The <see cref="ZipArchiveEntry"/> if found, or null.</returns>
        private static ZipArchiveEntry FindEntry(ZipArchive archive, string normalizedPath)
        {
            // Try exact match first
            ZipArchiveEntry entry = archive.GetEntry(normalizedPath);
            if (entry != null)
            {
                return entry;
            }

            // Try case-insensitive match
            foreach (ZipArchiveEntry candidate in archive.Entries)
            {
                string candidatePath = candidate.FullName.Replace('\\', '/');
                if (string.Equals(candidatePath, normalizedPath, StringComparison.OrdinalIgnoreCase))
                {
                    return candidate;
                }
            }

            return null;
        }

        /// <summary>
        /// Adds a file to the ZIP archive from a local file path.
        /// </summary>
        /// <param name="localFilePath">The local file path to add.</param>
        /// <param name="normalizedPath">The normalized path within the archive (forward slashes).</param>
        public void AddFile(string localFilePath, string normalizedPath)
        {
            if (string.IsNullOrEmpty(_zipFilePath) || !File.Exists(_zipFilePath))
            {
                throw new FileNotFoundException("ZIP file not found: " + _zipFilePath);
            }

            if (!File.Exists(localFilePath))
            {
                throw new FileNotFoundException("Local file not found: " + localFilePath);
            }

            using (ZipArchive archive = ZipFile.Open(_zipFilePath, ZipArchiveMode.Update))
            {
                ZipArchiveEntry existingEntry = archive.GetEntry(normalizedPath);
                if (existingEntry != null)
                {
                    existingEntry.Delete();
                }

                archive.CreateEntryFromFile(localFilePath, normalizedPath, CompressionLevel.Optimal);
            }
        }

        /// <summary>
        /// Deletes an entry (file or directory) from the ZIP archive.
        /// </summary>
        /// <param name="normalizedPath">The normalized path within the archive (forward slashes).</param>
        public void DeleteEntry(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_zipFilePath) || !File.Exists(_zipFilePath))
            {
                throw new FileNotFoundException("ZIP file not found: " + _zipFilePath);
            }

            using (ZipArchive archive = ZipFile.Open(_zipFilePath, ZipArchiveMode.Update))
            {
                ZipArchiveEntry entry = FindEntry(archive, normalizedPath);
                if (entry == null)
                {
                    throw new FileNotFoundException("Entry not found in ZIP: " + normalizedPath);
                }

                entry.Delete();

                string dirPrefix = normalizedPath.TrimEnd('/') + "/";
                List<ZipArchiveEntry> childEntries = archive.Entries
                    .Where(e => e.FullName.Replace('\\', '/').StartsWith(dirPrefix, StringComparison.OrdinalIgnoreCase))
                    .ToList();

                foreach (ZipArchiveEntry childEntry in childEntries)
                {
                    childEntry.Delete();
                }
            }
        }

        /// <summary>
        /// Creates a directory entry in the ZIP archive.
        /// </summary>
        /// <param name="normalizedPath">The normalized directory path (forward slashes).</param>
        public void CreateDirectory(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_zipFilePath) || !File.Exists(_zipFilePath))
            {
                throw new FileNotFoundException("ZIP file not found: " + _zipFilePath);
            }

            string dirPath = normalizedPath.TrimEnd('/') + "/";

            using (ZipArchive archive = ZipFile.Open(_zipFilePath, ZipArchiveMode.Update))
            {
                ZipArchiveEntry existingEntry = archive.GetEntry(dirPath);
                if (existingEntry != null)
                {
                    return;
                }

                archive.CreateEntry(dirPath);
            }
        }

        /// <summary>
        /// Moves/renames an entry within the ZIP archive.
        /// </summary>
        /// <param name="sourceNormalizedPath">The source normalized path.</param>
        /// <param name="destNormalizedPath">The destination normalized path.</param>
        public void MoveEntry(string sourceNormalizedPath, string destNormalizedPath)
        {
            if (string.IsNullOrEmpty(_zipFilePath) || !File.Exists(_zipFilePath))
            {
                throw new FileNotFoundException("ZIP file not found: " + _zipFilePath);
            }

            using (ZipArchive archive = ZipFile.Open(_zipFilePath, ZipArchiveMode.Update))
            {
                ZipArchiveEntry sourceEntry = FindEntry(archive, sourceNormalizedPath);
                if (sourceEntry == null)
                {
                    throw new FileNotFoundException("Source entry not found: " + sourceNormalizedPath);
                }

                bool isDirectory = sourceEntry.FullName.EndsWith("/");

                if (isDirectory)
                {
                    string srcPrefix = sourceNormalizedPath.TrimEnd('/') + "/";
                    string destPrefix = destNormalizedPath.TrimEnd('/') + "/";

                    List<ZipArchiveEntry> allEntries = archive.Entries
                        .Where(e => e.FullName.Replace('\\', '/').StartsWith(srcPrefix, StringComparison.OrdinalIgnoreCase))
                        .ToList();

                    foreach (ZipArchiveEntry entry in allEntries)
                    {
                        string oldFullName = entry.FullName.Replace('\\', '/');
                        string newFullName = destPrefix + oldFullName.Substring(srcPrefix.Length);

                        ZipArchiveEntry newEntry = archive.CreateEntry(newFullName);
                        using (Stream oldStream = entry.Open())
                        using (Stream newStream = newEntry.Open())
                        {
                            oldStream.CopyTo(newStream);
                        }

                        entry.Delete();
                    }

                    archive.CreateEntry(destPrefix);
                }
                else
                {
                    ZipArchiveEntry newEntry = archive.CreateEntry(destNormalizedPath);
                    using (Stream oldStream = sourceEntry.Open())
                    using (Stream newStream = newEntry.Open())
                    {
                        oldStream.CopyTo(newStream);
                    }

                    newEntry.LastWriteTime = sourceEntry.LastWriteTime;
                    sourceEntry.Delete();
                }
            }
        }

        /// <summary>
        /// Sanitizes a name by removing invalid file system characters.
        /// </summary>
        /// <param name="name">The original name.</param>
        /// <returns>A sanitized name safe for use in Windows file paths.</returns>
        private static string SanitizeName(string name)
        {
            if (string.IsNullOrEmpty(name))
            {
                return "Untitled";
            }

            char[] invalidChars = Path.GetInvalidFileNameChars();
            foreach (char c in invalidChars)
            {
                name = name.Replace(c, '_');
            }

            return name;
        }
    }
}
