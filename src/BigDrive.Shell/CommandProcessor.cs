// <copyright file="CommandProcessor.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Collections.Generic;

    using BigDrive.Shell.Commands;

    /// <summary>
    /// Processes and dispatches shell commands.
    /// </summary>
    public class CommandProcessor
    {
        /// <summary>
        /// The shell context.
        /// </summary>
        private readonly ShellContext m_context;

        /// <summary>
        /// Dictionary mapping command names to command instances.
        /// </summary>
        private readonly Dictionary<string, ICommand> m_commands;

        /// <summary>
        /// Initializes a new instance of the <see cref="CommandProcessor"/> class.
        /// </summary>
        /// <param name="context">The shell context.</param>
        public CommandProcessor(ShellContext context)
        {
            m_context = context;
            m_commands = new Dictionary<string, ICommand>(StringComparer.OrdinalIgnoreCase);

            RegisterCommands();
        }

        /// <summary>
        /// Executes the specified command line.
        /// </summary>
        /// <param name="commandLine">The full command line input.</param>
        public void Execute(string commandLine)
        {
            string[] parts = ParseCommandLine(commandLine);
            if (parts.Length == 0)
            {
                return;
            }

            string commandName = parts[0];
            string[] args = new string[parts.Length - 1];
            Array.Copy(parts, 1, args, 0, args.Length);

            if (m_commands.TryGetValue(commandName, out ICommand command))
            {
                try
                {
                    command.Execute(m_context, args);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Error: " + ex.Message);
                }
            }
            else
            {
                Console.WriteLine("Unknown command: " + commandName);
                Console.WriteLine("Type 'help' for available commands.");
            }
        }

        /// <summary>
        /// Registers all available commands.
        /// </summary>
        private void RegisterCommands()
        {
            RegisterCommand(new HelpCommand(m_commands));
            RegisterCommand(new ExitCommand());
            RegisterCommand(new DrivesCommand());
            RegisterCommand(new DirCommand());
            RegisterCommand(new CdCommand());
            RegisterCommand(new CopyCommand());
            RegisterCommand(new MkdirCommand());
            RegisterCommand(new DelCommand());
            RegisterCommand(new MountCommand());
            RegisterCommand(new UnmountCommand());
        }

        /// <summary>
        /// Registers a command with all its aliases.
        /// </summary>
        /// <param name="command">The command to register.</param>
        private void RegisterCommand(ICommand command)
        {
            m_commands[command.Name] = command;
            foreach (string alias in command.Aliases)
            {
                m_commands[alias] = command;
            }
        }

        /// <summary>
        /// Parses a command line into parts, respecting quoted strings.
        /// </summary>
        /// <param name="commandLine">The command line to parse.</param>
        /// <returns>Array of command parts.</returns>
        private static string[] ParseCommandLine(string commandLine)
        {
            List<string> parts = new List<string>();
            bool inQuotes = false;
            int start = 0;

            for (int i = 0; i < commandLine.Length; i++)
            {
                char c = commandLine[i];

                if (c == '"')
                {
                    inQuotes = !inQuotes;
                }
                else if (c == ' ' && !inQuotes)
                {
                    if (i > start)
                    {
                        string part = commandLine.Substring(start, i - start).Trim('"');
                        if (!string.IsNullOrEmpty(part))
                        {
                            parts.Add(part);
                        }
                    }

                    start = i + 1;
                }
            }

            if (start < commandLine.Length)
            {
                string part = commandLine.Substring(start).Trim('"');
                if (!string.IsNullOrEmpty(part))
                {
                    parts.Add(part);
                }
            }

            return parts.ToArray();
        }
    }
}
