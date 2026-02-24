# BigDrive Provider Development Guide

## Overview

This guide explains how to create a new BigDrive provider that exposes external storage
(cloud services, APIs, databases, etc.) as a virtual file system in Windows Explorer.

BigDrive providers are COM+ ServicedComponents that implement a set of interfaces defined
in `BigDrive.Interfaces`. Providers run out-of-process in `dllhost.exe` for isolation and
are activated via COM when the Shell or BigDrive.Shell needs to enumerate files or read data.

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────────┐
│  Windows Explorer / BigDrive.Shell                                     │
│                                                                         │
│  CoCreateInstance(Provider CLSID)                                       │
│                          │                                              │
└──────────────────────────┼──────────────────────────────────────────────┘
                           │ COM Activation
                           ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  dllhost.exe (COM+ Surrogate)                                           │
│                                                                         │
│  ┌─────────────────────┐  ┌─────────────────────┐                       │
│  │ Your.Provider       │  │ Provider.Flickr     │  ...                  │
│  │ (ServicedComponent) │  │ (ServicedComponent) │                       │
│  └─────────────────────┘  └─────────────────────┘                       │
│                                                                         │
│  Identity: Interactive User (logged-in user)                            │
│  - Access to user's Credential Manager for secrets                      │
│  - Access to HKCU registry                                              │
└─────────────────────────────────────────────────────────────────────────┘
                           │
                           │ Your API Calls
                           ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  External Storage (Cloud API, Database, Network Share, etc.)            │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Provider Registration Flow

Providers are **self-registering** using the `[ComRegisterFunction]` attribute.
When you run `regsvcs.exe` (elevated), the following happens automatically:

```
regsvcs.exe YourProvider.dll (Run As Administrator)
       │
       ├─► 1. Create COM+ Application
       │       - Reads [ApplicationActivation(Server)] attribute
       │       - Registers CLSIDs in HKCR\CLSID
       │       - Creates COM+ App: "BigDrive.Provider.YourService"
       │
       └─► 2. Call [ComRegisterFunction] method
               │
               ├─► SetApplicationIdentityToInteractiveUser()
               │       - Uses COMAdmin catalog API
               │       - Sets identity = "Interactive User"
               │       - Enables access to user's Credential Manager
               │
               └─► Provider.Register()
                       - ProviderManager.RegisterProvider()
                       - DriveManager.WriteConfiguration()
```

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
