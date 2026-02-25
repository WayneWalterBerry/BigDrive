// <copyright file="ArchiveClientWrapper.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Archive
{
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using SharpCompress.Archives;
    using SharpCompress.Common;
    using SharpCompress.Writers;

    /// <summary>
    /// Wrapper for reading archive contents using SharpCompress library.
    /// Supports multiple archive formats: ZIP, TAR, TAR.GZ, 7z, RAR, etc.
    /// </summary>
    internal class ArchiveClientWrapper
    {
        /// <summary>
        /// Cache of Archive clients per drive GUID.
        /// </summary>
        private static readonly ConcurrentDictionary<Guid, ArchiveClientWrapper> DriveClients =
            new ConcurrentDictionary<Guid, ArchiveClientWrapper>();

        /// <summary>
        /// The drive GUID this client is configured for.
        /// </summary>
        private readonly Guid _driveGuid;

        /// <summary>
        /// The path to the archive file on the local file system.
        /// </summary>
        private readonly string _archiveFilePath;

        /// <summary>
        /// The drive property name for the archive file path.
        /// </summary>
        private const string ArchiveFilePathProperty = "ArchiveFilePath";

        /// <summary>
        /// Initializes a new instance of the <see cref="ArchiveClientWrapper"/> class
        /// configured for a specific drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID to configure for.</param>
        private ArchiveClientWrapper(Guid driveGuid)
        {
            _driveGuid = driveGuid;
            _archiveFilePath = DriveManager.ReadDriveProperty(driveGuid, ArchiveFilePathProperty, CancellationToken.None);
            EnsureArchiveFileExists();
        }

        /// <summary>
        /// Ensures the archive file exists, creating an empty archive if it doesn't.
        /// Only creates writable formats (ZIP, TAR).
        /// </summary>
        private void EnsureArchiveFileExists()
        {
            if (string.IsNullOrEmpty(_archiveFilePath))
            {
                return;
            }

            if (File.Exists(_archiveFilePath))
            {
                return;
            }

            string directory = Path.GetDirectoryName(_archiveFilePath);
            if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
            }

            string extension = Path.GetExtension(_archiveFilePath)?.ToLowerInvariant();

            if (extension == ".zip")
            {
                using (var archive = SharpCompress.Archives.Zip.ZipArchive.Create())
                {
                    using (var stream = File.Create(_archiveFilePath))
                    {
                        archive.SaveTo(stream, new WriterOptions(CompressionType.Deflate));
                    }
                }
            }
            else if (extension == ".tar")
            {
                using (var archive = SharpCompress.Archives.Tar.TarArchive.Create())
                {
                    using (var stream = File.Create(_archiveFilePath))
                    {
                        archive.SaveTo(stream, new WriterOptions(CompressionType.None));
                    }
                }
            }
        }

        /// <summary>
        /// Gets or creates an ArchiveClientWrapper for the specified drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>An <see cref="ArchiveClientWrapper"/> configured for the drive.</returns>
        public static ArchiveClientWrapper GetForDrive(Guid driveGuid)
        {
            return DriveClients.GetOrAdd(driveGuid, guid => new ArchiveClientWrapper(guid));
        }

        /// <summary>
        /// Gets the folder names at the specified path within the archive.
        /// </summary>
        /// <param name="normalizedPath">The normalized path (forward slashes, no leading/trailing separators). Empty string for root.</param>
        /// <returns>Array of folder names at the specified path.</returns>
        public string[] GetFolders(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_archiveFilePath) || !File.Exists(_archiveFilePath))
            {
                return Array.Empty<string>();
            }

            string prefix = string.IsNullOrEmpty(normalizedPath) ? "" : normalizedPath + "/";
            HashSet<string> folders = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            try
            {
                using (var archive = ArchiveFactory.Open(_archiveFilePath))
                {
                    foreach (var entry in archive.Entries)
                    {
                        if (entry.IsDirectory)
                        {
                            continue;
                        }

                        string fullName = entry.Key.Replace('\\', '/');

                        if (!string.IsNullOrEmpty(prefix) && !fullName.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
                        {
                            continue;
                        }

                        string relativePath = string.IsNullOrEmpty(prefix) ? fullName : fullName.Substring(prefix.Length);

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
            }
            catch
            {
                return Array.Empty<string>();
            }

            return folders.ToArray();
        }

        /// <summary>
        /// Gets the file names at the specified path within the archive.
        /// </summary>
        /// <param name="normalizedPath">The normalized path (forward slashes, no leading/trailing separators). Empty string for root.</param>
        /// <returns>Array of file names at the specified path.</returns>
        public string[] GetFiles(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_archiveFilePath) || !File.Exists(_archiveFilePath))
            {
                return Array.Empty<string>();
            }

            string prefix = string.IsNullOrEmpty(normalizedPath) ? "" : normalizedPath + "/";
            List<string> files = new List<string>();

            try
            {
                using (var archive = ArchiveFactory.Open(_archiveFilePath))
                {
                    foreach (var entry in archive.Entries)
                    {
                        if (entry.IsDirectory)
                        {
                            continue;
                        }

                        string fullName = entry.Key.Replace('\\', '/');

                        if (!string.IsNullOrEmpty(prefix) && !fullName.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
                        {
                            continue;
                        }

                        string relativePath = string.IsNullOrEmpty(prefix) ? fullName : fullName.Substring(prefix.Length);

                        if (!relativePath.Contains("/") && !string.IsNullOrEmpty(relativePath))
                        {
                            files.Add(SanitizeName(relativePath));
                        }
                    }
                }
            }
            catch
            {
                return Array.Empty<string>();
            }

            return files.ToArray();
        }

        /// <summary>
        /// Gets the last modified time for the specified file entry in the archive.
        /// </summary>
        /// <param name="normalizedPath">The normalized file path within the archive.</param>
        /// <returns>The last modified <see cref="DateTime"/>, or <see cref="DateTime.MinValue"/> if not found.</returns>
        public DateTime GetLastModifiedTime(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_archiveFilePath) || !File.Exists(_archiveFilePath) || string.IsNullOrEmpty(normalizedPath))
            {
                return DateTime.MinValue;
            }

            try
            {
                using (var archive = ArchiveFactory.Open(_archiveFilePath))
                {
                    var entry = FindEntry(archive, normalizedPath);
                    if (entry != null && entry.LastModifiedTime.HasValue)
                    {
                        return entry.LastModifiedTime.Value;
                    }
                }
            }
            catch
            {
                return DateTime.MinValue;
            }

            return DateTime.MinValue;
        }

        /// <summary>
        /// Gets the uncompressed file size for the specified file entry in the archive.
        /// </summary>
        /// <param name="normalizedPath">The normalized file path within the archive.</param>
        /// <returns>The uncompressed file size in bytes, or 0 if not found.</returns>
        public ulong GetFileSize(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_archiveFilePath) || !File.Exists(_archiveFilePath) || string.IsNullOrEmpty(normalizedPath))
            {
                return 0;
            }

            try
            {
                using (var archive = ArchiveFactory.Open(_archiveFilePath))
                {
                    var entry = FindEntry(archive, normalizedPath);
                    if (entry != null)
                    {
                        return (ulong)entry.Size;
                    }
                }
            }
            catch
            {
                return 0;
            }

            return 0;
        }

        /// <summary>
        /// Gets the file data for the specified file entry in the archive.
        /// </summary>
        /// <param name="normalizedPath">The normalized file path within the archive.</param>
        /// <returns>The file data as a byte array, or null if not found.</returns>
        public byte[] GetFileData(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_archiveFilePath) || !File.Exists(_archiveFilePath) || string.IsNullOrEmpty(normalizedPath))
            {
                return null;
            }

            try
            {
                using (var archive = ArchiveFactory.Open(_archiveFilePath))
                {
                    var entry = FindEntry(archive, normalizedPath);
                    if (entry == null)
                    {
                        return null;
                    }

                    using (var entryStream = entry.OpenEntryStream())
                    using (var memoryStream = new MemoryStream())
                    {
                        entryStream.CopyTo(memoryStream);
                        return memoryStream.ToArray();
                    }
                }
            }
            catch
            {
                return null;
            }
        }

        /// <summary>
        /// Finds an archive entry by its normalized path.
        /// </summary>
        /// <param name="archive">The open archive to search.</param>
        /// <param name="normalizedPath">The normalized file path within the archive (forward slashes).</param>
        /// <returns>The archive entry if found, or null.</returns>
        private static IArchiveEntry FindEntry(IArchive archive, string normalizedPath)
        {
            foreach (var entry in archive.Entries)
            {
                if (entry.IsDirectory)
                {
                    continue;
                }

                string entryPath = entry.Key.Replace('\\', '/');
                if (string.Equals(entryPath, normalizedPath, StringComparison.OrdinalIgnoreCase))
                {
                    return entry;
                }
            }

            return null;
        }

        /// <summary>
        /// Adds a file to the archive from a local file path.
        /// Only supported for writable formats (ZIP, TAR).
        /// </summary>
        /// <param name="localFilePath">The local file path to add.</param>
        /// <param name="normalizedPath">The normalized path within the archive (forward slashes).</param>
        public void AddFile(string localFilePath, string normalizedPath)
        {
            if (string.IsNullOrEmpty(_archiveFilePath) || !File.Exists(_archiveFilePath))
            {
                throw new FileNotFoundException("Archive file not found: " + _archiveFilePath);
            }

            if (!File.Exists(localFilePath))
            {
                throw new FileNotFoundException("Local file not found: " + localFilePath);
            }

            string extension = Path.GetExtension(_archiveFilePath)?.ToLowerInvariant();

            if (extension == ".zip")
            {
                AddFileToZip(localFilePath, normalizedPath);
            }
            else if (extension == ".tar")
            {
                AddFileToTar(localFilePath, normalizedPath);
            }
            else
            {
                throw new NotSupportedException($"Write operations not supported for {extension} archives. Supported: .zip, .tar");
            }
        }

        /// <summary>
        /// Adds a file to a ZIP archive.
        /// </summary>
        private void AddFileToZip(string localFilePath, string normalizedPath)
        {
            using (var archive = SharpCompress.Archives.Zip.ZipArchive.Open(_archiveFilePath))
            {
                var existingEntry = archive.Entries.FirstOrDefault(
                    e => string.Equals(e.Key.Replace('\\', '/'), normalizedPath, StringComparison.OrdinalIgnoreCase));

                if (existingEntry != null)
                {
                    archive.RemoveEntry(existingEntry);
                }

                archive.AddEntry(normalizedPath, localFilePath);
                archive.SaveTo(_archiveFilePath, new WriterOptions(CompressionType.Deflate));
            }
        }

        /// <summary>
        /// Adds a file to a TAR archive.
        /// </summary>
        private void AddFileToTar(string localFilePath, string normalizedPath)
        {
            using (var archive = SharpCompress.Archives.Tar.TarArchive.Open(_archiveFilePath))
            {
                var existingEntry = archive.Entries.FirstOrDefault(
                    e => string.Equals(e.Key.Replace('\\', '/'), normalizedPath, StringComparison.OrdinalIgnoreCase));

                if (existingEntry != null)
                {
                    archive.RemoveEntry(existingEntry);
                }

                archive.AddEntry(normalizedPath, localFilePath);
                archive.SaveTo(_archiveFilePath, new WriterOptions(CompressionType.None));
            }
        }

        /// <summary>
        /// Deletes an entry (file or directory) from the archive.
        /// Only supported for writable formats (ZIP, TAR).
        /// </summary>
        /// <param name="normalizedPath">The normalized path within the archive (forward slashes).</param>
        public void DeleteEntry(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_archiveFilePath) || !File.Exists(_archiveFilePath))
            {
                throw new FileNotFoundException("Archive file not found: " + _archiveFilePath);
            }

            string extension = Path.GetExtension(_archiveFilePath)?.ToLowerInvariant();

            if (extension == ".zip")
            {
                DeleteFromZip(normalizedPath);
            }
            else if (extension == ".tar")
            {
                DeleteFromTar(normalizedPath);
            }
            else
            {
                throw new NotSupportedException($"Delete operations not supported for {extension} archives. Supported: .zip, .tar");
            }
        }

        /// <summary>
        /// Deletes an entry from a ZIP archive.
        /// </summary>
        private void DeleteFromZip(string normalizedPath)
        {
            using (var archive = SharpCompress.Archives.Zip.ZipArchive.Open(_archiveFilePath))
            {
                var entry = archive.Entries.FirstOrDefault(
                    e => !e.IsDirectory && string.Equals(e.Key.Replace('\\', '/'), normalizedPath, StringComparison.OrdinalIgnoreCase));

                if (entry == null)
                {
                    throw new FileNotFoundException("Entry not found in archive: " + normalizedPath);
                }

                archive.RemoveEntry(entry);

                string dirPrefix = normalizedPath.TrimEnd('/') + "/";
                var childEntries = archive.Entries
                    .Where(e => e.Key.Replace('\\', '/').StartsWith(dirPrefix, StringComparison.OrdinalIgnoreCase))
                    .ToList();

                foreach (var childEntry in childEntries)
                {
                    archive.RemoveEntry(childEntry);
                }

                archive.SaveTo(_archiveFilePath, new WriterOptions(CompressionType.Deflate));
            }
        }

        /// <summary>
        /// Deletes an entry from a TAR archive.
        /// </summary>
        private void DeleteFromTar(string normalizedPath)
        {
            using (var archive = SharpCompress.Archives.Tar.TarArchive.Open(_archiveFilePath))
            {
                var entry = archive.Entries.FirstOrDefault(
                    e => !e.IsDirectory && string.Equals(e.Key.Replace('\\', '/'), normalizedPath, StringComparison.OrdinalIgnoreCase));

                if (entry == null)
                {
                    throw new FileNotFoundException("Entry not found in archive: " + normalizedPath);
                }

                archive.RemoveEntry(entry);

                string dirPrefix = normalizedPath.TrimEnd('/') + "/";
                var childEntries = archive.Entries
                    .Where(e => e.Key.Replace('\\', '/').StartsWith(dirPrefix, StringComparison.OrdinalIgnoreCase))
                    .ToList();

                foreach (var childEntry in childEntries)
                {
                    archive.RemoveEntry(childEntry);
                }

                archive.SaveTo(_archiveFilePath, new WriterOptions(CompressionType.None));
            }
        }

        /// <summary>
        /// Creates a directory entry in the archive.
        /// Only supported for writable formats (ZIP, TAR).
        /// </summary>
        /// <param name="normalizedPath">The normalized directory path (forward slashes).</param>
        public void CreateDirectory(string normalizedPath)
        {
            if (string.IsNullOrEmpty(_archiveFilePath) || !File.Exists(_archiveFilePath))
            {
                throw new FileNotFoundException("Archive file not found: " + _archiveFilePath);
            }

            string dirPath = normalizedPath.TrimEnd('/') + "/";
            string extension = Path.GetExtension(_archiveFilePath)?.ToLowerInvariant();

            if (extension == ".zip")
            {
                using (var archive = SharpCompress.Archives.Zip.ZipArchive.Open(_archiveFilePath))
                {
                    var existingEntry = archive.Entries.FirstOrDefault(
                        e => string.Equals(e.Key.Replace('\\', '/'), dirPath, StringComparison.OrdinalIgnoreCase));

                    if (existingEntry == null)
                    {
                        using (var memStream = new MemoryStream())
                        {
                            archive.AddEntry(dirPath, memStream, false, 0, null);
                        }

                        archive.SaveTo(_archiveFilePath, new WriterOptions(CompressionType.Deflate));
                    }
                }
            }
            else if (extension == ".tar")
            {
                using (var archive = SharpCompress.Archives.Tar.TarArchive.Open(_archiveFilePath))
                {
                    var existingEntry = archive.Entries.FirstOrDefault(
                        e => string.Equals(e.Key.Replace('\\', '/'), dirPath, StringComparison.OrdinalIgnoreCase));

                    if (existingEntry == null)
                    {
                        using (var memStream = new MemoryStream())
                        {
                            archive.AddEntry(dirPath, memStream, false, 0, null);
                        }

                        archive.SaveTo(_archiveFilePath, new WriterOptions(CompressionType.None));
                    }
                }
            }
            else
            {
                throw new NotSupportedException($"Write operations not supported for {extension} archives.");
            }
        }

        /// <summary>
        /// Moves/renames an entry within the archive.
        /// Only supported for writable formats (ZIP, TAR).
        /// </summary>
        /// <param name="sourceNormalizedPath">The source normalized path.</param>
        /// <param name="destNormalizedPath">The destination normalized path.</param>
        public void MoveEntry(string sourceNormalizedPath, string destNormalizedPath)
        {
            if (string.IsNullOrEmpty(_archiveFilePath) || !File.Exists(_archiveFilePath))
            {
                throw new FileNotFoundException("Archive file not found: " + _archiveFilePath);
            }

            string extension = Path.GetExtension(_archiveFilePath)?.ToLowerInvariant();

            if (extension == ".zip")
            {
                MoveEntryInZip(sourceNormalizedPath, destNormalizedPath);
            }
            else if (extension == ".tar")
            {
                MoveEntryInTar(sourceNormalizedPath, destNormalizedPath);
            }
            else
            {
                throw new NotSupportedException($"Move operations not supported for {extension} archives.");
            }
        }

        /// <summary>
        /// Moves an entry within a ZIP archive.
        /// </summary>
        private void MoveEntryInZip(string sourceNormalizedPath, string destNormalizedPath)
        {
            using (var archive = SharpCompress.Archives.Zip.ZipArchive.Open(_archiveFilePath))
            {
                var sourceEntry = archive.Entries.FirstOrDefault(
                    e => !e.IsDirectory && string.Equals(e.Key.Replace('\\', '/'), sourceNormalizedPath, StringComparison.OrdinalIgnoreCase));

                if (sourceEntry == null)
                {
                    throw new FileNotFoundException("Source entry not found: " + sourceNormalizedPath);
                }

                byte[] data;
                using (var entryStream = sourceEntry.OpenEntryStream())
                using (var memoryStream = new MemoryStream())
                {
                    entryStream.CopyTo(memoryStream);
                    data = memoryStream.ToArray();
                }

                archive.RemoveEntry(sourceEntry);

                using (var dataStream = new MemoryStream(data))
                {
                    archive.AddEntry(destNormalizedPath, dataStream, false, data.Length, sourceEntry.LastModifiedTime);
                }

                archive.SaveTo(_archiveFilePath, new WriterOptions(CompressionType.Deflate));
            }
        }

        /// <summary>
        /// Moves an entry within a TAR archive.
        /// </summary>
        private void MoveEntryInTar(string sourceNormalizedPath, string destNormalizedPath)
        {
            using (var archive = SharpCompress.Archives.Tar.TarArchive.Open(_archiveFilePath))
            {
                var sourceEntry = archive.Entries.FirstOrDefault(
                    e => !e.IsDirectory && string.Equals(e.Key.Replace('\\', '/'), sourceNormalizedPath, StringComparison.OrdinalIgnoreCase));

                if (sourceEntry == null)
                {
                    throw new FileNotFoundException("Source entry not found: " + sourceNormalizedPath);
                }

                byte[] data;
                using (var entryStream = sourceEntry.OpenEntryStream())
                using (var memoryStream = new MemoryStream())
                {
                    entryStream.CopyTo(memoryStream);
                    data = memoryStream.ToArray();
                }

                archive.RemoveEntry(sourceEntry);

                using (var dataStream = new MemoryStream(data))
                {
                    archive.AddEntry(destNormalizedPath, dataStream, false, data.Length, sourceEntry.LastModifiedTime);
                }

                archive.SaveTo(_archiveFilePath, new WriterOptions(CompressionType.None));
            }
        }

        /// <summary>
        /// Determines if the archive format supports write operations.
        /// </summary>
        /// <returns>True if writable (ZIP, TAR); false otherwise (7z, RAR, etc.).</returns>
        public bool IsWritable()
        {
            string extension = Path.GetExtension(_archiveFilePath)?.ToLowerInvariant();
            return extension == ".zip" || extension == ".tar";
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
