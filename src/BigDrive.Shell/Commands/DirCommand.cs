// <copyright file="DirCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    using BigDrive.ConfigProvider.Model;
    using BigDrive.Interfaces;

    /// <summary>
    /// Lists drives at root, or files and folders when inside a drive.
    /// </summary>
    public class DirCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "dir"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "ls" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Lists drives or folder contents"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "dir [path]"; }
        }

        /// <summary>
        /// Executes the dir command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            // If no drive selected, show available drives (like root of file system)
            if (context.CurrentDriveLetter == '\0')
            {
                ListDrives(context);
                return;
            }

            // Inside a drive - show folder contents
            ListFolderContents(context, args);
        }

        /// <summary>
        /// Lists available BigDrive drives (shown at root level).
        /// </summary>
        /// <param name="context">The shell context.</param>
        private static void ListDrives(ShellContext context)
        {
            DriveLetterManager driveManager = context.DriveLetterManager;
            IReadOnlyDictionary<char, DriveConfiguration> bigDrives = driveManager.BigDriveLetters;

            Console.WriteLine();
            Console.WriteLine(" Directory of BigDrive");
            Console.WriteLine();

            if (bigDrives.Count > 0)
            {
                // Sort by drive letter descending (Z, Y, X...)
                List<char> sortedLetters = bigDrives.Keys.OrderByDescending(c => c).ToList();

                foreach (char letter in sortedLetters)
                {
                    DriveConfiguration config = bigDrives[letter];
                    Console.WriteLine("    <DRIVE>  {0}:  {1}", letter, config.Name);
                }

                Console.WriteLine();
                Console.WriteLine("       {0} Drive(s)", bigDrives.Count);
                Console.WriteLine();
                Console.WriteLine("Use 'cd X:' to enter a drive.");
            }
            else
            {
                Console.WriteLine("    No BigDrive drives registered.");
                Console.WriteLine();
                Console.WriteLine("Run BigDrive.Setup.exe to register providers.");
            }
        }

        /// <summary>
        /// Lists folder contents when inside a drive.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">Command arguments.</param>
        private static void ListFolderContents(ShellContext context, string[] args)
        {
            if (!context.CurrentDriveGuid.HasValue)
            {
                Console.WriteLine("Current drive is not a BigDrive.");
                return;
            }

            string path = context.CurrentPath;
            if (args.Length > 0)
            {
                path = ResolvePath(context.CurrentPath, args[0]);
            }

            IBigDriveEnumerate enumerate = ProviderFactory.GetEnumerateProvider(context.CurrentDriveGuid.Value);
            if (enumerate == null)
            {
                Console.WriteLine("Provider does not support enumeration.");
                return;
            }

            Console.WriteLine();
            Console.WriteLine(" Directory of {0}:{1}", context.CurrentDriveLetter, path);
            Console.WriteLine();

            // List folders
            string[] folders = enumerate.EnumerateFolders(context.CurrentDriveGuid.Value, path);
            foreach (string folder in folders)
            {
                Console.WriteLine("    <DIR>    " + folder);
            }

            // List files
            string[] files = enumerate.EnumerateFiles(context.CurrentDriveGuid.Value, path);
            foreach (string file in files)
            {
                Console.WriteLine("             " + file);
            }

            Console.WriteLine();
            Console.WriteLine("       {0} Dir(s)    {1} File(s)", folders.Length, files.Length);
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
