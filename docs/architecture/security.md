# BigDrive Security Architecture

## Overview

This document describes the security model of the BigDrive system, including process
isolation, user accounts, registry permissions, credential storage, and COM+ activation.
Understanding this architecture is essential for security reviews and future development.

---

## System Components and Trust Boundaries

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                           USER SESSION (Interactive)                            │
│                                                                                 │
│  ┌───────────────────────────────────┐  ┌───────────────────────────────────┐  │
│  │        explorer.exe               │  │      BigDrive.Shell.exe           │  │
│  │        (User Context)             │  │      (User Context)               │  │
│  │                                   │  │                                   │  │
│  │  ┌─────────────────────────────┐  │  │  Runs as: Logged-in User          │  │
│  │  │ BigDrive.ShellFolder.dll   │  │  │  Can access:                      │  │
│  │  │ BigDrive.Client.dll        │  │  │   - HKCU registry                 │  │
│  │  │                             │  │  │   - User's Credential Manager    │  │
│  │  │ Runs as: Logged-in User     │  │  │   - User's file system           │  │
│  │  │ In-process: explorer.exe    │  │  │                                   │  │
│  │  └─────────────────────────────┘  │  └───────────────────────────────────┘  │
│  └───────────────────────────────────┘                                          │
│                    │                                    │                       │
│                    │ CoCreateInstance(CLSID)            │                       │
│                    │ CLSCTX_LOCAL_SERVER                │                       │
│                    └────────────────┬───────────────────┘                       │
│                                     │                                           │
└─────────────────────────────────────┼───────────────────────────────────────────┘
                                      │
                    ══════════════════╪══════════════════════
                         PROCESS BOUNDARY / TRUST BOUNDARY
                    ══════════════════╪══════════════════════
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────────┐
│                    dllhost.exe (COM+ Surrogate Processes)                       │
│                                                                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐   │
│  │ BigDrive.Service (Identity: BigDriveInstaller)                          │   │
│  │                                                                         │   │
│  │ Requires elevated permissions to:                                       │   │
│  │  - Write to HKCR\CLSID (COM class registration)                         │   │
│  │  - Write to HKLM shell namespace (MyComputer\NameSpace)                 │   │
│  │  - Register shell folders in "This PC"                                  │   │
│  └─────────────────────────────────────────────────────────────────────────┘   │
│                                                                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐   │
│  │ Providers (Identity: Interactive User)                                  │   │
│  │                                                                         │   │
│  │  ┌─────────────────────┐  ┌─────────────────────┐                       │   │
│  │  │ Provider.Flickr     │  │ Provider.Sample     │  ...                  │   │
│  │  └─────────────────────┘  └─────────────────────┘                       │   │
│  │                                                                         │   │
│  │ Runs as logged-in user, can access:                                     │   │
│  │  - User's Credential Manager (for API keys, OAuth tokens)               │   │
│  │  - User's HKCU registry                                                 │   │
│  │  - Network (for API calls to Flickr, Azure, etc.)                       │   │
│  │  - Event Log (BigDrive sources)                                         │   │
│  └─────────────────────────────────────────────────────────────────────────┘   │
│                                                                                 │
└─────────────────────────────────────────────────────────────────────────────────┘
```

---

## Dual-Identity Architecture

BigDrive uses a **dual-identity model** where different COM+ components run under
different security contexts based on their permission requirements:

| Component | COM+ Identity | Reason |
|-----------|---------------|--------|
| **BigDrive.Service** | BigDriveInstaller | Needs write access to HKCR and shell namespace |
| **Provider.Flickr** | Interactive User | Only needs read access + user's credentials |
| **Provider.Sample** | Interactive User | Only needs read access + user's credentials |

### Why Separate Identities?

1. **BigDrive.Service** creates shell folders visible in "This PC", which requires
   writing to protected registry locations (HKCR, HKLM shell namespace). These
   locations cannot be written by standard users.

2. **Providers** only need to read drive configuration and call external APIs.
   Running as Interactive User allows them to access the user's Credential Manager
   for secure credential storage.

---

## Installation Phase (Requires Administrator)

Installation is performed by `BigDrive.Setup.exe` running with Administrator privileges.
This is the only phase that requires elevation.

### Setup Flow

```
BigDrive.Setup.exe (Run As Administrator)
       │
       ├─► 1. Create Local Service Account
       │       Name: BigDriveInstaller
       │       Type: Local machine account
       │       Password: Auto-generated, discarded after setup
       │       Properties:
       │         - PasswordNeverExpires = true
       │         - UserCannotChangePassword = true
       │
       ├─► 2. Create Event Log Sources
       │       - BigDrive.Service
       │       - BigDrive.ShellFolder
       │       - BigDrive.Client
       │       - BigDrive.Provider.*
       │
       ├─► 3. Create Registry Structure
       │       HKLM\SOFTWARE\BigDrive\
       │       ├── Providers\    (populated by regsvcs.exe)
       │       └── Drives\       (populated by mount command)
       │
       ├─► 4. Grant Registry Permissions
       │       Grant BigDriveInstaller Full Control on:
       │         - HKLM\SOFTWARE\BigDrive
       │         - HKCR\CLSID (for COM registration)
       │         - HKCR\Component Categories\{00021493-...}
       │         - HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace
       │
       ├─► 5. Register COM+ Applications
       │       │
       │       ├─► BigDrive.Service (as BigDriveInstaller)
       │       │     regsvcs.exe BigDrive.Service.dll
       │       │     Identity = BigDriveInstaller
       │       │     Needs elevated permissions for shell namespace
       │       │
       │       └─► Providers (as Interactive User)
       │             regsvcs.exe BigDrive.Provider.Flickr.dll
       │             regsvcs.exe BigDrive.Provider.Sample.dll
       │             Identity = "Interactive User"
       │             Runs as logged-in user for credential access
       │
       └─► 6. Validate Installation
               - CoCreateInstance(BigDriveService CLSID)
               - Call IBigDriveSetup.Validate()
               - Verify Event Log entry
