// <copyright file="LineBuffer.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.LineInput
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// Encapsulates the state of a line being edited in the console.
    /// Provides methods for manipulating the buffer and rendering to console.
    /// </summary>
    public class LineBuffer
    {
        /// <summary>
        /// The character buffer.
        /// </summary>
        private readonly List<char> m_buffer;

        /// <summary>
        /// The prompt length for calculating cursor positions.
        /// </summary>
        private readonly int m_promptLength;

        /// <summary>
        /// The current cursor position within the buffer.
        /// </summary>
        private int m_cursorPosition;

        /// <summary>
        /// Initializes a new instance of the <see cref="LineBuffer"/> class.
        /// </summary>
        /// <param name="promptLength">The length of the prompt (for cursor positioning).</param>
        public LineBuffer(int promptLength)
        {
            m_buffer = new List<char>();
            m_promptLength = promptLength;
            m_cursorPosition = 0;
        }

        /// <summary>
        /// Gets the current cursor position within the buffer.
        /// </summary>
        public int CursorPosition
        {
            get { return m_cursorPosition; }
        }

        /// <summary>
        /// Gets the length of the buffer.
        /// </summary>
        public int Length
        {
            get { return m_buffer.Count; }
        }

        /// <summary>
        /// Gets the prompt length.
        /// </summary>
        public int PromptLength
        {
            get { return m_promptLength; }
        }

        /// <summary>
        /// Gets the current text in the buffer.
        /// </summary>
        /// <returns>The buffer contents as a string.</returns>
        public string GetText()
        {
            return new string(m_buffer.ToArray());
        }

        /// <summary>
        /// Inserts a character at the current cursor position.
        /// </summary>
        /// <param name="c">The character to insert.</param>
        public void Insert(char c)
        {
            m_buffer.Insert(m_cursorPosition, c);
            m_cursorPosition++;
            Redraw();
        }

        /// <summary>
        /// Deletes the character before the cursor (backspace).
        /// </summary>
        /// <returns>True if a character was deleted.</returns>
        public bool Backspace()
        {
            if (m_cursorPosition > 0)
            {
                m_buffer.RemoveAt(m_cursorPosition - 1);
                m_cursorPosition--;
                Redraw();
                return true;
            }

            return false;
        }

        /// <summary>
        /// Deletes the character at the cursor position.
        /// </summary>
        /// <returns>True if a character was deleted.</returns>
        public bool Delete()
        {
            if (m_cursorPosition < m_buffer.Count)
            {
                m_buffer.RemoveAt(m_cursorPosition);
                Redraw();
                return true;
            }

            return false;
        }

        /// <summary>
        /// Moves the cursor left one position.
        /// </summary>
        /// <returns>True if the cursor moved.</returns>
        public bool MoveLeft()
        {
            if (m_cursorPosition > 0)
            {
                m_cursorPosition--;
                Console.SetCursorPosition(m_promptLength + m_cursorPosition, Console.CursorTop);
                return true;
            }

            return false;
        }

        /// <summary>
        /// Moves the cursor right one position.
        /// </summary>
        /// <returns>True if the cursor moved.</returns>
        public bool MoveRight()
        {
            if (m_cursorPosition < m_buffer.Count)
            {
                m_cursorPosition++;
                Console.SetCursorPosition(m_promptLength + m_cursorPosition, Console.CursorTop);
                return true;
            }

            return false;
        }

        /// <summary>
        /// Moves the cursor to the beginning of the line.
        /// </summary>
        public void MoveToStart()
        {
            m_cursorPosition = 0;
            Console.SetCursorPosition(m_promptLength, Console.CursorTop);
        }

        /// <summary>
        /// Moves the cursor to the end of the line.
        /// </summary>
        public void MoveToEnd()
        {
            m_cursorPosition = m_buffer.Count;
            Console.SetCursorPosition(m_promptLength + m_cursorPosition, Console.CursorTop);
        }

        /// <summary>
        /// Clears the buffer and resets the cursor.
        /// </summary>
        public void Clear()
        {
            int oldLength = m_buffer.Count;
            m_buffer.Clear();
            m_cursorPosition = 0;

            // Clear the display
            Console.SetCursorPosition(m_promptLength, Console.CursorTop);
            Console.Write(new string(' ', oldLength));
            Console.SetCursorPosition(m_promptLength, Console.CursorTop);
        }

        /// <summary>
        /// Replaces the entire buffer contents with new text.
        /// </summary>
        /// <param name="text">The new text.</param>
        public void SetText(string text)
        {
            m_buffer.Clear();
            if (text != null)
            {
                foreach (char c in text)
                {
                    m_buffer.Add(c);
                }
            }

            m_cursorPosition = m_buffer.Count;
            Redraw();
        }

        /// <summary>
        /// Redraws the buffer to the console.
        /// </summary>
        public void Redraw()
        {
            // Move to start of input area
            Console.SetCursorPosition(m_promptLength, Console.CursorTop);

            // Write the buffer
            string text = GetText();
            Console.Write(text);

            // Clear any remaining characters from previous longer text
            int clearCount = Console.BufferWidth - m_promptLength - text.Length - 1;
            if (clearCount > 0)
            {
                Console.Write(new string(' ', clearCount));
            }

            // Set cursor to correct position
            Console.SetCursorPosition(m_promptLength + m_cursorPosition, Console.CursorTop);
        }
    }
}
