# BigDrive.Provider.Archive

## Overview

BigDrive provider that exposes the contents of archive files as a virtual file system
in Windows Explorer and BigDrive.Shell. Supports multiple archive formats using the
SharpCompress library: ZIP, TAR, TAR.GZ, TAR.BZ2, 7z, RAR (read-only), and more.

## Supported Archive Formats

### Writable Formats (Full Support)
- **ZIP** (.zip) — Read/Write
- **TAR** (.tar) — Read/Write

### Read-Only Formats
- **TAR.GZ** (.tar.gz, .tgz) — Gzip-compressed TAR
- **TAR.BZ2** (.tar.bz2, .tbz, .tbz2) — BZip2-compressed TAR
- **7-Zip** (.7z) — 7z archive format
- **RAR** (.rar) — WinRAR archive format
- **GZIP** (.gz) — Single-file compression
- **BZIP2** (.bz2) — Single-file compression

**Read-only behavior**: You can browse, enumerate, and extract files from these formats,
but write operations (copy to archive, delete, move, mkdir) will fail with "not supported" error.

## Architecture

This provider runs as a COM+ ServicedComponent in `dllhost.exe` (out-of-process). It reads
an archive file path from drive-specific configuration and uses SharpCompress library
to enumerate and extract entries on demand.

```
┌──────────────────────────────┐
│  BigDrive.Shell / Explorer   │
│  CoCreateInstance(CLSID)     │
└──────────────┬───────────────┘
               │ COM Activation
               ▼
┌──────────────────────────────┐
│  dllhost.exe (COM+)         │
│  BigDrive.Provider.Archive   │
│  Identity: Interactive User  │
└──────────────┬───────────────┘
               │ SharpCompress
               ▼
┌──────────────────────────────┐
│  Local Archive File          │
│  (ZIP, TAR, 7z, RAR, etc.)   │
│  (path from ArchiveFilePath) │
└──────────────────────────────┘
```

## Drive Configuration

| Property        | Type   | Description                                      |
|----------------|--------|--------------------------------------------------|
| ArchiveFilePath | string | Full path to the local archive file to browse.   |

The archive format is auto-detected from the file extension and content by SharpCompress.

## Implemented Interfaces

| Interface              | Purpose                                      |
|-----------------------|----------------------------------------------|
| IProcessInitializer   | COM+ lifecycle hooks (Startup/Shutdown)       |
| IBigDriveRegistration | Self-registration via regsvcs.exe             |
| IBigDriveDriveInfo    | Drive parameter requirements (ArchiveFilePath)|
| IBigDriveEnumerate    | List folders and files within the archive     |
| IBigDriveFileInfo     | File metadata (size, last modified time)      |
| IBigDriveFileData     | Stream file content from the archive          |
| IBigDriveFileOperations| Copy, move, delete, create (ZIP/TAR only)    |

## File Structure

```
BigDrive.Provider.Archive/
├── Properties/
│   └── AssemblyInfo.cs                      # Assembly attributes, COM+ config
├── Provider.cs                               # Main class, GUID, ComRegister
├── Provider.IProcessInitializer.cs           # Startup/Shutdown
├── Provider.IBigDriveRegistration.cs         # Register/Unregister
├── Provider.IBigDriveDriveInfo.cs            # GetDriveParameters
├── Provider.IBigDriveEnumerate.cs            # EnumerateFolders/EnumerateFiles
├── Provider.IBigDriveFileInfo.cs             # LastModifiedTime/GetFileSize
├── Provider.IBigDriveFileData.cs             # GetFileData (IStream)
├── Provider.IBigDriveFileOperations.cs       # Copy/Move/Delete/CreateDir
├── ArchiveClientWrapper.cs                   # SharpCompress archive logic
├── BigDriveTraceSource.cs                    # Logging
├── ComStream.cs                              # IStream wrapper
├── ProviderConfigurationFactory.cs           # Configuration loading
├── AssemblyResolver.cs                       # NuGet dependency resolution
├── README.md                                 # This file
└── BigDrive.Provider.Archive.csproj          # Project file
```

## Dependencies