```

### Why Different COM+ Identities?

| Component | Identity | Reason |
|-----------|----------|--------|
| **BigDrive.Service** | BigDriveInstaller | Must write to HKCR and shell namespace (protected locations) |
| **Providers** | Interactive User | Only reads config, calls APIs, needs user's Credential Manager |

---

## Credential Storage

BigDrive uses a **tiered credential storage model** with Windows Credential Manager
for sensitive secrets.

### Storage Locations

| Data Type | Storage Location | Access |
|-----------|------------------|--------|
| Drive registration (id, name, clsid) | HKLM registry | All users can read |
| Non-sensitive config (paths, options) | HKLM registry | All users can read |
| **Secrets (API keys, OAuth tokens)** | **Credential Manager** | **Per-user, encrypted** |

### Credential Manager Integration

Providers running as Interactive User can access the logged-in user's Credential Manager:

```csharp
// Writing a secret (from BigDrive.Shell or mount command)
DriveManager.WriteSecretProperty(driveGuid, "FlickrApiKey", apiKey, token);
DriveManager.WriteSecretProperty(driveGuid, "FlickrOAuthToken", oauthToken, token);

// Reading a secret (from Provider)
string apiKey = DriveManager.ReadSecretProperty(driveGuid, "FlickrApiKey", token);
```

### Credential Target Name Format

Credentials are stored with target names in this format:
```
BigDrive:{DriveGuid}:{SecretName}
```

Example:
```
BigDrive:{6369DDE1-1234-5678-9ABC-DEF012345678}:FlickrApiKey
BigDrive:{6369DDE1-1234-5678-9ABC-DEF012345678}:FlickrOAuthToken
```

### Configuration Priority (Provider Read Order)

When a provider requests a configuration value, it checks in this order:

1. **Credential Manager** (secrets) - `DriveManager.ReadSecretProperty()`
2. **Registry (drive-specific)** - `HKLM\...\Drives\{GUID}\PropertyName`
3. **Registry (provider-level)** - `HKLM\...\Providers\{CLSID}\PropertyName`
4. **Hard-coded default** - in provider code

### Security Benefits

| Benefit | Explanation |
|---------|-------------|
| **Per-user isolation** | Each user's credentials are separate |
| **Encrypted at rest** | Windows encrypts Credential Manager data |
| **No registry exposure** | Secrets not readable by other users |
| **Standard Windows API** | Uses proven Windows security infrastructure |

---

## Runtime Phase (Standard User)

After installation, BigDrive operates without requiring elevation.

### Shell Extension Loading

```
1. User opens Windows Explorer
2. Explorer loads BigDrive.ShellFolder.dll (in-process)
3. ShellFolder reads drive configurations from HKLM\SOFTWARE\BigDrive\Drives
4. For each drive, ShellFolder calls CoCreateInstance(Provider CLSID)
5. COM+ activates provider in dllhost.exe as Interactive User (logged-in user)
6. Provider reads secrets from user's Credential Manager
7. Provider calls external APIs (Flickr, Azure, etc.)
8. Results returned to ShellFolder via COM interfaces
```

### COM Activation Details

The shell extension uses `CLSCTX_LOCAL_SERVER` for out-of-process activation:

```cpp
// From BigDriveInterfaceProvider.cpp
hr = ::CoCreateInstance(
    m_clsid,                    // Provider CLSID (e.g., Flickr Provider)
    nullptr,                    // No aggregation
    CLSCTX_LOCAL_SERVER,        // Out-of-process activation
    iid,                        // Requested interface (IBigDriveEnumerate, etc.)
    reinterpret_cast<void**>(&pIUnknown)
);
```

This ensures:
- Provider code runs in dllhost.exe, not explorer.exe
- Provider crash doesn't crash Explorer
- Provider runs with BigDriveInstaller credentials, not user credentials

### Interface Call Flow

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│ explorer.exe (User: Wayne)                                                      │
│                                                                                 │
│  BigDriveShellFolder::EnumObjects()                                             │
│    │                                                                            │
│    ├─► BigDriveConfigurationClient::GetDriveConfiguration(driveGuid)            │
│    │     └─► Reads HKLM\SOFTWARE\BigDrive\Drives\{GUID}                         │
│    │         Returns: { id, name, clsid, properties... }                        │
│    │                                                                            │
│    ├─► BigDriveInterfaceProvider provider(driveConfig.clsid)                    │
│    │                                                                            │
│    ├─► provider.GetIBigDriveEnumerate(&pEnumerate)                              │
│    │     └─► CoCreateInstance(clsid, CLSCTX_LOCAL_SERVER, IID_IBigDriveEnumerate)
│    │                                                                            │
│    │         ┌─────────────────────────────────────────────────────────────┐    │
│    │         │                    COM+ MARSHALING                         │    │
│    │         └─────────────────────────────────────────────────────────────┘    │
│    │                              │                                             │
│    │                              ▼                                             │
│    │         ┌─────────────────────────────────────────────────────────────┐    │
│    │         │ dllhost.exe (User: Interactive User / Logged-in User)       │    │
│    │         │                                                             │    │
│    │         │  Provider.Flickr::EnumerateFolders(driveGuid, path)         │    │
│    │         │    └─► FlickrClientWrapper.GetForDrive(driveGuid)           │    │
│    │         │          └─► DriveManager.ReadSecretProperty(...)           │    │
│    │         │                └─► CredentialManager.ReadSecret(...)        │    │
│    │         │                     └─► User's encrypted credential store   │    │
│    │         │          └─► Flickr API call (HTTPS)                        │    │
│    │         │    └─► Returns ["Vacation 2024", "Family Photos"]           │    │
│    │         └─────────────────────────────────────────────────────────────┘    │
│    │                              │                                             │
│    │                              ▼                                             │
│    └─► Receives SAFEARRAY of folder names                                       │
│                                                                                 │
└─────────────────────────────────────────────────────────────────────────────────┘
```

