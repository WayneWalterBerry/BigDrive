================================================================================
                      BigDrive.Provider.Flickr Project
                         Architecture Documentation
================================================================================

OVERVIEW
--------------------------------------------------------------------------------
BigDrive.Provider.Flickr is a BigDrive provider that integrates Flickr photo
storage with Windows Explorer. It exposes Flickr photosets as folders and
photos as files, allowing users to browse, view, upload, and download photos
directly from File Explorer.

ARCHITECTURAL ROLE
--------------------------------------------------------------------------------
This provider implements the BigDrive.Interfaces contracts and runs as a COM+
out-of-process server, communicating with the BigDrive shell extension:

    +-------------------+       COM+        +---------------------------+
    |  explorer.exe     |  <=============>  |  BigDrive.Provider.Flickr |
    |  (BigDrive.       |   Out-of-Process  |  (ServicedComponent)      |
    |   ShellFolder)    |   COM Interop     |                           |
    +-------------------+                   +---------------------------+
                                                       |
                                                       | HTTPS/REST
                                                       v
                                            +-------------------+
                                            |   Flickr API      |
                                            |   (FlickrNet SDK) |
                                            +-------------------+

VIRTUAL FILE SYSTEM MAPPING
--------------------------------------------------------------------------------
The provider maps Flickr's structure to a file system hierarchy:

    Flickr Concept          Virtual File System
    --------------          -------------------
    User's Photosets   -->  Root Folders
    Photoset (Album)   -->  Folder
    Photo              -->  .jpg File

Example structure in Windows Explorer:

    Flickr Photos (BigDrive)
    +-- Vacation 2024\
    |   +-- Beach Sunset.jpg
    |   +-- Mountain View.jpg
    +-- Family Photos\
    |   +-- Birthday Party.jpg
    +-- Nature\
        +-- Forest Trail.jpg
        +-- Waterfall.jpg

INTERFACE IMPLEMENTATIONS
--------------------------------------------------------------------------------

Provider.cs (Main Class)
  - COM CLSID: B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B
  - Inherits: ServicedComponent (COM+ hosting)
  - Implements all BigDrive interfaces as partial classes

Provider.IBigDriveEnumerate.cs
  - EnumerateFolders(): Returns photoset names at root level
  - EnumerateFiles(): Returns photo names within a photoset
  - Helper methods for path parsing and name sanitization

Provider.IBigDriveFileInfo.cs
  - LastModifiedTime(): Returns photo upload date
  - GetFileSize(): Returns photo file size (from Flickr metadata)

Provider.IBigDriveFileOperations.cs
  - CopyFileToBigDrive(): Uploads local photo to Flickr
  - CopyFileFromBigDrive(): Downloads photo to local storage
  - DeleteFile(): Deletes photo or photoset from Flickr
  - CreateDirectory(): Creates new photoset (requires initial photo)
  - OpenFile(): Opens photo URL in default browser
  - MoveFile(): Moves photo between photosets

Provider.IBigDriveFileData.cs
  - GetFileData(): Downloads and returns photo as IStream

Provider.IProcessInitializer.cs
  - Startup(): Called when COM+ application starts
  - Shutdown(): Called when COM+ application stops

Provider.IBigDriveRegistration.cs
  - Register(): Registers provider with BigDrive system
  - Unregister(): Removes provider registration

FLICKR API INTEGRATION
--------------------------------------------------------------------------------

FlickrClientWrapper.cs
  Encapsulates all Flickr API interactions using the FlickrNet SDK:

  Authentication:
    - API Key and Secret read from registry
    - OAuth support for user-specific operations (TODO)

  Caching:
    - Photoset list cached for 5 minutes
    - Reduces API calls for repeated folder enumeration

  API Methods Used:
    - PhotosetsGetList(): List user's photosets
    - PhotosetsGetPhotos(): List photos in a photoset
    - UploadPicture(): Upload new photo
    - PhotosDelete(): Delete a photo
    - PhotosetsAddPhoto(): Add photo to photoset
    - PhotosetsRemovePhoto(): Remove photo from photoset

SUPPORTING CLASSES
--------------------------------------------------------------------------------

ComStream.cs
  Wraps .NET Stream as COM IStream for cross-process data transfer.
  Required for IBigDriveFileData implementation.

BigDriveTraceSource.cs
  Provides Windows Event Log tracing for diagnostics.
  Source Name: "BigDrive.Provider.Flickr"

ProviderConfigurationFactory.cs
  Creates ProviderConfiguration from assembly metadata.

PhotosetInfo / PhotoInfo
  Internal DTOs for Flickr data (defined in FlickrClientWrapper.cs).

CONFIGURATION
--------------------------------------------------------------------------------
Configuration is stored in the Windows Registry:

  HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\Providers\Flickr
    FlickrApiKey     (REG_SZ) - Your Flickr API key
    FlickrApiSecret  (REG_SZ) - Your Flickr API secret

To obtain API credentials:
  1. Go to https://www.flickr.com/services/apps/create/
  2. Create a new application
  3. Copy the API Key and Secret to the registry

REGISTRATION
--------------------------------------------------------------------------------
The provider registers itself during COM+ installation:

  1. Provider CLSID registered in: SOFTWARE\BigDrive\Providers
  2. Drive configuration created with:
     - Name: "Flickr Photos"
     - Drive ID: A2B3C4D5-E6F7-8901-A2B3-C4D5E6F78901

Pre/Post Build Events:
  - PreBuild: regsvcs.exe /u (unregister previous version)
  - PostBuild: regsvcs.exe (register new version)

DEPENDENCIES
--------------------------------------------------------------------------------

NuGet Packages:
  - FlickrNet (3.24.0) - Flickr API client library

Project References:
  - BigDrive.Interfaces - Interface contracts
  - BigDrive.ConfigProvider - Configuration management
  - BigDrive.Service - Drive and provider registration

Framework References:
  - System.EnterpriseServices - COM+ hosting
  - System.Net.Http - HTTP client for photo downloads

TARGET FRAMEWORK
--------------------------------------------------------------------------------
.NET Framework 4.7.2

Required for System.EnterpriseServices (COM+) support and COM interop
compatibility with the native BigDrive shell extension.

LIMITATIONS AND FUTURE WORK
--------------------------------------------------------------------------------

Current Limitations:
  - OAuth authentication not yet implemented (read-only public photos)
  - CreateDirectory requires uploading a photo first (Flickr API limitation)
  - File size may not be accurate until photo is downloaded
  - No thumbnail/preview support

Future Enhancements:
  - Implement OAuth flow for authenticated user access
  - Add support for photo tags and descriptions
  - Implement photo search functionality
  - Add thumbnail generation for Explorer previews
  - Support for Flickr collections (groups of photosets)

================================================================================
                    Copyright (c) Wayne Walter Berry. All rights reserved.
================================================================================
