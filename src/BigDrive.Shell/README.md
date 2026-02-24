# BigDrive.Shell

A custom shell for interacting with BigDrive providers. Following Scott Hanselman's taxonomy:

- **Terminal**: Windows Terminal, conhost, or any console host (dumb I/O)
- **Outer Shell**: PowerShell, cmd.exe (launches BigDrive.Shell)
- **BigDrive.Shell**: The command interpreter for BigDrive operations

## Usage

Run `BigDrive.Shell.exe` from PowerShell or cmd:

```powershell
PS> .\BigDrive.Shell.exe
BigDrive Shell v1.0
Type 'help' for available commands, 'exit' to quit.

BD:\>
```

## Commands

| Command | Aliases | Description |
|---------|---------|-------------|
| `help` | `?` | Displays help information |
| `exit` | `quit`, `q` | Exits the shell |
| `drives` | `list` | Lists registered BigDrive drives |
| `select` | `use`, `mount` | Selects a drive to work with |
| `dir` | `ls` | Lists files and folders |
| `cd` | `chdir` | Changes the current directory |
| `copy` | `cp` | Copies files to/from BigDrive |
| `mkdir` | `md` | Creates a new directory |
| `del` | `rm`, `delete`, `erase` | Deletes a file or directory |

## Example Session

```
BD:\> drives
Registered BigDrive drives:

  [1] Flickr Photos
      GUID: {12345678-1234-1234-1234-123456789ABC}
      Provider: {ABCDEFAB-ABCD-ABCD-ABCD-ABCDEFABCDEF}

Use 'select <number>' or 'select <name>' to select a drive.

BD:\> select 1
Selected drive: Flickr Photos

BD:Flickr Photos\> dir

 Directory of BD:Flickr Photos\

    <DIR>    Vacation 2024
    <DIR>    Family Photos
    <DIR>    Nature

       3 Dir(s)    0 File(s)

BD:Flickr Photos\> cd "Vacation 2024"
BD:Flickr Photos\Vacation 2024> dir

 Directory of BD:Flickr Photos\Vacation 2024

             Beach.jpg
             Sunset.jpg
             Mountains.jpg

       0 Dir(s)    3 File(s)

BD:Flickr Photos\Vacation 2024> copy Beach.jpg C:\Downloads\Beach.jpg
        1 file(s) copied.

BD:Flickr Photos\Vacation 2024> exit
Goodbye!
```

## Architecture

The shell implements the command pattern with:

- `ShellContext` - Maintains current drive, path, and session state
- `CommandProcessor` - Parses input and dispatches to command handlers
- `ICommand` - Interface for all shell commands
- `ProviderFactory` - Creates COM+ provider instances via COM activation

### COM+ Provider Activation

The shell **never directly references provider assemblies**. Instead:

1. **Drive enumeration** uses `BigDrive.ConfigProvider.DriveManager` to read drive
   configurations from the registry (`HKLM\SOFTWARE\BigDrive\Drives`)

2. **Provider activation** uses COM interop to instantiate providers hosted in COM+:
   ```csharp
   // Get CLSID from drive configuration (registry)
   DriveConfiguration config = DriveManager.ReadConfiguration(driveGuid, cancellationToken);

   // Activate provider via COM (runs in dllhost.exe under COM+)
   Type providerType = Type.GetTypeFromCLSID(config.CLSID);
   object provider = Activator.CreateInstance(providerType);

   // Cast to BigDrive interface
   IBigDriveEnumerate enumerate = provider as IBigDriveEnumerate;
   ```

3. **Process isolation**: Providers run out-of-process in `dllhost.exe` (COM+ surrogate),
   not in the shell's process space.

```
┌─────────────────────────────────────────────────────────────────────────┐
│  BigDrive.Shell.exe                                                     │
│                                                                         │
│  ┌─────────────────┐    ┌──────────────────┐                            │
│  │ CommandProcessor│───▶│ ProviderFactory  │                            │
│  └─────────────────┘    └────────┬─────────┘                            │
│                                  │                                      │
│                     CoCreateInstance(CLSID)                             │
│                                  │                                      │
└──────────────────────────────────┼──────────────────────────────────────┘
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

### Dependencies

- `BigDrive.Interfaces` - COM interface definitions (IBigDriveEnumerate, etc.)
- `BigDrive.ConfigProvider` - DriveManager for reading drive configurations from registry

The shell does NOT reference provider assemblies directly (Provider.Flickr, Provider.Sample, etc.)
