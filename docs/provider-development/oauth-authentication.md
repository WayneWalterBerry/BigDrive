# OAuth Authentication for BigDrive Providers

## Overview

If your provider connects to a cloud service that requires OAuth authentication (Google Drive,
OneDrive, Flickr, Azure, etc.), implement the `IBigDriveAuthentication` interface.

This enables BigDrive Shell to:
- ✅ Perform OAuth flows on behalf of your provider
- ✅ Store tokens securely in Windows Credential Manager
- ✅ Automatically prompt users to login when tokens expire
- ✅ Handle browser redirects and callback URLs

**Your provider doesn't need OAuth client libraries!** BigDrive Shell handles the entire
OAuth flow - your provider just declares the configuration and consumes the tokens.

---

## IBigDriveAuthentication Interface

```csharp
[Guid("2B3C4D5E-6F7A-8B9C-0D1E-2F3A4B5C6D7E")]
public interface IBigDriveAuthentication
{
    /// <summary>
    /// Returns OAuth configuration for BigDrive Shell to perform authentication.
    /// </summary>
    /// <param name="driveGuid">The drive GUID.</param>
    /// <param name="authInfo">Output parameter with OAuth configuration.</param>
    /// <returns>HRESULT: 0 = success.</returns>
    int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo);

    /// <summary>
    /// Called after successful OAuth - provider can validate or cache tokens.
    /// </summary>
    /// <param name="driveGuid">The drive GUID.</param>
    /// <param name="accessToken">The OAuth access token.</param>
    /// <param name="refreshToken">The OAuth refresh token (may be empty).</param>
    /// <param name="expiresIn">Token lifetime in seconds (0 if non-expiring).</param>
    /// <returns>HRESULT: 0 = success.</returns>
    int OnAuthenticationComplete(Guid driveGuid, string accessToken, 
                                 string refreshToken, int expiresIn);

    /// <summary>
    /// Check if the drive has valid authentication.
    /// </summary>
    /// <param name="driveGuid">The drive GUID.</param>
    /// <param name="isAuthenticated">True if authenticated, false otherwise.</param>
    /// <returns>HRESULT: 0 = success.</returns>
    int IsAuthenticated(Guid driveGuid, out bool isAuthenticated);
}
```

---

## AuthenticationInfo Structure

```csharp
public struct AuthenticationInfo
{
    /// <summary>
    /// OAuth authorization endpoint URL.
    /// For OAuth 1.0a, pack all three URLs separated by pipes (see below).
    /// </summary>
    public string AuthorizationUrl;

    /// <summary>
    /// OAuth token endpoint URL.
    /// Not used for OAuth 1.0a (packed in AuthorizationUrl).
    /// </summary>
    public string TokenUrl;

    /// <summary>
    /// Your application's client ID / consumer key.
    /// </summary>
    public string ClientId;

    /// <summary>
    /// Your application's client secret / consumer secret.
    /// Empty string for public clients (PKCE flow).
    /// </summary>
    public string ClientSecret;

    /// <summary>
    /// Space-separated OAuth scopes (e.g., "read write delete").
    /// Not used for OAuth 1.0a.
    /// </summary>
    public string Scopes;

    /// <summary>
    /// Device authorization endpoint for device code flow (RFC 8628).
    /// Optional - only for headless/CLI scenarios.
    /// </summary>
    public string DeviceAuthorizationUrl;

    /// <summary>
    /// Friendly provider name shown to users (e.g., "Flickr", "OneDrive").
    /// </summary>
    public string ProviderName;

    /// <summary>
    /// The OAuth flow type to use.
    /// </summary>
    public OAuthFlowType FlowType;

    /// <summary>
    /// Semicolon-separated list of secret key names for token storage.
    /// Example: "AccessToken;RefreshToken" or "OAuthToken;OAuthSecret" (OAuth 1.0a).
    /// </summary>
    public string SecretKeyNames;
}
```

---

