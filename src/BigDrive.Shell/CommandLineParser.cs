// <copyright file="CommandLineParser.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    /// <summary>
    /// Parses command-line arguments and switches.
    /// Supports both Windows-style switches (/switch) and Unix-style switches (-switch).
    /// </summary>
    public class CommandLineParser
    {
        /// <summary>
        /// Gets the list of parsed switches (without prefix).
        /// </summary>
        public List<string> Switches { get; private set; }

        /// <summary>
        /// Gets the list of arguments that are not switches.
        /// </summary>
        public List<string> Arguments { get; private set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="CommandLineParser"/> class.
        /// </summary>
        /// <param name="args">The command-line arguments to parse.</param>
        public CommandLineParser(string[] args)
        {
            Switches = new List<string>();
            Arguments = new List<string>();

            foreach (string arg in args)
            {
                if (arg.StartsWith("-") || arg.StartsWith("/"))
                {
                    // Remove prefix and add to switches
                    Switches.Add(arg.Substring(1).ToLowerInvariant());
                }
                else
                {
                    Arguments.Add(arg);
                }
            }
        }

        /// <summary>
        /// Checks if a switch is present (supports multiple forms).
        /// </summary>
        /// <param name="names">The switch names to check (e.g., "ad", "Directory").</param>
        /// <returns>True if any of the switch names are present.</returns>
        public bool HasSwitch(params string[] names)
        {
            foreach (string name in names)
            {
                if (Switches.Contains(name.ToLowerInvariant()))
                {
                    return true;
                }
            }

            return false;
        }

        /// <summary>
        /// Gets the value of a switch in the format "key:value" or "key=value".
        /// </summary>
        /// <param name="switchName">The switch name to find.</param>
        /// <returns>The value part of the switch, or null if not found or no value specified.</returns>
        public string GetSwitchValue(string switchName)
        {
            string lowerName = switchName.ToLowerInvariant();

            foreach (string sw in Switches)
            {
                // Check for colon separator (e.g., "o:n")
                if (sw.StartsWith(lowerName + ":") && sw.Length > lowerName.Length + 1)
                {
                    return sw.Substring(lowerName.Length + 1);
                }

                // Check for equals separator (e.g., "o=n")
                if (sw.StartsWith(lowerName + "=") && sw.Length > lowerName.Length + 1)
                {
                    return sw.Substring(lowerName.Length + 1);
                }
            }

            return null;
        }
    }
}
