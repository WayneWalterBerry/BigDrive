// <copyright file="Provider.IBigDriveFileInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using System.Collections.Generic;

    public partial class Provider
    {
        Dictionary<string, DateTime> lastModifiedData = new Dictionary<string, DateTime>();

        public DateTime LastModifiedTime(Guid driveGuid, string path)
        {
            // Cache the Dates For the Example, So that They Don't Appear To 
            // "Flutter" around, this isn't nessecary when you have a persisted 
            // data source.
            if (!lastModifiedData.ContainsKey(path))
            {
                // Get current time and the time 2 years ago
                DateTime now = DateTime.Now;
                DateTime twoYearsAgo = now.AddYears(-2);

                // Create a random number generator
                Random random = new Random();

                // Generate weighted random value (square the random value to bias toward 0)
                // This gives more weight to recent dates
                double randomValue = Math.Pow(random.NextDouble(), 2);

                // Calculate timespan between now and 2 years ago
                TimeSpan timeSpan = now - twoYearsAgo;

                // Calculate weighted date (closer to now)
                DateTime resultDate = now.AddDays(-randomValue * timeSpan.TotalDays);

                lastModifiedData.Add(path, resultDate);
               
            }

            return lastModifiedData[path];
        }
    }
}
