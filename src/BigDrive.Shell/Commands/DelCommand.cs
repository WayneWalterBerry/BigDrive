// <copyright file="DelCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;

    using BigDrive.Interfaces;

    /// <summary>
    /// Deletes a file or directory from BigDrive.
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
            get { return "Deletes a file or directory"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "del <file|directory>"; }
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

            string path = ResolvePath(context.CurrentPath, args[0]);

            fileOps.DeleteFile(context.CurrentDriveGuid.Value, path);
            Console.WriteLine("Deleted: " + args[0]);
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
