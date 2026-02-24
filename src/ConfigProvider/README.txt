================================================================================
                      BigDrive.ConfigProvider Project
                         Architecture Documentation
================================================================================

OVERVIEW
--------------------------------------------------------------------------------
BigDrive.ConfigProvider is the configuration management layer for the BigDrive
system. It handles the registration, storage, and retrieval of provider and
drive configurations using the Windows Registry as the persistent store.

This project serves as the bridge between BigDrive providers (COM+ components)
and the native C++ shell extension, enabling discovery and configuration of
virtual drives.

ARCHITECTURAL ROLE
--------------------------------------------------------------------------------
The ConfigProvider sits at the center of BigDrive's configuration architecture:

    +-------------------------+     +-------------------------+
    |  BigDrive.ShellFolder   |     |  BigDrive Providers     |
    |  (C++ Shell Extension)  |     |  (COM+ Components)      |
    +-------------------------+     +-------------------------+
              |                               |
              |  Read Config                  |  Write Config
              v                               v
    +-------------------------------------------------------+
    |              BigDrive.ConfigProvider                  |
    |  - DriveManager (Drive CRUD operations)               |
    |  - ProviderManager (Provider registration)            |
    +-------------------------------------------------------+
                            |
                            v
    +-------------------------------------------------------+
    |              Windows Registry                          |
    |  HKLM\SOFTWARE\BigDrive\Drives\{GUID}                 |
    |  HKLM\SOFTWARE\BigDrive\Providers\{GUID}              |
    +-------------------------------------------------------+

REGISTRY STRUCTURE
--------------------------------------------------------------------------------
BigDrive stores all configuration in the Windows Registry under:

  HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\

Provider Registration:
  SOFTWARE\BigDrive\Providers\{PROVIDER-CLSID}\
    Id      (REG_SZ) - Provider GUID (same as CLSID)
    Name    (REG_SZ) - Display name of the provider

Drive Configuration:
  SOFTWARE\BigDrive\Drives\{DRIVE-GUID}\
    id      (REG_SZ) - Unique drive identifier
    name    (REG_SZ) - Display name shown in Explorer
    clsid   (REG_SZ) - CLSID of the provider COM+ component

Example:
  SOFTWARE\BigDrive\Drives\{6369DDE1-9A63-4E3B-B3C0-62A8082ED32E}\
    id    = "{6369DDE1-9A63-4E3B-B3C0-62A8082ED32E}"
    name  = "Sample Drive"
    clsid = "{F8FE2E5A-E8B8-4207-BC04-EA4BCD4C4361}"
    [non-sensitive properties only]

  Windows Credential Manager (per-user, encrypted):
    BigDrive:{6369DDE1-...}:FlickrApiKey = "my-api-key"
    BigDrive:{6369DDE1-...}:FlickrOAuthToken = "oauth-token"

PROJECT STRUCTURE
--------------------------------------------------------------------------------

Model/
  DriveConfiguration.cs
    - Represents a drive's configuration
    - Properties: Id (Guid), Name (string), CLSID (Guid), Properties (Dictionary)
    - Uses [JsonPropertyName] for registry/JSON key mapping
    - Properties dictionary stores non-sensitive provider-specific settings

  ProviderConfiguration.cs
    - Represents a provider's configuration
    - Properties: Id (Guid), Name (string)
    - Uses [JsonPropertyName] for registry/JSON key mapping

CredentialManager.cs
  Static class for secure credential storage using Windows Credential Manager.
  NOTE: External callers should use DriveManager APIs instead of calling
  CredentialManager directly.

  Internal Methods:
  - WriteSecret(Guid driveGuid, string key, string value)
      Stores a secret in the user's Credential Manager.

  - ReadSecret(Guid driveGuid, string key) -> string
      Reads a secret from the user's Credential Manager.

  - DeleteSecret(Guid driveGuid, string key) -> bool
      Deletes a secret from the Credential Manager.

  - DeleteAllSecretsForDrive(Guid driveGuid)
      Removes all secrets for a drive (call on unmount).

  - SecretExists(Guid driveGuid, string key) -> bool
      Checks if a secret exists.

  - GetSecretNames(Guid driveGuid) -> List<string>
      Enumerates all secret key names for a drive.

