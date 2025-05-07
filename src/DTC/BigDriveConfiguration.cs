// <copyright file="BigDriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.ComObjects
{
    using System;
    using System.Diagnostics;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;
    using System.Text.Json;
    using System.Text.Json.Serialization;
    using System.Threading;
    using BigDrive.Interfaces;
    using ConfigProvider;
    using Microsoft.Win32;

    [Guid("E6F5A1B2-4C6E-4F8A-9D3E-1A2B3C4D5E7F")] // Unique GUID for the COM class
    [ClassInterface(ClassInterfaceType.None)] // No automatic interface generation
    [ComVisible(true)] // Make the class visible to COM
    public class BigDriveConfiguration : ServicedComponent, IBigDriveConfiguration
    {
        private static readonly TraceSource DefaultTraceSource = new TraceSource("BigDrive");

        /// <inheritdoc/>
        public string GetConfiguration(Guid guid)
        {
            System.Diagnostics.Debugger.Launch();

            DefaultTraceSource.TraceInformation("GetConfiguration called with GUID: {0}", guid);

            using (CancellationTokenSource cancellationTokenSource = new CancellationTokenSource())
            {
                var driveConfiguration = ConfigurationProvider.ReadConfiguration(guid, cancellationTokenSource.Token);

                // Serialize the configuration to JSON
                var options = new JsonSerializerOptions
                {
                    WriteIndented = true,
                    Converters =
                    {
                        new JsonStringEnumConverter(JsonNamingPolicy.CamelCase)
                    }
                };

                string json = JsonSerializer.Serialize(driveConfiguration, options);
                return json;
            }
        }
    }
}
