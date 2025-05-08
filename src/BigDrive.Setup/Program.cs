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
            string nestedPath = $@"BigDrive.{application}";
            string logPath = $@"SYSTEM\CurrentControlSet\Services\EventLog\{nestedPath}";

            using (RegistryKey key = Registry.LocalMachine.CreateSubKey(logPath))
            {
                key.SetValue("Retention", 0); // No automatic deletion
                key.SetValue("MaxSize", 1024000); // Set max log size (1MB)
            }

            if (EventLog.SourceExists(eventSource))
            {
                EventLog.DeleteEventSource(eventSource);
            }

            EventLog.CreateEventSource(eventSource, logName: nestedPath);

            VerifyLogging(eventSource);
        }

        private static void VerifyLogging(string eventSource)
        {
            Console.WriteLine($"Verifying Event Source: {eventSource}...");

            Guid activityId = Guid.NewGuid();

            EventLogTraceListener eventLogListener = new EventLogTraceListener(source: "BigDrive.Service");
            Trace.Listeners.Add(eventLogListener);

            Trace.CorrelationManager.ActivityId = activityId;
            Trace.TraceInformation($"Activity ID: {activityId} - Test Message.");
            Trace.Flush();
            Trace.Close();

            if (!EventLog.SourceExists(eventSource))
            {
                throw new Exception(message: $"Event Source '{eventSource}' does not exist.");
            }

            using (EventLog eventLog = new EventLog { Source = eventSource })
            {
                foreach (EventLogEntry entry in eventLog.Entries)
                {
                    if (entry.Message.Contains(activityId.ToString()))
                    {
                        Console.WriteLine("Log entry successfully written to the custom Event Log.");
                        return;
                    }
                }
            }

            throw new Exception(message: $"Log entry not found in the custom event source {eventSource}.");
        }
    }
}
