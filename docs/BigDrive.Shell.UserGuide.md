# BigDrive Shell User Guide

## Overview

BigDrive Shell is an interactive command-line shell for navigating and managing BigDrive virtual file systems. It allows you to browse cloud storage providers (like Flickr, Azure Blob, etc.) as if they were local drives, using familiar commands like `dir`, `cd`, and `copy`.

### Key Features

- **Drive Letters**: Each BigDrive is assigned a drive letter (Z:, Y:, X:...) avoiding conflict with OS drives
- **Multi-Drive Access**: All BigDrives are available simultaneously
- **Cross-Drive Copy**: Copy files between BigDrives or between BigDrive and local drives
- **Familiar Commands**: Uses Windows-style commands (`dir`, `cd`, `copy`)

### What is BigDrive Shell?

Following [Scott Hanselman's taxonomy](https://www.hanselman.com/blog/whats-the-difference-between-a-console-a-terminal-and-a-shell):

| Component | Role |
|-----------|------|
| **Terminal** | Windows Terminal, conhost (dumb I/O surface) |
| **Outer Shell** | PowerShell, cmd.exe (launches BigDrive.Shell) |
| **BigDrive.Shell** | Custom command interpreter for BigDrive operations |

BigDrive Shell is a **shell** — a smart command interpreter that understands BigDrive providers, paths, and file operations.

---

## Getting Started

### Prerequisites

1. **BigDrive must be installed** — Run `BigDrive.Setup.exe` as Administrator
2. **At least one drive must be registered** — Drives are configured in the Windows Registry
3. **Providers must be registered with COM+** — Handled automatically by setup

### Launching the Shell

Open PowerShell or Command Prompt and run:

```powershell
PS C:\Program Files\BigDrive> .\BigDrive.Shell.exe

BigDrive Shell v1.0
Type 'help' for available commands, 'exit' to quit.

BD>
```

The `BD>` prompt indicates you're in the BigDrive Shell but no drive is selected yet.

---

## Commands Reference

### General Commands

| Command | Aliases | Description |
|---------|---------|-------------|
| `help` | `?` | Display help for all commands or a specific command |
| `exit` | `quit`, `q` | Exit the BigDrive Shell |

### Drive Management

| Command | Aliases | Description |
|---------|---------|-------------|
| `drives` | `list` | List all drives with their assigned letters |
| `mount` | `register`, `add` | Mount a new BigDrive (like `net use`) |
| `unmount` | `unregister`, `remove`, `umount` | Unmount a BigDrive |

### File System Navigation

| Command | Aliases | Description |
|---------|---------|-------------|
| `cd` | `chdir` | Change drive or directory (`cd X:` or `cd folder`) |
| `dir` | `ls` | List drives (at root) or folder contents (in drive) |

### File Operations

| Command | Aliases | Description |
|---------|---------|-------------|
| `copy` | `cp` | Copy files between any drives |
| `mkdir` | `md` | Create a new directory |
| `del` | `rm`, `delete`, `erase` | Delete a file or directory |

### Authentication

| Command | Aliases | Description |
|---------|---------|-------------|
| `login` | `auth`, `authenticate` | Authenticate with the current drive's provider (OAuth) |
| `logout` | `signout` | Clear cached authentication tokens |
| `authstatus` | `whoami`, `status` | Show authentication status for current drive |

### Credential Management

| Command | Aliases | Description |
|---------|---------|-------------|
| `secret set` | — | Store a secret in Windows Credential Manager |
| `secret list` | — | List stored secrets for current drive |
| `secret exists` | — | Check if a secret exists |
| `secret del` | — | Delete a stored secret |

### Provider Information

| Command | Aliases | Description |
|---------|---------|-------------|
| `providers` | — | List registered BigDrive providers |

---

## Walkthrough: First-Time Usage

### Step 1: Get Help

```
BD> help
Available commands:

  help        Displays help information for commands
  exit        Exits the BigDrive Shell
  drives      Lists all drives (BigDrive and local)
  dir         Lists drives or folder contents
  cd          Changes the current directory or drive
  copy        Copies files between drives
  mkdir       Creates a new directory
  del         Deletes a file or directory

Type 'help <command>' for more information.
```

Get help for a specific command:

```
BD> help copy
copy - Copies files between drives
Usage: copy <source> <destination>  |  copy X:\file Y:\file
Aliases: cp
```

### Step 2: List Available Drives

At the root level, `dir` (or `ls`) shows available BigDrive drives:

```
BD> dir

 Directory of BigDrive

    <DRIVE>  Z:  Flickr Photos
    <DRIVE>  Y:  Sample Drive

       2 Drive(s)

Use 'cd X:' to enter a drive.
```

Drive letters are assigned automatically, starting from Z: and working backwards to avoid conflicts with OS drives.

### Step 3: Switch to a Drive

You can switch drives by typing the drive letter with a colon, just like in Windows cmd:

```
BD> Z:

Z:\>
```

Or use the `cd` command:

```
BD> cd Z:

Z:\>
```

The prompt changes to show the current drive letter and path.

### Step 4: Browse the Drive

List the root directory:

```
Z:\> dir

 Directory of Z:\

    <DIR>    Vacation 2024
    <DIR>    Family Reunion
    <DIR>    Nature Photography
    <DIR>    City Skylines

       4 Dir(s)    0 File(s)
```

### Step 5: Navigate Directories

Change to a subdirectory:

```
Z:\> cd "Vacation 2024"

Z:\Vacation 2024>
```

List contents:

```
Z:\Vacation 2024> dir

 Directory of Z:\Vacation 2024

             Beach Sunset.jpg
             Mountain View.jpg
             Hotel Pool.jpg
             Local Market.jpg
             Dinner with Friends.jpg

       0 Dir(s)    5 File(s)
```

### Step 6: Copy Files to Local Disk

```
Z:\Vacation 2024> copy "Beach Sunset.jpg" C:\Users\Wayne\Downloads\beach.jpg
        1 file(s) copied.
```

### Step 7: Navigate Back

Go up one level:

```
Z:\Vacation 2024> cd ..

Z:\>
```

Go to root:

```
Z:\Vacation 2024> cd \

Z:\>
```

### Step 8: Copy Between BigDrives

Switch to another BigDrive and copy a file from the first:

```
Z:\Vacation 2024> cd Y:

Y:\>

Y:\> copy Z:\Vacation 2024\Beach Sunset.jpg "Beach Backup.jpg"
        1 file(s) copied.
```

The shell remembers your position on each drive:

```
Y:\> cd Z:

Z:\Vacation 2024>
```

### Step 9: View All Drives

```
Z:\Vacation 2024> drives
BigDrive drives:

 *Z:  Flickr Photos
       Current: Z:\Vacation 2024
  Y:  Sample Drive

Local drives:
  C:, D:, E:

Use 'cd X:' to switch to a drive.
```

The `*` indicates the current drive.

### Step 10: Exit

```
Z:\Vacation 2024> exit
Goodbye!

PS C:\Program Files\BigDrive>
```

---

## Path Syntax

### Drive Letters

Use standard Windows drive letter syntax:

```
Z:\Vacation 2024> copy "photo.jpg" Y:\Backups\photo.jpg
```

### Absolute Paths

Start with `\` to specify a path from the drive root:

```
Z:\Vacation 2024> cd \Nature Photography

Z:\Nature Photography>
```

### Relative Paths

Paths without a drive letter or `\` are relative to the current directory:

```
Z:\> cd "Vacation 2024"

Z:\Vacation 2024>
```

### Special Paths

| Path | Description |
|------|-------------|
| `X:` | Switch to drive X |
| `X:\path` | Absolute path on drive X |
| `\` | Root of the current drive |
| `..` | Parent directory |

### Paths with Spaces

Use quotes for paths containing spaces:

```
Z:\> cd "Family Reunion"
```

---

## File Operations

### Copying Files

**From BigDrive to local:**

```
Z:\Vacation 2024> copy "Beach Sunset.jpg" C:\Downloads\beach.jpg
        1 file(s) copied.
```

**From local to BigDrive:**

```
Z:\Vacation 2024> copy C:\Pictures\new_photo.jpg "New Photo.jpg"
        1 file(s) copied.
```

**Between BigDrives:**

```
Z:\Vacation 2024> copy "Beach Sunset.jpg" Y:\Backups\beach.jpg
        1 file(s) copied.
```

> **Note:** Not all providers support uploading. Read-only providers will display an error.

### Creating Directories

```
Y:\> mkdir "New Folder"
Directory created: New Folder
```

### Deleting Files

```
Y:\> del "Old File.txt"
Deleted: Old File.txt
```

---

## Mounting and Unmounting Drives

### Mounting a New Drive (Interactive)

Run `mount` without arguments for interactive mode:

```
BD> mount

Available providers:

  [1] Flickr Provider
      CLSID: {B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}

  [2] Sample Provider
      CLSID: {F8FE2E5A-E8B8-4207-BC04-EA4BCD4C4361}

Select provider number: 1
Enter drive name: My Flickr Photos

Drive mounted successfully!

  Name:     My Flickr Photos
  GUID:     {A1B2C3D4-5E6F-7890-ABCD-EF1234567890}
  Provider: Flickr Provider

Use 'cd Z:' to access the new drive.
```

### Mounting a Drive (Command Line)

```
BD> mount 1 "My Flickr Photos"

Drive mounted successfully!

  Name:     My Flickr Photos
  GUID:     {A1B2C3D4-5E6F-7890-ABCD-EF1234567890}
  Provider: Flickr Provider
```

Or use the provider name:

```
BD> mount "Flickr Provider" "My Flickr Photos"
```

### Unmounting a Drive

```
Z:\> unmount Z
Unmount 'My Flickr Photos' (Z:)? [y/N]: y
Drive unmounted: My Flickr Photos
```

The unmount command calls BigDrive.Service via COM+ (`IBigDriveProvision.UnmountDrive`)
to remove the drive configuration, unregister the shell folder, and refresh Explorer.
The Shell does not require Administrator privileges because BigDrive.Service runs
under the **BigDriveInstaller** identity, which has write access to HKCR and the
HKLM shell namespace.

> **Note:** Mounting and unmounting requires BigDrive.Service to be registered with COM+
> (done automatically by `BigDrive.Setup.exe` or the Service's PostBuild `regsvcs.exe` step).

---

## Authentication

Many BigDrive providers require authentication with their cloud services. BigDrive Shell
supports OAuth authentication through the `login`, `logout`, and `authstatus` commands.

### Checking Authentication Status

Before using a drive, check if it's authenticated:

```
Z:\> authstatus

Authentication Status
=====================

  Drive:      Flickr Photos
  Drive ID:   a2b3c4d5-e6f7-8901-a2b3-c4d5e6f78901
  Provider:   Flickr Provider

  Status:     Not authenticated

  Use 'login' to authenticate with this provider.
```

### Authenticating with a Provider

Use `login` to authenticate with the current drive's provider:

```
Z:\> login
Flickr Authentication
=====================

Requesting temporary token from Flickr...

Opening your web browser for Flickr authorization...

If the browser does not open automatically, navigate to:
  https://www.flickr.com/services/oauth/authorize?oauth_token=...

After authorizing, Flickr will display a verification code.
Enter the verification code: 123-456-789

Exchanging for access token...
Authentication successful!

Flickr authentication complete!
Tokens saved to Windows Credential Manager.
```

### Device Code Flow (Headless/SSH)

For environments without a browser (e.g., SSH sessions), use the device code flow:

```
Z:\> login --device-code

To sign in to Flickr:

  1. Open a web browser and go to: https://flickr.com/device
  2. Enter the code: ABC-123

  (Code has been copied to your clipboard)

Waiting for you to authorize... Press Ctrl+C to cancel.
....
Authentication successful!
```

### Logging Out

Clear cached tokens with `logout`:

```
Z:\> logout
Clearing authentication tokens...

  Cleared: FlickrOAuthToken
  Cleared: FlickrOAuthSecret

Cleared 2 token(s).

You have been logged out of this drive.
Use 'login' to authenticate again.

Note: Clearing local tokens does not revoke access at the provider.
To fully revoke access, visit your account settings at the provider:

  Flickr:    https://www.flickr.com/services/auth/list.gne
```

### Clearing All Secrets

To remove all credentials (including API keys):

```
Z:\> logout --all
Clearing all secrets for this drive...

  Cleared: FlickrApiKey
  Cleared: FlickrApiSecret
  Cleared: FlickrOAuthToken
  Cleared: FlickrOAuthSecret

Cleared 4 secret(s).
```

### Where Are Credentials Stored?

Credentials are stored securely in **Windows Credential Manager**:
- Per-user (only accessible by your Windows account)
- Encrypted by Windows DPAPI
- Separate credentials per drive (multiple accounts supported)

View stored credentials with `secret list`:

```
Z:\> secret list

 Secrets for current drive:

    FlickrApiKey
    FlickrApiSecret
    FlickrOAuthToken
    FlickrOAuthSecret

    4 secret(s) configured.
```

---

## Error Messages

### No Drive Selected

```
BD> dir
```

At the root, `dir` shows available drives (not an error).

### No Drives Registered

```
BD> dir

 Directory of BigDrive

    No BigDrive drives registered.

Run BigDrive.Setup.exe to register providers.
```

**Solution:** Use `mount` to create a drive, or run `BigDrive.Setup.exe`.

### Drive Not Found

```
BD> cd Q:
Drive not found: Q:
Use 'drives' to see available drives.
```

**Solution:** Check available drive letters with `drives` command.

### Cannot Navigate to OS Drives

```
Z:\> cd C:
Cannot navigate to OS drives from BigDrive Shell.
Use 'copy' command to transfer files to/from local drives.
```

**Solution:** BigDrive Shell only navigates BigDrives. Use `copy` to transfer files.

### Path Not Found

```
Z:\> cd "Nonexistent Folder"
The system cannot find the path specified: Nonexistent Folder
```

**Solution:** Use `dir` to see available folders.

### Provider Doesn't Support Operation

```
Z:\Vacation 2024> mkdir "New Album"
Provider does not support file operations.
```

**Solution:** This provider is read-only. Use a different provider or the provider's native interface.

### Source File Not Found

```
Z:\> copy C:\NonExistent\file.txt "destination.txt"
Source file not found: C:\NonExistent\file.txt
```

**Solution:** Verify the local file path.

---

## Architecture

### Drive Letter Assignment

BigDrive Shell automatically assigns drive letters to registered BigDrives:

- Letters are assigned starting from **Z:** and working backwards
- OS drive letters (C:, D:, etc.) are never used
- Letters persist for the shell session
- Use `drives` to see current assignments

### How the Shell Works

```
┌─────────────────────────────────────────────────────────────────────────┐
│  BigDrive.Shell.exe                                                     │
│                                                                         │
│  ┌──────────────────┐    ┌──────────────────┐    ┌──────────────────┐   │
│  │DriveLetterManager│───▶│ CommandProcessor │───▶│ ProviderFactory  │   │
│  └──────────────────┘    └──────────────────┘    └────────┬─────────┘   │
│                                                           │             │
│    Z: → Flickr Photos                        CoCreateInstance(CLSID)    │
│    Y: → Sample Drive                                      │             │
│                                                           │             │
└───────────────────────────────────────────────────────────┼─────────────┘
                                                            │ COM Activation
                                                            ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  dllhost.exe (COM+ Surrogate)                                           │
│                                                                         │
│  ┌─────────────────────┐  ┌─────────────────────┐                       │
│  │ Provider.Flickr     │  │ Provider.Sample     │  ...                  │
│  │ (ServicedComponent) │  │ (ServicedComponent) │                       │
│  └─────────────────────┘  └─────────────────────┘                       │
│                                                                         │
│  Identity: BigDriveTrustedInstaller                                     │
└─────────────────────────────────────────────────────────────────────────┘
```

### Key Points

1. **Multiple drives available simultaneously** — All BigDrives are accessible via their letters
2. **Cross-drive operations** — Copy between BigDrives or between BigDrive and local
3. **Path memory** — Shell remembers your position on each drive
4. **COM+ activation** — Providers are instantiated via COM, not direct references
5. **Process isolation** — If a provider crashes, it doesn't crash the shell

---

## Supported Providers

### Flickr Provider (Z:)

Maps your Flickr photosets as folders and photos as files.

| Feature | Support |
|---------|---------|
| List folders (photosets) | ✅ |
| List files (photos) | ✅ |
| Download photos | ✅ |
| Upload photos | ❌ (read-only) |
| Create folders | ❌ |
| Delete photos | ❌ |

### Sample Provider (Y:)

A demonstration provider for testing purposes.

| Feature | Support |
|---------|---------|
| List folders | ✅ |
| List files | ✅ |
| Download files | ✅ |
| Upload files | ✅ |
| Create folders | ✅ |
| Delete files | ✅ |

---

## Troubleshooting

### Shell Won't Start

1. Ensure BigDrive.Setup.exe was run as Administrator
2. Check Event Viewer for errors under "BigDrive.Service"
3. Verify COM+ application is running in Component Services

### Provider Activation Fails

```
Error: Provider type not found for CLSID: {GUID}
```

1. Verify the provider DLL is registered: `regsvcs.exe Provider.dll`
2. Check Component Services for the BigDrive.Service application
3. Restart the COM+ application

### Slow Performance

1. Provider may be making network calls — check your connection
2. Large directories may take time to enumerate
3. Check provider logs in Event Viewer

### Commands Not Recognized

```
Unknown command: xyz
Type 'help' for available commands.
```

Use `help` to see valid commands. Commands are case-insensitive.

---

## Tips and Tricks

### Working with Multiple Drives

The shell remembers your current path on each drive:

```
Z:\Vacation 2024> cd Y:
Y:\> cd "My Folder"
Y:\My Folder> cd Z:
Z:\Vacation 2024>         ← Returned to previous location
```

### Cross-Drive Copy

Copy directly between BigDrives without switching:

```
Z:\> copy "photo.jpg" Y:\Backups\photo.jpg
```

### Quotes

Always use quotes for paths with spaces:

```
BD> cd "My Folder"      ✅ Correct
BD> cd My Folder        ❌ Incorrect (interprets as two arguments)
```

---

## Quick Reference Card

```
┌────────────────────────────────────────────────────────────────────┐
│                     BigDrive Shell Quick Reference                 │
├────────────────────────────────────────────────────────────────────┤
│  AT ROOT (BD>)                                                     │
│    dir / ls            List available BigDrive drives              │
│    cd X:               Enter BigDrive X                            │
│    mount               Mount a new drive (interactive)             │
│    mount <n> <name>    Mount a new drive (command line)            │
│    unmount X           Unmount drive X                             │
├────────────────────────────────────────────────────────────────────┤
│  INSIDE A DRIVE (X:\>)                                             │
│    dir [path]          List folder contents                        │
│    cd <path>           Change directory                            │
│    cd ..               Go up one level                             │
│    cd \                Go to drive root                            │
│    cd Y:               Switch to another drive                     │
├────────────────────────────────────────────────────────────────────┤
│  FILE OPERATIONS                                                   │
│    copy <src> <dst>    Copy between any drives                     │
│    mkdir <name>        Create directory                            │
│    del <name>          Delete file or directory                    │
├────────────────────────────────────────────────────────────────────┤
│  GENERAL                                                           │
│    help [cmd]          Show help                                   │
│    drives              List drives with local drive info           │
│    exit                Exit shell                                  │
├────────────────────────────────────────────────────────────────────┤
│  EXAMPLES                                                          │
│    dir                           List drives (at root)             │
│    mount                         Mount a new drive                 │
│    cd Z:                         Enter Flickr drive                │
│    dir                           List folders (in drive)           │
│    cd "My Photos"                Enter folder                      │
│    copy photo.jpg C:\Downloads\  Copy to local                     │
│    copy photo.jpg Y:\Backups\    Copy to another BigDrive          │
│    unmount Y                     Unmount drive Y                   │
│    exit                          Exit shell                        │
└────────────────────────────────────────────────────────────────────┘
```

---

## See Also

- [BigDrive.Setup README](../src/BigDrive.Setup/README.txt) — Installation and COM+ registration
- [BigDrive.Shell README](../src/BigDrive.Shell/README.md) — Shell architecture details
- [BigDrive.Provider.Flickr README](../src/BigDrive.Provider.Flickr/README.txt) — Flickr provider details
- [BigDrive.Interfaces README](../src/Interfaces/README.txt) — Provider interface definitions
- [Provider Development Guide](ProviderDevelopmentGuide.md) — Creating new providers

---

*Copyright © Wayne Walter Berry. All rights reserved.*