### Project References
- **BigDrive.Interfaces** — COM interface definitions
- **BigDrive.ConfigProvider** — Registry-based configuration and DriveManager
- **BigDrive.Service** — Shared service utilities

### NuGet Packages
- **SharpCompress 0.37.2** — Multi-format archive library

### Framework References
- **System.EnterpriseServices** — COM+ ServicedComponent base class

## NuGet Assembly Resolution in COM+

Because providers run in `dllhost.exe` (out-of-process COM+), NuGet dependencies are not
automatically resolved. The `AssemblyResolver.cs` class handles this:

```csharp
// Registered in static constructor
AppDomain.CurrentDomain.AssemblyResolve += ResolveAssembly;

// Loads from provider DLL directory:
// - SharpCompress.dll
// - System.Text.Json.dll (SharpCompress dependency)
// - System.Memory.dll
// - System.Buffers.dll
// - etc.
```

All NuGet dependencies are copied to the provider output directory thanks to:
```xml
<CopyLocalLockFileAssemblies>true</CopyLocalLockFileAssemblies>
```

## Registration

Build the project (PostBuild runs regsvcs.exe automatically), or register manually:

```powershell
# Register (elevated)
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "BigDrive.Provider.Archive.dll"

# Unregister (elevated)
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe /u "BigDrive.Provider.Archive.dll"
```

After registration, the provider appears in the BigDrive provider list:

```
HKLM\SOFTWARE\BigDrive\Providers\{A9B8C7D6-5E4F-3A2B-1C0D-9E8F7A6B5C4D}\
  ├── id   = "{A9B8C7D6-5E4F-3A2B-1C0D-9E8F7A6B5C4D}"
  └── name = "Archive"
```

## Usage

### Mounting an Archive Drive

```powershell
BD> mount

Available providers:
  [1] Archive
      CLSID: a9b8c7d6-5e4f-3a2b-1c0d-9e8f7a6b5c4d

Select provider number: 1
Enter drive name: MyBackups

This provider requires the following parameters:

  Full path to the archive file (ZIP, TAR, TAR.GZ, 7z, RAR).
  ArchiveFilePath: C:\Backups\project2024.tar.gz

Drive mounted successfully!

  Name:     MyBackups
  GUID:     abc123...
  Provider: Archive

Use 'cd Y:' to access the new drive.
```

### Browsing Archive Contents

```powershell
Y:\> dir

 Directory of BigDrive

<DIR>          docs
<DIR>          src
               README.md

Y:\> cd src
Y:\src> dir

<DIR>          BigDrive.Shell
<DIR>          Interfaces
               BigDrive.sln
```

### Extracting Files (Read-Only Archives)

```powershell
Y:\> copy README.md C:\temp\

        1 file(s) copied.  ← Extracts from .tar.gz to local drive
```

### Write Operations (ZIP/TAR Only)

```powershell
# Copying TO a writable archive (ZIP, TAR)
Y:\> copy C:\file.txt .

        1 file(s) copied.  ← Adds to archive

# Trying to write to a read-only archive (7z, RAR, TAR.GZ)
Y:\> copy C:\file.txt .

Error: Archive format is read-only. Write operations only supported for ZIP and TAR.
```

### Supported Operations by Format

| Operation | ZIP | TAR | TAR.GZ | 7z | RAR | GZIP | BZIP2 |
|-----------|-----|-----|--------|----|----|------|-------|
| Enumerate | ✅  | ✅  | ✅     | ✅ | ✅ | ✅   | ✅    |
| Read      | ✅  | ✅  | ✅     | ✅ | ✅ | ✅   | ✅    |
| Write     | ✅  | ✅  | ❌     | ❌ | ❌ | ❌   | ❌    |
| Delete    | ✅  | ✅  | ❌     | ❌ | ❌ | ❌   | ❌    |
| Move      | ✅  | ✅  | ❌     | ❌ | ❌ | ❌   | ❌    |
| CreateDir | ✅  | ✅  | ❌     | ❌ | ❌ | ❌   | ❌    |

## Implementation Details

### ArchiveClientWrapper

