# BigDrive Registry Structure

This document describes the registry structure used by BigDrive for providers, drives, and configuration management.

---

## Registry Overview

BigDrive stores all configuration in the Windows Registry under `HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\`.

**Two main branches:**
- **Providers** — Registered COM+ components that define storage backend implementations
- **Drives** — User-created instances that use providers to access specific storage

---

## Providers vs Drives

| Concept | Description | Example |
|---------|-------------|---------|
| **Provider** | COM+ component that knows *how* to access a storage backend | "Flickr Provider" (CLSID: {B3D8F2A1-...}) |
| **Drive** | User-created instance using a provider to access *specific* storage | "My Personal Flickr" (GUID: {6369DDE1-...}, uses Flickr Provider) |

**Key Relationship:**
- One provider can support multiple drives
- Each drive points to a provider via CLSID
- Drive-specific configuration overrides provider defaults

**Example:**
```
Flickr Provider (CLSID: {B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B})
├── Drive 1: "Personal Flickr" (driveGuid: {6369DDE1-...})
│   └── FlickrApiKey = "personal-key"
└── Drive 2: "Work Flickr" (driveGuid: {A1B2C3D4-...})
    └── FlickrApiKey = "work-key"
```

---

## Registry Structure

### Root Key

```
HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\
```

All BigDrive configuration lives under this root key.

---

## Providers Branch

### Location

```
HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\Providers\
```

### Structure

```
HKLM\SOFTWARE\BigDrive\Providers\
│
├── {B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}\    ← Flickr Provider CLSID
│   ├── id                  = "{B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}"
│   ├── name                = "Flickr Provider"
│   ├── FlickrApiKey        = "default-api-key"      ← Provider-level default
│   └── FlickrApiSecret     = "default-secret"
│
├── {A9B8C7D6-5E4F-3A2B-1C0D-9E8F7A6B5C4D}\    ← Archive Provider CLSID
│   ├── id                  = "{A9B8C7D6-5E4F-3A2B-1C0D-9E8F7A6B5C4D}"
│   └── name                = "Archive Provider"
│
├── {C7A1B2D3-E4F5-6789-AB01-CD23EF456789}\    ← Zip Provider CLSID
│   ├── id                  = "{C7A1B2D3-E4F5-6789-AB01-CD23EF456789}"
│   └── name                = "Zip Provider"
│
└── {F8FE2E5A-D1C0-4B3A-8E9F-7A6B5C4D3E2F}\    ← Sample Provider CLSID
    ├── id                  = "{F8FE2E5A-D1C0-4B3A-8E9F-7A6B5C4D3E2F}"
    └── name                = "Sample Provider"
```

### Required Keys

| Key | Type | Description |
|-----|------|-------------|
| `id` | String (REG_SZ) | Provider CLSID (same as registry key name) |
| `name` | String (REG_SZ) | Human-readable provider name |

### Optional Keys (Provider-Specific)

Providers can define custom keys for default configuration values:
- `FlickrApiKey`, `FlickrApiSecret` (Flickr Provider)
- Any provider-specific settings

These serve as fallback defaults when not specified on individual drives.

### Who Creates Provider Keys?

- **BigDrive.Setup.exe** during installation (via regsvcs.exe)
- **Provider.Register()** method (IBigDriveRegistration interface)
- COM+ application registration process

---

## Drives Branch

### Location

```
HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\Drives\
```

### Structure

```
HKLM\SOFTWARE\BigDrive\Drives\
│
├── {6369DDE1-C5B8-4A7F-9E2D-3C1B4A5E6F7A}\    ← Drive GUID
│   ├── id                  = "{6369DDE1-C5B8-4A7F-9E2D-3C1B4A5E6F7A}"
│   ├── name                = "My Personal Flickr"
│   ├── letter              = "Z"                    ← Drive letter assignment
│   ├── clsid               = "{B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}"  ← Points to provider
│   ├── FlickrApiKey        = "personal-api-key"     ← Drive-specific override
│   ├── FlickrOAuthToken    = "oauth-token"
│   └── FlickrOAuthSecret   = "oauth-secret"
│
└── {A1B2C3D4-E5F6-7890-AB12-CD34EF567890}\    ← Another drive (same provider)
    ├── id                  = "{A1B2C3D4-E5F6-7890-AB12-CD34EF567890}"
    ├── name                = "Work Flickr"
    ├── letter              = "Y"
    ├── clsid               = "{B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}"  ← Same provider
    ├── FlickrApiKey        = "work-api-key"         ← Different credentials
    ├── FlickrOAuthToken    = "work-oauth-token"
    └── FlickrOAuthSecret   = "work-oauth-secret"
