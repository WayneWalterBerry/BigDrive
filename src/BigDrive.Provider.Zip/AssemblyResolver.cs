// <copyright file="AssemblyResolver.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Zip
{
    using System;
    using System.IO;
    using System.Reflection;

    /// <summary>
    /// Provides assembly resolution for NuGet package dependencies.
    /// This resolves version conflicts when assemblies are loaded by COM+ (regsvcs/dllhost).
    /// </summary>
    /// <remarks>
    /// The static constructor ensures the resolver is registered before any other code runs,
    /// which is critical for COM+ ServicedComponents where assembly loading happens early.
    /// </remarks>
    internal static class AssemblyResolver
    {
        /// <summary>
        /// Static constructor ensures resolver is registered at type load time,
        /// before any other code in the assembly runs.
        /// </summary>
        static AssemblyResolver()
        {
            AppDomain.CurrentDomain.AssemblyResolve += ResolveAssembly;
        }

        /// <summary>
        /// Initializes the assembly resolver. Call this method early to ensure
        /// the static constructor has run.
        /// </summary>
        public static void Initialize()
        {
            // The static constructor does the work.
            // This method exists to provide an explicit initialization point.
        }

        /// <summary>
        /// Handles assembly resolution by loading assemblies from the same directory
        /// as the executing assembly, ignoring version mismatches.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="args">The event arguments containing the assembly name.</param>
        /// <returns>The resolved assembly, or null to let CLR continue resolution.</returns>
        private static Assembly ResolveAssembly(object sender, ResolveEventArgs args)
        {
            // Parse the requested assembly name
            AssemblyName requestedName = new AssemblyName(args.Name);

            // List of assemblies we handle
            string[] managedAssemblies = new string[]
            {
                "System.Text.Json",
                "System.Runtime.CompilerServices.Unsafe",
                "System.Memory",
                "System.Buffers",
                "System.Threading.Tasks.Extensions",
                "System.Text.Encodings.Web",
                "Microsoft.Bcl.AsyncInterfaces",
                "System.Numerics.Vectors",
                "System.ValueTuple"
            };

            foreach (string assemblyName in managedAssemblies)
            {
                if (requestedName.Name.Equals(assemblyName, StringComparison.OrdinalIgnoreCase))
                {
                    return TryLoadAssembly(assemblyName);
                }
            }

            return null;
        }

        /// <summary>
        /// Attempts to load an assembly from the executing assembly's directory.
        /// </summary>
        /// <param name="assemblyName">The simple name of the assembly.</param>
        /// <returns>The loaded assembly, or null if not found.</returns>
        private static Assembly TryLoadAssembly(string assemblyName)
        {
            // First, check if it's already loaded
            foreach (Assembly loaded in AppDomain.CurrentDomain.GetAssemblies())
            {
                if (loaded.GetName().Name.Equals(assemblyName, StringComparison.OrdinalIgnoreCase))
                {
                    return loaded;
                }
            }

            // Try to load from the same directory as this assembly
            string executingPath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            string assemblyPath = Path.Combine(executingPath, assemblyName + ".dll");

            if (File.Exists(assemblyPath))
            {
                try
                {
                    return Assembly.LoadFrom(assemblyPath);
                }
                catch
                {
                    // Fall through to return null
                }
            }

            return null;
        }
    }
}
