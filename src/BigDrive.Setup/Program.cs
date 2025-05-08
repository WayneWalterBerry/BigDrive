// <copyright file="Program.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Setup
{
    using System;
    using System.Diagnostics;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Security.Principal;
    using System.ServiceProcess;
    using System.Threading;
    using Microsoft.Win32;

    internal class Program
    {
        static void Main(string[] args)
        {
            System.Diagnostics.Debugger.Launch();

            if (!IsRunningElevated())
            {
                Console.WriteLine("Administrative Rights Required.");
                return;
            }

            Console.WriteLine("Bootstrapping BigDrive Event Logs...");
            BoostrapBigDriveEventLogs();
        }

        private static bool IsRunningElevated()
        {
            using (WindowsIdentity identity = WindowsIdentity.GetCurrent())
            {
                WindowsPrincipal principal = new WindowsPrincipal(identity);
                return principal.IsInRole(WindowsBuiltInRole.Administrator);
            }
        }

        private static void BoostrapBigDriveEventLogs()
        {
            BoostrapBigDriveEventLog("Service");
            BoostrapBigDriveEventLog("ShellFolder");
        }

        private static void BoostrapBigDriveEventLog(string application)
        {
            Console.WriteLine($"Creating Custom Event Source For BigDrive {application}...");

            string eventSource = $"BigDrive.{application}";
            string logName = $@"BigDrive";

            EventViewerManager eventViewerManager = new EventViewerManager(eventSource, logName);
            eventViewerManager.CreateEventSource();
        }
    }
}
