================================================================================
BigDrive.Setup - COM+ Provider Registration Architecture
================================================================================

OVERVIEW
--------
BigDrive.Setup is responsible for bootstrapping the BigDrive system by
registering .NET ServicedComponent assemblies with COM+ Component Services.
This enables out-of-process hosting of BigDrive providers in dllhost.exe,
providing process isolation, security boundaries, and lifecycle management.


ARCHITECTURE DIAGRAM
--------------------

    ┌─────────────────────────────────────────────────────────────────────────┐
    │                        WINDOWS EXPLORER (Shell)                         │
    │                                                                         │
    │  BigDrive.Client (Shell Extension)                                      │
    │    │                                                                    │
    │    │  CoCreateInstance(Provider CLSID)                                  │
    │    ▼                                                                    │
    ├─────────────────────────────────────────────────────────────────────────┤
    │                          COM+ RUNTIME                                   │
    │                                                                         │
    │  ┌─────────────────────────────────────────────────────────────────┐    │
    │  │  dllhost.exe (COM+ Surrogate Process)                           │    │
    │  │                                                                 │    │
    │  │  ┌─────────────────────┐  ┌─────────────────────┐               │    │
    │  │  │ BigDrive.Service    │  │ Provider.Flickr    │  ...          │    │
    │  │  │ (ServicedComponent) │  │ (ServicedComponent) │               │    │
    │  │  └─────────────────────┘  └─────────────────────┘               │    │
    │  │                                                                 │    │
    │  │  Identity: BigDriveTrustedInstaller (Local Service Account)     │    │
    │  └─────────────────────────────────────────────────────────────────┘    │
    │                                                                         │
    └─────────────────────────────────────────────────────────────────────────┘
                                       │
                                       │ Registry / Flickr API / etc.
                                       ▼
    ┌─────────────────────────────────────────────────────────────────────────┐
    │                         EXTERNAL RESOURCES                              │
    │  - HKLM\SOFTWARE\BigDrive (Configuration)                               │
    │  - Flickr API, Azure Blob Storage, etc.                                 │
    └─────────────────────────────────────────────────────────────────────────┘


COM+ REGISTRATION FLOW
----------------------

    BigDrive.Setup.exe (Run As Administrator)
           │
           ├─► 1. Create Event Log Sources
           │       - BigDrive.Service
           │       - BigDrive.ShellFolder
           │       - BigDrive.Extension
           │       - BigDrive.Client
           │       - BigDrive.Provider.Sample
           │
           ├─► 2. Create Service Account
           │       - Username: BigDriveTrustedInstaller
           │       - Local machine account with limited privileges
           │
           ├─► 3. Configure Registry Permissions
           │       - HKLM\SOFTWARE\BigDrive
           │       - Grant Full Control to BigDriveTrustedInstaller
           │
           ├─► 4. Register COM+ Assembly
           │       │
           │       └─► regsvcs.exe BigDrive.Service.dll
           │               │
           │               ├─► Reads [ApplicationActivation(Server)]
           │               ├─► Reads [ComVisible(true)] classes
           │               ├─► Creates COM+ Application: "BigDrive.Service"
           │               ├─► Registers CLSIDs in HKCR\CLSID
           │               └─► Configures component in Component Services
           │
           ├─► 5. Set COM+ Application Identity
           │       - COMAdminCatalog.Applications["BigDrive.Service"]
           │       - Identity = BigDriveTrustedInstaller
           │       - Password = [auto-generated]
           │
           └─► 6. Validate Installation
                   - CoCreateInstance(BigDriveService CLSID)
                   - Call IBigDriveSetup.Validate()
                   - Verify Event Log entry


KEY COMPONENTS
--------------

ComRegistrationManager.cs
    ├── RegisterComAssemblyAsUser()     - Calls regsvcs.exe to register assembly
    ├── DeleteComPlusApplication()      - Removes COM+ app via COMAdminCatalog
    ├── SetApplicationIdentityToThisUser() - Configures service account identity
    └── CallServiceValidate()           - Tests COM+ activation works

UserManager.cs
    ├── CreateBigDriveTrustedInstaller() - Creates local service account
    ├── DeleteBigDriveTrustedInstaller() - Removes service account
    └── UserExists()                     - Checks if account exists

RegistryManager.cs
    ├── EnsureBigDriveRegistryKeyExists() - Creates HKLM\SOFTWARE\BigDrive
    ├── GrantFullControl()                - ACL for service account
    └── RemoveFullControl()               - Revokes permissions

