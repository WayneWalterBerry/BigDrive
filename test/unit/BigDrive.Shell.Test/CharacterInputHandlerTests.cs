// <copyright file="CharacterInputHandlerTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Test
{
    using System;
    using BigDrive.Shell.LineInput;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests for the CharacterInputHandler class.
    /// </summary>
    [TestClass]
    public class CharacterInputHandlerTests
    {
        private CharacterInputHandler m_handler;
        private LineBuffer m_buffer;
        private TestConsoleOperations m_console;
        private bool m_callbackInvoked;

        /// <summary>
        /// Initializes the test environment.
        /// </summary>
        [TestInitialize]
        public void TestInitialize()
        {
            m_callbackInvoked = false;
            m_handler = new CharacterInputHandler(() => m_callbackInvoked = true);
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
            m_buffer = null;
        }

        /// <summary>
        /// Tests that printable characters are inserted.
        /// </summary>
        [TestMethod]
        public void HandleKey_PrintableCharacter_InsertsCharacter()
        {
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('a', ConsoleKey.A, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual("a", m_buffer.GetText());
            Assert.AreEqual(1, m_buffer.CursorPosition);
        }

        /// <summary>
        /// Tests that multiple characters are inserted in sequence.
        /// </summary>
        [TestMethod]
        public void HandleKey_MultipleCharacters_BuildsText()
        {
            m_handler.HandleKey(new ConsoleKeyInfo('h', ConsoleKey.H, false, false, false), m_buffer);
            m_handler.HandleKey(new ConsoleKeyInfo('i', ConsoleKey.I, false, false, false), m_buffer);

            Assert.AreEqual("hi", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that character insertion invokes callback.
        /// </summary>
        [TestMethod]
        public void HandleKey_PrintableCharacter_InvokesCallback()
        {
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('x', ConsoleKey.X, false, false, false);

            m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(m_callbackInvoked);
        }

        /// <summary>
        /// Tests that handler works without callback.
        /// </summary>
        [TestMethod]
        public void HandleKey_NoCallback_InsertsCharacter()
        {
            CharacterInputHandler handlerWithoutCallback = new CharacterInputHandler(null);
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('z', ConsoleKey.Z, false, false, false);

            bool handled = handlerWithoutCallback.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual("z", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that control characters are not handled.
        /// </summary>
        [TestMethod]
        public void HandleKey_ControlCharacter_ReturnsFalse()
        {
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\n', ConsoleKey.Enter, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsFalse(handled);
            Assert.AreEqual(string.Empty, m_buffer.GetText());
        }

        /// <summary>
        /// Tests that space character is inserted.
        /// </summary>
        [TestMethod]
        public void HandleKey_Space_InsertsSpace()
        {
            m_buffer.SetText("hello");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo(' ', ConsoleKey.Spacebar, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            Assert.AreEqual("hello ", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that special characters are inserted.
        /// </summary>
        [TestMethod]
        public void HandleKey_SpecialCharacters_InsertsCharacter()
        {
            m_handler.HandleKey(new ConsoleKeyInfo('!', ConsoleKey.D1, true, false, false), m_buffer);
            m_handler.HandleKey(new ConsoleKeyInfo('@', ConsoleKey.D2, true, false, false), m_buffer);
            m_handler.HandleKey(new ConsoleKeyInfo('#', ConsoleKey.D3, true, false, false), m_buffer);

            Assert.AreEqual("!@#", m_buffer.GetText());
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
