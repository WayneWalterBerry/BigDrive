# BigDrive.Provider.VirtualDisk

BigDrive provider for virtual machine disk images (VHD, VHDX, VMDK, VDI).

## Overview

This provider enables browsing and modifying virtual machine disk images as if they were
mounted drives, **without** requiring Hyper-V, VMware, or VirtualBox to be running.

**Supported formats:**
- ✅ **VHD** - Hyper-V legacy format (fixed, dynamic, differencing)
- ✅ **VHDX** - Hyper-V modern format (up to 64 TB)
- ✅ **VMDK** - VMware disk format
- ✅ **VDI** - VirtualBox disk format

**Supported file systems:**
- ✅ **NTFS** - Windows file system (read/write)
- ✅ **FAT32** - Legacy file system (read/write)
- ✅ **exFAT** - Large file support (read/write)
- ✅ **ext2/3/4** - Linux file systems (read/write)

---

## Architecture

### Provider Process Model

```
BigDrive.Shell
       │
       │ COM Activation
       ▼
   dllhost.exe (Interactive User)
       │
       ├─► BigDrive.Provider.VirtualDisk.dll
       │       │
       │       ├─► DiscUtils.Vhd/Vhdx/Vmdk/Vdi
       │       │       │
       │       │       └─► Opens disk.vhdx (FileAccess.ReadWrite)
       │       │
       │       ├─► DiscUtils.Partitions
       │       │       │
       │       │       └─► Detects MBR/GPT partition table
       │       │
       │       └─► DiscUtils.Ntfs/Fat/Ext
       │               │
       │               └─► Opens file system on partition
       │
       └─► Returns file listings, streams files, writes changes
```

**Key architectural constraints:**

1. **Out-of-process COM+ activation** - Provider runs in `dllhost.exe`, not `explorer.exe`
2. **Interactive User identity** - Provider runs as logged-in user for file access
3. **Per-drive file handles** - Each mounted drive opens its own file handle to the VHD
4. **File system abstraction** - DiscUtils provides unified API across NTFS/FAT/ext

---

## How It Works

### 1. Disk Format Detection

```csharp
// Auto-detect format from file extension
string ext = Path.GetExtension(diskPath).ToLowerInvariant();

switch (ext)
{
    case ".vhd":   return new DiscUtils.Vhd.Disk(diskPath, access);
    case ".vhdx":  return new DiscUtils.Vhdx.Disk(diskPath, access);
    case ".vmdk":  return new DiscUtils.Vmdk.Disk(diskPath, access);
    case ".vdi":   return new DiscUtils.Vdi.Disk(diskPath, access);
}
```

### 2. Partition Table Access

```csharp
// Get partition table (MBR or GPT)
PartitionTable partitionTable = disk.Partitions;

// Access specific partition
PartitionInfo partition = partitionTable[partitionIndex];
```

### 3. File System Detection

```csharp
// DiscUtils auto-detects NTFS, FAT32, ext2/3/4, XFS, Btrfs, HFS+
using (SparseStream partitionStream = partition.Open())
{
    FileSystemInfo fsInfo = FileSystemManager.DetectFileSystems(partitionStream).FirstOrDefault();
    DiscFileSystem fileSystem = fsInfo.Open(partitionStream);
}
```

### 4. File Operations

```csharp
// Read operations (works with all file systems)
string[] folders = fileSystem.GetDirectories(path);
string[] files = fileSystem.GetFiles(path);
Stream fileStream = fileSystem.OpenFile(path, FileMode.Open);

// Write operations (if not read-only)
using (Stream target = fileSystem.OpenFile(path, FileMode.Create))
{
    sourceStream.CopyTo(target);
}

fileSystem.DeleteFile(path);
fileSystem.CreateDirectory(path);
```

---

## Usage

### Create and Mount a Virtual Disk Drive

