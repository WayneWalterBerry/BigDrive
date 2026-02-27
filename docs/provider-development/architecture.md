# BigDrive Provider Architecture

This document explains the architectural foundations of BigDrive providers, including the COM+ process model, identity management, and component lifecycle.

---

## Overview

BigDrive providers are **COM+ ServicedComponents** that run **out-of-process** in `dllhost.exe`, isolated from both Windows Explorer and BigDrive Shell. This architecture provides:

- ✅ **Stability** - Provider crashes don't affect Explorer
- ✅ **Security** - Runs as Interactive User with Credential Manager access
- ✅ **Isolation** - Multiple providers run in separate processes
- ✅ **Performance** - COM+ manages lifecycle and pooling

---

## Process Model

### Out-of-Process COM+ Activation

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

### Why Out-of-Process?

| Benefit | Explanation |
|---------|-------------|
| **Fault Isolation** | If provider crashes, Explorer continues running |
| **Memory Isolation** | Provider memory leaks don't affect Explorer |
| **Clean Shutdown** | COM+ can terminate misbehaving providers |
| **Security Boundary** | Provider runs with different permissions than Explorer |
| **Version Isolation** | Different providers can use different NuGet package versions |

### Process Lifecycle

1. **First Call** - Shell calls `CoCreateInstance()` with provider CLSID
2. **COM+ Activation** - COM+ launches `dllhost.exe` if not already running
3. **Provider Load** - CLR loads provider assembly into dllhost.exe
4. **Static Constructor** - Provider's static constructor runs (AssemblyResolver initialized)
5. **IProcessInitializer.Startup()** - Provider initialization hook
6. **Method Calls** - Shell calls provider methods (EnumerateFolders, GetFileData, etc.)
7. **Idle Timeout** - After inactivity, COM+ may shut down process
8. **IProcessInitializer.Shutdown()** - Provider cleanup hook
9. **Process Exit** - dllhost.exe terminates

---

## Identity and Security

### Interactive User Identity

Providers run as **Interactive User**, which means:

```
Current User: DOMAIN\username (the logged-in user)
Process: dllhost.exe
Parent: svchost.exe (COM+ System Application)
Token: Interactive logon session
```

**Key Capabilities:**
- ✅ Read/write HKCU registry (user-specific configuration)
- ✅ Access Windows Credential Manager (encrypted secrets storage)
- ✅ Access user's network drives and shares
- ✅ Use user's OAuth tokens and API keys
- ❌ Cannot write to HKLM (requires elevation)
- ❌ Cannot access system-protected files

### Why Interactive User?

| Use Case | Reason |
|----------|--------|
| **OAuth Tokens** | Stored in user's Credential Manager |
| **API Keys** | Different keys per user/account |
| **Cloud Services** | User-specific credentials |
| **Network Shares** | User's authenticated sessions |

**Alternative:** Service Account (not used because providers need user context)

### Setting Identity During Registration

The provider's `[ComRegisterFunction]` automatically sets identity:

```csharp
[ComRegisterFunction]
public static void ComRegister(Type type)
{
    // Sets COM+ Application identity to "Interactive User"
    SetApplicationIdentityToInteractiveUser("BigDrive.Provider.YourService");
    
    // Register provider in BigDrive registry
    Provider provider = new Provider();
    provider.Register();
}
```

---

## Component Lifecycle

### Registration Flow

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

### Startup Sequence

```
1. Shell: CoCreateInstance({Provider-CLSID})
2. COM+: Launch dllhost.exe with COM+ Application configuration
3. CLR: Load provider assembly
4. CLR: Run Provider static constructor
    ├─► AssemblyResolver.Initialize() (CRITICAL!)
5. COM+: Create Provider instance
6. COM+: Call IProcessInitializer.Startup(null)
    ├─► Load configuration
    ├─► Initialize API client
    ├─► Connect to services
7. Shell: Call provider methods (EnumerateFolders, etc.)
```

### Shutdown Sequence

```
1. COM+: Idle timeout reached (default: 3 minutes)
2. COM+: Call IProcessInitializer.Shutdown()
    ├─► Close API connections
    ├─► Flush caches
    ├─► Dispose resources
3. COM+: Unload provider assembly
4. COM+: Terminate dllhost.exe process
```

---

## Registry Structure

### Providers (System-Wide)

```
HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\Providers\{Provider-CLSID}\
    Name = "YourService"
    Version = "1.0.0.0"
    Description = "Provider for YourService"
    
    (Optional provider-level defaults)
    ApiEndpoint = "https://api.yourservice.com"
    DefaultTimeout = "30000"
```

