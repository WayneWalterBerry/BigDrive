# BigDrive Provider Development Documentation

Welcome to the BigDrive Provider Development Guide! This documentation will help you create
custom providers that expose external storage (cloud services, APIs, databases, archives,
network shares) as virtual drives in Windows Explorer.

---

## What is a BigDrive Provider?

A BigDrive provider is a **COM+ ServicedComponent** that:
- Implements BigDrive interfaces (`IBigDriveEnumerate`, `IBigDriveFileInfo`, etc.)
- Runs out-of-process in `dllhost.exe` for isolation and stability
- Exposes external storage as a file system hierarchy
- Handles authentication, caching, and API communication
- Registers itself during installation via `regsvcs.exe`

**Example providers included:**
- **Iso** - Browse ISO 9660/UDF disc images
- **Archive** - Browse ZIP, TAR, 7z, RAR archives
- **Zip** - Browse and modify ZIP archives
- **Flickr** - Browse Flickr photos as files (OAuth 1.0a)

---

## Table of Contents

### 📖 Core Documentation

| Document | Description | Read If... |
|----------|-------------|-----------|
| **[Getting Started](getting-started.md)** | Project setup, naming conventions, architecture overview | You're creating a new provider |
| **[Architecture](architecture.md)** | **Provider architecture, COM+ model, process lifecycle** | **You need to understand how providers work** |
| **[Full Development Guide](guide.md)** | Complete reference guide with all interfaces, patterns, and registration details | You need comprehensive implementation details |
| **[Development Practices](practices.md)** | Build-register-test workflow, debugging, DLL locking solutions | You're developing and testing providers |
| **[Interfaces Reference](interfaces.md)** | All interface definitions and signatures | You need to know which interfaces to implement |
| **[NuGet Dependencies](nuget-dependencies.md)** | AssemblyResolver, app.config, static constructor setup | Your provider uses NuGet packages (CRITICAL!) |
| **[OAuth Authentication](oauth-authentication.md)** | OAuth 2.0, Device Code, OAuth 1.0a flows | Your provider connects to cloud services |
| **[Troubleshooting](troubleshooting.md)** | Common errors and solutions | Your provider isn't working |

---

## Quick Start Path

### For Local File Providers (ISO, VirtualDisk, Archive, Database)

1. ✅ **[Getting Started](getting-started.md)** - Create project and configure AssemblyInfo.cs
2. ✅ **[Architecture](architecture.md)** - Understand provider process model
3. ✅ **[Interfaces Reference](interfaces.md)** - Implement required interfaces
4. ✅ **[NuGet Dependencies](nuget-dependencies.md)** - Setup AssemblyResolver (CRITICAL!)
5. ✅ **Study existing providers** - See VirtualDisk or ISO provider source code
6. ✅ **[Development Practices](practices.md)** - Build-register-test workflow
7. ✅ **[Troubleshooting](troubleshooting.md)** - If registration fails

**Estimated time:** 2-4 hours

### For Cloud API Providers (OneDrive, Google Drive, Azure)

1. ✅ **[Getting Started](getting-started.md)** - Create project and configure AssemblyInfo.cs
2. ✅ **[Architecture](architecture.md)** - Understand Interactive User identity
3. ✅ **[OAuth Authentication](oauth-authentication.md)** - Implement IBigDriveAuthentication
4. ✅ **[Interfaces Reference](interfaces.md)** - Implement required interfaces
5. ✅ **[NuGet Dependencies](nuget-dependencies.md)** - Setup AssemblyResolver (CRITICAL!)
6. ✅ **Study Flickr provider** - Reference implementation for OAuth and cloud APIs
7. ✅ **[Development Practices](practices.md)** - Build-register-test workflow
8. ✅ **[Troubleshooting](troubleshooting.md)** - If authentication or registration fails

**Estimated time:** 4-8 hours

---

## Prerequisites

Before starting, ensure you have:

