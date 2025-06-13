// <copyright file="ShellHelper.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service
{
    using System;
    using System.Runtime.InteropServices;
    using System.Threading;

    namespace BigDrive.Service
    {
        public static class ShellHelper
        {
            /// <summary>
            /// Notifies the shell to refresh the contents of "My PC" (This PC) specifically.
            /// </summary>
            public static void RefreshMyPC(CancellationToken cancellationToken)
            {
                IntPtr pidlMyComputer = IntPtr.Zero;
                try
                {
                    // CSIDL_DRIVES = 0x11 refers to "My Computer"
                    if (SHGetSpecialFolderLocation(IntPtr.Zero, CSIDL_DRIVES, out pidlMyComputer) == 0 && pidlMyComputer != IntPtr.Zero)
                    {
                        SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidlMyComputer, IntPtr.Zero);
                    }
                }
                finally
                {
                    if (pidlMyComputer != IntPtr.Zero)
                    {
                        Marshal.FreeCoTaskMem(pidlMyComputer);
                    }
                }
            }

            private const int SHCNE_UPDATEDIR = 0x00001000;
            private const int SHCNF_IDLIST = 0x0000;
            private const int CSIDL_DRIVES = 0x11;

            [DllImport("shell32.dll")]
            private static extern void SHChangeNotify(int wEventId, uint uFlags, IntPtr dwItem1, IntPtr dwItem2);

            [DllImport("shell32.dll")]
            private static extern int SHGetSpecialFolderLocation(IntPtr hwndOwner, int nFolder, out IntPtr ppidl);
        }
    }
}