// <copyright file="EventViewerManager.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Setup
{
    using Polly.Retry;
    using Polly;
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.ServiceProcess;
    using System.IO;
    using Microsoft.Win32;
    using System.Security.AccessControl;
    using System.Security.Principal;

    internal class EventViewerManager
    {
        /// <summary>
        /// Identifies the origin of the event within a log
        /// </summary>
        private readonly string eventSource;

        /// <summary>
        /// Represents the container where events are stored
        /// </summary>
        private readonly string logName;

        /// <summary>
        /// Constructor for EventViewerManager
        /// </summary>
        /// <param name="eventSource"></param>
        /// <param name="logName"></param>
        public EventViewerManager(string eventSource, string logName)
        {
            this.eventSource = eventSource;
            this.logName = logName;
        }

        private void ClearLogs()
        {
            Console.WriteLine($"Clearing all logs for event source: {eventSource}...");
            using (EventLog eventLog = new EventLog(logName: logName) { Source = eventSource })
            {
                eventLog.Clear();
            }
        }

        private void DeleteEventSource()
        {
            if (EventLog.SourceExists(this.eventSource))
            {
                Console.WriteLine($"Deleting Event Source: {eventSource}...");
                EventLog.DeleteEventSource(eventSource);
            }
        }

        public void CreateEventSource()
        {
            Console.WriteLine($"Creating Event Source: {eventSource}...");

            Stopwatch stopwatch = Stopwatch.StartNew();

            // Define a retry policy with Polly to retry once
            RetryPolicy retryPolicy = Policy
                .Handle<Exception>() // Handle any exception
                .WaitAndRetry(
                    retryCount: 1, // Retrys once
                    sleepDurationProvider: retryAttempt => TimeSpan.FromSeconds(10), // Wait 1 second between retries
                    onRetry: (exception, timeSpan, retryCount, context) =>
                    {
                        Console.WriteLine($"Retry {retryCount} for event source '{eventSource}' after {stopwatch.Elapsed.TotalSeconds} seconds. Exception: {exception.Message}");
                    });

            // Execute the retry policy
            retryPolicy.Execute(() =>
            {
                try
                {
                    if (!IsEventLogService())
                    {
                        StartEventLogService();
                    }


                    if (!EventLog.SourceExists(this.eventSource))
                    {
                        try
                        {
                            // Create the event source
                            EventLog.CreateEventSource(source: eventSource, logName: logName);
                        }
                        catch (ArgumentException exception) when (exception.Message.Contains("already exists"))
                        {
                            // What this means is that the source already exists in another
                            // location then under the log name.
                            Console.WriteLine(exception.Message);
                        }
                    }

                    WaitForEventSource();
                }
                catch (InvalidOperationException)
                {
                    DeleteCorruptEventViewerLogs();
                    throw;
                }

                VerifyLogging();
            });
        }

        /// <summary>
        /// Waits for the event source to be created and available in the Event Viewer.
        /// </summary>
        /// <param name="eventSource">Event Source Name</param>
        /// <exception cref="InvalidOperationException"></exception>
        private void WaitForEventSource()
        {
            Stopwatch stopwatch = Stopwatch.StartNew();

            // Define a retry policy with Polly
            RetryPolicy retryPolicy = Policy
                .Handle<Exception>() // Handle any exception
                .WaitAndRetry(
                    retryCount: 5, // Retry up to 5 times
                    sleepDurationProvider: retryAttempt => TimeSpan.FromSeconds(10), // Wait 1 second between retries
                    onRetry: (exception, timeSpan, retryCount, context) =>
                    {
                        Console.WriteLine($"Retry {retryCount} for event source '{eventSource}' after {stopwatch.Elapsed.TotalSeconds} seconds. Exception: {exception.Message}");
                    });
            // Execute the retry policy
            retryPolicy.Execute(() =>
            {
                if (!EventLog.SourceExists(this.eventSource))
                {
                    throw new InvalidOperationException($"Event Source '{eventSource}' doesn't exist.");
                }

                Console.WriteLine($"Event Source '{eventSource}' Exists.");
            });
        }

        private void VerifyLogging()
        {
            if (!EventLog.SourceExists(this.eventSource))
            {
                throw new Exception(message: $"Event Source '{eventSource}' does not exist.");
            }

            ClearLogs();

            Console.WriteLine($"Writing To Event Source: {eventSource}...");

            Guid activityId = Guid.NewGuid();

            using (EventLogTraceListener eventLogListener = new EventLogTraceListener(source: this.eventSource))
            {
                Trace.Listeners.Add(eventLogListener);

                Trace.CorrelationManager.ActivityId = activityId;

                // Ensure that the trace is flushed after each write
                Trace.AutoFlush = true; 
                Trace.TraceInformation($"Activity ID: {activityId} - Test Message.");
                Trace.Flush();
                Trace.Close();

                eventLogListener.Flush();
            }

            Console.WriteLine($"Verifying Event Source: {eventSource}...");

            using (EventLog eventLog = new EventLog(logName: logName) { Source = eventSource })
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

        /// <summary>
        /// Checks if the Windows Event Log service is running.
        /// </summary>
        /// <param name="serviceControllerStatus">Service Controller Status</param>
        /// <returns>True if running otherwise false.</returns>
        private bool IsEventLogService(
            ServiceControllerStatus serviceControllerStatus = ServiceControllerStatus.Running)
        {
            string serviceName = "EventLog"; // The name of the Windows Event Log service

            Console.WriteLine("Verify the Windows Event Log service is running...");

            try
            {
                using (ServiceController serviceController = new ServiceController(serviceName))
                {
                    if (serviceController.Status == serviceControllerStatus)
                    {
                        Console.WriteLine("The Windows Event Log service is running.");
                        return true;
                    }
                    else
                    {
                        Console.WriteLine($"The Windows Event Log service is not running. Current status: {serviceController.Status}");
                        return false;
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

        /// <summary>
        /// Delete Corrupted Log Files
        /// </summary>
        /// <param name="eventSource">Event Source</param>
        private void DeleteCorruptEventViewerLogs()
        {
            StopEventLogService();
            DeleteEventLogFile();
            UnRegisterEventLog();
            StartEventLogService();
        }

        /// <summary>
        /// Stops the Windows Event Log service if it is running.
        /// </summary>
        private void StopEventLogService()
        {
            string serviceName = "EventLog"; // The name of the Windows Event Log service

            Console.WriteLine("Attempting to stop the Windows Event Log service...");

            try
            {
                using (ServiceController serviceController = new ServiceController(serviceName))
                {
                    if (serviceController.Status == ServiceControllerStatus.Running)
                    {
                        Console.WriteLine("The Windows Event Log service is currently running. Stopping the service...");

                        serviceController.Stop();
                        serviceController.WaitForStatus(ServiceControllerStatus.Stopped, TimeSpan.FromSeconds(30));

                        Console.WriteLine("The Windows Event Log service has been stopped successfully.");
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
            catch (System.ServiceProcess.TimeoutException ex)
            {
                Console.WriteLine($"Error: The service '{serviceName}' did not stop within the expected time. {ex.Message}");
                throw;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An unexpected error occurred while stopping the service: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Starts the Windows Event Log service if it is not already running.
        /// </summary>
        private void StartEventLogService()
        {
            string serviceName = "EventLog"; // The name of the Windows Event Log service

            Console.WriteLine("Attempting to start the Windows Event Log service...");

            try
            {
                using (ServiceController serviceController = new ServiceController(serviceName))
                {
                    if (serviceController.Status == ServiceControllerStatus.Stopped)
                    {
                        Console.WriteLine("The Windows Event Log service is currently stopped. Starting the service...");

                        serviceController.Start();
                        serviceController.WaitForStatus(ServiceControllerStatus.Running, TimeSpan.FromSeconds(30));

                        Console.WriteLine("The Windows Event Log service has been started successfully.");
                    }
                    else
                    {
                        Console.WriteLine($"The Windows Event Log service is already running. Current status: {serviceController.Status}");
                    }
                }
            }
            catch (InvalidOperationException ex)
            {
                Console.WriteLine($"Error: The service '{serviceName}' could not be found. {ex.Message}");
                throw;
            }
            catch (System.ServiceProcess.TimeoutException ex)
            {
                Console.WriteLine($"Error: The service '{serviceName}' did not start within the expected time. {ex.Message}");
                throw;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An unexpected error occurred while starting the service: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Deletes the .evtx log file for the specified event source from the Windows Event Log directory.
        /// </summary>
        /// <param name="eventSource">The name of the event source whose log file should be deleted.</param>
        private void DeleteEventLogFile()
        {
            try
            {
                // Get the Windows installation directory using the %SystemRoot% environment variable
                string logFilePath = Environment.ExpandEnvironmentVariables($@"%windir%\System32\winevt\Logs\{eventSource}.evtx");

                Console.WriteLine($"Attempting to delete the log file: {logFilePath}...");

                // Check if the file exists
                if (File.Exists(logFilePath))
                {
                    // Delete the file
                    File.Delete(logFilePath);
                    Console.WriteLine($"Log file '{logFilePath}' deleted successfully.");
                }
                else
                {
                    Console.WriteLine($"Log file '{logFilePath}' does not exist.");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An error occurred while deleting the log file: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Register the Event Log in the Windows Registry.
        /// </summary>
        /// <param name="logName">The name of the event source to register.</param>
        [Obsolete("This method is obsolete and should not be used. Event logs are automatically registered when created using EventLog.CreateEventSource().", true)]
        private void RegisterEventLog()
        {
            string logPath = $@"SYSTEM\CurrentControlSet\Services\EventLog\{logName}";

            using (RegistryKey key = Registry.LocalMachine.CreateSubKey(logPath, writable: true))
            {
                key.SetValue("Retention", 0); // No automatic deletion
                key.SetValue("MaxSize", 1024000); // Set max log size (1MB)
            }
        }

        /// <summary>
        /// Unregisters the specified event source by deleting its window registry key.
        /// </summary>
        /// <param name="eventSource">The name of the event source to unregister.</param>
        private void UnRegisterEventLog()
        {
            try
            {
                Console.WriteLine($"Attempting to unregister event log: {this.logName}...");

                // Open the registry key for deletion
                using (RegistryKey key = Registry.LocalMachine.OpenSubKey(@"SYSTEM\CurrentControlSet\Services\EventLog", writable: true))
                {
                    if (key != null && key.GetSubKeyNames().Contains(this.logName))
                    {
                        key.DeleteSubKeyTree(this.logName);
                        Console.WriteLine($"Event log '{this.logName}' unregistered successfully.");
                    }
                    else
                    {
                        Console.WriteLine($"Event log '{this.logName}' does not exist in the registry.");
                    }
                }
            }
            catch (UnauthorizedAccessException ex)
            {
                Console.WriteLine($"Error: Insufficient permissions to delete the registry key for event log '{this.logName}'. {ex.Message}");
                throw;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An unexpected error occurred while unregistering the event log '{this.logName}': {ex.Message}");
                throw;
            }
        }

        private void UnRegisterEventSource()
        {
            try
            {
                Console.WriteLine($"Attempting to unregister event source: {this.eventSource}...");

                // Open the registry key for deletion
                using (RegistryKey key = Registry.LocalMachine.OpenSubKey(
                    $@"SYSTEM\CurrentControlSet\Services\EventLog\{this.logName}", writable: true))
                {
                    if (key != null && key.GetSubKeyNames().Contains(this.eventSource))
                    {
                        key.DeleteSubKeyTree(this.logName);
                        Console.WriteLine($"Event source '{this.eventSource}' unregistered successfully.");
                    }
                    else
                    {
                        Console.WriteLine($"Event source '{this.eventSource}' does not exist in the registry.");
                    }
                }
            }
            catch (UnauthorizedAccessException ex)
            {
                Console.WriteLine($"Error: Insufficient permissions to delete the registry key for event source '{this.eventSource}'. {ex.Message}");
                throw;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An unexpected error occurred while unregistering the event source '{this.eventSource}': {ex.Message}");
                throw;
            }
        }

        private bool SourceExists()
        {
            string registryPath = @"SYSTEM\CurrentControlSet\Services\EventLog";

            try
            {
                using (RegistryKey eventLogKey = Registry.LocalMachine.OpenSubKey(registryPath, false))
                {
                    if (eventLogKey == null)
                    {
                        Console.WriteLine($"Registry path '{registryPath}' not found.");
                        return false;
                    }

                    foreach (string subKeyName in eventLogKey.GetSubKeyNames())
                    {
                        using (RegistryKey subKey = eventLogKey.OpenSubKey(subKeyName, false))
                        {
                            if (subKey != null && subKey.GetSubKeyNames().Contains(this.eventSource))
                            {
                                Console.WriteLine($"Event source '{this.eventSource}' found under '{subKeyName}'.");
                                return true;
                            }
                        }
                    }
                }
            }
            catch (UnauthorizedAccessException ex)
            {
                Console.WriteLine($"Unauthorized access to registry path '{registryPath}'. {ex.Message}");
                throw;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An error occurred while searching for the event source: {ex.Message}");
                throw;
            }

            Console.WriteLine($"Event source '{this.eventSource}' not found.");
            return false;
        }
    }
}
