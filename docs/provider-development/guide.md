# BigDrive Provider Development Guide

This is the **complete reference guide** for creating BigDrive providers. For a quicker overview, see [Provider Development README](README.md).

---

## 📚 Provider Development Documentation

### Quick Navigation

| Document | Purpose |
|----------|---------|
| **[README](README.md)** | Overview and quick start paths |
| **[Architecture](architecture.md)** | **NEW!** Provider architecture & COM+ model |
| **[Getting Started](getting-started.md)** | Project setup, naming conventions |
| **[This Guide](guide.md)** | **Complete reference** with all details |
| **[Development Practices](practices.md)** | Build-register-test workflow |
| **[Interfaces Reference](interfaces.md)** | Interface definitions |
| **[NuGet Dependencies](nuget-dependencies.md)** | **CRITICAL!** AssemblyResolver setup |
| **[OAuth Authentication](oauth-authentication.md)** | OAuth 2.0/1.0a flows |
| **[Troubleshooting](troubleshooting.md)** | Common errors |

---

## Architecture Overview

Providers are COM+ ServicedComponents that run out-of-process in `dllhost.exe` as the Interactive User.

**Key Concepts:**
- ✅ **Out-of-Process** - Runs in dllhost.exe, not explorer.exe (fault isolation)
- ✅ **Interactive User** - Access to Credential Manager and user's registry
- ✅ **Self-Registering** - Uses `[ComRegisterFunction]` for automated setup
- ✅ **Lifecycle Managed** - COM+ handles startup, idle timeout, and shutdown

**→ Full details:** [Provider Architecture](architecture.md)

---

## Naming Conventions

### Project Naming

| Component | Convention | Example |
|-----------|------------|---------|
| Project Name | `BigDrive.Provider.{ServiceName}` | `BigDrive.Provider.AzureBlob` |
| Namespace | `BigDrive.Provider.{ServiceName}` | `BigDrive.Provider.AzureBlob` |
| Assembly Name | `BigDrive.Provider.{ServiceName}` | `BigDrive.Provider.AzureBlob.dll` |

### File Naming (Partial Classes)

We use **one partial class file per interface** for maintainability:

| File | Contents |
|------|----------|
| `Provider.cs` | Main class declaration, GUID, shared fields |
| `Provider.IProcessInitializer.cs` | `Startup()` and `Shutdown()` methods |
| `Provider.IBigDriveRegistration.cs` | `Register()` and `Unregister()` methods |
| `Provider.IBigDriveEnumerate.cs` | `EnumerateFolders()` and `EnumerateFiles()` methods |
| `Provider.IBigDriveFileInfo.cs` | `LastModifiedTime()` and `GetFileSize()` methods |
| `Provider.IBigDriveFileOperations.cs` | `CopyFileToBigDrive()`, `CopyFileFromBigDrive()`, etc. |
| `Provider.IBigDriveFileData.cs` | `GetFileData()` method (returns IStream) |

### Helper Classes

| File | Purpose |
|------|---------|
| `{ServiceName}ClientWrapper.cs` | API client wrapper (e.g., `AzureBlobClientWrapper.cs`) |
| `BigDriveTraceSource.cs` | Logging/tracing (copy from existing provider) |
| `ComStream.cs` | IStream wrapper for file streaming (copy from existing provider) |
| `ProviderConfigurationFactory.cs` | Provider-specific configuration loading |
| `AssemblyResolver.cs` | **REQUIRED** - NuGet dependency resolution for COM+ (see NuGet Dependencies section) |
| `app.config` | **REQUIRED** - Assembly binding redirects for version conflicts (see app.config section) |

---

## Required Interfaces

### Core Interfaces (Required)

| Interface | Purpose | Methods |
|-----------|---------|---------|
| `IProcessInitializer` | COM+ lifecycle hooks | `Startup()`, `Shutdown()` |
| `IBigDriveEnumerate` | List folders and files | `EnumerateFolders()`, `EnumerateFiles()` |

### Optional Interfaces

| Interface | Purpose | When to Implement |
|-----------|---------|-------------------|
| `IBigDriveRegistration` | Shell registration callbacks | If provider needs setup during shell registration |
| `IBigDriveFileInfo` | File metadata | If you can provide file sizes and dates |
| `IBigDriveFileOperations` | Copy/delete/mkdir | If provider supports write operations |
| `IBigDriveFileData` | Stream file content | If files can be downloaded/read |
| `IBigDriveAuthentication` | OAuth authentication | If provider requires OAuth login (see below) |

---

## Implementing OAuth Authentication

If your provider requires OAuth authentication (most cloud services do), implement
`IBigDriveAuthentication`. This enables BigDrive Shell to perform OAuth flows on behalf
of your provider and store tokens securely in Windows Credential Manager.

### Interface Definition

```csharp
public interface IBigDriveAuthentication
{
    /// <summary>
    /// Returns OAuth configuration for BigDrive Shell to perform authentication.
    /// </summary>
    int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo);

    /// <summary>
    /// Called after successful OAuth - provider can validate or cache tokens.
    /// </summary>
    int OnAuthenticationComplete(Guid driveGuid, string accessToken, 
                                 string refreshToken, int expiresIn);

    /// <summary>
    /// Check if the drive has valid authentication.
    /// </summary>
    int IsAuthenticated(Guid driveGuid, out bool isAuthenticated);
}
```

### AuthenticationInfo Structure

```csharp
public struct AuthenticationInfo
{
    public string AuthorizationUrl;      // OAuth authorize endpoint
    public string TokenUrl;              // OAuth token endpoint
    public string ClientId;              // Your app's client ID
    public string ClientSecret;          // Your app's client secret (may be empty)
    public string Scopes;                // Space-separated scopes
    public string DeviceAuthorizationUrl; // For device code flow (optional)
    public string ProviderName;          // Friendly name ("Flickr", "OneDrive")
    public OAuthFlowType FlowType;       // AuthorizationCode, DeviceCode, OAuth1
    public string SecretKeyNames;        // "AccessToken;RefreshToken;AccessSecret"
}
```

### Supported OAuth Flow Types

