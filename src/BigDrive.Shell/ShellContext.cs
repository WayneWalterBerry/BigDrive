// <copyright file="ShellContext.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Collections.Generic;

    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Maintains the current state of the shell session.
    /// Supports multiple drives with drive letter assignments.
    /// </summary>
    public class ShellContext
    {
        /// <summary>
        /// The drive letter manager.
        /// </summary>
        private readonly DriveLetterManager m_driveLetterManager;

        /// <summary>
        /// The current drive letter (A-Z), or null character if no drive selected.
        /// </summary>
        private char m_currentDriveLetter;

        /// <summary>
        /// The current path within the drive.
        /// </summary>
        private string m_currentPath;

        /// <summary>
        /// Tracks the current path for each drive letter.
        /// </summary>
        private readonly Dictionary<char, string> m_driveCurrentPaths;

        /// <summary>
        /// Flag indicating whether the shell should exit.
        /// </summary>
        private bool m_shouldExit;

        /// <summary>
        /// Initializes a new instance of the <see cref="ShellContext"/> class.
        /// </summary>
        public ShellContext()
        {
            m_driveLetterManager = new DriveLetterManager();
            m_currentDriveLetter = '\0';
            m_currentPath = "\\";
            m_driveCurrentPaths = new Dictionary<char, string>();
            m_shouldExit = false;

            // Initialize current paths for all BigDrive letters
            foreach (char letter in m_driveLetterManager.BigDriveLetters.Keys)
            {
                m_driveCurrentPaths[letter] = "\\";
            }
        }

        /// <summary>
        /// Gets the drive letter manager.
        /// </summary>
        public DriveLetterManager DriveLetterManager
        {
            get { return m_driveLetterManager; }
        }

        /// <summary>
        /// Gets or sets the current drive letter.
        /// </summary>
        public char CurrentDriveLetter
        {
            get { return m_currentDriveLetter; }
            set { m_currentDriveLetter = value; }
        }

        /// <summary>
        /// Gets the current drive GUID, or null if no drive selected or OS drive.
        /// </summary>
        public Guid? CurrentDriveGuid
        {
            get
            {
                if (m_currentDriveLetter == '\0')
                {
                    return null;
                }

                DriveConfiguration config = m_driveLetterManager.GetDriveConfiguration(m_currentDriveLetter);
                return config?.Id;
            }
        }

        /// <summary>
        /// Gets or sets the current path within the drive.
        /// </summary>
        public string CurrentPath
        {
            get { return m_currentPath; }
            set
            {
                m_currentPath = value;

                // Also update the drive-specific path
                if (m_currentDriveLetter != '\0' && m_driveLetterManager.IsBigDrive(m_currentDriveLetter))
                {
                    m_driveCurrentPaths[m_currentDriveLetter] = value;
                }
            }
        }

        /// <summary>
        /// Gets the current drive name for display.
        /// </summary>
        public string CurrentDriveName
        {
            get
            {
                if (m_currentDriveLetter == '\0')
                {
                    return string.Empty;
                }

                DriveConfiguration config = m_driveLetterManager.GetDriveConfiguration(m_currentDriveLetter);
                return config?.Name ?? m_currentDriveLetter.ToString();
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether the shell should exit.
        /// </summary>
        public bool ShouldExit
        {
            get { return m_shouldExit; }
            set { m_shouldExit = value; }
        }

        /// <summary>
        /// Changes to the specified drive letter.
        /// </summary>
        /// <param name="letter">The drive letter to change to.</param>
        /// <returns>True if successful.</returns>
        public bool ChangeDrive(char letter)
        {
            letter = char.ToUpper(letter);

            if (!m_driveLetterManager.IsBigDrive(letter))
            {
                return false;
            }

            // Save current path for current drive
            if (m_currentDriveLetter != '\0' && m_driveLetterManager.IsBigDrive(m_currentDriveLetter))
            {
                m_driveCurrentPaths[m_currentDriveLetter] = m_currentPath;
            }

            // Switch to new drive
            m_currentDriveLetter = letter;

            // Restore path for new drive
            if (m_driveCurrentPaths.TryGetValue(letter, out string savedPath))
            {
                m_currentPath = savedPath;
            }
            else
            {
                m_currentPath = "\\";
            }

            return true;
        }

        /// <summary>
        /// Gets the current path for a specific drive letter.
        /// </summary>
        /// <param name="letter">The drive letter.</param>
        /// <returns>The current path for that drive.</returns>
        public string GetPathForDrive(char letter)
        {
            letter = char.ToUpper(letter);

            if (letter == m_currentDriveLetter)
            {
                return m_currentPath;
            }

            if (m_driveCurrentPaths.TryGetValue(letter, out string path))
            {
                return path;
            }

            return "\\";
        }

        /// <summary>
        /// Gets the shell prompt string.
        /// </summary>
        /// <returns>The formatted prompt string.</returns>
        public string GetPrompt()
        {
            if (m_currentDriveLetter != '\0')
            {
                string displayPath = m_currentPath == "\\" ? "\\" : m_currentPath;
                return string.Format("{0}:{1}> ", m_currentDriveLetter, displayPath);
            }

            return "BD> ";
        }

        /// <summary>
        /// Refreshes the drive letter assignments.
        /// </summary>
        public void RefreshDrives()
        {
            m_driveLetterManager.Refresh();

            // Initialize paths for any new drives
            foreach (char letter in m_driveLetterManager.BigDriveLetters.Keys)
            {
                if (!m_driveCurrentPaths.ContainsKey(letter))
                {
                    m_driveCurrentPaths[letter] = "\\";
                }
            }
        }
    }
}
