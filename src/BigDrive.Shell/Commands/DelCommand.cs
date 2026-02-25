// <copyright file="DelCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    using BigDrive.Interfaces;

    /// <summary>
    /// Deletes files or directories from BigDrive.
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
            if (context.CurrentDriveLetter == '\0' || !context.CurrentDriveGuid.HasValue)
            {
                Console.WriteLine("No drive selected. Use 'cd X:' to select a BigDrive.");
                return;
            }

            if (args.Length == 0)
            {
                Console.WriteLine("Usage: " + Usage);
                return;
            }

            IBigDriveFileOperations fileOps = ProviderFactory.GetFileOperationsProvider(context.CurrentDriveGuid.Value);
            if (fileOps == null)
            {
                Console.WriteLine("Provider does not support file operations.");
                return;
            }

            string inputPath = args[0];

            // Check for wildcard
            if (WildcardMatcher.ContainsWildcard(inputPath))
            {
                DeleteWithWildcard(context, fileOps, inputPath);
                return;
            }

            // Single file/folder delete
            string path = ResolvePath(context.CurrentPath, inputPath);
            ShellTrace.ComCall("IBigDriveFileOperations", "DeleteFile",
                string.Format("driveGuid={0}, path=\"{1}\"", context.CurrentDriveGuid.Value, path));

            fileOps.DeleteFile(context.CurrentDriveGuid.Value, path);
            ShellTrace.ComResult("IBigDriveFileOperations", "DeleteFile", 0);
            Console.WriteLine("Deleted: " + inputPath);
        }

        /// <summary>
        /// Deletes multiple files matching a wildcard pattern.
        /// </summary>
        private static void DeleteWithWildcard(ShellContext context, IBigDriveFileOperations fileOps, string inputPath)
        {
            // Split into directory and pattern
            WildcardMatcher.SplitPathAndPattern(inputPath, out string dirPart, out string filePattern);

            // Resolve directory
            string directoryPath;
            if (dirPart == "\\" || string.IsNullOrEmpty(dirPart))
            {
                directoryPath = context.CurrentPath;
            }
            else
            {
                directoryPath = ResolvePath(context.CurrentPath, dirPart);
            }

            ShellTrace.Verbose("Wildcard delete: directory=\"{0}\", pattern=\"{1}\"", directoryPath, filePattern);

            // Get enumerate provider to list files
            IBigDriveEnumerate enumerate = ProviderFactory.GetEnumerateProvider(context.CurrentDriveGuid.Value);
            if (enumerate == null)
            {
                Console.WriteLine("Provider does not support file enumeration.");
                return;
            }

            // Get all files in the directory
            string[] allFiles = enumerate.EnumerateFiles(context.CurrentDriveGuid.Value, directoryPath);

            // Filter by pattern
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

            // Delete each matching file
            int successCount = 0;
            int failCount = 0;

            foreach (string fileName in matchingFiles)
            {
                string fullPath = CombinePath(directoryPath, fileName);

                try
                {
                    ShellTrace.ComCall("IBigDriveFileOperations", "DeleteFile",
                        string.Format("driveGuid={0}, path=\"{1}\"", context.CurrentDriveGuid.Value, fullPath));
                    fileOps.DeleteFile(context.CurrentDriveGuid.Value, fullPath);
                    ShellTrace.ComResult("IBigDriveFileOperations", "DeleteFile", 0);
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

        /// <summary>
        /// Combines two path segments.
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
        /// Resolves a relative or absolute path.
        /// </summary>
        /// <param name="currentPath">The current path.</param>
        /// <param name="targetPath">The target path.</param>
        /// <returns>The resolved absolute path.</returns>
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
