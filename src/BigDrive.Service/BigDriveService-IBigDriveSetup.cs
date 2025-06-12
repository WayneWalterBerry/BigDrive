// <copyright file="BigDriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service
{
    using System;
    using System.Linq;
    using System.Security.Principal;

    public partial class BigDriveService
    {
        /// <inheritdoc/>
        public void Validate(Guid activityId)
        {
            // Big Drive Setup, Verifies That This Log Line Is Written
            DefaultTraceSource.TraceInformation($"IBigDriveSetup::Validate() Valiation Pass: {activityId}...");

            // Check if the current user is BigDriveInstaller
            string currentUser = WindowsIdentity.GetCurrent().Name;
            string login = currentUser.Split('\\').LastOrDefault() ?? string.Empty;

            if (login.Equals("BigDriveInstaller", StringComparison.OrdinalIgnoreCase))
            {
                DefaultTraceSource.TraceInformation("User {0} is authorized.", currentUser);
            }
            else
            {
                DefaultTraceSource.TraceError("User {0} is not authorized.", currentUser);
                throw new UnauthorizedAccessException("Current user is not authorized to perform this operation.");
            }
        }
    }
}
