// <copyright file="BigDriveTraceSource.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    internal class BigDriveTraceSource : TraceSource
    {
        private const string SourceName = "BigDrive.Provider.Sample";

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
        /// <param name="args">Arguments to Formatter</param>
        public void TraceError(string message, params object[] args)
        {
            TraceEvent(TraceEventType.Error, 0, message, args);
        }
    }
}
