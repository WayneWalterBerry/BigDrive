// <copyright file="CompletionKeyHandler.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.LineInput
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    using BigDrive.Interfaces;
    using BigDrive.Shell.Commands;

    /// <summary>
    /// Handles Tab/Shift+Tab keys for command and path completion.
    /// </summary>
    public class CompletionKeyHandler : IKeyHandler
    {
        /// <summary>
        /// The shell context for accessing drive and path information.
        /// </summary>
        private readonly ShellContext m_context;

        /// <summary>
        /// Dictionary mapping command names to command instances.
        /// </summary>
        private readonly Dictionary<string, ICommand> m_commands;

        /// <summary>
        /// The current completion candidates.
        /// </summary>
        private List<string> m_completionCandidates;

        /// <summary>
        /// The current index in the completion candidates list.
        /// </summary>
        private int m_completionIndex;

        /// <summary>
        /// The original text before any completion was applied.
        /// </summary>
        private string m_originalText;

        /// <summary>
        /// The cursor position when completion started.
        /// </summary>
        private int m_completionStartPosition;

        /// <summary>
        /// Whether we are currently in completion mode.
        /// </summary>
        private bool m_inCompletionMode;

        /// <summary>
        /// Initializes a new instance of the <see cref="CompletionKeyHandler"/> class.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="commands">The registered commands.</param>
        public CompletionKeyHandler(ShellContext context, Dictionary<string, ICommand> commands)
        {
            m_context = context;
            m_commands = commands;
            m_completionCandidates = new List<string>();
            m_completionIndex = 0;
            m_originalText = string.Empty;
            m_completionStartPosition = 0;
            m_inCompletionMode = false;
        }

        /// <summary>
        /// Attempts to handle the specified key press.
        /// </summary>
        /// <param name="keyInfo">The key that was pressed.</param>
        /// <param name="buffer">The line buffer to modify.</param>
        /// <returns>True if the key was handled.</returns>
        public bool HandleKey(ConsoleKeyInfo keyInfo, LineBuffer buffer)
        {
            if (keyInfo.Key != ConsoleKey.Tab)
            {
                // Any non-Tab key exits completion mode
                if (m_inCompletionMode)
                {
                    Reset();
                }

                return false;
            }

            bool reverse = (keyInfo.Modifiers & ConsoleModifiers.Shift) != 0;
            HandleTab(buffer, reverse);
            return true;
        }

        /// <summary>
        /// Resets the completion state.
        /// </summary>
        public void Reset()
        {
            m_inCompletionMode = false;
            m_completionCandidates.Clear();
            m_completionIndex = 0;
            m_originalText = string.Empty;
            m_completionStartPosition = 0;
        }

        /// <summary>
        /// Handles tab completion.
        /// </summary>
        /// <param name="buffer">The line buffer.</param>
        /// <param name="reverse">True if Shift+Tab was pressed.</param>
        private void HandleTab(LineBuffer buffer, bool reverse)
        {
            string currentText = buffer.GetText();

            if (!m_inCompletionMode)
            {
                // Start new completion
                m_originalText = currentText;
                m_completionStartPosition = buffer.CursorPosition;
                m_completionCandidates = GetCompletionCandidates(currentText, buffer.CursorPosition);
                m_completionIndex = reverse ? m_completionCandidates.Count - 1 : 0;
                m_inCompletionMode = true;
            }
            else
            {
                // Cycle through candidates
                if (m_completionCandidates.Count > 0)
                {
                    if (reverse)
                    {
                        m_completionIndex--;
                        if (m_completionIndex < 0)
                        {
                            m_completionIndex = m_completionCandidates.Count - 1;
                        }
                    }
                    else
                    {
                        m_completionIndex++;
                        if (m_completionIndex >= m_completionCandidates.Count)
                        {
                            m_completionIndex = 0;
                        }
                    }
                }
            }

            if (m_completionCandidates.Count > 0)
            {
                ApplyCompletion(buffer);
            }
        }

        /// <summary>
        /// Applies the current completion candidate to the buffer.
        /// </summary>
        /// <param name="buffer">The line buffer.</param>
        private void ApplyCompletion(LineBuffer buffer)
        {
            string completion = m_completionCandidates[m_completionIndex];

            // Find the word being completed
            int wordStart = FindWordStart(m_originalText, m_completionStartPosition);

            // Build the new text
            string prefix = wordStart > 0 ? m_originalText.Substring(0, wordStart) : string.Empty;
            string suffix = m_completionStartPosition < m_originalText.Length
                ? m_originalText.Substring(m_completionStartPosition)
                : string.Empty;

            // Check if completion needs quotes (contains spaces)
            string completionText = completion;
            if (completion.Contains(" ") && !completion.StartsWith("\""))
            {
                completionText = "\"" + completion + "\"";
            }

            string newText = prefix + completionText + suffix;
            buffer.SetText(newText);

            // Position cursor at end of completion (before suffix)
            // This is a simplification - SetText moves cursor to end
        }

        /// <summary>
        /// Gets completion candidates for the current input.
        /// </summary>
        /// <param name="text">The current input text.</param>
        /// <param name="cursorPosition">The cursor position.</param>
        /// <returns>List of completion candidates.</returns>
        private List<string> GetCompletionCandidates(string text, int cursorPosition)
        {
            List<string> candidates = new List<string>();

            string[] parts = ParseCommandLineParts(text);
            int wordStart = FindWordStart(text, cursorPosition);
            string currentWord = ExtractCurrentWord(text, wordStart, cursorPosition);

            if (parts.Length == 0 || (parts.Length == 1 && cursorPosition <= text.Length && !text.TrimEnd().EndsWith(" ")))
            {
                // Completing the command name
                candidates = GetCommandCompletions(currentWord);
            }
            else
            {
                // Completing an argument (file/folder name or drive letter)
                candidates = GetPathCompletions(currentWord);
            }

            return candidates.OrderBy(c => c, StringComparer.OrdinalIgnoreCase).ToList();
        }

        /// <summary>
        /// Gets command name completions.
        /// </summary>
        /// <param name="prefix">The current prefix to match.</param>
        /// <returns>Matching command names.</returns>
        private List<string> GetCommandCompletions(string prefix)
        {
            HashSet<string> uniqueCommands = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            foreach (var kvp in m_commands)
            {
                if (kvp.Key.Equals(kvp.Value.Name, StringComparison.OrdinalIgnoreCase))
                {
                    if (string.IsNullOrEmpty(prefix) || kvp.Key.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
                    {
                        uniqueCommands.Add(kvp.Key);
                    }
                }
            }

            return uniqueCommands.ToList();
        }

        /// <summary>
        /// Gets path completions (files, folders, and drive letters).
        /// </summary>
        /// <param name="prefix">The current prefix to match.</param>
        /// <returns>Matching paths.</returns>
        private List<string> GetPathCompletions(string prefix)
        {
            List<string> candidates = new List<string>();

            // Add drive letter completions
            if (prefix.Length <= 2 && (prefix.Length == 0 || char.IsLetter(prefix[0])))
            {
                foreach (char letter in m_context.DriveLetterManager.BigDriveLetters.Keys)
                {
                    string drivePath = letter + ":";
                    if (string.IsNullOrEmpty(prefix) || drivePath.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
                    {
                        candidates.Add(drivePath);
                    }
                }
            }

            // Get file/folder completions from provider
            if (m_context.CurrentDriveGuid.HasValue)
            {
                try
                {
                    IBigDriveEnumerate enumerate = ProviderFactory.GetEnumerateProvider(m_context.CurrentDriveGuid.Value);
                    if (enumerate != null)
                    {
                        ParsePathForCompletion(prefix, out string searchPath, out string searchPrefix);

                        string[] folders = enumerate.EnumerateFolders(m_context.CurrentDriveGuid.Value, searchPath);
                        foreach (string folder in folders)
                        {
                            if (string.IsNullOrEmpty(searchPrefix) || folder.StartsWith(searchPrefix, StringComparison.OrdinalIgnoreCase))
                            {
                                string fullPath = CombinePath(searchPath, folder);
                                candidates.Add(StripLeadingSlash(fullPath));
                            }
                        }

                        string[] files = enumerate.EnumerateFiles(m_context.CurrentDriveGuid.Value, searchPath);
                        foreach (string file in files)
                        {
                            if (string.IsNullOrEmpty(searchPrefix) || file.StartsWith(searchPrefix, StringComparison.OrdinalIgnoreCase))
                            {
                                string fullPath = CombinePath(searchPath, file);
                                candidates.Add(StripLeadingSlash(fullPath));
                            }
                        }
                    }
                }
                catch
                {
                    // Ignore errors during completion
                }
            }

            return candidates;
        }

        /// <summary>
        /// Parses a path prefix for completion.
        /// </summary>
        private void ParsePathForCompletion(string prefix, out string searchPath, out string searchPrefix)
        {
            if (string.IsNullOrEmpty(prefix))
            {
                searchPath = m_context.CurrentPath;
                searchPrefix = string.Empty;
                return;
            }

            int lastSeparator = prefix.LastIndexOfAny(new[] { '\\', '/' });

            if (lastSeparator >= 0)
            {
                string dirPart = prefix.Substring(0, lastSeparator);
                searchPrefix = prefix.Substring(lastSeparator + 1);

                if (dirPart.StartsWith("\\") || dirPart.StartsWith("/"))
                {
                    searchPath = dirPart;
                }
                else
                {
                    searchPath = CombinePath(m_context.CurrentPath, dirPart);
                }
            }
            else
            {
                searchPath = m_context.CurrentPath;
                searchPrefix = prefix;
            }
        }

        /// <summary>
        /// Combines two path segments.
        /// </summary>
        private static string CombinePath(string basePath, string relativePath)
        {
            if (string.IsNullOrEmpty(basePath) || basePath == "\\")
            {
                return "\\" + relativePath;
            }

            return basePath.TrimEnd('\\') + "\\" + relativePath;
        }

        /// <summary>
        /// Strips the leading slash from a path.
        /// </summary>
        private static string StripLeadingSlash(string path)
        {
            if (path.StartsWith("\\"))
            {
                return path.Substring(1);
            }

            return path;
        }

        /// <summary>
        /// Finds the start position of the word containing the cursor.
        /// </summary>
        private static int FindWordStart(string text, int cursorPosition)
        {
            if (cursorPosition == 0 || string.IsNullOrEmpty(text))
            {
                return 0;
            }

            int pos = Math.Min(cursorPosition - 1, text.Length - 1);
            bool inQuotes = false;

            while (pos >= 0)
            {
                char c = text[pos];

                if (c == '"')
                {
                    inQuotes = !inQuotes;
                }
                else if (c == ' ' && !inQuotes)
                {
                    return pos + 1;
                }

                pos--;
            }

            return 0;
        }

        /// <summary>
        /// Extracts the current word from the text.
        /// </summary>
        private static string ExtractCurrentWord(string text, int wordStart, int cursorPosition)
        {
            if (string.IsNullOrEmpty(text) || wordStart >= text.Length)
            {
                return string.Empty;
            }

            int length = cursorPosition - wordStart;
            if (length <= 0)
            {
                return string.Empty;
            }

            return text.Substring(wordStart, Math.Min(length, text.Length - wordStart)).Trim('"');
        }

        /// <summary>
        /// Parses the command line into parts.
        /// </summary>
        private static string[] ParseCommandLineParts(string commandLine)
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
