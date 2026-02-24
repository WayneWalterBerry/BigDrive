// <copyright file="CopyCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    using BigDrive.ConfigProvider.Model;
    using BigDrive.Interfaces;

    /// <summary>
    /// Copies files between BigDrive drives and local drives.
    /// Supports: Local to BigDrive, BigDrive to Local, BigDrive to BigDrive.
    /// </summary>
    public class CopyCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "copy"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "cp" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Copies files between drives"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "copy <source> <destination>  |  copy X:\\file Y:\\file"; }
        }

        /// <summary>
        /// Executes the copy command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("Usage: " + Usage);
                return;
            }

            string source = args[0];
            string destination = args[1];

            PathInfo sourcePath = ParsePath(source, context);
            PathInfo destPath = ParsePath(destination, context);

            // Validate source
            if (sourcePath.DriveLetter == '\0' && !sourcePath.IsOSDrive)
            {
                Console.WriteLine("No drive selected. Use 'cd X:' to select a drive first.");
                return;
            }

            // Route to appropriate copy method
            if (sourcePath.IsOSDrive && destPath.IsBigDrive)
            {
                CopyLocalToBigDrive(context, sourcePath, destPath);
            }
            else if (sourcePath.IsBigDrive && destPath.IsOSDrive)
            {
                CopyBigDriveToLocal(context, sourcePath, destPath);
            }
            else if (sourcePath.IsBigDrive && destPath.IsBigDrive)
            {
                CopyBigDriveToBigDrive(context, sourcePath, destPath);
            }
            else if (sourcePath.IsOSDrive && destPath.IsOSDrive)
            {
                Console.WriteLine("Use Windows copy for local-to-local operations.");
            }
            else
            {
                Console.WriteLine("Invalid source or destination path.");
            }
        }

        /// <summary>
        /// Parses a path string into source/destination info.
        /// </summary>
        private static PathInfo ParsePath(string path, ShellContext context)
        {
            PathInfo info = new PathInfo();

            // Check for drive letter
            if (path.Length >= 2 && path[1] == ':')
            {
                char letter = char.ToUpper(path[0]);
                info.DriveLetter = letter;
                info.Path = path.Length > 2 ? path.Substring(2) : "\\";
                info.IsBigDrive = context.DriveLetterManager.IsBigDrive(letter);
                info.IsOSDrive = context.DriveLetterManager.IsOSDrive(letter);
                info.IsRelative = false;
            }
            else
            {
                // Relative path - use current drive
                info.DriveLetter = context.CurrentDriveLetter;
                info.Path = path;
                info.IsBigDrive = context.CurrentDriveLetter != '\0' && context.DriveLetterManager.IsBigDrive(context.CurrentDriveLetter);
                info.IsOSDrive = context.CurrentDriveLetter != '\0' && context.DriveLetterManager.IsOSDrive(context.CurrentDriveLetter);
                info.IsRelative = true;
            }

            // Resolve relative paths
            if (info.IsRelative && info.IsBigDrive)
            {
                info.Path = ResolvePath(context.GetPathForDrive(info.DriveLetter), info.Path);
            }

            return info;
        }

        /// <summary>
        /// Copies a local file to a BigDrive.
        /// </summary>
        private static void CopyLocalToBigDrive(ShellContext context, PathInfo source, PathInfo dest)
        {
            string localPath = source.GetFullPath();

            if (!File.Exists(localPath))
            {
                Console.WriteLine("Source file not found: " + localPath);
                return;
            }

            DriveConfiguration destConfig = context.DriveLetterManager.GetDriveConfiguration(dest.DriveLetter);
            if (destConfig == null)
            {
                Console.WriteLine("Destination drive not found: " + dest.DriveLetter + ":");
                return;
            }

            IBigDriveFileOperations fileOps = ProviderFactory.GetFileOperationsProvider(destConfig.Id);
            if (fileOps == null)
            {
                Console.WriteLine("Destination provider does not support file operations.");
                return;
            }

            fileOps.CopyFileToBigDrive(destConfig.Id, localPath, dest.Path);
            Console.WriteLine("        1 file(s) copied.");
        }

        /// <summary>
        /// Copies a BigDrive file to local storage.
        /// </summary>
        private static void CopyBigDriveToLocal(ShellContext context, PathInfo source, PathInfo dest)
        {
            DriveConfiguration sourceConfig = context.DriveLetterManager.GetDriveConfiguration(source.DriveLetter);
            if (sourceConfig == null)
            {
                Console.WriteLine("Source drive not found: " + source.DriveLetter + ":");
                return;
            }

            string localPath = dest.GetFullPath();

            // Try IBigDriveFileOperations first
            IBigDriveFileOperations fileOps = ProviderFactory.GetFileOperationsProvider(sourceConfig.Id);
            if (fileOps != null)
            {
                fileOps.CopyFileFromBigDrive(sourceConfig.Id, source.Path, localPath);
                Console.WriteLine("        1 file(s) copied.");
                return;
            }

            // Fallback to IBigDriveFileData
            IBigDriveFileData fileData = ProviderFactory.GetFileDataProvider(sourceConfig.Id);
            if (fileData != null)
            {
                int hr = fileData.GetFileData(sourceConfig.Id, source.Path, out IStream stream);
                if (hr != 0)
                {
                    Console.WriteLine("Failed to get file data. HRESULT: 0x" + hr.ToString("X8"));
                    return;
                }

                WriteStreamToFile(stream, localPath);
                Console.WriteLine("        1 file(s) copied.");
                return;
            }

            Console.WriteLine("Source provider does not support file operations or file data.");
        }

        /// <summary>
        /// Copies a file from one BigDrive to another.
        /// </summary>
        private static void CopyBigDriveToBigDrive(ShellContext context, PathInfo source, PathInfo dest)
        {
            DriveConfiguration sourceConfig = context.DriveLetterManager.GetDriveConfiguration(source.DriveLetter);
            DriveConfiguration destConfig = context.DriveLetterManager.GetDriveConfiguration(dest.DriveLetter);

            if (sourceConfig == null)
            {
                Console.WriteLine("Source drive not found: " + source.DriveLetter + ":");
                return;
            }

            if (destConfig == null)
            {
                Console.WriteLine("Destination drive not found: " + dest.DriveLetter + ":");
                return;
            }

            // Get file data from source
            IBigDriveFileData sourceFileData = ProviderFactory.GetFileDataProvider(sourceConfig.Id);
            if (sourceFileData == null)
            {
                Console.WriteLine("Source provider does not support file data streaming.");
                return;
            }

            int hr = sourceFileData.GetFileData(sourceConfig.Id, source.Path, out IStream sourceStream);
            if (hr != 0)
            {
                Console.WriteLine("Failed to get source file data. HRESULT: 0x" + hr.ToString("X8"));
                return;
            }

            // Get destination file operations
            IBigDriveFileOperations destFileOps = ProviderFactory.GetFileOperationsProvider(destConfig.Id);
            if (destFileOps == null)
            {
                // Use a temp file as intermediary
                string tempFile = Path.GetTempFileName();

                try
                {
                    WriteStreamToFile(sourceStream, tempFile);
                    destFileOps = ProviderFactory.GetFileOperationsProvider(destConfig.Id);

                    if (destFileOps != null)
                    {
                        destFileOps.CopyFileToBigDrive(destConfig.Id, tempFile, dest.Path);
                        Console.WriteLine("        1 file(s) copied.");
                    }
                    else
                    {
                        Console.WriteLine("Destination provider does not support file operations.");
                    }
                }
                finally
                {
                    if (File.Exists(tempFile))
                    {
                        File.Delete(tempFile);
                    }
                }

                return;
            }

            // Use temp file for cross-provider copy
            string tempPath = Path.GetTempFileName();

            try
            {
                WriteStreamToFile(sourceStream, tempPath);
                destFileOps.CopyFileToBigDrive(destConfig.Id, tempPath, dest.Path);
                Console.WriteLine("        1 file(s) copied.");
            }
            finally
            {
                if (File.Exists(tempPath))
                {
                    File.Delete(tempPath);
                }
            }
        }

        /// <summary>
        /// Writes an IStream to a local file.
        /// </summary>
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

        /// <summary>
        /// Resolves a relative path against a current path.
        /// </summary>
        private static string ResolvePath(string currentPath, string targetPath)
        {
            if (targetPath.StartsWith("\\") || targetPath.StartsWith("/"))
            {
                return targetPath;
            }

            if (currentPath == "\\" || currentPath == "/")
            {
                return "\\" + targetPath;
            }

            return currentPath.TrimEnd('\\', '/') + "\\" + targetPath;
        }
    }
}
