# BigDrive

BigDrive is a modular, cross-language framework for building, configuring, and managing virtual drive providers and shell folder integrations on Windows. It is designed to support the development and registration of custom drive providers, facilitate configuration management, and enable integration with Windows shell extensions.

## Features
- **Provider Framework:** Create and register custom drive providers using a unified interface.
- **Configuration Management:** Manage provider and drive configurations, including parsing and storing settings in JSON.
- **Shell Integration:** Support for implementing IShellFolder and related interfaces for Windows Explorer integration.
- **Event Logging:** Utilities for writing to the Windows Event Viewer for diagnostics and monitoring.
- **Extensive Unit Tests:** Comprehensive test coverage for all major components.

## Architecture Overview
Windows Shell Folder Extensions are hard to program (7.5/10) for a variety of reasons:
1. They need to be implemented in C++, the shell folder extension is hosted in explorer.exe and you don't want to host C# and the CLR assemblies in this process space.
2. Shell Folder Extensions predate widespread documentation on the internet. These extensions have been part of Windows Shell architecture since early versions of Windows 95, when Microsoft introduced COM-based shell extensions to enhance Explorer functionality.
At that time, documentation was primarily available through Microsoft Developer Network (MSDN) CDs, printed manuals, and internal developer resources rather than online. The internet wasn’t as widely used for technical documentation as it is today, so early developers often relied on books, conferences, and direct Microsoft support to understand and implement Shell Folder Extensions.
3. The memory management for C++ interacting with the Shell Folder extensions needs to be perfect, explorer.exe stays running for days/weeks/months on end and doesn't recycle, leaking just a little memory will build up over time.
4. Access Violations can cause explorer.exe to crash, and the taskbar to disappear.  Which might require explorer.exe to restart.
5. Programming shell extension is about the implementation of a wide variety of COM interfaces, and clip board formats. 
   
To make any type of extensible shell folder extension that could reach a variety of virtual folder and files, would require that each implementation be hosted in explorer.exe loading a wide variety of SDK, and clients, which might have conflicting assemblies.  To solve this issue Big Drives Shell folder extension is a C++ shim that proxies the work of fetching the data to a hosted COM++ components.

COM+ components are hosted outside of the explorer.exe process space, when they crash, explorer.exe doesn’t crash.  They are hosted locally on Windows and can interact with users, allowing them to prompt security credentials.  COM+ components can be written in any language that supports COM on Windows.
To extend the functionality of BigDrive, anyone can write a COM++ component and register it locally with Microsoft Transaction Server.  The COM+ component supporting the predefined interfaces from BigDrive, will be discovered by the Big Drive Shell Folder extension and made available as a hard drive in File Explorer under My PC in windows.

# Installation
1) With Elevated Permissions by a local Adminsitrator BigDrive.Setup.exe is excuted:
   * A local user called BigDriveInstaller is created (unimplemented)
   * The BigDriveInstaller user is given permissions to register keys, so that later it can add or remove shell folders. (unimplemented)
   * The BigDrive Event Log is created.
   * A COM+ application is installed called BigDrive.Server that is running under BigDriveInstaller.
2) A Context Menu Extension Call BigDrive XXX is installed in "My PC" that allows the local user to call the COM+ BigDrive Service to register Drives.
3) Using the BigDrive Context Menu Extension, local users running on their own permisons (not elevated) register drives via the Big Drive Server:
   * The Big Drive Server Write the Register Keys Nessecary to visualize the drive in "My PC"
   
## Project Structure
- `src/BigDrive.Client/` — Core C++ client library for interacting with providers and managing configurations.
- `src/BigDrive.Provider.Sample/` — Example C# provider implementation demonstrating integration with the framework.
- `src/BigDrive.Setup/` — Setup utilities for bootstrapping event logs and managing event sources.
- `src/ConfigProvider/` — Configuration provider logic and models.
- `src/DTC/` — Distributed Transaction Coordinator integration and service logic.
- `src/Interfaces/` — Shared interface definitions for providers and clients.
- `src/IShellFolder/` — Shell extension implementation for Windows Explorer integration.
- `src/Shared/` — Shared utilities and logging components.
- `test/` — Unit tests for all major modules.

## Getting Started
1. **Build the Solution:** Open `BigDrive.sln` in Visual Studio and build all projects.
2. **Run Setup:** Use the setup utilities in `src/BigDrive.Setup/` to register event sources and prepare the environment (requires administrator privileges).
3. **Develop Providers:** Use the sample provider as a template for building your own drive providers.
4. **Testing:** Run the unit tests in the `test/` directory to verify functionality.

## License
This project is licensed under the MIT License. See the `LICENSE` file for details.
