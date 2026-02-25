// <copyright file="EditingKeyHandler.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.LineInput
{
    using System;

    /// <summary>
    /// Handles editing keys: Backspace, Delete, Escape.
    /// </summary>
    public class EditingKeyHandler : IKeyHandler
    {
        /// <summary>
        /// Callback invoked when Escape is pressed (to notify other handlers to reset).
        /// </summary>
        private readonly Action m_onEscape;

        /// <summary>
        /// Initializes a new instance of the <see cref="EditingKeyHandler"/> class.
        /// </summary>
        /// <param name="onEscape">Optional callback when Escape is pressed.</param>
        public EditingKeyHandler(Action onEscape = null)
        {
            m_onEscape = onEscape;
        }

        /// <summary>
        /// Attempts to handle the specified key press.
        /// </summary>
        /// <param name="keyInfo">The key that was pressed.</param>
        /// <param name="buffer">The line buffer to modify.</param>
        /// <returns>True if the key was handled.</returns>
        public bool HandleKey(ConsoleKeyInfo keyInfo, LineBuffer buffer)
        {
            switch (keyInfo.Key)
            {
                case ConsoleKey.Backspace:
                    buffer.Backspace();
                    return true;

                case ConsoleKey.Delete:
                    buffer.Delete();
                    return true;

                case ConsoleKey.Escape:
                    buffer.Clear();
                    m_onEscape?.Invoke();
                    return true;

                default:
                    return false;
            }
        }

        /// <summary>
        /// Resets any internal state. Editing handler has no state.
        /// </summary>
        public void Reset()
        {
            // No state to reset
        }
    }
}