| Flow | Use Case | Example Providers |
|------|----------|-------------------|
| `AuthorizationCode` | Standard OAuth 2.0 with browser redirect | Google, Azure, OneDrive |
| `DeviceCode` | Headless environments (RFC 8628) | Azure, GitHub |
| `OAuth1` | Legacy OAuth 1.0a with verifier codes | Flickr, Twitter |
| `AuthorizationCodePKCE` | Public clients (mobile, desktop) | Modern OAuth 2.0 |

### Example: OAuth 2.0 Provider

```csharp
// Provider.IBigDriveAuthentication.cs

public partial class Provider
{
    public int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo)
    {
        authInfo = new AuthenticationInfo
        {
            AuthorizationUrl = "https://login.microsoftonline.com/common/oauth2/v2.0/authorize",
            TokenUrl = "https://login.microsoftonline.com/common/oauth2/v2.0/token",
            ClientId = "your-client-id",
            ClientSecret = "", // Empty for public clients
            Scopes = "Files.Read Files.ReadWrite offline_access",
            DeviceAuthorizationUrl = "https://login.microsoftonline.com/common/oauth2/v2.0/devicecode",
            ProviderName = "OneDrive",
            FlowType = OAuthFlowType.AuthorizationCode,
            SecretKeyNames = "OneDriveAccessToken;OneDriveRefreshToken"
        };

        return 0; // S_OK
    }

    public int OnAuthenticationComplete(Guid driveGuid, string accessToken, 
                                        string refreshToken, int expiresIn)
    {
        // Invalidate any cached API clients so they pick up new tokens
        ApiClientCache.Invalidate(driveGuid);
        return 0; // S_OK
    }

    public int IsAuthenticated(Guid driveGuid, out bool isAuthenticated)
    {
        // Check if tokens exist in Credential Manager
        string token = DriveManager.ReadSecretProperty(driveGuid, "OneDriveAccessToken", 
                                                       CancellationToken.None);
        isAuthenticated = !string.IsNullOrEmpty(token);
        return 0; // S_OK
    }
}
```

### Example: OAuth 1.0a Provider (Flickr)

For OAuth 1.0a, pack URLs in the AuthorizationUrl field:

```csharp
public int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo)
{
    // Get API credentials (user must set these first via 'secret set')
    string apiKey = DriveManager.ReadSecretProperty(driveGuid, "FlickrApiKey", 
                                                    CancellationToken.None);
    string apiSecret = DriveManager.ReadSecretProperty(driveGuid, "FlickrApiSecret", 
                                                       CancellationToken.None);

    // OAuth 1.0a requires three URLs: request_token, authorize, access_token
    authInfo = new AuthenticationInfo
    {
        // Pack URLs as "requestTokenUrl|authorizeUrl|accessTokenUrl"
        AuthorizationUrl = "https://flickr.com/services/oauth/request_token|" +
                          "https://flickr.com/services/oauth/authorize|" +
                          "https://flickr.com/services/oauth/access_token",
        ClientId = apiKey,      // Consumer Key
        ClientSecret = apiSecret, // Consumer Secret
        ProviderName = "Flickr",
        FlowType = OAuthFlowType.OAuth1,
        SecretKeyNames = "FlickrOAuthToken;FlickrRefreshToken;FlickrOAuthSecret"
    };

    return 0; // S_OK
}
```

### Token Storage

BigDrive Shell stores tokens in Windows Credential Manager using:

```
BigDrive:{DriveGuid}:{SecretKeyName}
```

Your provider reads them with:

```csharp
string accessToken = DriveManager.ReadSecretProperty(driveGuid, "MyAccessToken", 
                                                     CancellationToken.None);
string refreshToken = DriveManager.ReadSecretProperty(driveGuid, "MyRefreshToken", 
                                                      CancellationToken.None);
```

### Automatic Re-Authentication

Providers should throw `BigDriveAuthenticationRequiredException` when API calls fail
due to authentication issues. BigDrive Shell catches this exception and automatically
prompts the user to login.

```csharp
// In your API wrapper class

public List<FileInfo> GetFiles(string path)
{
    try
    {
        return _apiClient.ListFiles(path);
    }
    catch (YourServiceAuthException ex)
    {
        // Convert service-specific exception to generic BigDrive exception
        throw new BigDriveAuthenticationRequiredException(
            _driveGuid,
            "YourService",
            AuthenticationFailureReason.TokenExpired,
            ex);
    }
}
```

**AuthenticationFailureReason values:**

| Value | When to Use |
|-------|-------------|
| `NotAuthenticated` | No credentials present |
| `TokenExpired` | OAuth token has expired |
| `TokenRevoked` | Token was revoked by user or provider |
| `InvalidToken` | Token or credentials are invalid |
| `InsufficientPermissions` | User lacks required scopes/permissions |
| `InvalidSignature` | OAuth signature verification failed (OAuth 1.0a) |
| `ApiKeyMissing` | API key or client credentials not configured |

When the shell catches this exception, it displays a user-friendly message and
offers to run the login command automatically:

```
Flickr requires authentication. Please run 'login' to authenticate.

Would you like to login now? [Y/n]: y

Opening your web browser for Flickr login...
```

---

## Step-by-Step: Creating a New Provider

### Step 1: Create the Project

1. Create a new **Class Library (.NET Framework 4.7.2)** project
2. Name it `BigDrive.Provider.{YourService}`
3. Add project references:
   - `BigDrive.Interfaces`
   - `BigDrive.ConfigProvider` (if using configuration)
4. Add assembly references:
   - `System.EnterpriseServices`

### Step 2: Configure AssemblyInfo.cs

