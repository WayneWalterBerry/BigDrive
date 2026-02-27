# BigDrive Provider Examples

This guide provides complete, working examples of BigDrive providers demonstrating
different patterns and capabilities.

---

## Example 1: Read-Only Local File Provider (ISO)

**Use case:** Mount ISO images as virtual drives for browsing.

**Key features:**
- Read-only file access
- Local file parameter (IsoFilePath)
- DiscUtils library for ISO reading
- Simple implementation (~250 lines total)

### Project Structure

```
BigDrive.Provider.Iso/
├── Provider.cs                             # Main class + static constructor
├── Provider.IProcessInitializer.cs         # Startup/Shutdown
├── Provider.IBigDriveRegistration.cs       # Register/Unregister
├── Provider.IBigDriveDriveInfo.cs          # GetDriveParameters
├── Provider.IBigDriveEnumerate.cs          # EnumerateFolders/EnumerateFiles
├── Provider.IBigDriveFileInfo.cs           # LastModifiedTime/GetFileSize
├── Provider.IBigDriveFileData.cs           # GetFileData
├── IsoClientWrapper.cs                     # DiscUtils wrapper
├── AssemblyResolver.cs                     # NuGet resolution
├── BigDriveTraceSource.cs                  # Logging
├── ComStream.cs                            # IStream wrapper
├── ProviderConfigurationFactory.cs         # Configuration
└── app.config                              # Binding redirects
```

### Key Implementation: IsoClientWrapper.cs

```csharp
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using DiscUtils.Iso9660;

namespace BigDrive.Provider.Iso
{
    internal class IsoClientWrapper : IDisposable
    {
        private readonly string _isoFilePath;
        private FileStream _isoStream;
        private CDReader _cdReader;

        public IsoClientWrapper(string isoFilePath)
        {
            _isoFilePath = isoFilePath;
            _isoStream = File.OpenRead(isoFilePath);
            _cdReader = new CDReader(_isoStream, true);
        }

        public string[] GetFolders(string path)
        {
            if (!_cdReader.DirectoryExists(path))
            {
                return Array.Empty<string>();
            }

            return _cdReader.GetDirectories(path)
                .Select(Path.GetFileName)
                .ToArray();
        }

        public string[] GetFiles(string path)
        {
            if (!_cdReader.DirectoryExists(path))
            {
                return Array.Empty<string>();
            }

            return _cdReader.GetFiles(path)
                .Select(Path.GetFileName)
                .ToArray();
        }

        public Stream OpenFile(string path)
        {
            if (!_cdReader.FileExists(path))
            {
                return null;
            }

            return _cdReader.OpenFile(path, FileMode.Open);
        }

        public long GetFileSize(string path)
        {
            if (!_cdReader.FileExists(path))
            {
                return 0;
            }

            return _cdReader.GetFileInfo(path).Length;
        }

        public DateTime GetLastModifiedTime(string path)
        {
            if (!_cdReader.FileExists(path))
            {
                return DateTime.MinValue;
            }

            return _cdReader.GetFileInfo(path).LastWriteTimeUtc;
        }

        public void Dispose()
        {
            _cdReader?.Dispose();
            _isoStream?.Dispose();
        }
    }
}
```

### Key Implementation: Provider.IBigDriveDriveInfo.cs

```csharp
using System.Text.Json;
using BigDrive.Interfaces.Model;

public partial class Provider
{
    public string GetDriveParameters()
    {
        DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
        {
            new DriveParameterDefinition
            {
                Name = "IsoFilePath",
                Description = "Full path to the ISO file to browse (e.g., C:\\ISOs\\game.iso).",
                Type = "existing-file" // Validates file exists
            }
        };

        return JsonSerializer.Serialize(parameters);
    }
}
```

**See full implementation:** `src/BigDrive.Provider.Iso/`

---

## Example 2: Multi-Format Archive Provider

**Use case:** Browse ZIP, TAR, 7z, RAR archives without extraction.

**Key features:**
- Multiple archive formats (via SharpCompress)
- Read-only access
- Stream-based file access
- Format auto-detection

