// <copyright file="HistoryNavigationKeyHandlerTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Test
{
    using System;
    using BigDrive.Shell.LineInput;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests for the HistoryNavigationKeyHandler class.
    /// </summary>
    [TestClass]
    public class HistoryNavigationKeyHandlerTests
    {
        private HistoryNavigationKeyHandler m_handler;
        private CommandHistory m_history;
        private LineBuffer m_buffer;
        private TestConsoleOperations m_console;

        /// <summary>
        /// Initializes the test environment.
        /// </summary>
        [TestInitialize]
        public void TestInitialize()
        {
            m_history = new CommandHistory();
            m_handler = new HistoryNavigationKeyHandler(m_history);
            m_console = new TestConsoleOperations();
            m_buffer = new LineBuffer(promptLength: 5, console: m_console);
        }

        /// <summary>
        /// Cleans up the test environment.
        /// </summary>
        [TestCleanup]
        public void TestCleanup()
        {
            m_handler = null;
            m_history = null;
            m_buffer = null;
        }

        /// <summary>
        /// Tests that constructor throws on null history.
        /// </summary>
        [TestMethod]
        [ExpectedException(typeof(ArgumentNullException))]
        public void Constructor_NullHistory_ThrowsArgumentNullException()
        {
            new HistoryNavigationKeyHandler(null);
        }

        /// <summary>
        /// Tests that UpArrow with empty history does nothing.
        /// </summary>
        [TestMethod]
        public void HandleKey_UpArrow_EmptyHistory_DoesNothing()
        {
            m_buffer.SetText("current");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.UpArrow, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual("current", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that UpArrow retrieves previous command.
        /// </summary>
        [TestMethod]
        public void HandleKey_UpArrow_RetrievesPreviousCommand()
        {
            m_history.Add("command1");
            m_history.Add("command2");
            m_buffer.SetText("current");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.UpArrow, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual("command2", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that multiple UpArrow presses navigate backwards.
        /// </summary>
        [TestMethod]
        public void HandleKey_MultipleUpArrows_NavigatesBackwards()
        {
            m_history.Add("command1");
            m_history.Add("command2");
            m_history.Add("command3");
            m_buffer.SetText("current");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.UpArrow, false, false, false);

            m_handler.HandleKey(keyInfo, m_buffer);
            Assert.AreEqual("command3", m_buffer.GetText());

            m_handler.HandleKey(keyInfo, m_buffer);
            Assert.AreEqual("command2", m_buffer.GetText());

            m_handler.HandleKey(keyInfo, m_buffer);
            Assert.AreEqual("command1", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that UpArrow stops at oldest command.
        /// </summary>
        [TestMethod]
        public void HandleKey_UpArrow_StopsAtOldest()
        {
            m_history.Add("command1");
            m_history.Add("command2");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.UpArrow, false, false, false);

            m_handler.HandleKey(keyInfo, m_buffer);
            m_handler.HandleKey(keyInfo, m_buffer);
            m_handler.HandleKey(keyInfo, m_buffer);
            m_handler.HandleKey(keyInfo, m_buffer);

            Assert.AreEqual("command1", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that DownArrow without navigation does nothing.
        /// </summary>
        [TestMethod]
        public void HandleKey_DownArrow_NotNavigating_DoesNothing()
        {
            m_buffer.SetText("current");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.DownArrow, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual("current", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that DownArrow navigates forward.
        /// </summary>
        [TestMethod]
        public void HandleKey_DownArrow_NavigatesForward()
        {
            m_history.Add("command1");
            m_history.Add("command2");
            m_history.Add("command3");
            m_buffer.SetText("current");

            ConsoleKeyInfo upKey = new ConsoleKeyInfo('\0', ConsoleKey.UpArrow, false, false, false);
            ConsoleKeyInfo downKey = new ConsoleKeyInfo('\0', ConsoleKey.DownArrow, false, false, false);

            m_handler.HandleKey(upKey, m_buffer);
            m_handler.HandleKey(upKey, m_buffer);
            Assert.AreEqual("command2", m_buffer.GetText());

            m_handler.HandleKey(downKey, m_buffer);
            Assert.AreEqual("command3", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that DownArrow restores saved input.
        /// </summary>
        [TestMethod]
        public void HandleKey_DownArrow_RestoresSavedInput()
        {
            m_history.Add("command1");
            m_history.Add("command2");
            m_buffer.SetText("current");

            ConsoleKeyInfo upKey = new ConsoleKeyInfo('\0', ConsoleKey.UpArrow, false, false, false);
            ConsoleKeyInfo downKey = new ConsoleKeyInfo('\0', ConsoleKey.DownArrow, false, false, false);

            m_handler.HandleKey(upKey, m_buffer);
            Assert.AreEqual("command2", m_buffer.GetText());

            m_handler.HandleKey(downKey, m_buffer);
            Assert.AreEqual("current", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that first UpArrow saves current input.
        /// </summary>
        [TestMethod]
        public void HandleKey_FirstUpArrow_SavesCurrentInput()
        {
            m_history.Add("old");
            m_buffer.SetText("new input");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.UpArrow, false, false, false);

            m_handler.HandleKey(keyInfo, m_buffer);

            Assert.AreEqual("new input", m_history.SavedInput);
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
