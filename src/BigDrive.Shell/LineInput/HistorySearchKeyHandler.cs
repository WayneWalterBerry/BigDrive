// <copyright file="HistorySearchKeyHandler.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.LineInput
{
    using System;

    /// <summary>
    /// Handles F8 key for prefix-based history search.
    /// </summary>
    public class HistorySearchKeyHandler : IKeyHandler
    {
        /// <summary>
        /// The shared command history.
        /// </summary>
        private readonly CommandHistory m_history;

        /// <summary>
        /// The search prefix for F8 history search.
        /// </summary>
        private string m_searchPrefix;

        /// <summary>
        /// Whether we are currently in F8 search mode.
        /// </summary>
        private bool m_inSearchMode;

        /// <summary>
        /// Initializes a new instance of the <see cref="HistorySearchKeyHandler"/> class.
        /// </summary>
        /// <param name="history">The shared command history.</param>
        public HistorySearchKeyHandler(CommandHistory history)
        {
            m_history = history ?? throw new ArgumentNullException(nameof(history));
            m_searchPrefix = string.Empty;
            m_inSearchMode = false;
        }

        /// <summary>
        /// Attempts to handle the specified key press.
        /// </summary>
        /// <param name="keyInfo">The key that was pressed.</param>
        /// <param name="buffer">The line buffer to modify.</param>
        /// <returns>True if the key was handled.</returns>
        public bool HandleKey(ConsoleKeyInfo keyInfo, LineBuffer buffer)
        {
            if (keyInfo.Key == ConsoleKey.F8)
            {
                HandleF8Search(buffer);
                return true;
            }

            // Any other key exits search mode
            if (m_inSearchMode)
            {
                m_inSearchMode = false;
                m_searchPrefix = string.Empty;
            }

            return false;
        }

        /// <summary>
        /// Resets the handler state.
        /// </summary>
        public void Reset()
        {
            m_searchPrefix = string.Empty;
            m_inSearchMode = false;
        }

        /// <summary>
        /// Handles F8 key for prefix-based history search.
        /// </summary>
        /// <param name="buffer">The line buffer.</param>
        private void HandleF8Search(LineBuffer buffer)
        {
            if (m_history.Count == 0)
            {
                return;
            }

            // If not in search mode, start a new search
            if (!m_inSearchMode)
            {
                m_searchPrefix = buffer.GetText();
                m_history.SavedInput = m_searchPrefix;
                m_inSearchMode = true;

                // Start searching from the end of history
                m_history.CurrentIndex = m_history.Count;
            }

            // Find the next matching entry
            int matchIndex = m_history.FindPreviousMatch(m_searchPrefix, m_history.CurrentIndex);

            if (matchIndex >= 0)
            {
                m_history.CurrentIndex = matchIndex;
                buffer.SetText(m_history.GetAt(matchIndex));
            }
            else
            {
                // No matches found
                Console.Beep();
            }
        }
    }
}
