// <copyright file="DelCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    using BigDrive.Shell.FileStores;

    /// <summary>
    /// Deletes files or directories from a BigDrive or local drive.
    /// Supports wildcard patterns (* and ?) for deleting multiple files.
    /// </summary>
    public class DelCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "del"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "rm", "delete", "erase" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Deletes files or directories (supports wildcards: *, ?)"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "del <file|directory>  |  del *.tmp  |  del file?.txt"; }
        }

        /// <summary>
        /// Executes the del command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            ShellTrace.Enter("DelCommand", "Execute", string.Format("args.Length={0}", args.Length));

            if (args.Length == 0)
            {
                Console.WriteLine("Usage: " + Usage);
                ShellTrace.Exit("DelCommand", "Execute", "insufficient args");
                return;
            }

            string inputPath = args[0];

            PathInfo pathInfo = PathInfo.Parse(inputPath, context.DriveLetterManager, context.CurrentDriveLetter);

            // Resolve relative BigDrive paths
            if (pathInfo.IsRelative && pathInfo.IsBigDrive)
            {
                pathInfo.Path = PathInfo.ResolvePath(context.GetPathForDrive(pathInfo.DriveLetter), pathInfo.Path);
            }

            IFileStore store = FileStoreFactory.Create(pathInfo, context);
            if (store == null)
            {
                Console.WriteLine("No drive selected. Use 'cd X:' to select a drive first.");
                ShellTrace.Exit("DelCommand", "Execute", "no store");
                return;
            }

            if (!store.SupportsFileOperations)
            {
                Console.WriteLine("Drive does not support file operations.");
                ShellTrace.Exit("DelCommand", "Execute", "no file ops");
                return;
            }

            // Handle wildcards
            if (WildcardMatcher.ContainsWildcard(pathInfo.Path))
            {
                DeleteWithWildcard(store, pathInfo.Path);
                ShellTrace.Exit("DelCommand", "Execute", "wildcard complete");
                return;
            }

            // Single file/folder delete
            store.DeleteFile(pathInfo.Path);
            Console.WriteLine("Deleted: " + inputPath);
            ShellTrace.Exit("DelCommand", "Execute", "complete");
        }

        /// <summary>
        /// Deletes multiple files matching a wildcard pattern.
        /// </summary>
        /// <param name="store">The file store to delete from.</param>
        /// <param name="pathWithWildcard">The path containing wildcard pattern.</param>
        private static void DeleteWithWildcard(IFileStore store, string pathWithWildcard)
        {
            WildcardMatcher.SplitPathAndPattern(pathWithWildcard, out string dirPart, out string filePattern);

            string directoryPath = (dirPart == "\\" || string.IsNullOrEmpty(dirPart)) ? "\\" : dirPart;

            ShellTrace.Verbose("Wildcard delete: directory=\"{0}\", pattern=\"{1}\"", directoryPath, filePattern);

            if (!store.SupportsEnumeration)
            {
                Console.WriteLine("Drive does not support file enumeration.");
                return;
            }

            string[] allFiles = store.EnumerateFiles(directoryPath);
            List<string> matchingFiles = WildcardMatcher.Filter(allFiles, filePattern).ToList();
            ShellTrace.Info("Wildcard \"{0}\" matched {1} file(s)", filePattern, matchingFiles.Count);

            if (matchingFiles.Count == 0)
            {
                Console.WriteLine("No files matching '{0}' found.", filePattern);
                return;
            }

            // Confirm deletion of multiple files
            if (matchingFiles.Count > 1)
            {
                Console.Write("Delete {0} files? [y/N]: ", matchingFiles.Count);
                string response = Console.ReadLine();
                if (string.IsNullOrEmpty(response) ||
                    (!response.Equals("y", StringComparison.OrdinalIgnoreCase) &&
                     !response.Equals("yes", StringComparison.OrdinalIgnoreCase)))
                {
                    Console.WriteLine("Delete cancelled.");
                    return;
                }
            }

            int successCount = 0;
            int failCount = 0;

            foreach (string fileName in matchingFiles)
            {
                string fullPath = FileTransferService.CombinePath(directoryPath, fileName);

                try
                {
                    store.DeleteFile(fullPath);
                    successCount++;
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Error deleting {0}: {1}", fileName, ex.Message);
                    ShellTrace.Error("Failed to delete \"{0}\": {1}", fileName, ex.Message);
                    failCount++;
                }
            }

            Console.WriteLine("        {0} file(s) deleted.", successCount);
            if (failCount > 0)
            {
                Console.WriteLine("        {0} file(s) failed.", failCount);
            }
        }
    }
}