- ✅ **Windows 10/11** (COM+ components require Windows)
- ✅ **Visual Studio 2022** or later (with .NET Framework 4.7.2 SDK)
- ✅ **Administrator privileges** (required for COM+ registration)
- ✅ **BigDrive solution** cloned and building successfully

---

## Key Concepts

### Providers vs Drives

| Concept | Definition | Example |
|---------|------------|---------|
| **Provider** | Registered COM component that defines *how* to access storage | "Flickr Provider" |
| **Drive** | User-created instance with specific configuration | "My Flickr Photos" (with my OAuth tokens) |

**One provider, many drives:**
- User can create multiple drives using the same provider
- Each drive has its own configuration (different API keys, file paths, etc.)
- Example: Two Flickr drives for two different Flickr accounts

### Registry Structure

```
HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\Providers\{Provider-CLSID}\
    Name = "Flickr"
    Version = "1.0.0.0"

HKEY_CURRENT_USER\SOFTWARE\BigDrive\Drives\{Drive-GUID}\
    CLSID = {Provider-CLSID}
    Name = "My Flickr"
    IsoFilePath = "C:\ISOs\game.iso"  (drive-specific parameters)
```

### COM+ Activation

```
User runs: bigdrive ls
    ↓
BigDrive Shell calls: CoCreateInstance({Provider-CLSID})
    ↓
COM+ Service Control Manager launches: dllhost.exe
    ↓
dllhost.exe loads: YourProvider.dll
    ↓
COM+ calls: Provider.Startup()
    ↓
Shell calls: Provider.EnumerateFiles(driveGuid, path)
    ↓
Provider returns: ["file1.txt", "file2.jpg"]
```

---

## Architecture Decision: When to Use BigDrive

### ✅ Good Fit for BigDrive

- Cloud storage APIs (OneDrive, Google Drive, Dropbox, Azure Blob)
- Archive formats (ZIP, TAR, 7z, RAR, ISO)
- Database file exports (SQLite, Access, CSV exports)
- Network protocols (FTP, WebDAV, SFTP)
- REST APIs exposing file-like resources

### ❌ Not a Good Fit

- Real-time file system monitoring (use IFileSystemWatcher instead)
- High-performance streaming (use direct file access)
- Block-level storage (use volume drivers instead)
- Scenarios requiring kernel-mode drivers

**BigDrive is user-mode only** and optimized for browsing/copying, not real-time editing.

---

## Development Workflow

### 1. Create Provider Project

```powershell
# Create project from Visual Studio or CLI
dotnet new classlib -f net472 -n BigDrive.Provider.YourService
```

See: [Getting Started](getting-started.md)

### 2. Implement Interfaces

Implement required interfaces in partial class files:
- IProcessInitializer (lifecycle)
- IBigDriveRegistration (self-registration)
- IBigDriveDriveInfo (configuration parameters)
- IBigDriveEnumerate (folder/file listing)

See: [Interfaces Reference](interfaces.md)

### 3. Setup NuGet Dependencies

**CRITICAL STEP!** Add AssemblyResolver and app.config:

See: [NuGet Dependencies](nuget-dependencies.md)

### 4. Build and Register

```powershell
# Build (Debug configuration)
msbuild YourProvider.csproj /p:Configuration=Debug

# Register (as Administrator)
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "bin\Debug\net472\YourProvider.dll"
```

### 5. Test with BigDrive Shell

```sh
# Create drive
bigdrive drive create --provider YourService --name "Test"

# Set required parameters
bigdrive set YourParameter "value"

# Test enumeration
bigdrive ls

# Test file access
bigdrive copy "file.txt" "C:\Temp\"
```

### 6. Debug Issues

- Check Windows Event Log (Application → BigDrive.Provider.YourService)
- Attach Visual Studio debugger to dllhost.exe
- Add verbose logging with DefaultTraceSource.TraceInformation()

See: [Troubleshooting](troubleshooting.md)

---

## Required Files Checklist

When creating a new provider, ensure you have ALL of these files:

