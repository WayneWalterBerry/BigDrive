// <copyright file="IKeyHandler.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.LineInput
{
    using System;

    /// <summary>
    /// Interface for key handlers in the Chain of Responsibility pattern.
    /// Each handler processes specific keystrokes and returns whether it handled the key.
    /// </summary>
    public interface IKeyHandler
    {
        /// <summary>
        /// Attempts to handle the specified key press.
        /// </summary>
        /// <param name="keyInfo">The key that was pressed.</param>
        /// <param name="buffer">The line buffer to modify.</param>
        /// <returns>True if the key was handled; false to pass to next handler in chain.</returns>
        bool HandleKey(ConsoleKeyInfo keyInfo, LineBuffer buffer);

        /// <summary>
        /// Resets any internal state maintained by the handler.
        /// Called when a new line input begins or when input is cancelled.
        /// </summary>
        void Reset();
    }
}
