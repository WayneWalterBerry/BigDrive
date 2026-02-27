# BigDrive Provider Interfaces Reference

This document describes all COM interfaces that providers can implement.

## Interface Overview

| Interface | Purpose | Required | Return Types |
|-----------|---------|----------|--------------|
| `IProcessInitializer` | COM+ lifecycle hooks | ✅ Yes | `void` |
| `IBigDriveRegistration` | Self-registration | ✅ Yes | `void` |
| `IBigDriveEnumerate` | List folders and files | ✅ Yes | `string[]` |
| `IBigDriveDriveInfo` | Drive parameter definitions | Recommended | `string` (JSON) |
| `IBigDriveFileInfo` | File metadata | Optional | `DateTime`, `ulong` |
| `IBigDriveFileData` | Stream file content | Optional | `int` (HRESULT) |
| `IBigDriveFileOperations` | Copy/delete/mkdir | Optional | `void` |
| `IBigDriveAuthentication` | OAuth authentication | Optional | `int` (HRESULT) |

---

## Core Interfaces (Required)

### IProcessInitializer

**Namespace:** `System.EnterpriseServices`

Called by COM+ when the provider process starts and stops.

```csharp
public interface IProcessInitializer
{
    /// <summary>
    /// Called when the COM+ application starts (dllhost.exe launches).
    /// </summary>
    /// <param name="punkProcessControl">Process control interface (may be null).</param>
    void Startup(object punkProcessControl);

    /// <summary>
    /// Called when the COM+ application shuts down (dllhost.exe terminates).
    /// </summary>
    void Shutdown();
}
```

**Implementation example:**

```csharp
public partial class Provider
{
    public void Startup(object punkProcessControl)
    {
        DefaultTraceSource.TraceInformation("Provider Startup");
        // Initialize API clients, load configuration, etc.
    }

    public void Shutdown()
    {
        DefaultTraceSource.TraceInformation("Provider Shutdown");
        // Clean up resources, dispose clients, etc.
    }
}
```

---

### IBigDriveRegistration

**Namespace:** `BigDrive.Interfaces`

Called during COM+ registration to allow providers to self-register in the BigDrive system.

```csharp
[Guid("FF0FA03A-5DC1-464F-AFCE-5C60ECAA3912")]
public interface IBigDriveRegistration
{
    /// <summary>
    /// Called by regsvcs.exe during installation to register the provider.
    /// </summary>
    void Register();

    /// <summary>
    /// Called by regsvcs.exe during uninstallation to unregister the provider.
    /// </summary>
    void Unregister();
}
```

**Implementation example:**

```csharp
public partial class Provider
{
    public void Register()
    {
        WindowsIdentity identity = WindowsIdentity.GetCurrent();
        DefaultTraceSource.TraceInformation($"Register: User={identity.Name}");

        // Register provider in HKLM\SOFTWARE\BigDrive\Providers\{CLSID}
        ProviderManager.RegisterProvider(ProviderConfig, CancellationToken.None);

        DefaultTraceSource.TraceInformation("Register: Provider registered successfully.");
    }

    public void Unregister()
    {
        DefaultTraceSource.TraceInformation("Unregister: Provider");
        // TODO: Clean up registry entries
    }
}
```

---

### IBigDriveEnumerate

**Namespace:** `BigDrive.Interfaces`

Lists folders and files at a specific path within the virtual drive.

```csharp
[Guid("8A7B6C5D-4E3F-2A1B-0C9D-8E7F6A5B4C3D")]
public interface IBigDriveEnumerate
{
    /// <summary>
    /// Returns folder names (not full paths) at the specified path.
    /// </summary>
    /// <param name="driveGuid">The drive GUID.</param>
    /// <param name="path">The path to enumerate (e.g., "\folder\subfolder").</param>
    /// <returns>Array of folder names.</returns>
    string[] EnumerateFolders(Guid driveGuid, string path);

    /// <summary>
    /// Returns file names (not full paths) at the specified path.
    /// </summary>
    /// <param name="driveGuid">The drive GUID.</param>
    /// <param name="path">The path to enumerate (e.g., "\folder\subfolder").</param>
    /// <returns>Array of file names.</returns>
    string[] EnumerateFiles(Guid driveGuid, string path);
}
```

