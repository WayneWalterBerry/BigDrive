# BigDrive.Provider.Iso

## Overview

BigDrive provider that exposes the contents of ISO 9660 and UDF disc images as a virtual file system
in Windows Explorer and BigDrive.Shell. The directory structure and files inside the ISO image are
presented as folders and files on a BigDrive virtual drive.

This provider is ideal for browsing CD/DVD images, software distributions, and bootable disc images
without needing to mount them using third-party tools or burn them to physical media.

## Supported ISO Formats

- **ISO 9660** — Standard CD-ROM filesystem (original 8.3 filenames)
- **Joliet extensions** — Long filenames and Unicode character support
- **Rock Ridge extensions** — POSIX file attributes and longer filenames
- **UDF (Universal Disk Format)** — DVD filesystem format
- **Multi-session discs** — Reads the most recent session

## Architecture

This provider runs as a COM+ ServicedComponent in `dllhost.exe` (out-of-process). It reads
an ISO file path from drive-specific configuration and uses the DiscUtils.Iso9660 library
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
│  BigDrive.Provider.Iso       │
│  Identity: Interactive User  │
└──────────────┬───────────────┘
               │ DiscUtils.Iso9660
               ▼
┌──────────────────────────────┐
│  Local ISO File              │
│  (path from IsoFilePath)     │
└──────────────────────────────┘
```

## Drive Configuration

| Property     | Type   | Description                                      |
|-------------|--------|--------------------------------------------------|
| IsoFilePath | string | Full path to the local ISO file to browse.       |

Set the property via the BigDrive.Shell `set` command or directly in the registry:

```
HKLM\SOFTWARE\BigDrive\Drives\{DriveGuid}\IsoFilePath = "C:\path\to\disc.iso"
```

## Implemented Interfaces

| Interface              | Purpose                                      | Notes |
|-----------------------|----------------------------------------------|-------|
| IProcessInitializer   | COM+ lifecycle hooks (Startup/Shutdown)       | Required |
| IBigDriveRegistration | Self-registration via regsvcs.exe             | Required |
| IBigDriveDriveInfo    | Drive parameter requirements (IsoFilePath)    | Required |
| IBigDriveEnumerate    | List folders and files within the ISO image   | Read-only |
| IBigDriveFileInfo     | File metadata (size, last modified time)      | Read-only |
| IBigDriveFileData     | Stream file content from the ISO image        | Read-only |

**Note**: This provider is **read-only**. ISO images cannot be modified after creation. Write operations
(copy to ISO, delete, move, mkdir) are not supported.

## File Structure

```
BigDrive.Provider.Iso/
├── Properties/
│   └── AssemblyInfo.cs                    # Assembly attributes, COM+ config
├── Provider.cs                             # Main class, GUID, ComRegister
├── Provider.IProcessInitializer.cs         # Startup/Shutdown
├── Provider.IBigDriveRegistration.cs       # Register/Unregister
├── Provider.IBigDriveDriveInfo.cs          # GetDriveParameters
├── Provider.IBigDriveEnumerate.cs          # EnumerateFolders/EnumerateFiles
├── Provider.IBigDriveFileInfo.cs           # LastModifiedTime/GetFileSize
├── Provider.IBigDriveFileData.cs           # GetFileData (IStream)
├── IsoClientWrapper.cs                     # DiscUtils ISO reading logic
├── BigDriveTraceSource.cs                  # Logging
├── ComStream.cs                            # IStream wrapper
├── ProviderConfigurationFactory.cs         # Configuration loading
├── AssemblyResolver.cs                     # NuGet dependency resolution
├── README.md                               # This file
└── BigDrive.Provider.Iso.csproj            # Project file
```

## Dependencies

### Project References
- **BigDrive.Interfaces** — COM interface definitions
- **BigDrive.ConfigProvider** — Registry-based configuration and DriveManager
- **BigDrive.Service** — Shared service utilities

### NuGet Packages
- **DiscUtils.Iso9660 0.16.13** — ISO 9660 and UDF disc image library
  - Automatically pulls in: DiscUtils.Core, DiscUtils.Streams

### Framework References
- **System.EnterpriseServices** — COM+ ServicedComponent base class

## NuGet Assembly Resolution in COM+

Because providers run in `dllhost.exe` (out-of-process COM+), NuGet dependencies are not
automatically resolved. The `AssemblyResolver.cs` class handles this:

```csharp
// Registered in static constructor
AppDomain.CurrentDomain.AssemblyResolve += ResolveAssembly;

// Loads from provider DLL directory:
// - DiscUtils.Iso9660.dll
// - DiscUtils.Core.dll
// - DiscUtils.Streams.dll
// - System.Buffers.dll
// - System.Memory.dll
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
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "BigDrive.Provider.Iso.dll"