DriveManager.cs
  Static class for drive configuration operations:

  Registry Operations:
  - DriveExists(Guid, CancellationToken) -> bool
      Checks if a drive configuration exists in the registry.

  - ReadConfigurations(CancellationToken) -> IEnumerable<DriveConfiguration>
      Enumerates all registered drives (including custom properties).

  - ReadConfiguration(Guid, CancellationToken) -> DriveConfiguration
      Reads a specific drive's configuration (including custom properties).

  - WriteConfiguration(DriveConfiguration, CancellationToken)
      Creates or updates a drive configuration (including custom properties).

  - DeleteConfiguration(Guid, CancellationToken)
      Removes a drive configuration from the registry.

  - ReadDriveProperty(Guid, string, CancellationToken) -> string
      Reads a single property value from a drive's configuration.
      Used by providers to get non-sensitive drive-specific settings.

  - WriteDriveProperty(Guid, string, string, CancellationToken)
      Writes a single property value to a drive's configuration.

  Secret Operations (Credential Manager):
  - ReadSecretProperty(Guid, string, CancellationToken) -> string
      Reads a secret from Credential Manager for a drive.
      Use for API keys, OAuth tokens, passwords.
      Returns null if not found.

  - WriteSecretProperty(Guid, string, string, CancellationToken)
      Writes a secret to Credential Manager for a drive.
      Pass null value to delete the secret.

  - DeleteSecretProperty(Guid, string, CancellationToken) -> bool
      Deletes a single secret from Credential Manager.
      Returns true if deleted, false if not found.

  - GetSecretNames(Guid, CancellationToken) -> List<string>
      Lists all secret key names configured for a drive.
      Use to enumerate what secrets exist without exposing values.

  - DeleteAllSecrets(Guid, CancellationToken)
      Deletes all secrets for a drive.
      Call when unmounting/deleting a drive.

  JSON Operations:
  - ReadConfigurationFromJson(string, CancellationToken) -> DriveConfiguration
      Deserializes a drive configuration from JSON.

  - ToJson(DriveConfiguration, CancellationToken) -> string
      Serializes a drive configuration to JSON.

ProviderManager.cs
  Static class for provider registration:

  - ReadProviders(CancellationToken) -> IEnumerable<ProviderConfiguration>
      Enumerates all registered providers.

  - ReadProvider(string, CancellationToken) -> ProviderConfiguration
      Reads a specific provider's configuration.

  - RegisterProvider(ProviderConfiguration, CancellationToken)
      Registers a provider in the registry.
      Called by providers during COM+ installation.

  - UnRegisterProvider(Guid, CancellationToken)
      Removes a provider's registration.
      Called during provider uninstallation.

Extensions/
  DriveConfigurationExtensions.cs
    - ToJson() extension method for DriveConfiguration
    - Provides compact JSON serialization

PROVIDERS VS DRIVES
--------------------------------------------------------------------------------
Understanding the distinction between Providers and Drives is critical:

PROVIDERS (SOFTWARE\BigDrive\Providers\{CLSID})
  - Registered automatically when COM+ component is installed (regsvcs.exe)
  - Define HOW to access a storage backend (API client, auth, etc.)
  - Have a CLSID (COM Class ID) for activation
  - Examples: "Flickr Provider", "Azure Blob Provider"

DRIVES (SOFTWARE\BigDrive\Drives\{DRIVE-GUID})
  - Created by users via BigDrive.Shell 'mount' command or setup
  - Define WHAT storage to access (specific account, container, etc.)
  - Reference a Provider via CLSID
  - Multiple drives can use the same provider
  - Examples: "My Flickr Photos", "Work Azure Storage", "Personal Azure Storage"

Relationship:
  ┌─────────────────────┐
  │ Drive: "My Photos"  │──┐
  │ GUID: {DRIVE-1}     │  │
  └─────────────────────┘  │    ┌──────────────────────────┐
                           ├───▶│ Provider: Flickr Provider │
  ┌─────────────────────┐  │    │ CLSID: {PROVIDER-CLSID}  │
  │ Drive: "Work Photos"│──┘    └──────────────────────────┘
  │ GUID: {DRIVE-2}     │
  └─────────────────────┘

DATA FLOW
--------------------------------------------------------------------------------

Provider Registration Flow (Installation):
  1. Provider's COM+ PostBuild runs regsvcs.exe
  2. Provider's IProcessInitializer.Startup() is called
  3. Provider's IBigDriveRegistration.Register() is called
  4. Provider calls ProviderManager.RegisterProvider()
  5. ProviderManager writes to SOFTWARE\BigDrive\Providers\{CLSID}

Drive Creation Flow (User mounts a drive):
  1. User runs BigDrive.Shell and types 'mount'
  2. Shell calls ProviderManager.ReadProviders() to list available providers
  3. User selects a provider and names the drive
  4. Shell calls DriveManager.WriteConfiguration()
  5. DriveManager writes to SOFTWARE\BigDrive\Drives\{DriveId}
  6. DriveLetterManager assigns a drive letter (Z:, Y:, etc.)

