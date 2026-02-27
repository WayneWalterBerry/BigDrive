// <copyright file="HistorySearchKeyHandlerTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Test
{
    using System;
    using BigDrive.Shell.LineInput;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests for the HistorySearchKeyHandler class.
    /// </summary>
    [TestClass]
    public class HistorySearchKeyHandlerTests
    {
        private HistorySearchKeyHandler m_handler;
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
            m_handler = new HistorySearchKeyHandler(m_history);
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
            new HistorySearchKeyHandler(null);
        }

        /// <summary>
        /// Tests that F8 with empty history does nothing.
        /// </summary>
        [TestMethod]
        public void HandleKey_F8_EmptyHistory_DoesNothing()
        {
            m_buffer.SetText("search");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.F8, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual("search", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that F8 finds matching command by prefix.
        /// </summary>
        [TestMethod]
        public void HandleKey_F8_FindsMatchingCommand()
        {
            m_history.Add("dir c:\\temp");
            m_history.Add("copy file.txt");
            m_history.Add("dir y:\\folder");
            m_buffer.SetText("dir");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.F8, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual("dir y:\\folder", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that multiple F8 presses cycle through matches.
        /// </summary>
        [TestMethod]
        public void HandleKey_F8_MultiplePress_CyclesThroughMatches()
        {
            m_history.Add("dir c:\\temp");
            m_history.Add("copy file.txt");
            m_history.Add("dir y:\\folder");
            m_buffer.SetText("dir");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.F8, false, false, false);

            m_handler.HandleKey(keyInfo, m_buffer);
            Assert.AreEqual("dir y:\\folder", m_buffer.GetText());

            m_handler.HandleKey(keyInfo, m_buffer);
            Assert.AreEqual("dir c:\\temp", m_buffer.GetText());

            m_handler.HandleKey(keyInfo, m_buffer);
            Assert.AreEqual("dir y:\\folder", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that F8 with no matches does nothing.
        /// </summary>
        [TestMethod]
        public void HandleKey_F8_NoMatch_DoesNothing()
        {
            m_history.Add("dir c:\\temp");
            m_history.Add("copy file.txt");
            m_buffer.SetText("nomatch");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.F8, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual("nomatch", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that F8 is case-insensitive.
        /// </summary>
        [TestMethod]
        public void HandleKey_F8_CaseInsensitive()
        {
            m_history.Add("DIR c:\\temp");
            m_buffer.SetText("dir");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.F8, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual("DIR c:\\temp", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that F8 saves search prefix.
        /// </summary>
        [TestMethod]
        public void HandleKey_F8_SavesSearchPrefix()
        {
            m_history.Add("dir c:\\temp");
            m_buffer.SetText("dir");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.F8, false, false, false);

            m_handler.HandleKey(keyInfo, m_buffer);

            Assert.AreEqual("dir", m_history.SavedInput);
        }

        /// <summary>
        /// Tests that non-F8 keys exit search mode.
        /// </summary>
        [TestMethod]
        public void HandleKey_NonF8Key_ExitsSearchMode()
        {
            m_history.Add("dir c:\\temp");
            m_buffer.SetText("dir");
            ConsoleKeyInfo f8Key = new ConsoleKeyInfo('\0', ConsoleKey.F8, false, false, false);
            ConsoleKeyInfo otherKey = new ConsoleKeyInfo('a', ConsoleKey.A, false, false, false);

            m_handler.HandleKey(f8Key, m_buffer);
            bool handled = m_handler.HandleKey(otherKey, m_buffer);

            Assert.IsFalse(handled);
        }

        /// <summary>
        /// Tests that Reset clears search state.
        /// </summary>
        [TestMethod]
        public void Reset_ClearsSearchState()
        {
            m_history.Add("dir c:\\temp");
            m_buffer.SetText("dir");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.F8, false, false, false);
            m_handler.HandleKey(keyInfo, m_buffer);

            m_handler.Reset();

            m_buffer.SetText("dir");
            m_handler.HandleKey(keyInfo, m_buffer);
            Assert.AreEqual("dir c:\\temp", m_buffer.GetText());
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
    }
}