```csharp
// Properties/AssemblyInfo.cs

using System.EnterpriseServices;
using System.Reflection;
using System.Runtime.InteropServices;

[assembly: AssemblyTitle("BigDrive.Provider.YourService")]
[assembly: AssemblyDescription("BigDrive provider for YourService")]
[assembly: AssemblyCompany("Your Company")]
[assembly: AssemblyProduct("BigDrive")]
[assembly: AssemblyCopyright("Copyright © Your Company 2025")]

// CRITICAL: These three attributes are required for COM+ hosting
[assembly: ComVisible(true)]
[assembly: Guid("YOUR-UNIQUE-GUID-HERE")]  // Generate a new GUID!

// COM+ Server activation (runs in dllhost.exe)
[assembly: ApplicationActivation(ActivationOption.Server)]
[assembly: ApplicationAccessControl(false)]

[assembly: AssemblyVersion("1.0.0.0")]
[assembly: AssemblyFileVersion("1.0.0.0")]
```

### Step 3: Create the Main Provider Class

```csharp
// Provider.cs

// <copyright file="Provider.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.YourService
{
    using System;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;

    using BigDrive.Interfaces;

    /// <summary>
    /// BigDrive provider for YourService integration.
    /// </summary>
    [Guid("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")]  // Generate unique CLSID!
    [ClassInterface(ClassInterfaceType.None)]
    [ComVisible(true)]
    public partial class Provider : ServicedComponent,
        IProcessInitializer,
        IBigDriveRegistration,
        IBigDriveEnumerate,
        IBigDriveFileInfo,
        IBigDriveFileData
        // Add other interfaces as needed
    {
        /// <summary>
        /// The trace source for logging.
        /// </summary>
        private static readonly BigDriveTraceSource DefaultTraceSource = BigDriveTraceSource.Instance;

        /// <summary>
        /// Static constructor to ensure AssemblyResolver is initialized early.
        /// CRITICAL: This must run before any COM+ registration code executes.
        /// </summary>
        static Provider()
        {
            // Force AssemblyResolver static constructor to run
            AssemblyResolver.Initialize();
        }

        /// <summary>
        /// Your API client wrapper.
        /// </summary>
        private static readonly YourServiceClientWrapper ApiClient = new YourServiceClientWrapper();

        /// <summary>
        /// Gets the CLSID of this provider.
        /// </summary>
        public static Guid CLSID
        {
            get
            {
                Type providerType = typeof(Provider);
                GuidAttribute guidAttribute = (GuidAttribute)Attribute.GetCustomAttribute(
                    providerType, typeof(GuidAttribute));
                return Guid.Parse(guidAttribute.Value);
            }
        }

        /// <summary>
        /// Gets the provider configuration for registry registration.
        /// </summary>
        private static BigDrive.ConfigProvider.Model.ProviderConfiguration ProviderConfig
        {
            get
            {
                return ProviderConfigurationFactory.Create();
            }
        }
    }
}
```

### Step 4: Implement IBigDriveRegistration (Self-Registration)

This is **critical** for provider installation. The `[ComRegisterFunction]` attribute
causes this method to be called automatically during `regsvcs.exe` registration.

```csharp
// Provider.IBigDriveRegistration.cs

// <copyright file="Provider.IBigDriveRegistration.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.YourService
{
    using System;
    using System.Runtime.InteropServices;
    using System.Security.Principal;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;

    public partial class Provider
    {
        /// <summary>
        /// Called automatically by regsvcs.exe during COM registration.
        /// Sets the COM+ application identity to Interactive User and registers the provider.
        /// </summary>
        /// <param name="type">The type being registered.</param>
        [ComRegisterFunction]
        public static void ComRegister(Type type)
        {
            DefaultTraceSource.TraceInformation($"ComRegister: Registering provider {type.FullName}");

            // IMPORTANT: Set COM+ application identity to Interactive User
            // This allows the provider to access the user's Credential Manager
            SetApplicationIdentityToInteractiveUser("BigDrive.Provider.YourService");

            // Create an instance and call Register()
            Provider provider = new Provider();
            provider.Register();

            DefaultTraceSource.TraceInformation("ComRegister: Provider registration completed.");
        }

        /// <summary>
        /// Called automatically by regsvcs.exe during COM unregistration.
        /// </summary>
        /// <param name="type">The type being unregistered.</param>
        [ComUnregisterFunction]
        public static void ComUnregister(Type type)
        {
            DefaultTraceSource.TraceInformation($"ComUnregister: Unregistering provider {type.FullName}");

            Provider provider = new Provider();
            provider.Unregister();
        }

        /// <inheritdoc/>
        public void Register()
        {
            WindowsIdentity identity = WindowsIdentity.GetCurrent();
            DefaultTraceSource.TraceInformation($"Register: User={identity.Name}");

            // 1. Register the provider in BigDrive's provider registry
            var providerConfig = ProviderConfigurationFactory.Create();
            ProviderManager.RegisterProvider(providerConfig, CancellationToken.None);

            // 2. Optionally create a default drive configuration
            DriveConfiguration driveConfig = new DriveConfiguration
            {
                CLSID = providerConfig.Id,
                Name = "YourService Drive",
                Id = Guid.NewGuid()  // Or a fixed GUID for your provider
            };
            DriveManager.WriteConfiguration(driveConfig, CancellationToken.None);

            DefaultTraceSource.TraceInformation("Register: Provider registered successfully.");
        }

        /// <inheritdoc/>
        public void Unregister()
        {
            DefaultTraceSource.TraceInformation("Unregister: Provider");
            // TODO: Clean up registry entries
        }

        /// <summary>
        /// Sets the identity of a COM+ application to "Interactive User".
        /// This allows the provider to run as the logged-in user and access Credential Manager.
        /// </summary>
        /// <param name="applicationName">The name of the COM+ application.</param>
        private static void SetApplicationIdentityToInteractiveUser(string applicationName)
        {
            Type comAdminType = Type.GetTypeFromProgID("COMAdmin.COMAdminCatalog");
            if (comAdminType == null)
            {
                DefaultTraceSource.TraceError("COMAdminCatalog is not available.");
                return;
            }

            dynamic comAdmin = Activator.CreateInstance(comAdminType);
            try
            {
                dynamic applications = comAdmin.GetCollection("Applications");
                applications.Populate();

                foreach (dynamic app in applications)
                {
                    if (string.Equals((string)app.Name, applicationName, StringComparison.OrdinalIgnoreCase))
                    {
                        app.Value["Identity"] = "Interactive User";
                        app.Value["Password"] = "";
                        applications.SaveChanges();

                        DefaultTraceSource.TraceInformation(
                            $"COM+ application '{applicationName}' identity set to 'Interactive User'.");
                        return;
                    }
                }

                DefaultTraceSource.TraceInformation($"COM+ application '{applicationName}' not found.");
            }
            finally
            {
                if (comAdmin != null && Marshal.IsComObject(comAdmin))
                {
                    Marshal.ReleaseComObject(comAdmin);
                }
            }
        }
    }
}
```

