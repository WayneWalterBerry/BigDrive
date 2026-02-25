// <copyright file="RenameCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;

    using BigDrive.Interfaces;
    using BigDrive.Shell.FileStores;

    /// <summary>
    /// Renames a file or directory within a BigDrive.
    /// Implemented as a move within the same directory.
    /// </summary>
    public class RenameCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "rename"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "ren" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Renames a file or directory"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "rename <oldname> <newname>"; }
        }

        /// <summary>
        /// Executes the rename command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            ShellTrace.Enter("RenameCommand", "Execute", string.Format("args.Length={0}", args.Length));

            if (context.CurrentDriveLetter == '\0' || !context.CurrentDriveGuid.HasValue)
            {
                Console.WriteLine("No drive selected. Use 'cd X:' to select a BigDrive.");
                ShellTrace.Exit("RenameCommand", "Execute", "no drive selected");
                return;
            }

            if (args.Length < 2)
            {
                Console.WriteLine("Usage: " + Usage);
                ShellTrace.Exit("RenameCommand", "Execute", "insufficient args");
                return;
            }

            string oldName = args[0];
            string newName = args[1];

            ShellTrace.Verbose("Old name: \"{0}\"", oldName);
            ShellTrace.Verbose("New name: \"{0}\"", newName);

            // New name should be just a name, not a path
            if (newName.Contains("\\") || newName.Contains("/"))
            {
                Console.WriteLine("New name must not contain path separators. Use 'move' to relocate files.");
                ShellTrace.Exit("RenameCommand", "Execute", "new name contains path separators");
                return;
            }

            IBigDriveFileOperations fileOps = ProviderFactory.GetFileOperationsProvider(context.CurrentDriveGuid.Value);
            if (fileOps == null)
            {
                Console.WriteLine("Provider does not support file operations.");
                ShellTrace.Exit("RenameCommand", "Execute", "no file operations");
                return;
            }

            // Resolve the source path
            string sourcePath = PathInfo.ResolvePath(context.CurrentPath, oldName);

            // Build destination path: same directory, new name
            string sourceDirectory = GetDirectoryPart(sourcePath);
            string destPath = FileTransferService.CombinePath(sourceDirectory, newName);

            ShellTrace.Verbose("Resolved source: \"{0}\"", sourcePath);
            ShellTrace.Verbose("Resolved destination: \"{0}\"", destPath);

            ShellTrace.ComCall("IBigDriveFileOperations", "MoveFile",
                string.Format("driveGuid={0}, source=\"{1}\", dest=\"{2}\"", context.CurrentDriveGuid.Value, sourcePath, destPath));

            fileOps.MoveFile(context.CurrentDriveGuid.Value, sourcePath, destPath);
            ShellTrace.ComResult("IBigDriveFileOperations", "MoveFile", 0);
            Console.WriteLine("{0} => {1}", oldName, newName);
            ShellTrace.Exit("RenameCommand", "Execute", "complete");
        }

        /// <summary>
        /// Gets the directory portion of a path.
        /// </summary>
        /// <param name="path">The full path.</param>
        /// <returns>The directory portion, or root if no separator found.</returns>
        private static string GetDirectoryPart(string path)
        {
            int lastSep = path.LastIndexOf('\\');
            if (lastSep <= 0)
            {
                return "\\";
            }

            return path.Substring(0, lastSep);
        }

            }
        }
