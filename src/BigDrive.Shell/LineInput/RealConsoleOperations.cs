// <copyright file="RealConsoleOperations.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.LineInput
{
    using System;

    /// <summary>
    /// Real console operations implementation.
    /// </summary>
    internal class RealConsoleOperations : IConsoleOperations
    {
        /// <summary>
        /// Gets the current cursor top position.
        /// </summary>
        public int CursorTop
        {
            get { return Console.CursorTop; }
        }

        /// <summary>
        /// Sets the cursor position.
        /// </summary>
        /// <param name="left">The left position.</param>
        /// <param name="top">The top position.</param>
        public void SetCursorPosition(int left, int top)
        {
            Console.SetCursorPosition(left, top);
        }

        /// <summary>
        /// Writes text to the console.
        /// </summary>
        /// <param name="text">The text to write.</param>
        public void Write(string text)
        {
            Console.Write(text);
        }
    }
}