Shell/Explorer Discovery Flow:
  1. BigDrive.Shell or BigDrive.ShellFolder starts
  2. DriveManager.ReadConfigurations() enumerates all drives
  3. For each drive, reads the CLSID to identify the provider
  4. COM+ instance created using CoCreateInstance(CLSID)
  5. Provider interfaces called for enumeration/file operations

REFLECTION-BASED SERIALIZATION
--------------------------------------------------------------------------------
Both DriveManager and ProviderManager use reflection to map properties
to registry values:

  1. Get all properties of the configuration class
  2. Check for [JsonPropertyName] attribute
  3. Use attribute name or property name as registry value name
  4. Read/write value with appropriate type conversion

Supported property types:
  - Guid (stored as "{GUID-STRING}")
  - String (stored as REG_SZ)
  - Enum (stored as string name)
  - Other types (via Convert.ChangeType)

JSON SERIALIZATION
--------------------------------------------------------------------------------
Configuration objects can be serialized to/from JSON for:
  - Inter-process communication
  - Configuration export/import
  - Debugging and logging

JSON options used:
  - PropertyNameCaseInsensitive = true (for reading)
  - WriteIndented = false (compact output)
  - JsonStringEnumConverter for enum handling

THREAD SAFETY
--------------------------------------------------------------------------------
All public methods accept a CancellationToken parameter for:
  - Cooperative cancellation during long operations
  - Async pattern compatibility
  - Clean shutdown during COM+ application termination

Registry operations are atomic at the value level, but callers should
ensure proper synchronization when multiple processes may write
simultaneously.

DEPENDENCIES
--------------------------------------------------------------------------------

Framework References:
  - Microsoft.Win32 (Registry access)
  - System.Text.Json (JSON serialization)

This project has no external NuGet dependencies.

TARGET FRAMEWORK
--------------------------------------------------------------------------------
.NET Framework 4.7.2

Required for compatibility with:
  - BigDrive.Interfaces (COM interop)
  - BigDrive provider projects (COM+ components)
  - System.EnterpriseServices integration

USAGE EXAMPLES
--------------------------------------------------------------------------------

Listing Available Providers:
  foreach (var provider in ProviderManager.ReadProviders(CancellationToken.None))
  {
      Console.WriteLine($"Provider: {provider.Name}, CLSID: {provider.Id}");
  }

Registering a Provider (called from provider's Register() method):
  var config = new ProviderConfiguration
  {
      Id = Provider.CLSID,
      Name = "My Provider"
  };
  ProviderManager.RegisterProvider(config, CancellationToken.None);

Creating a Drive (called from BigDrive.Shell 'mount' command):
  var driveConfig = new DriveConfiguration
  {
      Id = Guid.NewGuid(),
      Name = "My Virtual Drive",
      CLSID = selectedProvider.Id  // CLSID of the provider
  };
  DriveManager.WriteConfiguration(driveConfig, CancellationToken.None);

Deleting a Drive (called from BigDrive.Shell 'unmount' command):
  DriveManager.DeleteConfiguration(driveId, CancellationToken.None);

Reading All Drives:
  foreach (var drive in DriveManager.ReadConfigurations(CancellationToken.None))
  {
      Console.WriteLine($"Drive: {drive.Name}, Provider CLSID: {drive.CLSID}");
  }

Storing Secrets (API keys, OAuth tokens):
  // Write a secret (encrypted in Windows Credential Manager)
  DriveManager.WriteSecretProperty(
      driveId, 
      "FlickrApiKey", 
      apiKey, 
      CancellationToken.None);

  // Read a secret
  string apiKey = DriveManager.ReadSecretProperty(
      driveId, 
      "FlickrApiKey", 
      CancellationToken.None);

  // List all secrets for a drive
  List<string> secretNames = DriveManager.GetSecretNames(
      driveId, 
      CancellationToken.None);

  // Delete a specific secret
  DriveManager.DeleteSecretProperty(
      driveId, 
      "FlickrApiKey", 
      CancellationToken.None);

  // Delete all secrets (during unmount)
  DriveManager.DeleteAllSecrets(driveId, CancellationToken.None);

SEE ALSO
--------------------------------------------------------------------------------
  - docs/BigDrive.Shell.UserGuide.md - Shell commands including mount/unmount
  - docs/ProviderDevelopmentGuide.md - Creating new providers
  - src/BigDrive.Shell/README.md - Shell architecture

================================================================================
                    Copyright (c) Wayne Walter Berry. All rights reserved.
================================================================================