## Supported OAuth Flow Types

| Flow Type | OAuth Version | Description | Use Cases |
|-----------|---------------|-------------|-----------|
| `AuthorizationCode` | OAuth 2.0 | Browser-based authorization with redirect | Google, OneDrive, Azure |
| `AuthorizationCodePKCE` | OAuth 2.0 | Authorization Code + PKCE for public clients | Mobile apps, desktop apps |
| `DeviceCode` | OAuth 2.0 | Device flow for headless environments (RFC 8628) | CLIs, IoT devices, SSH sessions |
| `OAuth1` | OAuth 1.0a | Legacy three-legged OAuth with signatures | Flickr, Twitter (old) |

---

## OAuth 2.0: Authorization Code Flow

**Best for:** Cloud services with standard OAuth 2.0 (Google Drive, OneDrive, Azure, GitHub)

### Implementation

```csharp
// Provider.IBigDriveAuthentication.cs

public partial class Provider
{
    public int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo)
    {
        authInfo = new AuthenticationInfo
        {
            // OAuth 2.0 endpoints (find in your service's API documentation)
            AuthorizationUrl = "https://accounts.google.com/o/oauth2/v2/auth",
            TokenUrl = "https://oauth2.googleapis.com/token",

            // Your registered OAuth application credentials
            ClientId = "your-app-client-id.apps.googleusercontent.com",
            ClientSecret = "your-app-client-secret", // Empty for PKCE

            // Space-separated scopes (check API docs for required permissions)
            Scopes = "https://www.googleapis.com/auth/drive.readonly",

            // Optional: Device code flow endpoint for headless scenarios
            DeviceAuthorizationUrl = "",

            // Friendly name shown to users
            ProviderName = "Google Drive",

            // Flow type
            FlowType = OAuthFlowType.AuthorizationCode,

            // Secret key names for storing tokens in Credential Manager
            // Format: "AccessTokenKeyName;RefreshTokenKeyName"
            SecretKeyNames = "GoogleDriveAccessToken;GoogleDriveRefreshToken"
        };

        return 0; // S_OK
    }

    public int OnAuthenticationComplete(Guid driveGuid, string accessToken, 
                                        string refreshToken, int expiresIn)
    {
        DefaultTraceSource.TraceInformation($"OnAuthenticationComplete: driveGuid={driveGuid}, expiresIn={expiresIn}s");

        // Invalidate any cached API clients so they pick up the new tokens
        GoogleDriveClientWrapper.InvalidateCache(driveGuid);

        return 0; // S_OK
    }

    public int IsAuthenticated(Guid driveGuid, out bool isAuthenticated)
    {
        // Check if access token exists in Credential Manager
        string token = DriveManager.ReadSecretProperty(driveGuid, "GoogleDriveAccessToken", 
                                                       CancellationToken.None);
        isAuthenticated = !string.IsNullOrEmpty(token);
        return 0; // S_OK
    }
}
```

### What BigDrive Shell Does

When the user runs `bigdrive login`:

1. Calls `GetAuthenticationInfo()` to get OAuth config
2. Opens system browser to `AuthorizationUrl` with PKCE parameters
3. User logs in and authorizes the application
4. Browser redirects to `http://localhost:8080/callback` with authorization code
5. Shell exchanges code for tokens at `TokenUrl`
6. Shell stores tokens in Credential Manager: `BigDrive:{driveGuid}:GoogleDriveAccessToken`
7. Shell calls `OnAuthenticationComplete()` to notify your provider

---

## OAuth 2.0: Device Code Flow

**Best for:** Headless environments (SSH, Remote Desktop, Docker containers)

### Implementation

