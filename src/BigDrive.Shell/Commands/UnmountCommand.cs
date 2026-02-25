// <copyright file="UnmountCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Runtime.InteropServices;

    using BigDrive.ConfigProvider.Model;
    using BigDrive.Service.Interfaces;

    /// <summary>
    /// Unmounts a BigDrive by calling BigDriveService via COM+ to remove the drive
    /// configuration, unregister the shell folder, and refresh Explorer.
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
        /// Executes the unmount command by activating BigDriveService via COM+ and
        /// calling IBigDriveProvision.Delete to remove the drive.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            ShellTrace.Enter("UnmountCommand", "Execute", args.Length > 0 ? args[0] : "(no args)");

            if (args.Length == 0)
            {
                Console.WriteLine("Usage: " + Usage);
                Console.WriteLine();
                Console.WriteLine("Example: unmount Z");
                ShellTrace.Exit("UnmountCommand", "Execute", "no args provided");
                return;
            }

            string driveSelector = args[0].TrimEnd(':');

            if (driveSelector.Length != 1)
            {
                Console.WriteLine("Invalid drive letter: " + args[0]);
                ShellTrace.Exit("UnmountCommand", "Execute", "invalid drive letter: " + args[0]);
                return;
            }

            char driveLetter = char.ToUpper(driveSelector[0]);
            ShellTrace.Info("Unmount requested for drive: {0}:", driveLetter);

            // Check if it's a BigDrive
            if (!context.DriveLetterManager.IsBigDrive(driveLetter))
            {
                if (context.DriveLetterManager.IsOSDrive(driveLetter))
                {
                    Console.WriteLine("Cannot unmount OS drive: " + driveLetter + ":");
                    ShellTrace.Warning("Attempted to unmount OS drive: {0}:", driveLetter);
                }
                else
                {
                    Console.WriteLine("Drive not found: " + driveLetter + ":");
                    ShellTrace.Warning("Drive not found: {0}:", driveLetter);
                }

                ShellTrace.Exit("UnmountCommand", "Execute", "drive not eligible");
                return;
            }

            // Get the drive configuration
            DriveConfiguration driveConfig = context.DriveLetterManager.GetDriveConfiguration(driveLetter);
            if (driveConfig == null)
            {
                Console.WriteLine("Drive configuration not found for: " + driveLetter + ":");
                ShellTrace.Error("Drive configuration not found for: {0}:", driveLetter);
                ShellTrace.Exit("UnmountCommand", "Execute", "no config");
                return;
            }

            ShellTrace.Info("Drive config: Name=\"{0}\", Id={1}, CLSID={2}", driveConfig.Name, driveConfig.Id, driveConfig.CLSID);

            // Confirm unmount
            Console.Write("Unmount '{0}' ({1}:)? [y/N]: ", driveConfig.Name, driveLetter);
            string confirmation = Console.ReadLine();

            if (!string.Equals(confirmation, "y", StringComparison.OrdinalIgnoreCase) &&
                !string.Equals(confirmation, "yes", StringComparison.OrdinalIgnoreCase))
            {
                Console.WriteLine("Unmount cancelled.");
                ShellTrace.Info("Unmount cancelled by user");
                ShellTrace.Exit("UnmountCommand", "Execute", "cancelled");
                return;
            }

            // If currently on this drive, move to root
            if (context.CurrentDriveLetter == driveLetter)
            {
                ShellTrace.Info("Currently on drive {0}:, moving to root", driveLetter);
                context.CurrentDriveLetter = '\0';
                context.CurrentPath = "\\";
            }

            try
            {
                // Activate BigDriveService via COM+ and call Delete
                ShellTrace.Info("Activating BigDriveService via COM+ to delete drive: {0}", driveConfig.Id);
                IBigDriveProvision provisionService = ServiceFactory.GetProvisionService();

                ShellTrace.ComCall("IBigDriveProvision", "UnmountDrive", driveConfig.Id);
                provisionService.UnmountDrive(driveConfig.Id);
                ShellTrace.ComResult("IBigDriveProvision", "UnmountDrive", 0);

                Console.WriteLine("Drive unmounted: " + driveConfig.Name);

                // Release the COM object
                if (Marshal.IsComObject(provisionService))
                {
                    Marshal.ReleaseComObject(provisionService);
                    ShellTrace.Verbose("Released IBigDriveProvision COM object");
                }

                // Refresh drive letters
                ShellTrace.Info("Refreshing drive letter assignments");
                context.RefreshDrives();
            }
            catch (Exception ex)
            {
                ShellTrace.Error("Unmount failed: {0}", ex.Message);
                Console.WriteLine("Failed to unmount drive: " + ex.Message);
            }

            ShellTrace.Exit("UnmountCommand", "Execute", "completed");
        }
    }
}
