// <copyright file="Provider.IProcessInitializer.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using BigDrive.ConfigProvider.Model;
    using BigDrive.ConfigProvider;

    public partial class Provider
    {
        static ProviderConfiguration providerConfiguration = ProviderConfigurationFactory.Create();

        public void Startup(object punkProcessControl)
        {
            System.Diagnostics.Debugger.Launch();
            ProviderManager.RegisterProvider(providerConfiguration, CancellationToken.None);
        }

        public void Shutdown()
        {
            ProviderManager.UnRegisterProvider(providerConfiguration.Id, CancellationToken.None);
        }
    }
}
