# BigDrive.Provider.Zip

## Overview

BigDrive provider that exposes the contents of a local ZIP archive as a virtual file system
in Windows Explorer and BigDrive.Shell. The directory structure and files inside the ZIP are
presented as folders and files on a BigDrive virtual drive.

## Architecture

This provider runs as a COM+ ServicedComponent in `dllhost.exe` (out-of-process). It reads
a ZIP file path from drive-specific configuration and uses `System.IO.Compression.ZipArchive`
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
│  BigDrive.Provider.Zip      │
│  Identity: Interactive User  │
└──────────────┬───────────────┘
               │ System.IO.Compression
               ▼
┌──────────────────────────────┐
│  Local ZIP File              │
│  (path from ZipFilePath)     │
└──────────────────────────────┘
```

## Drive Configuration

| Property     | Type   | Description                                      |
|-------------|--------|--------------------------------------------------|
| ZipFilePath | string | Full path to the local ZIP file to browse.       |

Set the property via the BigDrive.Shell `set` command or directly in the registry:

```
HKLM\SOFTWARE\BigDrive\Drives\{DriveGuid}\ZipFilePath = "C:\path\to\archive.zip"
```

## Implemented Interfaces

| Interface              | Purpose                                      |
|-----------------------|----------------------------------------------|
| IProcessInitializer   | COM+ lifecycle hooks (Startup/Shutdown)       |
| IBigDriveRegistration | Self-registration via regsvcs.exe             |
| IBigDriveEnumerate    | List folders and files within the ZIP archive |
| IBigDriveFileInfo     | File metadata (size, last modified time)      |
| IBigDriveFileData     | Stream file content from the ZIP archive      |

## File Structure

```
BigDrive.Provider.Zip/
├── Properties/
│   └── AssemblyInfo.cs                    # Assembly attributes, COM+ config
├── Provider.cs                             # Main class, GUID, ComRegister
├── Provider.IProcessInitializer.cs         # Startup/Shutdown
├── Provider.IBigDriveRegistration.cs       # Register/Unregister
├── Provider.IBigDriveEnumerate.cs          # EnumerateFolders/EnumerateFiles
├── Provider.IBigDriveFileInfo.cs           # LastModifiedTime/GetFileSize
├── Provider.IBigDriveFileData.cs           # GetFileData (IStream)
├── ZipClientWrapper.cs                     # ZIP archive reading logic
├── BigDriveTraceSource.cs                  # Logging
├── ComStream.cs                            # IStream wrapper
├── ProviderConfigurationFactory.cs         # Configuration loading
├── AssemblyResolver.cs                     # NuGet dependency resolution
├── README.txt                              # This file
└── BigDrive.Provider.Zip.csproj            # Project file
```

## Dependencies

- **BigDrive.Interfaces** — COM interface definitions
- **BigDrive.ConfigProvider** — Registry-based configuration and DriveManager
- **BigDrive.Service** — Shared service utilities
- **System.EnterpriseServices** — COM+ ServicedComponent base class
- **System.IO.Compression** — ZIP archive reading (built-in .NET Framework)

No external NuGet packages are required.

## Registration

Build the project (PostBuild runs regsvcs.exe automatically), or register manually:

```powershell
# Register (elevated)
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "BigDrive.Provider.Zip.dll"

# Unregister (elevated)
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe /u "BigDrive.Provider.Zip.dll"
```

## Usage

1. Register the provider (build or run regsvcs.exe)
2. Set the ZipFilePath property for the drive
3. Browse the ZIP contents via BigDrive.Shell or Windows Explorer

```powershell
BD> set Z: ZipFilePath "C:\Archives\myfiles.zip"
BD> cd Z:
Z:\> dir
    <DIR>    Documents
    <DIR>    Images
             readme.txt
```

## See Also

- [Provider Development Guide](../../docs/ProviderDevelopmentGuide.md)
- [BigDrive.Interfaces README](../Interfaces/README.txt)
- [BigDrive.Provider.Flickr](../BigDrive.Provider.Flickr/) — Reference implementation