**Why Interactive User?**
- Providers run as the logged-in user, not a service account
- This enables access to Windows Credential Manager for storing API keys/secrets
- Users can configure secrets via `BigDrive.Shell` with the `secret` command

### Step 5: Implement IProcessInitializer

```csharp
// Provider.IProcessInitializer.cs

// <copyright file="Provider.IProcessInitializer.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.YourService
{
    /// <summary>
    /// Implementation of <see cref="System.EnterpriseServices.IProcessInitializer"/>.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Called when the COM+ application starts.
        /// </summary>
        /// <param name="punkProcessControl">Process control interface (may be null).</param>
        public void Startup(object punkProcessControl)
        {
            DefaultTraceSource.TraceInformation("YourService Provider Startup");

            // Initialize your API client, authenticate, load config, etc.
            // ApiClient.Initialize();
        }

        /// <summary>
        /// Called when the COM+ application shuts down.
        /// </summary>
        public void Shutdown()
        {
            DefaultTraceSource.TraceInformation("YourService Provider Shutdown");
            
            // Clean up resources
            // ApiClient.Dispose();
        }
    }
}
```

### Step 5: Implement IBigDriveEnumerate

```csharp
// Provider.IBigDriveEnumerate.cs

// <copyright file="Provider.IBigDriveEnumerate.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.YourService
{
    using System;
    using System.Linq;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveEnumerate"/>.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Enumerates folder names at the specified path.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The path to enumerate.</param>
        /// <returns>Array of folder names.</returns>
        public string[] EnumerateFolders(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"EnumerateFolders: path={path}");

                // Your implementation:
                // 1. Call your API to list folders at 'path'
                // 2. Return folder names (not full paths)
                // 3. Sanitize names to remove invalid file system characters

                var folders = ApiClient.ListFolders(path);
                return folders.Select(f => SanitizeName(f.Name)).ToArray();
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"EnumerateFolders failed: {ex.Message}");
                return Array.Empty<string>();
            }
        }

        /// <summary>
        /// Enumerates file names at the specified path.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The path to enumerate.</param>
        /// <returns>Array of file names.</returns>
        public string[] EnumerateFiles(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"EnumerateFiles: path={path}");

                var files = ApiClient.ListFiles(path);
                return files.Select(f => SanitizeName(f.Name)).ToArray();
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"EnumerateFiles failed: {ex.Message}");
                return Array.Empty<string>();
            }
        }

        /// <summary>
        /// Sanitizes a name by removing invalid file system characters.
        /// </summary>
        private static string SanitizeName(string name)
        {
            if (string.IsNullOrEmpty(name))
            {
                return "Untitled";
            }

            char[] invalidChars = System.IO.Path.GetInvalidFileNameChars();
            foreach (char c in invalidChars)
            {
                name = name.Replace(c, '_');
            }

            return name;
        }
    }
}
```

### Step 6: Implement IBigDriveFileData (for downloads)

```csharp
// Provider.IBigDriveFileData.cs

// <copyright file="Provider.IBigDriveFileData.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.YourService
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices.ComTypes;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveFileData"/>.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets file data as an IStream.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The file path.</param>
        /// <param name="stream">The output stream.</param>
        /// <returns>HRESULT (0 = success).</returns>
        public int GetFileData(Guid driveGuid, string path, out IStream stream)
        {
            stream = null;

            try
            {
                DefaultTraceSource.TraceInformation($"GetFileData: path={path}");

                // Get file content from your API
                Stream dataStream = ApiClient.DownloadFile(path);

                // Wrap in COM IStream
                stream = new ComStream(dataStream);
                return 0; // S_OK
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"GetFileData failed: {ex.Message}");
                return unchecked((int)0x80004005); // E_FAIL
            }
        }
    }
}
```

### Step 7: Add Build Events for COM+ Registration

Add to your `.csproj`:

```xml
<Target Name="PreBuild" BeforeTargets="PreBuildEvent">
  <Exec Command="C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe /u &quot;$(TargetPath)&quot; || exit /b 0" />
</Target>

<Target Name="PostBuild" AfterTargets="PostBuildEvent">
  <Exec Command="C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe &quot;$(TargetPath)&quot;" />
</Target>
```

---

## NuGet Dependencies and AssemblyResolver

### Quick Checklist: NuGet Dependencies

If your provider uses **any** NuGet packages, you **MUST** do ALL of the following:

- [x] **1. Add `AssemblyResolver.cs`** to your project (see template below)
- [x] **2. List all NuGet assemblies** in the `managedAssemblies` array
- [x] **3. Add static constructor** to `Provider.cs` that calls `AssemblyResolver.Initialize()`
- [x] **4. Create `app.config`** with binding redirects for version conflicts
- [x] **5. Add `<CopyLocalLockFileAssemblies>true</CopyLocalLockFileAssemblies>`** to `.csproj`

**Missing ANY of these will cause runtime failures!**

### Why AssemblyResolver Is Required

**CRITICAL:** Providers run in `dllhost.exe` (COM+ surrogate process), which does NOT
automatically resolve NuGet package dependencies. Without an `AssemblyResolver`, your
provider will fail at runtime with errors like:

```
Could not load file or assembly 'System.Text.Json, Version=9.0.0.5' or one of its dependencies.
```

Even though your project builds successfully, `dllhost.exe` doesn't know where to find
NuGet packages at runtime because:

1. COM+ applications don't use .deps.json files
2. `dllhost.exe` doesn't probe NuGet cache directories
3. Version-forwarding redirects in app.config alone are insufficient (AssemblyResolver is needed too)

