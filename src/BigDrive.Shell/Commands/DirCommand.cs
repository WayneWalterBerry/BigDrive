// <copyright file="DirCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    using BigDrive.ConfigProvider.Model;
    using BigDrive.Interfaces;
    using BigDrive.Interfaces.Model;
    using BigDrive.Shell.FileStores;

    /// <summary>
    /// Lists drives at root, or files and folders when inside a drive.
    /// Supports wildcard patterns (* and ?) for filtering and various display switches.
    /// </summary>
    public class DirCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "dir"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "ls" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get
            {
                return "Lists drives or folder contents (supports wildcards: *, ?)\n" +
                       "  Switches:\n" +
                       "    -ad          Directories only\n" +
                       "    -af          Files only\n" +
                       "    -r           Recursive (include subdirectories)\n" +
                       "    -Name / -b   Bare format (names only, no headers)\n" +
                       "    -w           Wide format (multiple columns)\n" +
                       "    -o:n         Sort by name (A-Z)\n" +
                       "    -o:-n        Sort by name descending (Z-A)\n" +
                       "    -o:d         Sort by date (oldest first)\n" +
                       "    -o:s         Sort by size (smallest first)\n" +
                       "    -o:e         Sort by extension";
            }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get
            {
                return "dir [switches] [path]\n" +
                       "  Examples:\n" +
                       "    dir                    List current directory\n" +
                       "    dir -ad                List directories only\n" +
                       "    dir -af *.txt          List .txt files only\n" +
                       "    dir -r                 List recursively\n" +
                       "    dir -Name              Bare format (names only)\n" +
                       "    dir -w                 Wide format\n" +
                       "    dir -o:n               Sort by name\n" +
                       "    dir -o:s -af           Sort files by size";
            }
        }

        /// <summary>
        /// Executes the dir command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            // Parse switches and arguments
            CommandLineParser parser = new CommandLineParser(args);

            // Check if user provided a path with a drive letter
            if (parser.Arguments.Count > 0)
            {
                string inputPath = parser.Arguments[0];

                // Check for drive letter (e.g., "Y:" or "Y:\path")
                if (inputPath.Length >= 2 && inputPath[1] == ':')
                {
                    char letter = char.ToUpper(inputPath[0]);

                    if (letter >= 'A' && letter <= 'Z')
                    {
                        // Check if it's a BigDrive
                        if (context.DriveLetterManager.IsBigDrive(letter))
                        {
                            // List contents of the specified drive
                            ListDriveContents(context, parser, letter, inputPath);
                            return;
                        }
                        else if (context.DriveLetterManager.IsOSDrive(letter))
                        {
                            Console.WriteLine("Cannot access OS drives from BigDrive Shell.");
                            return;
                        }
                        else
                        {
                            Console.WriteLine("Drive not found: {0}:", letter);
                            Console.WriteLine("Use 'drives' to see available drives.");
                            return;
                        }
                    }
                }
            }

            // If no drive selected, show available drives (like root of file system)
            if (context.CurrentDriveLetter == '\0')
            {
                ListDrives(context);
                return;
            }

            // Inside a drive - show folder contents
            ListFolderContents(context, parser);
        }

        /// <summary>
        /// Lists available BigDrive drives (shown at root level).
        /// </summary>
        /// <param name="context">The shell context.</param>
        private static void ListDrives(ShellContext context)
        {
            DriveLetterManager driveManager = context.DriveLetterManager;
            IReadOnlyDictionary<char, DriveConfiguration> bigDrives = driveManager.BigDriveLetters;

            Console.WriteLine();
            Console.WriteLine(" Directory of BigDrive");
            Console.WriteLine();

            if (bigDrives.Count > 0)
            {
                // Sort by drive letter descending (Z, Y, X...)
                List<char> sortedLetters = bigDrives.Keys.OrderByDescending(c => c).ToList();

                foreach (char letter in sortedLetters)
                {
                    DriveConfiguration config = bigDrives[letter];
                    Console.WriteLine("    <DRIVE>  {0}:  {1}", letter, config.Name);
                }

                Console.WriteLine();
                Console.WriteLine("       {0} Drive(s)", bigDrives.Count);
                Console.WriteLine();
                Console.WriteLine("Use 'cd X:' to enter a drive.");
            }
            else
            {
                Console.WriteLine("    No BigDrive drives registered.");
                Console.WriteLine();
                Console.WriteLine("Run BigDrive.Setup.exe to register providers.");
            }
        }

        /// <summary>
        /// Lists contents of a specific drive when user provides a drive letter argument (e.g., "dir Y:\").
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="parser">Parsed command-line arguments and switches.</param>
        /// <param name="driveLetter">The drive letter to list.</param>
        /// <param name="inputPath">The full input path (e.g., "Y:" or "Y:\folder").</param>
        private static void ListDriveContents(ShellContext context, CommandLineParser parser, char driveLetter, string inputPath)
        {
            // Get the drive configuration
            DriveConfiguration driveConfig = context.DriveLetterManager.BigDriveLetters[driveLetter];
            if (driveConfig == null)
            {
                Console.WriteLine("Drive not found: {0}:", driveLetter);
                return;
            }

            // Create a temporary context for this drive
            char originalDrive = context.CurrentDriveLetter;
            string originalPath = context.CurrentPath;

            try
            {
                // Temporarily switch to the specified drive
                context.ChangeDrive(driveLetter);

                // Extract the path part after the drive letter (if any)
                string pathPart = "\\";
                if (inputPath.Length > 2)
                {
                    pathPart = inputPath.Substring(2);
                    if (string.IsNullOrEmpty(pathPart))
                    {
                        pathPart = "\\";
                    }
                }

                // Update the first argument to be the path part (without drive letter)
                if (!string.IsNullOrEmpty(pathPart) && pathPart != "\\")
                {
                    // Create a new parser with the path part
                    List<string> newArgs = new List<string>();

                    // Add the path part
                    newArgs.Add(pathPart);

                    // Add remaining arguments (skip the first one which was the drive path)
                    for (int i = 1; i < parser.Arguments.Count; i++)
                    {
                        newArgs.Add(parser.Arguments[i]);
                    }

                    // Add switches
                    foreach (string switchStr in parser.Switches)
                    {
                        newArgs.Add("-" + switchStr);
                    }

                    parser = new CommandLineParser(newArgs.ToArray());
                }
                else
                {
                    // Just list the root of the drive, keep switches
                    List<string> newArgs = new List<string>();
                    foreach (string switchStr in parser.Switches)
                    {
                        newArgs.Add("-" + switchStr);
                    }

                    parser = new CommandLineParser(newArgs.ToArray());
                }

                // List the folder contents
                ListFolderContents(context, parser);
            }
            finally
            {
                // Restore original context
                if (originalDrive != '\0')
                {
                    context.ChangeDrive(originalDrive);
                    context.CurrentPath = originalPath;
                }
                else
                {
                    // Was not on any drive, clear context
                    context.CurrentDriveLetter = '\0';
                    context.CurrentPath = "\\";
                }
            }
        }

        /// <summary>
        /// Lists folder contents when inside a drive. Supports wildcard patterns and switches.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="parser">Parsed command-line arguments and switches.</param>
        private static void ListFolderContents(ShellContext context, CommandLineParser parser)
        {
            if (!context.CurrentDriveGuid.HasValue)
            {
                Console.WriteLine("Current drive is not a BigDrive.");
                return;
            }

            // Check for switches
            bool directoriesOnly = parser.HasSwitch("ad", "Directory");
            bool filesOnly = parser.HasSwitch("af", "File");
            bool recursive = parser.HasSwitch("r", "Recurse");
            bool bareFormat = parser.HasSwitch("Name", "b");
            bool wideFormat = parser.HasSwitch("w");
            string sortMode = parser.GetSwitchValue("o");

            // Resolve path and pattern
            string path = context.CurrentPath;
            string filePattern = null;

            if (parser.Arguments.Count > 0)
            {
                string inputPath = parser.Arguments[0];

                // Check if input contains a wildcard
                if (WildcardMatcher.ContainsWildcard(inputPath))
                {
                    // Split into directory and pattern
                    WildcardMatcher.SplitPathAndPattern(inputPath, out string dirPart, out filePattern);

                    // Resolve the directory part
                    if (dirPart == "\\" || string.IsNullOrEmpty(dirPart))
                    {
                        path = context.CurrentPath;
                    }
                    else
                    {
                        path = PathInfo.ResolvePath(context.CurrentPath, dirPart);
                    }

                    ShellTrace.Verbose("Wildcard dir: path=\"{0}\", pattern=\"{1}\"", path, filePattern);
                }
                else
                {
                    path = PathInfo.ResolvePath(context.CurrentPath, inputPath);
                }
            }

            IBigDriveEnumerate enumerate = ProviderFactory.GetEnumerateProvider(context.CurrentDriveGuid.Value);
            if (enumerate == null)
            {
                Console.WriteLine("Provider does not support enumeration.");
                return;
            }

            // Query provider capabilities to determine which metadata columns to show.
            // Uses IBigDriveCapabilities (separate interface with its own IID) so
            // QueryInterface returns null cleanly for providers that don't implement it.
            // Providers without IBigDriveCapabilities are assumed to support all metadata.
            FileInfoCapabilities capabilities = QueryFileInfoCapabilities(context.CurrentDriveGuid.Value);

            // Collect all entries
            List<DirEntry> allEntries = new List<DirEntry>();

            if (recursive)
            {
                CollectEntriesRecursive(context.CurrentDriveGuid.Value, path, filePattern, enumerate, allEntries, directoriesOnly, filesOnly, path, capabilities);
            }
            else
            {
                CollectEntries(context.CurrentDriveGuid.Value, path, filePattern, enumerate, allEntries, directoriesOnly, filesOnly, capabilities);
            }

            // Sort entries
            SortEntries(allEntries, sortMode);

            // Display entries
            if (bareFormat)
            {
                DisplayBareFormat(allEntries);
            }
            else if (wideFormat)
            {
                DisplayWideFormat(context, path, filePattern, allEntries);
            }
            else
            {
                DisplayStandardFormat(context, path, filePattern, allEntries, capabilities);
            }
        }

        /// <summary>
        /// Represents a directory entry (file or folder).
        /// </summary>
        private class DirEntry
        {
            public string Name { get; set; }
            public string FullPath { get; set; }
            public bool IsDirectory { get; set; }
            public long Size { get; set; }
            public DateTime LastModified { get; set; }
        }

        /// <summary>
        /// Queries the provider's file-info capabilities via the
        /// <see cref="IBigDriveCapabilities"/> COM interface.
        /// Returns <see cref="FileInfoCapabilities.All"/> when the provider does not
        /// implement the interface (backward compatible with older providers).
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The provider's capabilities, or <see cref="FileInfoCapabilities.All"/> if not supported.</returns>
        private static FileInfoCapabilities QueryFileInfoCapabilities(Guid driveGuid)
        {
            try
            {
                IBigDriveCapabilities caps = ProviderFactory.GetCapabilitiesProvider(driveGuid);
                if (caps != null)
                {
                    int raw = caps.GetFileInfoCapabilities(driveGuid);
                    FileInfoCapabilities result = (FileInfoCapabilities)raw;
                    ShellTrace.Verbose("GetFileInfoCapabilities returned: {0} (raw={1})", result, raw);
                    return result;
                }
                else
                {
                    ShellTrace.Verbose("GetFileInfoCapabilities: provider does not implement IBigDriveCapabilities, defaulting to All");
                }
            }
            catch (Exception ex)
            {
                ShellTrace.Warning("GetFileInfoCapabilities failed ({0}): {1}", ex.GetType().Name, ex.Message);
            }

            return FileInfoCapabilities.All;
        }

        /// <summary>
        /// Collects entries from a single directory.
        /// Only queries metadata the provider advertises via <paramref name="capabilities"/>.
        /// </summary>
        private static void CollectEntries(Guid driveGuid, string path, string filePattern, IBigDriveEnumerate enumerate, List<DirEntry> entries, bool directoriesOnly, bool filesOnly, FileInfoCapabilities capabilities)
        {
            // Get provider for file info (optional)
            IBigDriveFileInfo fileInfo = ProviderFactory.GetFileInfoProvider(driveGuid);

            bool querySize = fileInfo != null && capabilities.HasFlag(FileInfoCapabilities.FileSize);
            bool queryDate = fileInfo != null && capabilities.HasFlag(FileInfoCapabilities.LastModified);

            // Collect folders
            if (!filesOnly)
            {
                string[] folders = enumerate.EnumerateFolders(driveGuid, path);
                IEnumerable<string> filteredFolders = filePattern == null ? folders : WildcardMatcher.Filter(folders, filePattern);

                foreach (string folder in filteredFolders)
                {
                    string fullPath = FileTransferService.CombinePath(path, folder);
                    entries.Add(new DirEntry
                    {
                        Name = folder,
                        FullPath = fullPath,
                        IsDirectory = true,
                        Size = 0,
                        LastModified = DateTime.MinValue
                    });
                }
            }

            // Collect files
            if (!directoriesOnly)
            {
                string[] files = enumerate.EnumerateFiles(driveGuid, path);
                IEnumerable<string> filteredFiles = filePattern == null ? files : WildcardMatcher.Filter(files, filePattern);

                foreach (string file in filteredFiles)
                {
                    string fullPath = FileTransferService.CombinePath(path, file);

                    long size = 0;
                    DateTime lastModified = DateTime.MinValue;

                    try
                    {
                        if (querySize)
                        {
                            ulong fileSize = fileInfo.GetFileSize(driveGuid, fullPath);
                            size = fileSize > long.MaxValue ? long.MaxValue : (long)fileSize;
                        }

                        if (queryDate)
                        {
                            lastModified = fileInfo.LastModifiedTime(driveGuid, fullPath);
                        }
                    }
                    catch
                    {
                        // Ignore errors - provider may not support all operations
                    }

                    entries.Add(new DirEntry
                    {
                        Name = file,
                        FullPath = fullPath,
                        IsDirectory = false,
                        Size = size,
                        LastModified = lastModified
                    });
                }
            }
        }

        /// <summary>
        /// Collects entries recursively from a directory tree.
        /// </summary>
        private static void CollectEntriesRecursive(Guid driveGuid, string path, string filePattern, IBigDriveEnumerate enumerate, List<DirEntry> entries, bool directoriesOnly, bool filesOnly, string basePath, FileInfoCapabilities capabilities)
        {
            // Collect entries in current directory
            CollectEntries(driveGuid, path, filePattern, enumerate, entries, directoriesOnly, filesOnly, capabilities);

            // Recurse into subdirectories
            string[] folders = enumerate.EnumerateFolders(driveGuid, path);
            foreach (string folder in folders)
            {
                string fullPath = FileTransferService.CombinePath(path, folder);
                CollectEntriesRecursive(driveGuid, fullPath, filePattern, enumerate, entries, directoriesOnly, filesOnly, basePath, capabilities);
            }
        }

        /// <summary>
        /// Sorts entries based on sort mode.
        /// </summary>
        private static void SortEntries(List<DirEntry> entries, string sortMode)
        {
            if (string.IsNullOrEmpty(sortMode))
            {
                return;
            }

            bool descending = sortMode.StartsWith("-");
            string mode = descending ? sortMode.Substring(1) : sortMode;

            switch (mode.ToLowerInvariant())
            {
                case "n": // Sort by name
                    if (descending)
                    {
                        entries.Sort((a, b) => string.Compare(b.Name, a.Name, StringComparison.OrdinalIgnoreCase));
                    }
                    else
                    {
                        entries.Sort((a, b) => string.Compare(a.Name, b.Name, StringComparison.OrdinalIgnoreCase));
                    }
                    break;

                case "d": // Sort by date
                    if (descending)
                    {
                        entries.Sort((a, b) => b.LastModified.CompareTo(a.LastModified));
                    }
                    else
                    {
                        entries.Sort((a, b) => a.LastModified.CompareTo(b.LastModified));
                    }
                    break;

                case "s": // Sort by size
                    if (descending)
                    {
                        entries.Sort((a, b) => b.Size.CompareTo(a.Size));
                    }
                    else
                    {
                        entries.Sort((a, b) => a.Size.CompareTo(b.Size));
                    }
                    break;

                case "e": // Sort by extension
                    if (descending)
                    {
                        entries.Sort((a, b) => string.Compare(GetExtension(b.Name), GetExtension(a.Name), StringComparison.OrdinalIgnoreCase));
                    }
                    else
                    {
                        entries.Sort((a, b) => string.Compare(GetExtension(a.Name), GetExtension(b.Name), StringComparison.OrdinalIgnoreCase));
                    }
                    break;
            }
        }

        /// <summary>
        /// Gets the file extension from a filename.
        /// </summary>
        private static string GetExtension(string filename)
        {
            int dotIndex = filename.LastIndexOf('.');
            return dotIndex >= 0 ? filename.Substring(dotIndex) : string.Empty;
        }

        /// <summary>
        /// Displays entries in bare format (names only).
        /// </summary>
        private static void DisplayBareFormat(List<DirEntry> entries)
        {
            foreach (DirEntry entry in entries)
            {
                Console.WriteLine(entry.Name);
            }
        }

        /// <summary>
        /// Displays entries in wide format (multiple columns).
        /// </summary>
        private static void DisplayWideFormat(ShellContext context, string path, string filePattern, List<DirEntry> entries)
        {
            Console.WriteLine();
            if (filePattern != null)
            {
                Console.WriteLine(" Directory of {0}:{1}  (filter: {2})", context.CurrentDriveLetter, path, filePattern);
            }
            else
            {
                Console.WriteLine(" Directory of {0}:{1}", context.CurrentDriveLetter, path);
            }
            Console.WriteLine();

            // Calculate column width
            int maxNameLength = entries.Count > 0 ? entries.Max(e => e.Name.Length) : 0;
            int columnWidth = Math.Max(maxNameLength + 4, 20);
            int consoleWidth = 80; // Default console width
            try
            {
                consoleWidth = Console.WindowWidth;
            }
            catch
            {
                // Ignore - use default
            }

            int columnsPerRow = Math.Max(1, consoleWidth / columnWidth);
            int column = 0;

            foreach (DirEntry entry in entries)
            {
                string displayName = entry.IsDirectory ? "[" + entry.Name + "]" : entry.Name;
                Console.Write(displayName.PadRight(columnWidth));

                column++;
                if (column >= columnsPerRow)
                {
                    Console.WriteLine();
                    column = 0;
                }
            }

            if (column > 0)
            {
                Console.WriteLine();
            }

            Console.WriteLine();

            int dirCount = entries.Count(e => e.IsDirectory);
            int fileCount = entries.Count(e => !e.IsDirectory);
            Console.WriteLine("       {0} Dir(s)    {1} File(s)", dirCount, fileCount);
        }

        /// <summary>
        /// Format string when all columns are shown: Mode, LastWriteTime, Length, Name.
        /// Matches PowerShell's Get-ChildItem layout.
        /// </summary>
        private const string FormatAll = "{0,-5}{1,29}{2,15} {3}";

        /// <summary>
        /// Format string when only date is shown (no Length column): Mode, LastWriteTime, Name.
        /// </summary>
        private const string FormatDateOnly = "{0,-5}{1,29} {2}";

        /// <summary>
        /// Format string when only size is shown (no LastWriteTime column): Mode, Length, Name.
        /// </summary>
        private const string FormatSizeOnly = "{0,-5}{1,15} {2}";

        /// <summary>
        /// Format string when no metadata columns are shown: Mode, Name.
        /// </summary>
        private const string FormatNone = "{0,-5} {1}";

        /// <summary>
        /// Displays entries in standard format matching PowerShell's dir output.
        /// Dynamically hides columns the provider cannot populate based on
        /// <paramref name="capabilities"/>.
        /// </summary>
        private static void DisplayStandardFormat(ShellContext context, string path, string filePattern, List<DirEntry> entries, FileInfoCapabilities capabilities)
        {
            bool showDate = capabilities.HasFlag(FileInfoCapabilities.LastModified);
            bool showSize = capabilities.HasFlag(FileInfoCapabilities.FileSize);

            Console.WriteLine();
            if (filePattern != null)
            {
                Console.WriteLine("    Directory: {0}:{1}  (filter: {2})", context.CurrentDriveLetter, path, filePattern);
            }
            else
            {
                Console.WriteLine("    Directory: {0}:{1}", context.CurrentDriveLetter, path);
            }

            Console.WriteLine();

            // Write header and separator based on which columns are visible
            if (showDate && showSize)
            {
                Console.WriteLine(FormatAll, "Mode", "LastWriteTime", "Length", "Name");
                Console.WriteLine(FormatAll, "----", "-------------", "------", "----");
            }
            else if (showDate)
            {
                Console.WriteLine(FormatDateOnly, "Mode", "LastWriteTime", "Name");
                Console.WriteLine(FormatDateOnly, "----", "-------------", "----");
            }
            else if (showSize)
            {
                Console.WriteLine(FormatSizeOnly, "Mode", "Length", "Name");
                Console.WriteLine(FormatSizeOnly, "----", "------", "----");
            }
            else
            {
                Console.WriteLine(FormatNone, "Mode", "Name");
                Console.WriteLine(FormatNone, "----", "----");
            }

            long totalBytes = 0;

            foreach (DirEntry entry in entries)
            {
                string mode = entry.IsDirectory ? "d----" : "-a---";

                if (showDate && showSize)
                {
                    string dateStr = FormatLastWriteTime(entry.LastModified);
                    string sizeStr = entry.IsDirectory ? string.Empty : entry.Size.ToString();
                    Console.WriteLine(FormatAll, mode, dateStr, sizeStr, entry.Name);
                }
                else if (showDate)
                {
                    string dateStr = FormatLastWriteTime(entry.LastModified);
                    Console.WriteLine(FormatDateOnly, mode, dateStr, entry.Name);
                }
                else if (showSize)
                {
                    string sizeStr = entry.IsDirectory ? string.Empty : entry.Size.ToString();
                    Console.WriteLine(FormatSizeOnly, mode, sizeStr, entry.Name);
                }
                else
                {
                    Console.WriteLine(FormatNone, mode, entry.Name);
                }

                if (!entry.IsDirectory)
                {
                    totalBytes += entry.Size;
                }
            }

            Console.WriteLine();

            int fileCount = entries.Count(e => !e.IsDirectory);
            int dirCount = entries.Count(e => e.IsDirectory);

            if (showSize)
            {
                Console.WriteLine("{0,16} File(s) {1,14} bytes", fileCount, totalBytes.ToString("N0"));
            }
            else
            {
                Console.WriteLine("{0,16} File(s)", fileCount);
            }

            Console.WriteLine("{0,16} Dir(s)", dirCount);
        }

        /// <summary>
        /// Formats a DateTime in the same style as PowerShell's Get-ChildItem LastWriteTime column.
        /// Uses right-aligned short date (10 chars) + two spaces + right-aligned short time (8 chars).
        /// </summary>
        /// <param name="dateTime">The date and time to format.</param>
        /// <returns>A 20-character formatted date/time string, or empty if the date is not set.</returns>
        private static string FormatLastWriteTime(DateTime dateTime)
        {
            if (dateTime == DateTime.MinValue)
            {
                return string.Empty;
            }

            string datePart = dateTime.ToString("d");
            string timePart = dateTime.ToString("t");
            return string.Format("{0,10}  {1,8}", datePart, timePart);
        }

            }
        }
