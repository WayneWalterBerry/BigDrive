// <copyright file="CopyCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    using BigDrive.Shell.FileStores;

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
            get { return "copy <source> [destination]  |  copy *.txt c:\\temp\\  |  copy X:\\*.jpg Y:\\backup\\"; }
        }

        /// <summary>
        /// Executes the copy command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            ShellTrace.Enter("CopyCommand", "Execute", string.Format("args.Length={0}", args.Length));

            if (args.Length < 1)
            {
                Console.WriteLine("Usage: " + Usage);
                ShellTrace.Exit("CopyCommand", "Execute", "insufficient args");
                return;
            }

            string source = args[0];

            // Default destination to current drive+path when not specified (matches cmd.exe behavior)
            string destination;
            if (args.Length >= 2)
            {
                destination = args[1];
            }
            else
            {
                if (context.CurrentDriveLetter != '\0')
                {
                    destination = context.CurrentDriveLetter + ":" + context.CurrentPath;
                }
                else
                {
                    Console.WriteLine("No destination specified and no current drive selected.");
                    ShellTrace.Exit("CopyCommand", "Execute", "no destination");
                    return;
                }
            }

            ShellTrace.Verbose("Source argument: \"{0}\"", source);
            ShellTrace.Verbose("Destination argument: \"{0}\"", destination);

            PathInfo sourcePath = PathInfo.Parse(source, context.DriveLetterManager, context.CurrentDriveLetter);
            PathInfo destPath = PathInfo.Parse(destination, context.DriveLetterManager, context.CurrentDriveLetter);

            // Resolve relative BigDrive paths
            if (sourcePath.IsRelative && sourcePath.IsBigDrive)
            {
                sourcePath.Path = PathInfo.ResolvePath(context.GetPathForDrive(sourcePath.DriveLetter), sourcePath.Path);
            }

            if (destPath.IsRelative && destPath.IsBigDrive)
            {
                destPath.Path = PathInfo.ResolvePath(context.GetPathForDrive(destPath.DriveLetter), destPath.Path);
            }

            ShellTrace.PathResolution(source, sourcePath.GetFullPath(),
                sourcePath.IsBigDrive ? "BigDrive" : (sourcePath.IsOSDrive ? "OS" : "Unknown"));
            ShellTrace.PathResolution(destination, destPath.GetFullPath(),
                destPath.IsBigDrive ? "BigDrive" : (destPath.IsOSDrive ? "OS" : "Unknown"));

            ShellTrace.Verbose("Source: DriveLetter={0}, Path=\"{1}\", IsBigDrive={2}, IsOSDrive={3}",
                sourcePath.DriveLetter, sourcePath.Path, sourcePath.IsBigDrive, sourcePath.IsOSDrive);
            ShellTrace.Verbose("Dest: DriveLetter={0}, Path=\"{1}\", IsBigDrive={2}, IsOSDrive={3}",
                destPath.DriveLetter, destPath.Path, destPath.IsBigDrive, destPath.IsOSDrive);

            // Create file stores
            IFileStore sourceStore = FileStoreFactory.Create(sourcePath, context);
            IFileStore destStore = FileStoreFactory.Create(destPath, context);

            if (sourceStore == null)
            {
                Console.WriteLine("No drive selected. Use 'cd X:' to select a drive first.");
                ShellTrace.Exit("CopyCommand", "Execute", "no source store");
                return;
            }

            if (destStore == null)
            {
                Console.WriteLine("Destination drive not found.");
                ShellTrace.Exit("CopyCommand", "Execute", "no dest store");
                return;
            }

            // Handle wildcards
            if (WildcardMatcher.ContainsWildcard(sourcePath.Path))
            {
                CopyWithWildcard(sourceStore, sourcePath.Path, destStore, destPath.Path);
                ShellTrace.Exit("CopyCommand", "Execute", "wildcard complete");
                return;
            }

            // Single file copy
            if (!sourceStore.FileExists(sourcePath.Path))
            {
                Console.WriteLine("Source file not found: " + source);
                ShellTrace.Exit("CopyCommand", "Execute", "source not found");
                return;
            }

            string destFilePath = FileTransferService.ResolveDestinationFilePath(sourcePath.Path, destPath.Path);

            FileTransferService.CopyFile(sourceStore, sourcePath.Path, destStore, destFilePath);
            Console.WriteLine("        1 file(s) copied.");
            ShellTrace.Exit("CopyCommand", "Execute", "complete");
        }

        /// <summary>
        /// Copies multiple files matching a wildcard pattern.
        /// </summary>
        /// <param name="sourceStore">The source file store.</param>
        /// <param name="sourcePathWithWildcard">The source path containing wildcard.</param>
        /// <param name="destStore">The destination file store.</param>
        /// <param name="destPath">The destination directory path.</param>
        private static void CopyWithWildcard(IFileStore sourceStore, string sourcePathWithWildcard, IFileStore destStore, string destPath)
        {
            WildcardMatcher.SplitPathAndPattern(sourcePathWithWildcard, out string dirPart, out string filePattern);

            string directoryPath = (dirPart == "\\" || string.IsNullOrEmpty(dirPart)) ? "\\" : dirPart;

            ShellTrace.Verbose("Wildcard copy: directory=\"{0}\", pattern=\"{1}\", dest=\"{2}\"", directoryPath, filePattern, destPath);

            if (!sourceStore.SupportsEnumeration)
            {
                Console.WriteLine("Source does not support file enumeration.");
                return;
            }

            string[] allFiles = sourceStore.EnumerateFiles(directoryPath);
            List<string> matchingFiles = WildcardMatcher.Filter(allFiles, filePattern).ToList();
            ShellTrace.Info("Wildcard \"{0}\" matched {1} file(s)", filePattern, matchingFiles.Count);

            if (matchingFiles.Count == 0)
            {
                Console.WriteLine("No files matching '{0}' found.", filePattern);
                return;
            }

            int successCount = 0;
            int failCount = 0;

            foreach (string fileName in matchingFiles)
            {
                string sourceFullPath = FileTransferService.CombinePath(directoryPath, fileName);
                string destFullPath = FileTransferService.CombinePath(destPath, fileName);

                try
                {
                    FileTransferService.CopyFile(sourceStore, sourceFullPath, destStore, destFullPath);
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

            }
        }
