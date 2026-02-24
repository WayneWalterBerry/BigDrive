// <copyright file="Program.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;

    /// <summary>
    /// Entry point for the BigDrive Shell application.
    /// </summary>
    public class Program
    {
        /// <summary>
        /// Main entry point for the shell.
        /// </summary>
        /// <param name="args">Command line arguments.</param>
        public static void Main(string[] args)
        {
            Console.WriteLine("BigDrive Shell v1.0");
            Console.WriteLine("Type 'help' for available commands, 'exit' to quit.");
            Console.WriteLine();

            ShellContext context = new ShellContext();
            CommandProcessor processor = new CommandProcessor(context);

            while (!context.ShouldExit)
            {
                Console.Write(context.GetPrompt());
                string input = Console.ReadLine();

                if (string.IsNullOrWhiteSpace(input))
                {
                    continue;
                }

                processor.Execute(input.Trim());
            }
        }
    }
}
