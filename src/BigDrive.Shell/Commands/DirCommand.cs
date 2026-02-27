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

            // Collect all entries
            List<DirEntry> allEntries = new List<DirEntry>();

            if (recursive)
            {
                CollectEntriesRecursive(context.CurrentDriveGuid.Value, path, filePattern, enumerate, allEntries, directoriesOnly, filesOnly, path);
            }
            else
            {
                CollectEntries(context.CurrentDriveGuid.Value, path, filePattern, enumerate, allEntries, directoriesOnly, filesOnly);
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
                DisplayStandardFormat(context, path, filePattern, allEntries);
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
        /// Collects entries from a single directory.
        /// </summary>
        private static void CollectEntries(Guid driveGuid, string path, string filePattern, IBigDriveEnumerate enumerate, List<DirEntry> entries, bool directoriesOnly, bool filesOnly)
        {
            // Get provider for file info (optional)
            IBigDriveFileInfo fileInfo = ProviderFactory.GetFileInfoProvider(driveGuid);

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

                    // Try to get file metadata
                    if (fileInfo != null)
                    {
                        try
                        {
                            ulong fileSize = fileInfo.GetFileSize(driveGuid, fullPath);
                            size = fileSize > long.MaxValue ? long.MaxValue : (long)fileSize;
                            lastModified = fileInfo.LastModifiedTime(driveGuid, fullPath);
                        }
                        catch
                        {
                            // Ignore errors - provider may not support all operations
                        }
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
        private static void CollectEntriesRecursive(Guid driveGuid, string path, string filePattern, IBigDriveEnumerate enumerate, List<DirEntry> entries, bool directoriesOnly, bool filesOnly, string basePath)
        {
            // Collect entries in current directory
            CollectEntries(driveGuid, path, filePattern, enumerate, entries, directoriesOnly, filesOnly);

            // Recurse into subdirectories
            string[] folders = enumerate.EnumerateFolders(driveGuid, path);
            foreach (string folder in folders)
            {
                string fullPath = FileTransferService.CombinePath(path, folder);
                CollectEntriesRecursive(driveGuid, fullPath, filePattern, enumerate, entries, directoriesOnly, filesOnly, basePath);
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
        /// Displays entries in standard format.
        /// </summary>
        private static void DisplayStandardFormat(ShellContext context, string path, string filePattern, List<DirEntry> entries)
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

            foreach (DirEntry entry in entries)
            {
                if (entry.IsDirectory)
                {
                    Console.WriteLine("    <DIR>    " + entry.Name);
                }
                else
                {
                    Console.WriteLine("             " + entry.Name);
                }
            }

            Console.WriteLine();

            int dirCount = entries.Count(e => e.IsDirectory);
            int fileCount = entries.Count(e => !e.IsDirectory);
            Console.WriteLine("       {0} Dir(s)    {1} File(s)", dirCount, fileCount);
        }

            }
        }
