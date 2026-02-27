# BigDrive Process Isolation

This document describes how BigDrive uses COM+ activation and separate processes to ensure stability, security, and cross-architecture compatibility.

---

## Overview

BigDrive uses **out-of-process COM+ activation** to run providers and services in separate `dllhost.exe` processes. This provides:

- **Process isolation** — Provider crashes don't affect Explorer or the Shell
- **Security boundaries** — Different components run with different identities
- **Cross-architecture support** — C# (Any CPU) can be called from C++ (x86/x64)

---

## Process Architecture

### Process Diagram

```
┌──────────────────────────────────────────────────────────────┐
│ explorer.exe (32-bit or 64-bit)                              │
│ Identity: Interactive User                                   │
│                                                              │
│  ┌────────────────────────────────────┐                      │
│  │ BigDrive.ShellFolder.dll           │ In-process           │
│  │ (x86 or x64 - matches Explorer)    │ shell extension      │
│  └────────────────────────────────────┘                      │
└──────────────────────────┬───────────────────────────────────┘
                           │
                           │ COM+ Activation
                           │ CoCreateInstance(ProviderCLSID)
                           ▼
┌──────────────────────────────────────────────────────────────┐
│ dllhost.exe (Provider Process)                               │
│ Identity: Interactive User                                   │
│ Architecture: Any CPU (JIT-compiled)                         │
│                                                              │
│  Provider.Flickr.dll, Provider.Archive.dll, etc.             │
│  (Separate dllhost.exe per COM+ application)                 │
└──────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────┐
│ BigDrive.Shell.exe                                           │
│ Identity: Interactive User                                   │
│ Architecture: Any CPU                                        │
└──────────────────────────┬───────────────────────────────────┘
                           │
                           │ COM+ Activation
                           │ CoCreateInstance(ServiceCLSID)
                           ▼
┌──────────────────────────────────────────────────────────────┐
│ dllhost.exe (Service Process)                                │
│ Identity: BigDriveInstaller (Elevated Local Service Account)│
│ Architecture: Any CPU (JIT-compiled)                         │
│                                                              │
│  BigDrive.Service.dll                                        │
└──────────────────────────────────────────────────────────────┘
```

### Process Boundaries

| Process | Component | Identity | Purpose |
|---------|-----------|----------|---------|
| **explorer.exe** | BigDrive.ShellFolder.dll | Interactive User | Windows Explorer integration |
| **BigDrive.Shell.exe** | (Standalone) | Interactive User | Command-line interface |
| **dllhost.exe #1** | Provider.Flickr.dll | Interactive User | Flickr storage backend |
| **dllhost.exe #2** | Provider.Archive.dll | Interactive User | Archive file access |
| **dllhost.exe #3** | BigDrive.Service.dll | BigDriveInstaller | Drive provisioning (elevated) |

**Note:** Each COM+ application runs in its own `dllhost.exe` process.

---

## COM+ Activation

### What is COM+ Activation?

COM+ (Component Object Model+) is a Windows technology for creating and managing server components. It provides:

- **Process isolation:** Components run in separate processes
- **Identity management:** Components can run under different user accounts
- **Automatic lifecycle:** Windows manages process startup/shutdown
- **Marshaling:** Automatic data transfer across process boundaries

### In-Process vs Out-of-Process

#### In-Process (DLL loaded into caller)

```
┌────────────────────────┐
│ explorer.exe           │
│                        │
│  ┌──────────────────┐  │
│  │ ShellFolder.dll  │  │ ← Loaded directly
│  └──────────────────┘  │
└────────────────────────┘

Pros:
✅ Fast (no marshaling)
✅ Low memory overhead

Cons:
❌ Crash affects host process
❌ Must match host architecture
❌ No security isolation
```

#### Out-of-Process (COM+ activation)

```
┌────────────────────────┐
│ BigDrive.Shell.exe     │
└────────┬───────────────┘
         │
         │ COM+ Activation
         │ (cross-process)
         ▼
┌────────────────────────┐
│ dllhost.exe            │
│                        │
│  Provider.Flickr.dll   │
└────────────────────────┘

Pros:
✅ Crash isolated from caller
✅ Can use different identity
✅ Cross-architecture (x86 ↔ x64)
✅ Security boundaries

Cons:
❌ Slower (marshaling overhead)
❌ More memory (separate process)
```

