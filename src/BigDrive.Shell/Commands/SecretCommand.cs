// <copyright file="SecretCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Threading;

    using BigDrive.ConfigProvider;

    /// <summary>
    /// Manages secrets (API keys, tokens) for BigDrive drives using Windows Credential Manager.
    /// </summary>
    public class SecretCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "secret"; }
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
            get { return "Manages secrets for the current drive"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "secret <set|exists|del|list> [key] [value]"; }
        }

        /// <summary>
        /// Executes the secret command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            // Must be on a BigDrive
            Guid? driveGuid = context.CurrentDriveGuid;

            if (driveGuid == null || driveGuid == Guid.Empty)
            {
                Console.WriteLine("Error: You must switch to a BigDrive first (e.g., 'cd Z:')");
                Console.WriteLine("Use 'drives' to see available drives.");
                return;
            }

            if (args.Length == 0)
            {
                ShowUsage();
                return;
            }

            string subCommand = args[0].ToLowerInvariant();

            switch (subCommand)
            {
                case "set":
                    ExecuteSet(driveGuid.Value, args);
                    break;

                case "exists":
                    ExecuteExists(driveGuid.Value, args);
                    break;

                case "del":
                case "delete":
                    ExecuteDelete(driveGuid.Value, args);
                    break;

                case "list":
                    ExecuteList(driveGuid.Value);
                    break;

                default:
                    Console.WriteLine("Unknown subcommand: {0}", subCommand);
                    ShowUsage();
                    break;
            }
        }

        /// <summary>
        /// Executes the 'set' subcommand to store a secret.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="args">The command arguments.</param>
        private void ExecuteSet(Guid driveGuid, string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("Usage: secret set <key> [value]");
                Console.WriteLine("If value is omitted, you will be prompted (masked input).");
                return;
            }

            string key = args[1];
            string value;

            if (args.Length >= 3)
            {
                // Value provided on command line
                value = args[2];
            }
            else
            {
                // Prompt for value with masked input
                Console.Write("Enter value for '{0}': ", key);
                value = ReadMaskedInput();
                Console.WriteLine();
            }

            if (string.IsNullOrEmpty(value))
            {
                Console.WriteLine("Error: Value cannot be empty.");
                return;
            }

            try
            {
                DriveManager.WriteSecretProperty(driveGuid, key, value, CancellationToken.None);
                Console.WriteLine("Secret '{0}' saved.", key);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error saving secret: {0}", ex.Message);
            }
        }

        /// <summary>
        /// Executes the 'exists' subcommand to check if a secret exists.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="args">The command arguments.</param>
        private void ExecuteExists(Guid driveGuid, string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("Usage: secret exists <key>");
                return;
            }

            string key = args[1];

            try
            {
                string value = DriveManager.ReadSecretProperty(driveGuid, key, CancellationToken.None);
                if (value != null)
                {
                    Console.WriteLine("    {0}: exists", key);
                }
                else
                {
                    Console.WriteLine("    {0}: not found", key);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error checking secret: {0}", ex.Message);
            }
        }

        /// <summary>
        /// Executes the 'del' subcommand to delete a secret.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="args">The command arguments.</param>
        private void ExecuteDelete(Guid driveGuid, string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("Usage: secret del <key>");
                return;
            }

            string key = args[1];

            try
            {
                bool deleted = DriveManager.DeleteSecretProperty(driveGuid, key, CancellationToken.None);
                if (deleted)
                {
                    Console.WriteLine("Deleted: {0}", key);
                }
                else
                {
                    Console.WriteLine("Secret '{0}' not found.", key);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error deleting secret: {0}", ex.Message);
            }
        }

        /// <summary>
        /// Executes the 'list' subcommand to list known secret keys for Flickr provider.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        private void ExecuteList(Guid driveGuid)
        {
            try
            {
                List<string> secretNames = DriveManager.GetSecretNames(driveGuid, CancellationToken.None);

                Console.WriteLine();
                Console.WriteLine(" Secrets for current drive:");
                Console.WriteLine();

                if (secretNames.Count == 0)
                {
                    Console.WriteLine("    No secrets configured.");
                    Console.WriteLine();
                    Console.WriteLine("    Use 'secret set <key>' to add a secret.");
                    return;
                }

                foreach (string key in secretNames)
                {
                    Console.WriteLine("    {0}", key);
                }

                Console.WriteLine();
                Console.WriteLine("    {0} secret(s) configured.", secretNames.Count);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error listing secrets: {0}", ex.Message);
            }
        }

        /// <summary>
        /// Shows usage information for the secret command.
        /// </summary>
        private void ShowUsage()
        {
            Console.WriteLine("Usage: secret <subcommand> [arguments]");
            Console.WriteLine();
            Console.WriteLine("Subcommands:");
            Console.WriteLine("    set <key> [value]   Set a secret (prompts if value omitted)");
            Console.WriteLine("    exists <key>        Check if a secret exists");
            Console.WriteLine("    del <key>           Delete a secret");
            Console.WriteLine("    list                List all secrets for this drive");
            Console.WriteLine();
            Console.WriteLine("Examples:");
            Console.WriteLine("    secret set FlickrApiKey");
            Console.WriteLine("    secret set FlickrApiKey YOUR_API_KEY");
            Console.WriteLine("    secret exists FlickrApiKey");
            Console.WriteLine("    secret del FlickrApiKey");
            Console.WriteLine("    secret list");
        }

        /// <summary>
        /// Reads input from the console with masked characters (asterisks).
        /// </summary>
        /// <returns>The entered string.</returns>
        private string ReadMaskedInput()
        {
            string input = string.Empty;

            while (true)
            {
                ConsoleKeyInfo keyInfo = Console.ReadKey(intercept: true);

                if (keyInfo.Key == ConsoleKey.Enter)
                {
                    break;
                }
                else if (keyInfo.Key == ConsoleKey.Backspace)
                {
                    if (input.Length > 0)
                    {
                        input = input.Substring(0, input.Length - 1);
                        Console.Write("\b \b");
                    }
                }
                else if (!char.IsControl(keyInfo.KeyChar))
                {
                    input += keyInfo.KeyChar;
                    Console.Write("*");
                }
            }

            return input;
        }
    }
}
