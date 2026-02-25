// <copyright file="CommandHistory.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.LineInput
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// Manages the command history list and navigation state.
    /// Shared by history-related key handlers.
    /// </summary>
    public class CommandHistory
    {
        /// <summary>
        /// The command history list.
        /// </summary>
        private readonly List<string> m_entries;

        /// <summary>
        /// Maximum number of commands to keep in history.
        /// </summary>
        private readonly int m_maxSize;

        /// <summary>
        /// The current position in the history when navigating.
        /// -1 means not navigating history (at the "new command" position).
        /// </summary>
        private int m_currentIndex;

        /// <summary>
        /// The current input saved when starting history navigation.
        /// </summary>
        private string m_savedInput;

        /// <summary>
        /// Initializes a new instance of the <see cref="CommandHistory"/> class.
        /// </summary>
        /// <param name="maxSize">Maximum number of commands to store.</param>
        public CommandHistory(int maxSize = 100)
        {
            m_entries = new List<string>();
            m_maxSize = maxSize;
            m_currentIndex = -1;
            m_savedInput = string.Empty;
        }

        /// <summary>
        /// Gets the number of entries in history.
        /// </summary>
        public int Count
        {
            get { return m_entries.Count; }
        }

        /// <summary>
        /// Gets the current navigation index.
        /// -1 means not navigating (at "new command" position).
        /// </summary>
        public int CurrentIndex
        {
            get { return m_currentIndex; }
            set { m_currentIndex = value; }
        }

        /// <summary>
        /// Gets or sets the saved input from before navigation started.
        /// </summary>
        public string SavedInput
        {
            get { return m_savedInput; }
            set { m_savedInput = value ?? string.Empty; }
        }

        /// <summary>
        /// Gets whether we are currently navigating history.
        /// </summary>
        public bool IsNavigating
        {
            get { return m_currentIndex >= 0; }
        }

        /// <summary>
        /// Gets the history entry at the specified index.
        /// </summary>
        /// <param name="index">The index.</param>
        /// <returns>The history entry.</returns>
        public string GetAt(int index)
        {
            if (index < 0 || index >= m_entries.Count)
            {
                return string.Empty;
            }

            return m_entries[index];
        }

        /// <summary>
        /// Adds a command to the history.
        /// </summary>
        /// <param name="command">The command to add.</param>
        public void Add(string command)
        {
            if (string.IsNullOrWhiteSpace(command))
            {
                return;
            }

            // Don't add duplicate of last command
            if (m_entries.Count > 0 && m_entries[m_entries.Count - 1].Equals(command, StringComparison.Ordinal))
            {
                return;
            }

            m_entries.Add(command);

            // Trim history if too large
            if (m_entries.Count > m_maxSize)
            {
                m_entries.RemoveAt(0);
            }
        }

        /// <summary>
        /// Finds the next history entry (searching backwards) that starts with the given prefix.
        /// </summary>
        /// <param name="prefix">The prefix to search for (case-insensitive).</param>
        /// <param name="startIndex">The index to start searching from (exclusive).</param>
        /// <returns>The index of the matching entry, or -1 if not found.</returns>
        public int FindPreviousMatch(string prefix, int startIndex)
        {
            // Search backwards from startIndex
            for (int i = startIndex - 1; i >= 0; i--)
            {
                if (IsMatch(m_entries[i], prefix))
                {
                    return i;
                }
            }

            // Wrap around to end of history
            for (int i = m_entries.Count - 1; i >= startIndex; i--)
            {
                if (IsMatch(m_entries[i], prefix))
                {
                    return i;
                }
            }

            return -1;
        }

        /// <summary>
        /// Resets the navigation state.
        /// </summary>
        public void ResetNavigation()
        {
            m_currentIndex = -1;
            m_savedInput = string.Empty;
        }

        /// <summary>
        /// Checks if a history entry matches the search prefix.
        /// </summary>
        /// <param name="entry">The history entry.</param>
        /// <param name="prefix">The search prefix.</param>
        /// <returns>True if the entry matches.</returns>
        private static bool IsMatch(string entry, string prefix)
        {
            if (string.IsNullOrEmpty(prefix))
            {
                return true;
            }

            return entry.StartsWith(prefix, StringComparison.OrdinalIgnoreCase);
        }
    }
}