```csharp
public int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo)
{
    authInfo = new AuthenticationInfo
    {
        AuthorizationUrl = "https://login.microsoftonline.com/common/oauth2/v2.0/authorize",
        TokenUrl = "https://login.microsoftonline.com/common/oauth2/v2.0/token",
        DeviceAuthorizationUrl = "https://login.microsoftonline.com/common/oauth2/v2.0/devicecode",
        ClientId = "your-client-id",
        ClientSecret = "",
        Scopes = "Files.Read offline_access",
        ProviderName = "OneDrive",
        FlowType = OAuthFlowType.DeviceCode,
        SecretKeyNames = "OneDriveAccessToken;OneDriveRefreshToken"
    };

    return 0;
}
```

### What BigDrive Shell Does

When the user runs `bigdrive login`:

1. Calls `GetAuthenticationInfo()` to get OAuth config
2. POSTs to `DeviceAuthorizationUrl` to get device code
3. Displays to user:
   ```
   To sign in, use a web browser to open https://microsoft.com/devicelogin
   and enter the code: ABCD-1234
   ```
4. Polls `TokenUrl` until user completes authentication
5. Stores tokens in Credential Manager
6. Calls `OnAuthenticationComplete()`

---

## OAuth 1.0a Flow (Legacy)

**Best for:** Legacy services still using OAuth 1.0a (Flickr, old Twitter)

### URL Packing Format

OAuth 1.0a requires **three** URLs. Pack them in `AuthorizationUrl` separated by pipes:

```
RequestTokenUrl|AuthorizeUrl|AccessTokenUrl
```

### Implementation

```csharp
public int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo)
{
    // Get API credentials from Credential Manager
    // Users must set these first: bigdrive secret set FlickrApiKey "..."
    string apiKey = DriveManager.ReadSecretProperty(driveGuid, "FlickrApiKey", 
                                                    CancellationToken.None);
    string apiSecret = DriveManager.ReadSecretProperty(driveGuid, "FlickrApiSecret", 
                                                       CancellationToken.None);

    if (string.IsNullOrEmpty(apiKey) || string.IsNullOrEmpty(apiSecret))
    {
        throw new BigDriveAuthenticationRequiredException(
            driveGuid,
            "Flickr",
            AuthenticationFailureReason.ApiKeyMissing,
            new InvalidOperationException("FlickrApiKey and FlickrApiSecret must be set"));
    }

    authInfo = new AuthenticationInfo
    {
        // Pack all three OAuth 1.0a URLs separated by pipes
        AuthorizationUrl = "https://www.flickr.com/services/oauth/request_token|" +
                          "https://www.flickr.com/services/oauth/authorize|" +
                          "https://www.flickr.com/services/oauth/access_token",

        ClientId = apiKey,        // Consumer Key
        ClientSecret = apiSecret, // Consumer Secret

        ProviderName = "Flickr",
        FlowType = OAuthFlowType.OAuth1,

        // OAuth 1.0a stores: oauth_token + oauth_token_secret
        SecretKeyNames = "FlickrOAuthToken;FlickrRefreshToken;FlickrOAuthSecret"
    };

    return 0;
}
```

### What BigDrive Shell Does

When the user runs `bigdrive login`:

1. Calls `GetAuthenticationInfo()` to get OAuth 1.0a config
2. POSTs to Request Token URL with OAuth signature
3. Opens browser to Authorize URL with `oauth_token` parameter
4. User authorizes and gets a verifier code
5. User enters verifier code in shell prompt
6. POSTs to Access Token URL to exchange verifier for access token + secret
7. Stores `oauth_token` and `oauth_token_secret` in Credential Manager
8. Calls `OnAuthenticationComplete()`

---

## Token Storage and Retrieval

### Storage Location

Tokens are stored in **Windows Credential Manager** with the key format:

```
BigDrive:{DriveGuid}:{SecretKeyName}
```

Example:
```
BigDrive:12345678-90ab-cdef-1234-567890abcdef:GoogleDriveAccessToken
BigDrive:12345678-90ab-cdef-1234-567890abcdef:GoogleDriveRefreshToken
```

### Reading Tokens in Your Provider

