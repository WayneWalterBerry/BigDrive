// <copyright file="AssemblyResolver.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service
{
    using System;
    using System.Diagnostics;
    using System.Reflection;

    /// <summary>
    /// Provides a singleton-based mechanism to resolve and load specific assemblies at runtime.
    /// Registers an event handler for the <see cref="AppDomain.AssemblyResolve"/> event to ensure
    /// that required assemblies are loaded from disk if not already present in the current AppDomain.
    /// </summary>
    internal sealed class AssemblyResolver
    {
        // Forces reference to System.Threading.Tasks.Extensions
        private static readonly Type _forceReference = typeof(System.Threading.Tasks.ValueTask);

        /// <summary>
        /// Singleton instance of the <see cref="AssemblyResolver"/> class, initialized lazily.
        /// </summary>
        private static readonly Lazy<AssemblyResolver> _instance = new Lazy<AssemblyResolver>(() => new AssemblyResolver());

        /// <summary>
        /// Gets the singleton instance of the <see cref="AssemblyResolver"/>.
        /// </summary>
        public static AssemblyResolver Instance => _instance.Value;

        /// <summary>
        /// Represents the default trace source for logging and diagnostics within the application.
        /// </summary>
        /// <remarks>This trace source is a singleton instance of <see cref="BigDriveTraceSource"/> and is
        /// used for  centralized logging and tracing. It provides a consistent mechanism for emitting diagnostic 
        /// information throughout the application.</remarks>
        private static readonly BigDriveTraceSource DefaultTraceSource = BigDriveTraceSource.Instance;

        /// <summary>
        /// Initializes a new instance of the <see cref="AssemblyResolver"/> class.
        /// Subscribes to the <see cref="AppDomain.AssemblyResolve"/> event to handle assembly resolution.
        /// </summary>
        private AssemblyResolver()
        {
            AppDomain.CurrentDomain.AssemblyResolve += ResolveAssembly;
        }

        /// <summary>
        /// Handles the <see cref="AppDomain.AssemblyResolve"/> event to resolve and load specific assemblies at runtime.
        /// Attempts to resolve the requested assembly by checking known assembly names and loading them from disk if necessary.
        /// Returns the resolved <see cref="Assembly"/> if successful; otherwise, returns null.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="args">The event arguments containing details about the assembly to resolve.</param>
        /// <returns>The resolved <see cref="Assembly"/> if found; otherwise, null.</returns>
        private static Assembly ResolveAssembly(object sender, ResolveEventArgs args)
        {
            if (TryResolveAssembly(args, "System.Text.Json", "System.Text.Json.dll", out var assembly) ||
                TryResolveAssembly(args, "System.Runtime.CompilerServices.Unsafe", "System.Runtime.CompilerServices.Unsafe.dll", out assembly) ||
                TryResolveAssembly(args, "System.Diagnostics.DiagnosticSource", "System.Diagnostics.DiagnosticSource.dll", out assembly) ||
                TryResolveAssembly(args, "System.Memory", "System.Memory.dll", out assembly) ||
                TryResolveAssembly(args, "System.Buffers", "System.Buffers.dll", out assembly) ||
                TryResolveAssembly(args, "System.Threading.Tasks.Extensions", "System.Threading.Tasks.Extensions.dll", out assembly) ||
                TryResolveAssembly(args, "System.Numerics.Vectors", "System.Numerics.Vectors.dll", out assembly))
            {
                if (assembly == null)
                {
                    DefaultTraceSource.TraceEvent(TraceEventType.Warning, 0, $"Failed to resolve assembly: {args.Name}");
                }

                return assembly;
            }

            return null;
        }

        /// <summary>
        /// Attempts to resolve and load a specific assembly by name and DLL file path during an assembly resolution event.
        /// Checks if the requested assembly matches the specified name, searches loaded assemblies, and if not found,
        /// attempts to load it from the given DLL file path. Returns true if the assembly is successfully resolved and loaded; otherwise, false.
        /// </summary>
        /// <param name="args">The event arguments containing details about the assembly resolution request.</param>
        /// <param name="assemblyName">The simple name of the assembly to resolve.</param>
        /// <param name="dllName">The file name of the DLL to load if the assembly is not already loaded.</param>
        /// <param name="assembly">When this method returns, contains the resolved <see cref="Assembly"/> if successful; otherwise, null.</param>
        /// <returns>True if the assembly was successfully resolved and loaded; otherwise, false.</returns>
        private static bool TryResolveAssembly(ResolveEventArgs args, string assemblyName, string dllName, out Assembly assembly)
        {
            assembly = null;

            if (args.Name.StartsWith(assemblyName))
            {
                foreach (var a in AppDomain.CurrentDomain.GetAssemblies())
                {
                    if (a.FullName.StartsWith(assemblyName))
                    {
                        assembly = a;
                        return true;
                    }
                }

                string executingAssemblyLocation = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), dllName);
                if (System.IO.File.Exists(executingAssemblyLocation))
                {
                    assembly = Assembly.LoadFrom(executingAssemblyLocation);
                    if (assembly != null)
                    {
                        return true;
                    }
                }
                else
                {
                    DefaultTraceSource.TraceEvent(TraceEventType.Warning, 0, $"Assembly '{assemblyName}' not found at expected location: {executingAssemblyLocation}");
                }
            }

            return false;
        }
    }
}