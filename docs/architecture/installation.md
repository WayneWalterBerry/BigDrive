# Installation Architecture

## Overview

BigDrive uses a two-phase approach:
1. **Installation** (requires Administrator) — Registers COM+ providers and shell extensions
2. **Drive Management** (standard user) — Mount/unmount drives via BigDrive.Shell

---

## Installation Process

### BigDrive.Setup.exe

When running `BigDrive.Setup.exe` with elevated permissions (as Administrator):

1. **COM+ Provider Registration**
   - Providers (e.g., `BigDrive.Provider.Flickr.dll`) are registered using `regsvcs.exe`
   - Each provider is installed as a COM+ Server Application running in `dllhost.exe`
   - Providers implement `IBigDriveRegistration.Register()` to write to the registry

2. **Service Account Creation**
   - A local user named `BigDriveTrustedInstaller` is created
   - COM+ applications run under this identity for process isolation
   - Password is discarded after setup

3. **Registry Structure Created**
   ```
   HKLM\SOFTWARE\BigDrive\
   ├── Providers\{CLSID}\          ← One per installed provider
   │   ├── id = "{CLSID}"
   │   └── name = "Flickr Provider"
   │
   └── Drives\{GUID}\              ← Created later by mount command
       ├── id = "{DRIVE-GUID}"
       ├── name = "My Flickr Photos"
       └── clsid = "{PROVIDER-CLSID}"
   ```

4. **Shell Extension Installation** (optional)
   - `BigDrive.ShellFolder.dll` registered for Explorer integration
   - Context menu extension for "This PC"

### Notes

- **Idempotency**: `BigDrive.Setup.exe` can be run multiple times safely
- **Provider Independence**: Each provider is a separate COM+ application

---

## Drive Management (Post-Installation)

After installation, drives are managed via **BigDrive.Shell**:

### Mounting a Drive

```
BD> mount

Available providers:

  [1] Flickr Provider
      CLSID: {B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}

Select provider number: 1
Enter drive name: My Flickr Photos

Drive mounted successfully!
Use 'cd Z:' to access the new drive.
```

This creates a registry entry under `SOFTWARE\BigDrive\Drives\{GUID}`.

### Unmounting a Drive

```
Z:\> unmount Z
Unmount 'My Flickr Photos' (Z:)? [y/N]: y
Drive unmounted: My Flickr Photos
```

This removes the registry entry.

### No Elevation Required

- BigDrive.Shell writes to `HKLM\SOFTWARE\BigDrive\Drives`
- Standard users can mount/unmount if registry permissions allow
- Alternatively, run BigDrive.Shell as Administrator

---

## Component Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│  Installation (One-time, requires Admin)                                │
│                                                                         │
│  BigDrive.Setup.exe                                                     │
│       │                                                                 │
│       ├── regsvcs.exe Provider.Flickr.dll                               │
│       │       └── Creates COM+ Application                              │
│       │       └── Calls IBigDriveRegistration.Register()                │
│       │               └── Writes to SOFTWARE\BigDrive\Providers\{CLSID} │
│       │                                                                 │
│       ├── regsvcs.exe Provider.Sample.dll                               │
│       │       └── (same as above)                                       │
│       │                                                                 │
│       └── regsvr32 BigDrive.ShellFolder.dll (optional)                  │
│               └── Explorer integration                                  │
└─────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────┐
│  Runtime (Standard user)                                                │
│                                                                         │
│  BigDrive.Shell.exe                                                     │
│       │                                                                 │
│       ├── mount command                                                 │
│       │       └── Reads SOFTWARE\BigDrive\Providers\*                   │
│       │       └── Writes SOFTWARE\BigDrive\Drives\{GUID}                │
│       │                                                                 │
│       ├── dir, cd, copy commands                                        │
│       │       └── Reads SOFTWARE\BigDrive\Drives\*                      │
│       │       └── CoCreateInstance(CLSID) → dllhost.exe                 │
│       │       └── Calls IBigDriveEnumerate, IBigDriveFileData, etc.     │
│       │                                                                 │
│       └── unmount command                                               │
│               └── Deletes SOFTWARE\BigDrive\Drives\{GUID}               │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Registry Permissions

For standard users to mount/unmount drives:

```
HKLM\SOFTWARE\BigDrive\Drives
    → Grant "Users" group: Full Control
```

Or require BigDrive.Shell to run as Administrator.

---

## See Also

- [BigDrive.Shell User Guide](../BigDrive.Shell.UserGuide.md) — mount/unmount commands
- [Mount Command Handshake](mount-handshake.md) — Detailed drive creation protocol
- [Provider Development Guide](../ProviderDevelopmentGuide.md) — Creating providers
- [BigDrive.Setup README](../../src/BigDrive.Setup/README.txt) — Setup internals
