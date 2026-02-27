// <copyright file="CommandHistoryTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Test
{
    using System;
    using BigDrive.Shell.LineInput;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests for the CommandHistory class.
    /// </summary>
    [TestClass]
    public class CommandHistoryTests
    {
        private CommandHistory m_history;

        /// <summary>
        /// Initializes the test environment.
        /// </summary>
        [TestInitialize]
        public void TestInitialize()
        {
            m_history = new CommandHistory(maxSize: 5);
        }

        /// <summary>
        /// Cleans up the test environment.
        /// </summary>
        [TestCleanup]
        public void TestCleanup()
        {
            m_history = null;
        }

        /// <summary>
        /// Tests that new history is empty.
        /// </summary>
        [TestMethod]
        public void Constructor_CreatesEmptyHistory()
        {
            Assert.AreEqual(0, m_history.Count);
            Assert.IsFalse(m_history.IsNavigating);
            Assert.AreEqual(-1, m_history.CurrentIndex);
        }

        /// <summary>
        /// Tests that Add adds command to history.
        /// </summary>
        [TestMethod]
        public void Add_AddsCommandToHistory()
        {
            m_history.Add("command1");

            Assert.AreEqual(1, m_history.Count);
            Assert.AreEqual("command1", m_history.GetAt(0));
        }

        /// <summary>
        /// Tests that Add ignores null or whitespace.
        /// </summary>
        [TestMethod]
        public void Add_NullOrWhitespace_Ignores()
        {
            m_history.Add(null);
            m_history.Add(string.Empty);
            m_history.Add("   ");

            Assert.AreEqual(0, m_history.Count);
        }

        /// <summary>
        /// Tests that Add ignores duplicate consecutive commands.
        /// </summary>
        [TestMethod]
        public void Add_DuplicateConsecutive_Ignores()
        {
            m_history.Add("command1");
            m_history.Add("command1");

            Assert.AreEqual(1, m_history.Count);
        }

        /// <summary>
        /// Tests that Add allows same command after different command.
        /// </summary>
        [TestMethod]
        public void Add_SameCommandAfterDifferent_Adds()
        {
            m_history.Add("command1");
            m_history.Add("command2");
            m_history.Add("command1");

            Assert.AreEqual(3, m_history.Count);
        }

        /// <summary>
        /// Tests that Add enforces max size.
        /// </summary>
        [TestMethod]
        public void Add_ExceedsMaxSize_RemovesOldest()
        {
            m_history.Add("command1");
            m_history.Add("command2");
            m_history.Add("command3");
            m_history.Add("command4");
            m_history.Add("command5");
            m_history.Add("command6");

            Assert.AreEqual(5, m_history.Count);
            Assert.AreEqual("command2", m_history.GetAt(0));
            Assert.AreEqual("command6", m_history.GetAt(4));
        }

        /// <summary>
        /// Tests that GetAt returns correct entry.
        /// </summary>
        [TestMethod]
        public void GetAt_ReturnsCorrectEntry()
        {
            m_history.Add("first");
            m_history.Add("second");
            m_history.Add("third");

            Assert.AreEqual("first", m_history.GetAt(0));
            Assert.AreEqual("second", m_history.GetAt(1));
            Assert.AreEqual("third", m_history.GetAt(2));
        }

        /// <summary>
        /// Tests that GetAt with invalid index returns empty.
        /// </summary>
        [TestMethod]
        public void GetAt_InvalidIndex_ReturnsEmpty()
        {
            m_history.Add("command");

            Assert.AreEqual(string.Empty, m_history.GetAt(-1));
            Assert.AreEqual(string.Empty, m_history.GetAt(10));
        }

        /// <summary>
        /// Tests that CurrentIndex can be set.
        /// </summary>
        [TestMethod]
        public void CurrentIndex_CanBeSet()
        {
            m_history.CurrentIndex = 3;

            Assert.AreEqual(3, m_history.CurrentIndex);
        }

        /// <summary>
        /// Tests that IsNavigating reflects CurrentIndex.
        /// </summary>
        [TestMethod]
        public void IsNavigating_ReflectsCurrentIndex()
        {
            Assert.IsFalse(m_history.IsNavigating);

            m_history.CurrentIndex = 0;
            Assert.IsTrue(m_history.IsNavigating);

            m_history.CurrentIndex = -1;
            Assert.IsFalse(m_history.IsNavigating);
        }

        /// <summary>
        /// Tests that SavedInput can be set and retrieved.
        /// </summary>
        [TestMethod]
        public void SavedInput_CanBeSetAndRetrieved()
        {
            m_history.SavedInput = "saved text";

            Assert.AreEqual("saved text", m_history.SavedInput);
        }

        /// <summary>
        /// Tests that SavedInput handles null.
        /// </summary>
        [TestMethod]
        public void SavedInput_Null_StoresEmpty()
        {
            m_history.SavedInput = null;

            Assert.AreEqual(string.Empty, m_history.SavedInput);
        }

        /// <summary>
        /// Tests that FindPreviousMatch finds matching entry.
        /// </summary>
        [TestMethod]
        public void FindPreviousMatch_FindsMatch()
        {
            m_history.Add("dir c:\\temp");
            m_history.Add("copy file.txt");
            m_history.Add("dir y:\\folder");

            int index = m_history.FindPreviousMatch("dir", m_history.Count);

            Assert.AreEqual(2, index);
        }

        /// <summary>
        /// Tests that FindPreviousMatch is case-insensitive.
        /// </summary>
        [TestMethod]
        public void FindPreviousMatch_CaseInsensitive()
        {
            m_history.Add("DIR c:\\temp");

            int index = m_history.FindPreviousMatch("dir", m_history.Count);

            Assert.AreEqual(0, index);
        }

        /// <summary>
        /// Tests that FindPreviousMatch wraps around.
        /// </summary>
        [TestMethod]
        public void FindPreviousMatch_WrapsAround()
        {
            m_history.Add("dir first");
            m_history.Add("copy file");
            m_history.Add("dir second");

            int firstMatch = m_history.FindPreviousMatch("dir", m_history.Count);
            Assert.AreEqual(2, firstMatch);

            int secondMatch = m_history.FindPreviousMatch("dir", firstMatch);
            Assert.AreEqual(0, secondMatch);

            int wrapped = m_history.FindPreviousMatch("dir", secondMatch);
            Assert.AreEqual(2, wrapped);
        }

        /// <summary>
        /// Tests that FindPreviousMatch returns -1 when no match.
        /// </summary>
        [TestMethod]
        public void FindPreviousMatch_NoMatch_ReturnsNegative()
        {
            m_history.Add("dir c:\\temp");
            m_history.Add("copy file.txt");

            int index = m_history.FindPreviousMatch("xyz", m_history.Count);

            Assert.AreEqual(-1, index);
        }

        /// <summary>
        /// Tests that ResetNavigation clears navigation state.
        /// </summary>
        [TestMethod]
        public void ResetNavigation_ClearsState()
        {
            m_history.CurrentIndex = 5;
            m_history.SavedInput = "saved";

            m_history.ResetNavigation();

            Assert.AreEqual(-1, m_history.CurrentIndex);
            Assert.AreEqual(string.Empty, m_history.SavedInput);
            Assert.IsFalse(m_history.IsNavigating);
        }
    }
}
