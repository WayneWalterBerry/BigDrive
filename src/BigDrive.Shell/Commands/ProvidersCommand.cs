// <copyright file="ProvidersCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Lists all registered BigDrive providers.
    /// </summary>
    public class ProvidersCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "providers"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[0]; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Lists all registered BigDrive providers"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "providers"; }
        }

        /// <summary>
        /// Executes the providers command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            List<ProviderConfiguration> providers = ProviderManager.ReadProviders(CancellationToken.None).ToList();

            if (providers.Count == 0)
            {
                Console.WriteLine("No providers registered.");
                Console.WriteLine();
                Console.WriteLine("Providers are registered in HKLM:\\SOFTWARE\\BigDrive\\Providers");
                return;
            }

            Console.WriteLine();
            Console.WriteLine(" Registered Providers:");
            Console.WriteLine();

            foreach (ProviderConfiguration provider in providers)
            {
                Console.WriteLine("    {0}", provider.Name);
                Console.WriteLine("    CLSID: {0:B}", provider.Id);
                Console.WriteLine();
            }

            Console.WriteLine("    {0} Provider(s)", providers.Count);
        }
    }
}
