================================================================================
                        BigDrive.Interfaces Project
                         Architecture Documentation
================================================================================

OVERVIEW
--------------------------------------------------------------------------------
BigDrive.Interfaces defines the COM-visible interface contracts that enable
communication between the native C++ shell extension (BigDrive.ShellFolder)
and managed .NET provider implementations. This project serves as the
contract layer in BigDrive's cross-language, cross-process architecture.

ARCHITECTURAL ROLE
--------------------------------------------------------------------------------
BigDrive uses a shim architecture to integrate custom storage providers with
Windows Explorer:

    +-------------------+       COM+        +----------------------+
    |  explorer.exe     |  <=============>  |  Provider (C#/.NET)  |
    |  (BigDrive.       |   Out-of-Process  |  Implements these    |
    |   ShellFolder)    |   COM Interop     |  interfaces          |
    +-------------------+                   +----------------------+
            ^                                         |
            |                                         |
            +--------------- References ---------------+
                        BigDrive.Interfaces

Key Benefits:
  - Process Isolation: Providers run outside explorer.exe in dllhost.exe,
    preventing crashes from affecting the Windows shell. Provider assemblies
    must NEVER be loaded into the shell extension process.
  - Language Flexibility: Providers can be written in C# or any COM-compatible
    language.
  - Extensibility: Third-party developers implement these interfaces to create
    custom virtual drives.

Important: This project contains ONLY provider interfaces. Interfaces for
BigDrive.Service (IBigDriveProvision, IBigDriveConfiguration, IBigDriveSetup)
are in the separate BigDrive.Service.Interfaces project. BigDrive.Service is
NOT a provider — it manages providers and drives under the BigDriveInstaller
identity with elevated registry permissions. Providers run as Interactive User.

INTERFACE DEFINITIONS
--------------------------------------------------------------------------------

IBigDriveEnumerate (457ED786-889A-4C16-A6E5-6A25013D0AFA)
  Purpose: Enumerate folders and files within a virtual drive.
  Methods:
    - EnumerateFolders(driveGuid, path) -> string[]
    - EnumerateFiles(driveGuid, path) -> string[]

IBigDriveFileInfo (A98A0D26-4D5D-4B50-B6FF-8BCB360CB066)
  Purpose: Retrieve file metadata.
  Methods:
    - LastModifiedTime(driveGuid, path) -> DateTime
    - GetFileSize(driveGuid, path) -> ulong

IBigDriveFileOperations (7BE23F90-8D32-4D88-B4E7-59BFDA941F04)
  Purpose: Perform file and folder operations.
  Methods:
    - CopyFileToBigDrive(driveGuid, localFilePath, bigDriveTargetPath)
    - CopyFileFromBigDrive(driveGuid, bigDriveFilePath, localTargetPath)
    - DeleteFile(driveGuid, bigDriveFilePath)
    - CreateDirectory(driveGuid, bigDriveDirectoryPath)
    - OpenFile(driveGuid, bigDriveFilePath, hwndParent)
    - MoveFile(driveGuid, sourcePath, destinationPath)

IBigDriveFileData (0F471AE9-1787-437F-B230-60CA6717DD04)
  Purpose: Retrieve file content as a COM IStream.
  Methods:
    - GetFileData(driveGuid, path, out IStream stream) -> HRESULT

IBigDriveDriveInfo (3A2B1C4D-5E6F-7A8B-9C0D-1E2F3A4B5C6D)
  Purpose: Declare custom parameter requirements for mounting a drive.
  Methods:
    - GetDriveParameters() -> string (JSON array of name/description/type objects)

  Supporting Types:
    - DriveParameterDefinition (Model/DriveParameterDefinition.cs):
        Class representing a single parameter definition.
        Properties: Name, Description, Type (defaults to DriveParameterType.String).
    - DriveParameterType (Model/DriveParameterType.cs):
        Enum controlling how the Shell prompts for user input:
          String       -> "string"        Standard text input.
          ExistingFile -> "existing-file"  Tab completion, must exist.
          FilePath     -> "filepath"       Tab completion, may not exist.
          Secret       -> "secret"         Input masked with asterisks.
    - DriveParameterSerializer (Serialization/DriveParameterSerializer.cs):
        Zero-dependency JSON serializer. Providers construct an array of
        DriveParameterDefinition and call DriveParameterSerializer.Serialize()
        to produce the JSON string returned by GetDriveParameters().

  Notes:
    This interface is optional. Providers that do not require custom parameters
    do not need to implement it. The Shell queries this interface before creating
    a drive so it can prompt the user for the required values. The collected values
    are sent as Properties in the DriveConfiguration JSON passed to
    IBigDriveProvision.Mount(string jsonConfiguration).

    Each parameter object has these fields:
      - name:        The property key stored in DriveConfiguration.Properties.
      - description: User-facing text shown before the prompt.
      - type:        "string" | "existing-file" | "filepath" | "secret".

    Example JSON return value:
      [
        { "name": "ZipFilePath", "description": "Full path to the ZIP file.", "type": "existing-file" },
        { "name": "ApiSecret", "description": "Your API secret.", "type": "secret" }
      ]

IBigDriveAuthentication (7E8F9A0B-1C2D-3E4F-5A6B-7C8D9E0F1A2B)
  Purpose: Describe OAuth authentication requirements for BigDrive Shell.
  Methods:
    - GetAuthenticationInfo(driveGuid, out AuthenticationInfo) -> HRESULT
        Returns OAuth endpoints, client ID, flow type, etc.
    - OnAuthenticationComplete(driveGuid, accessToken, refreshToken, expiresIn) -> HRESULT
        Called after successful OAuth to notify the provider.
    - IsAuthenticated(driveGuid, out bool) -> HRESULT
        Check if valid tokens are available.

  Supporting Types:
    - AuthenticationInfo: Struct containing OAuth configuration
    - OAuthFlowType: Enum (AuthorizationCode, DeviceCode, OAuth1, AuthorizationCodePKCE)

  Notes:
    This interface is optional. Providers that don't require OAuth don't need
    to implement it. When implemented, BigDrive Shell can perform generic OAuth
    flows and store tokens in Windows Credential Manager.

  Note on BigDrive.Service Interfaces:
    Interfaces exposed by BigDrive.Service (IBigDriveProvision, IBigDriveConfiguration,
    IBigDriveSetup) are defined in the separate BigDrive.Service.Interfaces project.
    See src/BigDrive.Service.Interfaces/README.md for details.

EXCEPTION TYPES
--------------------------------------------------------------------------------

BigDriveAuthenticationRequiredException
  Purpose: Thrown by providers when an operation fails due to missing/invalid auth.

  Properties:
    - DriveGuid: The drive GUID that requires authentication
    - ProviderName: Friendly name (e.g., "Flickr", "OneDrive")
    - Reason: AuthenticationFailureReason enum value

  AuthenticationFailureReason values:
    - Unknown (0): Unspecified reason
    - NotAuthenticated (1): No credentials present
    - TokenExpired (2): OAuth token has expired
    - TokenRevoked (3): OAuth token was revoked
    - InvalidToken (4): Token or credentials are invalid
    - InsufficientPermissions (5): User lacks required permissions
    - InvalidSignature (6): OAuth signature verification failed (OAuth 1.0a)
    - ApiKeyMissing (7): API key or client credentials missing

  Usage:
    Providers catch service-specific exceptions and throw this generic exception.
    BigDrive.Shell catches it and automatically prompts for login:

    ```csharp
    catch (FlickrNet.OAuthException ex)
    {
        throw new BigDriveAuthenticationRequiredException(
            driveGuid, "Flickr", AuthenticationFailureReason.InvalidToken, ex);
    }
    ```

COM INTEROP DESIGN
--------------------------------------------------------------------------------
All interfaces follow these patterns for cross-language compatibility:

  1. [ComVisible(true)] - Exposes the interface to COM clients.
  2. [Guid("...")] - Unique IID for each interface.
  3. [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)] - Pure IUnknown-based
     interface for maximum compatibility with C++ clients.
  4. Drive Identification: All methods accept a driveGuid parameter to identify
     which registered drive the operation targets.

