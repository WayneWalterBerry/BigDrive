// <copyright file="ConsoleLineReader.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.LineInput
{
    using System;
    using System.Collections.Generic;

    using BigDrive.Shell.Commands;

    /// <summary>
    /// Reads a line of input from the console with support for tab completion,
    /// command history, and line editing using the Chain of Responsibility pattern.
    /// </summary>
    public class ConsoleLineReader
    {
        /// <summary>
        /// The shell context.
        /// </summary>
        private readonly ShellContext m_context;

        /// <summary>
        /// The chain of key handlers.
        /// </summary>
        private readonly List<IKeyHandler> m_handlers;

        /// <summary>
        /// The shared command history.
        /// </summary>
        private readonly CommandHistory m_history;

        /// <summary>
        /// The completion key handler.
        /// </summary>
        private readonly CompletionKeyHandler m_completionHandler;

        /// <summary>
        /// Initializes a new instance of the <see cref="ConsoleLineReader"/> class.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="commands">The registered commands for completion.</param>
        public ConsoleLineReader(ShellContext context, Dictionary<string, ICommand> commands)
        {
            m_context = context;
            m_handlers = new List<IKeyHandler>();

            // Create shared command history
            m_history = new CommandHistory();

            // Create handlers
            HistoryNavigationKeyHandler navigationHandler = new HistoryNavigationKeyHandler(m_history);
            HistorySearchKeyHandler searchHandler = new HistorySearchKeyHandler(m_history);
            m_completionHandler = new CompletionKeyHandler(context, commands);

            // Action to reset stateful handlers when input changes
            Action resetStatefulHandlers = () =>
            {
                m_history.ResetNavigation();
                searchHandler.Reset();
                m_completionHandler.Reset();
            };

            // Build the chain of responsibility
            // Order matters: more specific handlers first, fallback last
            m_handlers.Add(navigationHandler);
            m_handlers.Add(searchHandler);
            m_handlers.Add(m_completionHandler);
            m_handlers.Add(new NavigationKeyHandler());
            m_handlers.Add(new EditingKeyHandler(resetStatefulHandlers));
            m_handlers.Add(new CharacterInputHandler(resetStatefulHandlers));
        }

        /// <summary>
        /// Reads a line of input from the console.
        /// </summary>
        /// <returns>The input line.</returns>
        public string ReadLine()
        {
            LineBuffer buffer = new LineBuffer(m_context.GetPrompt().Length);

            // Reset all handlers for new input
            ResetAllHandlers();

            while (true)
            {
                ConsoleKeyInfo keyInfo = Console.ReadKey(intercept: true);

                // Check for Enter key (line complete)
                if (keyInfo.Key == ConsoleKey.Enter)
                {
                    Console.WriteLine();
                    string command = buffer.GetText();

                    // Add to history
                    m_history.Add(command);

                    return command;
                }

                // Pass key through the chain of handlers
                foreach (IKeyHandler handler in m_handlers)
                {
                    if (handler.HandleKey(keyInfo, buffer))
                    {
                        break; // Key was handled, stop chain
                    }
                }
            }
        }

        /// <summary>
        /// Resets all handlers to their initial state.
        /// </summary>
        private void ResetAllHandlers()
        {
            m_history.ResetNavigation();

            foreach (IKeyHandler handler in m_handlers)
            {
                handler.Reset();
            }
        }
    }
}