```sh
# Create drive
bigdrive drive create --provider VirtualDisk --name "MyVM"

# Configure VHD file path
bigdrive set VhdFilePath "C:\VMs\Windows10.vhdx"

# Optional: Specify partition (default: 0)
bigdrive set PartitionIndex "0"

# Optional: Mount read-only (default: false)
bigdrive set ReadOnly "false"

# Browse files
bigdrive ls
bigdrive cd "Users\John\Documents"
bigdrive ls
```

### Read Operations

```sh
# Copy file from VHD to local disk
bigdrive copy "config.txt" "C:\Backup\"

# View file
bigdrive type "readme.txt"

# List directory
bigdrive ls "Program Files"
```

### Write Operations

```sh
# Copy file to VHD
bigdrive copy "C:\Files\data.txt" "Temp\data.txt"

# Delete file from VHD
bigdrive del "Temp\oldfile.txt"

# Create directory in VHD
bigdrive mkdir "Temp\NewFolder"
```

### Read-Only Mode

For safety, mount active VM disks read-only:

```sh
bigdrive set VhdFilePath "C:\VMs\ProductionServer.vhdx"
bigdrive set ReadOnly "true"

# Now only read operations work
bigdrive ls              # ✅ Works
bigdrive copy "log.txt" "C:\Logs\"  # ✅ Works
bigdrive del "file.txt"  # ❌ Error: "Virtual disk is mounted read-only"
```

---

## Supported Scenarios

### 1. Browse VM Disk Without Starting VM

```sh
bigdrive drive create --provider VirtualDisk --name "DevVM"
bigdrive set VhdFilePath "C:\VMs\Development.vhdx"
bigdrive ls "Users"
bigdrive copy "Users\Developer\project.zip" "C:\Backup\"
```

### 2. Extract Files from Windows Backup VHD

```sh
bigdrive drive create --provider VirtualDisk --name "Backup"
bigdrive set VhdFilePath "C:\Backups\2025-01-15-WindowsBackup.vhd"
bigdrive set ReadOnly "true"  # Safe mode
bigdrive copy "Users\John\Documents\*" "C:\Restore\" --recursive
```

### 3. Modify VMware VMDK

```sh
bigdrive drive create --provider VirtualDisk --name "VMware"
bigdrive set VhdFilePath "C:\VMs\Ubuntu.vmdk"
bigdrive copy "C:\Scripts\setup.sh" "home\user\setup.sh"
bigdrive mkdir "home\user\logs"
```

### 4. Access VirtualBox VDI

```sh
bigdrive drive create --provider VirtualDisk --name "VBox"
bigdrive set VhdFilePath "C:\VirtualBox\Debian.vdi"
bigdrive ls "etc"
bigdrive copy "etc\hosts" "C:\Temp\"
```

---

## Configuration Parameters

### VhdFilePath (Required)

Full path to the virtual disk file.

**Valid formats:** `.vhd`, `.vhdx`, `.vmdk`, `.vdi`

```sh
bigdrive set VhdFilePath "C:\Hyper-V\Virtual Hard Disks\Ubuntu.vhdx"
```

### PartitionIndex (Optional, Default: 0)

Zero-based index of the partition to mount.

Most virtual disks have a single partition (index 0). Multi-partition disks:
- **Windows**: Partition 0 = System Reserved (100-500 MB), Partition 1 = C: drive
- **Linux**: Partition 0 = boot, Partition 1 = root, Partition 2 = swap

```sh
# Mount first partition (typical Windows C: drive or Linux root)
bigdrive set PartitionIndex "0"

# Mount second partition (if multi-partition disk)
bigdrive set PartitionIndex "1"
```

**Special value: -1** lists all partitions:

```sh
bigdrive set PartitionIndex "-1"
bigdrive ls
# Output:
# Partition0 (100 MB, NTFS)
# Partition1 (127 GB, NTFS)
```

### ReadOnly (Optional, Default: false)