### Core Files (Required)
- [ ] `Provider.cs` - Main class with GUID, static constructor, helpers
- [ ] `Provider.IProcessInitializer.cs` - Startup/Shutdown
- [ ] `Provider.IBigDriveRegistration.cs` - Register/Unregister + ComRegisterFunction
- [ ] `Provider.IBigDriveDriveInfo.cs` - GetDriveParameters (JSON)
- [ ] `Provider.IBigDriveEnumerate.cs` - EnumerateFolders/EnumerateFiles
- [ ] `Properties/AssemblyInfo.cs` - COM+ attributes, GUID

### Helper Files (Required)
- [ ] `{Service}ClientWrapper.cs` - Your API/SDK wrapper
- [ ] `BigDriveTraceSource.cs` - Event Log logging
- [ ] `ProviderConfigurationFactory.cs` - Provider config factory
- [ ] `AssemblyResolver.cs` - NuGet dependency resolution **(if using NuGet)**
- [ ] `app.config` - Assembly binding redirects **(if using NuGet)**

### Optional Files
- [ ] `ComStream.cs` - IStream wrapper (if implementing IBigDriveFileData)
- [ ] `Provider.IBigDriveFileInfo.cs` - File metadata (if available)
- [ ] `Provider.IBigDriveFileData.cs` - File streaming (if downloadable)
- [ ] `Provider.IBigDriveFileOperations.cs` - Write operations (if supported)
- [ ] `Provider.IBigDriveAuthentication.cs` - OAuth (if cloud service)

### Documentation
- [ ] `README.md` - Provider architecture and constraints (see ISO provider)

---

## Common Pitfalls

### ❌ Forgetting AssemblyResolver

**Problem:** Provider builds but fails at runtime with "Could not load file or assembly"

**Solution:** See [NuGet Dependencies](nuget-dependencies.md) - Add AssemblyResolver + app.config + static constructor

### ❌ Missing Static Constructor

**Problem:** AssemblyResolver doesn't run early enough, registration fails

**Solution:** Add `static Provider() { AssemblyResolver.Initialize(); }` to Provider.cs

### ❌ Not Running as Administrator

**Problem:** "Access to registry key denied" during registration

**Solution:** Run Visual Studio as Administrator

### ❌ Returning Null from Enumerate Methods

**Problem:** Shell crashes or shows empty directories

**Solution:** Return `Array.Empty<string>()` on errors, never `null`

### ❌ Returning Full Paths from Enumerate Methods

**Problem:** Duplicate path components in Shell (e.g., `\folder\folder\file.txt`)

**Solution:** Return **names only**, not paths: `"file.txt"`, not `"\folder\file.txt"`

### ❌ Not Handling Root Path Variations

**Problem:** Shell can't list root directory

**Solution:** Handle `null`, `""`, `"\"`, `"/"`, `"//"` as root in NormalizePath()

---

## Testing Your Provider

### Unit Testing

Create a test project targeting .NET Framework 4.7.2:

```csharp
using NUnit.Framework;
using BigDrive.Provider.YourService;

[TestFixture]
public class ProviderTests
{
    [Test]
    public void EnumerateFolders_ReturnsExpectedFolders()
    {
        Provider provider = new Provider();
        Guid testDriveGuid = Guid.NewGuid();

        string[] folders = provider.EnumerateFolders(testDriveGuid, "\\");

        Assert.IsNotNull(folders);
        Assert.IsNotEmpty(folders);
    }
}
```

### Integration Testing

```sh
# Create test drive
bigdrive drive create --provider YourService --name "IntegrationTest"

# Configure
bigdrive set YourParameter "test-value"

# Test operations
bigdrive ls
bigdrive cd "TestFolder"
bigdrive copy "test.txt" "C:\Temp\"

# Cleanup
bigdrive drive delete IntegrationTest
```

---

## Performance Best Practices

### 1. Implement Caching

Cache API responses to reduce latency:

```csharp
private readonly Dictionary<string, CachedResult> _cache = 
    new Dictionary<string, CachedResult>();

public string[] GetFolders(string path)
{
    if (_cache.TryGetValue(path, out CachedResult cached) &&
        DateTime.UtcNow - cached.Timestamp < TimeSpan.FromMinutes(5))
    {
        return cached.Folders;
    }

    string[] folders = FetchFromApi(path);
    _cache[path] = new CachedResult { Folders = folders, Timestamp = DateTime.UtcNow };
    return folders;
}
```

### 2. Use Batch APIs

Fetch metadata for all files in one call instead of per-file:

```csharp
// SLOW: One API call per file
foreach (string file in files)
{
    fileInfo = GetFileInfo(file); // N API calls
}

// FAST: One batched API call
Dictionary<string, FileInfo> allInfo = GetBatchFileInfo(files); // 1 API call
```

### 3. Lazy Loading

Don't load all data in Startup():

```csharp
// WRONG - loads everything at startup
public void Startup(object punkProcessControl)
{
    _allFiles = LoadAllFilesFromApi(); // Slow!
}

// CORRECT - load on demand
public string[] EnumerateFiles(Guid driveGuid, string path)
{
    return LoadFilesForPath(path); // Fast - only loads requested path
}
```

### 4. Stream Files Efficiently

Don't buffer entire files in memory:

```csharp
// WRONG - loads entire file into memory
public int GetFileData(Guid driveGuid, string path, out IStream stream)
{
    byte[] allBytes = DownloadEntireFile(path); // May be GB!
    stream = new ComStream(new MemoryStream(allBytes));
}

// CORRECT - stream directly
public int GetFileData(Guid driveGuid, string path, out IStream stream)
{
    Stream fileStream = OpenFileStream(path); // Streams chunks as needed
    stream = new ComStream(fileStream);
}
```

---

## Documentation Navigation

### By Task

| I Want To... | Read This |
|--------------|-----------|
| Create my first provider | [Getting Started](getting-started.md) → Study existing provider source code |
| Add OAuth login | [OAuth Authentication](oauth-authentication.md) |
| Fix "Could not load assembly" error | [NuGet Dependencies](nuget-dependencies.md) → [Troubleshooting](troubleshooting.md) |
| Understand interface signatures | [Interfaces Reference](interfaces.md) |
| See working code | Open provider source: `src/BigDrive.Provider.Iso/` or `src/BigDrive.Provider.VirtualDisk/` |
| Debug registration issues | [Development Practices](practices.md) → [Troubleshooting](troubleshooting.md) |

### By Provider Type

| Provider Type | Recommended Reading Order |
|---------------|---------------------------|
| **Local File** (ISO, VirtualDisk) | Getting Started → Interfaces → NuGet Dependencies → Study ISO source code |
| **Cloud API** (OneDrive, Google) | Getting Started → OAuth Authentication → Interfaces → NuGet Dependencies → Study Flickr source code |
| **Database** (SQLite, Access) | Getting Started → Interfaces → NuGet Dependencies → Study Archive source code |

---

## Quick Reference

### Required Interfaces

Every provider must implement:
- ✅ `IProcessInitializer` - Startup/Shutdown
- ✅ `IBigDriveRegistration` - Register/Unregister
- ✅ `IBigDriveEnumerate` - EnumerateFolders/EnumerateFiles

### Critical Setup (If Using NuGet)

**You MUST have all three:**
1. ✅ `AssemblyResolver.cs` with list of NuGet assemblies
2. ✅ `app.config` with binding redirects
3. ✅ `static Provider()` constructor calling `AssemblyResolver.Initialize()`

**Missing any = runtime failure!** See [NuGet Dependencies](nuget-dependencies.md).

### File Naming Pattern

```
Provider.cs                           # Main class
Provider.IProcessInitializer.cs       # One file per interface
Provider.IBigDriveRegistration.cs
Provider.IBigDriveDriveInfo.cs
Provider.IBigDriveEnumerate.cs
{Service}ClientWrapper.cs             # Your API wrapper
AssemblyResolver.cs                   # If using NuGet
app.config                            # If using NuGet
```

