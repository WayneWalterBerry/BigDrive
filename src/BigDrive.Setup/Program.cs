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
            string nestedPath = $@"BigDrive.{application}";
            string logPath = $@"SYSTEM\CurrentControlSet\Services\EventLog\{nestedPath}";

            using (RegistryKey key = Registry.LocalMachine.CreateSubKey(logPath))
            {
                key.SetValue("Retention", 0); // No automatic deletion
                key.SetValue("MaxSize", 1024000); // Set max log size (1MB)
            }

            VerifyEventLogServiceIsRunning();

            if (EventLog.SourceExists(eventSource))
            {
                Console.WriteLine($"Deleting Event Source: {eventSource}...");
                EventLog.DeleteEventSource(eventSource);
            }

            Console.WriteLine($"Creating Event Source: {eventSource}...");

            EventLog.CreateEventSource(eventSource, logName: nestedPath);

            // Retry logic to ensure the source is recognized

            Stopwatch stopwatch = Stopwatch.StartNew();

            int retries = 5;
            while (retries > 0)
            {
                if (EventLog.SourceExists(eventSource))
                {
                    Console.WriteLine($"Event Source '{eventSource}' Created.");
                    break;
                }

                Console.WriteLine("Waiting for event source to be created...");

                Thread.Sleep(1000); // Wait 1 second

                retries--;
            }

            if (retries == 0)
            {
                throw new InvalidOperationException($"Event Source '{eventSource}' Failed To Be Created in {stopwatch.Elapsed.TotalSeconds} Seconds.");
            }

            VerifyLogging(eventSource);
        }

        private static void VerifyLogging(string eventSource)
        {
            if (!EventLog.SourceExists(eventSource))
            {
                throw new Exception(message: $"Event Source '{eventSource}' does not exist.");
            }

            Console.WriteLine($"Clearing all logs for event source: {eventSource}...");
            using (EventLog eventLog = new EventLog { Source = eventSource })
            {
                eventLog.Clear();
            }

            Console.WriteLine($"Writing To Event Source: {eventSource}...");

            Guid activityId = Guid.NewGuid();

            EventLogTraceListener eventLogListener = new EventLogTraceListener(source: "BigDrive.Service");
            Trace.Listeners.Add(eventLogListener);

            Trace.CorrelationManager.ActivityId = activityId;
            Trace.TraceInformation($"Activity ID: {activityId} - Test Message.");
            Trace.Flush();
            Trace.Close();

            eventLogListener.Flush();

            Console.WriteLine($"Verifying Event Source: {eventSource}...");

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

        private static void VerifyEventLogServiceIsRunning()
        {
            string serviceName = "EventLog"; // The name of the Windows Event Log service

            Console.WriteLine("VErify the Windows Event Log service is running...");

            try
            {
                using (ServiceController serviceController = new ServiceController(serviceName))
                {
                    if (serviceController.Status == ServiceControllerStatus.Running)
                    {
                        Console.WriteLine("The Windows Event Log service is running.");
                    }
                    else
                    {
                        Console.WriteLine($"The Windows Event Log service is not running. Current status: {serviceController.Status}");
                    }
                }
            }
            catch (InvalidOperationException ex)
            {
                Console.WriteLine($"Error: The service '{serviceName}' could not be found. {ex.Message}");
                throw;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An unexpected error occurred: {ex.Message}");
                throw;
            }
        }
    }
}