---

## Registry Security

### Registry Structure

Registry stores **non-sensitive** configuration only. Secrets go to Credential Manager.

```
HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\
│
├── Providers\                         
│   └── {CLSID}\                       ← Created by regsvcs.exe during installation
│       ├── id = "{CLSID}"
│       ├── name = "Flickr Provider"
│       └── [provider-level defaults - non-sensitive only]
│
└── Drives\
    └── {DRIVE-GUID}\                  ← Created by mount command or setup
        ├── id = "{GUID}"
        ├── name = "My Flickr Photos"
        ├── clsid = "{PROVIDER-CLSID}"
        └── [non-sensitive properties only]

Windows Credential Manager (per-user):
    BigDrive:{DRIVE-GUID}:FlickrApiKey      ← Encrypted secret
    BigDrive:{DRIVE-GUID}:FlickrApiSecret   ← Encrypted secret
    BigDrive:{DRIVE-GUID}:FlickrOAuthToken  ← Encrypted secret
```

### Access Control

| Principal | Providers Key | Drives Key | Notes |
|-----------|---------------|------------|-------|
| Administrators | Full Control | Full Control | Can manage everything |
| BigDriveInstaller | Full Control | Full Control | Service account for COM+ providers |
| SYSTEM | Full Control | Full Control | Operating system |
| Users | Read | Read | Standard users can enumerate drives |

