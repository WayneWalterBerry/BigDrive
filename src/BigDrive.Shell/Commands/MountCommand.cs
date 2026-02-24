// <copyright file="MountCommand.cs" company="Wayne Walter Berry">
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
    /// Mounts a new BigDrive by creating a drive configuration.
    /// Similar to 'net use' for network drives.
    /// </summary>
    public class MountCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "mount"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "register", "add" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Mounts a new BigDrive"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "mount [<provider-number> <drive-name>]"; }
        }

        /// <summary>
        /// Executes the mount command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            // Get available providers
            List<ProviderConfiguration> providers;

            try
            {
                providers = ProviderManager.ReadProviders(CancellationToken.None).ToList();
            }
            catch (Exception)
            {
                Console.WriteLine("No providers registered. Run BigDrive.Setup.exe first.");
                return;
            }

            if (providers.Count == 0)
            {
                Console.WriteLine("No providers registered. Run BigDrive.Setup.exe first.");
                return;
            }

            // If no arguments, show interactive mode
            if (args.Length == 0)
            {
                InteractiveMount(context, providers);
                return;
            }

            // If arguments provided, try direct mount
            if (args.Length >= 2)
            {
                DirectMount(context, providers, args);
                return;
            }

            Console.WriteLine("Usage: " + Usage);
            Console.WriteLine();
            Console.WriteLine("Run 'mount' without arguments for interactive mode.");
        }

        /// <summary>
        /// Interactive mount - prompts user for provider and name.
        /// </summary>
        private static void InteractiveMount(ShellContext context, List<ProviderConfiguration> providers)
        {
            Console.WriteLine();
            Console.WriteLine("Available providers:");
            Console.WriteLine();

            for (int i = 0; i < providers.Count; i++)
            {
                Console.WriteLine("  [{0}] {1}", i + 1, providers[i].Name);
                Console.WriteLine("      CLSID: {0}", providers[i].Id);
            }

            Console.WriteLine();
            Console.Write("Select provider number: ");
            string providerInput = Console.ReadLine();

            if (!int.TryParse(providerInput, out int providerIndex) ||
                providerIndex < 1 || providerIndex > providers.Count)
            {
                Console.WriteLine("Invalid provider selection.");
                return;
            }

            ProviderConfiguration selectedProvider = providers[providerIndex - 1];

            Console.Write("Enter drive name: ");
            string driveName = Console.ReadLine();

            if (string.IsNullOrWhiteSpace(driveName))
            {
                Console.WriteLine("Drive name cannot be empty.");
                return;
            }

            CreateDrive(context, selectedProvider, driveName.Trim());
        }

        /// <summary>
        /// Direct mount with command line arguments.
        /// </summary>
        private static void DirectMount(ShellContext context, List<ProviderConfiguration> providers, string[] args)
        {
            string providerSelector = args[0];
            string driveName = string.Join(" ", args.Skip(1));

            ProviderConfiguration selectedProvider = null;

            // Try to parse as number
            if (int.TryParse(providerSelector, out int index))
            {
                if (index >= 1 && index <= providers.Count)
                {
                    selectedProvider = providers[index - 1];
                }
            }

            // Try to match by name
            if (selectedProvider == null)
            {
                foreach (ProviderConfiguration provider in providers)
                {
                    if (string.Equals(provider.Name, providerSelector, StringComparison.OrdinalIgnoreCase))
                    {
                        selectedProvider = provider;
                        break;
                    }
                }
            }

            // Try to match by CLSID
            if (selectedProvider == null && Guid.TryParse(providerSelector, out Guid clsid))
            {
                foreach (ProviderConfiguration provider in providers)
                {
                    if (provider.Id == clsid)
                    {
                        selectedProvider = provider;
                        break;
                    }
                }
            }

            if (selectedProvider == null)
            {
                Console.WriteLine("Provider not found: " + providerSelector);
                Console.WriteLine("Use 'mount' without arguments to see available providers.");
                return;
            }

            if (string.IsNullOrWhiteSpace(driveName))
            {
                Console.WriteLine("Drive name cannot be empty.");
                return;
            }

            CreateDrive(context, selectedProvider, driveName.Trim());
        }

        /// <summary>
        /// Creates a new drive configuration.
        /// </summary>
        private static void CreateDrive(ShellContext context, ProviderConfiguration provider, string driveName)
        {
            try
            {
                // Generate a new GUID for this drive
                Guid driveId = Guid.NewGuid();

                DriveConfiguration driveConfig = new DriveConfiguration
                {
                    Id = driveId,
                    Name = driveName,
                    CLSID = provider.Id
                };

                DriveManager.WriteConfiguration(driveConfig, CancellationToken.None);

                Console.WriteLine();
                Console.WriteLine("Drive mounted successfully!");
                Console.WriteLine();
                Console.WriteLine("  Name:     {0}", driveName);
                Console.WriteLine("  GUID:     {0}", driveId);
                Console.WriteLine("  Provider: {0}", provider.Name);
                Console.WriteLine();

                // Refresh drive letters
                context.RefreshDrives();

                // Show the new drive letter
                char newLetter = context.DriveLetterManager.GetDriveLetter(driveId);
                if (newLetter != '\0')
                {
                    Console.WriteLine("Use 'cd {0}:' to access the new drive.", newLetter);
                }
                else
                {
                    Console.WriteLine("Use 'dir' to see the new drive.");
                }
            }
            catch (UnauthorizedAccessException)
            {
                Console.WriteLine("Access denied. Run BigDrive.Shell as Administrator to mount drives.");
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to mount drive: " + ex.Message);
            }
        }
    }
}
