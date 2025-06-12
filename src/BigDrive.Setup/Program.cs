// <copyright file="Program.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Setup
{
    using System;
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
            BoostrapBigDriveEventLogs();

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

            /// Grant Full Control to the BigDriveInstaller for the BigDrive registry keys
            RegistryManager.GrantFullControl(password);

            // Build the full path to BigDrive.Service.dll in the same directory as the executable
            string assemblyPath = System.IO.Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory,
                "BigDrive.Service.dll");

            ComRegistrationManager.RegisterComAssemblyAsUser(assemblyPath: assemblyPath);

            ComRegistrationManager.SetApplicationIdentityToThisUser(
                applicationName: ComRegistrationManager.ComPlusServiceName,
                username: UserManager.BigDriveTrustedInstallerUserName,
                password: password.ToString());

            Guid passIdentifier = Guid.NewGuid();
            Console.WriteLine($"Pass Identifier: {passIdentifier}");

            ComRegistrationManager.CallServiceValidate(passIdentifier);
        }

        private static bool IsRunningElevated()
        {
            using (WindowsIdentity identity = WindowsIdentity.GetCurrent())
            {
                WindowsPrincipal principal = new WindowsPrincipal(identity);
                return principal.IsInRole(WindowsBuiltInRole.Administrator);
            }
        }

        private static void BoostrapBigDriveEventLogs()
        {
            BoostrapBigDriveEventLog(Constants.EventLogService);
            BoostrapBigDriveEventLog(Constants.EventLogShellFolder);
            BoostrapBigDriveEventLog(Constants.EventLogClient);
            BoostrapBigDriveEventLog(Constants.EventLogProviderSample);
        }

        private static void BoostrapBigDriveEventLog(string application)
        {
            ConsoleExtensions.WriteIndented($"Creating Custom Event Source For BigDrive {application}...");

            EventViewerManager eventViewerManager = EventViewerManager.CreateEventViewerManager(application);
            eventViewerManager.CreateEventSource();
        }
    }
}