```csharp
using System.Threading;
using BigDrive.ConfigProvider;

string accessToken = DriveManager.ReadSecretProperty(
    driveGuid, 
    "GoogleDriveAccessToken", 
    CancellationToken.None);

string refreshToken = DriveManager.ReadSecretProperty(
    driveGuid, 
    "GoogleDriveRefreshToken", 
    CancellationToken.None);
```

### Checking if Authenticated

```csharp
public int IsAuthenticated(Guid driveGuid, out bool isAuthenticated)
{
    string token = DriveManager.ReadSecretProperty(driveGuid, "YourAccessToken", 
                                                   CancellationToken.None);
    isAuthenticated = !string.IsNullOrEmpty(token);
    return 0;
}
```

---

## Automatic Re-Authentication Flow

### 1. Define AuthenticationFailureReason

When your API client detects authentication failures, throw `BigDriveAuthenticationRequiredException`:

```csharp
public class YourServiceClientWrapper
{
    public List<FileInfo> GetFiles(string path)
    {
        try
        {
            // Make API call
            return _httpClient.Get<List<FileInfo>>($"/api/files?path={path}");
        }
        catch (HttpRequestException ex) when (ex.StatusCode == HttpStatusCode.Unauthorized)
        {
            // Convert to BigDrive authentication exception
            throw new BigDriveAuthenticationRequiredException(
                _driveGuid,
                "YourService",
                AuthenticationFailureReason.TokenExpired,
                ex);
        }
        catch (YourServiceTokenExpiredException ex)
        {
            throw new BigDriveAuthenticationRequiredException(
                _driveGuid,
                "YourService",
                AuthenticationFailureReason.TokenExpired,
                ex);
        }
    }
}
```

### 2. BigDrive Shell Catches Exception

When Shell catches `BigDriveAuthenticationRequiredException`:

```
YourService requires authentication. Please run 'login' to authenticate.

Would you like to login now? [Y/n]: y

Opening your web browser for YourService login...
[User completes OAuth flow in browser]
Authentication successful!
```

### 3. Your Provider Gets Notified

After successful login, Shell calls `OnAuthenticationComplete()`:

```csharp
public int OnAuthenticationComplete(Guid driveGuid, string accessToken, 
                                    string refreshToken, int expiresIn)
{
    DefaultTraceSource.TraceInformation($"OnAuthenticationComplete: driveGuid={driveGuid}");

    // Invalidate any cached API clients so they pick up the new tokens
    YourServiceClientWrapper.InvalidateCache(driveGuid);

    // Optionally validate tokens or perform initial API call
    try
    {
        var client = YourServiceClientWrapper.GetForDrive(driveGuid);
        client.ValidateToken(accessToken);
    }
    catch (Exception ex)
    {
        DefaultTraceSource.TraceError($"Token validation failed: {ex.Message}");
        return -1;
    }

    return 0; // S_OK
}
```

---

## AuthenticationFailureReason Reference

| Reason | When to Use | User Experience |
|--------|-------------|-----------------|
| `NotAuthenticated` | No tokens present in Credential Manager | "You must login first" |
| `TokenExpired` | OAuth token lifetime exceeded | "Your session expired, please login again" |
| `TokenRevoked` | User revoked access via service's UI | "Access was revoked, please re-authorize" |
| `InvalidToken` | Token signature validation failed | "Authentication failed, please login again" |
| `InsufficientPermissions` | User lacks required scopes | "Additional permissions required" |
| `InvalidSignature` | OAuth 1.0a signature mismatch | "Authentication signature invalid" |
| `ApiKeyMissing` | No API key/secret configured | "API credentials not configured" |

---

## Complete Examples

### Example 1: Google Drive (OAuth 2.0 with PKCE)