PATH FORMAT CONVENTIONS
--------------------------------------------------------------------------------
All BigDrive path parameters follow these conventions. Providers MUST handle
paths in this format:

  Format:
    - Paths use backslash (\) as the separator
    - Paths are absolute within the drive (start with \)
    - Root path is represented as "\" or "\\"
    - Example: "\FolderName\SubFolder\FileName.txt"

  Path Components:
    - Root:     "\"
    - Folder:   "\FolderName" or "\Parent\Child"
    - File:     "\FolderName\File.txt" or "\File.txt" (file at root)

  Provider Implementation Requirements:
    1. ALWAYS handle leading backslash - paths like "\File.txt" are valid
    2. Use path.Trim('\\').Split(new[] { '\\' }, ...) to parse path segments
    3. EnumerateFolders/EnumerateFiles return NAMES only, not full paths
    4. File operations receive full paths including the filename
    5. Case sensitivity is provider-specific (document your behavior)

  Example Path Resolution:
    Shell Command: copy "A File.txt" c:\temp\

    Shell resolves "A File.txt" (relative) to "\A File.txt" (absolute)
    Provider receives: bigDriveFilePath = "\A File.txt"
    Provider parses:   path segments = ["A File.txt"]
                       directory = "\" (root)
                       filename = "A File.txt"

  Common Implementation Pattern:
    ```csharp
    private Node FindFileByPath(string path)
    {
        var segments = path.Trim('\\').Split(
            new[] { '\\' }, StringSplitOptions.RemoveEmptyEntries);

        if (segments.Length == 0) return null;

        string fileName = segments[segments.Length - 1];
        Node folder = root;

        // Navigate to parent folder
        for (int i = 0; i < segments.Length - 1; i++)
        {
            folder = folder.Children.FirstOrDefault(
                c => c.Name == segments[i] && c.IsFolder);
            if (folder == null) return null;
        }

        // Find file in folder
        return folder.Children.FirstOrDefault(
            c => c.Name == fileName && c.IsFile);
    }
    ```

