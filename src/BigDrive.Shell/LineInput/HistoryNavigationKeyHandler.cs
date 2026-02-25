// <copyright file="HistoryNavigationKeyHandler.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.LineInput
{
    using System;

    /// <summary>
    /// Handles Up/Down arrow keys for command history navigation.
    /// </summary>
    public class HistoryNavigationKeyHandler : IKeyHandler
    {
        /// <summary>
        /// The shared command history.
        /// </summary>
        private readonly CommandHistory m_history;

        /// <summary>
        /// Initializes a new instance of the <see cref="HistoryNavigationKeyHandler"/> class.
        /// </summary>
        /// <param name="history">The shared command history.</param>
        public HistoryNavigationKeyHandler(CommandHistory history)
        {
            m_history = history ?? throw new ArgumentNullException(nameof(history));
        }

        /// <summary>
        /// Attempts to handle the specified key press.
        /// </summary>
        /// <param name="keyInfo">The key that was pressed.</param>
        /// <param name="buffer">The line buffer to modify.</param>
        /// <returns>True if the key was handled.</returns>
        public bool HandleKey(ConsoleKeyInfo keyInfo, LineBuffer buffer)
        {
            if (keyInfo.Key == ConsoleKey.UpArrow)
            {
                HandleUpArrow(buffer);
                return true;
            }

            if (keyInfo.Key == ConsoleKey.DownArrow)
            {
                HandleDownArrow(buffer);
                return true;
            }

            return false;
        }

        /// <summary>
        /// Resets the handler state.
        /// </summary>
        public void Reset()
        {
            // Navigation state is managed by CommandHistory
        }

        /// <summary>
        /// Handles the Up arrow key for history navigation.
        /// </summary>
        /// <param name="buffer">The line buffer.</param>
        private void HandleUpArrow(LineBuffer buffer)
        {
            if (m_history.Count == 0)
            {
                return;
            }

            // Save current input if this is the first Up press
            if (!m_history.IsNavigating)
            {
                m_history.SavedInput = buffer.GetText();
                m_history.CurrentIndex = m_history.Count - 1;
            }
            else if (m_history.CurrentIndex > 0)
            {
                m_history.CurrentIndex--;
            }
            else
            {
                // Already at the oldest command
                return;
            }

            buffer.SetText(m_history.GetAt(m_history.CurrentIndex));
        }

        /// <summary>
        /// Handles the Down arrow key for history navigation.
        /// </summary>
        /// <param name="buffer">The line buffer.</param>
        private void HandleDownArrow(LineBuffer buffer)
        {
            if (!m_history.IsNavigating)
            {
                return;
            }

            if (m_history.CurrentIndex < m_history.Count - 1)
            {
                // Move to newer history entry
                m_history.CurrentIndex++;
                buffer.SetText(m_history.GetAt(m_history.CurrentIndex));
            }
            else
            {
                // Restore the saved current input
                buffer.SetText(m_history.SavedInput);
                m_history.ResetNavigation();
            }
        }
    }
}
