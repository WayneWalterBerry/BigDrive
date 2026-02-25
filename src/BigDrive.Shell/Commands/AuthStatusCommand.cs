// <copyright file="AuthStatusCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;
    using BigDrive.Interfaces;

    /// <summary>
    /// Displays authentication status for the current drive.
    /// </summary>
    /// <remarks>
    /// Shows whether the current drive is authenticated and what tokens are configured.
    /// Similar to 'gh auth status' or 'az account show'.
    /// </remarks>
    public class AuthStatusCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "authstatus"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "whoami", "status" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Shows authentication status for the current drive"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "authstatus"; }
        }

        /// <summary>
        /// Executes the authstatus command.
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

            // Get drive configuration
            DriveConfiguration driveConfig = DriveManager.ReadConfiguration(driveGuid.Value, CancellationToken.None);
            if (driveConfig == null)
            {
                Console.WriteLine("Error: Could not read drive configuration.");
                return;
            }

            // Get provider configuration
            ProviderConfiguration providerConfig = ProviderManager.ReadProvider(
                driveConfig.CLSID.ToString("B"), CancellationToken.None);

            Console.WriteLine();
            Console.WriteLine("Authentication Status");
            Console.WriteLine("=====================");
            Console.WriteLine();
            Console.WriteLine("  Drive:      {0}", driveConfig.Name ?? "Unknown");
            Console.WriteLine("  Drive ID:   {0}", driveGuid.Value);
            Console.WriteLine("  Provider:   {0}", providerConfig?.Name ?? driveConfig.CLSID.ToString());
            Console.WriteLine();

            // Check for stored credentials
            List<string> secretNames = DriveManager.GetSecretNames(driveGuid.Value, CancellationToken.None);

            if (secretNames.Count == 0)
            {
                Console.WriteLine("  Status:     Not authenticated");
                Console.WriteLine();
                Console.WriteLine("  Use 'login' to authenticate with this provider.");
            }
            else
            {
                // Categorize secrets
                bool hasApiKey = false;
                bool hasOAuthToken = false;
                bool hasRefreshToken = false;

                foreach (string secretName in secretNames)
                {
                    string lowerName = secretName.ToLowerInvariant();

                    if (lowerName.Contains("apikey") || lowerName.Contains("api_key"))
                    {
                        hasApiKey = true;
                    }

                    if (lowerName.Contains("token") || lowerName.Contains("oauth"))
                    {
                        hasOAuthToken = true;
                    }

                    if (lowerName.Contains("refresh"))
                    {
                        hasRefreshToken = true;
                    }
                }

                // Try to check with provider if authenticated
                bool? providerSaysAuthenticated = CheckWithProvider(driveGuid.Value, driveConfig);

                if (providerSaysAuthenticated == true)
                {
                    Console.WriteLine("  Status:     Authenticated");
                }
                else if (providerSaysAuthenticated == false)
                {
                    Console.WriteLine("  Status:     Token may be expired or invalid");
                }
                else if (hasOAuthToken)
                {
                    Console.WriteLine("  Status:     Tokens configured");
                }
                else if (hasApiKey)
                {
                    Console.WriteLine("  Status:     API key configured (no OAuth token)");
                }
                else
                {
                    Console.WriteLine("  Status:     Secrets configured");
                }

                Console.WriteLine();
                Console.WriteLine("  Configured secrets:");

                foreach (string secretName in secretNames)
                {
                    // Show secret name but mask the value
                    Console.WriteLine("    - {0}", secretName);
                }

                if (hasOAuthToken && hasRefreshToken)
                {
                    Console.WriteLine();
                    Console.WriteLine("  Refresh token available - session can be renewed automatically.");
                }
                else if (hasOAuthToken && !hasRefreshToken)
                {
                    Console.WriteLine();
                    Console.WriteLine("  No refresh token - you may need to re-authenticate when token expires.");
                }
            }

            Console.WriteLine();
        }

        /// <summary>
        /// Checks with the provider if the current credentials are valid.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="driveConfig">The drive configuration.</param>
        /// <returns>True if authenticated, false if not, null if unknown.</returns>
        private bool? CheckWithProvider(Guid driveGuid, DriveConfiguration driveConfig)
        {
            try
            {
                IBigDriveAuthentication authProvider = ProviderFactory.GetAuthenticationProvider(driveGuid);

                if (authProvider == null)
                {
                    return null;
                }

                int hr = authProvider.IsAuthenticated(driveGuid, out bool isAuthenticated);
                if (hr == 0)
                {
                    return isAuthenticated;
                }

                return null;
            }
            catch
            {
                return null;
            }
        }
    }
}
