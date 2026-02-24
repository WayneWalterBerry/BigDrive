// <copyright file="HelpCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// Displays help information for available commands.
    /// </summary>
    public class HelpCommand : ICommand
    {
        /// <summary>
        /// Reference to the command dictionary for listing commands.
        /// </summary>
        private readonly Dictionary<string, ICommand> m_commands;

        /// <summary>
        /// Initializes a new instance of the <see cref="HelpCommand"/> class.
        /// </summary>
        /// <param name="commands">The command dictionary.</param>
        public HelpCommand(Dictionary<string, ICommand> commands)
        {
            m_commands = commands;
        }

        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "help"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "?" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Displays help information for commands"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "help [command]"; }
        }

        /// <summary>
        /// Executes the help command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            if (args.Length > 0)
            {
                if (m_commands.TryGetValue(args[0], out ICommand command))
                {
                    Console.WriteLine(command.Name + " - " + command.Description);
                    Console.WriteLine("Usage: " + command.Usage);
                    if (command.Aliases.Length > 0)
                    {
                        Console.WriteLine("Aliases: " + string.Join(", ", command.Aliases));
                    }
                }
                else
                {
                    Console.WriteLine("Unknown command: " + args[0]);
                }

                return;
            }

            Console.WriteLine("Available commands:");
            Console.WriteLine();

            HashSet<string> printed = new HashSet<string>();
            foreach (KeyValuePair<string, ICommand> kvp in m_commands)
            {
                if (!printed.Contains(kvp.Value.Name))
                {
                    Console.WriteLine("  " + kvp.Value.Name.PadRight(12) + kvp.Value.Description);
                    printed.Add(kvp.Value.Name);
                }
            }

            Console.WriteLine();
            Console.WriteLine("Type 'help <command>' for more information.");
        }
    }
}