### Solution: Custom AssemblyResolver

Every provider that uses NuGet packages **MUST** implement an `AssemblyResolver` class:

```csharp
// AssemblyResolver.cs

// <copyright file="AssemblyResolver.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.YourService
{
    using System;
    using System.IO;
    using System.Reflection;

    /// <summary>
    /// Provides assembly resolution for NuGet package dependencies.
    /// This resolves version conflicts when assemblies are loaded by COM+ (regsvcs/dllhost).
    /// </summary>
    /// <remarks>
    /// The static constructor ensures the resolver is registered before any other code runs,
    /// which is critical for COM+ ServicedComponents where assembly loading happens early.
    /// </remarks>
    internal static class AssemblyResolver
    {
        /// <summary>
        /// Static constructor ensures resolver is registered at type load time,
        /// before any other code in the assembly runs.
        /// </summary>
        static AssemblyResolver()
        {
            AppDomain.CurrentDomain.AssemblyResolve += ResolveAssembly;
        }

        /// <summary>
        /// Initializes the assembly resolver. Call this method early to ensure
        /// the static constructor has run.
        /// </summary>
        public static void Initialize()
        {
            // The static constructor does the work.
            // This method exists to provide an explicit initialization point.
        }

        /// <summary>
        /// Handles assembly resolution by loading assemblies from the same directory
        /// as the executing assembly, ignoring version mismatches.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="args">The event arguments containing the assembly name.</param>
        /// <returns>The resolved assembly, or null to let CLR continue resolution.</returns>
        private static Assembly ResolveAssembly(object sender, ResolveEventArgs args)
        {
            // Parse the requested assembly name
            AssemblyName requestedName = new AssemblyName(args.Name);

            // List of assemblies we handle
            string[] managedAssemblies = new string[]
            {
                // Add YOUR provider's NuGet dependencies here!
                // Example for a provider using System.Text.Json:
                "System.Text.Json",
                "System.Runtime.CompilerServices.Unsafe",
                "System.Memory",
                "System.Buffers",
                "System.Threading.Tasks.Extensions",
                "System.Text.Encodings.Web",
                "Microsoft.Bcl.AsyncInterfaces",
                "System.Numerics.Vectors",
                "System.ValueTuple"
            };

            foreach (string assemblyName in managedAssemblies)
            {
                if (requestedName.Name.Equals(assemblyName, StringComparison.OrdinalIgnoreCase))
                {
                    return TryLoadAssembly(assemblyName);
                }
            }

            return null;
        }

        /// <summary>
        /// Attempts to load an assembly from the executing assembly's directory.
        /// </summary>
        /// <param name="assemblyName">The simple name of the assembly.</param>
        /// <returns>The loaded assembly, or null if not found.</returns>
        private static Assembly TryLoadAssembly(string assemblyName)
        {
            // First, check if it's already loaded
            foreach (Assembly loaded in AppDomain.CurrentDomain.GetAssemblies())
            {
                if (loaded.GetName().Name.Equals(assemblyName, StringComparison.OrdinalIgnoreCase))
                {
                    return loaded;
                }
            }

            // Try to load from the same directory as this assembly
            string executingPath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            string assemblyPath = Path.Combine(executingPath, assemblyName + ".dll");

            if (File.Exists(assemblyPath))
            {
                try
                {
                    return Assembly.LoadFrom(assemblyPath);
                }
                catch
                {
                    // Fall through to return null
                }
            }

            return null;
        }
    }
}
```

### Required .csproj Settings

Add this to your `.csproj` to ensure NuGet assemblies are copied to output:

```xml
<PropertyGroup>
  <CopyLocalLockFileAssemblies>true</CopyLocalLockFileAssemblies>
</PropertyGroup>
```

This copies **all** NuGet package DLLs (including transitive dependencies) to your
output directory, making them available for `AssemblyResolver` to load.

### Why BOTH AssemblyResolver AND app.config Are Needed

**You need BOTH** - they solve different problems:

| Component | Problem It Solves | When It's Used |
|-----------|-------------------|----------------|
| **AssemblyResolver** | Finds assemblies not in standard probe paths | When CLR can't locate a DLL at all |
| **app.config** | Redirects version requests to actual versions | When CLR finds DLL but version doesn't match |

**Example scenario:**

1. Your provider references `System.Text.Json 9.0.5`
2. ConfigProvider (dependency) was built with `System.Text.Json 9.0.0.11`
3. At runtime, ConfigProvider asks for version 9.0.0.11
4. Your bin folder only has version 9.0.5 (from NuGet restore)

**What happens:**

- **Without AssemblyResolver:** CLR can't find the DLL → "Could not load file or assembly"
- **Without app.config:** CLR finds DLL but rejects it → "manifest definition does not match"
- **With BOTH:** CLR uses AssemblyResolver to find the DLL, then uses binding redirect to accept the version mismatch

**Bottom line:** Always include both! Copy from an existing provider (Archive, Zip, ISO).

### Examples by Provider Type

#### Archive Provider (SharpCompress + System.Text.Json)

```csharp
string[] managedAssemblies = new string[]
{
    // SharpCompress and its dependencies
    "SharpCompress",
    // System.Text.Json for IBigDriveDriveInfo.GetDriveParameters()
    "System.Text.Json",
    "System.Runtime.CompilerServices.Unsafe",
    "System.Memory",
    "System.Buffers",
    "System.Threading.Tasks.Extensions",
    "System.Text.Encodings.Web",
    "Microsoft.Bcl.AsyncInterfaces",
    "System.Numerics.Vectors"
};
```

#### ISO Provider (DiscUtils + System.Text.Json)

```csharp
string[] managedAssemblies = new string[]
{
    // DiscUtils and its dependencies
    "DiscUtils.Core",
    "DiscUtils.Streams",
    "DiscUtils.Iso9660",
    // System.Text.Json for IBigDriveDriveInfo.GetDriveParameters()
    "System.Text.Json",
    "System.Runtime.CompilerServices.Unsafe",
    "System.Memory",
    "System.Buffers",
    "System.Threading.Tasks.Extensions",
    "System.Text.Encodings.Web",
    "Microsoft.Bcl.AsyncInterfaces",
    "System.Numerics.Vectors",
    "System.ValueTuple"
};
```

