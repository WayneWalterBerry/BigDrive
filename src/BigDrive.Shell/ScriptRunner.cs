// <copyright file="ScriptRunner.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.IO;

    /// <summary>
    /// Executes shell commands from a script file in non-interactive mode.
    /// Supports golden file testing by comparing stdout output to expected results.
    /// </summary>
    public class ScriptRunner
    {
        /// <summary>
        /// The shell context.
        /// </summary>
        private readonly ShellContext m_context;

        /// <summary>
        /// The command processor.
        /// </summary>
        private readonly CommandProcessor m_processor;

        /// <summary>
        /// Initializes a new instance of the <see cref="ScriptRunner"/> class.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="processor">The command processor.</param>
        public ScriptRunner(ShellContext context, CommandProcessor processor)
        {
            m_context = context;
            m_processor = processor;
        }

        /// <summary>
        /// Executes all commands from the specified script file.
        /// Each command is echoed with the shell prompt so the output reads
        /// like a transcript of an interactive session, enabling golden file testing.
        /// </summary>
        /// <param name="scriptPath">Path to the script file containing commands.</param>
        /// <returns>Exit code: 0 for success, non-zero for failure.</returns>
        public int ExecuteScript(string scriptPath)
        {
            if (!File.Exists(scriptPath))
            {
                Console.Error.WriteLine("Error: Script file not found: {0}", scriptPath);
                return 1;
            }

            string[] lines;
            try
            {
                lines = File.ReadAllLines(scriptPath);
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("Error reading script file: {0}", ex.Message);
                return 1;
            }

            int lineNumber = 0;
            foreach (string line in lines)
            {
                lineNumber++;

                // Skip empty lines and comments
                string trimmedLine = line.Trim();
                if (string.IsNullOrEmpty(trimmedLine) || trimmedLine.StartsWith("#"))
                {
                    continue;
                }

                ShellTrace.Verbose("Script line {0}: {1}", lineNumber, trimmedLine);

                // Echo the prompt and command so output reads like an interactive session
                Console.WriteLine("{0}{1}", m_context.GetPrompt(), trimmedLine);

                try
                {
                    m_processor.Execute(trimmedLine);

                    // Check if the command caused an exit
                    if (m_context.ShouldExit)
                    {
                        ShellTrace.Info("Script terminated by exit command at line {0}", lineNumber);
                        return 0;
                    }
                }
                catch (Exception ex)
                {
                    Console.Error.WriteLine("Error at line {0}: {1}", lineNumber, ex.Message);
                    return 1;
                }
            }

            return 0;
        }

        /// <summary>
        /// Executes a single command string.
        /// The command is echoed with the shell prompt so the output reads
        /// like a transcript of an interactive session.
        /// </summary>
        /// <param name="command">The command to execute.</param>
        /// <returns>Exit code: 0 for success, non-zero for failure.</returns>
        public int ExecuteCommand(string command)
        {
            if (string.IsNullOrWhiteSpace(command))
            {
                return 0;
            }

            ShellTrace.Verbose("Executing command: {0}", command);

            // Echo the prompt and command so output reads like an interactive session
            Console.WriteLine("{0}{1}", m_context.GetPrompt(), command.Trim());

            try
            {
                m_processor.Execute(command.Trim());
                return 0;
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("Error: {0}", ex.Message);
                return 1;
            }
        }
    }
}