---

## Why Out-of-Process for Providers?

### 1. Stability

**Problem:** Provider bugs (null reference, infinite loop, unhandled exception) crash the host process.

**Solution:** Provider crashes only affect `dllhost.exe`, not Explorer or Shell.

```
Without Process Isolation:
┌──────────────────────┐
│ explorer.exe         │
│  ┌────────────────┐  │
│  │ Provider.Flickr│  │ ← Exception thrown
│  └────────────────┘  │
│                      │
│  💥 CRASH            │ ← Explorer crashes
└──────────────────────┘

With Process Isolation:
┌──────────────────────┐
│ explorer.exe         │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ dllhost.exe          │
│  Provider.Flickr.dll │ ← Exception thrown
│                      │
│  💥 CRASH            │ ← Only dllhost crashes
└──────────────────────┘
           ↓
┌──────────────────────┐
│ explorer.exe         │ ← Still running
│  (Shows error msg)   │
└──────────────────────┘
```

### 2. Security Boundaries

**Problem:** Shell extensions run with Interactive User permissions. If a provider needs elevated access (registry writes), it can't run in the same process.

**Solution:** BigDrive.Service runs in separate process with BigDriveInstaller identity.

```
User Process (Limited):
┌──────────────────────┐
│ BigDrive.Shell.exe   │
│ Identity: Bob        │
│ Can read HKLM        │
│ Can't write HKLM     │ ❌
└──────────┬───────────┘
           │
           │ COM+ Activation
           ▼
Service Process (Elevated):
┌──────────────────────┐
│ dllhost.exe          │
│ Identity:            │
│   BigDriveInstaller  │
│ Can read HKLM        │ ✅
│ Can write HKLM       │ ✅
│ Can write HKCR       │ ✅
└──────────────────────┘
```

### 3. Cross-Architecture Support

**Problem:** Explorer can be 32-bit or 64-bit. BigDrive.ShellFolder.dll must match. But we don't want to maintain two versions of every provider.

**Solution:** C++ shell extension (x86/x64) activates C# provider (Any CPU) out-of-process. COM+ handles marshaling.

```
32-bit Explorer:
┌──────────────────────┐
│ explorer.exe (x86)   │
│                      │
│  ShellFolder.dll     │ ← x86 version
└──────────┬───────────┘
           │
           │ COM+ Activation
           │ (marshaling handles bitness)
           ▼
┌──────────────────────┐
│ dllhost.exe          │
│                      │
│  Provider.Flickr.dll │ ← Any CPU
│  (JIT to x86)        │
└──────────────────────┘

64-bit Explorer:
┌──────────────────────┐
│ explorer.exe (x64)   │
│                      │
│  ShellFolder.dll     │ ← x64 version
└──────────┬───────────┘
           │
           │ COM+ Activation
           ▼
┌──────────────────────┐
│ dllhost.exe          │
│                      │
│  Provider.Flickr.dll │ ← Same DLL!
│  (JIT to x64)        │ ← JIT-compiled to x64
└──────────────────────┘
```

---

## COM+ Application Configuration

### What is a COM+ Application?

A COM+ application is a logical grouping of COM components with shared configuration:
- **Identity:** User account to run as
- **Activation:** In-process or out-of-process
- **Security:** Access permissions
- **Lifecycle:** Shutdown timeout, pooling, etc.

### BigDrive COM+ Applications

**Provider Applications** (one per provider):
```
Application Name: BigDrive.Provider.Flickr
Identity: Interactive User
Activation: Server (out-of-process)
Components:
  - Provider.Flickr.Provider (CLSID: {B3D8F2A1-...})
```

**Service Application:**
```
Application Name: BigDrive.Service
Identity: BigDriveInstaller (local service account)
Activation: Server (out-of-process)
Components:
  - BigDrive.Service.ProvisionService (CLSID: {C1D2E3F4-...})
```

### Viewing COM+ Applications

**Component Services MMC:**
1. Open: `dcomcnfg.exe`
2. Navigate: Component Services → Computers → My Computer → COM+ Applications
3. Look for: BigDrive.Provider.*, BigDrive.Service

---

## Process Activation Flow

### Step-by-Step: Shell activates Provider

**1. Shell wants to list files on Flickr drive Z:**

