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
        /// <param name="args">
        /// Command line arguments:
        ///   -d, --debug     Enable debug mode with verbose tracing.
        ///   -f, --file      Execute commands from a script file.
        ///   -c, --command   Execute a single command and exit.
        /// </param>
        /// <returns>Exit code: 0 for success, non-zero for failure.</returns>
        public static int Main(string[] args)
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

            // Check for script file mode
            string scriptFile = GetArgumentValue(args, "-f", "--file", "/f", "/file");
            if (scriptFile != null)
            {
                return RunScriptMode(scriptFile);
            }

            // Check for single command mode
            string command = GetArgumentValue(args, "-c", "--command", "/c", "/command");
            if (command != null)
            {
                return RunCommandMode(command);
            }

            // Interactive mode
            return RunInteractiveMode(debugMode);
        }

        /// <summary>
        /// Gets the value following a command line argument switch.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        /// <param name="switches">The switch names to look for.</param>
        /// <returns>The value following the switch, or null if not found.</returns>
        private static string GetArgumentValue(string[] args, params string[] switches)
        {
            for (int i = 0; i < args.Length - 1; i++)
            {
                foreach (string sw in switches)
                {
                    if (args[i].Equals(sw, StringComparison.OrdinalIgnoreCase))
                    {
                        return args[i + 1];
                    }
                }
            }

            return null;
        }

        /// <summary>
        /// Runs the shell in script mode, executing commands from a file.
        /// </summary>
        /// <param name="scriptPath">Path to the script file.</param>
        /// <returns>Exit code: 0 for success, non-zero for failure.</returns>
        private static int RunScriptMode(string scriptPath)
        {
            ShellTrace.Info("Running in script mode: {0}", scriptPath);

            ShellContext context = new ShellContext();
            CommandProcessor processor = new CommandProcessor(context);
            ScriptRunner runner = new ScriptRunner(context, processor);

            return runner.ExecuteScript(scriptPath);
        }

        /// <summary>
        /// Runs the shell in single command mode.
        /// </summary>
        /// <param name="command">The command to execute.</param>
        /// <returns>Exit code: 0 for success, non-zero for failure.</returns>
        private static int RunCommandMode(string command)
        {
            ShellTrace.Info("Running in command mode: {0}", command);

            ShellContext context = new ShellContext();
            CommandProcessor processor = new CommandProcessor(context);
            ScriptRunner runner = new ScriptRunner(context, processor);

            return runner.ExecuteCommand(command);
        }

        /// <summary>
        /// Runs the shell in interactive mode with line editing support.
        /// </summary>
        /// <param name="debugMode">True if debug mode is enabled.</param>
        /// <returns>Exit code: always 0.</returns>
        private static int RunInteractiveMode(bool debugMode)
        {
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
            return 0;
        }
    }
}
