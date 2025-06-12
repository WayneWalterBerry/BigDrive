// <copyright file="ConsoleExtensions.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Setup
{
    using System;
    using System.Diagnostics;
    using System.Runtime.CompilerServices;

    /// <summary>
    /// Extension methods for the Console class.
    /// </summary>
    public static class ConsoleExtensions
    {
        /// <summary>
        /// Writes a message to the console, indented by one tab for each layer deep in the call stack.
        /// </summary>
        /// <param name="console">The console instance (ignored, for extension method syntax).</param>
        /// <param name="message">The message to write.</param>
        public static void WriteIndented(string message)
        {
            int depth = new StackTrace(1, false).FrameCount;
            string indent = new string(' ', depth);
            Console.WriteLine($"{indent}{message}");
        }
    }
}