```csharp
// Shell/ProviderFactory.cs
Guid clsid = DriveManager.ReadProviderClsid(driveGuid);
// Returns: {B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}

Type providerType = Type.GetTypeFromCLSID(clsid);
// Looks up CLSID in registry (HKCR\CLSID\{B3D8F2A1-...})
```

**2. .NET runtime calls COM:**

```
CoCreateInstance(
    clsid: {B3D8F2A1-...},
    context: CLSCTX_LOCAL_SERVER,  // Out-of-process
    iid: IID_IUnknown
)
```

**3. COM+ Service (comsvcs.dll) checks registration:**

```
HKCR\CLSID\{B3D8F2A1-...}\
├── AppID = "{COM+ App GUID}"
└── LocalServer32 = "dllhost.exe /Processid:{...}"
```

**4. COM+ starts dllhost.exe:**

```cmd
dllhost.exe /Processid:{COM+ App GUID}
```

**5. dllhost.exe loads provider DLL:**

```
dllhost.exe loads: BigDrive.Provider.Flickr.dll
Calls: IProcessInitializer.Startup()
```

**6. COM+ creates instance and marshals interface:**

```
Provider.Flickr.Provider object created in dllhost.exe
COM+ creates proxy in Shell process
Proxy forwards calls to dllhost.exe via RPC
```

**7. Shell calls interface methods:**

```csharp
IBigDriveEnumerate enumerator = (IBigDriveEnumerate)provider;
string[] folders = enumerator.EnumerateFolders(driveGuid, @"\");
// RPC call to dllhost.exe
// Provider executes EnumerateFolders()
// RPC marshals result back to Shell
```

---

## Marshaling

### What is Marshaling?

**Marshaling** is the process of packaging data to send across process boundaries.

**Simple types** (int, string, GUID) are copied directly.

**Complex types** require custom marshaling:
- **Arrays:** Copied element-by-element
- **Interfaces:** Replaced with proxies
- **IStream:** Special COM marshaler (stream across process boundary)

### Example: Marshaling EnumerateFolders() Call

**Shell process:**
```csharp
string[] folders = enumerator.EnumerateFolders(driveGuid, @"\");
```

**Marshaling steps:**

1. **Shell side (caller):**
   ```
   Pack parameters:
     - driveGuid: {6369DDE1-...} → 16 bytes
     - path: "\" → Unicode string (2 bytes + null)
   ```

2. **RPC (Remote Procedure Call):**
   ```
   Send message to dllhost.exe:
     - Method ID: 3 (EnumerateFolders)
     - Parameters: [16 bytes GUID] [4 bytes path]
   ```

3. **Provider side (callee):**
   ```
   Unpack parameters:
     - driveGuid ← 16 bytes
     - path ← Unicode string
   Execute method:
     string[] result = EnumerateFolders(driveGuid, path);
   Pack result:
     - Array length: 2
     - Element 0: "Vacation 2024"
     - Element 1: "Family Photos"
   ```

4. **RPC (return):**
   ```
   Send result to Shell:
     - Array length: 2
     - Strings: [28 bytes] [26 bytes]
   ```

5. **Shell side (return):**
   ```
   Unpack result:
     folders ← ["Vacation 2024", "Family Photos"]
   ```

**Cost:** ~0.1-1ms per call (local machine RPC)

---

## Process Lifecycle

### Provider Process Startup

```
1. User action triggers provider activation
   (e.g., Shell runs "dir Z:")

2. COM+ checks if provider process already running
   - If yes: use existing dllhost.exe
   - If no: start new dllhost.exe

3. dllhost.exe loads provider DLL

4. COM+ calls IProcessInitializer.Startup()
   Provider initializes shared resources

5. COM+ creates provider instance

6. Provider services Shell request
```

### Provider Process Shutdown

```
1. No active COM objects for N minutes (configurable timeout)
   Default: 3 minutes

2. COM+ calls IProcessInitializer.Shutdown()
   Provider cleans up shared resources

3. COM+ unloads provider DLL

4. dllhost.exe terminates
```

**Configuration:** Component Services → COM+ Application → Properties → Advanced → Shutdown after N minutes idle

### Service Process Lifecycle

BigDrive.Service uses **on-demand activation:**