---

## Example Provider Comparison

| Provider | Complexity | LOC | NuGet Packages | OAuth | Write |
|----------|-----------|-----|----------------|-------|-------|
| **Iso** | ⭐ Simple | ~250 | DiscUtils | ❌ No | ❌ No |
| **Zip** | ⭐ Simple | ~300 | None | ❌ No | ✅ Yes |
| **Archive** | ⭐⭐ Medium | ~400 | SharpCompress | ❌ No | ❌ No |
| **Flickr** | ⭐⭐⭐ Complex | ~800 | FlickrNet | ✅ OAuth 1.0a | ❌ No |

**Recommendation:** Start with ISO or Zip as a template, they're the simplest!

---

## Common Use Cases

### Use Case 1: Browse Cloud Photos as Files

**Provider:** Flickr, Google Photos, iCloud
**Interfaces:** IBigDriveEnumerate, IBigDriveFileInfo, IBigDriveFileData, IBigDriveAuthentication
**Complexity:** ⭐⭐⭐ Complex (OAuth + API pagination)
**Reference:** `src/BigDrive.Provider.Flickr/`

### Use Case 2: Browse Database Tables as Folders

**Provider:** SQLite, Access, SQL Server
**Interfaces:** IBigDriveEnumerate, IBigDriveFileData (export as CSV/JSON)
**Complexity:** ⭐⭐ Medium (SQL queries + result streaming)
**Reference:** Similar to Archive provider

### Use Case 3: Mount Network Shares

**Provider:** FTP, WebDAV, SFTP
**Interfaces:** IBigDriveEnumerate, IBigDriveFileInfo, IBigDriveFileData, IBigDriveFileOperations
**Complexity:** ⭐⭐ Medium (network I/O + authentication)
**Reference:** Similar to Flickr provider (OAuth) + Zip provider (write operations)

### Use Case 4: Browse API Resources as Files

**Provider:** REST API, GitHub, Jira
**Interfaces:** IBigDriveEnumerate, IBigDriveFileData
**Complexity:** ⭐⭐ Medium (API pagination + JSON parsing)
**Reference:** Flickr provider (API pattern)

---

## Tips for Success

### 1. Start Small

Don't implement all interfaces at once! Start with:
1. IProcessInitializer (Startup/Shutdown)
2. IBigDriveRegistration (Register)
3. IBigDriveDriveInfo (GetDriveParameters)
4. IBigDriveEnumerate (EnumerateFolders/EnumerateFiles)

Test enumeration works, then add file access (IBigDriveFileData) and metadata (IBigDriveFileInfo).

### 2. Copy from Existing Providers

Don't reinvent the wheel! Copy boilerplate files:
- `AssemblyResolver.cs` - Identical except for assembly list
- `BigDriveTraceSource.cs` - Identical except for source name
- `ComStream.cs` - Identical for all providers
- `ProviderConfigurationFactory.cs` - Simple factory pattern

### 3. Log Everything During Development

```csharp
DefaultTraceSource.TraceInformation($"MethodName: param={param}");
// ... your code
DefaultTraceSource.TraceInformation($"MethodName: result={result}");
```

View logs in Event Viewer or PowerShell:
```powershell
Get-EventLog -LogName Application -Source "BigDrive.Provider.YourService" -Newest 10
```

### 4. Test in Isolation First

Before testing through BigDrive Shell, test your Provider class directly:

```csharp
Provider p = new Provider();
string[] folders = p.EnumerateFolders(testGuid, "\\");
Console.WriteLine($"Found {folders.Length} folders");
```

This eliminates COM+ and Shell variables when debugging.

### 5. Handle Root Path Correctly

The shell may pass root path as `null`, `""`, `"\"`, `"/"`, or `"//"`. Normalize it:

```csharp
private static string NormalizePath(string path)
{
    if (string.IsNullOrEmpty(path) || path == "\\" || path == "/" || path == "//")
    {
        return string.Empty; // Root
    }

    return path.Trim('\\', '/').Replace('\\', '/');
}
```

---

## Getting Help

### Check Event Log First

99% of provider issues show detailed errors in the Windows Event Log:

```powershell
Get-EventLog -LogName Application -Source "BigDrive.Provider.YourService" -Newest 20
```

Look for:
- "Could not load file or assembly" → [NuGet Dependencies](nuget-dependencies.md)
- "Access to registry key denied" → Run as Administrator
- "Exception has been thrown" → Check inner exception details

### Study Working Examples

The best way to learn is by studying existing providers:

| Provider | Best For Learning | Capabilities | Source Path |
|----------|-------------------|--------------|-------------|
| **VirtualDisk** | Read-write operations, multi-format support | VHD/VHDX/VMDK/VDI, Write/Delete/Mkdir | `src/BigDrive.Provider.VirtualDisk/` |
| **Iso** | Simplest read-only provider | ISO disc images, Read-only | `src/BigDrive.Provider.Iso/` |
| **Archive** | NuGet integration patterns | ZIP/TAR/7z/RAR, Read-only | `src/BigDrive.Provider.Archive/` |
| **Zip** | Write operations on archives | ZIP archives, Read-write | `src/BigDrive.Provider.Zip/` |
| **Flickr** | OAuth and cloud APIs | OAuth 1.0a, Caching, Cloud API | `src/BigDrive.Provider.Flickr/` |

### Provider Comparison Matrix

| Feature | VirtualDisk | Iso | Archive | Zip | Flickr |
|---------|------------|-----|---------|-----|--------|
| **Complexity** | ⭐⭐⭐ Medium | ⭐⭐ Simple | ⭐⭐⭐ Medium | ⭐⭐ Simple | ⭐⭐⭐⭐ Complex |
| **Lines of Code** | ~400 | ~250 | ~400 | ~300 | ~800 |
| **Read Files** | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Write Files** | ✅ | ❌ | ❌ | ✅ | ❌ |
| **Delete Files** | ✅ | ❌ | ❌ | ✅ | ❌ |
| **Create Folders** | ✅ | ❌ | ❌ | ✅ | ❌ |
| **OAuth** | ❌ | ❌ | ❌ | ❌ | ✅ OAuth 1.0a |
| **Formats** | 4 formats | ISO/UDF | 4 formats | ZIP only | Cloud API |
| **AssemblyResolver** | ✅ Required | ✅ Required | ✅ Required | ✅ Required | ✅ Required |
| **Best Template For** | VM disks, Read-write | Disc images, Read-only | Archives, Multi-format | Archives, Simple | Cloud APIs, OAuth |

### Documentation Links

- [Getting Started](getting-started.md) - Project setup and architecture
- [Full Development Guide](guide.md) - Complete reference with all details
- [Development Practices](practices.md) - Build-register-test workflow
- [Interfaces Reference](interfaces.md) - All interface definitions
- [NuGet Dependencies](nuget-dependencies.md) - Critical! AssemblyResolver setup
- [OAuth Authentication](oauth-authentication.md) - Cloud service authentication
- [Troubleshooting](troubleshooting.md) - Common errors and solutions

---

## Related Documentation

- [BigDrive Architecture Overview](../architecture/overview.md) - System architecture
- [BigDrive Shell User Guide](../BigDrive.Shell.UserGuide.md) - End-user commands
- [Interface Source Code](../../src/Interfaces/README.txt) - COM interface definitions
- [ConfigProvider Documentation](../../src/ConfigProvider/README.txt) - Registry API

---

## Contributing

Found an issue or want to improve this documentation?

1. Submit issues on GitHub: https://github.com/WayneWalterBerry/BigDrive/issues
2. Submit pull requests with documentation improvements
3. Share your provider implementations as examples

---

**Happy provider development!** 🚀