### Security Implications

1. **All users can READ** drive configurations including any stored API keys
2. **BigDriveInstaller can WRITE** to create/modify drives
3. **Administrators can WRITE** for management purposes

**Current Limitation**: API keys and secrets stored in `HKLM\SOFTWARE\BigDrive\Drives\{GUID}`
are readable by any authenticated user on the machine. This is acceptable for:
- Single-user machines
- Trusted multi-user environments

For high-security scenarios, consider:
- Encrypting sensitive values with DPAPI (LocalMachine scope)
- Storing credentials in Windows Credential Manager
- Using per-user configuration in HKCU

---

## COM+ Application Configuration

After registration, each provider appears in Component Services (comexp.msc):

```
Component Services
└── Computers
    └── My Computer
        └── COM+ Applications
            ├── BigDrive.Service
            │   └── Components
            │       └── BigDrive.Service.BigDriveService
            │
            └── BigDrive.Provider.Flickr
                └── Components
                    └── BigDrive.Provider.Flickr.Provider
```

### Application Properties

| Property | Value | Security Impact |
|----------|-------|-----------------|
| **Activation Type** | Server Application | Runs in dllhost.exe, not in-process |
| **Identity** | BigDriveInstaller | Runs as service account, not user |
| **Access Control** | Disabled | Any user can activate (controlled by shell) |
| **Authentication Level** | Packet | Standard COM security |

### Why Access Control is Disabled

COM+ access control is disabled (`[ApplicationAccessControl(false)]`) because:
1. The shell extension runs as the logged-in user
2. The user must be able to activate providers to browse drives
3. Security is enforced at the registry level (what drives exist)
4. Additional security would prevent Explorer from working

---

## Process Isolation Benefits

Running providers out-of-process provides:

### 1. Stability
```
┌────────────────┐     ┌────────────────┐
│  explorer.exe  │     │  dllhost.exe   │
│                │     │                │
│  If provider   │     │  Provider      │
│  crashes...    │ ──► │  CRASHES       │
│                │     │                │
│  Explorer      │     │  (Explorer     │
│  keeps running │     │   unaffected)  │
└────────────────┘     └────────────────┘
```

### 2. Security Boundaries
```
User "Wayne" (interactive)          Providers (Interactive User)
        │                                   │
        │  ┌─────────────────┐              │  ┌─────────────────┐
        └──│ explorer.exe    │              └──│ dllhost.exe     │
           │                 │                 │  (as Wayne)     │
           │ Can access:     │                 │                 │
           │  - User's files │                 │ Can access:     │
           │  - User's HKCU  │                 │  - Wayne's creds│
           │  - User's creds │                 │  - Network APIs │
           └─────────────────┘                 │  - Event Log    │
                                               └─────────────────┘

BigDrive.Service (BigDriveInstaller - separate dllhost.exe)
           │
           │  ┌─────────────────┐
           └──│ dllhost.exe     │
              │  (as Installer) │
              │                 │
              │ Can access:     │
              │  - HKCR, HKLM   │
              │  - Shell reg    │
              └─────────────────┘
```

### 3. Resource Management
- COM+ manages component pooling
- Automatic cleanup when Explorer closes
- Memory isolation between providers
- Each COM+ application runs in its own dllhost.exe

---

## Attack Surface Analysis

### Potential Attack Vectors