### Key Implementation: ArchiveClientWrapper.cs

```csharp
using SharpCompress.Archives;
using SharpCompress.Common;

namespace BigDrive.Provider.Archive
{
    internal class ArchiveClientWrapper : IDisposable
    {
        private readonly string _archiveFilePath;
        private IArchive _archive;

        public ArchiveClientWrapper(string archiveFilePath)
        {
            _archiveFilePath = archiveFilePath;
            
            // SharpCompress auto-detects format (zip, tar, 7z, rar)
            using (FileStream fs = File.OpenRead(archiveFilePath))
            {
                _archive = ArchiveFactory.Open(fs);
            }
        }

        public string[] GetFolders(string path)
        {
            HashSet<string> folders = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            string prefix = path.TrimStart('\\', '/');
            int depth = string.IsNullOrEmpty(prefix) ? 0 : prefix.Split('/').Length;

            foreach (IArchiveEntry entry in _archive.Entries)
            {
                if (!entry.IsDirectory)
                {
                    continue;
                }

                string entryPath = entry.Key.Replace('\\', '/').TrimStart('/');

                if (string.IsNullOrEmpty(prefix) || entryPath.StartsWith(prefix + "/"))
                {
                    string relativePath = string.IsNullOrEmpty(prefix) 
                        ? entryPath 
                        : entryPath.Substring(prefix.Length + 1);

                    string[] parts = relativePath.Split('/');
                    if (parts.Length > 0 && !string.IsNullOrEmpty(parts[0]))
                    {
                        folders.Add(parts[0]);
                    }
                }
            }

            return folders.ToArray();
        }

        public Stream OpenFile(string path)
        {
            string normalizedPath = path.Replace('\\', '/').TrimStart('/');

            IArchiveEntry entry = _archive.Entries.FirstOrDefault(
                e => e.Key.Replace('\\', '/').TrimStart('/').Equals(normalizedPath, 
                     StringComparison.OrdinalIgnoreCase));

            if (entry == null || entry.IsDirectory)
            {
                return null;
            }

            return entry.OpenEntryStream();
        }

        public void Dispose()
        {
            _archive?.Dispose();
        }
    }
}
```

**See full implementation:** `src/BigDrive.Provider.Archive/`

---

## Example 3: Cloud Provider with OAuth (Flickr)

**Use case:** Browse Flickr photos as files in Windows Explorer.

**Key features:**
- OAuth 1.0a authentication
- Paginated API calls
- Photo metadata as file properties
- Thumbnail caching
- API rate limiting

### Key Implementation: FlickrClientWrapper.cs