```
1. Shell calls mount/unmount command

2. Shell activates BigDrive.Service via COM+
   ServiceFactory.GetProvisionService()

3. COM+ starts dllhost.exe (if not already running)

4. Service performs registry writes

5. Shell releases service reference

6. After idle timeout, dllhost.exe shuts down
```

---

## Security Boundaries

### Identity Separation

| Component | Process | Identity | Capabilities |
|-----------|---------|----------|--------------|
| **BigDrive.Shell** | BigDrive.Shell.exe | Interactive User (Bob) | Read HKLM, read/write HKCU |
| **Providers** | dllhost.exe | Interactive User (Bob) | Read HKLM, network access |
| **BigDrive.Service** | dllhost.exe | BigDriveInstaller | Write HKLM, write HKCR |

### Why Separate Identity for Service?

**Problem:** Interactive User (Bob) cannot write to HKLM or HKCR.

**Solution:** Create local service account **BigDriveInstaller** with elevated permissions:

```powershell
# During BigDrive.Setup installation:
New-LocalUser -Name "BigDriveInstaller" `
              -Description "BigDrive Service Account" `
              -NoPassword

# Grant registry permissions:
$acl = Get-Acl "HKLM:\SOFTWARE\BigDrive"
$rule = New-Object System.Security.AccessControl.RegistryAccessRule(
    "BigDriveInstaller",
    "FullControl",
    "ContainerInherit,ObjectInherit",
    "None",
    "Allow"
)
$acl.AddAccessRule($rule)
Set-Acl "HKLM:\SOFTWARE\BigDrive" $acl
```

### COM+ Application Identity Configuration

**Component Services → BigDrive.Service → Properties → Identity:**
```
○ Interactive user
● This user: BigDriveInstaller
  Password: (managed by COM+)
```

---

## Failure Scenarios

### Provider Crash

```
1. Provider throws unhandled exception

2. dllhost.exe process terminates

3. COM+ detects process termination

4. Shell receives COMException:
   "The RPC server is unavailable"

5. Shell displays error to user:
   "Provider 'Flickr' encountered an error"

6. User can retry operation
   → COM+ starts new dllhost.exe
```

**Impact:** Isolated to failing provider, Shell continues running.

### Service Crash

```
1. Service throws unhandled exception during mount

2. dllhost.exe (Service) terminates

3. Shell receives COMException

4. Shell displays error:
   "Failed to mount drive"

5. Partial registry writes may occur
   → Manual cleanup may be needed
```

**Mitigation:** Service uses try/catch and transaction-like patterns.

### Shell Crash

```
1. Shell crashes before releasing provider

2. Provider dllhost.exe continues running

3. After idle timeout, dllhost.exe shuts down automatically
```

**Impact:** None, COM+ cleans up automatically.

---

## Debugging Across Processes

### Attaching Debugger to dllhost.exe

**Visual Studio:**

1. Start BigDrive.Shell in debugger (F5)
2. Trigger provider activation (e.g., `dir Z:`)
3. Debug → Attach to Process → Select `dllhost.exe`
4. Set breakpoints in provider code

**Tip:** Add `System.Diagnostics.Debugger.Launch()` in provider code to auto-prompt for debugger attachment.

### Viewing COM+ Activation Logs

**Event Viewer:**
1. Open: `eventvwr.msc`
2. Navigate: Applications and Services Logs → Microsoft → Windows → COMPLUS
3. Look for: Application startup/shutdown events

---

## Performance Considerations

### Marshaling Overhead

**Cost per COM+ call:** ~0.1-1ms (local machine RPC)

**Mitigation strategies:**
- **Batch operations:** Return arrays instead of calling multiple times
- **Cache results:** Don't re-enumerate unchanged directories
- **Async calls:** Use Task/async for parallel provider calls

### Process Startup Time

**First call to provider:** ~100-500ms (start dllhost.exe, load DLL, JIT compile)

**Subsequent calls:** <1ms (process already running)

**Mitigation:**
- Keep dllhost.exe alive with longer idle timeout
- Pre-warm providers during Shell startup

---

## See Also

- [Overview](overview.md) — High-level architecture
- [Components](components.md) — Component details
- [Interfaces](interfaces.md) — COM interface definitions
- [Data Flow](data-flow.md) — Request/response flows
- [Installation](installation.md) — COM+ registration
