// <copyright file="LineBufferTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Test
{
    using System;
    using BigDrive.Shell.LineInput;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests for the LineBuffer class.
    /// </summary>
    [TestClass]
    public class LineBufferTests
    {
        private LineBuffer m_buffer;
        private TestConsoleOperations m_console;

        /// <summary>
        /// Initializes the test environment.
        /// </summary>
        [TestInitialize]
        public void TestInitialize()
        {
            m_console = new TestConsoleOperations();
            m_buffer = new LineBuffer(promptLength: 5, console: m_console);
        }

        /// <summary>
        /// Cleans up the test environment.
        /// </summary>
        [TestCleanup]
        public void TestCleanup()
        {
            m_buffer = null;
        }

        /// <summary>
        /// Tests that new buffer is empty.
        /// </summary>
        [TestMethod]
        public void Constructor_CreatesEmptyBuffer()
        {
            Assert.AreEqual(string.Empty, m_buffer.GetText());
            Assert.AreEqual(0, m_buffer.Length);
            Assert.AreEqual(0, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that Insert adds character at cursor.
        /// </summary>
        [TestMethod]
        public void Insert_AddsCharacterAtCursor()
        {
            m_buffer.Insert('h');
            m_buffer.Insert('i');

            Assert.AreEqual("hi", m_buffer.GetText());
            Assert.AreEqual(2, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that Insert in middle works.
        /// </summary>
        [TestMethod]
        public void Insert_InMiddle_InsertsAtCursor()
        {
            m_buffer.SetText("hllo");
            m_buffer.MoveToStart();
            m_buffer.MoveRight();

            m_buffer.Insert('e');

            Assert.AreEqual("hello", m_buffer.GetText());
            Assert.AreEqual(2, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that Backspace deletes character before cursor.
        /// </summary>
        [TestMethod]
        public void Backspace_DeletesCharacterBeforeCursor()
        {
            m_buffer.SetText("hello");

            bool result = m_buffer.Backspace();

            Assert.IsTrue(result);
            Assert.AreEqual("hell", m_buffer.GetText());
            Assert.AreEqual(4, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that Backspace at start returns false.
        /// </summary>
        [TestMethod]
        public void Backspace_AtStart_ReturnsFalse()
        {
            m_buffer.SetText("hello");
            m_buffer.MoveToStart();

            bool result = m_buffer.Backspace();

            Assert.IsFalse(result);
            Assert.AreEqual("hello", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that Delete removes character at cursor.
        /// </summary>
        [TestMethod]
        public void Delete_RemovesCharacterAtCursor()
        {
            m_buffer.SetText("hello");
            m_buffer.MoveToStart();

            bool result = m_buffer.Delete();

            Assert.IsTrue(result);
            Assert.AreEqual("ello", m_buffer.GetText());
            Assert.AreEqual(0, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that Delete at end returns false.
        /// </summary>
        [TestMethod]
        public void Delete_AtEnd_ReturnsFalse()
        {
            m_buffer.SetText("hello");

            bool result = m_buffer.Delete();

            Assert.IsFalse(result);
            Assert.AreEqual("hello", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that MoveLeft moves cursor.
        /// </summary>
        [TestMethod]
        public void MoveLeft_MovesCursor()
        {
            m_buffer.SetText("hello");

            bool result = m_buffer.MoveLeft();

            Assert.IsTrue(result);
            Assert.AreEqual(4, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that MoveLeft at start returns false.
        /// </summary>
        [TestMethod]
        public void MoveLeft_AtStart_ReturnsFalse()
        {
            m_buffer.SetText("hello");
            m_buffer.MoveToStart();

            bool result = m_buffer.MoveLeft();

            Assert.IsFalse(result);
            Assert.AreEqual(0, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that MoveRight moves cursor.
        /// </summary>
        [TestMethod]
        public void MoveRight_MovesCursor()
        {
            m_buffer.SetText("hello");
            m_buffer.MoveToStart();

            bool result = m_buffer.MoveRight();

            Assert.IsTrue(result);
            Assert.AreEqual(1, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that MoveRight at end returns false.
        /// </summary>
        [TestMethod]
        public void MoveRight_AtEnd_ReturnsFalse()
        {
            m_buffer.SetText("hello");

            bool result = m_buffer.MoveRight();

            Assert.IsFalse(result);
            Assert.AreEqual(5, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that MoveToStart moves cursor to beginning.
        /// </summary>
        [TestMethod]
        public void MoveToStart_MovesCursorToBeginning()
        {
            m_buffer.SetText("hello");

            m_buffer.MoveToStart();

            Assert.AreEqual(0, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that MoveToEnd moves cursor to end.
        /// </summary>
        [TestMethod]
        public void MoveToEnd_MovesCursorToEnd()
        {
            m_buffer.SetText("hello");
            m_buffer.MoveToStart();

            m_buffer.MoveToEnd();

            Assert.AreEqual(5, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that Clear empties buffer.
        /// </summary>
        [TestMethod]
        public void Clear_EmptiesBuffer()
        {
            m_buffer.SetText("hello");

            m_buffer.Clear();

            Assert.AreEqual(string.Empty, m_buffer.GetText());
            Assert.AreEqual(0, m_buffer.Length);
            Assert.AreEqual(0, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that SetText replaces content.
        /// </summary>
        [TestMethod]
        public void SetText_ReplacesContent()
        {
            m_buffer.SetText("hello");

            m_buffer.SetText("world");

            Assert.AreEqual("world", m_buffer.GetText());
            Assert.AreEqual(5, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that SetText with null clears buffer.
        /// </summary>
        [TestMethod]
        public void SetText_Null_ClearsBuffer()
        {
            m_buffer.SetText("hello");

            m_buffer.SetText(null);

            Assert.AreEqual(string.Empty, m_buffer.GetText());
            Assert.AreEqual(0, m_buffer.Length);
        }

        /// <summary>
        /// Tests that SetText with empty string clears buffer.
        /// </summary>
        [TestMethod]
        public void SetText_EmptyString_ClearsBuffer()
        {
            m_buffer.SetText("hello");

            m_buffer.SetText(string.Empty);

            Assert.AreEqual(string.Empty, m_buffer.GetText());
            Assert.AreEqual(0, m_buffer.Length);
        }

        /// <summary>
        /// Tests that PromptLength is preserved.
        /// </summary>
        [TestMethod]
        public void PromptLength_ReturnsConstructorValue()
        {
            Assert.AreEqual(5, m_buffer.PromptLength);
        }
    }
}