EventViewerManager.cs
    ├── CreateEventSource()              - Registers Event Log source
    └── VerifyLogging()                  - Validates provider can log


PROVIDER ASSEMBLY REQUIREMENTS
------------------------------

For a .NET assembly to be registered as a COM+ provider:

1. Assembly-Level Attributes (AssemblyInfo.cs):

    [assembly: ApplicationActivation(ActivationOption.Server)]
    [assembly: ApplicationAccessControl(false)]
    [assembly: ComVisible(true)]

2. Provider Class:

    [Guid("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")]
    [ClassInterface(ClassInterfaceType.None)]
    [ComVisible(true)]
    public class Provider : ServicedComponent,
        IProcessInitializer,       // COM+ lifecycle hooks
        IBigDriveEnumerate,        // Folder/file enumeration
        IBigDriveFileInfo,         // File metadata
        IBigDriveFileOperations,   // Copy/move/delete
        IBigDriveFileData          // File content streaming

3. IProcessInitializer Implementation:

    public void Startup(object punkProcessControl)
    {
        // Called when COM+ starts the component
        // Initialize resources, authenticate to APIs, etc.
    }

    public void Shutdown()
    {
        // Called when COM+ shuts down the component
        // Clean up resources
    }


BUILD-TIME REGISTRATION
-----------------------

Provider projects include PostBuild events for development convenience:

    <Target Name="PreBuild" BeforeTargets="PreBuildEvent">
        <Exec Command="regsvcs.exe /u $(TargetPath) || exit /b 0" />
    </Target>

    <Target Name="PostBuild" AfterTargets="PostBuildEvent">
        <Exec Command="regsvcs.exe $(TargetPath)" />
    </Target>

This automatically registers the provider after each build.


SECURITY MODEL
--------------

    ┌──────────────────────┐
    │   Explorer.exe       │  User Context (Interactive)
    │   (BigDrive.Client)  │
    └──────────┬───────────┘
               │ COM Activation Request
               ▼
    ┌──────────────────────┐
    │   dllhost.exe        │  BigDriveTrustedInstaller Context
    │   (COM+ Surrogate)   │
    │                      │  - Limited local account
    │   - BigDrive.Service │  - Registry access to HKLM\SOFTWARE\BigDrive
    │   - Provider.Flickr  │  - Network access for APIs
    │   - Provider.Sample  │  - No interactive desktop access
    └──────────────────────┘

Process isolation ensures:
    - Shell extension crashes don't take down Explorer
    - Provider code runs with minimal privileges
    - Credential isolation (API keys stored securely)


COMPONENT SERVICES MMC
----------------------

After registration, providers appear in:

    Component Services
    └── Computers
        └── My Computer
            └── COM+ Applications
                └── BigDrive.Service
                    └── Components
                        ├── BigDrive.Service.BigDriveService
                        ├── BigDrive.Provider.Sample.Provider
                        └── BigDrive.Provider.Flickr.Provider


TROUBLESHOOTING
---------------

1. "Access Denied" during registration
   - Run BigDrive.Setup.exe as Administrator
   - Ensure no COM+ applications are using the DLLs

2. Provider not appearing in Component Services
   - Verify [ComVisible(true)] on assembly and class
   - Verify [ApplicationActivation(ActivationOption.Server)]
   - Check Event Viewer > Application for regsvcs errors

3. CoCreateInstance fails at runtime
   - Verify CLSID is registered in HKCR\CLSID
   - Check COM+ application is running (dllhost.exe in Task Manager)
   - Review BigDrive Event Log for errors

4. IProcessInitializer.Startup() not called
   - Ensure class inherits from ServicedComponent
   - Ensure class implements IProcessInitializer explicitly
   - COM+ may delay initialization until first method call


RELATED FILES
-------------

    src\BigDrive.Setup\
    ├── Program.cs                  - Entry point, orchestrates setup
    ├── ComRegistrationManager.cs   - COM+ registration logic
    ├── UserManager.cs              - Service account management
    ├── RegistryManager.cs          - Registry key management
    ├── EventViewerManager.cs       - Event Log configuration
    └── Constants.cs                - Event Log names

    src\BigDrive.Service\
    └── Properties\AssemblyInfo.cs  - COM+ attributes

    src\BigDrive.Provider.Flickr\
    ├── Provider.cs                 - ServicedComponent class
    ├── Provider.IProcessInitializer.cs - Lifecycle hooks
    └── Properties\AssemblyInfo.cs  - COM+ attributes


================================================================================
Copyright (c) Wayne Walter Berry. All rights reserved.
================================================================================
