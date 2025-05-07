// <copyright file="AssemblyResolver.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.ComObjects
{
    using System;
    using System.Reflection;

    internal sealed class AssemblyResolver
    {
        private static readonly Lazy<AssemblyResolver> _instance = new Lazy<AssemblyResolver>(() => new AssemblyResolver());

        public static AssemblyResolver Instance => _instance.Value;

        private AssemblyResolver()
        {
            AppDomain.CurrentDomain.AssemblyResolve += ResolveAssembly;
        }

        private static Assembly ResolveAssembly(object sender, ResolveEventArgs args)
        {
            if (args.Name.StartsWith("System.Runtime.CompilerServices.Unsafe"))
            {
                foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
                {
                    if (assembly.FullName.StartsWith("System.Runtime.CompilerServices.Unsafe"))
                    {
                        return assembly;
                    }
                }

                string executingAssemblyLocation = System.IO.Path.Combine(
                    System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location),
                    "System.Runtime.CompilerServices.Unsafe.dll");

                if (System.IO.File.Exists(executingAssemblyLocation))
                {
                    return Assembly.LoadFrom(executingAssemblyLocation);
                }
            }

            return null;
        }
    }
}