```csharp
// Provider.IBigDriveAuthentication.cs

namespace BigDrive.Provider.GoogleDrive
{
    using System;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using BigDrive.Interfaces;

    public partial class Provider
    {
        public int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo)
        {
            authInfo = new AuthenticationInfo
            {
                AuthorizationUrl = "https://accounts.google.com/o/oauth2/v2/auth",
                TokenUrl = "https://oauth2.googleapis.com/token",
                ClientId = "123456789.apps.googleusercontent.com",
                ClientSecret = "", // Empty = use PKCE
                Scopes = "https://www.googleapis.com/auth/drive.readonly",
                ProviderName = "Google Drive",
                FlowType = OAuthFlowType.AuthorizationCodePKCE,
                SecretKeyNames = "GoogleDriveAccessToken;GoogleDriveRefreshToken"
            };

            return 0;
        }

        public int OnAuthenticationComplete(Guid driveGuid, string accessToken, 
                                            string refreshToken, int expiresIn)
        {
            GoogleDriveClientWrapper.InvalidateCache(driveGuid);
            return 0;
        }

        public int IsAuthenticated(Guid driveGuid, out bool isAuthenticated)
        {
            string token = DriveManager.ReadSecretProperty(driveGuid, "GoogleDriveAccessToken", 
                                                           CancellationToken.None);
            isAuthenticated = !string.IsNullOrEmpty(token);
            return 0;
        }
    }
}
```

---

### Example 2: Azure (OAuth 2.0 with Device Code)

```csharp
// Provider.IBigDriveAuthentication.cs

namespace BigDrive.Provider.AzureBlob
{
    public partial class Provider
    {
        public int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo)
        {
            authInfo = new AuthenticationInfo
            {
                AuthorizationUrl = "https://login.microsoftonline.com/common/oauth2/v2.0/authorize",
                TokenUrl = "https://login.microsoftonline.com/common/oauth2/v2.0/token",
                DeviceAuthorizationUrl = "https://login.microsoftonline.com/common/oauth2/v2.0/devicecode",
                ClientId = "your-azure-app-client-id",
                ClientSecret = "",
                Scopes = "https://storage.azure.com/user_impersonation offline_access",
                ProviderName = "Azure Blob Storage",
                FlowType = OAuthFlowType.DeviceCode, // Better for CLI/headless
                SecretKeyNames = "AzureAccessToken;AzureRefreshToken"
            };

            return 0;
        }

        public int OnAuthenticationComplete(Guid driveGuid, string accessToken, 
                                            string refreshToken, int expiresIn)
        {
            AzureBlobClientWrapper.InvalidateCache(driveGuid);
            return 0;
        }

        public int IsAuthenticated(Guid driveGuid, out bool isAuthenticated)
        {
            string token = DriveManager.ReadSecretProperty(driveGuid, "AzureAccessToken", 
                                                           CancellationToken.None);
            isAuthenticated = !string.IsNullOrEmpty(token);
            return 0;
        }
    }
}
```

---

### Example 3: Flickr (OAuth 1.0a)

