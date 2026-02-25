// <copyright file="MkdirCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;

    using BigDrive.Interfaces;

    /// <summary>
    /// Creates a new directory in BigDrive.
    /// </summary>
    public class MkdirCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "mkdir"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "md" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Creates a new directory"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "mkdir <directory>"; }
        }

        /// <summary>
        /// Executes the mkdir command.
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

            string path = PathInfo.ResolvePath(context.CurrentPath, args[0]);

            fileOps.CreateDirectory(context.CurrentDriveGuid.Value, path);
            Console.WriteLine("Directory created: " + args[0]);
        }
    }
}