# Unregister (elevated)
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe /u "BigDrive.Provider.Iso.dll"
```

After registration, the provider appears in the BigDrive provider list:

```
HKLM\SOFTWARE\BigDrive\Providers\{F8E7D6C5-B4A3-9281-7605-4C3B2A1D0E9F}\
  ├── id   = "{F8E7D6C5-B4A3-9281-7605-4C3B2A1D0E9F}"
  └── name = "Iso"
```

## Usage

### Creating an ISO Drive

Using BigDrive.Shell:

```bash
# Create a new ISO drive
bigdrive drive create --provider Iso --name "Windows ISO"

# Set the ISO file path
bigdrive set IsoFilePath "C:\ISOs\windows11.iso"

# List files
bigdrive ls
```

### Browsing in Windows Explorer

Once mounted via the Shell integration, the ISO contents appear as a normal drive in Explorer:

```
💿 WindowsISO (I:)
  └── sources\
      ├── boot.wim
      ├── install.wim
      └── ...
  └── setup.exe
  └── autorun.inf
```

### Extracting Files

```bash
# Copy a file from the ISO to local disk
bigdrive copy "sources\install.wim" "C:\Temp\install.wim"

# Copy entire folder
bigdrive copy "sources" "C:\Temp\sources"
```

## Limitations

1. **Read-Only**: ISO images cannot be modified through this provider. To create or modify ISO files,
   use dedicated ISO authoring tools (mkisofs, genisoimage, etc.)

2. **File System Support**: Only ISO 9660 and UDF formats are supported. Other disc image formats
   (BIN/CUE, MDF/MDS, NRG) are not supported. Use the Archive provider for compressed archives.

3. **Bootable ISOs**: Boot sectors and bootable disc metadata are ignored. Files are accessible,
   but boot information is not exposed.

4. **Multi-Session**: Only the most recent session is read. Earlier sessions are not accessible.

## Comparison with Other Providers

| Feature | Zip Provider | Archive Provider | ISO Provider |
|---------|-------------|-----------------|--------------|
| **Format** | ZIP only | ZIP, TAR, 7z, RAR | ISO 9660, UDF |
| **Read** | ✅ Yes | ✅ Yes | ✅ Yes |
| **Write** | ✅ Yes | ✅ Yes (ZIP/TAR) | ❌ No |
| **Compression** | ✅ DEFLATE | ✅ Multiple | ❌ No |
| **Use Case** | General archives | Multi-format archives | Disc images |

## Common Use Cases

### Software Distribution
Browse installation media without burning to disc:
```bash
bigdrive drive create --provider Iso --name "Office2021"
bigdrive set IsoFilePath "C:\ISOs\Office2021.iso"
bigdrive ls
```

### Operating System Images
Access Windows or Linux installation files:
```bash
bigdrive drive create --provider Iso --name "Ubuntu"
bigdrive set IsoFilePath "C:\ISOs\ubuntu-22.04-desktop-amd64.iso"
bigdrive copy "casper\vmlinuz" "C:\Temp\"
```

### Archive Browsing
View contents of archived disc images without extraction:
```bash
bigdrive drive create --provider Iso --name "OldGames"
bigdrive set IsoFilePath "C:\Archive\RetroGames.iso"
bigdrive ls
```

## Troubleshooting

### "File not found" error
- Verify the IsoFilePath property points to an existing ISO file
- Check file permissions (provider runs as Interactive User)

### "Format not supported" error
- Ensure the file is a valid ISO 9660 or UDF image
- Some exotic formats (BIN/CUE) require conversion to ISO first

### Empty directory listings
- The ISO may be corrupted or use an unsupported format
- Try opening in another ISO reader to verify integrity

### Assembly load errors
- Ensure all DiscUtils DLLs are in the same directory as the provider DLL
- Check Windows Event Log for detailed error messages

## See Also

- [Provider Development Guide](../../docs/ProviderDevelopmentGuide.md) — Creating custom providers
- [BigDrive Shell User Guide](../../docs/BigDrive.Shell.UserGuide.md) — Shell commands reference
- [Archive Provider](../BigDrive.Provider.Archive/README.md) — Multi-format archive support
- [Zip Provider](../BigDrive.Provider.Zip/README.txt) — ZIP-specific provider

## Technical Notes

### Filename Sanitization
ISO 9660 uses version suffixes on filenames (e.g., `FILE.TXT;1`). The provider automatically
strips these suffixes when enumerating files to present clean filenames to users.

### Path Conversion
BigDrive uses forward slashes (`/`) for path separators, while DiscUtils uses backslashes (`\`).
The `IsoClientWrapper.ConvertToIsoPath()` method handles this conversion transparently.

### Memory Management
Files are copied to memory streams to allow the CDReader and FileStream to be disposed promptly.
For large files (>100MB), this may cause memory pressure. Future versions may implement streaming
without full memory buffering.
