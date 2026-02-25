// <copyright file="BigDriveFileStore.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.FileStores
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    using BigDrive.ConfigProvider.Model;
    using BigDrive.Interfaces;

    /// <summary>
    /// IFileStore implementation backed by a BigDrive provider via COM+ interfaces.
    /// Uses out-of-process COM activation (CLSCTX_LOCAL_SERVER).
    /// </summary>
    public class BigDriveFileStore : IFileStore
    {
        /// <summary>
        /// The drive configuration.
        /// </summary>
        private readonly DriveConfiguration m_config;

        /// <summary>
        /// Initializes a new instance of the <see cref="BigDriveFileStore"/> class.
        /// </summary>
        /// <param name="config">The BigDrive drive configuration.</param>
        public BigDriveFileStore(DriveConfiguration config)
        {
            m_config = config;
        }

        /// <summary>
        /// Gets the display name for this store.
        /// </summary>
        public string DisplayName
        {
            get { return m_config.Name + " (BigDrive)"; }
        }

        /// <summary>
        /// Gets the drive GUID.
        /// </summary>
        public Guid DriveGuid
        {
            get { return m_config.Id; }
        }

        /// <summary>
        /// Gets a value indicating whether this store supports file operations.
        /// </summary>
        public bool SupportsFileOperations
        {
            get { return ProviderFactory.GetFileOperationsProvider(m_config.Id) != null; }
        }

        /// <summary>
        /// Gets a value indicating whether this store supports file enumeration.
        /// </summary>
        public bool SupportsEnumeration
        {
            get { return ProviderFactory.GetEnumerateProvider(m_config.Id) != null; }
        }

        /// <summary>
        /// Checks if a file exists at the specified path.
        /// </summary>
        /// <param name="path">The BigDrive path (e.g., "\folder\file.txt").</param>
        /// <returns>True if the file exists.</returns>
        public bool FileExists(string path)
        {
            IBigDriveEnumerate enumerate = ProviderFactory.GetEnumerateProvider(m_config.Id);
            if (enumerate == null)
            {
                return false;
            }

            string directory = GetDirectoryPart(path);
            string fileName = GetFileNamePart(path);

            string[] files = enumerate.EnumerateFiles(m_config.Id, directory);
            foreach (string file in files)
            {
                if (string.Equals(file, fileName, StringComparison.OrdinalIgnoreCase))
                {
                    return true;
                }
            }

            return false;
        }

        /// <summary>
        /// Enumerates files in the specified directory.
        /// </summary>
        /// <param name="directoryPath">The directory path in the BigDrive.</param>
        /// <returns>Array of file names.</returns>
        public string[] EnumerateFiles(string directoryPath)
        {
            IBigDriveEnumerate enumerate = ProviderFactory.GetEnumerateProvider(m_config.Id);
            if (enumerate == null)
            {
                return new string[0];
            }

            ShellTrace.ComCall("IBigDriveEnumerate", "EnumerateFiles",
                string.Format("driveGuid={0}, path=\"{1}\"", m_config.Id, directoryPath));
            string[] files = enumerate.EnumerateFiles(m_config.Id, directoryPath);
            ShellTrace.ComResult("IBigDriveEnumerate", "EnumerateFiles", 0,
                string.Format("{0} files found", files.Length));
            return files;
        }

        /// <summary>
        /// Enumerates subdirectories in the specified directory.
        /// </summary>
        /// <param name="directoryPath">The directory path in the BigDrive.</param>
        /// <returns>Array of subdirectory names.</returns>
        public string[] EnumerateFolders(string directoryPath)
        {
            IBigDriveEnumerate enumerate = ProviderFactory.GetEnumerateProvider(m_config.Id);
            if (enumerate == null)
            {
                return new string[0];
            }

            ShellTrace.ComCall("IBigDriveEnumerate", "EnumerateFolders",
                string.Format("driveGuid={0}, path=\"{1}\"", m_config.Id, directoryPath));
            string[] folders = enumerate.EnumerateFolders(m_config.Id, directoryPath);
            ShellTrace.ComResult("IBigDriveEnumerate", "EnumerateFolders", 0,
                string.Format("{0} folders found", folders.Length));
            return folders;
        }

        /// <summary>
        /// Copies a BigDrive file to a local path.
        /// Tries IBigDriveFileOperations first, falls back to IBigDriveFileData streaming.
        /// </summary>
        /// <param name="sourcePath">The source path within the BigDrive.</param>
        /// <param name="localTempPath">The local file path to write to.</param>
        public void CopyToLocal(string sourcePath, string localTempPath)
        {
            // Try IBigDriveFileOperations first
            IBigDriveFileOperations fileOps = ProviderFactory.GetFileOperationsProvider(m_config.Id);
            if (fileOps != null)
            {
                ShellTrace.ComCall("IBigDriveFileOperations", "CopyFileFromBigDrive",
                    string.Format("driveGuid={0}, sourcePath=\"{1}\", localPath=\"{2}\"", m_config.Id, sourcePath, localTempPath));
                fileOps.CopyFileFromBigDrive(m_config.Id, sourcePath, localTempPath);
                ShellTrace.ComResult("IBigDriveFileOperations", "CopyFileFromBigDrive", 0);
                return;
            }

            // Fallback to IBigDriveFileData
            IBigDriveFileData fileData = ProviderFactory.GetFileDataProvider(m_config.Id);
            if (fileData != null)
            {
                ShellTrace.ComCall("IBigDriveFileData", "GetFileData",
                    string.Format("driveGuid={0}, path=\"{1}\"", m_config.Id, sourcePath));
                int hr = fileData.GetFileData(m_config.Id, sourcePath, out IStream stream);
                ShellTrace.ComResult("IBigDriveFileData", "GetFileData", hr,
                    stream != null ? "stream returned" : "stream is null");

                if (hr != 0)
                {
                    throw new IOException("Failed to get file data. HRESULT: 0x" + hr.ToString("X8"));
                }

                string destDir = Path.GetDirectoryName(localTempPath);
                if (!string.IsNullOrEmpty(destDir) && !Directory.Exists(destDir))
                {
                    Directory.CreateDirectory(destDir);
                }

                WriteStreamToFile(stream, localTempPath);
                return;
            }

            throw new InvalidOperationException("Provider does not support file operations or file data.");
        }

        /// <summary>
        /// Copies a local file into the BigDrive.
        /// </summary>
        /// <param name="localSourcePath">The local file path to read from.</param>
        /// <param name="destinationPath">The destination path within the BigDrive.</param>
        public void CopyFromLocal(string localSourcePath, string destinationPath)
        {
            IBigDriveFileOperations fileOps = ProviderFactory.GetFileOperationsProvider(m_config.Id);
            if (fileOps == null)
            {
                throw new InvalidOperationException("Provider does not support file operations.");
            }

            ShellTrace.ComCall("IBigDriveFileOperations", "CopyFileToBigDrive",
                string.Format("driveGuid={0}, localPath=\"{1}\", destPath=\"{2}\"", m_config.Id, localSourcePath, destinationPath));
            fileOps.CopyFileToBigDrive(m_config.Id, localSourcePath, destinationPath);
            ShellTrace.ComResult("IBigDriveFileOperations", "CopyFileToBigDrive", 0);
        }

        /// <summary>
        /// Deletes a file from the BigDrive.
        /// </summary>
        /// <param name="path">The path of the file to delete.</param>
        public void DeleteFile(string path)
        {
            IBigDriveFileOperations fileOps = ProviderFactory.GetFileOperationsProvider(m_config.Id);
            if (fileOps == null)
            {
                throw new InvalidOperationException("Provider does not support file operations.");
            }

            ShellTrace.ComCall("IBigDriveFileOperations", "DeleteFile",
                string.Format("driveGuid={0}, path=\"{1}\"", m_config.Id, path));
            fileOps.DeleteFile(m_config.Id, path);
            ShellTrace.ComResult("IBigDriveFileOperations", "DeleteFile", 0);
        }

        /// <summary>
        /// Moves a file within the BigDrive.
        /// </summary>
        /// <param name="sourcePath">The source path.</param>
        /// <param name="destinationPath">The destination path.</param>
        public void MoveFile(string sourcePath, string destinationPath)
        {
            IBigDriveFileOperations fileOps = ProviderFactory.GetFileOperationsProvider(m_config.Id);
            if (fileOps == null)
            {
                throw new InvalidOperationException("Provider does not support file operations.");
            }

            ShellTrace.ComCall("IBigDriveFileOperations", "MoveFile",
                string.Format("driveGuid={0}, source=\"{1}\", dest=\"{2}\"", m_config.Id, sourcePath, destinationPath));
            fileOps.MoveFile(m_config.Id, sourcePath, destinationPath);
            ShellTrace.ComResult("IBigDriveFileOperations", "MoveFile", 0);
        }

        /// <summary>
        /// Creates a directory in the BigDrive.
        /// </summary>
        /// <param name="directoryPath">The directory path to create.</param>
        public void CreateDirectory(string directoryPath)
        {
            IBigDriveFileOperations fileOps = ProviderFactory.GetFileOperationsProvider(m_config.Id);
            if (fileOps == null)
            {
                throw new InvalidOperationException("Provider does not support file operations.");
            }

            ShellTrace.ComCall("IBigDriveFileOperations", "CreateDirectory",
                string.Format("driveGuid={0}, path=\"{1}\"", m_config.Id, directoryPath));
            fileOps.CreateDirectory(m_config.Id, directoryPath);
            ShellTrace.ComResult("IBigDriveFileOperations", "CreateDirectory", 0);
        }

        /// <summary>
        /// Gets the directory portion of a path.
        /// </summary>
        /// <param name="path">The full path.</param>
        /// <returns>The directory portion.</returns>
        private static string GetDirectoryPart(string path)
        {
            int lastSep = path.LastIndexOf('\\');
            if (lastSep <= 0)
            {
                return "\\";
            }

            return path.Substring(0, lastSep);
        }

        /// <summary>
        /// Gets the file name portion of a path.
        /// </summary>
        /// <param name="path">The full path.</param>
        /// <returns>The file name.</returns>
        private static string GetFileNamePart(string path)
        {
            int lastSep = path.LastIndexOf('\\');
            if (lastSep < 0)
            {
                return path;
            }

            return path.Substring(lastSep + 1);
        }

        /// <summary>
        /// Writes a COM IStream to a local file.
        /// </summary>
        /// <param name="stream">The source IStream.</param>
        /// <param name="filePath">The local file path to write to.</param>
        private static void WriteStreamToFile(IStream stream, string filePath)
        {
            using (FileStream fileStream = new FileStream(filePath, FileMode.Create, FileAccess.Write))
            {
                byte[] buffer = new byte[8192];
                IntPtr bytesReadPtr = Marshal.AllocCoTaskMem(sizeof(int));

                try
                {
                    while (true)
                    {
                        stream.Read(buffer, buffer.Length, bytesReadPtr);
                        int bytesRead = Marshal.ReadInt32(bytesReadPtr);
                        if (bytesRead == 0)
                        {
                            break;
                        }

                        fileStream.Write(buffer, 0, bytesRead);
                    }
                }
                finally
                {
                    Marshal.FreeCoTaskMem(bytesReadPtr);
                }
            }
        }
    }
}
