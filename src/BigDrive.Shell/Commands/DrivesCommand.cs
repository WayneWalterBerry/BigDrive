// <copyright file="DrivesCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Lists all registered BigDrive drives and their assigned drive letters.
    /// </summary>
    public class DrivesCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "drives"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "list" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Lists all drives (BigDrive and local)"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "drives"; }
        }

        /// <summary>
        /// Executes the drives command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            DriveLetterManager driveManager = context.DriveLetterManager;

            // Show BigDrive drives
            IReadOnlyDictionary<char, DriveConfiguration> bigDrives = driveManager.BigDriveLetters;

            if (bigDrives.Count > 0)
            {
                Console.WriteLine("BigDrive drives:");
                Console.WriteLine();

                // Sort by drive letter descending (Z, Y, X...)
                List<char> sortedLetters = bigDrives.Keys.OrderByDescending(c => c).ToList();

                foreach (char letter in sortedLetters)
                {
                    DriveConfiguration config = bigDrives[letter];
                    string currentPath = context.GetPathForDrive(letter);
                    string marker = (letter == context.CurrentDriveLetter) ? " *" : "  ";

                    Console.WriteLine("{0}{1}:  {2}", marker, letter, config.Name);

                    if (currentPath != "\\")
                    {
                        Console.WriteLine("       Current: {0}:{1}", letter, currentPath);
                    }
                }

                Console.WriteLine();
            }
            else
            {
                Console.WriteLine("No BigDrive drives registered.");
                Console.WriteLine();
            }

            // Show local OS drives
            IReadOnlyCollection<char> osDrives = driveManager.OSDriveLetters;

            if (osDrives.Count > 0)
            {
                Console.WriteLine("Local drives:");
                Console.Write("  ");

                List<char> sortedOsDrives = osDrives.OrderBy(c => c).ToList();
                Console.WriteLine(string.Join(", ", sortedOsDrives.Select(c => c + ":")));
                Console.WriteLine();
            }

            Console.WriteLine("Use 'cd X:' to switch to a drive.");
        }
    }
}