```csharp
// Provider.IBigDriveAuthentication.cs

namespace BigDrive.Provider.Flickr
{
    public partial class Provider
    {
        public int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo)
        {
            // OAuth 1.0a requires API Key and Secret to be pre-configured
            string apiKey = DriveManager.ReadSecretProperty(driveGuid, "FlickrApiKey", 
                                                            CancellationToken.None);
            string apiSecret = DriveManager.ReadSecretProperty(driveGuid, "FlickrApiSecret", 
                                                               CancellationToken.None);

            if (string.IsNullOrEmpty(apiKey) || string.IsNullOrEmpty(apiSecret))
            {
                throw new BigDriveAuthenticationRequiredException(
                    driveGuid,
                    "Flickr",
                    AuthenticationFailureReason.ApiKeyMissing,
                    new InvalidOperationException("FlickrApiKey and FlickrApiSecret must be set. Use 'bigdrive secret set FlickrApiKey \"...\"'"));
            }

            // OAuth 1.0a: Pack three URLs separated by pipes
            authInfo = new AuthenticationInfo
            {
                AuthorizationUrl = "https://www.flickr.com/services/oauth/request_token|" +
                                  "https://www.flickr.com/services/oauth/authorize|" +
                                  "https://www.flickr.com/services/oauth/access_token",

                ClientId = apiKey,        // Consumer Key
                ClientSecret = apiSecret, // Consumer Secret
                Scopes = "", // OAuth 1.0a doesn't use scopes

                ProviderName = "Flickr",
                FlowType = OAuthFlowType.OAuth1,

                // OAuth 1.0a stores: oauth_token + oauth_token_secret
                SecretKeyNames = "FlickrOAuthToken;FlickrRefreshToken;FlickrOAuthSecret"
            };

            return 0;
        }

        public int OnAuthenticationComplete(Guid driveGuid, string accessToken, 
                                            string refreshToken, int expiresIn)
        {
            FlickrClientWrapper.InvalidateCache(driveGuid);
            return 0;
        }

        public int IsAuthenticated(Guid driveGuid, out bool isAuthenticated)
        {
            string token = DriveManager.ReadSecretProperty(driveGuid, "FlickrOAuthToken", 
                                                           CancellationToken.None);
            string secret = DriveManager.ReadSecretProperty(driveGuid, "FlickrOAuthSecret", 
                                                            CancellationToken.None);
            isAuthenticated = !string.IsNullOrEmpty(token) && !string.IsNullOrEmpty(secret);
            return 0;
        }
    }
}
```

**OAuth 1.0a differences:**

- Requires **API Key and Secret** to be set BEFORE login
- User must obtain these from the service's developer portal
- Uses three-legged flow with request tokens and verifier codes
- Stores `oauth_token` and `oauth_token_secret` (not access/refresh tokens)

---

## User Workflow

### OAuth 2.0 Workflow

```sh
# 1. Create drive
bigdrive drive create --provider GoogleDrive --name "MyDrive"

# 2. Login (opens browser automatically)
bigdrive login
# → Browser opens to accounts.google.com
# → User authorizes application
# → Browser redirects to localhost callback
# → Shell stores tokens

# 3. Use drive
bigdrive ls
bigdrive cd Photos
bigdrive copy "vacation.jpg" "C:\Pictures\"
```

### OAuth 1.0a Workflow (Flickr)

```sh
# 1. Create drive
bigdrive drive create --provider Flickr --name "MyFlickr"

# 2. Set API credentials (get from https://www.flickr.com/services/apps/create/)
bigdrive secret set FlickrApiKey "your-api-key"
bigdrive secret set FlickrApiSecret "your-api-secret"

# 3. Login
bigdrive login
# → Opens browser to flickr.com/services/oauth/authorize
# → User authorizes
# → User copies verifier code from browser
# → User enters verifier code in shell prompt
# → Shell exchanges verifier for oauth_token + oauth_token_secret

# 4. Use drive
bigdrive ls
```

---

## Implementing Token Refresh (OAuth 2.0)

If your provider's tokens expire, implement refresh logic in your API client wrapper:

```csharp
public class YourServiceClientWrapper
{
    private readonly Guid _driveGuid;
    private string _accessToken;
    private string _refreshToken;

    public YourServiceClientWrapper(Guid driveGuid)
    {
        _driveGuid = driveGuid;
        LoadTokens();
    }

    private void LoadTokens()
    {
        _accessToken = DriveManager.ReadSecretProperty(_driveGuid, "YourAccessToken", 
                                                       CancellationToken.None);
        _refreshToken = DriveManager.ReadSecretProperty(_driveGuid, "YourRefreshToken", 
                                                        CancellationToken.None);
    }

    public List<FileInfo> GetFiles(string path)
    {
        try
        {
            return ApiCall(() => _client.ListFiles(path));
        }
        catch (TokenExpiredException)
        {
            // Token expired - let Shell handle re-authentication
            throw new BigDriveAuthenticationRequiredException(
                _driveGuid,
                "YourService",
                AuthenticationFailureReason.TokenExpired,
                null);
        }
    }

    private T ApiCall<T>(Func<T> apiMethod)
    {
        try
        {
            return apiMethod();
        }
        catch (UnauthorizedException ex)
        {
            // Throw BigDrive exception so Shell can prompt for login
            throw new BigDriveAuthenticationRequiredException(
                _driveGuid,
                "YourService",
                AuthenticationFailureReason.TokenExpired,
                ex);
        }
    }
}
```

