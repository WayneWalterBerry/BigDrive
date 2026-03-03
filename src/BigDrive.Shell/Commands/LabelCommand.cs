// <copyright file="LabelCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Changes the display name (label) of a mounted BigDrive.
    /// Similar to the cmd.exe 'label' command for disk volume labels.
    /// </summary>
    public class LabelCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "label"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[0]; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Changes the display name of a BigDrive"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get
            {
                return "label [drive-letter:] <new-name>\n" +
                       "  Examples:\n" +
                       "    label \"My Photos\"         Rename current drive\n" +
                       "    label Y: \"My Photos\"      Rename drive Y:";
            }
        }

        /// <summary>
        /// Executes the label command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            ShellTrace.Enter("LabelCommand", "Execute", string.Format("args.Length={0}", args.Length));

            if (args.Length < 1)
            {
                Console.WriteLine("Usage: " + Usage);
                ShellTrace.Exit("LabelCommand", "Execute", "insufficient args");
                return;
            }

            char driveLetter;
            string newName;

            // Check if first argument is a drive letter (e.g., "Y:" or "Y")
            if (args.Length >= 2 && args[0].Length <= 2 && char.IsLetter(args[0][0]))
            {
                string driveArg = args[0].TrimEnd(':');
                driveLetter = char.ToUpper(driveArg[0]);
                newName = args[1];
            }
            else
            {
                // No drive letter specified — use current drive
                driveLetter = context.CurrentDriveLetter;
                newName = args[0];
            }

            if (driveLetter == '\0')
            {
                Console.WriteLine("No drive selected. Use 'cd X:' to select a drive, or specify one: label Y: \"name\"");
                ShellTrace.Exit("LabelCommand", "Execute", "no drive");
                return;
            }

            if (!context.DriveLetterManager.IsBigDrive(driveLetter))
            {
                Console.WriteLine("Drive {0}: is not a BigDrive.", driveLetter);
                ShellTrace.Exit("LabelCommand", "Execute", "not a BigDrive");
                return;
            }

            if (string.IsNullOrWhiteSpace(newName))
            {
                Console.WriteLine("New name cannot be empty.");
                ShellTrace.Exit("LabelCommand", "Execute", "empty name");
                return;
            }

            DriveConfiguration driveConfig = context.DriveLetterManager.BigDriveLetters[driveLetter];
            string oldName = driveConfig.Name;

            ShellTrace.Verbose("Renaming drive {0}: from \"{1}\" to \"{2}\"", driveLetter, oldName, newName);

            try
            {
                DriveManager.WriteDriveProperty(driveConfig.Id, "name", newName, CancellationToken.None);

                // Refresh so the shell picks up the new name
                context.RefreshDrives();

                Console.WriteLine("{0}: {1} => {2}", driveLetter, oldName, newName);
            }
            catch (UnauthorizedAccessException)
            {
                Console.WriteLine("Access denied. Run BigDrive.Shell as Administrator to rename drives.");
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to rename drive: {0}", ex.Message);
                ShellTrace.Error("Label failed: {0}", ex.Message);
            }

            ShellTrace.Exit("LabelCommand", "Execute", "complete");
        }
    }
}
