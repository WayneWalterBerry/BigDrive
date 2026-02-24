// <copyright file="CdCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;

    using BigDrive.ConfigProvider.Model;
    using BigDrive.Interfaces;

    /// <summary>
    /// Changes the current directory or switches to a different drive.
    /// </summary>
    public class CdCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "cd"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "chdir" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Changes the current directory or drive"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "cd <path>  |  cd X:  |  cd X:\\folder  |  cd .."; }
        }

        /// <summary>
        /// Executes the cd command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            if (args.Length == 0)
            {
                // Show current location
                if (context.CurrentDriveLetter != '\0')
                {
                    Console.WriteLine("{0}:{1}", context.CurrentDriveLetter, context.CurrentPath);
                }
                else
                {
                    Console.WriteLine("No drive selected. Use 'cd X:' to select a drive.");
                }

                return;
            }

            string targetPath = args[0];

            // Check for drive letter change (e.g., "X:" or "X:\path")
            if (targetPath.Length >= 2 && targetPath[1] == ':')
            {
                char letter = char.ToUpper(targetPath[0]);

                if (letter >= 'A' && letter <= 'Z')
                {
                    // Check if it's a BigDrive
                    if (context.DriveLetterManager.IsBigDrive(letter))
                    {
                        // Switch to the drive
                        context.ChangeDrive(letter);

                        // If there's a path after the drive letter, navigate to it
                        if (targetPath.Length > 2)
                        {
                            string pathPart = targetPath.Substring(2);
                            if (!string.IsNullOrEmpty(pathPart) && pathPart != "\\")
                            {
                                NavigateToPath(context, pathPart);
                            }
                        }

                        return;
                    }
                    else if (context.DriveLetterManager.IsOSDrive(letter))
                    {
                        Console.WriteLine("Cannot navigate to OS drives from BigDrive Shell.");
                        Console.WriteLine("Use 'copy' command to transfer files to/from local drives.");
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

            // Regular path navigation
            if (context.CurrentDriveLetter == '\0')
            {
                Console.WriteLine("No drive selected. Use 'cd X:' to select a BigDrive.");
                return;
            }

            NavigateToPath(context, targetPath);
        }

        /// <summary>
        /// Navigates to the specified path on the current drive.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="targetPath">The target path.</param>
        private static void NavigateToPath(ShellContext context, string targetPath)
        {
            // Handle special cases
            if (targetPath == "\\" || targetPath == "/")
            {
                context.CurrentPath = "\\";
                return;
            }

            if (targetPath == "..")
            {
                NavigateUp(context);
                return;
            }

            // Resolve the path
            string newPath = ResolvePath(context.CurrentPath, targetPath);

            // Verify the folder exists
            if (!VerifyFolderExists(context, newPath))
            {
                Console.WriteLine("The system cannot find the path specified: " + targetPath);
                return;
            }

            context.CurrentPath = newPath;
        }

        /// <summary>
        /// Navigates up one directory level.
        /// </summary>
        /// <param name="context">The shell context.</param>
        private static void NavigateUp(ShellContext context)
        {
            string path = context.CurrentPath.TrimEnd('\\', '/');

            int lastSep = path.LastIndexOfAny(new char[] { '\\', '/' });
            if (lastSep <= 0)
            {
                context.CurrentPath = "\\";
            }
            else
            {
                context.CurrentPath = path.Substring(0, lastSep);
                if (string.IsNullOrEmpty(context.CurrentPath))
                {
                    context.CurrentPath = "\\";
                }
            }
        }

        /// <summary>
        /// Resolves a relative or absolute path.
        /// </summary>
        /// <param name="currentPath">The current path.</param>
        /// <param name="targetPath">The target path.</param>
        /// <returns>The resolved absolute path.</returns>
        private static string ResolvePath(string currentPath, string targetPath)
        {
            // Absolute path
            if (targetPath.StartsWith("\\") || targetPath.StartsWith("/"))
            {
                return NormalizePath(targetPath);
            }

            // Relative path
            string combined;
            if (currentPath == "\\" || currentPath == "/")
            {
                combined = "\\" + targetPath;
            }
            else
            {
                combined = currentPath.TrimEnd('\\', '/') + "\\" + targetPath;
            }

            return NormalizePath(combined);
        }

        /// <summary>
        /// Normalizes a path (handles .. and .).
        /// </summary>
        /// <param name="path">The path to normalize.</param>
        /// <returns>The normalized path.</returns>
        private static string NormalizePath(string path)
        {
            string[] parts = path.Split(new char[] { '\\', '/' }, StringSplitOptions.RemoveEmptyEntries);
            var stack = new System.Collections.Generic.Stack<string>();

            foreach (string part in parts)
            {
                if (part == "..")
                {
                    if (stack.Count > 0)
                    {
                        stack.Pop();
                    }
                }
                else if (part != ".")
                {
                    stack.Push(part);
                }
            }

            if (stack.Count == 0)
            {
                return "\\";
            }

            string[] result = new string[stack.Count];
            for (int i = stack.Count - 1; i >= 0; i--)
            {
                result[i] = stack.Pop();
            }

            return "\\" + string.Join("\\", result);
        }

        /// <summary>
        /// Verifies that a folder exists at the specified path.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="path">The path to verify.</param>
        /// <returns>True if the folder exists.</returns>
        private static bool VerifyFolderExists(ShellContext context, string path)
        {
            // Root always exists
            if (path == "\\" || path == "/")
            {
                return true;
            }

            if (!context.CurrentDriveGuid.HasValue)
            {
                return false;
            }

            IBigDriveEnumerate enumerate = ProviderFactory.GetEnumerateProvider(context.CurrentDriveGuid.Value);
            if (enumerate == null)
            {
                return false;
            }

            // Get parent path and folder name
            string parentPath;
            string folderName;

            int lastSep = path.TrimEnd('\\', '/').LastIndexOfAny(new char[] { '\\', '/' });
            if (lastSep <= 0)
            {
                parentPath = "\\";
                folderName = path.TrimStart('\\', '/');
            }
            else
            {
                parentPath = path.Substring(0, lastSep);
                folderName = path.Substring(lastSep + 1);
            }

            string[] folders = enumerate.EnumerateFolders(context.CurrentDriveGuid.Value, parentPath);
            foreach (string folder in folders)
            {
                if (string.Equals(folder, folderName, StringComparison.OrdinalIgnoreCase))
                {
                    return true;
                }
            }

            return false;
        }
    }
}