Mount disk in read-only mode for safety.

```sh
# Read-write (can modify files)
bigdrive set ReadOnly "false"

# Read-only (prevents accidental changes)
bigdrive set ReadOnly "true"
```

**Use read-only when:**
- Browsing production VM disks
- Extracting from backups
- VHD is in use by Hyper-V/VMware/VirtualBox

---

## Limitations

### 1. File Locking

**Problem:** Windows locks VHD files when Hyper-V/VMware/VirtualBox is using them.

**Solution:** 
- Stop the VM before mounting with BigDrive
- OR mount read-only (may still fail if VM has exclusive lock)

```sh
# If you see "The process cannot access the file because it is being used by another process"
# Stop the VM first, then mount
```

### 2. Partition Limitations

**Single partition default:** Provider mounts only one partition at a time (specified by PartitionIndex).

**Workaround:** Create multiple drives pointing to the same VHD with different partition indices:

```sh
bigdrive drive create --provider VirtualDisk --name "VM-Boot"
bigdrive set VhdFilePath "C:\VMs\disk.vhdx"
bigdrive set PartitionIndex "0"  # Boot partition

bigdrive drive create --provider VirtualDisk --name "VM-System"
bigdrive set VhdFilePath "C:\VMs\disk.vhdx"
bigdrive set PartitionIndex "1"  # System partition
```

### 3. Unsupported File Systems

DiscUtils supports most common file systems, but some are **read-only**:
- ✅ NTFS, FAT32, exFAT, ext2/3/4 - Full read-write
- ⚠️ XFS, Btrfs, HFS+ - Read-only

**Error:** "Write operations not supported for this file system"

### 4. Dynamic Disk Limitations

**Fixed-size VHDs:** No expansion needed, always works.

**Dynamically expanding VHDs:** DiscUtils can expand them automatically as files are added.

**Differencing/snapshot disks:** May have limitations depending on format.

---

## File System Compatibility

| File System | Read | Write | Delete | Mkdir | Notes |
|-------------|------|-------|--------|-------|-------|
| **NTFS** | ✅ | ✅ | ✅ | ✅ | Full support |
| **FAT32** | ✅ | ✅ | ✅ | ✅ | Full support |
| **exFAT** | ✅ | ✅ | ✅ | ✅ | Full support |
| **ext2** | ✅ | ✅ | ✅ | ✅ | Full support |
| **ext3** | ✅ | ✅ | ✅ | ✅ | Full support |
| **ext4** | ✅ | ✅ | ✅ | ✅ | Full support |
| **XFS** | ✅ | ❌ | ❌ | ❌ | Read-only |
| **Btrfs** | ✅ | ❌ | ❌ | ❌ | Read-only |
| **HFS+** | ✅ | ⚠️ | ⚠️ | ⚠️ | Limited write support |

---

## NuGet Dependencies

This provider uses **DiscUtils** libraries for virtual disk access:

```xml
<PackageReference Include="DiscUtils.Vhd" Version="0.16.13" />
<PackageReference Include="DiscUtils.Vhdx" Version="0.16.13" />
<PackageReference Include="DiscUtils.Vmdk" Version="0.16.13" />
<PackageReference Include="DiscUtils.Vdi" Version="0.16.13" />
<PackageReference Include="DiscUtils.Ntfs" Version="0.16.13" />
<PackageReference Include="DiscUtils.Fat" Version="0.16.13" />
<PackageReference Include="System.Text.Json" Version="9.0.5" />
```

**CRITICAL:** AssemblyResolver and app.config are **required** for these packages to load correctly in COM+.

See: [Provider Development Guide - NuGet Dependencies](../../docs/provider-development/nuget-dependencies.md)

---

## Comparison with Other Providers