```csharp
using System.Net.Http;
using System.Threading;
using BigDrive.ConfigProvider;

namespace BigDrive.Provider.Flickr
{
    internal class FlickrClientWrapper
    {
        private readonly Guid _driveGuid;
        private readonly HttpClient _httpClient;
        private string _oauthToken;
        private string _oauthSecret;

        public FlickrClientWrapper(Guid driveGuid)
        {
            _driveGuid = driveGuid;
            _httpClient = new HttpClient();
            LoadTokens();
        }

        private void LoadTokens()
        {
            _oauthToken = DriveManager.ReadSecretProperty(_driveGuid, "FlickrOAuthToken", 
                                                          CancellationToken.None);
            _oauthSecret = DriveManager.ReadSecretProperty(_driveGuid, "FlickrOAuthSecret", 
                                                           CancellationToken.None);
        }

        public List<FlickrPhoto> GetPhotos(string albumId)
        {
            if (string.IsNullOrEmpty(_oauthToken))
            {
                throw new BigDriveAuthenticationRequiredException(
                    _driveGuid,
                    "Flickr",
                    AuthenticationFailureReason.NotAuthenticated,
                    null);
            }

            // Build OAuth 1.0a signed request
            var request = BuildFlickrRequest("flickr.photosets.getPhotos", new Dictionary<string, string>
            {
                { "photoset_id", albumId },
                { "extras", "date_taken,url_o" }
            });

            HttpResponseMessage response = _httpClient.SendAsync(request).Result;

            if (response.StatusCode == HttpStatusCode.Unauthorized)
            {
                throw new BigDriveAuthenticationRequiredException(
                    _driveGuid,
                    "Flickr",
                    AuthenticationFailureReason.TokenExpired,
                    null);
            }

            response.EnsureSuccessStatusCode();
            return ParsePhotosResponse(response.Content.ReadAsStringAsync().Result);
        }

        private HttpRequestMessage BuildFlickrRequest(string method, Dictionary<string, string> parameters)
        {
            // OAuth 1.0a signature generation
            string timestamp = DateTimeOffset.UtcNow.ToUnixTimeSeconds().ToString();
            string nonce = Guid.NewGuid().ToString("N");

            parameters["method"] = method;
            parameters["oauth_token"] = _oauthToken;
            parameters["oauth_timestamp"] = timestamp;
            parameters["oauth_nonce"] = nonce;
            parameters["oauth_signature_method"] = "HMAC-SHA1";
            parameters["oauth_version"] = "1.0";
            parameters["oauth_consumer_key"] = GetApiKey();

            string signature = GenerateOAuthSignature(parameters);
            parameters["oauth_signature"] = signature;

            string queryString = string.Join("&", parameters.Select(kvp => $"{kvp.Key}={Uri.EscapeDataString(kvp.Value)}"));
            return new HttpRequestMessage(HttpMethod.Get, $"https://api.flickr.com/services/rest?{queryString}");
        }

        // ... OAuth signature generation, photo parsing, etc.
    }
}
```

**See full implementation:** `src/BigDrive.Provider.Flickr/`

---

## Example 4: Minimal Provider Template

**Use case:** Starting point for new providers - implements only required interfaces.

### Complete Provider.cs

```csharp
// <copyright file="Provider.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Minimal
{
    using System;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;

    using BigDrive.Interfaces;

    [Guid("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")]
    [ClassInterface(ClassInterfaceType.None)]
    [ComVisible(true)]
    public partial class Provider : ServicedComponent,
        IProcessInitializer,
        IBigDriveRegistration,
        IBigDriveDriveInfo,
        IBigDriveEnumerate
    {
        private static readonly BigDriveTraceSource DefaultTraceSource = BigDriveTraceSource.Instance;

        static Provider()
        {
            AssemblyResolver.Initialize();
        }

        public static Guid CLSID
        {
            get
            {
                Type providerType = typeof(Provider);
                GuidAttribute guidAttribute = (GuidAttribute)Attribute.GetCustomAttribute(
                    providerType, typeof(GuidAttribute));
                return Guid.Parse(guidAttribute.Value);
            }
        }

        private static BigDrive.ConfigProvider.Model.ProviderConfiguration ProviderConfig
        {
            get
            {
                return ProviderConfigurationFactory.Create();
            }
        }
    }
}
```

### Complete Provider.IProcessInitializer.cs

```csharp
// <copyright file="Provider.IProcessInitializer.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Minimal
{
    public partial class Provider
    {
        public void Startup(object punkProcessControl)
        {
            DefaultTraceSource.TraceInformation("Minimal Provider Startup");
        }

        public void Shutdown()
        {
            DefaultTraceSource.TraceInformation("Minimal Provider Shutdown");
        }
    }
}
```

### Complete Provider.IBigDriveDriveInfo.cs

```csharp
// <copyright file="Provider.IBigDriveDriveInfo.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Minimal
{
    using System.Text.Json;

    using BigDrive.Interfaces.Model;

    public partial class Provider
    {
        public string GetDriveParameters()
        {
            DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
            {
                new DriveParameterDefinition
                {
                    Name = "RootPath",
                    Description = "Root path to expose as virtual drive.",
                    Type = "string"
                }
            };

            return JsonSerializer.Serialize(parameters);
        }
    }
}
```

### Complete Provider.IBigDriveEnumerate.cs

