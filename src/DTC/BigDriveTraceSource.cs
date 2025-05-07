// <copyright file="BigDriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

using System.Diagnostics;

namespace BigDrive.Service
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    internal class BigDriveTraceSource : TraceSource
    {
        private const string SourceName = "BigDrive.Service";

        private static readonly Lazy<BigDriveTraceSource> _instance =
            new Lazy<BigDriveTraceSource>(() => new BigDriveTraceSource());

        public static BigDriveTraceSource Instance => _instance.Value;

        static BigDriveTraceSource()
        {
            System.Diagnostics.Debugger.Launch();

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
    }
}
