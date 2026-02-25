# BigDrive Architecture Overview

## What is BigDrive?

BigDrive exposes cloud storage services (Flickr, Azure Blob, etc.) as virtual file systems
in Windows. Users can browse and manage remote files using familiar tools like Windows
Explorer or BigDrive.Shell.

---

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              USER INTERFACES                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌───────────────────────┐         ┌───────────────────────┐                │
│  │   Windows Explorer    │         │   BigDrive.Shell      │                │
│  │                       │         │                       │                │
│  │  BigDrive.ShellFolder │         │  Command-line shell   │                │
│  │  (Shell Extension)    │         │  dir, cd, copy, mount │                │
│  └───────────┬───────────┘         └────┬─────────────┬────┘                │
│              │                          │             │                     │
│              │ IShellFolder             │ IBigDrive*  │ IBigDriveProvision  │
│              │ IEnumIDList              │ (providers) │ (service)           │
│              │                          │             │                     │
│              └──────────┬───────────────┘             │                     │
│                         │                             │                     │
│           CoCreateInstance(CLSID)        CoCreateInstance(CLSID)           │
│                         │                             │                     │
└─────────────────────────┼─────────────────────────────┼─────────────────────┘
                          │                             │
              COM+ Activation               COM+ Activation
              (out-of-process)              (out-of-process)
                          │                             │
                          ▼                             ▼
┌──────────────────────────────────┐ ┌────────────────────────────────┐
│  COM+ APPLICATION (dllhost.exe)  │ │  COM+ APPLICATION (dllhost.exe)│
│  Providers                       │ │  BigDrive.Service              │
├──────────────────────────────────┤ ├────────────────────────────────┤
│                                  │ │                                │
│ ┌────────────────────┐           │ │  IBigDriveProvision            │
│ │ Provider.Flickr    │  ...      │ │  ├── Create()                  │
│ │ IBigDriveEnumerate │           │ │  ├── CreateFromConfiguration() │
│ │ IBigDriveFileData  │           │ │  └── UnmountDrive()            │
│ └────────┬───────────┘           │ │                                │
│          │                       │ │  Writes to HKCR, HKLM shell   │
│ Identity: Interactive User       │ │  namespace, and HKLM\BigDrive. │
└──────────┼───────────────────────┘ │  Refreshes Explorer.           │
           │                         │                                │
           ▼                         │ Identity: BigDriveInstaller    │
┌─────────────────────┐              │ (elevated local service acct)  │
│   Flickr API        │              └────────────────────────────────┘
│   Flickr API        │
│   (HTTPS)           │  ...other cloud APIs
└─────────────────────┘
```

---

## Key Concepts

### Providers vs Drives

| Concept | Description | Registry Location |
|---------|-------------|-------------------|
| **Provider** | COM+ component that knows *how* to access a storage backend | `SOFTWARE\BigDrive\Providers\{CLSID}` |
| **Drive** | User-created instance that uses a provider to access *specific* storage | `SOFTWARE\BigDrive\Drives\{GUID}` |

**Example**: One "Flickr Provider" can be used by multiple drives:
- "Personal Flickr" → user's personal account (with personal API key)
- "Work Flickr" → company account (with work API key)

### Registry Structure

```
HKLM\SOFTWARE\BigDrive\
│
├── Providers\                         ← Registered by COM+ installation
│   ├── {B3D8F2A1-...}\               ← Flickr Provider CLSID
│   │   ├── id   = "{B3D8F2A1-...}"
│   │   ├── name = "Flickr Provider"
│   │   ├── FlickrApiKey = "default-api-key"     ← Provider-level defaults
│   │   └── FlickrApiSecret = "default-secret"
│   │
│   └── {F8FE2E5A-...}\               ← Sample Provider CLSID
│       ├── id   = "{F8FE2E5A-...}"
│       └── name = "Sample Provider"
│
└── Drives\                            ← Created by user (mount command)
    ├── {6369DDE1-...}\               ← Drive GUID
    │   ├── id    = "{6369DDE1-...}"
    │   ├── name  = "My Personal Flickr"
    │   ├── clsid = "{B3D8F2A1-...}"  ← Points to Flickr Provider
    │   ├── FlickrApiKey = "personal-key"         ← Drive-specific override
    │   ├── FlickrOAuthToken = "oauth-token"
    │   └── FlickrOAuthSecret = "oauth-secret"
    │
    └── {A1B2C3D4-...}\               ← Another drive (same provider)
        ├── id    = "{A1B2C3D4-...}"
        ├── name  = "Work Flickr"
        ├── clsid = "{B3D8F2A1-...}"  ← Same provider
        ├── FlickrApiKey = "work-key"             ← Different credentials
        ├── FlickrOAuthToken = "work-oauth-token"
        └── FlickrOAuthSecret = "work-oauth-secret"
