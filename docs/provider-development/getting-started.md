# Getting Started with BigDrive Provider Development

## Overview

This guide walks you through creating a new BigDrive provider that exposes external storage
(cloud services, APIs, databases, archive files, etc.) as a virtual file system in Windows Explorer.

BigDrive providers are COM+ ServicedComponents that implement interfaces defined
in `BigDrive.Interfaces`. Providers run out-of-process in `dllhost.exe` for isolation and
are activated via COM when the Shell or BigDrive.Shell needs to enumerate files or read data.

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

**Key architectural principles:**

- **Out-of-process isolation**: Providers run in separate `dllhost.exe` processes, not in `explorer.exe`
- **Interactive User identity**: Providers run as the logged-in user, enabling Credential Manager access
- **COM+ activation**: Automatic process lifetime management and COM marshaling
- **Per-drive configuration**: Each mounted drive has its own configuration (API keys, file paths, etc.)

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
                       - Writes to HKLM\SOFTWARE\BigDrive\Providers\{CLSID}
```

**After registration**, the provider is available system-wide and users can create drives via BigDrive.Shell.

---

## Naming Conventions

### Project Naming

| Component | Convention | Example |
|-----------|------------|---------|
| Project Name | `BigDrive.Provider.{ServiceName}` | `BigDrive.Provider.AzureBlob` |
| Namespace | `BigDrive.Provider.{ServiceName}` | `BigDrive.Provider.AzureBlob` |
| Assembly Name | `BigDrive.Provider.{ServiceName}` | `BigDrive.Provider.AzureBlob.dll` |
| COM+ Application | `BigDrive.Provider.{ServiceName}` | `BigDrive.Provider.AzureBlob` |

**ServiceName guidelines:**
- Use PascalCase (e.g., `AzureBlob`, not `azure-blob`)
- Use the official service name (e.g., `Flickr`, not `FlickrStorage`)
- For file formats, use the format name (e.g., `Zip`, `Iso`, `Archive`)

### File Naming (Partial Classes)

We use **one partial class file per interface** for maintainability:

| File | Contents | Required |
|------|----------|----------|
| `Provider.cs` | Main class declaration, GUID, shared fields | ✅ Yes |
| `Provider.IProcessInitializer.cs` | `Startup()` and `Shutdown()` methods | ✅ Yes |
| `Provider.IBigDriveRegistration.cs` | `Register()` and `Unregister()` methods | ✅ Yes |
| `Provider.IBigDriveEnumerate.cs` | `EnumerateFolders()` and `EnumerateFiles()` methods | ✅ Yes |
| `Provider.IBigDriveDriveInfo.cs` | `GetDriveParameters()` method | Recommended |
| `Provider.IBigDriveFileInfo.cs` | `LastModifiedTime()` and `GetFileSize()` methods | Optional |
| `Provider.IBigDriveFileOperations.cs` | `CopyFileToBigDrive()`, `CopyFileFromBigDrive()`, etc. | Optional |
| `Provider.IBigDriveFileData.cs` | `GetFileData()` method (returns IStream) | Optional |
| `Provider.IBigDriveAuthentication.cs` | OAuth authentication methods | Optional |

**Why partial classes?**
- Each file is focused on a single interface
- Easy to navigate and maintain
- Matches the interface-per-file pattern used throughout BigDrive

### Helper Classes

| File | Purpose | Required |
|------|---------|----------|
| `{ServiceName}ClientWrapper.cs` | API client wrapper (e.g., `FlickrClientWrapper.cs`) | ✅ Yes |
| `BigDriveTraceSource.cs` | Event log tracing | ✅ Yes |
| `AssemblyResolver.cs` | NuGet dependency resolution for COM+ | ✅ Yes (if using NuGet) |
| `ProviderConfigurationFactory.cs` | Provider configuration factory | ✅ Yes |
| `ComStream.cs` | IStream wrapper for file streaming | ✅ Yes (if IBigDriveFileData) |
| `app.config` | Assembly binding redirects | ✅ Yes (if using NuGet) |

---

## Step-by-Step: Creating a New Provider

### Step 1: Create the Project

1. **Create a new Class Library project** targeting **.NET Framework 4.7.2**
2. Name it `BigDrive.Provider.{YourService}`
3. **Add project references:**
   - `BigDrive.Interfaces` - COM interface definitions
   - `BigDrive.ConfigProvider` - Registry configuration and DriveManager

   > **Do not** reference `BigDrive.Service`. Providers and the Service are independent
   > COM+ applications that communicate only via COM activation. See
   > [Architecture — Dependency Rules](../architecture/components.md#dependency-rules).
4. **Add framework references:**
   - `System.EnterpriseServices` - COM+ ServicedComponent base class
5. **Configure signing:**
   - Set `<SignAssembly>true</SignAssembly>`
   - Set `<AssemblyOriginatorKeyFile>..\..\BigDrive.snk</AssemblyOriginatorKeyFile>`

**Example .csproj:**

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net472</TargetFramework>
    <RootNamespace>BigDrive.Provider.YourService</RootNamespace>
    <AssemblyName>BigDrive.Provider.YourService</AssemblyName>
    <GenerateAssemblyInfo>false</GenerateAssemblyInfo>
    <SignAssembly>true</SignAssembly>
    <AssemblyOriginatorKeyFile>..\..\BigDrive.snk</AssemblyOriginatorKeyFile>
    <CopyLocalLockFileAssemblies>true</CopyLocalLockFileAssemblies>
  </PropertyGroup>

  <ItemGroup>
    <!-- Add your NuGet packages here -->
    <PackageReference Include="YourPackage" Version="1.0.0" />
  </ItemGroup>

  <ItemGroup>
    <Reference Include="Microsoft.CSharp" />
    <Reference Include="System.Configuration.Install" />
    <Reference Include="System.EnterpriseServices" />
  </ItemGroup>

  <ItemGroup>
    <ProjectReference Include="..\ConfigProvider\BigDrive.ConfigProvider.csproj" />
    <ProjectReference Include="..\Interfaces\BigDrive.Interfaces.csproj" />
  </ItemGroup>

  <Target Name="PreBuild" BeforeTargets="PreBuildEvent">
    <Exec Command="cmd.exe C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe /u &quot;$(TargetPath)&quot; || exit /b 0" />
  </Target>

  <Target Name="PostBuild" AfterTargets="PostBuildEvent">
    <Exec Command="C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe &quot;$(TargetPath)&quot;" />
  </Target>
</Project>
```