| Vector | Risk | Mitigation |
|--------|------|------------|
| Malicious drive registration | Medium | Registry write requires Admin or BigDriveInstaller |
| Provider DLL hijacking | Low | COM+ loads from registered paths only |
| API key theft | Low | Secrets in Credential Manager, encrypted per-user |
| Provider crash exploitation | Low | Process isolation; provider can't access other users' data |
| Man-in-the-middle (API calls) | Medium | Providers should use HTTPS with cert validation |
| Cross-user credential access | None | Each user's credentials isolated in Credential Manager |

### Trust Assumptions

1. **BigDrive.Setup.exe is run by a trusted administrator**
2. **The machine's Administrators group is trusted**
3. **The BigDriveInstaller account is not compromised**
4. **Provider assemblies are signed and from trusted sources**
5. **The registry ACLs are not modified by other software**
6. **Users do not share their Windows login credentials**

---

## BigDrive.Shell Security

`BigDrive.Shell.exe` is a console application that runs with the user's permissions.

### Capabilities

| Action | Permission Required | How It Works |
|--------|---------------------|--------------|
| List drives | User (Read HKLM) | Reads `SOFTWARE\BigDrive\Drives` |
| Mount drive | Admin or special ACL | Writes to `SOFTWARE\BigDrive\Drives` |
| Unmount drive | Admin or special ACL | Deletes from `SOFTWARE\BigDrive\Drives` |
| Browse files | User | Activates COM+ provider |
| Copy files | User | Calls provider via COM |

### Mount Command Security

The `mount` command creates new drive configurations in HKLM and stores secrets
in Credential Manager:

1. **Registry write** requires Administrator privileges or ACL modification
2. **Credential write** uses the current user's Credential Manager

In the default installation, only Administrators and BigDriveInstaller can mount new drives.

---

## Event Logging

All BigDrive components log to the Windows Event Log for audit purposes.

### Event Sources

| Source | Component | Log |
|--------|-----------|-----|
| BigDrive.Service | COM+ service | Application |
| BigDrive.ShellFolder | Shell extension | Application |
| BigDrive.Client | COM client library | Application |
| BigDrive.Provider.Flickr | Flickr provider | Application |
| BigDrive.Provider.Sample | Sample provider | Application |

### What Gets Logged

- Provider startup/shutdown
- COM+ activation events
- Errors and exceptions
- Configuration changes (when implemented)

---

## Implemented Security Features

### Windows Credential Manager Integration ✓

Secrets (API keys, OAuth tokens) are stored in Windows Credential Manager:

```csharp
// From ConfigProvider/CredentialManager.cs
public static void WriteSecret(Guid driveGuid, string key, string value)
public static string ReadSecret(Guid driveGuid, string key)
public static bool DeleteSecret(Guid driveGuid, string key)
public static void DeleteAllSecretsForDrive(Guid driveGuid)
```

**Benefits:**
- Per-user isolation
- Encrypted at rest by Windows
- Standard Windows security model
- No secrets in registry

### Interactive User Provider Identity ✓

Providers run as Interactive User instead of a service account:

```csharp
// From ComRegistrationManager.cs
public static void SetApplicationIdentityToInteractiveUser(string applicationName)
{
    app.Value["Identity"] = "Interactive User";
    app.Value["Password"] = "";
}
```

**Benefits:**
- Providers can access user's Credential Manager
- Per-user credential isolation
- No shared service account for credential access

### Dual-Identity COM+ Architecture ✓

| Component | Identity | Purpose |
|-----------|----------|---------|
| BigDrive.Service | BigDriveInstaller | Shell namespace registration |
| Providers | Interactive User | User credential access |

---

## Future Security Considerations

### Per-User Drive Configuration

Currently all drives are machine-wide (HKLM). Per-user drives (HKCU) would provide:
- User isolation on shared machines
- User-specific credentials

However, this would require changes to how COM+ providers access configuration.

---

## See Also

- [Installation Architecture](installation.md) — Setup process details
- [Architecture Overview](overview.md) — System architecture
- [Provider Development Guide](../ProviderDevelopmentGuide.md) — Creating secure providers
- [BigDrive.Setup README](../../src/BigDrive.Setup/README.txt) — Setup internals

---

*Copyright © Wayne Walter Berry. All rights reserved.*