```

### Required Keys

| Key | Type | Description |
|-----|------|-------------|
| `id` | String (REG_SZ) | Drive GUID (same as registry key name) |
| `name` | String (REG_SZ) | Human-readable drive name (shown in Explorer) |
| `clsid` | String (REG_SZ) | Provider CLSID to use for this drive |
| `letter` | String (REG_SZ) | Drive letter assignment (e.g., "Z") |

### Optional Keys (Drive-Specific)

Drives can override any provider configuration key:
- Provider-specific settings (e.g., `FlickrApiKey`)
- Authentication tokens
- Custom configuration

### Who Creates Drive Keys?

- **BigDrive.Service.dll** via `IBigDriveProvision.CreateFromConfiguration()`
- **BigDrive.Shell** `mount` command (calls BigDrive.Service)
- Runs under **BigDriveInstaller** identity (elevated permissions)

---

## Configuration Priority

When a provider reads configuration, the lookup order is:

### 1. Drive-Specific Value (Highest Priority)

```
HKLM\SOFTWARE\BigDrive\Drives\{DriveGuid}\PropertyName
```

### 2. Provider Default Value

```
HKLM\SOFTWARE\BigDrive\Providers\{CLSID}\PropertyName
```

### 3. Hard-Coded Default (Lowest Priority)

If not found in registry, provider uses a hard-coded default in code.

### Example

**Provider code:**
```csharp
public string[] EnumerateFolders(Guid driveGuid, string path)
{
    // 1. Try drive-specific value
    string apiKey = DriveManager.ReadDriveProperty(
        driveGuid, 
        "FlickrApiKey", 
        CancellationToken.None
    );
    
    // 2. If null, falls back to provider default
    if (apiKey == null)
    {
        apiKey = ProviderManager.ReadProviderProperty(
            this.GetType().GUID,
            "FlickrApiKey",
            CancellationToken.None
        );
    }
    
    // 3. If still null, use hard-coded default
    if (apiKey == null)
    {
        apiKey = "default-hardcoded-key";
    }
    
    // Use apiKey to call Flickr API...
}
```

---

## Shell Namespace Registration

When a drive is created, BigDrive.Service also registers it in the Windows shell namespace.

### Registry Location

```
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace\{DriveGuid}\
```

### Structure

```
HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace\
│
└── {6369DDE1-C5B8-4A7F-9E2D-3C1B4A5E6F7A}\    ← Drive GUID
    └── (Default) = "My Personal Flickr"        ← Drive name
```

This makes the drive appear under "This PC" in Windows Explorer.

---

## COM+ Class Registration

When a provider is installed, COM+ registers it in HKEY_CLASSES_ROOT.

### Registry Location

```
HKEY_CLASSES_ROOT\CLSID\{ProviderCLSID}\
```

### Structure (Simplified)

```
HKCR\CLSID\
│
└── {B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}\    ← Provider CLSID
    ├── (Default)           = "FlickrProvider.Provider"
    ├── AppID               = "{COM+ App GUID}"
    └── InprocServer32\
        └── (Default)       = "C:\Windows\System32\mscoree.dll"
```

**Note:** BigDrive.Service writes to HKCR, not providers. Providers are registered by regsvcs.exe during installation.

---

## Reading and Writing Registry

### BigDrive.ConfigProvider API

All BigDrive components use `BigDrive.ConfigProvider.dll` to access the registry:

**Read Drive Configuration:**
```csharp
using BigDrive.ConfigProvider;