| Feature | ISO Provider | Archive Provider | VirtualDisk Provider |
|---------|-------------|-----------------|---------------------|
| **Format** | ISO 9660, UDF | ZIP, TAR, 7z, RAR | VHD, VHDX, VMDK, VDI |
| **Read** | ✅ Yes | ✅ Yes | ✅ Yes |
| **Write** | ❌ No | ⚠️ Limited | ✅ Yes |
| **Delete** | ❌ No | ❌ No | ✅ Yes |
| **Mkdir** | ❌ No | ❌ No | ✅ Yes |
| **File Systems** | ISO 9660 only | N/A | NTFS, FAT32, ext4, XFS |
| **Use Case** | Disc images | Compressed archives | Virtual machine disks |

---

## Common Use Cases

### VM Development Workflow

Share files with VM without network or shared folders:

```sh
# Mount VM disk
bigdrive drive create --provider VirtualDisk --name "DevVM"
bigdrive set VhdFilePath "C:\VMs\Development.vhdx"

# Copy build artifacts to VM
bigdrive copy "C:\Build\app.exe" "Users\Developer\Desktop\app.exe"

# Extract logs from VM
bigdrive copy "Windows\Logs\*" "C:\AnalysisLogs\" --recursive
```

### Backup Recovery

Extract specific files from VM backups:

```sh
bigdrive drive create --provider VirtualDisk --name "Backup"
bigdrive set VhdFilePath "C:\Backups\Server-2025-01-15.vhd"
bigdrive set ReadOnly "true"  # Safety first

# Recover database
bigdrive copy "Program Files\SQL Server\Data\MyDB.mdf" "C:\Recovery\"
```

### Multi-Platform Development

Access Linux VM disks from Windows:

```sh
bigdrive drive create --provider VirtualDisk --name "LinuxVM"
bigdrive set VhdFilePath "C:\VMs\Ubuntu.vdi"

# Browse Linux file system
bigdrive ls "etc"
bigdrive copy "etc\nginx\nginx.conf" "C:\Config\"

# Deploy code to Linux VM
bigdrive copy "C:\Code\webapp\*" "var\www\html\" --recursive
```

---

## Performance Characteristics

### Read Operations

| Operation | Speed | Notes |
|-----------|-------|-------|
| Enumerate folders | Fast | Direct file system access |
| Enumerate files | Fast | No extraction needed |
| Open file stream | Fast | Direct read from disk image |
| Copy large files | Fast | Streaming, no memory buffering |

### Write Operations

| Operation | Speed | Notes |
|-----------|-------|-------|
| Copy to VHD | Medium | Writes to disk image file |
| Delete file | Fast | File system metadata update |
| Create directory | Fast | File system metadata update |

**Performance tips:**

1. **Use dynamically expanding VHDs** for better disk space utilization
2. **Mount read-only** when only browsing (prevents file system journal updates)
3. **Close drive** when done (flushes file system cache)

---

## Error Handling

### File Locking Errors

**Error:** "The process cannot access the file 'disk.vhdx' because it is being used by another process."

**Cause:** VHD is open in Hyper-V, VMware, or another BigDrive mount.

**Solution:**
1. Stop the VM or unmount the disk in Hyper-V Manager
2. OR mount as read-only: `bigdrive set ReadOnly "true"`

### Partition Not Found

**Error:** "Partition index 0 is out of range. Disk has 0 partition(s)."

**Cause:** Disk has no partition table (raw file system) or corrupted.

**Solution:** Use a partitioned disk image, or investigate with disk tools.

### File System Not Supported

**Error:** "No supported file system detected on partition 0."

**Cause:** Partition is unformatted, encrypted, or uses unsupported file system.

**Solution:** Format partition, decrypt, or use compatible file system.

### Write Denied in Read-Only Mode

**Error:** "Virtual disk is mounted read-only. Set ReadOnly=false to enable write operations."

**Cause:** Drive is mounted with `ReadOnly="true"`.

**Solution:**
```sh
bigdrive set ReadOnly "false"
# Or unmount and remount without read-only flag
```

