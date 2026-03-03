// <copyright file="Provider.IBigDriveAuthentication.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Flickr
{
    using System;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using BigDrive.Interfaces;

    /// <summary>
    /// Implementation of <see cref="IBigDriveAuthentication"/> for the Flickr provider.
    /// </summary>
    /// <remarks>
    /// Flickr uses OAuth 1.0a authentication. This implementation provides the
    /// authentication metadata to the BigDrive Shell so it can perform the OAuth flow
    /// and store tokens securely in Windows Credential Manager.
    /// </remarks>
    public partial class Provider
    {
        /// <summary>
        /// Flickr OAuth 1.0a request token URL.
        /// </summary>
        private const string FlickrRequestTokenUrl = "https://www.flickr.com/services/oauth/request_token";

        /// <summary>
        /// Flickr OAuth 1.0a authorization URL.
        /// </summary>
        private const string FlickrAuthorizeUrl = "https://www.flickr.com/services/oauth/authorize";

        /// <summary>
        /// Flickr OAuth 1.0a access token URL.
        /// </summary>
        private const string FlickrAccessTokenUrl = "https://www.flickr.com/services/oauth/access_token";

        /// <inheritdoc/>
        public int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo)
        {
            DefaultTraceSource.TraceInformation($"GetAuthenticationInfo: driveGuid={driveGuid}");

            authInfo = new AuthenticationInfo();

            // Get API credentials from secrets
            string apiKey = GetDriveSecret(driveGuid, "FlickrApiKey");
            string apiSecret = GetDriveSecret(driveGuid, "FlickrApiSecret");

            if (string.IsNullOrEmpty(apiKey) || string.IsNullOrEmpty(apiSecret))
            {
                DefaultTraceSource.TraceInformation("GetAuthenticationInfo: API credentials not configured.");
                // Return S_OK but with empty auth info - shell will prompt for API key setup
                authInfo.ProviderName = "Flickr";
                authInfo.FlowType = OAuthFlowType.OAuth1;
                return 0; // S_OK
            }

            // For OAuth 1.0a, we pack URLs as "requestTokenUrl|authorizeUrl|accessTokenUrl"
            authInfo.AuthorizationUrl = string.Format("{0}|{1}|{2}",
                FlickrRequestTokenUrl,
                FlickrAuthorizeUrl,
                FlickrAccessTokenUrl);

            authInfo.TokenUrl = FlickrAccessTokenUrl;
            authInfo.ClientId = apiKey;
            authInfo.ClientSecret = apiSecret;
            authInfo.Scopes = string.Empty; // OAuth 1.0a doesn't use scopes
            authInfo.DeviceAuthorizationUrl = string.Empty; // No device code for OAuth 1.0a
            authInfo.ProviderName = "Flickr";
            authInfo.FlowType = OAuthFlowType.OAuth1;
            authInfo.SecretKeyNames = "FlickrOAuthToken;FlickrRefreshToken;FlickrOAuthSecret";

            return 0; // S_OK
        }

        /// <inheritdoc/>
        public int OnAuthenticationComplete(Guid driveGuid, string accessToken, string refreshToken, int expiresIn)
        {
            DefaultTraceSource.TraceInformation($"OnAuthenticationComplete: driveGuid={driveGuid}");

            // Invalidate any cached Flickr client for this drive
            FlickrClientWrapper.InvalidateDrive(driveGuid);

            DefaultTraceSource.TraceInformation("OnAuthenticationComplete: Flickr client cache invalidated.");
            return 0; // S_OK
        }

        /// <inheritdoc/>
        public int IsAuthenticated(Guid driveGuid, out bool isAuthenticated)
        {
            isAuthenticated = false;

            try
            {
                // Check if OAuth tokens are present
                string oauthToken = GetDriveSecret(driveGuid, "FlickrOAuthToken");
                string oauthSecret = GetDriveSecret(driveGuid, "FlickrOAuthSecret");

                if (string.IsNullOrEmpty(oauthToken) || string.IsNullOrEmpty(oauthSecret))
                {
                    DefaultTraceSource.TraceInformation($"IsAuthenticated: No OAuth tokens for drive {driveGuid}");
                    return 0; // S_OK - not authenticated
                }

                // Optionally, we could verify the token by making a test API call
                // For now, we just check if tokens exist
                isAuthenticated = true;

                DefaultTraceSource.TraceInformation($"IsAuthenticated: Drive {driveGuid} has OAuth tokens.");
                return 0; // S_OK
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"IsAuthenticated: Error checking auth status: {ex.Message}");
                return -1; // E_FAIL
            }
        }

        /// <summary>
        /// Reads a secret or configuration value for a drive with the following priority:
        /// 1. Windows Credential Manager (secure storage for secrets)
        /// 2. Registry drive-specific property
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="secretName">The secret or property name.</param>
        /// <returns>The value, or null if not found in either location.</returns>
        private static string GetDriveSecret(Guid driveGuid, string secretName)
        {
            // First, try Credential Manager (secure storage)
            try
            {
                string secret = DriveManager.ReadSecretProperty(driveGuid, secretName, CancellationToken.None);
                if (!string.IsNullOrEmpty(secret))
                {
                    return secret;
                }
            }
            catch
            {
                // Credential Manager may not be available or no secret stored
            }

            // Fall back to registry drive property
            try
            {
                string property = DriveManager.ReadDriveProperty(driveGuid, secretName, CancellationToken.None);
                if (!string.IsNullOrEmpty(property))
                {
                    return property;
                }
            }
            catch
            {
                // Registry property not found
            }

            return null;
        }
    }
}
