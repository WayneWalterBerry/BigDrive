// <copyright file="UnmountCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Unmounts a BigDrive by removing its drive configuration.
    /// Similar to 'net use /delete' for network drives.
    /// </summary>
    public class UnmountCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "unmount"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "unregister", "remove", "umount" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Unmounts a BigDrive"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "unmount <drive-letter>"; }
        }

        /// <summary>
        /// Executes the unmount command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            if (args.Length == 0)
            {
                Console.WriteLine("Usage: " + Usage);
                Console.WriteLine();
                Console.WriteLine("Example: unmount Z");
                return;
            }

            string driveSelector = args[0].TrimEnd(':');

            if (driveSelector.Length != 1)
            {
                Console.WriteLine("Invalid drive letter: " + args[0]);
                return;
            }

            char driveLetter = char.ToUpper(driveSelector[0]);

            // Check if it's a BigDrive
            if (!context.DriveLetterManager.IsBigDrive(driveLetter))
            {
                if (context.DriveLetterManager.IsOSDrive(driveLetter))
                {
                    Console.WriteLine("Cannot unmount OS drive: " + driveLetter + ":");
                }
                else
                {
                    Console.WriteLine("Drive not found: " + driveLetter + ":");
                }

                return;
            }

            // Get the drive configuration
            DriveConfiguration driveConfig = context.DriveLetterManager.GetDriveConfiguration(driveLetter);
            if (driveConfig == null)
            {
                Console.WriteLine("Drive configuration not found for: " + driveLetter + ":");
                return;
            }

            // Confirm unmount
            Console.Write("Unmount '{0}' ({1}:)? [y/N]: ", driveConfig.Name, driveLetter);
            string confirmation = Console.ReadLine();

            if (!string.Equals(confirmation, "y", StringComparison.OrdinalIgnoreCase) &&
                !string.Equals(confirmation, "yes", StringComparison.OrdinalIgnoreCase))
            {
                Console.WriteLine("Unmount cancelled.");
                return;
            }

            // If currently on this drive, move to root
            if (context.CurrentDriveLetter == driveLetter)
            {
                context.CurrentDriveLetter = '\0';
                context.CurrentPath = "\\";
            }

            try
            {
                // Remove from registry
                DriveManager.DeleteConfiguration(driveConfig.Id, CancellationToken.None);

                Console.WriteLine("Drive unmounted: " + driveConfig.Name);

                // Refresh drive letters
                context.RefreshDrives();
            }
            catch (UnauthorizedAccessException)
            {
                Console.WriteLine("Access denied. Run BigDrive.Shell as Administrator to unmount drives.");
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to unmount drive: " + ex.Message);
            }
        }
    }
}