NATIVE C++ COUNTERPARTS
--------------------------------------------------------------------------------
The BigDrive.Client project contains C++ header equivalents of these interfaces
for use by the shell extension:

  - IBigDriveEnumerate.h
  - IBigDriveFileInfo.h
  - IBigDriveFileOperations.h
  - IBigDriveFileData.h
  - IBigDriveConfiguration.h

These headers define identical IIDs and method signatures, enabling seamless
QueryInterface calls across the COM boundary.

IMPLEMENTING A PROVIDER
--------------------------------------------------------------------------------
To create a custom BigDrive provider:

  1. Reference BigDrive.Interfaces in your .NET project.
  2. Create a class that inherits from ServicedComponent (for COM+ hosting).
  3. Implement the required interfaces:
       - IBigDriveEnumerate (required)
       - IBigDriveFileInfo (required)
       - IBigDriveFileOperations (optional)
       - IBigDriveFileData (optional)
       - IBigDriveAuthentication (optional - for OAuth-enabled providers)
       - IBigDriveDriveInfo (optional - for providers requiring custom drive parameters)
  4. Register the COM+ application with the BigDrive service.

Example: See BigDrive.Provider.Sample for a complete implementation.
Example: See BigDrive.Provider.Flickr for OAuth 1.0a authentication.

DEPENDENCY CONSTRAINTS
--------------------------------------------------------------------------------
BigDrive.Interfaces must have ZERO external package dependencies. It references
only System and System.Core from the .NET Framework.

Why: This assembly is loaded very early during COM+ activation (by regsvcs.exe
and dllhost.exe), before any provider's AssemblyResolver has been registered.
If Interfaces depends on a NuGet package (e.g., System.Text.Json), the CLR
will attempt to load that assembly before the provider can resolve it, causing
a catastrophic failure (0x8000FFFF) during COM+ activation.

This is why DriveParameterSerializer uses StringBuilder instead of
System.Text.Json — it produces identical JSON output with no external
dependencies.

When adding new types to this project:
  - Use ONLY types from System and System.Core.
  - Do NOT add NuGet PackageReferences or assembly references to external DLLs.
  - If serialization is needed, use the zero-dependency pattern in
    Serialization/DriveParameterSerializer.cs.

TARGET FRAMEWORK
--------------------------------------------------------------------------------
.NET Framework 4.7.2

This framework version is required for System.EnterpriseServices (COM+) support
and COM interop compatibility with the native shell extension.

================================================================================
                    Copyright (c) Wayne Walter Berry. All rights reserved.
================================================================================
