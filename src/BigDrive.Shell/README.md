# BigDrive.Shell

A custom command-line shell for interacting with BigDrive virtual file systems.

## Overview

BigDrive.Shell provides a command-line interface for navigating and managing BigDrive
providers. It assigns drive letters (Z:, Y:, X:...) to registered BigDrives and
supports cross-drive file operations including mount/unmount of drives.

## User Documentation

For usage instructions, see the **[User Guide](../../docs/BigDrive.Shell.UserGuide.md)**.

---

## Project Structure

```
BigDrive.Shell/
├── Program.cs                 # Entry point, REPL loop
├── ShellContext.cs            # Session state (current drive, paths)
├── DriveLetterManager.cs      # Assigns drive letters, avoids OS conflicts
├── CommandProcessor.cs        # Parses input, dispatches to commands
├── ProviderFactory.cs         # COM+ provider activation
├── PathInfo.cs                # Path parsing utilities
├── OAuthHelper.cs             # OAuth 2.0 authentication flows
├── OAuth1Helper.cs            # OAuth 1.0a authentication flows (for Flickr)
├── Commands/
│   ├── ICommand.cs            # Command interface
│   ├── HelpCommand.cs
│   ├── ExitCommand.cs
│   ├── DrivesCommand.cs
│   ├── DirCommand.cs
│   ├── CdCommand.cs
│   ├── CopyCommand.cs
│   ├── MkdirCommand.cs
│   ├── DelCommand.cs
│   ├── MountCommand.cs        # Create new drive (like 'net use')
│   ├── UnmountCommand.cs      # Remove drive
│   ├── SecretCommand.cs       # Manage secrets in Windows Credential Manager
│   ├── LoginCommand.cs        # OAuth authentication
│   ├── LogoutCommand.cs       # Clear authentication tokens
│   └── AuthStatusCommand.cs   # Check authentication status
└── Properties/
    └── AssemblyInfo.cs
```

---

## Architecture

### Key Components

| Component | Responsibility |
|-----------|----------------|
| `ShellContext` | Current drive letter, path per drive, session state |
| `DriveLetterManager` | Assigns drive letters to BigDrives, avoids OS drives |
| `CommandProcessor` | Parses input, dispatches to command handlers |
| `ProviderFactory` | Creates COM+ provider instances via COM activation |
| `ICommand` | Interface implemented by all shell commands |

### Registry Structure

BigDrive uses two registry locations under `HKLM\SOFTWARE\BigDrive`:

```
SOFTWARE\BigDrive\
├── Providers\                    ← Registered by regsvcs.exe (COM+ installation)
│   └── {PROVIDER-CLSID}\
│       ├── id   = "{CLSID}"
│       └── name = "Flickr Provider"
│
└── Drives\                       ← Created by 'mount' command or setup
    └── {DRIVE-GUID}\
        ├── id   = "{DRIVE-GUID}"
        ├── name = "My Flickr Photos"
        └── clsid = "{PROVIDER-CLSID}"   ← Points to provider
```

**Providers** are COM+ components registered during installation. They define *how* to
access a storage backend (Flickr API, Azure Blob, etc.).

**Drives** are user-created instances that use a provider. Multiple drives can use the
same provider (e.g., two different Flickr accounts).

### COM+ Provider Activation

The shell **never directly references provider assemblies**. Instead:

1. **Configuration** is read from the registry via `DriveManager.ReadConfigurations()`

2. **Provider activation** uses COM interop:
   ```csharp
   // Get CLSID from drive configuration (registry)
   DriveConfiguration config = DriveManager.ReadConfiguration(driveGuid, token);

   // Activate provider via COM (runs in dllhost.exe under COM+)
   Type providerType = Type.GetTypeFromCLSID(config.CLSID);
   object provider = Activator.CreateInstance(providerType);

   // Cast to BigDrive interface
   IBigDriveEnumerate enumerate = provider as IBigDriveEnumerate;
   ```

3. **Process isolation**: Providers run out-of-process in `dllhost.exe` (COM+ surrogate)

