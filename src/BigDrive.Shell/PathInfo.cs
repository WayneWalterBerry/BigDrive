// <copyright file="PathInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;

    /// <summary>
    /// Represents a parsed path with drive letter and path components.
    /// </summary>
    public class PathInfo
    {
        /// <summary>
        /// Gets or sets the drive letter (A-Z), or null character for relative paths.
        /// </summary>
        public char DriveLetter { get; set; }

        /// <summary>
        /// Gets or sets the path portion after the drive letter.
        /// </summary>
        public string Path { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether this is a BigDrive path.
        /// </summary>
        public bool IsBigDrive { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether this is an OS drive path.
        /// </summary>
        public bool IsOSDrive { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether this is a relative path.
        /// </summary>
        public bool IsRelative { get; set; }

        /// <summary>
        /// Gets the full path string (e.g., "X:\folder\file.jpg").
        /// </summary>
        /// <returns>The full path string.</returns>
        public string GetFullPath()
        {
            if (IsRelative)
            {
                return Path;
            }

            return DriveLetter + ":\\" + Path.TrimStart('\\');
        }

        /// <summary>
        /// Parses a path string into a PathInfo object.
        /// </summary>
        /// <param name="pathString">The path string to parse.</param>
        /// <param name="driveManager">The drive letter manager for determining drive types.</param>
        /// <param name="currentDriveLetter">The current drive letter for relative paths.</param>
        /// <returns>A PathInfo object.</returns>
        public static PathInfo Parse(string pathString, DriveLetterManager driveManager, char currentDriveLetter)
        {
            PathInfo info = new PathInfo();

            if (string.IsNullOrEmpty(pathString))
            {
                info.IsRelative = true;
                info.Path = string.Empty;
                return info;
            }

            pathString = pathString.Trim();

            // Check for drive letter prefix (X: or X:\)
            if (pathString.Length >= 2 && pathString[1] == ':')
            {
                char letter = char.ToUpper(pathString[0]);
                if (letter >= 'A' && letter <= 'Z')
                {
                    info.DriveLetter = letter;
                    info.IsRelative = false;
                    info.IsBigDrive = driveManager.IsBigDrive(letter);
                    info.IsOSDrive = driveManager.IsOSDrive(letter);

                    // Extract path after drive letter
                    if (pathString.Length > 2)
                    {
                        info.Path = pathString.Substring(2);
                    }
                    else
                    {
                        info.Path = "\\";
                    }

                    return info;
                }
            }

            // Relative path - use current drive
            info.IsRelative = true;
            info.Path = pathString;

            if (currentDriveLetter != '\0')
            {
                info.DriveLetter = currentDriveLetter;
                info.IsBigDrive = driveManager.IsBigDrive(currentDriveLetter);
                info.IsOSDrive = driveManager.IsOSDrive(currentDriveLetter);
            }

            return info;
        }

        /// <summary>
        /// Resolves a relative or absolute path against a current directory,
        /// normalizing . and .. segments.
        /// </summary>
        /// <param name="currentPath">The current directory path.</param>
        /// <param name="targetPath">The target path (relative or absolute).</param>
        /// <returns>The resolved normalized absolute path.</returns>
        public static string ResolvePath(string currentPath, string targetPath)
        {
            string combined;

            if (targetPath.StartsWith("\\") || targetPath.StartsWith("/"))
            {
                combined = targetPath;
            }
            else if (currentPath == "\\" || currentPath == "/")
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
        /// Normalizes a path by resolving . and .. segments.
        /// </summary>
        /// <param name="path">The path to normalize.</param>
        /// <returns>The normalized path.</returns>
        public static string NormalizePath(string path)
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
    }
}
