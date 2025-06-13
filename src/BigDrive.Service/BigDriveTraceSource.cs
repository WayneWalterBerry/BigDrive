// <copyright file="BigDriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service
{
    using System;
    using System.Diagnostics;

    internal class BigDriveTraceSource : TraceSource
    {
        private const string SourceName = "BigDrive.Service";

        private static readonly Lazy<BigDriveTraceSource> _instance =
            new Lazy<BigDriveTraceSource>(() => new BigDriveTraceSource());

        public static BigDriveTraceSource Instance => _instance.Value;

        static BigDriveTraceSource()
        {
            EventLogTraceListener eventLogListener = new EventLogTraceListener(SourceName);
            Instance.Listeners.Add(eventLogListener);
            Instance.Switch = new SourceSwitch("BigDriveSwitch", "All");

            Instance.TraceInformation($"{SourceName} Trace Source Initialized");
            Instance.Flush();
        }

        private BigDriveTraceSource() 
            : base(SourceName)
        {
        }

        /// <summary>
        /// Trace a message with the specified event type and event ID.
        /// </summary>
        /// <param name="message">Message with Formatter</param>
        /// <param name="args">Arguements to Formatter</param>
        public void TraceError(string message, params object[] args)
        {
            TraceEvent(TraceEventType.Error, 0, message, args);
        }
    }
}
