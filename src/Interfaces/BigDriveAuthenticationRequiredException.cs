// <copyright file="BigDriveAuthenticationRequiredException.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;
    using System.Runtime.Serialization;

    /// <summary>
    /// Exception thrown by providers when an operation fails due to missing or invalid authentication.
    /// </summary>
    /// <remarks>
    /// Providers should catch their service-specific authentication exceptions and throw this
    /// generic exception instead. BigDrive.Shell catches this exception and automatically
    /// initiates the OAuth login flow, providing a seamless user experience.
    /// 
    /// This follows patterns from Azure CLI and GitHub CLI where authentication is triggered
    /// automatically when needed, rather than requiring explicit login commands.
    /// 
    /// Example provider implementation:
    /// <code>
    /// try
    /// {
    ///     return _flickr.PhotosetsGetList();
    /// }
    /// catch (FlickrNet.OAuthException ex)
    /// {
    ///     throw new BigDriveAuthenticationRequiredException(
    ///         driveGuid,
    ///         "Flickr",
    ///         "OAuth token is invalid or expired.",
    ///         ex);
    /// }
    /// </code>
    /// </remarks>
    [ComVisible(true)]
    [Serializable]
    public class BigDriveAuthenticationRequiredException : Exception
    {
        /// <summary>
        /// The default error message.
        /// </summary>
        private const string DefaultMessage = "Authentication is required to complete this operation.";

        /// <summary>
        /// Initializes a new instance of the <see cref="BigDriveAuthenticationRequiredException"/> class.
        /// </summary>
        public BigDriveAuthenticationRequiredException()
            : base(DefaultMessage)
        {
            DriveGuid = Guid.Empty;
            ProviderName = string.Empty;
            Reason = AuthenticationFailureReason.Unknown;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BigDriveAuthenticationRequiredException"/> class.
        /// </summary>
        /// <param name="message">The error message.</param>
        public BigDriveAuthenticationRequiredException(string message)
            : base(message)
        {
            DriveGuid = Guid.Empty;
            ProviderName = string.Empty;
            Reason = AuthenticationFailureReason.Unknown;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BigDriveAuthenticationRequiredException"/> class.
        /// </summary>
        /// <param name="message">The error message.</param>
        /// <param name="innerException">The inner exception.</param>
        public BigDriveAuthenticationRequiredException(string message, Exception innerException)
            : base(message, innerException)
        {
            DriveGuid = Guid.Empty;
            ProviderName = string.Empty;
            Reason = AuthenticationFailureReason.Unknown;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BigDriveAuthenticationRequiredException"/> class
        /// with drive-specific information.
        /// </summary>
        /// <param name="driveGuid">The drive GUID that requires authentication.</param>
        /// <param name="providerName">The friendly name of the provider (e.g., "Flickr").</param>
        /// <param name="reason">The reason authentication failed.</param>
        public BigDriveAuthenticationRequiredException(
            Guid driveGuid,
            string providerName,
            AuthenticationFailureReason reason)
            : base(GetMessageForReason(providerName, reason))
        {
            DriveGuid = driveGuid;
            ProviderName = providerName ?? string.Empty;
            Reason = reason;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BigDriveAuthenticationRequiredException"/> class
        /// with drive-specific information and a custom message.
        /// </summary>
        /// <param name="driveGuid">The drive GUID that requires authentication.</param>
        /// <param name="providerName">The friendly name of the provider (e.g., "Flickr").</param>
        /// <param name="message">The error message.</param>
        /// <param name="innerException">The inner exception from the provider's SDK.</param>
        public BigDriveAuthenticationRequiredException(
            Guid driveGuid,
            string providerName,
            string message,
            Exception innerException)
            : base(message, innerException)
        {
            DriveGuid = driveGuid;
            ProviderName = providerName ?? string.Empty;
            Reason = AuthenticationFailureReason.Unknown;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BigDriveAuthenticationRequiredException"/> class
        /// with full details.
        /// </summary>
        /// <param name="driveGuid">The drive GUID that requires authentication.</param>
        /// <param name="providerName">The friendly name of the provider (e.g., "Flickr").</param>
        /// <param name="reason">The reason authentication failed.</param>
        /// <param name="innerException">The inner exception from the provider's SDK.</param>
        public BigDriveAuthenticationRequiredException(
            Guid driveGuid,
            string providerName,
            AuthenticationFailureReason reason,
            Exception innerException)
            : base(GetMessageForReason(providerName, reason), innerException)
        {
            DriveGuid = driveGuid;
            ProviderName = providerName ?? string.Empty;
            Reason = reason;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BigDriveAuthenticationRequiredException"/> class
        /// with serialized data.
        /// </summary>
        /// <param name="info">The serialization info.</param>
        /// <param name="context">The streaming context.</param>
        protected BigDriveAuthenticationRequiredException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
            DriveGuid = (Guid)info.GetValue("DriveGuid", typeof(Guid));
            ProviderName = info.GetString("ProviderName");
            Reason = (AuthenticationFailureReason)info.GetInt32("Reason");
        }

        /// <summary>
        /// Gets the drive GUID that requires authentication.
        /// </summary>
        public Guid DriveGuid { get; }

        /// <summary>
        /// Gets the friendly name of the provider (e.g., "Flickr", "OneDrive").
        /// </summary>
        public string ProviderName { get; }

        /// <summary>
        /// Gets the reason authentication failed.
        /// </summary>
        public AuthenticationFailureReason Reason { get; }

        /// <summary>
        /// Gets object data for serialization.
        /// </summary>
        /// <param name="info">The serialization info.</param>
        /// <param name="context">The streaming context.</param>
        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            base.GetObjectData(info, context);
            info.AddValue("DriveGuid", DriveGuid);
            info.AddValue("ProviderName", ProviderName);
            info.AddValue("Reason", (int)Reason);
        }

        /// <summary>
        /// Gets a user-friendly message for the given failure reason.
        /// </summary>
        /// <param name="providerName">The provider name.</param>
        /// <param name="reason">The failure reason.</param>
        /// <returns>A user-friendly error message.</returns>
        private static string GetMessageForReason(string providerName, AuthenticationFailureReason reason)
        {
            string provider = string.IsNullOrEmpty(providerName) ? "The provider" : providerName;

            switch (reason)
            {
                case AuthenticationFailureReason.NotAuthenticated:
                    return string.Format("{0} requires authentication. Please run 'login' to authenticate.", provider);

                case AuthenticationFailureReason.TokenExpired:
                    return string.Format("Your {0} session has expired. Please run 'login' to re-authenticate.", provider);

                case AuthenticationFailureReason.TokenRevoked:
                    return string.Format("Your {0} access has been revoked. Please run 'login' to re-authenticate.", provider);

                case AuthenticationFailureReason.InvalidToken:
                    return string.Format("Your {0} credentials are invalid. Please run 'login' to authenticate.", provider);

                case AuthenticationFailureReason.InsufficientPermissions:
                    return string.Format("Insufficient permissions for {0}. Please run 'login' to request additional permissions.", provider);

                case AuthenticationFailureReason.InvalidSignature:
                    return string.Format("{0} authentication signature is invalid. Please run 'login' to re-authenticate.", provider);

                case AuthenticationFailureReason.ApiKeyMissing:
                    return string.Format("{0} API credentials are not configured. Use 'secret set' to configure API keys.", provider);

                default:
                    return string.Format("{0} authentication is required. Please run 'login' to authenticate.", provider);
            }
        }
    }

    /// <summary>
    /// Describes the reason authentication failed.
    /// </summary>
    [ComVisible(true)]
    public enum AuthenticationFailureReason
    {
        /// <summary>
        /// Unknown or unspecified reason.
        /// </summary>
        Unknown = 0,

        /// <summary>
        /// No authentication credentials are present.
        /// </summary>
        NotAuthenticated = 1,

        /// <summary>
        /// The OAuth token has expired.
        /// </summary>
        TokenExpired = 2,

        /// <summary>
        /// The OAuth token has been revoked by the user or provider.
        /// </summary>
        TokenRevoked = 3,

        /// <summary>
        /// The OAuth token or credentials are invalid.
        /// </summary>
        InvalidToken = 4,

        /// <summary>
        /// The user does not have sufficient permissions for the requested operation.
        /// </summary>
        InsufficientPermissions = 5,

        /// <summary>
        /// The OAuth signature verification failed (OAuth 1.0a).
        /// </summary>
        InvalidSignature = 6,

        /// <summary>
        /// API key or client credentials are missing.
        /// </summary>
        ApiKeyMissing = 7
    }
}
