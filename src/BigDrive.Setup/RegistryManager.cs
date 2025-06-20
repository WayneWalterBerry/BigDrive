// <copyright file="RegistryManager.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Setup
{
    using Microsoft.Win32;
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Runtime.InteropServices;
    using System.Security;
    using System.Security.AccessControl;
    using System.Security.Principal;

    /// <summary>
    /// Provides methods to take ownership and grant full control of a registry key, then invoke a callback.
    /// </summary>
    public static class RegistryManager
    {
        private static List<(RegistryKey, string)> _pairs = new List<(RegistryKey, string)>();

        static RegistryManager()
        {
            // Initialize the pairs with the root keys and their respective paths
            _pairs.Add((Registry.ClassesRoot, @"Component Categories\{00021493-0000-0000-C000-000000000046}"));
            _pairs.Add((Registry.LocalMachine, @"SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace"));

            //	HKEY_CLASSES_ROOT is a merged view of HKEY_LOCAL_MACHINE\SOFTWARE\Classes and HKEY_CURRENT_USER\Software\Classes.Writes to
            //	HKCR typically redirect to HKLM\SOFTWARE\Classes for system - wide changes.
            _pairs.Add((Registry.ClassesRoot, @"CLSID"));
            _pairs.Add((Registry.LocalMachine, @"SOFTWARE\Classes"));

            _pairs.Add((Registry.LocalMachine, @"SOFTWARE\BigDrive"));
        }

        /// <summary>
        /// Ensures that the registry key 'Software\BigDrive' exists under HKEY_LOCAL_MACHINE.
        /// Creates the key if it does not exist.
        /// </summary>
        public static void EnsureBigDriveRegistryKeyExists()
        {
            const string keyPath = @"Software\BigDrive";
            using (var key = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(keyPath, writable: false))
            {
                if (key == null)
                {
                    // Key does not exist, create it
                    using (Microsoft.Win32.Registry.LocalMachine.CreateSubKey(keyPath, Microsoft.Win32.RegistryKeyPermissionCheck.ReadWriteSubTree))
                    {
                        // Key created, nothing else to do
                    }
                }
            }
        }

        /// <summary>
        /// Grants full control to the BigDriveTrustedInstaller user for the default key.
        /// </summary>
        public static void GrantFullControl(SecureString password)
        {
            ConsoleExtensions.WriteIndented($"Updating Registry To Allow {UserManager.BigDriveTrustedInstallerUserName} User Access...");

            if (password == default(SecureString))
            {
                throw new ArgumentNullException(nameof(password), "Password cannot be null.");
            }

            foreach (var pair in _pairs)
            {
                GrantFullControl(password, pair.Item1, pair.Item2);
            }
        }

        public static void RemoveFullControl()
        {
            ConsoleExtensions.WriteIndented($"Removing Registry Access of {UserManager.BigDriveTrustedInstallerUserName} User...");

            foreach (var pair in _pairs)
            {
                RemoveFullControlForTrustedInstaller(pair.Item1, pair.Item2);
            }
        }

        private static void GrantFullControl(SecureString password, RegistryKey rootKey, string keyPath)
        {
            TakeOwnershipAndGrantFullControl(rootKey, keyPath);

            TestFullControlWithTrustedInstaller(password, rootKey, keyPath);
        }

        private static void TakeOwnershipAndGrantFullControl(RegistryKey rootKey, string keyPath)
        {
            ConsoleExtensions.WriteIndented($"Taking ownership and granting full control for {keyPath}...");

            ExecuteUnderOwnership(rootKey, keyPath, (hKey) =>
            {
                ConsoleExtensions.WriteIndented($"Granting Access Control for {UserManager.BigDriveTrustedInstallerUserName} on {keyPath}...");

                // 4. Grant full control to BigDriveTrustedInstaller user
                var trustedInstallerSid = GetUserSid(UserManager.BigDriveTrustedInstallerUserName);

                var ruleToAdd = new RegistryAccessRule(
                    trustedInstallerSid,
                    RegistryRights.FullControl,
                    InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit,
                    PropagationFlags.None,
                    AccessControlType.Allow);

                var securityToModify = hKey.GetAccessControl();

                ConsoleExtensions.WriteIndented($"Adding full control rule for {UserManager.BigDriveTrustedInstallerUserName}...");
                securityToModify.AddAccessRule(ruleToAdd);

                ConsoleExtensions.WriteIndented($"Setting access control for {UserManager.BigDriveTrustedInstallerUserName}...");
                hKey.SetAccessControl(securityToModify);
            });
        }

        /// <summary>
        /// Removes all access rules for the BigDriveTrustedInstaller user from the specified registry key.
        /// Takes ownership if necessary, and restores the original owner if ownership was taken.
        /// </summary>
        /// <param name="rootKey">The root registry key (e.g., Registry.ClassesRoot).</param>
        /// <param name="keyPath">The registry key path to modify.</param>
        public static void RemoveFullControlForTrustedInstaller(RegistryKey rootKey, string keyPath)
        {
            ConsoleExtensions.WriteIndented($"Removing all access for {UserManager.BigDriveTrustedInstallerUserName} from registry key: {keyPath}...");

            ExecuteUnderOwnership(rootKey, keyPath, (hKey) =>
            {
                ConsoleExtensions.WriteIndented($"Removing access control for {UserManager.BigDriveTrustedInstallerUserName} on {keyPath}...");

                var trustedInstallerSid = GetUserSid(UserManager.BigDriveTrustedInstallerUserName);

                var security = hKey.GetAccessControl();

                // Remove all access rules for the TrustedInstaller user
                bool modified = false;
                foreach (RegistryAccessRule rule in security.GetAccessRules(true, true, typeof(SecurityIdentifier)))
                {
                    if (rule.IdentityReference.Equals(trustedInstallerSid))
                    {
                        security.RemoveAccessRuleAll(rule);
                        modified = true;
                    }
                }

                if (modified)
                {
                    ConsoleExtensions.WriteIndented($"Removed all access rules for {UserManager.BigDriveTrustedInstallerUserName}.");
                    hKey.SetAccessControl(security);
                }
                else
                {
                    ConsoleExtensions.WriteIndented($"No access rules found for {UserManager.BigDriveTrustedInstallerUserName}.");
                }
            });
        }

        /// <summary>
        /// Grants full control of the specified registry key to the BigDriveTrustedInstaller user.
        /// Takes ownership only if the current user does not have full control.
        /// Restores the original owner only if ownership was taken.
        /// Throws exceptions on error.
        /// </summary>
        /// <param name="keyPath">The registry key path to modify.</param>
        private static void ExecuteUnderOwnership(RegistryKey rootKey, string keyPath, Action<RegistryKey> action)
        {
            RegistrySecurity originalSecurity = null;
            SecurityIdentifier originalOwner = null;
            bool ownershipTaken = false;
            bool hasFullControl = false;
            RegistryKey hKey = null;

            try
            {
                // Get the current user SID
                var currentUser = WindowsIdentity.GetCurrent();
                var sid = currentUser.User;

                // 1. Open with READ_CONTROL to read OWNER and DACL
                using (hKey = rootKey.OpenSubKey(keyPath, RegistryKeyPermissionCheck.ReadSubTree, RegistryRights.ReadPermissions))
                {
                    if (hKey == null)
                    {
                        throw new InvalidOperationException($"Failed to open registry key '{keyPath}'.");
                    }

                    originalSecurity = hKey.GetAccessControl(AccessControlSections.Owner);

                    // Save original OWNER
                    originalOwner = originalSecurity.GetOwner(typeof(SecurityIdentifier)) as SecurityIdentifier;
                    if (originalOwner != null)
                    {
                        string ownerName = originalOwner.Translate(typeof(NTAccount)).Value;
                        ConsoleExtensions.WriteIndented($"Current Owner of keyPath: {ownerName}");
                    }

                    // 2. Check if current user has full control
                    var security = hKey.GetAccessControl(AccessControlSections.Access);

                    foreach (AuthorizationRule rule in security.GetAccessRules(true, true, typeof(SecurityIdentifier)))
                    {
                        if (rule is RegistryAccessRule regRule &&
                            regRule.IdentityReference.Equals(sid) &&
                            regRule.AccessControlType == AccessControlType.Allow &&
                            (regRule.RegistryRights & RegistryRights.FullControl) == RegistryRights.FullControl)
                        {
                            hasFullControl = true;
                            break;
                        }
                    }
                }

                // 3. Take ownership if needed
                if (!hasFullControl)
                {
                    ConsoleExtensions.WriteIndented($"Current user '{currentUser.Name}' does not have full control over the key '{keyPath}'.");

                    bool isOwner = originalOwner != null && sid != null && originalOwner.Equals(sid);
                    if (!isOwner)
                    {
                        ConsoleExtensions.WriteIndented("Current user is not the owner, attempting to take ownership...");

                        EnablePrivilege("SeTakeOwnershipPrivilege");
                        EnablePrivilege("SeRestorePrivilege");

                        // Reopen the key with WRITE_OWNER | READ_CONTROL
                        using (hKey = rootKey.OpenSubKey(keyPath, RegistryKeyPermissionCheck.ReadWriteSubTree, RegistryRights.TakeOwnership | RegistryRights.ReadPermissions))
                        {
                            if (hKey == null)
                            {
                                throw new InvalidOperationException($"Failed to open registry key '{keyPath}' for WRITE_OWNER.");
                            }

                            var newSecurity = hKey.GetAccessControl();
                            newSecurity.SetOwner(sid);
                            hKey.SetAccessControl(newSecurity);
                            ownershipTaken = true;
                        }
                    }
                }

                // After taking ownership, re-open with access rights for DACL modification
                hKey = rootKey.OpenSubKey(keyPath, RegistryKeyPermissionCheck.ReadWriteSubTree, RegistryRights.ChangePermissions | RegistryRights.ReadPermissions);
                if (hKey == null)
                {
                    throw new InvalidOperationException($"Failed to open registry key '{keyPath}' for DACL modification.");
                }

                try
                {
                    // 4. Execute the provided action to modify the key
                    action(hKey);
                }
                finally
                {
                    hKey?.Close();
                    hKey = null;

                    using (hKey = rootKey.OpenSubKey(keyPath, RegistryKeyPermissionCheck.ReadWriteSubTree, RegistryRights.TakeOwnership | RegistryRights.ReadPermissions))
                    {
                        if (hKey == null)
                        {
                            throw new InvalidOperationException($"Failed to open registry key '{keyPath}' for WRITE_OWNER.");
                        }

                        // 5. Restore only the original owner if we took ownership
                        if (ownershipTaken && originalOwner != null && !sid.Equals(originalOwner))
                        {
                            ConsoleExtensions.WriteIndented("Restoring original owner of the key...");
                            var ownerSecurity = hKey.GetAccessControl();
                            ownerSecurity.SetOwner(originalOwner);
                            hKey.SetAccessControl(ownerSecurity);
                        }
                    }
                }
            }
            finally
            {
                hKey?.Close();
                hKey = null;
            }
        }

        /// <summary>
        /// Impersonates the BigDriveTrustedInstaller user, writes a GUID as a value to the specified registry key, then deletes it.
        /// This tests that the user has full control over the key.
        /// </summary>
        /// <param name="password">The Password For BigDriveInstaller</param>
        /// <param name="rootKey">The root registry key (e.g., Registry.ClassesRoot).</param>
        /// <param name="keyPath">The registry key path (relative to the rootKey) to test.</param>
        private static void TestFullControlWithTrustedInstaller(SecureString password, RegistryKey rootKey, string keyPath)
        {
            ConsoleExtensions.WriteIndented($"Testing full control with BigDriveTrustedInstaller user on {keyPath}");

            string userName = UserManager.BigDriveTrustedInstallerUserName;
            string domain = Environment.MachineName;

            IntPtr userToken = IntPtr.Zero;
            WindowsImpersonationContext impersonationContext = null;

            try
            {
                // Logon as the BigDriveTrustedInstaller user
                bool loggedOn = LogonUser(
                    userName,
                    domain,
                    password.ToString(),
                    LOGON32_LOGON_INTERACTIVE,
                    LOGON32_PROVIDER_DEFAULT,
                    out userToken);

                if (!loggedOn)
                {
                    throw new Win32Exception(Marshal.GetLastWin32Error(), $"LogonUser failed for {domain}\\{userName}");
                }

                using (WindowsIdentity identity = new WindowsIdentity(userToken))
                {
                    impersonationContext = identity.Impersonate();

                    // Now, as the impersonated user, try to open, write, and delete a value
                    using (var hKey = rootKey.OpenSubKey(
                        keyPath,
                        RegistryKeyPermissionCheck.ReadWriteSubTree,
                        RegistryRights.SetValue | RegistryRights.Delete | RegistryRights.ReadKey | RegistryRights.WriteKey))
                    {
                        if (hKey == null)
                        {
                            throw new InvalidOperationException($"Failed to open registry key '{keyPath}' as {domain}\\{userName}.");
                        }

                        string valueName = "TestGuid";
                        string guidValue = Guid.NewGuid().ToString("B");
                        hKey.SetValue(valueName, guidValue, RegistryValueKind.String);

                        hKey.DeleteValue(valueName, throwOnMissingValue: false);
                    }
                }
            }
            finally
            {
                impersonationContext?.Undo();
                if (userToken != IntPtr.Zero)
                {
                    CloseHandle(userToken);
                }
            }
        }

        /// <summary>
        /// Enables a privilege for the current process token.
        /// </summary>
        /// <param name="privilege">The name of the privilege to enable (e.g., "SeTakeOwnershipPrivilege").</param>
        private static void EnablePrivilege(string privilege)
        {
            ConsoleExtensions.WriteIndented($"Enabling {privilege} privilege...");

            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, out IntPtr hToken))
            {
                throw new Win32Exception(Marshal.GetLastWin32Error(), "OpenProcessToken failed.");
            }

            try
            {
                if (!LookupPrivilegeValue(null, privilege, out LUID luid))
                {
                    throw new Win32Exception(Marshal.GetLastWin32Error(), "LookupPrivilegeValue failed.");
                }

                TOKEN_PRIVILEGES tp = new TOKEN_PRIVILEGES
                {
                    PrivilegeCount = 1,
                    Privileges = new LUID_AND_ATTRIBUTES[1]
                };
                tp.Privileges[0].Luid = luid;
                tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                if (!AdjustTokenPrivileges(hToken, false, ref tp, 0, IntPtr.Zero, IntPtr.Zero))
                {
                    throw new Win32Exception(Marshal.GetLastWin32Error(), "AdjustTokenPrivileges failed.");
                }
            }
            finally
            {
                CloseHandle(hToken);
            }
        }

        /// <summary>
        /// Gets the SecurityIdentifier for a local user account.
        /// </summary>
        /// <param name="userName">The user name.</param>
        /// <returns>SecurityIdentifier for the user.</returns>
        private static SecurityIdentifier GetUserSid(string userName)
        {
            var ntAccount = new NTAccount(Environment.MachineName, userName);
            try
            {
                return (SecurityIdentifier)ntAccount.Translate(typeof(SecurityIdentifier));
            }
            catch (IdentityNotMappedException ex)
            {
                throw new InvalidOperationException($"User '{userName}' does not exist or cannot be resolved to a SID.", ex);
            }
        }

        private const int SE_PRIVILEGE_ENABLED = 0x00000002;
        private const int TOKEN_ADJUST_PRIVILEGES = 0x0020;
        private const int TOKEN_QUERY = 0x0008;
        private const int LOGON32_LOGON_INTERACTIVE = 2;
        private const int LOGON32_PROVIDER_DEFAULT = 0;

        [StructLayout(LayoutKind.Sequential)]
        private struct LUID
        {
            public uint LowPart;
            public int HighPart;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct LUID_AND_ATTRIBUTES
        {
            public LUID Luid;
            public int Attributes;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct TOKEN_PRIVILEGES
        {
            public int PrivilegeCount;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1)]
            public LUID_AND_ATTRIBUTES[] Privileges;
        }

        [DllImport("advapi32.dll", SetLastError = true)]
        private static extern bool OpenProcessToken(IntPtr ProcessHandle, int DesiredAccess, out IntPtr TokenHandle);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetCurrentProcess();

        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern bool LookupPrivilegeValue(string lpSystemName, string lpName, out LUID lpLuid);

        [DllImport("advapi32.dll", SetLastError = true)]
        private static extern bool AdjustTokenPrivileges(
            IntPtr TokenHandle,
            bool DisableAllPrivileges,
            ref TOKEN_PRIVILEGES NewState,
            int BufferLength,
            IntPtr PreviousState,
            IntPtr ReturnLength);

        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern bool LogonUser(
            string lpszUsername,
            string lpszDomain,
            string lpszPassword,
            int dwLogonType,
            int dwLogonProvider,
            out IntPtr phToken);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(IntPtr hObject);
    }
}