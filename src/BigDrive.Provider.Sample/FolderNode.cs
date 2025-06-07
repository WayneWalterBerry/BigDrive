// <copyright file="ProviderConfigurationFactory.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using System.Collections.Generic;

    public enum NodeType
    {
        File,
        Folder
    }

    /// <summary>
    /// Represents a node in the sample provider's folder structure, supporting files and folders.
    /// </summary>
    public class FolderNode
    {
        /// <summary>
        /// The value of this node.
        /// </summary>
        public string Value { get; set; }

        /// <summary>
        /// The type of this node (File or Folder).
        /// </summary>
        public NodeType Type { get; set; }

        /// <summary>
        /// The child nodes nested under this node.
        /// </summary>
        public List<FolderNode> Children { get; set; }

        /// <summary>
        /// The last modified date of this node.
        /// </summary>
        public DateTime LastModifiedDate { get; set; }

        /// <summary>
        /// The size of this node, in bytes.
        /// </summary>
        public ulong Size { get; set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="FolderNode"/> class.
        /// </summary>
        /// <param name="value">The string value for this node.</param>
        public FolderNode(string value)
        {
            Value = value;
            Children = new List<FolderNode>();
            LastModifiedDate = DateTime.UtcNow;
            Size = 0;
        }
    }
}