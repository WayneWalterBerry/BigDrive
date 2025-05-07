// <copyright file="BigDriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service.ComObjects
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

    [Guid("E6F5A1B2-4C6E-4F8A-9D3E-1A2B3C4D5E7F")] // Unique GUID for the COM class
    [ClassInterface(ClassInterfaceType.None)] // No automatic interface generation
    [ComVisible(true)] // Make the class visible to COM
    public class BigDriveConfiguration : ServicedComponent, IBigDriveConfiguration
    {
        private static readonly AssemblyResolver asssemblyResolver;
        private static readonly TraceSource DefaultTraceSource = BigDriveTraceSource.Instance;

        static BigDriveConfiguration()
        {
            asssemblyResolver = AssemblyResolver.Instance;
        }

        /// <inheritdoc/>
        public string GetConfiguration(Guid guid)
        {
            DefaultTraceSource.TraceInformation("BigDriveConfiguration::GetConfiguration() called for drive: {0}", guid);

            using (CancellationTokenSource cancellationTokenSource = new CancellationTokenSource())
            {
                var driveConfiguration = ConfigurationProvider.ReadConfiguration(guid, cancellationTokenSource.Token);

                // Serialize the configuration to JSON
                var options = new JsonSerializerOptions
                {
                    WriteIndented = false,
                    Converters =
                    {
                        new JsonStringEnumConverter(JsonNamingPolicy.CamelCase)
                    }
                };

                string json = JsonSerializer.Serialize(driveConfiguration, options);
                json = json.Replace("\r", "").Replace("\n", "");

                DefaultTraceSource.TraceInformation("BigDriveConfiguration::GetConfiguration() returned: {0}", json);

                return json;
            }
        }
    }
}