### Step 2: Configure AssemblyInfo.cs

Create `Properties/AssemblyInfo.cs`:

```csharp
// <copyright file="AssemblyInfo.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

using System.EnterpriseServices;
using System.Reflection;
using System.Runtime.InteropServices;

[assembly: AssemblyTitle("YourService")]
[assembly: AssemblyDescription("BigDrive provider for YourService")]
[assembly: AssemblyConfiguration("")]
[assembly: AssemblyCompany("Your Company")]
[assembly: AssemblyProduct("BigDrive")]
[assembly: AssemblyCopyright("Copyright © Your Company 2025")]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]

// CRITICAL: ComVisible must be true for COM+ hosting
[assembly: ComVisible(true)]

// The following GUID is for the ID of the typelib if this project is exposed to COM
// Generate a unique GUID using Tools -> Create GUID in Visual Studio
[assembly: Guid("YOUR-UNIQUE-TYPELIB-GUID-HERE")]

[assembly: AssemblyVersion("1.0.0.0")]
[assembly: AssemblyFileVersion("1.0.0.0")]

// COM+ Server activation (runs in dllhost.exe)
[assembly: ApplicationActivation(ActivationOption.Server)]
[assembly: ApplicationAccessControl(false)] // Optional: Disable role-based security
```

**Important:**
- Generate a **unique GUID** for the `[Guid]` attribute (this is the TypeLib GUID)
- This is different from the Provider's CLSID (which goes in Provider.cs)
- Use Visual Studio's Tools → Create GUID feature

### Step 3: Create the Main Provider Class

