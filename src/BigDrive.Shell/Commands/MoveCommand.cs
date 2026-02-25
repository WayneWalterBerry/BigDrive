// <copyright file="MoveCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    using BigDrive.Shell.FileStores;

    /// <summary>
    /// Moves files between BigDrive drives and local drives.
    /// Supports: BigDrive to BigDrive, Local to BigDrive, BigDrive to Local.
    /// Supports wildcard patterns (* and ?) for moving multiple files.
    /// </summary>
    public class MoveCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "move"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "mv" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Moves files between drives (supports wildcards: *, ?)"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "move <source> [destination]  |  move *.txt archive\\"; }
        }

        /// <summary>
        /// Executes the move command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            ShellTrace.Enter("MoveCommand", "Execute", string.Format("args.Length={0}", args.Length));

            if (args.Length < 1)
            {
                Console.WriteLine("Usage: " + Usage);
                ShellTrace.Exit("MoveCommand", "Execute", "insufficient args");
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
                    ShellTrace.Exit("MoveCommand", "Execute", "no destination");
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
                ShellTrace.Exit("MoveCommand", "Execute", "no source store");
                return;
            }

            if (destStore == null)
            {
                Console.WriteLine("Destination drive not found.");
                ShellTrace.Exit("MoveCommand", "Execute", "no dest store");
                return;
            }

            if (!sourceStore.SupportsFileOperations)
            {
                Console.WriteLine("Source does not support file operations.");
                ShellTrace.Exit("MoveCommand", "Execute", "source no file ops");
                return;
            }

            if (!destStore.SupportsFileOperations)
            {
                Console.WriteLine("Destination does not support file operations.");
                ShellTrace.Exit("MoveCommand", "Execute", "dest no file ops");
                return;
            }

            // Handle wildcards
            if (WildcardMatcher.ContainsWildcard(sourcePath.Path))
            {
                MoveWithWildcard(sourceStore, sourcePath.Path, destStore, destPath.Path);
                ShellTrace.Exit("MoveCommand", "Execute", "wildcard complete");
                return;
            }

            // Single file move
            string destFilePath = FileTransferService.ResolveDestinationFilePath(sourcePath.Path, destPath.Path);

            FileTransferService.MoveFile(sourceStore, sourcePath.Path, destStore, destFilePath);
            Console.WriteLine("        1 file(s) moved.");
            ShellTrace.Exit("MoveCommand", "Execute", "complete");
        }

        /// <summary>
        /// Moves multiple files matching a wildcard pattern.
        /// </summary>
        /// <param name="sourceStore">The source file store.</param>
        /// <param name="sourcePathWithWildcard">The source path containing wildcard.</param>
        /// <param name="destStore">The destination file store.</param>
        /// <param name="destPath">The destination directory path.</param>
        private static void MoveWithWildcard(IFileStore sourceStore, string sourcePathWithWildcard, IFileStore destStore, string destPath)
        {
            WildcardMatcher.SplitPathAndPattern(sourcePathWithWildcard, out string dirPart, out string filePattern);

            string directoryPath = (dirPart == "\\" || string.IsNullOrEmpty(dirPart)) ? "\\" : dirPart;

            ShellTrace.Verbose("Wildcard move: directory=\"{0}\", pattern=\"{1}\", dest=\"{2}\"", directoryPath, filePattern, destPath);

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
                    FileTransferService.MoveFile(sourceStore, sourceFullPath, destStore, destFullPath);
                    successCount++;
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Error moving {0}: {1}", fileName, ex.Message);
                    ShellTrace.Error("Failed to move \"{0}\": {1}", fileName, ex.Message);
                    failCount++;
                }
            }

            Console.WriteLine("        {0} file(s) moved.", successCount);
            if (failCount > 0)
            {
                Console.WriteLine("        {0} file(s) failed.", failCount);
            }
        }
    }
}