**Implementation guidelines:**

- Return **empty array** on errors, never `null`
- Return **names only**, not full paths (e.g., `"file.txt"`, not `"\folder\file.txt"`)
- Sanitize names to remove invalid Windows filename characters (`< > : " / \ | ? *`)
- Handle root path variations: `null`, `""`, `"\"`, `"/"`, `"//"` all mean root
- Use `NormalizePath()` helper to standardize path format

**Implementation example:**

```csharp
public partial class Provider
{
    public string[] EnumerateFolders(Guid driveGuid, string path)
    {
        try
        {
            DefaultTraceSource.TraceInformation($"EnumerateFolders: path={path}");

            YourServiceClientWrapper client = GetClient(driveGuid);
            return client.GetFolders(NormalizePath(path));
        }
        catch (Exception ex)
        {
            DefaultTraceSource.TraceError($"EnumerateFolders failed: {ex.Message}");
            return Array.Empty<string>();
        }
    }

    public string[] EnumerateFiles(Guid driveGuid, string path)
    {
        try
        {
            DefaultTraceSource.TraceInformation($"EnumerateFiles: path={path}");

            YourServiceClientWrapper client = GetClient(driveGuid);
            return client.GetFiles(NormalizePath(path));
        }
        catch (Exception ex)
        {
            DefaultTraceSource.TraceError($"EnumerateFiles failed: {ex.Message}");
            return Array.Empty<string>();
        }
    }
}
```

---

## Recommended Interfaces

### IBigDriveDriveInfo

**Namespace:** `BigDrive.Interfaces`

Declares the custom parameters your provider requires when creating a new drive.

```csharp
[Guid("3A2B1C4D-5E6F-7A8B-9C0D-1E2F3A4B5C6D")]
public interface IBigDriveDriveInfo
{
    /// <summary>
    /// Returns a JSON array of parameter definitions.
    /// </summary>
    /// <returns>JSON string: [{"name":"ParamName","description":"...","type":"string"}]</returns>
    string GetDriveParameters();
}
```

**Parameter types:**

| Type | Description | UI Behavior |
|------|-------------|-------------|
| `string` | Plain text input | Standard prompt |
| `existing-file` | Local file path that must exist | Tab completion, validates existence |
| `filepath` | Local file path (may be new) | Tab completion, allows non-existent paths |

**Implementation example:**

```csharp
public partial class Provider
{
    public string GetDriveParameters()
    {
        DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
        {
            new DriveParameterDefinition
            {
                Name = "IsoFilePath",
                Description = "Full path to the ISO file to browse.",
                Type = "existing-file"
            },
            new DriveParameterDefinition
            {
                Name = "CacheFolder",
                Description = "Optional cache directory for extracted files.",
                Type = "filepath"
            }
        };

        return JsonSerializer.Serialize(parameters);
    }
}
```

**Important:** Requires `System.Text.Json` NuGet package. See [nuget-dependencies.md](nuget-dependencies.md).

---

## Optional Interfaces

### IBigDriveFileInfo

**Namespace:** `BigDrive.Interfaces`

Provides file metadata (size, timestamps).

```csharp
[Guid("9B8A7C6D-5E4F-3A2B-1C0D-9E8F7A6B5C4D")]
public interface IBigDriveFileInfo
{
    /// <summary>
    /// Returns the file's last modified time.
    /// </summary>
    /// <param name="driveGuid">The drive GUID.</param>
    /// <param name="path">The file path (e.g., "\folder\file.txt").</param>
    /// <returns>Last modified DateTime, or DateTime.MinValue if not available.</returns>
    DateTime LastModifiedTime(Guid driveGuid, string path);

    /// <summary>
    /// Returns the file size in bytes.
    /// </summary>
    /// <param name="driveGuid">The drive GUID.</param>
    /// <param name="path">The file path (e.g., "\folder\file.txt").</param>
    /// <returns>File size in bytes, or 0 if not available.</returns>
    ulong GetFileSize(Guid driveGuid, string path);
}
```

**Implementation example:**

