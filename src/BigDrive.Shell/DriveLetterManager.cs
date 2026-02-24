// <copyright file="DriveLetterManager.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Manages drive letter assignments for BigDrive drives.
    /// Assigns letters starting from Z: and working backwards, avoiding OS drive letters.
    /// </summary>
    public class DriveLetterManager
    {
        /// <summary>
        /// Maps drive letters to BigDrive configurations.
        /// </summary>
        private readonly Dictionary<char, DriveConfiguration> m_bigDriveLetters;

        /// <summary>
        /// Set of drive letters used by the OS.
        /// </summary>
        private readonly HashSet<char> m_osDriveLetters;

        /// <summary>
        /// Initializes a new instance of the <see cref="DriveLetterManager"/> class.
        /// </summary>
        public DriveLetterManager()
        {
            m_bigDriveLetters = new Dictionary<char, DriveConfiguration>();
            m_osDriveLetters = new HashSet<char>();

            Initialize();
        }

        /// <summary>
        /// Gets the BigDrive letter assignments.
        /// </summary>
        public IReadOnlyDictionary<char, DriveConfiguration> BigDriveLetters
        {
            get { return m_bigDriveLetters; }
        }

        /// <summary>
        /// Gets the OS drive letters.
        /// </summary>
        public IReadOnlyCollection<char> OSDriveLetters
        {
            get { return m_osDriveLetters; }
        }

        /// <summary>
        /// Initializes drive letter assignments.
        /// </summary>
        public void Initialize()
        {
            m_bigDriveLetters.Clear();
            m_osDriveLetters.Clear();

            // Get OS drive letters
            DriveInfo[] systemDrives = DriveInfo.GetDrives();
            foreach (DriveInfo drive in systemDrives)
            {
                char letter = char.ToUpper(drive.Name[0]);
                m_osDriveLetters.Add(letter);
            }

            // Get BigDrive configurations
            List<DriveConfiguration> bigDrives;
            try
            {
                bigDrives = DriveManager.ReadConfigurations(CancellationToken.None).ToList();
            }
            catch (InvalidOperationException)
            {
                // No drives registered
                return;
            }

            // Assign letters starting from Z and working backwards
            char nextLetter = 'Z';
            foreach (DriveConfiguration config in bigDrives)
            {
                // Find next available letter
                while (nextLetter >= 'A' && (m_osDriveLetters.Contains(nextLetter) || m_bigDriveLetters.ContainsKey(nextLetter)))
                {
                    nextLetter--;
                }

                if (nextLetter < 'A')
                {
                    // No more letters available
                    break;
                }

                m_bigDriveLetters[nextLetter] = config;
                nextLetter--;
            }
        }

        /// <summary>
        /// Refreshes the drive letter assignments.
        /// </summary>
        public void Refresh()
        {
            Initialize();
        }

        /// <summary>
        /// Checks if a drive letter is a BigDrive.
        /// </summary>
        /// <param name="letter">The drive letter.</param>
        /// <returns>True if the letter is assigned to a BigDrive.</returns>
        public bool IsBigDrive(char letter)
        {
            return m_bigDriveLetters.ContainsKey(char.ToUpper(letter));
        }

        /// <summary>
        /// Checks if a drive letter is an OS drive.
        /// </summary>
        /// <param name="letter">The drive letter.</param>
        /// <returns>True if the letter is an OS drive.</returns>
        public bool IsOSDrive(char letter)
        {
            return m_osDriveLetters.Contains(char.ToUpper(letter));
        }

        /// <summary>
        /// Gets the drive configuration for a BigDrive letter.
        /// </summary>
        /// <param name="letter">The drive letter.</param>
        /// <returns>The drive configuration, or null if not a BigDrive.</returns>
        public DriveConfiguration GetDriveConfiguration(char letter)
        {
            if (m_bigDriveLetters.TryGetValue(char.ToUpper(letter), out DriveConfiguration config))
            {
                return config;
            }

            return null;
        }

        /// <summary>
        /// Gets the drive letter for a BigDrive GUID.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The drive letter, or null character if not found.</returns>
        public char GetDriveLetter(Guid driveGuid)
        {
            foreach (KeyValuePair<char, DriveConfiguration> kvp in m_bigDriveLetters)
            {
                if (kvp.Value.Id == driveGuid)
                {
                    return kvp.Key;
                }
            }

            return '\0';
        }
    }
}