#### Cloud Provider (Azure.Storage.Blobs + Newtonsoft.Json)

```csharp
string[] managedAssemblies = new string[]
{
    // Azure SDK dependencies
    "Azure.Core",
    "Azure.Storage.Blobs",
    "Azure.Storage.Common",
    // Newtonsoft.Json (if using)
    "Newtonsoft.Json",
    "System.Text.Json",
    "System.Memory",
    "System.Buffers",
    // ... etc
};
```

### How to Find Required Assemblies

1. **Build your project** - NuGet will restore packages
2. **Check your bin\Debug folder** - Look for all non-Microsoft DLLs
3. **Run regsvcs.exe** - If registration fails, check Windows Event Log for:
   ```
   Could not load file or assembly 'YourPackage, Version=...'
   ```
4. **Add missing assemblies** to the `managedAssemblies` array

### Common Dependencies to Include

If your provider uses `IBigDriveDriveInfo.GetDriveParameters()`, you **MUST** include
System.Text.Json and its dependencies:

```csharp
// Always include these if you implement IBigDriveDriveInfo
"System.Text.Json",
"System.Runtime.CompilerServices.Unsafe",
"System.Memory",
"System.Buffers",
"System.Threading.Tasks.Extensions",
"System.Text.Encodings.Web",
"Microsoft.Bcl.AsyncInterfaces"
```

### Triggering the Static Constructor

**CRITICAL:** The `AssemblyResolver` static constructor **must run BEFORE** COM+ loads
any types that reference NuGet assemblies. To ensure this, add a static constructor to
your `Provider` class that explicitly initializes the resolver:

```csharp
// In Provider.cs, add this static constructor BEFORE any other members:

public partial class Provider : ServicedComponent,
    IProcessInitializer,
    IBigDriveRegistration,
    IBigDriveEnumerate
{
    private static readonly BigDriveTraceSource DefaultTraceSource = BigDriveTraceSource.Instance;

    /// <summary>
    /// Static constructor to ensure AssemblyResolver is initialized early.
    /// CRITICAL: This must run before any COM+ registration code executes.
    /// </summary>
    static Provider()
    {
        // Force AssemblyResolver static constructor to run
        AssemblyResolver.Initialize();
    }

    // ... rest of your Provider class
}
```

**Why this is required:**

- Without explicit initialization, the AssemblyResolver may not register in time
- During `regsvcs.exe` registration, COM+ loads your Provider class before executing any code
- If System.Text.Json (or other NuGet dependencies) are needed during type loading, 
  the resolver must already be registered
- The static constructor runs **before** any static members are accessed

**Symptoms without this:**

```
Failed to register assembly 'YourProvider, Version=1.0.0.0'
Exception has been thrown by the target of an invocation.
Could not load file or assembly 'System.Text.Json, Version=...'
```

### Required app.config with Binding Redirects

**CRITICAL:** In addition to AssemblyResolver, you **MUST** add an `app.config` file
with binding redirects. This tells COM+ which version of assemblies to use when there
are version conflicts.

Create `app.config` in your provider project root:

```xml
<?xml version="1.0" encoding="utf-8"?>
<configuration>
    <runtime>
        <assemblyBinding xmlns="urn:schemas-microsoft-com:asm.v1">
            <!-- System.Text.Json and dependencies (required for IBigDriveDriveInfo) -->
            <dependentAssembly>
                <assemblyIdentity name="System.Text.Json" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
                <bindingRedirect oldVersion="0.0.0.0-9.0.0.11" newVersion="9.0.0.11" />
            </dependentAssembly>
            <dependentAssembly>
                <assemblyIdentity name="System.Runtime.CompilerServices.Unsafe" publicKeyToken="b03f5f7f11d50a3a" culture="neutral" />
                <bindingRedirect oldVersion="0.0.0.0-6.0.0.0" newVersion="6.0.0.0" />
            </dependentAssembly>
            <dependentAssembly>
                <assemblyIdentity name="System.Memory" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
                <bindingRedirect oldVersion="0.0.0.0-4.0.5.0" newVersion="4.0.1.2" />
            </dependentAssembly>
            <dependentAssembly>
                <assemblyIdentity name="System.Buffers" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
                <bindingRedirect oldVersion="0.0.0.0-4.0.3.0" newVersion="4.0.3.0" />
            </dependentAssembly>
            <dependentAssembly>
                <assemblyIdentity name="System.Threading.Tasks.Extensions" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
                <bindingRedirect oldVersion="0.0.0.0-4.2.0.1" newVersion="4.2.0.1" />
            </dependentAssembly>
            <dependentAssembly>
                <assemblyIdentity name="System.Text.Encodings.Web" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
                <bindingRedirect oldVersion="0.0.0.0-9.0.0.0" newVersion="9.0.0.0" />
            </dependentAssembly>
            <dependentAssembly>
                <assemblyIdentity name="Microsoft.Bcl.AsyncInterfaces" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
                <bindingRedirect oldVersion="0.0.0.0-9.0.0.0" newVersion="9.0.0.0" />
            </dependentAssembly>
        </assemblyBinding>
    </runtime>
</configuration>
```

**Why app.config is required:**

- Different project references may request different versions of System.Text.Json
  (e.g., ConfigProvider wants 9.0.0.11, your provider references 9.0.0.5)
- Without binding redirects, `regsvcs.exe` fails with version mismatch errors
- The binding redirects tell the CLR "use version X for all requests from 0.0.0.0 to Y"

**How to determine the correct version numbers:**

1. Build your project successfully
2. Look in `bin\Debug\net472\` folder for the actual DLL versions
3. Use the **highest version present** in your binding redirects
4. Example: If you have `System.Text.Json.dll` version 9.0.0.11, use `newVersion="9.0.0.11"`

**Common version conflict error during regsvcs.exe:**

```
Failed to register assembly 'YourProvider'
Could not load file or assembly 'System.Text.Json, Version=4.0.1.2'
The located assembly's manifest definition does not match the assembly reference.
```

**Solution:** Add or update the binding redirect in app.config to redirect the requested
version (4.0.1.2) to the actual version present in your bin folder (e.g., 9.0.0.11).

### Testing AssemblyResolver

After building, test that assemblies resolve correctly:

```powershell
# Register the provider (requires elevation)
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "YourProvider.dll"

