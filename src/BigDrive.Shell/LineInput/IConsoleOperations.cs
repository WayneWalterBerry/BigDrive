// <copyright file="IConsoleOperations.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.LineInput
{
    /// <summary>
    /// Abstraction for console operations to enable testability.
    /// </summary>
    public interface IConsoleOperations
    {
        /// <summary>
        /// Gets the current cursor top position.
        /// </summary>
        int CursorTop { get; }

        /// <summary>
        /// Sets the cursor position.
        /// </summary>
        /// <param name="left">The left position.</param>
        /// <param name="top">The top position.</param>
        void SetCursorPosition(int left, int top);

        /// <summary>
        /// Writes text to the console.
        /// </summary>
        /// <param name="text">The text to write.</param>
        void Write(string text);
    }
}
