// <copyright file="RegistrationHelper.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Setup
{
    using Microsoft.Win32;
    using System;
    using System.Threading;

    /// <summary>
    /// Provides static methods for registering, unregistering, and managing custom shell folder extensions
    /// in the Windows registry for the BigDrive application.
    /// </summary>
    public static class RegistrationHelper
    {
        /// <summary>
        /// Registers the shell folder with the given drive Guid and display name.
        /// </summary>
        /// <param name="guidDrive">Drive Guid</param>
        /// <param name="displayName">Display name</param>
        public static void RegisterShellFolder(Guid guidDrive, string displayName, CancellationToken cancellationToken)
        {
            if (guidDrive == Guid.Empty)
            {
                throw new ArgumentException("The drive GUID cannot be empty.", nameof(guidDrive));
            }

            // Register the in-process server for the drive shell folder
            RegisterInprocServer32(guidDrive, displayName, cancellationToken);

            // Register the namespace for the drive shell folder
            RegisterNamespace(guidDrive, cancellationToken);

            // Create the component category registry key for the drive shell folder
            CreateComponentCategoryRegistryKey(guidDrive, cancellationToken);

            // Register the DefaultIcon for the ShellFolder
            RegisterDefaultIcon(guidDrive, cancellationToken);
        }

        public static void RegisterNamespace(Guid guidDrive, CancellationToken cancellationToken)
        {
            string guidString = guidDrive.ToString("B");
            string namespacePath = $@"Software\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace\{guidString}";

            // Register as a Drive (directly as a ShellFolder) in the user's namespace
            using (RegistryKey key = Registry.LocalMachine.CreateSubKey(namespacePath, RegistryKeyPermissionCheck.ReadWriteSubTree))
            {
                // No value needs to be set, just the key created
            }
        }

        /// <summary>
        /// Registers the COM in-process server for the specified drive shell folder CLSID.
        /// </summary>
        /// <param name="guidDrive">The CLSID of the drive shell folder to register.</param>
        /// <param name="displayName">The display name of the drive shell folder.</param>
        private static void RegisterInprocServer32(Guid guidDrive, string displayName, CancellationToken cancellationToken)
        {
            string guidString = guidDrive.ToString("B");
            string clsidPath = $@"CLSID\{guidString}";
            string inprocPath = $@"CLSID\{guidString}\InprocServer32";
            string implementedCategoriesPath = $@"CLSID\{guidString}\Implemented Categories\{{00021490-0000-0000-C000-000000000046}}";
            string shellFolderPath = $@"CLSID\{guidString}\ShellFolder";

            // Set display name
            using (RegistryKey clsidKey = Registry.ClassesRoot.CreateSubKey(clsidPath, RegistryKeyPermissionCheck.ReadWriteSubTree))
            {
                clsidKey.SetValue(null, displayName, RegistryValueKind.String);
            }

            // Set InprocServer32
            using (RegistryKey inprocKey = Registry.ClassesRoot.CreateSubKey(inprocPath, RegistryKeyPermissionCheck.ReadWriteSubTree))
            {
                string modulePath = System.Reflection.Assembly.GetExecutingAssembly().Location;
                inprocKey.SetValue(null, modulePath, RegistryValueKind.String);
                inprocKey.SetValue("ThreadingModel", "Apartment", RegistryValueKind.String);
            }

            // Implemented Categories
            using (RegistryKey catKey = Registry.ClassesRoot.CreateSubKey(implementedCategoriesPath, RegistryKeyPermissionCheck.ReadWriteSubTree))
            {
                // No value needed, just the key
            }

            // ShellFolder attributes
            using (RegistryKey shellFolderKey = Registry.ClassesRoot.CreateSubKey(shellFolderPath, RegistryKeyPermissionCheck.ReadWriteSubTree))
            {
                uint dwAttributes = 0x20000000 | 0x80000000 | 0x10000000; // SFGAO_FOLDER | SFGAO_HASSUBFOLDER | SFGAO_FILESYSANCESTOR
                shellFolderKey.SetValue("Attributes", unchecked((int)dwAttributes), RegistryValueKind.DWord);
                shellFolderKey.SetValue("FolderType", "Storage", RegistryValueKind.String);
            }
        }

        /// <summary>
        /// Registers the DefaultIcon registry entry for the specified drive shell extension CLSID.
        /// </summary>
        /// <param name="guidDrive">The CLSID of the drive shell folder to register the icon for.</param>
        private static void RegisterDefaultIcon(Guid guidDrive, CancellationToken cancellationToken)
        {
            string guidString = guidDrive.ToString("B");
            string defaultIconKeyPath = $@"CLSID\{guidString}\DefaultIcon";
            using (RegistryKey iconKey = Registry.ClassesRoot.CreateSubKey(defaultIconKeyPath, RegistryKeyPermissionCheck.ReadWriteSubTree))
            {
                iconKey.SetValue(null, @"%SystemRoot%\System32\imageres.dll,-30", RegistryValueKind.String);
            }
        }

        /// <summary>
        /// Creates the component category registry key for the specified drive GUID.
        /// </summary>
        /// <param name="guidDrive">Drive Guid</param>
        private static void CreateComponentCategoryRegistryKey(Guid guidDrive, CancellationToken cancellationToken)
        {
            string guidString = guidDrive.ToString("B");
            string componentCategoryPath = $@"Component Categories\{{00021493-0000-0000-C000-000000000046}}\Implementations";
            using (RegistryKey catKey = Registry.ClassesRoot.CreateSubKey(componentCategoryPath, RegistryKeyPermissionCheck.ReadWriteSubTree))
            {
                using (RegistryKey guidKey = catKey.CreateSubKey(guidString, RegistryKeyPermissionCheck.ReadWriteSubTree))
                {
                    // No value needed, just the key
                }
            }
        }
    }
}