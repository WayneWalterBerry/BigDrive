// <copyright file="BigDriveTraceSource.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Archive
{
    using System;
    using System.Diagnostics;

    /// <summary>
    /// Provides tracing capabilities for the Archive provider.
    /// Logs to the Windows Event Log.
    /// </summary>
    internal class BigDriveTraceSource : TraceSource
    {
        /// <summary>
        /// The name of the trace source.
        /// </summary>
        private const string SourceName = "BigDrive.Provider.Archive";

        /// <summary>
        /// Lazy singleton instance.
        /// </summary>
        private static readonly Lazy<BigDriveTraceSource> _instance =
            new Lazy<BigDriveTraceSource>(() => new BigDriveTraceSource());

        /// <summary>
        /// Gets the singleton instance of the trace source.
        /// </summary>
        public static BigDriveTraceSource Instance => _instance.Value;

        /// <summary>
        /// The name of the event log to write to.
        /// </summary>
        private const string LogName = "BigDrive";

        /// <summary>
        /// Static constructor to initialize the trace source.
        /// </summary>
        static BigDriveTraceSource()
        {
            EventLog eventLog = new EventLog(LogName, ".", SourceName);
            EventLogTraceListener eventLogListener = new EventLogTraceListener(eventLog);
            Instance.Listeners.Add(eventLogListener);
            Instance.Switch = new SourceSwitch("BigDriveArchiveSwitch", "All");

            Instance.TraceInformation($"{SourceName} Trace Source Initialized");
            Instance.Flush();
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BigDriveTraceSource"/> class.
        /// </summary>
        private BigDriveTraceSource()
            : base(SourceName)
        {
        }

        /// <summary>
        /// Traces an error message.
        /// </summary>
        /// <param name="message">The message format string.</param>
        /// <param name="args">The format arguments.</param>
        public void TraceError(string message, params object[] args)
        {
            TraceEvent(TraceEventType.Error, 0, message, args);
        }
    }
}
