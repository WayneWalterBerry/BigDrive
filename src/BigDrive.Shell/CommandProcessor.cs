// <copyright file="CommandProcessor.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;

    using BigDrive.Interfaces;
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
        /// Gets the registered commands dictionary for tab completion.
        /// </summary>
        public Dictionary<string, ICommand> Commands
        {
            get { return m_commands; }
        }

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

            // Handle drive letter shortcut (e.g., "Z:" or "z:" switches to drive Z)
            if (IsDriveLetterCommand(commandName))
            {
                commandName = "cd";
                args = new string[] { parts[0] };
            }
            if (m_commands.TryGetValue(commandName, out ICommand command))
            {
                try
                {
                    command.Execute(m_context, args);
                }
                catch (BigDriveAuthenticationRequiredException authEx)
                {
                    HandleAuthenticationRequired(authEx);
                }
                catch (Exception ex)
                {
                    // Check if inner exception is auth-related
                    if (ex.InnerException is BigDriveAuthenticationRequiredException innerAuthEx)
                    {
                        HandleAuthenticationRequired(innerAuthEx);
                    }
                    else
                    {
                        Console.WriteLine("Error: " + ex.Message);
                    }
                }
            }
            else
            {
                // Pass unrecognized commands to the host OS shell (cmd.exe)
                PassThroughToShell(commandLine);
            }
        }

        /// <summary>
        /// Passes an unrecognized command to the host OS shell (cmd.exe /c).
        /// This allows commands like cls, type, echo, etc. to work transparently.
        /// </summary>
        /// <param name="commandLine">The full command line to pass through.</param>
        private static void PassThroughToShell(string commandLine)
        {
            ShellTrace.Verbose("Passing through to cmd.exe: \"{0}\"", commandLine);

            try
            {
                ProcessStartInfo startInfo = new ProcessStartInfo
                {
                    FileName = "cmd.exe",
                    Arguments = "/c " + commandLine,
                    UseShellExecute = false,
                    CreateNoWindow = false
                };

                using (Process process = Process.Start(startInfo))
                {
                    process.WaitForExit();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                Console.WriteLine("Type 'help' for available commands.");
            }
        }

        /// <summary>
        /// Handles authentication required exceptions by prompting for login.
        /// </summary>
        /// <param name="authEx">The authentication exception.</param>
        private void HandleAuthenticationRequired(BigDriveAuthenticationRequiredException authEx)
        {
            Console.WriteLine();
            Console.WriteLine(authEx.Message);
            Console.WriteLine();

            // Only prompt for auto-login if we have a current drive
            if (m_context.CurrentDriveGuid == null || m_context.CurrentDriveGuid == Guid.Empty)
            {
                Console.WriteLine("Please switch to a drive and run 'login' to authenticate.");
                return;
            }

            // Prompt user to login
            Console.Write("Would you like to login now? [Y/n]: ");
            string response = Console.ReadLine();

            if (string.IsNullOrEmpty(response) ||
                response.Equals("y", StringComparison.OrdinalIgnoreCase) ||
                response.Equals("yes", StringComparison.OrdinalIgnoreCase))
            {
                Console.WriteLine();

                // Execute login command
                if (m_commands.TryGetValue("login", out ICommand loginCommand))
                {
                    try
                    {
                        loginCommand.Execute(m_context, new string[0]);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Login failed: " + ex.Message);
                    }
                }
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
            RegisterCommand(new ProvidersCommand());
            RegisterCommand(new SecretCommand());
            RegisterCommand(new LoginCommand());
            RegisterCommand(new LogoutCommand());
            RegisterCommand(new AuthStatusCommand());
            RegisterCommand(new DirCommand());
            RegisterCommand(new CdCommand());
            RegisterCommand(new CopyCommand());
            RegisterCommand(new MkdirCommand());
            RegisterCommand(new DelCommand());
            RegisterCommand(new MoveCommand());
            RegisterCommand(new RenameCommand());
            RegisterCommand(new LabelCommand());
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

        /// <summary>
        /// Determines if the input is a drive letter command (e.g., "Z:" or "C:").
        /// This allows users to type "Z:" to switch drives, like in Windows cmd.exe.
        /// </summary>
        /// <param name="input">The input to check.</param>
        /// <returns>True if the input is a drive letter followed by a colon.</returns>
        private static bool IsDriveLetterCommand(string input)
        {
            if (string.IsNullOrEmpty(input))
            {
                return false;
            }

            // Check for pattern: single letter followed by colon (e.g., "Z:" or "z:")
            if (input.Length == 2 && char.IsLetter(input[0]) && input[1] == ':')
            {
                return true;
            }

            return false;
        }
    }
}
