// <copyright file="CopyCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    using BigDrive.ConfigProvider.Model;
    using BigDrive.Interfaces;

    /// <summary>
    /// Copies files between BigDrive drives and local drives.
    /// Supports: Local to BigDrive, BigDrive to Local, BigDrive to BigDrive.
    /// Supports wildcard patterns (* and ?) for source files.
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
            get { return "Copies files between drives (supports wildcards: *, ?)"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "copy <source> <destination>  |  copy *.txt c:\\temp\\  |  copy X:\\*.jpg Y:\\backup\\"; }
        }

        /// <summary>
        /// Executes the copy command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            ShellTrace.Enter("CopyCommand", "Execute", string.Format("args.Length={0}", args.Length));

            if (args.Length < 2)
            {
                Console.WriteLine("Usage: " + Usage);
                ShellTrace.Exit("CopyCommand", "Execute", "insufficient args");
                return;
            }

            string source = args[0];
            string destination = args[1];

            ShellTrace.Verbose("Source argument: \"{0}\"", source);
            ShellTrace.Verbose("Destination argument: \"{0}\"", destination);

            PathInfo sourcePath = ParsePath(source, context);
            PathInfo destPath = ParsePath(destination, context);

            ShellTrace.PathResolution(source, sourcePath.GetFullPath(), 
                sourcePath.IsBigDrive ? "BigDrive" : (sourcePath.IsOSDrive ? "OS" : "Unknown"));
            ShellTrace.PathResolution(destination, destPath.GetFullPath(),
                destPath.IsBigDrive ? "BigDrive" : (destPath.IsOSDrive ? "OS" : "Unknown"));

            ShellTrace.Verbose("Source: DriveLetter={0}, Path=\"{1}\", IsBigDrive={2}, IsOSDrive={3}, IsRelative={4}",
                sourcePath.DriveLetter, sourcePath.Path, sourcePath.IsBigDrive, sourcePath.IsOSDrive, sourcePath.IsRelative);
            ShellTrace.Verbose("Dest: DriveLetter={0}, Path=\"{1}\", IsBigDrive={2}, IsOSDrive={3}, IsRelative={4}",
                destPath.DriveLetter, destPath.Path, destPath.IsBigDrive, destPath.IsOSDrive, destPath.IsRelative);

            // Validate source
            if (sourcePath.DriveLetter == '\0' && !sourcePath.IsOSDrive)
            {
                Console.WriteLine("No drive selected. Use 'cd X:' to select a drive first.");
                ShellTrace.Exit("CopyCommand", "Execute", "no drive selected");
                return;
            }

            // Route to appropriate copy method
            if (sourcePath.IsOSDrive && destPath.IsBigDrive)
            {
                ShellTrace.Info("Copy operation: Local => BigDrive");
                CopyLocalToBigDrive(context, sourcePath, destPath);
            }
            else if (sourcePath.IsBigDrive && destPath.IsOSDrive)
            {
                ShellTrace.Info("Copy operation: BigDrive => Local");
                CopyBigDriveToLocal(context, sourcePath, destPath);
            }
            else if (sourcePath.IsBigDrive && destPath.IsBigDrive)
            {
                ShellTrace.Info("Copy operation: BigDrive => BigDrive");
                CopyBigDriveToBigDrive(context, sourcePath, destPath);
            }
            else if (sourcePath.IsOSDrive && destPath.IsOSDrive)
            {
                Console.WriteLine("Use Windows copy for local-to-local operations.");
                ShellTrace.Exit("CopyCommand", "Execute", "local-to-local not supported");
            }
            else
            {
                Console.WriteLine("Invalid source or destination path.");
                ShellTrace.Error("Invalid paths: source.IsOSDrive={0}, source.IsBigDrive={1}, dest.IsOSDrive={2}, dest.IsBigDrive={3}",
                    sourcePath.IsOSDrive, sourcePath.IsBigDrive, destPath.IsOSDrive, destPath.IsBigDrive);
            }

            ShellTrace.Exit("CopyCommand", "Execute");
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
        /// Copies local file(s) to a BigDrive. Supports wildcard patterns.
        /// </summary>
        private static void CopyLocalToBigDrive(ShellContext context, PathInfo source, PathInfo dest)
        {
            ShellTrace.Enter("CopyCommand", "CopyLocalToBigDrive",
                string.Format("source={0}, dest={1}", source.GetFullPath(), dest.GetFullPath()));

            string localPath = source.GetFullPath();

            DriveConfiguration destConfig = context.DriveLetterManager.GetDriveConfiguration(dest.DriveLetter);
            if (destConfig == null)
            {
                Console.WriteLine("Destination drive not found: " + dest.DriveLetter + ":");
                ShellTrace.Exit("CopyCommand", "CopyLocalToBigDrive", "dest drive not found");
                return;
            }

            IBigDriveFileOperations fileOps = ProviderFactory.GetFileOperationsProvider(destConfig.Id);
            if (fileOps == null)
            {
                Console.WriteLine("Destination provider does not support file operations.");
                ShellTrace.Exit("CopyCommand", "CopyLocalToBigDrive", "no file operations");
                return;
            }

            // Check for wildcard in local path
            if (WildcardMatcher.ContainsWildcard(localPath))
            {
                ShellTrace.Info("Wildcard detected in local source path, expanding...");
                CopyLocalToBigDriveWithWildcard(destConfig, fileOps, localPath, dest.Path);
                return;
            }

            // Single file copy
            if (!File.Exists(localPath))
            {
                Console.WriteLine("Source file not found: " + localPath);
                return;
            }

            ShellTrace.ComCall("IBigDriveFileOperations", "CopyFileToBigDrive",
                string.Format("driveGuid={0}, localPath=\"{1}\", destPath=\"{2}\"", destConfig.Id, localPath, dest.Path));
            fileOps.CopyFileToBigDrive(destConfig.Id, localPath, dest.Path);
            ShellTrace.ComResult("IBigDriveFileOperations", "CopyFileToBigDrive", 0);
            Console.WriteLine("        1 file(s) copied.");
            ShellTrace.Exit("CopyCommand", "CopyLocalToBigDrive", "complete");
        }

        /// <summary>
        /// Copies multiple local files matching a wildcard pattern to BigDrive.
        /// </summary>
        private static void CopyLocalToBigDriveWithWildcard(
            DriveConfiguration destConfig,
            IBigDriveFileOperations fileOps,
            string localPathWithWildcard,
            string bigDriveDestPath)
        {
            string directory = Path.GetDirectoryName(localPathWithWildcard);
            string pattern = Path.GetFileName(localPathWithWildcard);

            if (string.IsNullOrEmpty(directory))
            {
                directory = Environment.CurrentDirectory;
            }

            ShellTrace.Verbose("Local wildcard expansion: directory=\"{0}\", pattern=\"{1}\"", directory, pattern);

            if (!Directory.Exists(directory))
            {
                Console.WriteLine("Source directory not found: " + directory);
                return;
            }

            // Use Directory.GetFiles which supports wildcards natively
            string[] matchingFiles;
            try
            {
                matchingFiles = Directory.GetFiles(directory, pattern);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error enumerating files: " + ex.Message);
                return;
            }

            ShellTrace.Info("Local wildcard \"{0}\" matched {1} file(s)", pattern, matchingFiles.Length);

            if (matchingFiles.Length == 0)
            {
                Console.WriteLine("No files matching '{0}' found.", pattern);
                return;
            }

            int successCount = 0;
            int failCount = 0;

            foreach (string localFile in matchingFiles)
            {
                string fileName = Path.GetFileName(localFile);
                string destFilePath = CombinePath(bigDriveDestPath.TrimEnd('\\', '/'), fileName);

                ShellTrace.Verbose("Copying: \"{0}\" => \"{1}\"", localFile, destFilePath);

                try
                {
                    ShellTrace.ComCall("IBigDriveFileOperations", "CopyFileToBigDrive",
                        string.Format("driveGuid={0}, localPath=\"{1}\", destPath=\"{2}\"", destConfig.Id, localFile, destFilePath));
                    fileOps.CopyFileToBigDrive(destConfig.Id, localFile, destFilePath);
                    ShellTrace.ComResult("IBigDriveFileOperations", "CopyFileToBigDrive", 0);
                    successCount++;
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Error copying {0}: {1}", fileName, ex.Message);
                    ShellTrace.Error("Failed to copy \"{0}\": {1}", fileName, ex.Message);
                    failCount++;
                }
            }

            Console.WriteLine("        {0} file(s) copied.", successCount);
            if (failCount > 0)
            {
                Console.WriteLine("        {0} file(s) failed.", failCount);
            }
        }

        /// <summary>
        /// Copies a BigDrive file to local storage. Supports wildcard patterns.
        /// </summary>
        private static void CopyBigDriveToLocal(ShellContext context, PathInfo source, PathInfo dest)
        {
            ShellTrace.Enter("CopyCommand", "CopyBigDriveToLocal", 
                string.Format("source={0}, dest={1}", source.GetFullPath(), dest.GetFullPath()));

            DriveConfiguration sourceConfig = context.DriveLetterManager.GetDriveConfiguration(source.DriveLetter);
            if (sourceConfig == null)
            {
                Console.WriteLine("Source drive not found: " + source.DriveLetter + ":");
                ShellTrace.Error("Drive configuration not found for letter '{0}'", source.DriveLetter);
                ShellTrace.Exit("CopyCommand", "CopyBigDriveToLocal", "drive not found");
                return;
            }

            ShellTrace.Verbose("Source drive: Id={0}, Name=\"{1}\", CLSID={2}", 
                sourceConfig.Id, sourceConfig.Name, sourceConfig.CLSID);

            string localPath = dest.GetFullPath();
            ShellTrace.Verbose("Destination local path: \"{0}\"", localPath);

            // Check for wildcard pattern
            if (WildcardMatcher.ContainsWildcard(source.Path))
            {
                ShellTrace.Info("Wildcard detected in source path, expanding...");
                CopyBigDriveToLocalWithWildcard(context, sourceConfig, source, localPath);
                return;
            }

            // Single file copy
            CopySingleBigDriveFileToLocal(sourceConfig, source.Path, localPath);
            ShellTrace.Exit("CopyCommand", "CopyBigDriveToLocal", "complete");
        }

        /// <summary>
        /// Copies multiple BigDrive files matching a wildcard pattern to local storage.
        /// </summary>
        private static void CopyBigDriveToLocalWithWildcard(
            ShellContext context, 
            DriveConfiguration sourceConfig, 
            PathInfo source, 
            string localPath)
        {
            // Split path into directory and pattern
            WildcardMatcher.SplitPathAndPattern(source.Path, out string directoryPath, out string filePattern);
            ShellTrace.Verbose("Wildcard expansion: directory=\"{0}\", pattern=\"{1}\"", directoryPath, filePattern);

            // Get enumerate provider to list files
            IBigDriveEnumerate enumerate = ProviderFactory.GetEnumerateProvider(sourceConfig.Id);
            if (enumerate == null)
            {
                Console.WriteLine("Provider does not support file enumeration.");
                return;
            }

            // Get all files in the directory
            ShellTrace.ComCall("IBigDriveEnumerate", "EnumerateFiles", 
                string.Format("driveGuid={0}, path=\"{1}\"", sourceConfig.Id, directoryPath));
            string[] allFiles = enumerate.EnumerateFiles(sourceConfig.Id, directoryPath);
            ShellTrace.ComResult("IBigDriveEnumerate", "EnumerateFiles", 0, 
                string.Format("{0} files found", allFiles.Length));

            // Filter by wildcard pattern
            List<string> matchingFiles = WildcardMatcher.Filter(allFiles, filePattern).ToList();
            ShellTrace.Info("Wildcard \"{0}\" matched {1} file(s)", filePattern, matchingFiles.Count);

            if (matchingFiles.Count == 0)
            {
                Console.WriteLine("No files matching '{0}' found.", filePattern);
                return;
            }

            // Ensure destination is a directory for multiple files
            string destDirectory = localPath;
            if (matchingFiles.Count > 1)
            {
                if (!Directory.Exists(destDirectory.TrimEnd('\\', '/')))
                {
                    Console.WriteLine("Destination must be a directory when copying multiple files.");
                    return;
                }
            }

            // Copy each matching file
            int successCount = 0;
            int failCount = 0;

            foreach (string fileName in matchingFiles)
            {
                string sourceFilePath = CombinePath(directoryPath, fileName);
                string destFilePath;

                // Determine destination file path
                if (Directory.Exists(destDirectory.TrimEnd('\\', '/')) || 
                    destDirectory.EndsWith("\\") || 
                    destDirectory.EndsWith("/"))
                {
                    destFilePath = Path.Combine(destDirectory.TrimEnd('\\', '/'), fileName);
                }
                else
                {
                    destFilePath = destDirectory;
                }

                ShellTrace.Verbose("Copying: \"{0}\" => \"{1}\"", sourceFilePath, destFilePath);

                try
                {
                    CopySingleBigDriveFileToLocal(sourceConfig, sourceFilePath, destFilePath);
                    successCount++;
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Error copying {0}: {1}", fileName, ex.Message);
                    ShellTrace.Error("Failed to copy \"{0}\": {1}", fileName, ex.Message);
                    failCount++;
                }
            }

            Console.WriteLine("        {0} file(s) copied.", successCount);
            if (failCount > 0)
            {
                Console.WriteLine("        {0} file(s) failed.", failCount);
            }
        }

        /// <summary>
        /// Copies a single BigDrive file to local storage (no wildcard handling).
        /// </summary>
        private static void CopySingleBigDriveFileToLocal(
            DriveConfiguration sourceConfig, 
            string sourcePath, 
            string localPath)
        {
            // Try IBigDriveFileOperations first
            IBigDriveFileOperations fileOps = ProviderFactory.GetFileOperationsProvider(sourceConfig.Id);
            if (fileOps != null)
            {
                ShellTrace.ComCall("IBigDriveFileOperations", "CopyFileFromBigDrive", 
                    string.Format("driveGuid={0}, sourcePath=\"{1}\", localPath=\"{2}\"", sourceConfig.Id, sourcePath, localPath));

                fileOps.CopyFileFromBigDrive(sourceConfig.Id, sourcePath, localPath);
                ShellTrace.ComResult("IBigDriveFileOperations", "CopyFileFromBigDrive", 0);
                return;
            }

            // Fallback to IBigDriveFileData
            IBigDriveFileData fileData = ProviderFactory.GetFileDataProvider(sourceConfig.Id);
            if (fileData != null)
            {
                ShellTrace.ComCall("IBigDriveFileData", "GetFileData",
                    string.Format("driveGuid={0}, path=\"{1}\"", sourceConfig.Id, sourcePath));

                int hr = fileData.GetFileData(sourceConfig.Id, sourcePath, out IStream stream);
                ShellTrace.ComResult("IBigDriveFileData", "GetFileData", hr, 
                    stream != null ? "stream returned" : "stream is null");

                if (hr != 0)
                {
                    throw new IOException("Failed to get file data. HRESULT: 0x" + hr.ToString("X8"));
                }

                // Ensure destination directory exists
                string destDir = Path.GetDirectoryName(localPath);
                if (!string.IsNullOrEmpty(destDir) && !Directory.Exists(destDir))
                {
                    Directory.CreateDirectory(destDir);
                }

                WriteStreamToFile(stream, localPath);
                return;
            }

            throw new InvalidOperationException("Source provider does not support file operations or file data.");
        }

        /// <summary>
        /// Combines two path segments for BigDrive paths.
        /// </summary>
        private static string CombinePath(string basePath, string fileName)
        {
            if (string.IsNullOrEmpty(basePath) || basePath == "\\")
            {
                return "\\" + fileName;
            }

            return basePath.TrimEnd('\\') + "\\" + fileName;
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
