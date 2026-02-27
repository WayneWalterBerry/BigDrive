// <copyright file="VirtualDiskClientWrapper.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.VirtualDisk
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Threading;

    using BigDrive.ConfigProvider;

    using DiscUtils;
    using DiscUtils.Partitions;
    using DiscUtils.Streams;

    /// <summary>
    /// Wrapper for DiscUtils virtual disk access.
    /// Supports VHD, VHDX, VMDK, and VDI formats with NTFS, FAT32, and ext2/3/4 file systems.
    /// </summary>
    internal class VirtualDiskClientWrapper : IDisposable
    {
        private static readonly Dictionary<Guid, VirtualDiskClientWrapper> ClientCache = new Dictionary<Guid, VirtualDiskClientWrapper>();

        private readonly Guid m_driveGuid;

        private readonly string m_diskFilePath;

        private readonly int m_partitionIndex;

        private readonly bool m_readOnly;

        private VirtualDisk m_disk;

        private DiscFileSystem m_fileSystem;

        /// <summary>
        /// Initializes a new instance of the <see cref="VirtualDiskClientWrapper"/> class.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        private VirtualDiskClientWrapper(Guid driveGuid)
        {
            m_driveGuid = driveGuid;

            m_diskFilePath = DriveManager.ReadDriveProperty(driveGuid, "VhdFilePath", CancellationToken.None);
            if (string.IsNullOrEmpty(m_diskFilePath))
            {
                throw new InvalidOperationException("VhdFilePath property is required. Use 'bigdrive set VhdFilePath \"C:\\path\\to\\disk.vhdx\"'");
            }

            if (!File.Exists(m_diskFilePath))
            {
                throw new FileNotFoundException($"Virtual disk file not found: {m_diskFilePath}");
            }

            string partitionIndexStr = DriveManager.ReadDriveProperty(driveGuid, "PartitionIndex", CancellationToken.None);
            m_partitionIndex = string.IsNullOrEmpty(partitionIndexStr) ? 0 : int.Parse(partitionIndexStr);

            string readOnlyStr = DriveManager.ReadDriveProperty(driveGuid, "ReadOnly", CancellationToken.None);
            m_readOnly = !string.IsNullOrEmpty(readOnlyStr) && bool.Parse(readOnlyStr);

            Initialize();
        }

        /// <summary>
        /// Gets the client wrapper for a specific drive, creating it if necessary.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The client wrapper instance.</returns>
        public static VirtualDiskClientWrapper GetForDrive(Guid driveGuid)
        {
            lock (ClientCache)
            {
                if (!ClientCache.TryGetValue(driveGuid, out VirtualDiskClientWrapper client))
                {
                    client = new VirtualDiskClientWrapper(driveGuid);
                    ClientCache[driveGuid] = client;
                }

                return client;
            }
        }

        /// <summary>
        /// Disposes all cached client wrappers.
        /// </summary>
        public static void DisposeAll()
        {
            lock (ClientCache)
            {
                foreach (VirtualDiskClientWrapper client in ClientCache.Values)
                {
                    client.Dispose();
                }

                ClientCache.Clear();
            }
        }

        /// <summary>
        /// Gets the folder names at the specified path.
        /// </summary>
        /// <param name="path">The path to enumerate.</param>
        /// <returns>Array of folder names.</returns>
        public string[] GetFolders(string path)
        {
            if (!m_fileSystem.DirectoryExists(path))
            {
                return Array.Empty<string>();
            }

            return m_fileSystem.GetDirectories(path)
                .Select(p => Path.GetFileName(p.TrimEnd('\\')))
                .Where(n => !string.IsNullOrEmpty(n))
                .ToArray();
        }

        /// <summary>
        /// Gets the file names at the specified path.
        /// </summary>
        /// <param name="path">The path to enumerate.</param>
        /// <returns>Array of file names.</returns>
        public string[] GetFiles(string path)
        {
            if (!m_fileSystem.DirectoryExists(path))
            {
                return Array.Empty<string>();
            }

            return m_fileSystem.GetFiles(path)
                .Select(Path.GetFileName)
                .Where(n => !string.IsNullOrEmpty(n))
                .ToArray();
        }

        /// <summary>
        /// Opens a file for reading.
        /// </summary>
        /// <param name="path">The file path.</param>
        /// <returns>A stream for reading the file, or null if not found.</returns>
        public Stream OpenFile(string path)
        {
            if (!m_fileSystem.FileExists(path))
            {
                return null;
            }

            return m_fileSystem.OpenFile(path, FileMode.Open, FileAccess.Read);
        }

        /// <summary>
        /// Gets the size of a file in bytes.
        /// </summary>
        /// <param name="path">The file path.</param>
        /// <returns>File size in bytes, or 0 if not found.</returns>
        public ulong GetFileSize(string path)
        {
            if (!m_fileSystem.FileExists(path))
            {
                return 0;
            }

            return (ulong)m_fileSystem.GetFileInfo(path).Length;
        }

        /// <summary>
        /// Gets the last modified time of a file.
        /// </summary>
        /// <param name="path">The file path.</param>
        /// <returns>Last modified time, or DateTime.MinValue if not found.</returns>
        public DateTime GetLastModifiedTime(string path)
        {
            if (!m_fileSystem.FileExists(path))
            {
                return DateTime.MinValue;
            }

            return m_fileSystem.GetFileInfo(path).LastWriteTimeUtc;
        }

        /// <summary>
        /// Writes a file to the virtual disk.
        /// </summary>
        /// <param name="path">The target path.</param>
        /// <param name="sourceStream">The source stream to copy from.</param>
        public void WriteFile(string path, Stream sourceStream)
        {
            if (m_readOnly)
            {
                throw new InvalidOperationException("Virtual disk is mounted read-only. Set ReadOnly=false to enable write operations.");
            }

            string directory = Path.GetDirectoryName(path);
            if (!string.IsNullOrEmpty(directory) && !m_fileSystem.DirectoryExists(directory))
            {
                m_fileSystem.CreateDirectory(directory);
            }

            using (Stream targetStream = m_fileSystem.OpenFile(path, FileMode.Create, FileAccess.Write))
            {
                sourceStream.CopyTo(targetStream);
            }
        }

        /// <summary>
        /// Deletes a file from the virtual disk.
        /// </summary>
        /// <param name="path">The file path.</param>
        public void DeleteFile(string path)
        {
            if (m_readOnly)
            {
                throw new InvalidOperationException("Virtual disk is mounted read-only. Set ReadOnly=false to enable write operations.");
            }

            if (!m_fileSystem.FileExists(path))
            {
                throw new FileNotFoundException($"File not found: {path}");
            }

            m_fileSystem.DeleteFile(path);
        }

        /// <summary>
        /// Creates a directory in the virtual disk.
        /// </summary>
        /// <param name="path">The directory path.</param>
        public void CreateDirectory(string path)
        {
            if (m_readOnly)
            {
                throw new InvalidOperationException("Virtual disk is mounted read-only. Set ReadOnly=false to enable write operations.");
            }

            if (m_fileSystem.DirectoryExists(path))
            {
                return;
            }

            m_fileSystem.CreateDirectory(path);
        }

        /// <inheritdoc/>
        public void Dispose()
        {
            if (m_fileSystem != null)
            {
                m_fileSystem.Dispose();
                m_fileSystem = null;
            }

            if (m_disk != null)
            {
                m_disk.Dispose();
                m_disk = null;
            }
        }

        /// <summary>
        /// Initializes the virtual disk and file system.
        /// </summary>
        private void Initialize()
        {
            FileAccess access = m_readOnly ? FileAccess.Read : FileAccess.ReadWrite;

            m_disk = OpenVirtualDisk(m_diskFilePath, access);

            if (m_disk == null)
            {
                throw new InvalidOperationException($"Unable to open virtual disk: {m_diskFilePath}. Unsupported format or corrupted file.");
            }

            m_fileSystem = OpenFileSystem(m_disk, m_partitionIndex);

            if (m_fileSystem == null)
            {
                throw new InvalidOperationException($"Unable to open file system on partition {m_partitionIndex}. Disk may be unformatted or partition may not exist.");
            }
        }

        /// <summary>
        /// Opens a virtual disk file, auto-detecting the format.
        /// </summary>
        /// <param name="diskPath">The path to the disk file.</param>
        /// <param name="access">The file access mode.</param>
        /// <returns>A VirtualDisk instance, or null if format is unsupported.</returns>
        private VirtualDisk OpenVirtualDisk(string diskPath, FileAccess access)
        {
            string ext = Path.GetExtension(diskPath).ToLowerInvariant();

            try
            {
                switch (ext)
                {
                    case ".vhd":
                        return new DiscUtils.Vhd.Disk(diskPath, access);

                    case ".vhdx":
                        return new DiscUtils.Vhdx.Disk(diskPath, access);

                    case ".vmdk":
                        return new DiscUtils.Vmdk.Disk(diskPath, access);

                    case ".vdi":
                        return new DiscUtils.Vdi.Disk(diskPath, access);

                    default:
                        throw new NotSupportedException($"Unsupported virtual disk format: {ext}. Supported formats: VHD, VHDX, VMDK, VDI");
                }
            }
            catch (Exception ex)
            {
                throw new InvalidOperationException($"Failed to open virtual disk '{diskPath}': {ex.Message}", ex);
            }
        }

        /// <summary>
        /// Opens the file system on the specified partition.
        /// </summary>
        /// <param name="disk">The virtual disk.</param>
        /// <param name="partitionIndex">The partition index.</param>
        /// <returns>A DiscFileSystem instance, or null if partition or file system not found.</returns>
        private DiscFileSystem OpenFileSystem(VirtualDisk disk, int partitionIndex)
        {
            PartitionTable partitionTable = disk.Partitions;
            if (partitionTable == null || partitionTable.Count == 0)
            {
                throw new InvalidOperationException("No partitions found on disk. Disk may be unformatted or use a raw file system.");
            }

            if (partitionIndex < 0 || partitionIndex >= partitionTable.Count)
            {
                throw new ArgumentOutOfRangeException(
                    nameof(partitionIndex),
                    $"Partition index {partitionIndex} is out of range. Disk has {partitionTable.Count} partition(s).");
            }

            PartitionInfo partition = partitionTable[partitionIndex];

            using (SparseStream partitionStream = partition.Open())
            {
                DiscUtils.FileSystemInfo fsInfo = FileSystemManager.DetectFileSystems(partitionStream).FirstOrDefault();

                if (fsInfo == null)
                {
                    throw new InvalidOperationException($"No supported file system detected on partition {partitionIndex}.");
                }

                DiscFileSystem fileSystem = fsInfo.Open(partitionStream);
                return fileSystem;
            }
        }
    }
}