```csharp
public partial class Provider
{
    public DateTime LastModifiedTime(Guid driveGuid, string path)
    {
        try
        {
            YourServiceClientWrapper client = GetClient(driveGuid);
            return client.GetLastModifiedTime(NormalizePath(path));
        }
        catch
        {
            return DateTime.MinValue;
        }
    }

    public ulong GetFileSize(Guid driveGuid, string path)
    {
        try
        {
            YourServiceClientWrapper client = GetClient(driveGuid);
            return client.GetFileSize(NormalizePath(path));
        }
        catch
        {
            return 0;
        }
    }
}
```

---

### IBigDriveFileData

**Namespace:** `BigDrive.Interfaces`

Streams file content from the provider to BigDrive Shell or Windows Explorer.

```csharp
[Guid("0F471AE9-1787-437F-B230-60CA6717DD04")]
public interface IBigDriveFileData
{
    /// <summary>
    /// Retrieves the file data as an IStream.
    /// </summary>
    /// <param name="driveGuid">The drive GUID.</param>
    /// <param name="path">The file path (e.g., "\folder\file.txt").</param>
    /// <param name="stream">Output IStream for reading file contents.</param>
    /// <returns>HRESULT: 0 = success, negative = error.</returns>
    [PreserveSig]
    int GetFileData(Guid driveGuid, string path, [MarshalAs(UnmanagedType.Interface)] out IStream stream);
}
```

**Implementation example:**

```csharp
public partial class Provider
{
    public int GetFileData(Guid driveGuid, string path, out IStream stream)
    {
        try
        {
            DefaultTraceSource.TraceInformation($"GetFileData: path={path}");

            YourServiceClientWrapper client = GetClient(driveGuid);
            Stream fileStream = client.OpenFile(NormalizePath(path));

            if (fileStream == null)
            {
                stream = null;
                return -1; // E_FAIL
            }

            stream = new ComStream(fileStream);
            return 0; // S_OK
        }
        catch (Exception ex)
        {
            DefaultTraceSource.TraceError($"GetFileData failed: {ex.Message}");
            stream = null;
            return unchecked((int)0x80004005); // E_FAIL
        }
    }
}
```

**Important:**
- Use `ComStream` wrapper class to convert .NET Stream → COM IStream
- Return `0` (S_OK) on success
- Return `unchecked((int)0x80004005)` (E_FAIL) on error
- The `[PreserveSig]` attribute is **required** in the interface definition

---

### IBigDriveFileOperations

**Namespace:** `BigDrive.Interfaces`

Supports write operations (copy, delete, create directory).

```csharp
[Guid("1A2B3C4D-5E6F-7A8B-9C0D-1E2F3A4B5C6D")]
public interface IBigDriveFileOperations
{
    /// <summary>
    /// Copies a local file to the BigDrive provider.
    /// </summary>
    void CopyFileToBigDrive(Guid driveGuid, string localFilePath, string bigDriveTargetPath);

    /// <summary>
    /// Copies a file from the BigDrive provider to local storage.
    /// </summary>
    void CopyFileFromBigDrive(Guid driveGuid, string bigDriveFilePath, string localTargetPath);

    /// <summary>
    /// Deletes a file from the BigDrive provider.
    /// </summary>
    void DeleteFile(Guid driveGuid, string bigDriveFilePath);

    /// <summary>
    /// Creates a directory in the BigDrive provider.
    /// </summary>
    void CreateDirectory(Guid driveGuid, string bigDriveDirectoryPath);
}
```

**Implementation example:**

```csharp
public partial class Provider
{
    public void CopyFileToBigDrive(Guid driveGuid, string localFilePath, string bigDriveTargetPath)
    {
        DefaultTraceSource.TraceInformation($"CopyFileToBigDrive: {localFilePath} -> {bigDriveTargetPath}");

        YourServiceClientWrapper client = GetClient(driveGuid);
        using (FileStream fs = File.OpenRead(localFilePath))
        {
            client.UploadFile(NormalizePath(bigDriveTargetPath), fs);
        }
    }

    public void CopyFileFromBigDrive(Guid driveGuid, string bigDriveFilePath, string localTargetPath)
    {
        DefaultTraceSource.TraceInformation($"CopyFileFromBigDrive: {bigDriveFilePath} -> {localTargetPath}");

        YourServiceClientWrapper client = GetClient(driveGuid);
        using (Stream remoteStream = client.OpenFile(NormalizePath(bigDriveFilePath)))
        using (FileStream fs = File.Create(localTargetPath))
        {
            remoteStream.CopyTo(fs);
        }
    }

    public void DeleteFile(Guid driveGuid, string bigDriveFilePath)
    {
        DefaultTraceSource.TraceInformation($"DeleteFile: {bigDriveFilePath}");

        YourServiceClientWrapper client = GetClient(driveGuid);
        client.DeleteFile(NormalizePath(bigDriveFilePath));
    }

    public void CreateDirectory(Guid driveGuid, string bigDriveDirectoryPath)
    {
        DefaultTraceSource.TraceInformation($"CreateDirectory: {bigDriveDirectoryPath}");

        YourServiceClientWrapper client = GetClient(driveGuid);
        client.CreateDirectory(NormalizePath(bigDriveDirectoryPath));
    }
}
```