```csharp
// <copyright file="Provider.IBigDriveEnumerate.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Minimal
{
    using System;

    public partial class Provider
    {
        public string[] EnumerateFolders(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"EnumerateFolders: path={path}");

                // Your implementation here
                return new string[] { "Folder1", "Folder2" };
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"EnumerateFolders failed: {ex.Message}");
                return Array.Empty<string>();
            }
        }

        public string[] EnumerateFiles(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"EnumerateFiles: path={path}");

                // Your implementation here
                return new string[] { "File1.txt", "File2.jpg" };
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"EnumerateFiles failed: {ex.Message}");
                return Array.Empty<string>();
            }
        }
    }
}
```

**Download:** Copy template files from `src/BigDrive.Provider.Iso/` and modify for your needs.

---

## Example 5: Client Wrapper Pattern

All providers should wrap external API/SDK access in a client wrapper class. This provides:
- ✅ Consistent error handling
- ✅ Authentication token management
- ✅ Caching and performance optimization
- ✅ Testability (mock the wrapper)

### Pattern 1: Simple Wrapper (ISO Provider)

```csharp
internal class IsoClientWrapper : IDisposable
{
    private readonly string _isoFilePath;
    private CDReader _cdReader;

    public IsoClientWrapper(string isoFilePath)
    {
        _isoFilePath = isoFilePath;
        _cdReader = new CDReader(File.OpenRead(isoFilePath), true);
    }

    public string[] GetFolders(string path)
    {
        return _cdReader.GetDirectories(path)
            .Select(Path.GetFileName)
            .ToArray();
    }

    public void Dispose()
    {
        _cdReader?.Dispose();
    }
}
```

### Pattern 2: Per-Drive Wrapper with Caching (Flickr Provider)

```csharp
internal class FlickrClientWrapper
{
    private static readonly Dictionary<Guid, FlickrClientWrapper> Cache = 
        new Dictionary<Guid, FlickrClientWrapper>();

    private readonly Guid _driveGuid;
    private readonly HttpClient _httpClient;
    private Dictionary<string, CachedPhotos> _photoCache;

    public static FlickrClientWrapper GetForDrive(Guid driveGuid)
    {
        lock (Cache)
        {
            if (!Cache.TryGetValue(driveGuid, out FlickrClientWrapper wrapper))
            {
                wrapper = new FlickrClientWrapper(driveGuid);
                Cache[driveGuid] = wrapper;
            }

            return wrapper;
        }
    }

    public static void InvalidateCache(Guid driveGuid)
    {
        lock (Cache)
        {
            if (Cache.TryGetValue(driveGuid, out FlickrClientWrapper wrapper))
            {
                wrapper.Dispose();
                Cache.Remove(driveGuid);
            }
        }
    }

    private FlickrClientWrapper(Guid driveGuid)
    {
        _driveGuid = driveGuid;
        _httpClient = new HttpClient();
        _photoCache = new Dictionary<string, CachedPhotos>();
        LoadCredentials();
    }

    private void LoadCredentials()
    {
        _apiKey = DriveManager.ReadSecretProperty(_driveGuid, "FlickrApiKey", 
                                                  CancellationToken.None);
        _oauthToken = DriveManager.ReadSecretProperty(_driveGuid, "FlickrOAuthToken", 
                                                      CancellationToken.None);
        _oauthSecret = DriveManager.ReadSecretProperty(_driveGuid, "FlickrOAuthSecret", 
                                                       CancellationToken.None);
    }

    public List<FlickrPhoto> GetPhotos(string path)
    {
        // Check cache first
        if (_photoCache.TryGetValue(path, out CachedPhotos cached))
        {
            if (DateTime.UtcNow - cached.Timestamp < TimeSpan.FromMinutes(5))
            {
                return cached.Photos;
            }
        }

        // Cache miss - fetch from API
        List<FlickrPhoto> photos = FetchPhotosFromApi(path);
        _photoCache[path] = new CachedPhotos { Photos = photos, Timestamp = DateTime.UtcNow };
        return photos;
    }

    public void Dispose()
    {
        _httpClient?.Dispose();
    }
}
```

