// <copyright file="WildcardMatcher.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Collections.Generic;
    using System.Text.RegularExpressions;

    /// <summary>
    /// Provides wildcard pattern matching for file and folder names.
    /// Supports standard DOS/PowerShell wildcards: * (any characters) and ? (single character).
    /// </summary>
    public static class WildcardMatcher
    {
        /// <summary>
        /// Determines if a pattern contains wildcard characters.
        /// </summary>
        /// <param name="pattern">The pattern to check.</param>
        /// <returns>True if the pattern contains * or ? wildcards.</returns>
        public static bool ContainsWildcard(string pattern)
        {
            if (string.IsNullOrEmpty(pattern))
            {
                return false;
            }

            return pattern.Contains("*") || pattern.Contains("?");
        }

        /// <summary>
        /// Determines if a name matches a wildcard pattern.
        /// </summary>
        /// <param name="name">The name to match.</param>
        /// <param name="pattern">The wildcard pattern (e.g., "*.txt", "file?.doc").</param>
        /// <returns>True if the name matches the pattern.</returns>
        public static bool IsMatch(string name, string pattern)
        {
            if (string.IsNullOrEmpty(name) || string.IsNullOrEmpty(pattern))
            {
                return false;
            }

            // Convert wildcard pattern to regex
            string regexPattern = WildcardToRegex(pattern);
            return Regex.IsMatch(name, regexPattern, RegexOptions.IgnoreCase);
        }

        /// <summary>
        /// Filters a list of names by a wildcard pattern.
        /// </summary>
        /// <param name="names">The names to filter.</param>
        /// <param name="pattern">The wildcard pattern.</param>
        /// <returns>Names that match the pattern.</returns>
        public static IEnumerable<string> Filter(IEnumerable<string> names, string pattern)
        {
            if (names == null)
            {
                yield break;
            }

            if (string.IsNullOrEmpty(pattern) || !ContainsWildcard(pattern))
            {
                // No wildcard - return all or exact match
                foreach (string name in names)
                {
                    if (string.IsNullOrEmpty(pattern) || 
                        name.Equals(pattern, StringComparison.OrdinalIgnoreCase))
                    {
                        yield return name;
                    }
                }

                yield break;
            }

            string regexPattern = WildcardToRegex(pattern);
            Regex regex = new Regex(regexPattern, RegexOptions.IgnoreCase);

            foreach (string name in names)
            {
                if (regex.IsMatch(name))
                {
                    yield return name;
                }
            }
        }

        /// <summary>
        /// Converts a DOS/PowerShell wildcard pattern to a regex pattern.
        /// </summary>
        /// <param name="wildcardPattern">The wildcard pattern.</param>
        /// <returns>The equivalent regex pattern.</returns>
        private static string WildcardToRegex(string wildcardPattern)
        {
            // Escape regex special characters, then convert wildcards
            string escaped = Regex.Escape(wildcardPattern);

            // Replace escaped wildcards with regex equivalents
            // \* becomes .* (match any characters)
            // \? becomes . (match single character)
            string regexPattern = escaped
                .Replace("\\*", ".*")
                .Replace("\\?", ".");

            // Anchor the pattern to match the entire string
            return "^" + regexPattern + "$";
        }

        /// <summary>
        /// Extracts the directory path and filename pattern from a path that may contain wildcards.
        /// </summary>
        /// <param name="path">The path potentially containing wildcards (e.g., "\folder\*.txt").</param>
        /// <param name="directoryPath">The directory portion of the path.</param>
        /// <param name="filePattern">The filename pattern (may contain wildcards).</param>
        public static void SplitPathAndPattern(string path, out string directoryPath, out string filePattern)
        {
            if (string.IsNullOrEmpty(path))
            {
                directoryPath = "\\";
                filePattern = "*";
                return;
            }

            // Find the last path separator
            int lastSeparator = path.LastIndexOfAny(new[] { '\\', '/' });

            if (lastSeparator < 0)
            {
                // No separator - pattern is the entire string, directory is root
                directoryPath = "\\";
                filePattern = path;
            }
            else if (lastSeparator == 0)
            {
                // Separator at start (e.g., "\*.txt")
                directoryPath = "\\";
                filePattern = path.Substring(1);
            }
            else
            {
                // Separator in middle (e.g., "\folder\*.txt")
                directoryPath = path.Substring(0, lastSeparator);
                filePattern = path.Substring(lastSeparator + 1);
            }

            // If file pattern is empty, default to all files
            if (string.IsNullOrEmpty(filePattern))
            {
                filePattern = "*";
            }
        }
    }
}
