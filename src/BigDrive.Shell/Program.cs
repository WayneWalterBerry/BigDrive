// <copyright file="Program.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Linq;

    using BigDrive.Shell.LineInput;

    /// <summary>
    /// Entry point for the BigDrive Shell application.
    /// </summary>
    public class Program
    {
        /// <summary>
        /// Main entry point for the shell.
        /// </summary>
        /// <param name="args">Command line arguments. Supports -d or --debug for debug mode.</param>
        public static void Main(string[] args)
        {
            // Check for debug mode switch
            bool debugMode = args.Any(a => 
                a.Equals("-d", StringComparison.OrdinalIgnoreCase) ||
                a.Equals("--debug", StringComparison.OrdinalIgnoreCase) ||
                a.Equals("/d", StringComparison.OrdinalIgnoreCase) ||
                a.Equals("/debug", StringComparison.OrdinalIgnoreCase));

            if (debugMode)
            {
                ShellTrace.DebugEnabled = true;
                ShellTrace.Info("Debug mode enabled - verbose tracing active");
            }

            Console.WriteLine("BigDrive Shell v1.0" + (debugMode ? " [DEBUG MODE]" : string.Empty));
            Console.WriteLine("Type 'help' for available commands, 'exit' to quit.");
            Console.WriteLine();

            ShellContext context = new ShellContext();
            CommandProcessor processor = new CommandProcessor(context);
            ConsoleLineReader reader = new ConsoleLineReader(context, processor.Commands);

            ShellTrace.Info("Shell initialized with {0} drives", context.DriveLetterManager.BigDriveLetters.Count);

            while (!context.ShouldExit)
            {
                Console.Write(context.GetPrompt());
                string input = reader.ReadLine();

                if (string.IsNullOrWhiteSpace(input))
                {
                    continue;
                }

                ShellTrace.Verbose("Command input: \"{0}\"", input.Trim());
                processor.Execute(input.Trim());
            }

            ShellTrace.Info("Shell exiting");
        }
    }
}