Create `Provider.cs`:

```csharp
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
    /// <remarks>
    /// This provider supports drive-specific configuration. Each drive can have
    /// different credentials, paths, or settings.
    ///
    /// Drive-specific properties (stored in registry under each drive):
    /// - Document your provider's configuration properties here
    /// </remarks>
    [Guid("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")]  // Generate unique CLSID!
    [ClassInterface(ClassInterfaceType.None)]
    [ComVisible(true)]
    public partial class Provider : ServicedComponent,
        IProcessInitializer,
        IBigDriveRegistration,
        IBigDriveDriveInfo,
        IBigDriveEnumerate,
        IBigDriveFileInfo,
        IBigDriveFileData
        // Add other interfaces as needed (IBigDriveFileOperations, IBigDriveAuthentication)
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

        /// <summary>
        /// Sets the identity of a COM+ application to "Interactive User".
        /// This allows the provider to run as the logged-in user and access Credential Manager.
        /// </summary>
        /// <param name="applicationName">The name of the COM+ application.</param>
        private static void SetApplicationIdentityToInteractiveUser(string applicationName)
        {
            DefaultTraceSource.TraceInformation($"Setting COM+ application '{applicationName}' identity to Interactive User");

            Type comAdminType = Type.GetTypeFromProgID("COMAdmin.COMAdminCatalog");
            if (comAdminType == null)
            {
                DefaultTraceSource.TraceError("COMAdminCatalog is not available on this system.");
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

                        DefaultTraceSource.TraceInformation($"COM+ application '{applicationName}' identity set to 'Interactive User'.");
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

        /// <summary>
        /// Gets the client wrapper for a specific drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>A client wrapper configured for the drive.</returns>
        private static YourServiceClientWrapper GetClient(Guid driveGuid)
        {
            return YourServiceClientWrapper.GetForDrive(driveGuid);
        }

        /// <summary>
        /// Normalizes a path by trimming leading/trailing separators and converting to forward slashes.
        /// Returns an empty string for root paths.
        /// </summary>
        /// <param name="path">The path to normalize.</param>
        /// <returns>The normalized path, or empty string for root.</returns>
        private static string NormalizePath(string path)
        {
            if (string.IsNullOrEmpty(path) || path == "\\" || path == "/" || path == "//")
            {
                return string.Empty;
            }

            return path.Trim('\\', '/').Replace('\\', '/');
        }
    }
}
```

**Key components:**

- **Static constructor**: Initializes AssemblyResolver before any code runs
- **CLSID property**: Returns the provider's unique GUID
- **ProviderConfig property**: Factory for provider configuration
- **ComRegister/ComUnregister**: Called by regsvcs.exe
- **SetApplicationIdentityToInteractiveUser**: Critical for Credential Manager access
- **GetClient helper**: Returns drive-specific client instances
- **NormalizePath helper**: Standardizes path format across interfaces

---

## Project Structure

Your provider project should follow this structure:

```
BigDrive.Provider.YourService/
├── Properties/
│   └── AssemblyInfo.cs                    # Assembly attributes, COM+ config
├── Provider.cs                             # Main class, GUID, shared helpers
├── Provider.IProcessInitializer.cs         # Startup/Shutdown
├── Provider.IBigDriveRegistration.cs       # Register/Unregister
├── Provider.IBigDriveDriveInfo.cs          # GetDriveParameters
├── Provider.IBigDriveEnumerate.cs          # EnumerateFolders/EnumerateFiles
├── Provider.IBigDriveFileInfo.cs           # LastModifiedTime/GetFileSize (optional)
├── Provider.IBigDriveFileData.cs           # GetFileData (optional)
├── Provider.IBigDriveFileOperations.cs     # Copy/Delete/Mkdir (optional)
├── Provider.IBigDriveAuthentication.cs     # OAuth methods (optional)
├── YourServiceClientWrapper.cs             # API client wrapper
├── BigDriveTraceSource.cs                  # Logging
├── ComStream.cs                            # IStream wrapper (if IBigDriveFileData)
├── ProviderConfigurationFactory.cs         # Configuration factory
├── AssemblyResolver.cs                     # NuGet dependency resolution (REQUIRED if using NuGet!)
├── app.config                              # Assembly binding redirects (REQUIRED if using NuGet!)
├── README.md                               # Provider documentation
└── BigDrive.Provider.YourService.csproj    # Project file
```

