// <copyright file="Provider.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using BigDrive.Interfaces;
    using System;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;
    using System.Collections.Generic;
    using System.Runtime.InteropServices.ComTypes;

    [Guid("F8FE2E5A-E8B8-4207-BC04-EA4BCD4C4361")] // Unique GUID for the COM class
    [ClassInterface(ClassInterfaceType.None)] // No automatic interface generation
    [ComVisible(true)] // Make the class visible to COM
    public partial class Provider : ServicedComponent,
        IProcessInitializer,
        IBigDriveRegistration,
        IBigDriveEnumerate,
        IBigDriveFileInfo,
        IBigDriveFileOperations,
        IBigDriveFileData
    {
        private static readonly BigDriveTraceSource DefaultTraceSource = BigDriveTraceSource.Instance;

        private static Node root = new Node("//")
        {
            Type = NodeType.Folder
        };

        static Provider()
        {
            InitializeTree();
        }

        public static Guid CLSID
        {
            get
            {
                // Get the type of the Provider class
                Type providerType = typeof(Provider);

                // Get the GuidAttribute applied to the Provider class
                GuidAttribute guidAttribute = (GuidAttribute)Attribute.GetCustomAttribute(providerType, typeof(GuidAttribute));

                return Guid.Parse(guidAttribute.Value);
            }
        }

        /// <summary>
        /// Initializes the root node with all folders and files as a layered structure.
        /// Sets LastModifiedDate for each file node to a weighted random date within the last 2 years.
        /// Sets Size for each file node to a random value up to 4 MB.
        /// </summary>
        private static void InitializeTree()
        {
            root.Children.Clear();

            // Create a random number generator
            Random random = new Random();

            // Define 4MB in bytes = 4 * 1024 * 1024
            ulong fourMB = 4UL * 1024 * 1024;

            // Add root folders
            var rootFolder1 = new Node("RootFolder1") { Type = NodeType.Folder };
            var rootFolder2 = new Node("RootFolder2") { Type = NodeType.Folder };
            var rootFolder3 = new Node("RootFolder3") { Type = NodeType.Folder };
            root.Children.Add(rootFolder1);
            root.Children.Add(rootFolder2);
            root.Children.Add(rootFolder3);

            // Add subfolders
            var subFolder1 = new Node("SubFolder1") { Type = NodeType.Folder };
            rootFolder1.Children.Add(subFolder1);

            var subFolder2 = new Node("SubFolder2") { Type = NodeType.Folder };
            rootFolder2.Children.Add(subFolder2);

            var folder2 = new Node("Folder2") { Type = NodeType.Folder };
            rootFolder2.Children.Add(folder2);

            // Add root files with random LastModifiedDate and random Size
            root.Children.Add(CreateFileNodeWithRandomDateAndSize("A File.txt", random, fourMB));
            root.Children.Add(CreateFileNodeWithRandomDateAndSize("Root File 2.txt", random, fourMB));
            root.Children.Add(CreateFileNodeWithRandomDateAndSize("Z File.txt", random, fourMB));
            root.Children.Add(CreateFileNodeWithRandomDateAndSize("Compact.zip", random, fourMB));
            root.Children.Add(CreateFileNodeWithRandomDateAndSize("Photo.png", random, fourMB));
        }

        /// <summary>
        /// Creates a file node with a weighted random LastModifiedDate within the last 2 years and a random size up to 4 MB.
        /// </summary>
        /// <param name="fileName">The file name for the node.</param>
        /// <param name="random">The random number generator.</param>
        /// <param name="maxSize">The maximum file size in bytes.</param>
        /// <returns>A FolderNode representing a file with a random LastModifiedDate and Size.</returns>
        private static Node CreateFileNodeWithRandomDateAndSize(string fileName, Random random, ulong maxSize)
        {
            var node = new Node(fileName) { Type = NodeType.File };
            node.LastModifiedDate = GenerateWeightedRandomDate();
            double randomFactor = random.NextDouble();
            node.Size = (ulong)(randomFactor * maxSize);
            return node;
        }

        /// <summary>
        /// Generates a weighted random DateTime within the last 2 years, biased toward recent dates.
        /// </summary>
        /// <returns>A DateTime value.</returns>
        private static DateTime GenerateWeightedRandomDate()
        {
            DateTime now = DateTime.Now;
            DateTime twoYearsAgo = now.AddYears(-2);
            Random random = new Random(Guid.NewGuid().GetHashCode()); // More unique seed for each call
            double randomValue = Math.Pow(random.NextDouble(), 2);
            TimeSpan timeSpan = now - twoYearsAgo;
            return now.AddDays(-randomValue * timeSpan.TotalDays);
        }
    }
}