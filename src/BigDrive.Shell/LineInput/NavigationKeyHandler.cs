// <copyright file="NavigationKeyHandler.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.LineInput
{
    using System;

    /// <summary>
    /// Handles cursor navigation keys: Left, Right, Home, End.
    /// </summary>
    public class NavigationKeyHandler : IKeyHandler
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="NavigationKeyHandler"/> class.
        /// </summary>
        public NavigationKeyHandler()
        {
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
                case ConsoleKey.LeftArrow:
                    buffer.MoveLeft();
                    return true;

                case ConsoleKey.RightArrow:
                    buffer.MoveRight();
                    return true;

                case ConsoleKey.Home:
                    buffer.MoveToStart();
                    return true;

                case ConsoleKey.End:
                    buffer.MoveToEnd();
                    return true;

                default:
                    return false;
            }
        }

        /// <summary>
        /// Resets any internal state. Navigation handler has no state.
        /// </summary>
        public void Reset()
        {
            // No state to reset
        }
    }
}