# Check Windows Event Log (Application) for errors
# Source: "BigDrive.Provider.YourService"

# If you see "Could not load file or assembly" errors, add that assembly to managedAssemblies
```

### Troubleshooting Registration Errors

#### Error: "Access to the registry key 'HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\Providers\...' is denied"

**Symptom:**
```
Failed to register assembly 'YourProvider'
Exception has been thrown by the target of an invocation.
Access to the registry key 'HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\Providers\{GUID}' is denied.
```

**Cause:** The `regsvcs.exe` PostBuild event needs Administrator privileges to write to
HKEY_LOCAL_MACHINE registry keys.

**Solution:** Run Visual Studio as Administrator, or manually register:

```powershell
# Open PowerShell/CMD as Administrator
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "path\to\YourProvider.dll"
```

#### Error: "Could not load file or assembly 'System.Text.Json, Version=...'"

**Symptom:**
```
Failed to register assembly 'YourProvider'
Could not load file or assembly 'System.Text.Json, Version=4.0.1.2'
The located assembly's manifest definition does not match the assembly reference.
```

**Cause:** Missing or incorrect binding redirects in `app.config`.

**Solution:** Ensure your `app.config` has binding redirects for all NuGet dependencies
(see "Required app.config with Binding Redirects" section above).

#### Error: Assembly loads but methods fail at runtime

**Symptom:** Provider registers successfully, but when called from BigDrive Shell:
```
Method threw exception: Could not load file or assembly 'YourPackage'
```

**Cause:** Missing assembly in the `AssemblyResolver.managedAssemblies` array.

**Solution:**
1. Check Windows Event Log (Application log, source "BigDrive.Provider.YourService")
2. Look for the specific assembly name that failed to load
3. Add it to the `managedAssemblies` array in `AssemblyResolver.cs`
4. Rebuild and re-register

#### Error: "Static constructor threw exception"

**Symptom:**
```
TypeInitializationException: The type initializer for 'Provider' threw an exception.
```

**Cause:** Error in the Provider static constructor (usually AssemblyResolver initialization).

**Solution:**
1. Check Windows Event Log for inner exception details
2. Ensure `AssemblyResolver.cs` exists and has no syntax errors
3. Verify the static constructor calls `AssemblyResolver.Initialize()`

---

## Project Structure

Your provider project should look like this:

```
BigDrive.Provider.YourService/
├── Properties/
│   └── AssemblyInfo.cs              # Assembly attributes, COM+ config
├── Provider.cs                       # Main class, GUID, interfaces list
├── Provider.IProcessInitializer.cs   # Startup/Shutdown
├── Provider.IBigDriveEnumerate.cs    # EnumerateFolders/EnumerateFiles
├── Provider.IBigDriveFileInfo.cs     # LastModifiedTime/GetFileSize
├── Provider.IBigDriveFileData.cs     # GetFileData (IStream)
├── Provider.IBigDriveFileOperations.cs # Copy/Delete/CreateDirectory
├── Provider.IBigDriveRegistration.cs # Register/Unregister
├── YourServiceClientWrapper.cs       # API client wrapper
├── BigDriveTraceSource.cs            # Logging (copy from Flickr provider)
├── ComStream.cs                      # IStream wrapper (copy from Flickr provider)
├── ProviderConfigurationFactory.cs   # Configuration loading
├── AssemblyResolver.cs               # NuGet dependency resolution (REQUIRED!)
├── app.config                        # Binding redirects for version conflicts (REQUIRED!)
└── BigDrive.Provider.YourService.csproj
```

---

## Interface Reference

### IBigDriveEnumerate

```csharp
public interface IBigDriveEnumerate
{
    // Return folder names (not paths) at the given path
    // For root, path may be null, empty, "\", or "/"
    string[] EnumerateFolders(Guid driveGuid, string path);

    // Return file names (not paths) at the given path
    string[] EnumerateFiles(Guid driveGuid, string path);
}
```

**Important:**
- Return empty array on errors, not null
- Sanitize names to remove invalid Windows filename characters
- Root path may be null, empty, "\\", "/", or "//"

### IBigDriveFileInfo

```csharp
public interface IBigDriveFileInfo
{
    // Return file's last modified time
    DateTime LastModifiedTime(Guid driveGuid, string path);

    // Return file size in bytes
    ulong GetFileSize(Guid driveGuid, string path);
}
```

### IBigDriveFileData

```csharp
public interface IBigDriveFileData
{
    // Return file content as IStream
    // Returns HRESULT: 0 = success, negative = error
    [PreserveSig]
    int GetFileData(Guid driveGuid, string path, out IStream stream);
}
```

**Important:**
- Use `ComStream` wrapper class to convert .NET Stream to IStream
- Return `0` (S_OK) on success
- Return `0x80004005` (E_FAIL) on error
- The `[PreserveSig]` attribute is required

### IBigDriveFileOperations

```csharp
public interface IBigDriveFileOperations
{
    void CopyFileToBigDrive(Guid driveGuid, string localFilePath, string bigDriveTargetPath);
    void CopyFileFromBigDrive(Guid driveGuid, string bigDriveFilePath, string localTargetPath);
    void DeleteFile(Guid driveGuid, string bigDriveFilePath);
    void CreateDirectory(Guid driveGuid, string bigDriveDirectoryPath);
}
```

---

## Testing Your Provider

### 1. Build and Register

```powershell
# Build in Visual Studio (PostBuild will run regsvcs.exe)
# Or manually:
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "path\to\YourProvider.dll"
```

### 2. Verify in Component Services

1. Open `comexp.msc` (Component Services)
2. Navigate to: Component Services → Computers → My Computer → COM+ Applications
3. Find `BigDrive.Service` (or your assembly name)
4. Expand Components and verify your provider appears

### 3. Register a Test Drive

Add a registry key under `HKLM\SOFTWARE\BigDrive\Drives\{GUID}`:
- `id` = `{your-drive-guid}`
- `name` = `Your Service Name`
- `clsid` = `{your-provider-clsid}`

### 4. Test with BigDrive.Shell

```powershell
.\BigDrive.Shell.exe

