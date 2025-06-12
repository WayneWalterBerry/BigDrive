// <copyright file="UserManager.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Setup
{
    using System;
    using System.DirectoryServices.AccountManagement;
    using System.Runtime.InteropServices;
    using System.Security;

    internal static class UserManager
    {
        public const string BigDriveTrustedInstallerUserName = "BigDriveInstaller";

        /// <summary>
        /// Creates a local user account named "BigDriveTrustedInstaller" on the local machine.
        /// </summary>
        /// <returns>
        /// True if the user was created successfully or already exists; otherwise, false.
        /// </returns>
        public static SecureString CreateBigDriveTrustedInstaller(string userName = BigDriveTrustedInstallerUserName)
        {
            // Set a random strong password or a known one as per your security policy
            SecureString password = GenerateStrongPassword();

            if (password == null || password.Length == 0)
            {
                throw new InvalidOperationException("Password generation failed.");
            }

            // Use System.DirectoryServices.AccountManagement for user management
            using (var context = new PrincipalContext(ContextType.Machine))
            {
                UserPrincipal user = default(UserPrincipal);

                try
                {
                    user = UserPrincipal.FindByIdentity(context, IdentityType.SamAccountName, userName);
                    if (user != null)
                    {
                        throw new InvalidOperationException("User already exists. Please delete the user before creating a new one.");
                    }
                }
                catch (COMException comException) when (comException.Message == "The user name could not be found.")
                {
                }

                if (user == default(UserPrincipal))
                {
                    user = new UserPrincipal(context)
                    {
                        Name = userName,
                        Description = "BigDrive Trusted Installer Account",
                        PasswordNeverExpires = true,
                        UserCannotChangePassword = true
                    };

                    user.SetPassword(password.ToString());
                    user.Save();
                }
            }

            return password;
        }

        public static void DeleteBigDriveTrustedInstaller(string userName = BigDriveTrustedInstallerUserName)
        {
            using (var context = new PrincipalContext(ContextType.Machine))
            {
                var user = UserPrincipal.FindByIdentity(context, IdentityType.SamAccountName, userName);
                if (user != null)
                {
                    user.Delete();
                    ConsoleExtensions.WriteIndented($"User {userName} deleted successfully.");
                }
                else
                {
                    ConsoleExtensions.WriteIndented($"User {userName} does not exist.");
                }
            }
        }

        /// <summary>
        /// Checks if a local user account with the specified username exists on the local machine.
        /// </summary>
        /// <param name="userName">The name of the user to check for existence.</param>
        /// <returns>True if the user exists; otherwise, false.</returns>
        public static bool UserExists(string userName = BigDriveTrustedInstallerUserName)
        {
            using (var context = new PrincipalContext(ContextType.Machine))
            {
                try
                {
                    var user = UserPrincipal.FindByIdentity(context, IdentityType.SamAccountName, userName);
                    return user != null;
                }
                catch (COMException comException) when (comException.Message == "The user name could not be found.")
                {
                    return false;
                }

            }
        }

        /// <summary>
        /// Generates a strong, random password that meets Windows complexity requirements.
        /// </summary>
        /// <param name="length">The total length of the password (minimum 12 recommended).</param>
        /// <returns>A SecureString containing the generated password.</returns>
        public static SecureString GenerateStrongPassword(int length = 16)
        {
            if (length < 8)
            {
                throw new ArgumentException("Password length must be at least 8 characters.", nameof(length));
            }

            const string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            const string lower = "abcdefghijklmnopqrstuvwxyz";
            const string digits = "0123456789";
            const string special = "!@#$%^&*()-_=+[]{};:,.<>?";
            string all = upper + lower + digits + special;

            var random = new Random();
            char[] password = new char[length];

            // Ensure at least one character from each required set
            password[0] = upper[random.Next(upper.Length)];
            password[1] = lower[random.Next(lower.Length)];
            password[2] = digits[random.Next(digits.Length)];
            password[3] = special[random.Next(special.Length)];

            // Fill the rest with random characters from all sets
            for (int i = 4; i < length; i++)
            {
                password[i] = all[random.Next(all.Length)];
            }

            // Shuffle the password to avoid predictable placement
            for (int i = password.Length - 1; i > 0; i--)
            {
                int j = random.Next(i + 1);
                var temp = password[i];
                password[i] = password[j];
                password[j] = temp;
            }

            var secure = new SecureString();
            foreach (char c in password)
                secure.AppendChar(c);
            secure.MakeReadOnly();
            return secure;
        }
    }
}
