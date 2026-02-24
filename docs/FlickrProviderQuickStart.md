# Flickr Provider Quick Start Guide

This guide walks you through setting up and using the Flickr Provider with BigDrive Shell to browse, download, and upload photos to your Flickr account.

## Overview

The Flickr Provider maps your Flickr account to a virtual file system:

| Flickr Concept | Virtual File System |
|----------------|---------------------|
| Photosets (Albums) | Folders |
| Photos | `.jpg` files |

```
Flickr Photos (Z:)
├── Vacation 2024\
│   ├── Beach Sunset.jpg
│   └── Mountain View.jpg
├── Family Photos\
│   └── Birthday Party.jpg
└── Nature\
    ├── Forest Trail.jpg
    └── Waterfall.jpg
```

---

## Prerequisites

1. **BigDrive installed** — Run `BigDrive.Setup.exe` as Administrator
2. **Flickr account** — You need a Flickr account with API access
3. **Flickr API credentials** — Obtain from [Flickr App Garden](https://www.flickr.com/services/apps/create/)

---

## Step 1: Obtain Flickr API Credentials

1. Go to [Flickr App Garden](https://www.flickr.com/services/apps/create/apply/)
2. Click **Apply for a Non-Commercial Key** (or Commercial if applicable)
3. Fill in the application details:
   - **What is the name of your app?** `BigDrive`
   - **What are you building?** `A file system integration for browsing Flickr photos`
4. Accept the terms and submit
5. Copy your **API Key** and **API Secret** — you'll need these after launching the shell

---

## Step 2: Launch BigDrive Shell

Open PowerShell or Command Prompt:

```powershell
cd "C:\Program Files\BigDrive"
.\BigDrive.Shell.exe
```

You'll see the BigDrive Shell prompt:

```
BigDrive Shell v1.0
Type 'help' for available commands, 'exit' to quit.

BD>
```

---

## Step 3: Verify the Flickr Provider is Registered

Use the `providers` command to verify the Flickr provider is installed:

```
BD> providers

 Registered Providers:

    BigDrive.Provider.Flickr
    CLSID: {B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}

    1 Provider(s)
```

If you don't see the Flickr provider, re-run the BigDrive installer.

---

## Step 4: Switch to the Flickr Drive

Use `drives` to see available drives, then switch to the Flickr drive:

```
BD> drives

BigDrive drives:

  Z:  Flickr Photos

BD> cd Z:

Z:\>
```

---

## Step 5: Configure Your Flickr API Credentials

Use the `secret` command to securely store your Flickr API credentials. These are stored encrypted in Windows Credential Manager.

```
Z:\> secret set FlickrApiKey
Enter value for 'FlickrApiKey': ********************************
Secret 'FlickrApiKey' saved.

Z:\> secret set FlickrApiSecret
Enter value for 'FlickrApiSecret': ********************************
Secret 'FlickrApiSecret' saved.
```

Verify your secrets are configured:

```
Z:\> secret list

 Secrets for current drive:

    FlickrApiKey
    FlickrApiSecret

    2 secret(s) configured.
```

> **Note:** For full read/write access (upload, delete), you'll also need OAuth tokens (`FlickrOAuthToken`, `FlickrOAuthSecret`). See the [Provider Development Guide](ProviderDevelopmentGuide.md) for OAuth setup.

---

## Step 6: Browse Your Photosets

List your Flickr albums (photosets):

```
Z:\> dir

 Directory of Z:\

    <DIR>    Vacation 2024
    <DIR>    Family Reunion
    <DIR>    Nature Photography

       3 Dir(s)    0 File(s)
```

---

## Step 7: View Photos in an Album

Navigate into an album and list photos:

```
Z:\> cd "Vacation 2024"

Z:\Vacation 2024> dir

 Directory of Z:\Vacation 2024

             Beach Sunset.jpg
             Mountain View.jpg
             Hotel Pool.jpg

       0 Dir(s)    3 File(s)
```

---

## Step 8: Download a Photo

Copy a photo from Flickr to your local drive:

```
Z:\Vacation 2024> copy "Beach Sunset.jpg" C:\Users\YourName\Downloads\beach.jpg
        1 file(s) copied.
```

---

## Step 9: Upload a Photo

Copy a local photo to a Flickr album:

```
Z:\Vacation 2024> copy C:\Users\YourName\Pictures\NewPhoto.jpg "New Photo.jpg"
        1 file(s) copied.
```

> **Note:** Upload requires OAuth authentication with write permissions.

---

## Command Reference

| Command | Example | Description |
|---------|---------|-------------|
| `providers` | `providers` | List all registered providers |
| `drives` | `drives` | List all BigDrive drives |
| `cd X:` | `cd Z:` | Switch to a drive |
| `cd folder` | `cd "Vacation 2024"` | Enter a folder (album) |
| `cd ..` | `cd ..` | Go up one level |
| `dir` | `dir` | List current directory contents |
| `secret set` | `secret set FlickrApiKey` | Set a secret (prompts for value) |
| `secret list` | `secret list` | List secrets and their status |
| `secret exists` | `secret exists FlickrApiKey` | Check if a secret exists |
| `secret del` | `secret del FlickrApiKey` | Delete a secret |
| `copy src dst` | `copy photo.jpg C:\Downloads\` | Download a photo |
| `copy src dst` | `copy C:\pic.jpg "My Photo.jpg"` | Upload a photo |
| `del file` | `del "Old Photo.jpg"` | Delete a photo from Flickr |
| `mkdir name` | `mkdir "New Album"` | Create a new photoset |
| `exit` | `exit` | Exit BigDrive Shell |

---

## Troubleshooting

### "No drives found"

- Verify the provider is registered: Run `providers` command
- Verify a drive is configured: Check `HKCU:\SOFTWARE\BigDrive\Drives`

### "Failed to connect to Flickr"

- Verify your API Key and Secret are configured: Run `secret list`
- Check your internet connection
- Ensure your Flickr API key hasn't been revoked

### "Permission denied" on upload/delete

- Upload and delete require OAuth authentication
- Verify `FlickrOAuthToken` and `FlickrOAuthSecret` are configured
- Regenerate OAuth tokens if expired

### Photos not appearing

- The provider caches photoset data for 5 minutes
- Exit and restart BigDrive Shell to clear the cache
- Verify photos are in a photoset (unorganized photos may not appear)

---

## Security Notes

- API credentials are stored securely in **Windows Credential Manager**
- Credentials are encrypted and scoped to the current Windows user
- OAuth tokens provide delegated access without storing your Flickr password
- See [Security Architecture](architecture/security.md) for details

---

## See Also

- [BigDrive Shell User Guide](BigDrive.Shell.UserGuide.md) — Full shell command reference
- [Provider Development Guide](ProviderDevelopmentGuide.md) — Creating custom providers
- [Architecture Overview](architecture/overview.md) — System architecture
- [Security](architecture/security.md) — Credential storage and security
