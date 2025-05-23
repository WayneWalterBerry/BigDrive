﻿// <copyright file="Provider.IProcessInitializer.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using BigDrive.ConfigProvider.Model;

    public partial class Provider
    {
        static ProviderConfiguration providerConfiguration = ProviderConfigurationFactory.Create();

        /// <summary>
        /// This method is called when the COM++ Service starts the provider
        /// </summary>
        /// <param name="punkProcessControl">In Microsoft Windows XP, a pointer to the IUnknown interface of the COM component starting up. In Microsoft Windows 2000, this argument is always null.
        /// </param>
        public void Startup(object punkProcessControl)
        {
        }

        /// <summary>
        /// This method is called when the COM++ Service shutdown the provider
        /// </summary>
        public void Shutdown()
        {
        }
    }
}