---

## Example 6: Using IBigDriveDriveInfo for Configuration

### Simple Configuration (One Parameter)

```csharp
public string GetDriveParameters()
{
    DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
    {
        new DriveParameterDefinition
        {
            Name = "IsoFilePath",
            Description = "Full path to the ISO file.",
            Type = "existing-file"
        }
    };

    return JsonSerializer.Serialize(parameters);
}
```

**User experience:**
```sh
bigdrive drive create --provider Iso --name "Game"
bigdrive set IsoFilePath "C:\ISOs\game.iso"
```

### Complex Configuration (Multiple Parameters)

```csharp
public string GetDriveParameters()
{
    DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
    {
        new DriveParameterDefinition
        {
            Name = "ServerUrl",
            Description = "WebDAV server URL (e.g., https://myserver.com/webdav).",
            Type = "string"
        },
        new DriveParameterDefinition
        {
            Name = "Username",
            Description = "WebDAV username for authentication.",
            Type = "string"
        },
        new DriveParameterDefinition
        {
            Name = "RootDirectory",
            Description = "Root directory to mount (e.g., /shared/docs).",
            Type = "string"
        },
        new DriveParameterDefinition
        {
            Name = "CachePath",
            Description = "Local cache directory for downloaded files.",
            Type = "filepath"
        }
    };

    return JsonSerializer.Serialize(parameters);
}
```

**User experience:**
```sh
bigdrive drive create --provider WebDAV --name "Work"
bigdrive set ServerUrl "https://webdav.example.com"
bigdrive set Username "jsmith"
bigdrive set RootDirectory "/shared"
bigdrive set CachePath "C:\Temp\WebDAVCache"
bigdrive secret set WebDAVPassword "hunter2"  # Secrets stored separately
```

---

## Example 7: Error Handling Patterns

### Pattern 1: Enumerate Methods (Return Empty on Error)

```csharp
public string[] EnumerateFolders(Guid driveGuid, string path)
{
    try
    {
        DefaultTraceSource.TraceInformation($"EnumerateFolders: path={path}");

        YourServiceClientWrapper client = GetClient(driveGuid);
        string[] folders = client.GetFolders(NormalizePath(path));

        DefaultTraceSource.TraceInformation($"EnumerateFolders: returned {folders.Length} folders");
        return folders;
    }
    catch (BigDriveAuthenticationRequiredException)
    {
        // Re-throw authentication exceptions so Shell can handle them
        throw;
    }
    catch (Exception ex)
    {
        DefaultTraceSource.TraceError($"EnumerateFolders failed: {ex.Message}");
        return Array.Empty<string>(); // Return empty on error
    }
}
```

### Pattern 2: File Operations (Throw on Error)

```csharp
public void CopyFileToBigDrive(Guid driveGuid, string localFilePath, string bigDriveTargetPath)
{
    try
    {
        DefaultTraceSource.TraceInformation($"CopyFileToBigDrive: {localFilePath} -> {bigDriveTargetPath}");

        YourServiceClientWrapper client = GetClient(driveGuid);
        using (FileStream fs = File.OpenRead(localFilePath))
        {
            client.UploadFile(NormalizePath(bigDriveTargetPath), fs);
        }

        DefaultTraceSource.TraceInformation("CopyFileToBigDrive: succeeded");
    }
    catch (BigDriveAuthenticationRequiredException)
    {
        throw; // Re-throw for Shell to handle
    }
    catch (Exception ex)
    {
        DefaultTraceSource.TraceError($"CopyFileToBigDrive failed: {ex.Message}");
        throw; // Let Shell display error to user
    }
}
```

### Pattern 3: HRESULT Methods (Return Error Code)