```

### Drive-Specific Configuration

Providers receive the `driveGuid` parameter in all interface methods. This allows
providers to load drive-specific configuration:

```csharp
// In provider implementation
public string[] EnumerateFolders(Guid driveGuid, string path)
{
    // Load configuration for this specific drive
    string apiKey = DriveManager.ReadDriveProperty(driveGuid, "FlickrApiKey", CancellationToken.None);

    // If not found on drive, falls back to provider-level default
    // ...
}
```

**Configuration priority** (highest to lowest):
1. Drive-specific value (`SOFTWARE\BigDrive\Drives\{GUID}\PropertyName`)
2. Provider-level default (`SOFTWARE\BigDrive\Providers\{CLSID}\PropertyName`)
3. Hard-coded default in provider code

---

## Component Overview

### User-Facing Components

| Component | Type | Purpose |
|-----------|------|---------|
| `BigDrive.Shell.exe` | .NET Console App | Command-line shell for BigDrive operations |
| `BigDrive.ShellFolder.dll` | C++ Shell Extension | Explorer integration (namespace extension) |

### Core Libraries

| Component | Type | Purpose |
|-----------|------|---------|
| `BigDrive.Interfaces.dll` | .NET Assembly | COM interface definitions for provider implementations |
| `BigDrive.Service.Interfaces.dll` | .NET Assembly | COM interface definitions for BigDrive.Service |
| `BigDrive.ConfigProvider.dll` | .NET Assembly | Registry read/write for drives and providers |

### Service (COM+ Component)

| Component | Type | Purpose |
|-----------|------|---------|
| `BigDrive.Service.dll` | ServicedComponent | Drive provisioning/deprovisioning with elevated registry access |

`BigDrive.Service` runs in its own COM+ application in `dllhost.exe` under the
**BigDriveInstaller** identity (a local service account created during setup).
It has write access to HKCR (COM class registration), HKLM shell namespace
(MyComputer\\NameSpace), and HKLM\\SOFTWARE\\BigDrive. The Shell activates it
via COM+ and calls `IBigDriveProvision` to create or unmount drives.
The Shell must **never** write to the registry directly.

This is distinct from providers, which run as **Interactive User** in their
own separate COM+ applications.

### Providers (COM+ Components)

| Component | Type | Purpose |
|-----------|------|---------|
| `BigDrive.Provider.Flickr.dll` | ServicedComponent | Flickr API integration |
| `BigDrive.Provider.Sample.dll` | ServicedComponent | Sample/testing provider |

### Setup

| Component | Type | Purpose |
|-----------|------|---------|
| `BigDrive.Setup.exe` | .NET Console App | COM+ registration and installation |

---

## Interface Hierarchy

Providers implement these interfaces from `BigDrive.Interfaces`:

```
IProcessInitializer          ← COM+ lifecycle (Startup/Shutdown)
│
├── IBigDriveEnumerate       ← Required: List folders and files
│   ├── EnumerateFolders()
│   └── EnumerateFiles()
│
├── IBigDriveFileInfo        ← Optional: File metadata
│   ├── GetFileSize()
│   └── LastModifiedTime()
│
├── IBigDriveFileData        ← Optional: File content streaming
│   └── GetFileData() → IStream
│
├── IBigDriveFileOperations  ← Optional: Write operations
│   ├── CopyFileToBigDrive()
│   ├── CopyFileFromBigDrive()
│   ├── DeleteFile()
│   └── CreateDirectory()
│
├── IBigDriveAuthentication  ← Optional: OAuth support
│   ├── GetAuthenticationInfo()    → Returns OAuth endpoints, flow type
│   ├── OnAuthenticationComplete() → Called after successful auth
│   └── IsAuthenticated()          → Check auth status
│
└── IBigDriveRegistration    ← Optional: Setup callbacks
    ├── Register()
    └── Unregister()
