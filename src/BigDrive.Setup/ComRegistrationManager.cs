// <copyright file="ComRegistrationManager.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Setup
{
    using System;
    using System.Diagnostics;
    using System.Runtime.InteropServices;
    using System.Security.Principal;

    public static class ComRegistrationManager
    {
        public const string ComPlusServiceName = "BigDrive.Service";

        /// <summary>
        /// Registers a COM+ assembly using regsvcs.exe as the current process user.
        /// </summary>
        /// <param name="assemblyPath">The path to the assembly to register.</param>
        public static void RegisterComAssemblyAsUser(string assemblyPath)
        {
            string regsvcsPath = Environment.Is64BitProcess
                ? @"C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe"
                : @"C:\Windows\Microsoft.NET\Framework\v4.0.30319\regsvcs.exe";

            if (!System.IO.File.Exists(regsvcsPath))
            {
                throw new InvalidOperationException($"regsvcs.exe not found at {regsvcsPath}");
            }

            var psi = new ProcessStartInfo
            {
                FileName = regsvcsPath,
                Arguments = $"\"{assemblyPath}\"",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true
            };

            using (var process = Process.Start(psi))
            {
                process.WaitForExit();
                string output = process.StandardOutput.ReadToEnd();
                string error = process.StandardError.ReadToEnd();
                ConsoleExtensions.WriteIndented(output);

                if (!string.IsNullOrEmpty(error))
                {
                    ConsoleExtensions.WriteIndented(error);
                }
            }
        }

        /// <summary>
        /// Deletes a COM+ application (service) by its name.
        /// </summary>
        /// <param name="applicationName">The name of the COM+ application to delete.</param>
        public static void DeleteComPlusApplication(string applicationName = ComPlusServiceName)
        {
            ConsoleExtensions.WriteIndented($"Deleting COM+ application: {applicationName}");

            Type comAdminType = Type.GetTypeFromProgID("COMAdmin.COMAdminCatalog");
            if (comAdminType == null)
            {
                throw new InvalidOperationException("COMAdminCatalog is not available on this system.");
            }

            dynamic comAdmin = Activator.CreateInstance(comAdminType);
            try
            {
                dynamic applications = comAdmin.GetCollection("Applications");
                applications.Populate();

                int foundIndex = -1;
                int currentIndex = 0;
                foreach (dynamic app in applications)
                {
                    if (string.Equals((string)app.Name, applicationName, StringComparison.OrdinalIgnoreCase))
                    {
                        foundIndex = currentIndex;
                        break;
                    }
                    currentIndex++;
                }

                if (foundIndex != -1)
                {
                    applications.Remove(foundIndex);
                    applications.SaveChanges();
                    ConsoleExtensions.WriteIndented($"COM+ application '{applicationName}' deleted successfully.");
                    return;
                }

                ConsoleExtensions.WriteIndented($"COM+ application '{applicationName}' not found.");
            }
            finally
            {
                if (comAdmin != null && Marshal.IsComObject(comAdmin))
                {
                    Marshal.ReleaseComObject(comAdmin);
                }
            }
        }

        /// <summary>
        /// Sets the identity of a COM+ application to "This User" (service account) programmatically.
        /// </summary>
        /// <param name="applicationName">The name of the COM+ application.</param>
        /// <param name="username">The service account username (DOMAIN\username or .\username).</param>
        /// <param name="password">The service account password.</param>
        public static void SetApplicationIdentityToThisUser(string applicationName, string username, string password)
        {
            Type comAdminType = Type.GetTypeFromProgID("COMAdmin.COMAdminCatalog");
            if (comAdminType == null)
            {
                throw new InvalidOperationException("COMAdminCatalog is not available on this system.");
            }

            dynamic comAdmin = Activator.CreateInstance(comAdminType);
            try
            {
                dynamic applications = comAdmin.GetCollection("Applications");
                applications.Populate();

                foreach (dynamic app in applications)
                {
                    if (string.Equals((string)app.Name, applicationName, StringComparison.OrdinalIgnoreCase))
                    {
                        app.Value["Identity"] = username;
                        app.Value["Password"] = password;
                        applications.SaveChanges();
                        ConsoleExtensions.WriteIndented($"COM+ application '{applicationName}' identity set to 'This User' ({username}).");
                        return;
                    }
                }

                throw new InvalidOperationException($"COM+ application '{applicationName}' not found.");
            }
            finally
            {
                if (comAdmin != null && Marshal.IsComObject(comAdmin))
                {
                    Marshal.ReleaseComObject(comAdmin);
                }
            }
        }

        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern bool LogonUser(
            string lpszUsername,
            string lpszDomain,
            string lpszPassword,
            int dwLogonType,
            int dwLogonProvider,
            out IntPtr phToken);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(IntPtr hObject);
    }
}