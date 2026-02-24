// <copyright file="LogoutCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Threading;

    using BigDrive.ConfigProvider;

    /// <summary>
    /// Clears authentication tokens for the current drive.
    /// </summary>
    /// <remarks>
    /// Following OAuth best practices from the research document, this command:
    /// - Clears cached tokens from Windows Credential Manager
    /// - Signals the provider to invalidate any cached clients
    /// - Provides guidance on how to revoke tokens at the provider
    /// </remarks>
    public class LogoutCommand : ICommand
    {
        /// <summary>
        /// Well-known OAuth token secret key patterns.
        /// Used to identify which secrets are authentication tokens.
        /// </summary>
        private static readonly string[] CommonTokenKeys = new string[]
        {
            "OAuthAccessToken",
            "OAuthRefreshToken",
            "OAuthAccessSecret",
            "AccessToken",
            "RefreshToken",
            "AccessSecret",
            "Token",
            "OAuth"
        };

        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "logout"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "signout" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Clears authentication tokens for the current drive"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "logout [--all]"; }
        }

        /// <summary>
        /// Executes the logout command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            Guid? driveGuid = context.CurrentDriveGuid;

            if (driveGuid == null || driveGuid == Guid.Empty)
            {
                Console.WriteLine("Error: You must switch to a BigDrive first (e.g., 'cd Z:')");
                Console.WriteLine("Use 'drives' to see available drives.");
                return;
            }

            // Check for --all flag
            bool clearAll = false;
            foreach (string arg in args)
            {
                if (arg.Equals("--all", StringComparison.OrdinalIgnoreCase) ||
                    arg.Equals("-a", StringComparison.OrdinalIgnoreCase))
                {
                    clearAll = true;
                }
            }

            if (clearAll)
            {
                ClearAllSecrets(driveGuid.Value);
            }
            else
            {
                ClearTokenSecrets(driveGuid.Value);
            }
        }

        /// <summary>
        /// Clears only OAuth token secrets.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        private void ClearTokenSecrets(Guid driveGuid)
        {
            Console.WriteLine("Clearing authentication tokens...");
            Console.WriteLine();

            int clearedCount = 0;

            // Get all secrets and look for token-related ones
            List<string> secretNames = DriveManager.GetSecretNames(driveGuid, CancellationToken.None);

            foreach (string secretName in secretNames)
            {
                // Check if this looks like a token secret
                bool isTokenSecret = false;

                foreach (string tokenKey in CommonTokenKeys)
                {
                    if (secretName.IndexOf(tokenKey, StringComparison.OrdinalIgnoreCase) >= 0 ||
                        secretName.IndexOf("Token", StringComparison.OrdinalIgnoreCase) >= 0 ||
                        secretName.IndexOf("OAuth", StringComparison.OrdinalIgnoreCase) >= 0)
                    {
                        isTokenSecret = true;
                        break;
                    }
                }

                if (isTokenSecret)
                {
                    try
                    {
                        if (DriveManager.DeleteSecretProperty(driveGuid, secretName, CancellationToken.None))
                        {
                            Console.WriteLine("  Cleared: {0}", secretName);
                            clearedCount++;
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("  Error clearing {0}: {1}", secretName, ex.Message);
                    }
                }
            }

            Console.WriteLine();

            if (clearedCount > 0)
            {
                Console.WriteLine("Cleared {0} token(s).", clearedCount);
                Console.WriteLine();
                Console.WriteLine("You have been logged out of this drive.");
                Console.WriteLine("Use 'login' to authenticate again.");

                // Signal provider to invalidate cached clients
                ProviderCacheInvalidator.InvalidateDrive(driveGuid);
            }
            else
            {
                Console.WriteLine("No authentication tokens found.");
            }

            ShowRevocationGuidance();
        }

        /// <summary>
        /// Clears all secrets for the drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        private void ClearAllSecrets(Guid driveGuid)
        {
            Console.WriteLine("Clearing all secrets for this drive...");
            Console.WriteLine();

            List<string> secretNames = DriveManager.GetSecretNames(driveGuid, CancellationToken.None);
            int clearedCount = 0;

            foreach (string secretName in secretNames)
            {
                try
                {
                    if (DriveManager.DeleteSecretProperty(driveGuid, secretName, CancellationToken.None))
                    {
                        Console.WriteLine("  Cleared: {0}", secretName);
                        clearedCount++;
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine("  Error clearing {0}: {1}", secretName, ex.Message);
                }
            }

            Console.WriteLine();

            if (clearedCount > 0)
            {
                Console.WriteLine("Cleared {0} secret(s).", clearedCount);
                Console.WriteLine();
                Console.WriteLine("All credentials have been removed from this drive.");
                Console.WriteLine("You will need to reconfigure API keys and authenticate again.");

                // Signal provider to invalidate cached clients
                ProviderCacheInvalidator.InvalidateDrive(driveGuid);
            }
            else
            {
                Console.WriteLine("No secrets found for this drive.");
            }

            ShowRevocationGuidance();
        }

        /// <summary>
        /// Shows guidance on revoking tokens at the provider.
        /// </summary>
        private void ShowRevocationGuidance()
        {
            Console.WriteLine();
            Console.WriteLine("Note: Clearing local tokens does not revoke access at the provider.");
            Console.WriteLine("To fully revoke access, visit your account's authorized apps page");
            Console.WriteLine("at the provider's website.");
            Console.WriteLine();
        }
    }
}
