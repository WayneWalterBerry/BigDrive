// <copyright file="CompletionKeyHandlerTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Test
{
    using System;
    using System.Collections.Generic;
    using BigDrive.Shell.Commands;
    using BigDrive.Shell.LineInput;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests for the CompletionKeyHandler class.
    /// </summary>
    [TestClass]
    public class CompletionKeyHandlerTests
    {
        private CompletionKeyHandler m_handler;
        private ShellContext m_context;
        private Dictionary<string, ICommand> m_commands;
        private LineBuffer m_buffer;
        private TestConsoleOperations m_console;

        /// <summary>
        /// Initializes the test environment.
        /// </summary>
        [TestInitialize]
        public void TestInitialize()
        {
            m_context = new ShellContext();
            m_commands = new Dictionary<string, ICommand>(StringComparer.OrdinalIgnoreCase)
            {
                { "dir", new DirCommand() },
                { "copy", new CopyCommand() },
                { "del", new DelCommand() }
            };
            m_commands.Add("help", new HelpCommand(m_commands));
            m_handler = new CompletionKeyHandler(m_context, m_commands);
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
            m_context = null;
            m_commands = null;
            m_buffer = null;
        }

        /// <summary>
        /// Tests that Tab key is handled.
        /// </summary>
        [TestMethod]
        public void HandleKey_Tab_ReturnsTrue()
        {
            m_buffer.SetText("di");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\t', ConsoleKey.Tab, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
        }

        /// <summary>
        /// Tests that Tab completes command names.
        /// </summary>
        [TestMethod]
        public void HandleKey_Tab_CompletesCommandName()
        {
            m_buffer.SetText("di");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\t', ConsoleKey.Tab, false, false, false);

            m_handler.HandleKey(keyInfo, m_buffer);

            Assert.AreEqual("dir", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that multiple Tab presses cycle through completions.
        /// </summary>
        [TestMethod]
        public void HandleKey_MultipleTabs_CyclesThroughCompletions()
        {
            m_buffer.SetText("d");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\t', ConsoleKey.Tab, false, false, false);

            m_handler.HandleKey(keyInfo, m_buffer);
            string first = m_buffer.GetText();
            Assert.IsTrue(first == "del" || first == "dir");

            m_handler.HandleKey(keyInfo, m_buffer);
            string second = m_buffer.GetText();
            Assert.IsTrue(second == "del" || second == "dir");
            Assert.AreNotEqual(first, second);

            m_handler.HandleKey(keyInfo, m_buffer);
            string third = m_buffer.GetText();
            Assert.AreEqual(first, third);
        }

        /// <summary>
        /// Tests that Shift+Tab cycles backwards.
        /// </summary>
        [TestMethod]
        public void HandleKey_ShiftTab_CyclesBackwards()
        {
            m_buffer.SetText("d");
            ConsoleKeyInfo tabKey = new ConsoleKeyInfo('\t', ConsoleKey.Tab, false, false, false);
            ConsoleKeyInfo shiftTabKey = new ConsoleKeyInfo('\t', ConsoleKey.Tab, true, false, false);

            m_handler.HandleKey(tabKey, m_buffer);
            string first = m_buffer.GetText();

            m_handler.HandleKey(shiftTabKey, m_buffer);
            string second = m_buffer.GetText();

            Assert.AreNotEqual(first, second);
        }

        /// <summary>
        /// Tests that wildcard command completion works.
        /// </summary>
        [TestMethod]
        public void HandleKey_Tab_WildcardCommandCompletion()
        {
            m_buffer.SetText("d*l");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\t', ConsoleKey.Tab, false, false, false);

            m_handler.HandleKey(keyInfo, m_buffer);

            Assert.AreEqual("del", m_buffer.GetText());
        }

        /// <summary>
        /// Tests that non-Tab keys exit completion mode.
        /// </summary>
        [TestMethod]
        public void HandleKey_NonTabKey_ExitsCompletionMode()
        {
            m_buffer.SetText("d");
            ConsoleKeyInfo tabKey = new ConsoleKeyInfo('\t', ConsoleKey.Tab, false, false, false);
            ConsoleKeyInfo otherKey = new ConsoleKeyInfo('i', ConsoleKey.I, false, false, false);

            m_handler.HandleKey(tabKey, m_buffer);
            bool handled = m_handler.HandleKey(otherKey, m_buffer);

            Assert.IsFalse(handled);
        }

        /// <summary>
        /// Tests that Reset clears completion state.
        /// </summary>
        [TestMethod]
        public void Reset_ClearsCompletionState()
        {
            m_buffer.SetText("d");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\t', ConsoleKey.Tab, false, false, false);
            
            m_handler.HandleKey(keyInfo, m_buffer);
            m_handler.Reset();
            m_buffer.SetText("d");
            m_handler.HandleKey(keyInfo, m_buffer);

            string result = m_buffer.GetText();
            Assert.IsTrue(result == "del" || result == "dir");
        }

        /// <summary>
        /// Tests that completion works with empty prefix.
        /// </summary>
        [TestMethod]
        public void HandleKey_Tab_EmptyPrefix_ShowsAllCommands()
        {
            m_buffer.SetText(string.Empty);
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\t', ConsoleKey.Tab, false, false, false);

            bool handled = m_handler.HandleKey(keyInfo, m_buffer);

            Assert.IsTrue(handled);
            string result = m_buffer.GetText();
            Assert.IsTrue(m_commands.ContainsKey(result));
        }

        /// <summary>
        /// Tests that Tab with no matches does nothing.
        /// </summary>
        [TestMethod]
        public void HandleKey_Tab_NoMatches_DoesNothing()
        {
            m_buffer.SetText("xyz");
            ConsoleKeyInfo keyInfo = new ConsoleKeyInfo('\t', ConsoleKey.Tab, false, false, false);

            m_handler.HandleKey(keyInfo, m_buffer);

            Assert.AreEqual("xyz", m_buffer.GetText());
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
