// <copyright file="NavigationKeyHandlerTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Test
{
    using System;
    using BigDrive.Shell.LineInput;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests for the NavigationKeyHandler class.
    /// </summary>
    [TestClass]
    public class NavigationKeyHandlerTests
    {
        private NavigationKeyHandler m_handler;
        private LineBuffer m_buffer;
        private TestConsoleOperations m_console;

        /// <summary>
        /// Initializes the test environment.
        /// </summary>
        [TestInitialize]
        public void TestInitialize()
        {
            m_handler = new NavigationKeyHandler();
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
        /// Tests that LeftArrow moves cursor left.
        /// </summary>
        [TestMethod]
        public void HandleKey_LeftArrow_MovesCursorLeft()
        {
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.LeftArrow, false, false, false);
            
            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual(4, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that RightArrow moves cursor right.
        /// </summary>
        [TestMethod]
        public void HandleKey_RightArrow_MovesCursorRight()
        {
            m_buffer.MoveToStart();
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.RightArrow, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual(1, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that Home moves cursor to start.
        /// </summary>
        [TestMethod]
        public void HandleKey_Home_MovesCursorToStart()
        {
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.Home, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual(0, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that End moves cursor to end.
        /// </summary>
        [TestMethod]
        public void HandleKey_End_MovesCursorToEnd()
        {
            m_buffer.MoveToStart();
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\0', ConsoleKey.End, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual(5, m_buffer.CursorPosition);
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
