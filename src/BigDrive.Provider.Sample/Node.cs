// <copyright file="ProviderConfigurationFactory.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;

    public enum NodeType
    {
        File,
        Folder
    }

    /// <summary>
    /// Represents a node in the sample provider's folder structure, supporting files and folders.
    /// </summary>
    public class Node
    {
        /// <summary>
        /// The value of this node.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// The type of this node (File or Folder).
        /// </summary>
        public NodeType Type { get; set; }

        /// <summary>
        /// The child nodes nested under this node.
        /// </summary>
        public List<Node> Children { get; set; }

        /// <summary>
        /// The last modified date of this node.
        /// </summary>
        public DateTime LastModifiedDate { get; set; }

        /// <summary>
        /// The size of this node, in bytes.
        /// </summary>
        public ulong Size { get; set; }

        /// <summary>
        /// Gets the file data for this node from the embedded resources, if available.
        /// </summary>
        public byte[] Data
        {
            get
            {
                if (Type != NodeType.File || string.IsNullOrEmpty(Name))
                {
                    return null;
                }

                // Get the current assembly
                var assembly = typeof(Node).Assembly;

                // Build the resource name (adjust namespace and folder as needed)
                // Example: "BigDrive.Provider.Sample.Resources.A File.txt"
                string resourceName = $"BigDrive.Provider.Sample.Resources.{Name}";

                using (var stream = assembly.GetManifestResourceStream(resourceName))
                {
                    if (stream == null)
                        return null;

                    using (var ms = new System.IO.MemoryStream())
                    {
                        stream.CopyTo(ms);
                        return ms.ToArray();
                    }
                }
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Node"/> class.
        /// </summary>
        /// <param name="name">The string value for this node.</param>
        public Node(string name)
        {
            Name = name;
            Children = new List<Node>();
            LastModifiedDate = DateTime.UtcNow;
            Size = 0;
        }
    }
}