The `ArchiveClientWrapper` class provides:
- Per-drive caching of archive file paths
- Format-specific write operations (ZIP vs TAR)
- Read-only format detection
- SharpCompress `ArchiveFactory.Open()` for auto-format detection

```csharp
// Auto-detects format
using (var archive = ArchiveFactory.Open(_archiveFilePath))
{
    // Works for ZIP, TAR, 7z, RAR, etc.
}

// Format-specific write (ZIP)
using (var archive = SharpCompress.Archives.Zip.ZipArchive.Open(_archiveFilePath))
{
    archive.AddEntry(path, fileStream);
    archive.SaveTo(_archiveFilePath, new WriterOptions(CompressionType.Deflate));
}
```

### Creating New Archives

When mounting a **new** archive file (doesn't exist yet):
- Shell accepts the path (uses `filepath` parameter type)
- Provider creates an empty archive on first access
- Supported for ZIP and TAR only (read-only formats cannot be created)

```csharp
// In ArchiveClientWrapper.EnsureArchiveFileExists()
if (extension == ".zip")
{
    using (var archive = SharpCompress.Archives.Zip.ZipArchive.Create())
    {
        archive.SaveTo(stream, new WriterOptions(CompressionType.Deflate));
    }
}
```

### Path Normalization

All paths are normalized to forward slashes (`/`) to match SharpCompress conventions:
- Shell passes: `\folder\file.txt`
- Provider converts to: `folder/file.txt`
- Archive stores: `folder/file.txt` (ZIP/TAR standard)

## Comparison: Archive vs Zip Provider

| Feature           | Zip Provider | Archive Provider |
|------------------|-------------|------------------|
| **Formats**       | ZIP only    | ZIP, TAR, 7z, RAR, TAR.GZ, etc. |
| **Library**       | System.IO.Compression (framework) | SharpCompress (NuGet) |
| **Write Support** | ZIP         | ZIP, TAR only |
| **Dependencies**  | None        | SharpCompress + dependencies |
| **Use Case**      | ZIP-only workflows | Multi-format archive browsing |

**When to use**:
- **Zip Provider**: Lightweight, no dependencies, ZIP-only workflows
- **Archive Provider**: Need to browse TAR, 7z, RAR, or other archive formats

## Known Limitations

1. **Write operations limited to ZIP/TAR**: Other formats are read-only due to SharpCompress API limitations
2. **TAR.GZ write**: Cannot modify `.tar.gz` in place (would need extract → modify TAR → re-compress)
3. **Large files**: Entire file loaded into memory for read operations (SharpCompress design)
4. **Nested archives**: Browsing an archive inside another archive not supported

## Troubleshooting

### "Archive format is read-only"

**Cause**: Trying to copy/delete/move in a .7z, .rar, .tar.gz, or other read-only format

**Solution**: Use the Zip Provider for writable archives, or extract files to local drive first

### "SharpCompress.dll not found"

**Cause**: AssemblyResolver failed to load SharpCompress from provider directory

**Solution**:
1. Verify `SharpCompress.dll` exists in the provider output directory
2. Check that `CopyLocalLockFileAssemblies` is set in the project file
3. Rebuild the provider project

### "Access denied" during registration

**Cause**: regsvcs.exe requires Administrator privileges

**Solution**: Run Visual Studio or regsvcs.exe as Administrator

## Performance Considerations

- **First access**: Archive metadata is read on first operation per drive
- **Caching**: `ArchiveClientWrapper` instances cached per drive GUID
- **Memory**: Files are read into memory (byte arrays) for extraction
- **Large archives**: Performance degrades for archives with 10,000+ entries

For very large archives, consider extracting to local storage first.

## See Also

- [Provider Development Guide](../../docs/ProviderDevelopmentGuide.md)
- [Mount Handshake Protocol](../../docs/architecture/mount-handshake.md)
- [BigDrive.Interfaces README](../Interfaces/README.txt)
- [BigDrive.Provider.Zip](../BigDrive.Provider.Zip/README.txt) — ZIP-only provider
- [SharpCompress Documentation](https://github.com/adamhathcock/sharpcompress)