**Notes:**
- Only implement if your storage backend supports write operations
- Throw exceptions for unsupported operations (e.g., read-only providers like ISO)
- Use transactions if your backend supports them

---

## Path Format Conventions

**All paths follow these conventions:**

| Convention | Description | Example |
|-----------|-------------|---------|
| **Separator** | Backslash `\` | `"\folder\file.txt"` |
| **Absolute** | Paths start with `\` | `"\folder"`, not `"folder"` |
| **Root variations** | `null`, `""`, `"\"`, `"/"`, `"//"` all mean root | Handle all cases |
| **Normalization** | Convert to forward slashes internally | `path.Trim('\\', '/').Replace('\\', '/')` |

**NormalizePath helper** (add to Provider.cs):

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

## Return Value Guidelines

### String Arrays (EnumerateFolders, EnumerateFiles)

- **Success**: Return array of names
- **Empty directory**: Return `Array.Empty<string>()`
- **Error**: Return `Array.Empty<string>()`, log to trace source
- **Never return**: `null`

### DateTime (LastModifiedTime)

- **Success**: Return actual timestamp
- **Not available**: Return `DateTime.MinValue`
- **Error**: Return `DateTime.MinValue`, log to trace source

### ulong (GetFileSize)

- **Success**: Return file size in bytes
- **Not available**: Return `0`
- **Error**: Return `0`, log to trace source

### HRESULT (GetFileData)

- **Success**: Return `0` (S_OK)
- **Error**: Return `unchecked((int)0x80004005)` (E_FAIL)
- **Set out parameter**: `stream = new ComStream(...)` on success, `stream = null` on error

---

## Error Handling Patterns

### Logging Errors

Always log errors to the trace source before returning:

```csharp
catch (Exception ex)
{
    DefaultTraceSource.TraceError($"MethodName failed: {ex.Message}");
    return Array.Empty<string>(); // or appropriate default
}
```

### Authentication Errors

Throw `BigDriveAuthenticationRequiredException` for auth failures:

```csharp
catch (YourServiceAuthException ex)
{
    throw new BigDriveAuthenticationRequiredException(
        driveGuid,
        "YourService",
        AuthenticationFailureReason.TokenExpired,
        ex);
}
```

BigDrive Shell will catch this and prompt the user to login automatically.

---

## Complete Interface Implementation Checklist

When creating a provider, implement interfaces in this order:

- [ ] **1. IProcessInitializer** - Startup/Shutdown
- [ ] **2. IBigDriveRegistration** - Register/Unregister
- [ ] **3. IBigDriveDriveInfo** - GetDriveParameters (define configuration)
- [ ] **4. IBigDriveEnumerate** - EnumerateFolders/EnumerateFiles (basic browsing)
- [ ] **5. IBigDriveFileInfo** - LastModifiedTime/GetFileSize (metadata)
- [ ] **6. IBigDriveFileData** - GetFileData (download files)
- [ ] **7. IBigDriveFileOperations** - Copy/Delete/CreateDirectory (write operations)
- [ ] **8. IBigDriveAuthentication** - OAuth flows (if cloud service)

---

## See Also

- [Getting Started Guide](getting-started.md) - Project setup and naming conventions
- [OAuth Authentication](oauth-authentication.md) - Implementing OAuth flows
- [NuGet Dependencies](nuget-dependencies.md) - AssemblyResolver and app.config setup
- [Interface Source Code](../../src/Interfaces/) - Full C# interface definitions