**Purpose:** System-wide provider metadata and defaults

### Drives (Per-User)

```
HKEY_CURRENT_USER\SOFTWARE\BigDrive\Drives\{Drive-GUID}\
    CLSID = {Provider-CLSID}
    Name = "My YourService"
    
    (Drive-specific configuration - overrides provider defaults)
    ApiEndpoint = "https://api-region2.yourservice.com"
    RootPath = "/shared/folder"
    CachePath = "C:\Users\{user}\AppData\Local\BigDrive\YourService"
```

**Purpose:** User-specific drive instances with custom configuration

### Secrets (Windows Credential Manager)

```
Target: BigDrive:{Drive-GUID}:YourServiceAccessToken
User: DOMAIN\username
Password: (encrypted OAuth access token)

Target: BigDrive:{Drive-GUID}:YourServiceRefreshToken
User: DOMAIN\username
Password: (encrypted OAuth refresh token)
```

**Purpose:** Secure storage for sensitive credentials (encrypted per-user)

### Configuration Hierarchy

```
Priority (highest to lowest):

1. Drive-specific: HKCU\SOFTWARE\BigDrive\Drives\{Drive-GUID}\PropertyName
2. Provider-level: HKLM\SOFTWARE\BigDrive\Providers\{CLSID}\PropertyName
3. Hard-coded default in provider code
```

**Example:**
```csharp
// In your client wrapper
string endpoint = DriveManager.ReadDriveProperty(driveGuid, "ApiEndpoint", CancellationToken.None)
               ?? ProviderManager.ReadProviderProperty(CLSID, "ApiEndpoint", CancellationToken.None)
               ?? "https://api.default.com"; // Fallback
```

---

## Provider vs Drive Distinction

### Provider (Registered Component)

- **What:** COM+ ServicedComponent with unique CLSID
- **Registered:** During `regsvcs.exe` (Administrator required)
- **Stored:** HKLM (system-wide)
- **Lifetime:** Permanent until unregistered
- **Example:** "Flickr Provider" (one registration)

**Analogy:** Like a **printer driver** - installed once, used by many documents

### Drive (User Instance)

- **What:** User-created instance with specific configuration
- **Created:** By user via `bigdrive drive create` (no elevation)
- **Stored:** HKCU (per-user)
- **Lifetime:** Until user deletes drive
- **Example:** "My Photos" and "Work Account" (two drives, same provider)

**Analogy:** Like a **printer** - multiple printers can use the same driver

### Relationship

```
One Provider → Many Drives

Flickr Provider (CLSID {ABC...})
    ├─► Drive: "Personal Photos" (Drive GUID {123...})
    │       └─► OAuth tokens for user1@gmail.com
    │
    ├─► Drive: "Work Photos" (Drive GUID {456...})
    │       └─► OAuth tokens for user1@company.com
    │
    └─► Drive: "Family Album" (Drive GUID {789...})
            └─► OAuth tokens for family@gmail.com
```

---

## COM+ Application Configuration

### Required Attributes

```csharp
// AssemblyInfo.cs

[assembly: ComVisible(true)]
[assembly: Guid("YOUR-UNIQUE-GUID")]

// COM+ Server activation (out-of-process)
[assembly: ApplicationActivation(ActivationOption.Server)]
[assembly: ApplicationAccessControl(false)]
```

### Application Properties

| Property | Value | Purpose |
|----------|-------|---------|
| **Activation** | `Server` | Out-of-process (dllhost.exe) |
| **Identity** | `Interactive User` | Run as logged-in user |
| **Idle Timeout** | `3 minutes` | Auto-shutdown after inactivity |
| **Queuing** | `Disabled` | Synchronous calls only |
| **Pooling** | `Disabled` | One instance per call |

### Viewing Configuration

1. Open Component Services (`comexp.msc`)
2. Navigate to: Component Services → Computers → My Computer → COM+ Applications
3. Find: `BigDrive.Provider.YourService`
4. Properties → Identity → Check "Interactive User"

---

## Threading Model

### Single-Threaded Apartment (STA)

Providers typically use **STA** threading:

```csharp
// No explicit attribute needed - default is STA
public partial class Provider : ServicedComponent
{
    // COM+ ensures thread safety for STA
}
```

**Implications:**
- ✅ Calls are serialized (thread-safe by default)
- ✅ UI components can be created (if needed)
- ⚠️ Blocking calls block the COM+ thread
- ⚠️ Long operations should use async patterns

### Thread Safety Considerations

