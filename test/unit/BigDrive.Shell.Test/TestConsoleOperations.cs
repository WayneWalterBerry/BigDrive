// <copyright file="TestConsoleOperations.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Test
{
    using BigDrive.Shell.LineInput;

    /// <summary>
    /// Test stub for console operations that doesn't require actual console.
    /// </summary>
    public class TestConsoleOperations : IConsoleOperations
    {
        /// <summary>
        /// Gets or sets the simulated cursor top position.
        /// </summary>
        public int CursorTop { get; set; }

        /// <summary>
        /// Gets the last cursor left position that was set.
        /// </summary>
        public int LastCursorLeft { get; private set; }

        /// <summary>
        /// Gets the last cursor top position that was set.
        /// </summary>
        public int LastCursorTop { get; private set; }

        /// <summary>
        /// Gets the last text that was written.
        /// </summary>
        public string LastWrittenText { get; private set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="TestConsoleOperations"/> class.
        /// </summary>
        public TestConsoleOperations()
        {
            CursorTop = 0;
            LastCursorLeft = 0;
            LastCursorTop = 0;
            LastWrittenText = string.Empty;
        }

        /// <summary>
        /// Sets the cursor position.
        /// </summary>
        /// <param name="left">The left position.</param>
        /// <param name="top">The top position.</param>
        public void SetCursorPosition(int left, int top)
        {
            LastCursorLeft = left;
            LastCursorTop = top;
        }

        /// <summary>
        /// Writes text to the console.
        /// </summary>
        /// <param name="text">The text to write.</param>
        public void Write(string text)
        {
            LastWrittenText = text;
        }
    }
}
