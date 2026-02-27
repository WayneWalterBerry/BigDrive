// <copyright file="EditingKeyHandlerTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Test
{
    using System;
    using BigDrive.Shell.LineInput;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests for the EditingKeyHandler class.
    /// </summary>
    [TestClass]
    public class EditingKeyHandlerTests
    {
        private EditingKeyHandler m_handler;
        private LineBuffer m_buffer;
        private TestConsoleOperations m_console;
        private bool m_escapeCalled;

        /// <summary>
        /// Initializes the test environment.
        /// </summary>
        [TestInitialize]
        public void TestInitialize()
        {
            m_escapeCalled = false;
            m_handler = new EditingKeyHandler(() => m_escapeCalled = true);
            m_console = new TestConsoleOperations();
            m_buffer = new LineBuffer(promptLength: 5, console: m_console);
            m_buffer.SetText("hello");
        }

        /// <summary>
        /// Cleans up the test environment.
        /// </summary>
        [TestCleanup]
        public void TestCleanup()
        {
            m_handler = null;
            m_buffer = null;
        }

        /// <summary>
        /// Tests that Backspace deletes character before cursor.
        /// </summary>
        [TestMethod]
        public void HandleKey_Backspace_DeletesCharacterBeforeCursor()
        {
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\b', ConsoleKey.Backspace, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual("hell", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that Delete deletes character at cursor.
        /// </summary>
        [TestMethod]
        public void HandleKey_Delete_DeletesCharacterAtCursor()
        {
            m_buffer.MoveToStart();
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.Delete, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual("ello", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that Escape clears the buffer.
        /// </summary>
        [TestMethod]
        public void HandleKey_Escape_ClearsBuffer()
        {
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\u001b', ConsoleKey.Escape, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual(string.Empty, m_buffer.GetText());
        }

        /// <summary>
        /// Tests that Escape invokes callback.
        /// </summary>
        [TestMethod]
        public void HandleKey_Escape_InvokesCallback()
        {
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\u001b', ConsoleKey.Escape, false, false, false);

            m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(m_escapeCalled);
        }

        /// <summary>
        /// Tests that Escape works without callback.
        /// </summary>
        [TestMethod]
        public void HandleKey_Escape_WorksWithoutCallback()
        {
            EditingKeyHandler handlerWithoutCallback = new EditingKeyHandler(null);
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\u001b', ConsoleKey.Escape, false, false, false);

            bool handled = handlerWithoutCallback.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual(string.Empty, m_buffer.GetText());
        }

        /// <summary>
        /// Tests that unhandled keys return false.
        /// </summary>
        [TestMethod]
        public void HandleKey_UnhandledKey_ReturnsFalse()
        {
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('a', ConsoleKey.A, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsFalse(handled);
        }

        /// <summary>
        /// Tests that Reset does not throw.
        /// </summary>
        [TestMethod]
        public void Reset_NoState_DoesNotThrow()
        {
            m_handler.Reset();
        }
    }
}
