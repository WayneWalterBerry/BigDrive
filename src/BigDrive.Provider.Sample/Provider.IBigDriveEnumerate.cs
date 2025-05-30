// <copyright file="Provider.BigDriveRoot.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    public partial class Provider
    {
        public string[] EnumerateFolders(Guid driveGuid, string path)
        {
            return new string[] { "RootFolder1", "RootFolder2", "RootFolder3" };
        }
    }
}
