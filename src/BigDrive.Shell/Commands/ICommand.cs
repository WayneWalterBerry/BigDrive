// <copyright file="ICommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    /// <summary>
    /// Interface for shell commands.
    /// </summary>
    public interface ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        string Name { get; }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        string[] Aliases { get; }

        /// <summary>
        /// Gets the command description for help.
        /// </summary>
        string Description { get; }

        /// <summary>
        /// Gets the usage syntax for the command.
        /// </summary>
        string Usage { get; }

        /// <summary>
        /// Executes the command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        void Execute(ShellContext context, string[] args);
    }
}
