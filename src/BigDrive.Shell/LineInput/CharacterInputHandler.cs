// <copyright file="CharacterInputHandler.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.LineInput
{
    using System;

    /// <summary>
    /// Handles regular character input. This is the fallback handler at the end of the chain.
    /// </summary>
    public class CharacterInputHandler : IKeyHandler
    {
        /// <summary>
        /// Callback invoked when a character is inserted (to notify other handlers to reset).
        /// </summary>
        private readonly Action m_onCharacterInserted;

        /// <summary>
        /// Initializes a new instance of the <see cref="CharacterInputHandler"/> class.
        /// </summary>
        /// <param name="onCharacterInserted">Optional callback when a character is inserted.</param>
        public CharacterInputHandler(Action onCharacterInserted = null)
        {
            m_onCharacterInserted = onCharacterInserted;
        }

        /// <summary>
        /// Attempts to handle the specified key press.
        /// </summary>
        /// <param name="keyInfo">The key that was pressed.</param>
        /// <param name="buffer">The line buffer to modify.</param>
        /// <returns>True if the key was handled (always true for printable characters).</returns>
        public bool HandleKey(ConsoleKeyInfo keyInfo, LineBuffer buffer)
        {
            // Only handle printable characters
            if (char.IsControl(keyInfo.KeyChar))
            {
                return false;
            }

            buffer.Insert(keyInfo.KeyChar);
            m_onCharacterInserted?.Invoke();
            return true;
        }

        /// <summary>
        /// Resets any internal state. Character input handler has no state.
        /// </summary>
        public void Reset()
        {
            // No state to reset
        }
    }
}