### Process Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│  BigDrive.Shell.exe                                                     │
│                                                                         │
│  ┌──────────────────┐    ┌──────────────────┐    ┌──────────────────┐   │
│  │DriveLetterManager│───▶│ CommandProcessor │───▶│ ProviderFactory  │   │
│  │                  │    │                  │    │                  │   │
│  │ Z: → Drive A     │    │ dir, cd, copy... │    │ CoCreateInstance │   │
│  │ Y: → Drive B     │    │                  │    │                  │   │
│  └──────────────────┘    └──────────────────┘    └────────┬─────────┘   │
│                                                           │             │
│  References:                                              │             │
│   - BigDrive.Interfaces (COM interfaces)                  │             │
│   - BigDrive.ConfigProvider (registry access)             │             │
│                                                           │             │
└───────────────────────────────────────────────────────────┼─────────────┘
                                                            │
                                          COM Activation (out-of-process)
                                                            │
                                                            ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  dllhost.exe (COM+ Surrogate)                                           │
│                                                                         │
│  ┌─────────────────────┐  ┌─────────────────────┐                       │
│  │ Provider.Flickr     │  │ Provider.Sample     │  ...                  │
│  │                     │  │                     │                       │
│  │ IBigDriveEnumerate  │  │ IBigDriveEnumerate  │                       │
│  │ IBigDriveFileData   │  │ IBigDriveFileData   │                       │
│  │ IBigDriveFileInfo   │  │ IBigDriveFileInfo   │                       │
│  └─────────────────────┘  └─────────────────────┘                       │
│                                                                         │
│  Identity: BigDriveTrustedInstaller                                     │
└─────────────────────────────────────────────────────────────────────────┘
```

### Dependencies

| Assembly | Purpose |
|----------|---------|
| `BigDrive.Interfaces` | COM interface definitions (IBigDriveEnumerate, etc.) |
| `BigDrive.ConfigProvider` | DriveManager, ProviderManager for registry access |

The shell does **NOT** reference provider assemblies (Provider.Flickr, Provider.Sample, etc.)

---

## OAuth Authentication

BigDrive.Shell includes generic OAuth support for authenticating with cloud providers.
The implementation follows patterns from Azure CLI, GitHub CLI, and other modern CLIs
(see [OAuth Authentication Patterns](../../research/OAuth%20Authentication%20Patterns%20in%20Windows%20CLI%20Applications.md)).

### Supported OAuth Flows

| Flow | Use Case |
|------|----------|
| **Authorization Code with Loopback** | Default. Opens browser, listens on localhost for redirect. Seamless UX. |
| **Device Code** | For headless/SSH. Displays URL and code. User authorizes on any device. |
| **OAuth 1.0a** | For legacy providers like Flickr that don't support OAuth 2.0. |

### Authentication Commands

| Command | Description |
|---------|-------------|
| `login` | Authenticate with the current drive's provider (browser flow) |
| `login --device-code` | Use device code flow (for SSH/headless) |
| `logout` | Clear cached OAuth tokens |
| `logout --all` | Clear all secrets including API keys |
| `authstatus` | Check current authentication status |

### Token Storage

Tokens are stored securely using **Windows Credential Manager** (per-user, encrypted).
This follows the same pattern as GitHub CLI and Azure CLI. Tokens are associated with
each drive's GUID, allowing multiple accounts per provider.

### Provider Authentication Interface

Providers can implement `IBigDriveAuthentication` to describe their OAuth requirements:

```csharp
public interface IBigDriveAuthentication
{
    int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo);
    int OnAuthenticationComplete(Guid driveGuid, string accessToken, string refreshToken, int expiresIn);
    int IsAuthenticated(Guid driveGuid, out bool isAuthenticated);
}
```

The shell handles the OAuth flow generically; the provider just describes its endpoints.

---

## See Also

- [User Guide](../../docs/BigDrive.Shell.UserGuide.md) — Complete command reference
- [Provider Development Guide](../../docs/ProviderDevelopmentGuide.md) — Creating new providers
- [BigDrive.Interfaces](../Interfaces/README.txt) — Interface definitions
- [BigDrive.ConfigProvider](../ConfigProvider/README.txt) — Registry configuration

---

*Copyright © Wayne Walter Berry. All rights reserved.*
