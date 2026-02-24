// <copyright file="CredentialManager.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.ConfigProvider
{
    using System;
    using System.Collections.Generic;
    using System.Runtime.InteropServices;
    using System.Text;

    /// <summary>
    /// Provides methods for securely storing and retrieving credentials using Windows Credential Manager.
    /// Credentials are stored per-user and encrypted by Windows.
    /// </summary>
    public static class CredentialManager
    {
        /// <summary>
        /// The target name prefix for BigDrive credentials.
        /// </summary>
        private const string TargetPrefix = "BigDrive:";

        /// <summary>
        /// Writes a secret value to Windows Credential Manager.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="key">The secret key name.</param>
        /// <param name="value">The secret value to store.</param>
        public static void WriteSecret(Guid driveGuid, string key, string value)
        {
            if (driveGuid == Guid.Empty)
            {
                throw new ArgumentException("Drive GUID cannot be empty.", nameof(driveGuid));
            }

            if (string.IsNullOrEmpty(key))
            {
                throw new ArgumentNullException(nameof(key), "Key cannot be null or empty.");
            }

            string targetName = BuildTargetName(driveGuid, key);

            if (string.IsNullOrEmpty(value))
            {
                // Delete the credential if value is null/empty
                DeleteSecret(driveGuid, key);
                return;
            }

            byte[] credentialBlob = Encoding.Unicode.GetBytes(value);

            var credential = new CREDENTIAL
            {
                Type = CRED_TYPE_GENERIC,
                TargetName = targetName,
                CredentialBlobSize = (uint)credentialBlob.Length,
                CredentialBlob = Marshal.AllocHGlobal(credentialBlob.Length),
                Persist = CRED_PERSIST_LOCAL_MACHINE,
                UserName = $"BigDrive\\{driveGuid:N}"
            };

            try
            {
                Marshal.Copy(credentialBlob, 0, credential.CredentialBlob, credentialBlob.Length);

                if (!CredWrite(ref credential, 0))
                {
                    int error = Marshal.GetLastWin32Error();
                    throw new InvalidOperationException($"Failed to write credential. Error code: {error}");
                }
            }
            finally
            {
                if (credential.CredentialBlob != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(credential.CredentialBlob);
                }
            }
        }

        /// <summary>
        /// Reads a secret value from Windows Credential Manager.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="key">The secret key name.</param>
        /// <returns>The secret value, or null if not found.</returns>
        public static string ReadSecret(Guid driveGuid, string key)
        {
            if (driveGuid == Guid.Empty)
            {
                throw new ArgumentException("Drive GUID cannot be empty.", nameof(driveGuid));
            }

            if (string.IsNullOrEmpty(key))
            {
                throw new ArgumentNullException(nameof(key), "Key cannot be null or empty.");
            }

            string targetName = BuildTargetName(driveGuid, key);
            IntPtr credentialPtr = IntPtr.Zero;

            try
            {
                if (!CredRead(targetName, CRED_TYPE_GENERIC, 0, out credentialPtr))
                {
                    int error = Marshal.GetLastWin32Error();
                    if (error == ERROR_NOT_FOUND)
                    {
                        return null;
                    }

                    throw new InvalidOperationException($"Failed to read credential. Error code: {error}");
                }

                var credential = Marshal.PtrToStructure<CREDENTIAL>(credentialPtr);

                if (credential.CredentialBlob == IntPtr.Zero || credential.CredentialBlobSize == 0)
                {
                    return null;
                }

                byte[] credentialBlob = new byte[credential.CredentialBlobSize];
                Marshal.Copy(credential.CredentialBlob, credentialBlob, 0, (int)credential.CredentialBlobSize);

                return Encoding.Unicode.GetString(credentialBlob);
            }
            finally
            {
                if (credentialPtr != IntPtr.Zero)
                {
                    CredFree(credentialPtr);
                }
            }
        }

        /// <summary>
        /// Deletes a secret from Windows Credential Manager.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="key">The secret key name.</param>
        /// <returns>True if the credential was deleted, false if it didn't exist.</returns>
        public static bool DeleteSecret(Guid driveGuid, string key)
        {
            if (driveGuid == Guid.Empty)
            {
                throw new ArgumentException("Drive GUID cannot be empty.", nameof(driveGuid));
            }

            if (string.IsNullOrEmpty(key))
            {
                throw new ArgumentNullException(nameof(key), "Key cannot be null or empty.");
            }

            string targetName = BuildTargetName(driveGuid, key);

            if (!CredDelete(targetName, CRED_TYPE_GENERIC, 0))
            {
                int error = Marshal.GetLastWin32Error();
                if (error == ERROR_NOT_FOUND)
                {
                    return false;
                }

                throw new InvalidOperationException($"Failed to delete credential. Error code: {error}");
            }

            return true;
        }

        /// <summary>
        /// Deletes all secrets for a drive from Windows Credential Manager.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        public static void DeleteAllSecretsForDrive(Guid driveGuid)
        {
            if (driveGuid == Guid.Empty)
            {
                throw new ArgumentException("Drive GUID cannot be empty.", nameof(driveGuid));
            }

            string filterPattern = $"{TargetPrefix}{driveGuid:B}:*";
            IntPtr credentialsPtr = IntPtr.Zero;
            uint count = 0;

            try
            {
                if (!CredEnumerate(filterPattern, 0, out count, out credentialsPtr))
                {
                    int error = Marshal.GetLastWin32Error();
                    if (error == ERROR_NOT_FOUND)
                    {
                        return; // No credentials to delete
                    }

                    throw new InvalidOperationException($"Failed to enumerate credentials. Error code: {error}");
                }

                for (int i = 0; i < count; i++)
                {
                    IntPtr credPtr = Marshal.ReadIntPtr(credentialsPtr, i * IntPtr.Size);
                    var credential = Marshal.PtrToStructure<CREDENTIAL>(credPtr);
                    CredDelete(credential.TargetName, CRED_TYPE_GENERIC, 0);
                }
            }
            finally
            {
                if (credentialsPtr != IntPtr.Zero)
                {
                    CredFree(credentialsPtr);
                }
            }
        }

        /// <summary>
        /// Gets all secret key names for a drive from Windows Credential Manager.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>A list of secret key names (without the BigDrive prefix).</returns>
        public static List<string> GetSecretNames(Guid driveGuid)
        {
            if (driveGuid == Guid.Empty)
            {
                throw new ArgumentException("Drive GUID cannot be empty.", nameof(driveGuid));
            }

            var secretNames = new List<string>();
            string filterPattern = $"{TargetPrefix}{driveGuid:B}:*";
            string prefixToRemove = $"{TargetPrefix}{driveGuid:B}:";
            IntPtr credentialsPtr = IntPtr.Zero;
            uint count = 0;

            try
            {
                if (!CredEnumerate(filterPattern, 0, out count, out credentialsPtr))
                {
                    int error = Marshal.GetLastWin32Error();
                    if (error == ERROR_NOT_FOUND)
                    {
                        return secretNames; // No credentials found
                    }

                    throw new InvalidOperationException($"Failed to enumerate credentials. Error code: {error}");
                }

                for (int i = 0; i < count; i++)
                {
                    IntPtr credPtr = Marshal.ReadIntPtr(credentialsPtr, i * IntPtr.Size);
                    var credential = Marshal.PtrToStructure<CREDENTIAL>(credPtr);

                    // Extract the key name from the target name
                    if (credential.TargetName != null && credential.TargetName.StartsWith(prefixToRemove))
                    {
                        string keyName = credential.TargetName.Substring(prefixToRemove.Length);
                        secretNames.Add(keyName);
                    }
                }
            }
            finally
            {
                if (credentialsPtr != IntPtr.Zero)
                {
                    CredFree(credentialsPtr);
                }
            }

            return secretNames;
        }

        /// <summary>
        /// Checks if a secret exists in Windows Credential Manager.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="key">The secret key name.</param>
        /// <returns>True if the secret exists, false otherwise.</returns>
        public static bool SecretExists(Guid driveGuid, string key)
        {
            return ReadSecret(driveGuid, key) != null;
        }

        /// <summary>
        /// Builds the target name for a credential.
        /// Format: "BigDrive:{DriveGuid}:{Key}"
        /// </summary>
        private static string BuildTargetName(Guid driveGuid, string key)
        {
            return $"{TargetPrefix}{driveGuid:B}:{key}";
        }

        #region P/Invoke Declarations

        private const int CRED_TYPE_GENERIC = 1;
        private const int CRED_PERSIST_LOCAL_MACHINE = 2;
        private const int ERROR_NOT_FOUND = 1168;

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        private struct CREDENTIAL
        {
            public uint Flags;
            public int Type;
            public string TargetName;
            public string Comment;
            public System.Runtime.InteropServices.ComTypes.FILETIME LastWritten;
            public uint CredentialBlobSize;
            public IntPtr CredentialBlob;
            public uint Persist;
            public uint AttributeCount;
            public IntPtr Attributes;
            public string TargetAlias;
            public string UserName;
        }

        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern bool CredWrite(ref CREDENTIAL credential, uint flags);

        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern bool CredRead(string targetName, int type, uint flags, out IntPtr credential);

        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern bool CredDelete(string targetName, int type, uint flags);

        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern bool CredEnumerate(string filter, uint flags, out uint count, out IntPtr credentials);

        [DllImport("advapi32.dll", SetLastError = true)]
        private static extern void CredFree(IntPtr credential);

        #endregion
    }
}