```csharp
public int GetFileData(Guid driveGuid, string path, out IStream stream)
{
    stream = null;

    try
    {
        DefaultTraceSource.TraceInformation($"GetFileData: path={path}");

        YourServiceClientWrapper client = GetClient(driveGuid);
        Stream fileStream = client.OpenFile(NormalizePath(path));

        if (fileStream == null)
        {
            DefaultTraceSource.TraceError("GetFileData: file not found");
            return unchecked((int)0x80004005); // E_FAIL
        }

        stream = new ComStream(fileStream);
        return 0; // S_OK
    }
    catch (BigDriveAuthenticationRequiredException)
    {
        // Can't use HRESULT for auth exceptions - must throw
        throw;
    }
    catch (Exception ex)
    {
        DefaultTraceSource.TraceError($"GetFileData failed: {ex.Message}");
        return unchecked((int)0x80004005); // E_FAIL
    }
}
```

---

## Example 8: ComStream Implementation

Every provider that implements `IBigDriveFileData` needs a `ComStream` class to convert
.NET Stream → COM IStream:

```csharp
// <copyright file="ComStream.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.YourService
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    /// <summary>
    /// Wraps a .NET Stream as a COM IStream for interop.
    /// </summary>
    internal class ComStream : IStream, IDisposable
    {
        private readonly Stream _stream;

        public ComStream(Stream stream)
        {
            _stream = stream ?? throw new ArgumentNullException(nameof(stream));
        }

        public void Read(byte[] pv, int cb, IntPtr pcbRead)
        {
            int bytesRead = _stream.Read(pv, 0, cb);
            if (pcbRead != IntPtr.Zero)
            {
                Marshal.WriteInt32(pcbRead, bytesRead);
            }
        }

        public void Write(byte[] pv, int cb, IntPtr pcbWritten)
        {
            _stream.Write(pv, 0, cb);
            if (pcbWritten != IntPtr.Zero)
            {
                Marshal.WriteInt32(pcbWritten, cb);
            }
        }

        public void Seek(long dlibMove, int dwOrigin, IntPtr plibNewPosition)
        {
            SeekOrigin origin = (SeekOrigin)dwOrigin;
            long newPosition = _stream.Seek(dlibMove, origin);
            if (plibNewPosition != IntPtr.Zero)
            {
                Marshal.WriteInt64(plibNewPosition, newPosition);
            }
        }

        public void SetSize(long libNewSize)
        {
            _stream.SetLength(libNewSize);
        }

        public void CopyTo(IStream pstm, long cb, IntPtr pcbRead, IntPtr pcbWritten)
        {
            throw new NotImplementedException();
        }

        public void Commit(int grfCommitFlags)
        {
            _stream.Flush();
        }

        public void Revert()
        {
            throw new NotImplementedException();
        }

        public void LockRegion(long libOffset, long cb, int dwLockType)
        {
            throw new NotImplementedException();
        }

        public void UnlockRegion(long libOffset, long cb, int dwLockType)
        {
            throw new NotImplementedException();
        }

        public void Stat(out System.Runtime.InteropServices.ComTypes.STATSTG pstatstg, int grfStatFlag)
        {
            pstatstg = new System.Runtime.InteropServices.ComTypes.STATSTG
            {
                type = 2, // STGTY_STREAM
                cbSize = _stream.Length,
                grfMode = 0
            };
        }

        public void Clone(out IStream ppstm)
        {
            throw new NotImplementedException();
        }

        public void Dispose()
        {
            _stream?.Dispose();
        }
    }
}
```

**Copy from:** `src/BigDrive.Provider.Iso/ComStream.cs`

---

## Example 9: ProviderConfigurationFactory

Every provider needs a configuration factory:

```csharp
// <copyright file="ProviderConfigurationFactory.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.YourService
{
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Factory for creating provider configuration.
    /// </summary>
    internal static class ProviderConfigurationFactory
    {
        /// <summary>
        /// Creates the provider configuration for registry registration.
        /// </summary>
        /// <returns>Provider configuration instance.</returns>
        public static ProviderConfiguration Create()
        {
            return new ProviderConfiguration
            {
                Id = Provider.CLSID,
                Name = "YourService"
            };
        }
    }
}
```

---

## Example 10: BigDriveTraceSource

Logging implementation (same for all providers):