**Note:** Your provider doesn't need to implement token refresh logic! BigDrive Shell
handles refresh tokens automatically. Just throw `BigDriveAuthenticationRequiredException`
and Shell will re-run the OAuth flow.

---

## Testing OAuth Implementation

### Test 1: GetAuthenticationInfo

```csharp
// Unit test
[Test]
public void GetAuthenticationInfo_ReturnsValidConfig()
{
    Provider provider = new Provider();
    Guid testDriveGuid = Guid.NewGuid();

    int result = provider.GetAuthenticationInfo(testDriveGuid, out AuthenticationInfo authInfo);

    Assert.AreEqual(0, result);
    Assert.IsNotNull(authInfo.AuthorizationUrl);
    Assert.IsNotNull(authInfo.TokenUrl);
    Assert.IsNotNull(authInfo.ClientId);
    Assert.AreEqual(OAuthFlowType.AuthorizationCode, authInfo.FlowType);
}
```

### Test 2: Manual Login Flow

```sh
# Create test drive
bigdrive drive create --provider YourService --name "OAuthTest"

# Trigger login
bigdrive login

# Expected:
# - Browser opens to authorization URL
# - User authorizes
# - Shell displays "Authentication successful!"
# - Tokens stored in Credential Manager

# Verify authentication
bigdrive status
# Should show: Authenticated: Yes
```

### Test 3: Token Persistence

```sh
# After logging in, restart BigDrive Shell
bigdrive exit

# Start new session
bigdrive

# Try listing files (should work without re-login)
bigdrive ls
```

### Test 4: Re-Authentication

```sh
# Delete tokens to simulate expiration
bigdrive secret delete YourAccessToken

# Try operation (should prompt for re-login)
bigdrive ls
# Expected: "YourService requires authentication. Would you like to login now?"
```

---

## Security Considerations

### Credential Storage

- ✅ **DO** store tokens in Windows Credential Manager (handled by BigDrive Shell)
- ✅ **DO** use per-drive tokens (different credentials per mounted drive)
- ❌ **DO NOT** store tokens in registry or plain text files
- ❌ **DO NOT** hardcode client secrets in code (read from Credential Manager)

### Token Scopes

Request **minimal scopes** required for functionality:

| Functionality | Recommended Scopes |
|---------------|-------------------|
| Read-only browsing | `read`, `files.read` |
| Upload files | `read write`, `files.readwrite` |
| Delete files | `read write delete`, `files.readwrite.all` |

### Client Secret Protection

For **public desktop applications**, use PKCE (empty `ClientSecret`):

```csharp
authInfo = new AuthenticationInfo
{
    ClientId = "your-client-id",
    ClientSecret = "", // Empty = PKCE flow
    FlowType = OAuthFlowType.AuthorizationCodePKCE,
    // ...
};
```

For **confidential applications** (backend services), store secrets in Credential Manager:

```csharp
string clientSecret = DriveManager.ReadSecretProperty(driveGuid, "YourClientSecret", 
                                                      CancellationToken.None);
```

---

## See Also

- [Getting Started](getting-started.md) - Project setup
- [Interfaces Reference](interfaces.md) - All interface definitions
- [Troubleshooting](troubleshooting.md) - Common OAuth errors
- [Flickr Provider Source](../../src/BigDrive.Provider.Flickr/Provider.IBigDriveAuthentication.cs) - OAuth 1.0a example
