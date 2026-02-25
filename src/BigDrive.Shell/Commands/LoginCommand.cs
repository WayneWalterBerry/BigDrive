// <copyright file="LoginCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;
    using BigDrive.Interfaces;

    /// <summary>
    /// Handles OAuth authentication for BigDrive providers.
    /// </summary>
    /// <remarks>
    /// Implements authentication patterns from the OAuth Authentication Patterns research:
    /// - Authorization Code flow with loopback (default) for seamless browser-based auth
    /// - Device Code flow (--device-code) for headless environments
    /// - OAuth 1.0a for legacy providers like Flickr
    /// 
    /// Tokens are stored securely in Windows Credential Manager via the secret command infrastructure.
    /// </remarks>
    public class LoginCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "login"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "auth", "authenticate" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Authenticates with the current drive's provider"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "login [--device-code]"; }
        }

        /// <summary>
        /// Executes the login command.
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

            // Check for --device-code flag
            bool useDeviceCode = false;
            foreach (string arg in args)
            {
                if (arg.Equals("--device-code", StringComparison.OrdinalIgnoreCase) ||
                    arg.Equals("-d", StringComparison.OrdinalIgnoreCase))
                {
                    useDeviceCode = true;
                }
            }

            // Get drive configuration to determine provider
            DriveConfiguration driveConfig = DriveManager.ReadConfiguration(driveGuid.Value, CancellationToken.None);
            if (driveConfig == null)
            {
                Console.WriteLine("Error: Could not read drive configuration.");
                return;
            }

            // Try to get authentication interface from provider
            IBigDriveAuthentication authProvider = GetAuthenticationProvider(driveGuid.Value);

            if (authProvider != null)
            {
                // Provider supports IBigDriveAuthentication - use generic flow
                PerformGenericAuth(driveGuid.Value, authProvider, useDeviceCode);
            }
            else
            {
                // Fall back to provider-specific handling
                PerformProviderSpecificAuth(driveGuid.Value, driveConfig, useDeviceCode);
            }
        }

        /// <summary>
        /// Gets the authentication provider interface for a drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The authentication interface, or null if not supported.</returns>
        private IBigDriveAuthentication GetAuthenticationProvider(Guid driveGuid)
        {
            try
            {
                object provider = GetProviderInstance(driveGuid);
                return provider as IBigDriveAuthentication;
            }
            catch
            {
                return null;
            }
        }

        /// <summary>
        /// Gets the provider COM object for a drive using out-of-process activation.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The provider instance, or null if not found.</returns>
        private object GetProviderInstance(Guid driveGuid)
        {
            try
            {
                return ProviderFactory.GetProviderInstance(driveGuid);
            }
            catch
            {
                return null;
            }
        }

        /// <summary>
        /// Performs authentication using the generic IBigDriveAuthentication interface.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="authProvider">The authentication provider.</param>
        /// <param name="useDeviceCode">Whether to use device code flow.</param>
        private void PerformGenericAuth(Guid driveGuid, IBigDriveAuthentication authProvider, bool useDeviceCode)
        {
            int hr = authProvider.GetAuthenticationInfo(driveGuid, out AuthenticationInfo authInfo);

            if (hr != 0)
            {
                Console.WriteLine("This provider does not require authentication.");
                return;
            }

            OAuthResult result = null;
            CancellationTokenSource cts = new CancellationTokenSource();

            Console.CancelKeyPress += (s, e) =>
            {
                e.Cancel = true;
                cts.Cancel();
            };

            switch (authInfo.FlowType)
            {
                case OAuthFlowType.OAuth1:
                    result = PerformOAuth1Auth(authInfo, cts.Token);
                    break;

                case OAuthFlowType.DeviceCode:
                    result = OAuthHelper.PerformDeviceCodeFlow(
                        authInfo.DeviceAuthorizationUrl,
                        authInfo.TokenUrl,
                        authInfo.ClientId,
                        authInfo.Scopes,
                        authInfo.ProviderName,
                        cts.Token);
                    break;

                case OAuthFlowType.AuthorizationCode:
                case OAuthFlowType.AuthorizationCodePKCE:
                default:
                    if (useDeviceCode && !string.IsNullOrEmpty(authInfo.DeviceAuthorizationUrl))
                    {
                        result = OAuthHelper.PerformDeviceCodeFlow(
                            authInfo.DeviceAuthorizationUrl,
                            authInfo.TokenUrl,
                            authInfo.ClientId,
                            authInfo.Scopes,
                            authInfo.ProviderName,
                            cts.Token);
                    }
                    else
                    {
                        result = OAuthHelper.PerformAuthorizationCodeFlow(
                            authInfo.AuthorizationUrl,
                            authInfo.TokenUrl,
                            authInfo.ClientId,
                            authInfo.ClientSecret,
                            authInfo.Scopes,
                            authInfo.ProviderName,
                            cts.Token);
                    }
                    break;
            }

            if (result != null)
            {
                // Store tokens securely
                StoreTokens(driveGuid, authInfo, result);

                // Notify provider
                authProvider.OnAuthenticationComplete(
                    driveGuid,
                    result.AccessToken,
                    result.RefreshToken,
                    result.ExpiresIn);
            }
        }

        /// <summary>
        /// Performs OAuth 1.0a authentication.
        /// </summary>
        private OAuthResult PerformOAuth1Auth(AuthenticationInfo authInfo, CancellationToken cancellationToken)
        {
            // For OAuth 1.0a, AuthorizationUrl contains the authorize URL
            // TokenUrl contains the request token URL
            // DeviceAuthorizationUrl (if set) contains the access token URL

            // Parse URLs from authInfo - OAuth 1.0a providers need to pack URLs differently
            // Format: "requestTokenUrl|authorizeUrl|accessTokenUrl"
            string[] urls = authInfo.AuthorizationUrl.Split('|');
            if (urls.Length < 3)
            {
                Console.WriteLine("Invalid OAuth 1.0a configuration.");
                return null;
            }

            return OAuth1Helper.PerformOAuth1Flow(
                urls[0], // Request token URL
                urls[1], // Authorize URL
                urls[2], // Access token URL
                authInfo.ClientId, // Consumer key
                authInfo.ClientSecret, // Consumer secret
                authInfo.ProviderName,
                cancellationToken);
        }

        /// <summary>
        /// Stores OAuth tokens securely using Windows Credential Manager.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="authInfo">The authentication info.</param>
        /// <param name="result">The OAuth result.</param>
        private void StoreTokens(Guid driveGuid, AuthenticationInfo authInfo, OAuthResult result)
        {
            try
            {
                // Parse secret key names from authInfo
                // Format: "AccessTokenKey;RefreshTokenKey;AccessSecretKey"
                string[] keyNames = (authInfo.SecretKeyNames ?? "OAuthAccessToken;OAuthRefreshToken;OAuthAccessSecret")
                    .Split(';');

                string accessTokenKey = keyNames.Length > 0 ? keyNames[0] : "OAuthAccessToken";
                string refreshTokenKey = keyNames.Length > 1 ? keyNames[1] : "OAuthRefreshToken";
                string accessSecretKey = keyNames.Length > 2 ? keyNames[2] : "OAuthAccessSecret";

                // Store access token
                if (!string.IsNullOrEmpty(result.AccessToken))
                {
                    DriveManager.WriteSecretProperty(driveGuid, accessTokenKey, result.AccessToken, CancellationToken.None);
                }

                // Store refresh token (OAuth 2.0)
                if (!string.IsNullOrEmpty(result.RefreshToken))
                {
                    DriveManager.WriteSecretProperty(driveGuid, refreshTokenKey, result.RefreshToken, CancellationToken.None);
                }

                // Store access token secret (OAuth 1.0a)
                if (!string.IsNullOrEmpty(result.AccessTokenSecret))
                {
                    DriveManager.WriteSecretProperty(driveGuid, accessSecretKey, result.AccessTokenSecret, CancellationToken.None);
                }

                Console.WriteLine();
                Console.WriteLine("Tokens saved securely to Windows Credential Manager.");
            }
            catch (Exception ex)
            {
                Console.WriteLine("Warning: Could not save tokens: {0}", ex.Message);
                Console.WriteLine("You may need to authenticate again next time.");
            }
        }

        /// <summary>
        /// Performs provider-specific authentication for providers that don't support IBigDriveAuthentication.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="driveConfig">The drive configuration.</param>
        /// <param name="useDeviceCode">Whether to use device code flow.</param>
        private void PerformProviderSpecificAuth(Guid driveGuid, DriveConfiguration driveConfig, bool useDeviceCode)
        {
            // Provider doesn't implement IBigDriveAuthentication
            // Guide user to configure secrets manually
            Console.WriteLine("This provider does not support automatic authentication through BigDrive Shell.");
            Console.WriteLine();
            Console.WriteLine("The provider may require manual configuration of secrets.");
            Console.WriteLine("Use 'secret set <key> <value>' to configure credentials.");
            Console.WriteLine();
            Console.WriteLine("Example:");
            Console.WriteLine("  secret set ApiKey <your-api-key>");
            Console.WriteLine("  secret set ApiSecret <your-api-secret>");
            Console.WriteLine("  secret set OAuthAccessToken <your-token>");
            Console.WriteLine();
            Console.WriteLine("Check the provider's documentation for required secret keys.");
        }
    }

    /// <summary>
    /// Helper to invalidate provider client cache when tokens change.
    /// </summary>
    internal static class ProviderCacheInvalidator
    {
        /// <summary>
        /// Signals to the provider that authentication has changed.
        /// Providers can check for this value to invalidate cached clients.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        public static void InvalidateDrive(Guid driveGuid)
        {
            // We signal cache invalidation by writing a timestamp to secrets
            // The provider will check this and invalidate its cache
            try
            {
                DriveManager.WriteSecretProperty(driveGuid, "LastAuthUpdate", DateTime.UtcNow.Ticks.ToString(), CancellationToken.None);
            }
            catch
            {
                // Ignore - this is just a hint to the provider
            }
        }
    }
}
