// <copyright file="Program.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Setup
{
    using System;
    using System.IO;
    using System.Security;
    using System.Security.Principal;

    internal class Program
    {
        static void Main(string[] args)
        {
            if (!IsRunningElevated())
            {
                ConsoleExtensions.WriteIndented("Administrative Rights Required.");
                return;
            }

            ConsoleExtensions.WriteIndented("Bootstrapping BigDrive Event Logs...");
            BootstrapBigDriveEventLogs();

            // Check To See If the Trusted Installer User Exists
            if (UserManager.UserExists())
            {
                ConsoleExtensions.WriteIndented($"User {UserManager.BigDriveTrustedInstallerUserName} already exists. Deleting User...");
                RegistryManager.RemoveFullControl();

                ComRegistrationManager.DeleteComPlusApplication();

                UserManager.DeleteBigDriveTrustedInstaller();
            }

            ConsoleExtensions.WriteIndented($"Creating User: {UserManager.BigDriveTrustedInstallerUserName}...");
            SecureString password = UserManager.CreateBigDriveTrustedInstaller();

            // Ensure the BigDrive registry keys exist
            RegistryManager.EnsureBigDriveRegistryKeyExists();

            /// Grant Full Control to the BigDriveInstaller for the BigDrive registry keys
            RegistryManager.GrantFullControl(password);

            // =============================================================================
            // Register BigDrive.Service as BigDriveInstaller (elevated service account)
            // This service needs elevated permissions to:
            //   - Write to HKCR and shell namespace
            //   - Set provider COM+ application identity to Interactive User
            // =============================================================================
            string serviceAssemblyPath = Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory,
                "BigDrive.Service.dll");

            ConsoleExtensions.WriteIndented("Registering BigDrive.Service (as BigDriveInstaller)...");
            ComRegistrationManager.RegisterComAssemblyAsUser(assemblyPath: serviceAssemblyPath);

            ComRegistrationManager.SetApplicationIdentityToThisUser(
                applicationName: ComRegistrationManager.ComPlusServiceName,
                username: UserManager.BigDriveTrustedInstallerUserName,
                password: password.ToString());

            // =============================================================================
            // Validate the installation
            // =============================================================================
            Guid passIdentifier = Guid.NewGuid();
            Console.WriteLine($"Pass Identifier: {passIdentifier}");

            ComRegistrationManager.CallServiceValidate(passIdentifier);

            ConsoleExtensions.WriteIndented("BigDrive Setup completed successfully.");
            ConsoleExtensions.WriteIndented("");
            ConsoleExtensions.WriteIndented("Note: Providers are self-registering.");
            ConsoleExtensions.WriteIndented("To install a provider, run (as Administrator):");
            ConsoleExtensions.WriteIndented("  regsvcs.exe path\\to\\BigDrive.Provider.YourProvider.dll");
            ConsoleExtensions.WriteIndented("");
            ConsoleExtensions.WriteIndented("The provider's [ComRegisterFunction] will automatically:");
            ConsoleExtensions.WriteIndented("  - Set COM+ application identity to Interactive User");
            ConsoleExtensions.WriteIndented("  - Register the provider in BigDrive's configuration");
        }

        private static bool IsRunningElevated()
        {
            using (WindowsIdentity identity = WindowsIdentity.GetCurrent())
            {
                WindowsPrincipal principal = new WindowsPrincipal(identity);
                return principal.IsInRole(WindowsBuiltInRole.Administrator);
            }
        }

        private static void BootstrapBigDriveEventLogs()
        {
            BootstrapBigDriveEventLog(Constants.EventLogService);
            BootstrapBigDriveEventLog(Constants.EventLogShellFolder);
            BootstrapBigDriveEventLog(Constants.EventLogExtension);
            BootstrapBigDriveEventLog(Constants.EventLogClient);
            BootstrapBigDriveEventLog(Constants.EventLogProviderSample);
        }

        private static void BootstrapBigDriveEventLog(string application)
        {
            ConsoleExtensions.WriteIndented($"Creating Custom Event Source For BigDrive {application}...");

            EventViewerManager eventViewerManager = EventViewerManager.CreateEventViewerManager(application);
            eventViewerManager.CreateEventSource();
        }
    }
}
