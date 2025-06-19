# Installation Architecture

## Installation Process

### BigDrive.Setup.exe

When running `BigDrive.Setup.exe` with elevated permissions (as a local Administrator), the following steps are performed:

1. **Create Installer User**  
   - A local user named `BigDriveInstaller` is created with a temporary password.
   - The `BigDriveInstaller` user is granted permissions to register registry keys, enabling it to add or remove shell folders later.
2. **Event Log Creation**  
   - The BigDrive Event Log is created.
3. **COM+ Application Installation**  
   - A COM+ application named `BigDrive.Server` is installed and configured to run under the `BigDriveInstaller` user account.
4. **Password Handling**  
   - The password for the `BigDriveInstaller` user is discarded after setup.
5. **Context Menu Extension**  
   - A context menu extension (`BigDrive.Extension.dll`) is installed in "This PC" (formerly "My Computer"), allowing local users to invoke the COM+ BigDrive Service to register drives.


#### Notes

- **Administrator Rights:**  
  Elevated permissions are required only for the initial installation, which must be performed by a local administrator. After `BigDrive.Setup.exe` completes, drive registration can be performed by standard users without administrator rights, as the `BigDriveInstaller` account is used for these operations.

- **Idempotency:**  
  `BigDrive.Setup.exe` is idempotent. It can be run multiple times without causing issues. Each execution creates a new `BigDriveInstaller` user and removes any previous instance.

## Drive Registration

After installation, local users can register drives using the BigDrive context menu extension:

1. **Drive Registration via Context Menu**  
   - The user, running under their own (non-elevated) permissions, uses the context menu extension to register a drive.
2. **COM+ Server Communication**  
   - The extension calls the `BigDrive.Server` COM+ application to perform the registration.
3. **Registry Update**  
   - The server writes the necessary registry keys to make the drive visible in "This PC".
4. **Shell Notification**  
   - The Windows Shell is notified so the new drive appears in "This PC".

---

### Summary

- **Initial installation** requires administrator rights and sets up all necessary components and permissions.
- **Drive registration** can be performed by standard users without elevation, thanks to the `BigDriveInstaller` service account and COM+ server.