string apiKey = DriveManager.ReadDriveProperty(
    driveGuid, 
    "FlickrApiKey", 
    cancellationToken
);
```

**Read Provider Configuration:**
```csharp
string defaultKey = ProviderManager.ReadProviderProperty(
    providerClsid,
    "FlickrApiKey",
    cancellationToken
);
```

**Write Drive Configuration (Service Only):**
```csharp
// Only BigDrive.Service should write to registry
DriveManager.WriteDriveProperty(
    driveGuid,
    "FlickrOAuthToken",
    "new-token-value",
    cancellationToken
);
```

### Security Model

| Component | Read Drives | Write Drives | Read Providers | Write Providers |
|-----------|-------------|--------------|----------------|-----------------|
| **BigDrive.Shell** | ✅ Yes | ❌ No | ✅ Yes | ❌ No |
| **BigDrive.Service** | ✅ Yes | ✅ Yes | ✅ Yes | ✅ Yes |
| **Providers** | ✅ Yes | ❌ No | ✅ Yes | ❌ No |
| **BigDrive.ShellFolder** | ✅ Yes | ❌ No | ✅ Yes | ❌ No |

**Shell must NEVER write to registry directly.** All write operations go through BigDrive.Service, which runs with elevated permissions.

---

## Drive Creation Flow

### Command: `bigdrive mount Z: flickr "My Flickr"`

1. **Shell parses command** and gathers configuration
2. **Shell calls** `IBigDriveProvision.CreateFromConfiguration()` on BigDrive.Service
3. **Service (elevated) writes:**
   - `HKLM\SOFTWARE\BigDrive\Drives\{NewGuid}\*` (drive configuration)
   - `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace\{NewGuid}\` (shell namespace)
4. **Service refreshes Explorer** (SHChangeNotify)
5. **Shell returns success** to user

### JSON Configuration Format

```json
{
  "name": "My Personal Flickr",
  "clsid": "{B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}",
  "letter": "Z",
  "properties": {
    "FlickrApiKey": "personal-key",
    "FlickrOAuthToken": "oauth-token"
  }
}
```

---

## Drive Removal Flow

### Command: `bigdrive unmount Z:`

1. **Shell resolves** Z: → DriveGuid
2. **Shell calls** `IBigDriveProvision.UnmountDrive(driveGuid)` on BigDrive.Service
3. **Service (elevated) deletes:**
   - `HKLM\SOFTWARE\BigDrive\Drives\{DriveGuid}\` (entire key)
   - `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace\{DriveGuid}\` (shell namespace entry)
4. **Service refreshes Explorer** (SHChangeNotify)
5. **Shell returns success** to user

---

## Querying Available Providers

### Command: `bigdrive providers`

1. **Shell reads** `HKLM\SOFTWARE\BigDrive\Providers\`
2. **Enumerates all subkeys** (each is a provider CLSID)
3. **Reads `name` value** from each provider key
4. **Displays list:**
   ```
   Available Providers:
   - Flickr Provider ({B3D8F2A1-...})
   - Archive Provider ({A9B8C7D6-...})
   - Zip Provider ({C7A1B2D3-...})
   ```

---

## Querying Mounted Drives

### Command: `bigdrive drives`

1. **Shell reads** `HKLM\SOFTWARE\BigDrive\Drives\`
2. **Enumerates all subkeys** (each is a drive GUID)
3. **Reads configuration** (name, letter, clsid) from each drive
4. **Looks up provider name** from CLSID
5. **Displays list:**
   ```
   Mounted Drives:
   Z: My Personal Flickr (Flickr Provider)
   Y: Work Flickr (Flickr Provider)
   ```

---

## Best Practices

### For Shell and Providers (Read-Only Access)

✅ **DO:**
- Use `BigDrive.ConfigProvider.DriveManager` to read drive configuration
- Use `BigDrive.ConfigProvider.ProviderManager` to read provider defaults
- Implement fallback: drive → provider → hard-coded default

❌ **DON'T:**
- Write directly to registry (security violation)
- Hard-code CLSID lookups (use ProviderManager)
- Cache configuration indefinitely (drives can be unmounted)

### For BigDrive.Service (Write Access)

✅ **DO:**
- Validate all configuration before writing
- Use transactions if possible (registry doesn't support transactions natively)
- Refresh Explorer after shell namespace changes
- Log all write operations for auditing

❌ **DON'T:**
- Allow arbitrary registry writes from user input
- Write outside `SOFTWARE\BigDrive\` or shell namespace
- Assume write operations always succeed (handle errors)

---

## See Also

- [Overview](overview.md) — High-level architecture
- [Components](components.md) — Component details
- [Process Isolation](process-isolation.md) — Security boundaries
- [Data Flow](data-flow.md) — How configuration is used at runtime