```csharp
// <copyright file="BigDriveTraceSource.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.YourService
{
    using System;
    using System.Diagnostics;

    /// <summary>
    /// Provides logging to Windows Event Log.
    /// </summary>
    internal sealed class BigDriveTraceSource
    {
        private static readonly Lazy<BigDriveTraceSource> LazyInstance = 
            new Lazy<BigDriveTraceSource>(() => new BigDriveTraceSource());

        private readonly TraceSource _traceSource;

        private BigDriveTraceSource()
        {
            _traceSource = new TraceSource("BigDrive.Provider.YourService");
            _traceSource.Switch.Level = SourceLevels.All;

            // Write to Windows Event Log
            _traceSource.Listeners.Add(new EventLogTraceListener("BigDrive.Provider.YourService"));
        }

        public static BigDriveTraceSource Instance => LazyInstance.Value;

        public void TraceInformation(string message)
        {
            _traceSource.TraceInformation(message);
        }

        public void TraceError(string message)
        {
            _traceSource.TraceEvent(TraceEventType.Error, 0, message);
        }

        public void TraceWarning(string message)
        {
            _traceSource.TraceEvent(TraceEventType.Warning, 0, message);
        }
    }
}
```

**View logs:**
```powershell
Get-EventLog -LogName Application -Source "BigDrive.Provider.YourService" -Newest 20
```

---

## Comparison Matrix

| Feature | Iso | Archive | Zip | Flickr |
|---------|-----|---------|-----|--------|
| **Complexity** | Simple | Medium | Simple | Complex |
| **Lines of Code** | ~250 | ~400 | ~300 | ~800 |
| **Read Files** | ✅ Yes | ✅ Yes | ✅ Yes | ✅ Yes |
| **Write Files** | ❌ No | ❌ No | ✅ Yes | ❌ No |
| **OAuth** | ❌ No | ❌ No | ❌ No | ✅ Yes (1.0a) |
| **NuGet Packages** | DiscUtils | SharpCompress | System.IO.Compression | FlickrNet |
| **Configuration** | IsoFilePath | ArchiveFilePath | ZipFilePath | API keys + OAuth |
| **Caching** | ❌ No | ❌ No | ❌ No | ✅ Yes |
| **Best for Learning** | ✅ Start here | Step 2 | Step 2 | Advanced |

---

## Copy-Paste Checklist

When creating a new provider, copy these files from an existing provider:

### From ISO Provider (Simple, well-documented)
- [ ] `AssemblyResolver.cs` - Update managedAssemblies list
- [ ] `app.config` - Update version numbers
- [ ] `BigDriveTraceSource.cs` - Change source name
- [ ] `ComStream.cs` - Use as-is
- [ ] `ProviderConfigurationFactory.cs` - Update provider name

### From Your Chosen Template
- [ ] `Provider.cs` - Update GUID, interfaces, helpers
- [ ] `Provider.IProcessInitializer.cs` - Modify Startup/Shutdown
- [ ] `Provider.IBigDriveRegistration.cs` - Update app name
- [ ] `Provider.IBigDriveDriveInfo.cs` - Define your parameters
- [ ] `Provider.IBigDriveEnumerate.cs` - Implement your logic

### Customize
- [ ] `{Service}ClientWrapper.cs` - Implement your API/SDK wrapper
- [ ] `.csproj` - Add your NuGet packages

---

## See Also

- [Getting Started](getting-started.md) - Project setup guide
- [Interfaces Reference](interfaces.md) - Interface definitions
- [NuGet Dependencies](nuget-dependencies.md) - AssemblyResolver setup
- [OAuth Authentication](oauth-authentication.md) - OAuth implementation
- [Troubleshooting](troubleshooting.md) - Common errors

---

## Real Provider Source Code

Study these complete, working providers:

- `src/BigDrive.Provider.Iso/` - Simplest read-only provider
- `src/BigDrive.Provider.Archive/` - Multi-format archive support
- `src/BigDrive.Provider.Zip/` - Read-write archive provider
- `src/BigDrive.Provider.Flickr/` - Complex OAuth 1.0a cloud provider