### Event Source Registration

> **IMPORTANT:** Your provider's `BigDriveTraceSource` writes to the Windows Event Log under
> the `BigDrive` log with a source name matching your provider (e.g., `BigDrive.Provider.YourService`).
> This event source **must be registered** by `BigDrive.Setup` before logs will appear in Event Viewer.

Add your provider's event source to `BigDrive.Setup`:

1. **Add a constant** in `src/BigDrive.Setup/Constants.cs`:

   ```csharp
   public const string EventLogProviderYourService = "Provider.YourService";
   ```

2. **Register it** in `src/BigDrive.Setup/Program.cs` inside `BootstrapBigDriveEventLogs()`:

   ```csharp
   BootstrapBigDriveEventLog(Constants.EventLogProviderYourService);
   ```

3. **Re-run** `BigDrive.Setup.exe` as Administrator to create the event source.

After registration, logs appear in **Event Viewer → Applications and Services Logs → BigDrive**.

Without this step, the `EventLogTraceListener` in `BigDriveTraceSource` will silently fail
and no trace output will be visible.

---

Every provider **must** implement these interfaces:

| Interface | Purpose | Methods |
|-----------|---------|---------|
| `IProcessInitializer` | COM+ lifecycle hooks | `Startup(object)`, `Shutdown()` |
| `IBigDriveRegistration` | Self-registration | `Register()`, `Unregister()` |
| `IBigDriveEnumerate` | List folders and files | `EnumerateFolders(Guid, string)`, `EnumerateFiles(Guid, string)` |

**Recommended interface:**

| Interface | Purpose | When to Implement |
|-----------|---------|-------------------|
| `IBigDriveDriveInfo` | Drive parameter definitions | Always implement to declare required configuration |

**Optional interfaces** (implement based on your provider's capabilities):

| Interface | Purpose | When to Implement |
|-----------|---------|-------------------|
| `IBigDriveFileInfo` | File metadata | If you can provide file sizes and dates |
| `IBigDriveFileOperations` | Copy/delete/mkdir | If provider supports write operations |
| `IBigDriveFileData` | Stream file content | If files can be downloaded/read |
| `IBigDriveAuthentication` | OAuth authentication | If provider requires OAuth login |

See [interfaces.md](interfaces.md) for detailed interface documentation.

---

## Next Steps

1. **[Read Interfaces Guide](interfaces.md)** - Understand all available interfaces and their signatures
2. **[Setup NuGet Dependencies](nuget-dependencies.md)** - Critical! Learn about AssemblyResolver and app.config
3. **[Implement OAuth (if needed)](oauth-authentication.md)** - For cloud services requiring authentication
4. **[See Complete Examples](examples.md)** - Full provider implementations to reference
5. **[Troubleshooting](troubleshooting.md)** - Common errors and solutions

---

## Quick Reference: Existing Providers

Learn by example - study these existing providers:

| Provider | Type | Complexity | Key Features |
|----------|------|------------|--------------|
| **Zip** | Local files | Simple | Read/write ZIP archives |
| **Archive** | Local files | Medium | Multi-format (ZIP, TAR, 7z, RAR) using SharpCompress |
| **Iso** | Local files | Simple | Read-only ISO 9660/UDF using DiscUtils |
| **Flickr** | Cloud API | Complex | OAuth 1.0a, API pagination, photo metadata |

**Recommendation:** Start with the ISO or Zip provider as templates for local file providers,
or Flickr for cloud API providers.

---

## See Also

- [BigDrive Architecture Overview](../architecture/overview.md)
- [BigDrive Shell User Guide](../BigDrive.Shell.UserGuide.md)
- [Interface Definitions](../../src/Interfaces/README.txt)