---

## Technical Details

### DiscUtils Library Architecture

```
VirtualDisk (abstract)
    ├── Vhd.Disk
    ├── Vhdx.Disk
    ├── Vmdk.Disk
    └── Vdi.Disk
            │
            ├── .Partitions → PartitionTable
            │                     │
            │                     └── [PartitionInfo]
            │                             │
            └───────────────────────────────► .Open() → SparseStream
                                                            │
                                              FileSystemManager.DetectFileSystems()
                                                            │
                                                            ▼
                                                     DiscFileSystem (abstract)
                                                          ├── NtfsFileSystem
                                                          ├── FatFileSystem
                                                          ├── ExtFileSystem
                                                          └── XfsFileSystem
```

### Path Normalization

**Input paths from Shell:** `\folder\file.txt` (backslash, absolute)

**DiscUtils expects:** `folder/file.txt` (forward slash, relative to root)

**NormalizePath() method:**
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

### Client Wrapper Caching

Each drive gets its own cached `VirtualDiskClientWrapper` instance to avoid repeated
disk/partition/file system initialization:

```csharp
private static readonly Dictionary<Guid, VirtualDiskClientWrapper> ClientCache;

public static VirtualDiskClientWrapper GetForDrive(Guid driveGuid)
{
    lock (ClientCache)
    {
        if (!ClientCache.TryGetValue(driveGuid, out VirtualDiskClientWrapper client))
        {
            client = new VirtualDiskClientWrapper(driveGuid);
            ClientCache[driveGuid] = client;
        }

        return client;
    }
}
```

**Disposal:** All clients are disposed in `Provider.Shutdown()` to flush file system changes.

---

## Security Considerations

### 1. File Access Permissions

Provider runs as **Interactive User** (logged-in user), so:
- ✅ Can access user's VHD files in `C:\Users\{username}\`
- ✅ Can write to user's directories
- ❌ Cannot access VHDs in protected system directories (unless user is admin)

### 2. VM Security

**Mounting a running VM's disk can corrupt it!**

Always:
- ✅ Stop the VM before mounting read-write
- ✅ Use read-only mode for active VMs (if the VM allows shared read access)
- ✅ Create backups before modifying production VHDs

### 3. File System Integrity

DiscUtils maintains file system integrity through:
- ✅ NTFS transaction journaling
- ✅ FAT32 cluster allocation tracking
- ✅ Proper disposal flushes all pending writes

**Best practice:** Call `bigdrive exit` to ensure proper shutdown and file system flush.

---

## Troubleshooting

### Check Event Log

```powershell
Get-EventLog -LogName Application -Source "BigDrive.Provider.VirtualDisk" -Newest 20
```

### Common Issues

**1. Assembly load failures** → See [NuGet Dependencies Guide](../../docs/provider-development/nuget-dependencies.md)

**2. File locking errors** → Stop VMs, close Hyper-V Manager

**3. Partition detection** → Use Disk Management (diskmgmt.msc) to inspect VHD structure

**4. File system errors** → Mount in Windows (Mount-DiskImage) to run chkdsk

---

## Future Enhancements

### Potential Features

- [ ] **Multi-partition view** - Show all partitions as folders (`Partition0\`, `Partition1\`)
- [ ] **VHDX metadata** - Expose virtual disk properties (size, format, parent)
- [ ] **Snapshot support** - Browse differencing disk chains
- [ ] **Dynamic expansion** - Auto-grow VHDs as files are added
- [ ] **Compression** - Compact VHDs after deletions

---

## See Also

- [Provider Development Guide](../../docs/provider-development/README.md)
- [ISO Provider](../BigDrive.Provider.Iso/README.md) - Similar read-only provider
- [Archive Provider](../BigDrive.Provider.Archive/README.md) - Similar multi-format provider
- [DiscUtils Documentation](https://github.com/DiscUtils/DiscUtils) - Library documentation
