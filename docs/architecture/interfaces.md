# BigDrive Interface Hierarchy

This document describes all COM interfaces used by BigDrive, including provider interfaces and service interfaces.

---

## Interface Categories

BigDrive defines two sets of interfaces:

1. **Provider Interfaces** — Implemented by storage providers (`BigDrive.Interfaces.dll`)
2. **Service Interfaces** — Implemented by BigDrive.Service (`BigDrive.Service.Interfaces.dll`)

---

## Provider Interfaces

Defined in `BigDrive.Interfaces.dll`. Providers implement these interfaces to expose storage backends to BigDrive.

### Interface Hierarchy

```
IProcessInitializer              ← COM+ lifecycle (Startup/Shutdown)
│
├── IBigDriveEnumerate           ← Required: List folders and files
│   ├── EnumerateFolders()
│   └── EnumerateFiles()
│
├── IBigDriveFileInfo            ← Optional: File metadata
│   ├── GetFileSize()
│   └── LastModifiedTime()
│
├── IBigDriveFileData            ← Optional: File content streaming
│   └── GetFileData() → IStream
│
├── IBigDriveFileOperations      ← Optional: Write operations
│   ├── CopyFileToBigDrive()
│   ├── CopyFileFromBigDrive()
│   ├── DeleteFile()
│   └── CreateDirectory()
│
├── IBigDriveAuthentication      ← Optional: OAuth support
│   ├── GetAuthenticationInfo()
│   ├── OnAuthenticationComplete()
│   └── IsAuthenticated()
│
└── IBigDriveRegistration        ← Optional: Setup callbacks
    ├── Register()
    └── Unregister()
```

---

## Required Interface: IBigDriveEnumerate

All providers **must** implement this interface.

### Interface Definition

```csharp
[Guid("...")]
[ComVisible(true)]
public interface IBigDriveEnumerate
{
    /// <summary>
    /// Enumerate folders (subfolders) in a given path.
    /// </summary>
    /// <param name="driveGuid">Drive instance GUID</param>
    /// <param name="path">Path to enumerate (e.g., "\", "\folder1")</param>
    /// <returns>Array of folder names (not full paths)</returns>
    string[] EnumerateFolders(Guid driveGuid, string path);

    /// <summary>
    /// Enumerate files in a given path.
    /// </summary>
    /// <param name="driveGuid">Drive instance GUID</param>
    /// <param name="path">Path to enumerate</param>
    /// <returns>Array of file names (not full paths)</returns>
    string[] EnumerateFiles(Guid driveGuid, string path);
}
```

### Usage Example

**Shell command:** `Z:\> dir`

**Call sequence:**
1. Shell resolves Z: → driveGuid {6369DDE1-...}
2. Shell reads drive config → CLSID {B3D8F2A1-...}
3. Shell calls `IBigDriveEnumerate.EnumerateFolders(driveGuid, "\")`
4. Provider returns `["Vacation 2024", "Family Photos"]`
5. Shell calls `IBigDriveEnumerate.EnumerateFiles(driveGuid, "\")`
6. Provider returns `[]`
7. Shell displays output

### Implementation Notes

