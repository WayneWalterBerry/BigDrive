// <copyright file="FlickrClientWrapper.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Flickr
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Net;
    using FlickrNet;

    /// <summary>
    /// Wrapper for the FlickrNet SDK to provide Flickr API functionality.
    /// </summary>
    internal class FlickrClientWrapper
    {
        /// <summary>
        /// The Flickr API client.
        /// </summary>
        private readonly Flickr _flickr;

        /// <summary>
        /// Cache of photosets to avoid repeated API calls.
        /// </summary>
        private List<PhotosetInfo> _photosetCache;

        /// <summary>
        /// Cache expiration time.
        /// </summary>
        private DateTime _cacheExpiration = DateTime.MinValue;

        /// <summary>
        /// Cache duration in minutes.
        /// </summary>
        private const int CacheDurationMinutes = 5;

        /// <summary>
        /// Initializes a new instance of the <see cref="FlickrClientWrapper"/> class.
        /// </summary>
        public FlickrClientWrapper()
        {
            // TODO: Load API key and secret from configuration
            // For now, use placeholder values that should be replaced with actual credentials
            string apiKey = GetConfigValue("FlickrApiKey", string.Empty);
            string apiSecret = GetConfigValue("FlickrApiSecret", string.Empty);

            _flickr = new Flickr(apiKey, apiSecret);

            // TODO: Implement OAuth authentication for user-specific operations
            // string authToken = GetConfigValue("FlickrAuthToken", string.Empty);
            // if (!string.IsNullOrEmpty(authToken))
            // {
            //     _flickr.OAuthAccessToken = authToken;
            // }
        }

        /// <summary>
        /// Gets all photosets (albums) for the authenticated user.
        /// </summary>
        /// <returns>A list of photoset information.</returns>
        public List<PhotosetInfo> GetPhotosets()
        {
            if (_photosetCache != null && DateTime.Now < _cacheExpiration)
            {
                return _photosetCache;
            }

            try
            {
                var photosets = _flickr.PhotosetsGetList();
                _photosetCache = photosets.Select(ps => new PhotosetInfo
                {
                    Id = ps.PhotosetId,
                    Title = ps.Title,
                    Description = ps.Description,
                    PhotoCount = ps.NumberOfPhotos
                }).ToList();

                _cacheExpiration = DateTime.Now.AddMinutes(CacheDurationMinutes);
                return _photosetCache;
            }
            catch (Exception)
            {
                return new List<PhotosetInfo>();
            }
        }

        /// <summary>
        /// Gets all photos in a photoset.
        /// </summary>
        /// <param name="photosetName">The name of the photoset.</param>
        /// <returns>A list of photo information.</returns>
        public List<PhotoInfo> GetPhotosInPhotoset(string photosetName)
        {
            try
            {
                var photoset = GetPhotosetByName(photosetName);
                if (photoset == null)
                {
                    return new List<PhotoInfo>();
                }

                var photos = _flickr.PhotosetsGetPhotos(photoset.Id);
                return photos.Select(p => new PhotoInfo
                {
                    Id = p.PhotoId,
                    Title = p.Title,
                    DateUploaded = p.DateUploaded,
                    Url = p.LargeUrl ?? p.MediumUrl ?? p.SmallUrl
                }).ToList();
            }
            catch (Exception)
            {
                return new List<PhotoInfo>();
            }
        }

        /// <summary>
        /// Gets detailed information about a specific photo.
        /// </summary>
        /// <param name="photosetName">The name of the photoset.</param>
        /// <param name="photoName">The name of the photo.</param>
        /// <returns>Photo information, or null if not found.</returns>
        public PhotoInfo GetPhotoInfo(string photosetName, string photoName)
        {
            var photos = GetPhotosInPhotoset(photosetName);
            return photos.FirstOrDefault(p => 
                SanitizeName(p.Title).Equals(photoName, StringComparison.OrdinalIgnoreCase));
        }

        /// <summary>
        /// Gets the raw photo data.
        /// </summary>
        /// <param name="photosetName">The name of the photoset.</param>
        /// <param name="photoName">The name of the photo.</param>
        /// <returns>The photo data as a byte array, or null if not found.</returns>
        public byte[] GetPhotoData(string photosetName, string photoName)
        {
            var photoInfo = GetPhotoInfo(photosetName, photoName);
            if (photoInfo == null || string.IsNullOrEmpty(photoInfo.Url))
            {
                return null;
            }

            try
            {
                using (var client = new WebClient())
                {
                    return client.DownloadData(photoInfo.Url);
                }
            }
            catch (Exception)
            {
                return null;
            }
        }

        /// <summary>
        /// Gets the URL for a photo.
        /// </summary>
        /// <param name="photosetName">The name of the photoset.</param>
        /// <param name="photoName">The name of the photo.</param>
        /// <returns>The photo URL, or null if not found.</returns>
        public string GetPhotoUrl(string photosetName, string photoName)
        {
            var photoInfo = GetPhotoInfo(photosetName, photoName);
            return photoInfo?.Url;
        }

        /// <summary>
        /// Uploads a photo to Flickr.
        /// </summary>
        /// <param name="localFilePath">The local file path.</param>
        /// <param name="title">The photo title.</param>
        /// <param name="photosetName">The target photoset name (optional).</param>
        public void UploadPhoto(string localFilePath, string title, string photosetName)
        {
            string photoId = _flickr.UploadPicture(localFilePath, title, string.Empty, string.Empty, true, false, false);

            if (!string.IsNullOrEmpty(photosetName))
            {
                var photoset = GetPhotosetByName(photosetName);
                if (photoset != null)
                {
                    _flickr.PhotosetsAddPhoto(photoset.Id, photoId);
                }
            }

            InvalidateCache();
        }

        /// <summary>
        /// Downloads a photo from Flickr.
        /// </summary>
        /// <param name="photosetName">The name of the photoset.</param>
        /// <param name="photoName">The name of the photo.</param>
        /// <param name="localTargetPath">The local target path.</param>
        public void DownloadPhoto(string photosetName, string photoName, string localTargetPath)
        {
            byte[] data = GetPhotoData(photosetName, photoName);
            if (data != null)
            {
                File.WriteAllBytes(localTargetPath, data);
            }
            else
            {
                throw new FileNotFoundException("Photo not found.", photoName);
            }
        }

        /// <summary>
        /// Deletes a photo from Flickr.
        /// </summary>
        /// <param name="photosetName">The name of the photoset.</param>
        /// <param name="photoName">The name of the photo.</param>
        public void DeletePhoto(string photosetName, string photoName)
        {
            var photoInfo = GetPhotoInfo(photosetName, photoName);
            if (photoInfo != null)
            {
                _flickr.PhotosDelete(photoInfo.Id);
                InvalidateCache();
            }
        }

        /// <summary>
        /// Deletes a photoset from Flickr.
        /// </summary>
        /// <param name="photosetName">The name of the photoset.</param>
        public void DeletePhotoset(string photosetName)
        {
            var photoset = GetPhotosetByName(photosetName);
            if (photoset != null)
            {
                _flickr.PhotosetsDelete(photoset.Id);
                InvalidateCache();
            }
        }

        /// <summary>
        /// Creates a new photoset.
        /// </summary>
        /// <param name="photosetName">The name of the new photoset.</param>
        public void CreatePhotoset(string photosetName)
        {
            // Flickr requires at least one photo to create a photoset
            // This is a limitation of the Flickr API
            throw new NotSupportedException("Flickr requires at least one photo to create a photoset. Upload a photo first.");
        }

        /// <summary>
        /// Moves a photo from one photoset to another.
        /// </summary>
        /// <param name="sourcePhotosetName">The source photoset name.</param>
        /// <param name="photoName">The photo name.</param>
        /// <param name="destPhotosetName">The destination photoset name.</param>
        public void MovePhoto(string sourcePhotosetName, string photoName, string destPhotosetName)
        {
            var photoInfo = GetPhotoInfo(sourcePhotosetName, photoName);
            var sourcePhotoset = GetPhotosetByName(sourcePhotosetName);
            var destPhotoset = GetPhotosetByName(destPhotosetName);

            if (photoInfo == null || sourcePhotoset == null || destPhotoset == null)
            {
                throw new FileNotFoundException("Photo or photoset not found.");
            }

            _flickr.PhotosetsAddPhoto(destPhotoset.Id, photoInfo.Id);
            _flickr.PhotosetsRemovePhoto(sourcePhotoset.Id, photoInfo.Id);
            InvalidateCache();
        }

        /// <summary>
        /// Gets a photoset by name.
        /// </summary>
        /// <param name="name">The photoset name.</param>
        /// <returns>The photoset info, or null if not found.</returns>
        private PhotosetInfo GetPhotosetByName(string name)
        {
            var photosets = GetPhotosets();
            return photosets.FirstOrDefault(ps => 
                SanitizeName(ps.Title).Equals(name, StringComparison.OrdinalIgnoreCase));
        }

        /// <summary>
        /// Invalidates the photoset cache.
        /// </summary>
        private void InvalidateCache()
        {
            _photosetCache = null;
            _cacheExpiration = DateTime.MinValue;
        }

        /// <summary>
        /// Sanitizes a name for comparison.
        /// </summary>
        /// <param name="name">The original name.</param>
        /// <returns>A sanitized name.</returns>
        private static string SanitizeName(string name)
        {
            if (string.IsNullOrEmpty(name))
            {
                return "Untitled";
            }

            char[] invalidChars = Path.GetInvalidFileNameChars();
            foreach (char c in invalidChars)
            {
                name = name.Replace(c, '_');
            }

            return name;
        }

        /// <summary>
        /// Gets a configuration value from the registry.
        /// </summary>
        /// <param name="key">The configuration key.</param>
        /// <param name="defaultValue">The default value if not found.</param>
        /// <returns>The configuration value.</returns>
        private static string GetConfigValue(string key, string defaultValue)
        {
            // Read from registry under HKLM\SOFTWARE\BigDrive\Providers\Flickr
            try
            {
                using (var regKey = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(@"SOFTWARE\BigDrive\Providers\Flickr"))
                {
                    if (regKey != null)
                    {
                        object value = regKey.GetValue(key);
                        if (value != null)
                        {
                            return value.ToString();
                        }
                    }
                }
            }
            catch
            {
                // Ignore registry errors and return default
            }

            return defaultValue;
        }
    }

    /// <summary>
    /// Represents information about a Flickr photoset.
    /// </summary>
    internal class PhotosetInfo
    {
        /// <summary>
        /// Gets or sets the photoset ID.
        /// </summary>
        public string Id { get; set; }

        /// <summary>
        /// Gets or sets the photoset title.
        /// </summary>
        public string Title { get; set; }

        /// <summary>
        /// Gets or sets the photoset description.
        /// </summary>
        public string Description { get; set; }

        /// <summary>
        /// Gets or sets the number of photos in the photoset.
        /// </summary>
        public int PhotoCount { get; set; }
    }

    /// <summary>
    /// Represents information about a Flickr photo.
    /// </summary>
    internal class PhotoInfo
    {
        /// <summary>
        /// Gets or sets the photo ID.
        /// </summary>
        public string Id { get; set; }

        /// <summary>
        /// Gets or sets the photo title.
        /// </summary>
        public string Title { get; set; }

        /// <summary>
        /// Gets or sets the date the photo was uploaded.
        /// </summary>
        public DateTime DateUploaded { get; set; }

        /// <summary>
        /// Gets or sets the photo URL.
        /// </summary>
        public string Url { get; set; }

        /// <summary>
        /// Gets or sets the file size in bytes.
        /// </summary>
        public ulong FileSize { get; set; }
    }
}
