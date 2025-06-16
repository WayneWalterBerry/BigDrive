// <copyright file="ProviderConfigurationFactory.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System.Reflection;
    using BigDrive.ConfigProvider.Model;

    internal static class ProviderConfigurationFactory
    {
       public static ProviderConfiguration Create()
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