- **Path format:** Always use backslash (`\`) separators
- **Root path:** `\` (not empty string)
- **Return values:** File/folder names only, not full paths
- **Error handling:** Throw COM exceptions (e.g., `COMException` with HRESULT)

---

## Optional Interface: IBigDriveFileInfo

Provides file metadata (size, timestamps). Recommended for all read-capable providers.

### Interface Definition

```csharp
[Guid("...")]
[ComVisible(true)]
public interface IBigDriveFileInfo
{
    /// <summary>
    /// Get file size in bytes.
    /// </summary>
    long GetFileSize(Guid driveGuid, string path);

    /// <summary>
    /// Get last modified timestamp.
    /// </summary>
    DateTime LastModifiedTime(Guid driveGuid, string path);
}
```

### Usage Example

**Shell command:** `Z:\> dir` (detailed view with file sizes)

**Call sequence:**
1. Shell enumerates files (via `IBigDriveEnumerate`)
2. For each file, Shell calls `GetFileSize()` and `LastModifiedTime()`
3. Shell formats output with size and date

### Implementation Notes

- **Return DateTime.MinValue** if timestamp unavailable
- **Return 0** if size unavailable
- **Don't throw exceptions** for missing metadata (return default values)

---

## Optional Interface: IBigDriveFileData

Provides file content via IStream. Required for download/copy operations.

### Interface Definition

```csharp
[Guid("...")]
[ComVisible(true)]
public interface IBigDriveFileData
{
    /// <summary>
    /// Get file content as an IStream.
    /// </summary>
    /// <param name="driveGuid">Drive instance GUID</param>
    /// <param name="path">File path</param>
    /// <returns>IStream for reading file data</returns>
    [return: MarshalAs(UnmanagedType.Interface)]
    IStream GetFileData(Guid driveGuid, string path);
}
```

### IStream Interface

Uses standard COM IStream (from `System.Runtime.InteropServices.ComTypes`):

```csharp
public interface IStream
{
    void Read(byte[] pv, int cb, IntPtr pcbRead);
    void Write(byte[] pv, int cb, IntPtr pcbWritten);
    void Seek(long dlibMove, int dwOrigin, IntPtr plibNewPosition);
    // ... other methods
}
```

### Usage Example

**Shell command:** `Z:\> copy photo.jpg C:\`

**Call sequence:**
1. Shell resolves source (Z:\photo.jpg) and dest (C:\photo.jpg)
2. Shell calls `IBigDriveFileData.GetFileData(driveGuid, "\photo.jpg")`
3. Provider returns IStream pointing to remote file
4. Shell reads from IStream and writes to local file system
5. Shell closes IStream

### Implementation Notes

- **Streaming:** IStream allows chunked reads (no need to buffer entire file)
- **Dispose:** Ensure IStream is properly disposed after use
- **Thread-safety:** IStream may be accessed from multiple threads
- **Common implementation:** Use `System.IO.Stream` wrapper or direct HTTP stream

---

## Optional Interface: IBigDriveFileOperations

Provides write operations (upload, delete, mkdir). Required for writable providers.

### Interface Definition

```csharp
[Guid("...")]
[ComVisible(true)]
public interface IBigDriveFileOperations
{
    /// <summary>
    /// Copy a local file to BigDrive storage.
    /// </summary>
    void CopyFileToBigDrive(Guid driveGuid, string sourcePath, string destPath);

    /// <summary>
    /// Copy a file from BigDrive storage to local filesystem.
    /// </summary>
    void CopyFileFromBigDrive(Guid driveGuid, string sourcePath, string destPath);

    /// <summary>
    /// Delete a file from BigDrive storage.
    /// </summary>
    void DeleteFile(Guid driveGuid, string path);

    /// <summary>
    /// Create a directory in BigDrive storage.
    /// </summary>
    void CreateDirectory(Guid driveGuid, string path);
}
```

### Usage Example

**Shell command:** `Z:\> copy C:\photo.jpg .`

**Call sequence:**
1. Shell resolves source (C:\photo.jpg) and dest (Z:\photo.jpg)
2. Shell calls `IBigDriveFileOperations.CopyFileToBigDrive(driveGuid, "C:\\photo.jpg", "\\photo.jpg")`
3. Provider uploads file to remote storage
4. Shell displays success

### Implementation Notes

- **CopyFileToBigDrive:** Provider reads from `sourcePath` (local file system)
- **CopyFileFromBigDrive:** Provider writes to `destPath` (local file system)
- **Paths:** `sourcePath` and `destPath` are absolute paths
- **Errors:** Throw COM exceptions for failures (file exists, quota exceeded, etc.)

---

## Optional Interface: IBigDriveAuthentication

Provides OAuth or other authentication flows. Used by Flickr Provider.

### Interface Definition

```csharp
[Guid("...")]
[ComVisible(true)]
public interface IBigDriveAuthentication
{
    /// <summary>
    /// Get authentication information (OAuth URLs, flow type).
    /// </summary>
    string GetAuthenticationInfo(Guid driveGuid);

    /// <summary>
    /// Called after successful authentication to save tokens.
    /// </summary>
    void OnAuthenticationComplete(Guid driveGuid, string authData);

    /// <summary>
    /// Check if drive is currently authenticated.
    /// </summary>
    bool IsAuthenticated(Guid driveGuid);
}
```

### Usage Example

**Shell command:** `Z:\> auth`

**Call sequence:**
1. Shell calls `GetAuthenticationInfo(driveGuid)`
2. Provider returns JSON with OAuth URLs:
   ```json
   {
     "flowType": "OAuth1",
     "requestTokenUrl": "https://flickr.com/oauth/request_token",
     "authorizeUrl": "https://flickr.com/oauth/authorize",
     "accessTokenUrl": "https://flickr.com/oauth/access_token"
   }
   ```
3. Shell opens browser to authorization URL
4. User authorizes application
5. Shell receives callback with tokens
6. Shell calls `OnAuthenticationComplete(driveGuid, tokensJson)`
7. Provider saves tokens to drive configuration

### Implementation Notes

- **GetAuthenticationInfo:** Return JSON describing auth flow
- **OnAuthenticationComplete:** Save tokens to drive config (via `DriveManager.WriteDriveProperty`)
- **IsAuthenticated:** Check for valid tokens in drive config

---

## Optional Interface: IBigDriveRegistration

Provides setup/teardown callbacks during provider installation.

### Interface Definition

```csharp
[Guid("...")]
[ComVisible(true)]
public interface IBigDriveRegistration
{
    /// <summary>
    /// Called when provider is registered (during installation).
    /// </summary>
    void Register();

    /// <summary>
    /// Called when provider is unregistered (during uninstallation).
    /// </summary>
    void Unregister();
}
```

### Usage Example

**Setup:** `regsvcs BigDrive.Provider.Flickr.dll`

**Call sequence:**
1. regsvcs registers COM+ application
2. regsvcs calls `IBigDriveRegistration.Register()`
3. Provider writes default config to `HKLM\SOFTWARE\BigDrive\Providers\{CLSID}\`

### Implementation Notes

- **Register:** Write provider-level default configuration
- **Unregister:** Clean up provider-specific registry keys
- **Permissions:** Runs with installer permissions (can write to HKLM)

---

## Lifecycle Interface: IProcessInitializer

Standard COM+ interface for process-level startup/shutdown.

### Interface Definition

```csharp
[Guid("1113f52d-dc7f-4943-aed6-88d04027e32a")]
[ComVisible(true)]
public interface IProcessInitializer
{
    /// <summary>
    /// Called when COM+ application process starts.
    /// </summary>
    void Startup(object punkProcessControl);

    /// <summary>
    /// Called when COM+ application process shuts down.
    /// </summary>
    void Shutdown();
}
```

### Usage Example

**Provider startup:**
1. COM+ starts `dllhost.exe` for provider application
2. COM+ calls `IProcessInitializer.Startup()` once per process
3. Provider performs initialization (load config, establish connections)

**Provider shutdown:**
1. COM+ prepares to shut down `dllhost.exe`
2. COM+ calls `IProcessInitializer.Shutdown()` once per process
3. Provider performs cleanup (close connections, flush caches)

### Implementation Notes

- **Startup:** Initialize shared resources (connection pools, caches)
- **Shutdown:** Clean up shared resources (don't throw exceptions)
- **Per-process:** Called once per `dllhost.exe` process, not per instance

---

## Service Interfaces

Defined in `BigDrive.Service.Interfaces.dll`. BigDrive.Service implements these interfaces.

### IBigDriveProvision

Drive provisioning and deprovisioning interface.

```csharp
[Guid("...")]
[ComVisible(true)]
public interface IBigDriveProvision
{
    /// <summary>
    /// Create a drive from existing registry configuration.
    /// </summary>
    /// <param name="driveGuid">Drive GUID (must exist in registry)</param>
    void Create(Guid driveGuid);

    /// <summary>
    /// Create a drive from JSON configuration.
    /// </summary>
    /// <param name="json">Drive configuration JSON</param>
    /// <returns>Newly created drive GUID</returns>
    Guid CreateFromConfiguration(string json);

    /// <summary>
    /// Unmount and remove a drive.
    /// </summary>
    /// <param name="driveGuid">Drive GUID to remove</param>
    void UnmountDrive(Guid driveGuid);
}
```

### Usage Example

**Shell command:** `bigdrive mount Z: flickr "My Flickr"`

**Call sequence:**
1. Shell builds JSON configuration
2. Shell activates BigDrive.Service via COM+ (out-of-process)
3. Shell calls `IBigDriveProvision.CreateFromConfiguration(json)`
4. Service writes registry keys:
   - `HKLM\SOFTWARE\BigDrive\Drives\{NewGuid}\*`
   - `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace\{NewGuid}\`
5. Service refreshes Explorer (SHChangeNotify)
6. Service returns new drive GUID
7. Shell displays success

### Security Model

- **Runs elevated:** BigDrive.Service runs as **BigDriveInstaller** (local service account)
- **Can write to HKLM:** Has permissions to modify registry
- **Out-of-process:** Shell must call via COM+ (never call directly)

---

## Interface Discovery

### How Shell Finds Providers

1. **Read CLSID** from drive configuration:
   ```csharp
   Guid clsid = DriveManager.ReadProviderClsid(driveGuid);
   ```

2. **Activate provider via COM+:**
   ```csharp
   Type providerType = Type.GetTypeFromCLSID(clsid);
   object providerObj = Activator.CreateInstance(providerType);
   ```

3. **Query for interfaces:**
   ```csharp
   if (providerObj is IBigDriveEnumerate enumerator)
   {
       string[] folders = enumerator.EnumerateFolders(driveGuid, path);
   }
   
   if (providerObj is IBigDriveFileData fileData)
   {
       IStream stream = fileData.GetFileData(driveGuid, path);
   }
   ```

### Capability Detection

**Pattern:** Try to cast to interface; if successful, capability is available.

```csharp
// Check if provider supports file info
bool supportsFileInfo = (providerObj is IBigDriveFileInfo);

// Check if provider supports write operations
bool supportsWrites = (providerObj is IBigDriveFileOperations);

// Check if provider requires authentication
bool supportsAuth = (providerObj is IBigDriveAuthentication);
```

---

## Interface Implementation Checklist

### Minimal Read-Only Provider

- ✅ `IProcessInitializer` (optional but recommended)
- ✅ `IBigDriveEnumerate` (required)
- ✅ `IBigDriveFileInfo` (recommended)
- ✅ `IBigDriveFileData` (required for downloads)

### Full-Featured Provider

- ✅ `IProcessInitializer`
- ✅ `IBigDriveEnumerate`
- ✅ `IBigDriveFileInfo`
- ✅ `IBigDriveFileData`
- ✅ `IBigDriveFileOperations` (for uploads/deletes)
- ✅ `IBigDriveAuthentication` (if OAuth required)
- ✅ `IBigDriveRegistration` (for setup defaults)

---

## COM Interop Attributes

### Required Attributes

All interfaces must have:

```csharp
[Guid("...")] // Unique GUID (generate with guidgen.exe)
[ComVisible(true)] // Expose to COM
[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)] // COM interface type
```

All implementing classes must have:

```csharp
[Guid("...")] // Unique GUID (CLSID)
[ComVisible(true)]
[ProgId("Provider.ClassName")] // Optional ProgID
public class FlickrProvider : ServicedComponent, IBigDriveEnumerate, ...
{
    // Implementation
}
```

### ServicedComponent Base Class

All providers and BigDrive.Service must inherit from `System.EnterpriseServices.ServicedComponent`:

```csharp
using System.EnterpriseServices;

public class FlickrProvider : ServicedComponent, 
    IBigDriveEnumerate, 
    IBigDriveFileInfo, 
    IBigDriveFileData
{
    // COM+ managed component
}
```

---

## See Also

- [Overview](overview.md) — High-level architecture
- [Components](components.md) — Component details
- [Provider Development Guide](../ProviderDevelopmentGuide.md) — Step-by-step provider creation
- [Data Flow](data-flow.md) — How interfaces are called at runtime