```csharp
// SAFE: Static fields are fine (STA serializes access)
private static readonly HttpClient _httpClient = new HttpClient();

// SAFE: Instance fields (each call gets new instance OR STA serializes)
private readonly Dictionary<string, CachedData> _cache;

// UNSAFE: Static mutable state accessed from multiple drives
// Use ConcurrentDictionary if caching per-drive
private static readonly ConcurrentDictionary<Guid, ApiClient> _clients;
```

---

## NuGet Dependencies and Assembly Resolution

### The Problem

COM+ applications don't automatically resolve NuGet dependencies because:
- No `.deps.json` file processed
- `dllhost.exe` doesn't probe NuGet cache
- Binding redirects alone are insufficient

### The Solution

**Three Required Components:**

1. **AssemblyResolver.cs** - Custom assembly resolution
2. **app.config** - Binding redirects for version conflicts
3. **Static Constructor** - Early initialization

```csharp
// Provider.cs
static Provider()
{
    // MUST run before COM+ loads any types
    AssemblyResolver.Initialize();
}
```

**See:** [NuGet Dependencies Guide](nuget-dependencies.md) for complete details

---

## Performance Characteristics

### Startup Cost

| Phase | Duration | Notes |
|-------|----------|-------|
| dllhost.exe launch | 50-200ms | First call only |
| Assembly load | 100-500ms | Includes NuGet dependencies |
| Static constructor | 5-50ms | AssemblyResolver initialization |
| IProcessInitializer.Startup | Variable | Your initialization code |

**Total first call:** 200ms - 1s

**Subsequent calls:** <1ms (provider already loaded)

### Memory Usage

- Base dllhost.exe: ~10 MB
- Provider assembly + NuGet deps: 5-50 MB
- Runtime caches: Variable

**Recommendation:** Cache API responses, don't cache large file data

### COM+ Idle Timeout

Default: **3 minutes** of inactivity

- ✅ Saves memory when not in use
- ⚠️ Next call after timeout incurs startup cost
- Configurable in COM+ Application properties

---

## Debugging Architecture Issues

### Problem: "Class not registered" (REGDB_E_CLASSNOTREG)

**Cause:** COM+ Application not registered

**Solution:**
```cmd
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "YourProvider.dll"
```

### Problem: "Access denied" during registration

**Cause:** Not running as Administrator

**Solution:** Run Visual Studio or regsvcs.exe elevated

### Problem: Provider loads but methods fail

**Cause:** AssemblyResolver not initialized early enough

**Solution:** Verify static constructor exists:
```csharp
static Provider()
{
    AssemblyResolver.Initialize();
}
```

### Problem: Multiple dllhost.exe processes

**Normal:** Each COM+ Application gets its own process

```powershell
# View COM+ processes
Get-Process dllhost | Select-Object Id, StartTime, WorkingSet64
```

### Problem: Provider won't unload (DLL locked)

**Cause:** COM+ hasn't shut down process

**Solution:** Stop COM+ Application manually:
```powershell
# PowerShell
$comAdmin = New-Object -ComObject COMAdmin.COMAdminCatalog
$comAdmin.ShutdownApplication("BigDrive.Provider.YourService")
```

---

## Best Practices

### ✅ Do

- Use **Interactive User** identity for user-specific providers
- Implement **IProcessInitializer** for proper startup/shutdown
- Call **AssemblyResolver.Initialize()** in static constructor
- Handle **idle timeout** gracefully (reconnect on next call)
- **Cache API responses** to reduce latency
- **Dispose resources** in Shutdown() method

### ❌ Don't

- Don't use Service Account identity (loses Credential Manager access)
- Don't store secrets in registry (use Credential Manager)
- Don't assume provider stays loaded (handle cold starts)
- Don't hold locks across calls (STA serializes anyway)
- Don't create background threads (COM+ manages lifecycle)
- Don't forget to flush caches on Shutdown()

---

## See Also

- [Getting Started](getting-started.md) - Create your first provider
- [Full Development Guide](guide.md) - Complete reference
- [Development Practices](practices.md) - Build-register-test workflow
- [NuGet Dependencies](nuget-dependencies.md) - AssemblyResolver setup
- [Troubleshooting](troubleshooting.md) - Common errors

---

## Related Architecture Documentation

- [BigDrive Architecture Overview](../../architecture/overview.md) - System-wide architecture
- [Process Isolation](../../architecture/process-isolation.md) - Shell vs Provider separation
- [Installation Flow](../../architecture/installation.md) - Setup and registration

---

*Copyright © Wayne Walter Berry. All rights reserved.*
