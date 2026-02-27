// <copyright file="Provider.IBigDriveRegistration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.VirtualDisk
{
    using System;
    using System.Runtime.InteropServices;
    using System.Security.Principal;
    using System.Threading;

    using BigDrive.ConfigProvider;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveRegistration"/> for the VirtualDisk provider.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Called automatically by regsvcs.exe during COM registration.
        /// Sets the COM+ application identity to Interactive User and registers the provider.
        /// </summary>
        /// <param name="type">The type being registered.</param>
        [ComRegisterFunction]
        public static void ComRegister(Type type)
        {
            DefaultTraceSource.TraceInformation($"ComRegister: Registering provider {type.FullName}");

            SetApplicationIdentityToInteractiveUser("BigDrive.Provider.VirtualDisk");

            Provider provider = new Provider();
            provider.Register();

            DefaultTraceSource.TraceInformation("ComRegister: Provider registration completed.");
        }

        /// <summary>
        /// Called automatically by regsvcs.exe during COM unregistration.
        /// </summary>
        /// <param name="type">The type being unregistered.</param>
        [ComUnregisterFunction]
        public static void ComUnregister(Type type)
        {
            DefaultTraceSource.TraceInformation($"ComUnregister: Unregistering provider {type.FullName}");

            Provider provider = new Provider();
            provider.Unregister();
        }

        /// <inheritdoc/>
        public void Register()
        {
            WindowsIdentity identity = WindowsIdentity.GetCurrent();

            DefaultTraceSource.TraceInformation($"Register: User={identity.Name}, ImpersonationLevel={identity.ImpersonationLevel}");

            ProviderManager.RegisterProvider(ProviderConfig, CancellationToken.None);

            DefaultTraceSource.TraceInformation("Register: VirtualDisk provider registered successfully.");
        }

        /// <inheritdoc/>
        public void Unregister()
        {
            DefaultTraceSource.TraceInformation("Unregister: VirtualDisk provider");

            // TODO: Implement full unregistration
            // ProviderManager.UnregisterProvider(ProviderConfig.Id, CancellationToken.None);
        }

        /// <summary>
        /// Sets the identity of a COM+ application to "Interactive User".
        /// This allows the provider to run as the logged-in user and access Credential Manager.
        /// </summary>
        /// <param name="applicationName">The name of the COM+ application.</param>
        private static void SetApplicationIdentityToInteractiveUser(string applicationName)
        {
            DefaultTraceSource.TraceInformation($"Setting COM+ application '{applicationName}' identity to Interactive User");

            Type comAdminType = Type.GetTypeFromProgID("COMAdmin.COMAdminCatalog");
            if (comAdminType == null)
            {
                DefaultTraceSource.TraceError("COMAdminCatalog is not available on this system.");
                return;
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
                        app.Value["Identity"] = "Interactive User";
                        app.Value["Password"] = "";
                        applications.SaveChanges();

                        DefaultTraceSource.TraceInformation($"COM+ application '{applicationName}' identity set to 'Interactive User'.");
                        return;
                    }
                }

                DefaultTraceSource.TraceInformation($"COM+ application '{applicationName}' not found.");
            }
            finally
            {
                if (comAdmin != null && Marshal.IsComObject(comAdmin))
                {
                    Marshal.ReleaseComObject(comAdmin);
                }
            }
        }
    }
}