BD> dir
    <DRIVE>  Z:  Your Service Name

BD> cd Z:
Z:\> dir
    <DIR>    Folder1
    <DIR>    Folder2
             File1.txt

Z:\> copy File1.txt C:\Downloads\File1.txt
        1 file(s) copied.
```

---

## Best Practices

### Error Handling

- Never throw exceptions across COM boundaries in `IBigDriveFileData.GetFileData`
- Return HRESULT error codes instead
- Log all errors to trace source
- Return empty arrays instead of null for enumeration methods

### Performance

- Cache API responses where appropriate
- Implement lazy loading for large directories
- Consider pagination for APIs that support it

### Logging

- Use `BigDriveTraceSource` for consistent logging
- Log entry/exit of methods with parameters
- Log errors with full exception details

### Thread Safety

- COM+ may call your provider from multiple threads
- Use thread-safe collections or locking
- Make static fields immutable or thread-safe

### Drive-Specific Configuration

Providers should support **drive-specific configuration** to allow multiple drives
using the same provider with different credentials/settings.

**Configuration priority** (highest to lowest):
1. Drive-specific: `SOFTWARE\BigDrive\Drives\{GUID}\PropertyName`
2. Provider-level: `SOFTWARE\BigDrive\Providers\{CLSID}\PropertyName`
3. Hard-coded default

**Implementation pattern:**

```csharp
// In your API client wrapper
private static string GetDriveConfigValue(Guid driveGuid, string key, string defaultValue)
{
    // First, try drive-specific configuration
    if (driveGuid != Guid.Empty)
    {
        try
        {
            string value = DriveManager.ReadDriveProperty(driveGuid, key, CancellationToken.None);
            if (!string.IsNullOrEmpty(value))
            {
                return value;
            }
        }
        catch
        {
            // Fall through to provider-level config
        }
    }

    // Fall back to provider-level configuration
    return GetProviderConfigValue(key, defaultValue);
}
```

**Cache clients per drive:**

```csharp
private static readonly ConcurrentDictionary<Guid, ApiClientWrapper> DriveClients =
    new ConcurrentDictionary<Guid, ApiClientWrapper>();

public static ApiClientWrapper GetForDrive(Guid driveGuid)
{
    return DriveClients.GetOrAdd(driveGuid, guid => new ApiClientWrapper(guid));
}
```

**Use driveGuid in interface methods:**

```csharp
public string[] EnumerateFolders(Guid driveGuid, string path)
{
    // Get the client configured for this specific drive
    ApiClientWrapper client = ApiClientWrapper.GetForDrive(driveGuid);

    // Now use the client...
}
```

### Configuration Properties for Common Providers

| Provider Type | Common Properties |
|---------------|-------------------|
| Cloud Storage | ApiKey, ApiSecret, AccountId, Region |
| OAuth Services | OAuthToken, OAuthSecret, RefreshToken, ExpiresAt |
| Database | ConnectionString, DatabaseName, Username, Password |
| Network | ServerUrl, Username, Password, Domain |

---

## Common Pitfalls

| Issue | Solution |
|-------|----------|
| Provider not appearing in Component Services | Verify `[ComVisible(true)]` and `[ApplicationActivation(Server)]` |
| "Class not registered" errors | Run `regsvcs.exe` as Administrator |
| IStream not working | Use `ComStream` wrapper, verify `[PreserveSig]` on interface |
| Invalid characters in filenames | Sanitize all names with `Path.GetInvalidFileNameChars()` |
| Root path handling | Check for null, empty, "\\", "/", and "//" |
| Provider crashes Explorer | Ensure all exceptions are caught at interface boundaries |
| Same credentials for all drives | Use `GetDriveConfigValue(driveGuid, key)` pattern |
| Static client with shared state | Use `ConcurrentDictionary` to cache clients per drive |

---

## Reference Implementations

Study these existing providers:

| Provider | Location | Notes |
|----------|----------|-------|
| Flickr | `src/BigDrive.Provider.Flickr/` | Full implementation, drive-specific config |
| Sample | `src/BigDrive.Provider.Sample/` | Simple example, read/write |

---

## Checklist

Before submitting your provider:

- [ ] Project named `BigDrive.Provider.{ServiceName}`
- [ ] Unique GUID for assembly (`AssemblyInfo.cs`)
- [ ] Unique CLSID for Provider class
- [ ] `[ComVisible(true)]` on assembly and class
- [ ] `[ApplicationActivation(ActivationOption.Server)]` on assembly
- [ ] Inherits from `ServicedComponent`
- [ ] Implements `IProcessInitializer`
- [ ] Implements `IBigDriveEnumerate` at minimum
- [ ] **Uses `driveGuid` parameter** for drive-specific configuration
- [ ] One partial class file per interface
- [ ] Copyright headers on all files
- [ ] XML documentation on all public methods
- [ ] Error handling - no exceptions across COM boundaries
- [ ] Name sanitization for invalid characters
- [ ] Trace logging throughout
- [ ] README.txt in provider folder
- [ ] Build events for regsvcs.exe

---

## See Also

- [Provider Development Practices](ProviderDevelopmentPractices.md) — Build/debug workflow, DLL locking, COM+ management
- [BigDrive.Setup README](../src/BigDrive.Setup/README.txt) — COM+ registration architecture
- [BigDrive.Interfaces README](../src/Interfaces/README.txt) — Interface definitions
- [BigDrive.Shell User Guide](BigDrive.Shell.UserGuide.md) — Testing with BigDrive.Shell
- [BigDrive.Provider.Flickr](../src/BigDrive.Provider.Flickr/) — Reference implementation

---

*Copyright © Wayne Walter Berry. All rights reserved.*
