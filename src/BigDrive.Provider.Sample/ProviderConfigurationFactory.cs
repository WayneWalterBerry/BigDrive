// <copyright file="ProviderConfigurationFactory.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Reflection;
    using System.Text;
    using System.Threading.Tasks;
    using BigDrive.ConfigProvider.Model;

    internal static class ProviderConfigurationFactory
    {
       public static BigDrive.ConfigProvider.Model.ProviderConfiguration Create()
        {
            Assembly assembly = Assembly.GetExecutingAssembly();
            AssemblyTitleAttribute titleAttribute = assembly.GetCustomAttribute<AssemblyTitleAttribute>();

            return new ProviderConfiguration
            {
                Id = Provider.CLSID,
                Name = titleAttribute.Title
            };
        }
    }
}