```

BigDrive.Service implements this interface from `BigDrive.Service.Interfaces`:

```
IBigDriveProvision           ← Drive provisioning (implemented by BigDrive.Service)
├── Create()                 ← Create a drive from existing registry config
├── CreateFromConfiguration()← Create a drive from JSON
└── UnmountDrive()           ← Remove drive config, shell folder, refresh Explorer
```

The Shell calls `IBigDriveProvision` via COM+ out-of-process activation
(`ServiceFactory.GetProvisionService()`). The Shell must never write to the
registry directly. BigDrive.Service runs under the **BigDriveInstaller** identity
which has the permissions needed to write to HKCR and HKLM. Both the Shell and
the Service reference `BigDrive.Service.Interfaces` for the interface types —
neither takes a direct project dependency on the other.

---

## Data Flow

### Enumeration Flow (dir command)

```
┌─────────────────┐
│ User types:     │
│ Z:\> dir        │
└────────┬────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│ BigDrive.Shell                                          │
│                                                         │
│  1. DriveLetterManager: Z: → DriveGuid {6369DDE1-...}  │
│  2. DriveManager.ReadConfiguration(DriveGuid)           │
│     → Returns CLSID {B3D8F2A1-...}                     │
│  3. ProviderFactory.GetEnumerateProvider(DriveGuid)     │
│     → CoCreateInstance(CLSID)                           │
└─────────────────────────┬───────────────────────────────┘
                          │
              COM Activation
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ dllhost.exe                                             │
│                                                         │
│  Provider.Flickr                                        │
│  4. IBigDriveEnumerate.EnumerateFolders(DriveGuid, "\")│
│     → Calls Flickr API                                  │
│     → Returns ["Vacation 2024", "Family Photos"]        │
│                                                         │
│  5. IBigDriveEnumerate.EnumerateFiles(DriveGuid, "\")  │
│     → Returns []                                        │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────┐
│ Output:         │
│  <DIR> Vacation │
│  <DIR> Family   │
└─────────────────┘
```

### File Copy Flow (copy command)

```
┌──────────────────────────┐
│ User types:              │
│ Z:\> copy photo.jpg C:\  │
└────────────┬─────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────┐
│ BigDrive.Shell                                          │
│                                                         │
│  1. Parse source: Z:\photo.jpg → BigDrive file          │
│  2. Parse dest: C:\ → Local file system                 │
│  3. Get provider via ProviderFactory                    │
│  4. Call IBigDriveFileData.GetFileData()                │
│     → Returns IStream                                   │
│  5. Write stream to C:\photo.jpg                        │
└─────────────────────────────────────────────────────────┘
```

---

## Process Isolation

Providers run out-of-process for:

1. **Stability**: Provider crash doesn't crash Explorer or Shell
2. **Security**: Providers run under restricted identity
3. **Resource Management**: COM+ manages pooling and lifetime

```
┌───────────────────┐     ┌───────────────────┐
│ explorer.exe      │     │ BigDrive.Shell.exe│
│ (User process)    │     │ (User process)    │
└─────────┬─────────┘     └─────────┬─────────┘
          │                         │
          │    COM+ Activation      │
          └────────────┬────────────┘
                       │
                       ▼
          ┌───────────────────────┐
          │ dllhost.exe           │
          │ (COM+ Surrogate)      │
          │                       │
          │ Identity:             │
          │ BigDriveTrustedInstaller
          └───────────────────────┘
```

---

## See Also

- [Installation Architecture](architecture/installation.md) — Setup and registration
- [Provider Development Guide](ProviderDevelopmentGuide.md) — Creating new providers
- [BigDrive.Shell User Guide](BigDrive.Shell.UserGuide.md) — Shell commands
- [Drag and Drop Scenarios](scenarios/drag_files.md) — Virtual file transfers

---

*Copyright © Wayne Walter Berry. All rights reserved.*
