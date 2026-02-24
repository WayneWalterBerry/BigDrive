// <copyright file="IBigDriveAuthentication.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Describes the OAuth authentication requirements for a BigDrive provider.
    /// </summary>
    /// <remarks>
    /// Providers implement this interface to enable the BigDrive Shell to perform
    /// OAuth authentication on their behalf. The shell will:
    /// 1. Query the provider for auth requirements via GetAuthenticationInfo
    /// 2. Perform OAuth flow (browser redirect or device code)
    /// 3. Store obtained tokens securely in Windows Credential Manager
    /// 4. The provider retrieves tokens from Credential Manager when needed
    /// 
    /// This follows patterns used by Azure CLI, GitHub CLI, and other modern CLIs
    /// as documented in OAuth Authentication Patterns research.
    /// </remarks>
    [ComVisible(true)]
    [Guid("7E8F9A0B-1C2D-3E4F-5A6B-7C8D9E0F1A2B")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveAuthentication
    {
        /// <summary>
        /// Gets the authentication requirements for this provider.
        /// </summary>
        /// <param name="driveGuid">The drive GUID to authenticate.</param>
        /// <param name="authInfo">Receives the authentication information.</param>
        /// <returns>S_OK if successful, E_NOTIMPL if auth not required.</returns>
        [PreserveSig]
        int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo);

        /// <summary>
        /// Called after successful OAuth to allow provider to validate/transform tokens.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="accessToken">The OAuth access token.</param>
        /// <param name="refreshToken">The OAuth refresh token (may be null).</param>
        /// <param name="expiresIn">Token expiration in seconds (0 if no expiration).</param>
        /// <returns>S_OK if tokens are valid.</returns>
        [PreserveSig]
        int OnAuthenticationComplete(Guid driveGuid, string accessToken, string refreshToken, int expiresIn);

        /// <summary>
        /// Checks if the current drive has valid authentication.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="isAuthenticated">Receives true if authenticated.</param>
        /// <returns>S_OK if check completed.</returns>
        [PreserveSig]
        int IsAuthenticated(Guid driveGuid, out bool isAuthenticated);
    }

    /// <summary>
    /// Describes the OAuth authentication requirements for a provider.
    /// </summary>
    [ComVisible(true)]
    [Guid("8F9A0B1C-2D3E-4F5A-6B7C-8D9E0F1A2B3C")]
    [StructLayout(LayoutKind.Sequential)]
    public struct AuthenticationInfo
    {
        /// <summary>
        /// The OAuth authorization endpoint URL.
        /// </summary>
        [MarshalAs(UnmanagedType.BStr)]
        public string AuthorizationUrl;

        /// <summary>
        /// The OAuth token endpoint URL.
        /// </summary>
        [MarshalAs(UnmanagedType.BStr)]
        public string TokenUrl;

        /// <summary>
        /// The OAuth client ID.
        /// </summary>
        [MarshalAs(UnmanagedType.BStr)]
        public string ClientId;

        /// <summary>
        /// The OAuth client secret (may be empty for public clients).
        /// </summary>
        [MarshalAs(UnmanagedType.BStr)]
        public string ClientSecret;

        /// <summary>
        /// The OAuth scopes to request (space-separated).
        /// </summary>
        [MarshalAs(UnmanagedType.BStr)]
        public string Scopes;

        /// <summary>
        /// The device authorization endpoint URL (for device code flow).
        /// Empty if device code flow is not supported.
        /// </summary>
        [MarshalAs(UnmanagedType.BStr)]
        public string DeviceAuthorizationUrl;

        /// <summary>
        /// The friendly name of the authentication provider (e.g., "Flickr", "OneDrive").
        /// </summary>
        [MarshalAs(UnmanagedType.BStr)]
        public string ProviderName;

        /// <summary>
        /// The OAuth flow type supported by this provider.
        /// </summary>
        public OAuthFlowType FlowType;

        /// <summary>
        /// The names of the secret keys where tokens should be stored.
        /// Format: "AccessTokenKey;RefreshTokenKey;AccessSecretKey" (semicolon-separated).
        /// </summary>
        [MarshalAs(UnmanagedType.BStr)]
        public string SecretKeyNames;
    }

    /// <summary>
    /// The type of OAuth flow supported by a provider.
    /// </summary>
    [ComVisible(true)]
    public enum OAuthFlowType
    {
        /// <summary>
        /// Standard OAuth 2.0 Authorization Code flow with browser redirect.
        /// </summary>
        AuthorizationCode = 0,

        /// <summary>
        /// OAuth 2.0 Device Authorization Grant (RFC 8628).
        /// </summary>
        DeviceCode = 1,

        /// <summary>
        /// OAuth 1.0a flow (used by Flickr).
        /// </summary>
        OAuth1 = 2,

        /// <summary>
        /// Authorization Code with PKCE (recommended for public clients).
        /// </summary>
        AuthorizationCodePKCE = 3
    }
}
