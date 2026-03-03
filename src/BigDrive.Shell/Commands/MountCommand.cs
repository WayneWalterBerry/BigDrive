// <copyright file="MountCommand.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.Commands
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text.Json;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;
    using BigDrive.Interfaces;

    /// <summary>
    /// Mounts a new BigDrive by creating a drive configuration.
    /// Similar to 'net use' for network drives.
    /// </summary>
    public class MountCommand : ICommand
    {
        /// <summary>
        /// Gets the primary name of the command.
        /// </summary>
        public string Name
        {
            get { return "mount"; }
        }

        /// <summary>
        /// Gets the command aliases.
        /// </summary>
        public string[] Aliases
        {
            get { return new string[] { "register", "add" }; }
        }

        /// <summary>
        /// Gets the command description.
        /// </summary>
        public string Description
        {
            get { return "Mounts a new BigDrive"; }
        }

        /// <summary>
        /// Gets the usage syntax.
        /// </summary>
        public string Usage
        {
            get { return "mount [<provider-number> <drive-name>]"; }
        }

        /// <summary>
        /// Executes the mount command.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="args">The command arguments.</param>
        public void Execute(ShellContext context, string[] args)
        {
            // Get available providers
            List<ProviderConfiguration> providers;

            try
            {
                providers = ProviderManager.ReadProviders(CancellationToken.None).ToList();
            }
            catch (Exception)
            {
                Console.WriteLine("No providers registered. Run BigDrive.Setup.exe first.");
                return;
            }

            if (providers.Count == 0)
            {
                Console.WriteLine("No providers registered. Run BigDrive.Setup.exe first.");
                return;
            }

            // If no arguments, show interactive mode
            if (args.Length == 0)
            {
                InteractiveMount(context, providers);
                return;
            }

            // If arguments provided, try direct mount
            if (args.Length >= 2)
            {
                DirectMount(context, providers, args);
                return;
            }

            Console.WriteLine("Usage: " + Usage);
            Console.WriteLine();
            Console.WriteLine("Run 'mount' without arguments for interactive mode.");
        }

        /// <summary>
        /// Interactive mount - prompts user for provider and name.
        /// </summary>
        private static void InteractiveMount(ShellContext context, List<ProviderConfiguration> providers)
        {
            Console.WriteLine();
            Console.WriteLine("Available providers:");
            Console.WriteLine();

            for (int i = 0; i < providers.Count; i++)
            {
                Console.WriteLine("  [{0}] {1}", i + 1, providers[i].Name);
                Console.WriteLine("      CLSID: {0}", providers[i].Id);
            }

            Console.WriteLine();
            Console.Write("Select provider number: ");
            string providerInput = Console.ReadLine();

            if (!int.TryParse(providerInput, out int providerIndex) ||
                providerIndex < 1 || providerIndex > providers.Count)
            {
                Console.WriteLine("Invalid provider selection.");
                return;
            }

            ProviderConfiguration selectedProvider = providers[providerIndex - 1];

            Console.Write("Enter drive name: ");
            string driveName = Console.ReadLine();

            if (string.IsNullOrWhiteSpace(driveName))
            {
                Console.WriteLine("Drive name cannot be empty.");
                return;
            }

            CreateDrive(context, selectedProvider, driveName.Trim());
        }

        /// <summary>
        /// Direct mount with command line arguments.
        /// </summary>
        private static void DirectMount(ShellContext context, List<ProviderConfiguration> providers, string[] args)
        {
            string providerSelector = args[0];
            string driveName = string.Join(" ", args.Skip(1));

            ProviderConfiguration selectedProvider = null;

            // Try to parse as number
            if (int.TryParse(providerSelector, out int index))
            {
                if (index >= 1 && index <= providers.Count)
                {
                    selectedProvider = providers[index - 1];
                }
            }

            // Try to match by name
            if (selectedProvider == null)
            {
                foreach (ProviderConfiguration provider in providers)
                {
                    if (string.Equals(provider.Name, providerSelector, StringComparison.OrdinalIgnoreCase))
                    {
                        selectedProvider = provider;
                        break;
                    }
                }
            }

            // Try to match by CLSID
            if (selectedProvider == null && Guid.TryParse(providerSelector, out Guid clsid))
            {
                foreach (ProviderConfiguration provider in providers)
                {
                    if (provider.Id == clsid)
                    {
                        selectedProvider = provider;
                        break;
                    }
                }
            }

            if (selectedProvider == null)
            {
                Console.WriteLine("Provider not found: " + providerSelector);
                Console.WriteLine("Use 'mount' without arguments to see available providers.");
                return;
            }

            if (string.IsNullOrWhiteSpace(driveName))
            {
                Console.WriteLine("Drive name cannot be empty.");
                return;
            }

            CreateDrive(context, selectedProvider, driveName.Trim());
        }

        /// <summary>
        /// Creates a new drive configuration. Queries the provider for required parameters
        /// via <see cref="IBigDriveDriveInfo"/> and prompts the user for each value.
        /// </summary>
        /// <param name="context">The shell context.</param>
        /// <param name="provider">The selected provider configuration.</param>
        /// <param name="driveName">The user-specified drive name.</param>
        private static void CreateDrive(ShellContext context, ProviderConfiguration provider, string driveName)
        {
            try
            {
                // Generate a new GUID for this drive
                Guid driveId = Guid.NewGuid();

                DriveConfiguration driveConfig = new DriveConfiguration
                {
                    Id = driveId,
                    Name = driveName,
                    CLSID = provider.Id
                };

                // Query the provider for required drive parameters
                Dictionary<string, string> secrets;
                Dictionary<string, string> properties = QueryDriveParameters(provider.Id, out secrets);
                if (properties == null)
                {
                    // User cancelled
                    Console.WriteLine("Mount cancelled.");
                    return;
                }

                driveConfig.Properties = properties;

                DriveManager.WriteConfiguration(driveConfig, CancellationToken.None);

                // Store secret parameters in Windows Credential Manager
                foreach (KeyValuePair<string, string> secret in secrets)
                {
                    DriveManager.WriteSecretProperty(driveId, secret.Key, secret.Value, CancellationToken.None);
                }

                Console.WriteLine();
                Console.WriteLine("Drive mounted successfully!");
                Console.WriteLine();
                Console.WriteLine("  Name:     {0}", driveName);
                Console.WriteLine("  GUID:     {0}", driveId);
                Console.WriteLine("  Provider: {0}", provider.Name);
                Console.WriteLine();

                // Refresh drive letters
                context.RefreshDrives();

                // Show the new drive letter
                char newLetter = context.DriveLetterManager.GetDriveLetter(driveId);
                if (newLetter != '\0')
                {
                    Console.WriteLine("Use 'cd {0}:' to access the new drive.", newLetter);
                }
                else
                {
                    Console.WriteLine("Use 'dir' to see the new drive.");
                }
            }
            catch (UnauthorizedAccessException)
            {
                Console.WriteLine("Access denied. Run BigDrive.Shell as Administrator to mount drives.");
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to mount drive: " + ex.Message);
            }
        }

        /// <summary>
        /// Queries the provider for required drive parameters via <see cref="IBigDriveDriveInfo"/>
        /// and prompts the user for each value.
        /// </summary>
        /// <param name="providerClsid">The CLSID of the provider to query.</param>
        /// <param name="secrets">
        /// Output dictionary of secret parameter name/value pairs that should be stored
        /// in Windows Credential Manager rather than the registry.
        /// </param>
        /// <returns>
        /// A dictionary of non-secret parameter name/value pairs collected from the user,
        /// or null if the user cancelled.
        /// </returns>
        private static Dictionary<string, string> QueryDriveParameters(Guid providerClsid, out Dictionary<string, string> secrets)
        {
            Dictionary<string, string> properties = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            secrets = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

            IBigDriveDriveInfo driveInfo = ProviderFactory.GetDriveInfoProvider(providerClsid);
            if (driveInfo == null)
            {
                // Provider does not require custom parameters
                return properties;
            }

            string json = driveInfo.GetDriveParameters();
            if (string.IsNullOrWhiteSpace(json))
            {
                return properties;
            }

            JsonElement[] parameters;
            try
            {
                parameters = JsonSerializer.Deserialize<JsonElement[]>(json);
            }
            catch (JsonException)
            {
                Console.WriteLine("Warning: Provider returned invalid parameter definitions.");
                return properties;
            }

            if (parameters == null || parameters.Length == 0)
            {
                return properties;
            }

            Console.WriteLine();
            Console.WriteLine("This provider requires the following parameters:");
            Console.WriteLine();

            foreach (JsonElement param in parameters)
            {
                string name = string.Empty;
                string description = string.Empty;
                string type = "string";

                if (param.TryGetProperty("name", out JsonElement nameElement))
                {
                    name = nameElement.GetString();
                }

                if (param.TryGetProperty("description", out JsonElement descElement))
                {
                    description = descElement.GetString();
                }

                if (param.TryGetProperty("type", out JsonElement typeElement))
                {
                    type = typeElement.GetString() ?? "string";
                }

                if (string.IsNullOrWhiteSpace(name))
                {
                    continue;
                }

                if (!string.IsNullOrWhiteSpace(description))
                {
                    Console.WriteLine("  {0}", description);
                }

                Console.Write("  {0}: ", name);

                bool isSecret = string.Equals(type, "secret", StringComparison.OrdinalIgnoreCase);

                string value;
                if (string.Equals(type, "existing-file", StringComparison.OrdinalIgnoreCase))
                {
                    value = ReadLineWithFileCompletion();
                    if (!string.IsNullOrWhiteSpace(value) && !File.Exists(value))
                    {
                        Console.WriteLine();
                        Console.WriteLine("  Error: File does not exist: {0}", value);
                        return null;
                    }
                }
                else if (string.Equals(type, "filepath", StringComparison.OrdinalIgnoreCase) ||
                         string.Equals(type, "file", StringComparison.OrdinalIgnoreCase))
                {
                    value = ReadLineWithFileCompletion();
                }
                else if (isSecret)
                {
                    value = ReadMaskedInput();
                    Console.WriteLine();
                }
                else
                {
                    value = Console.ReadLine();
                }

                if (value == null)
                {
                    // User pressed Ctrl+C or input stream closed
                    return null;
                }

                if (isSecret)
                {
                    secrets[name] = value.Trim();
                }
                else
                {
                    properties[name] = value.Trim();
                }
            }

            return properties;
        }

        /// <summary>
        /// Reads a line of input from the console with Tab file-path completion.
        /// Pressing Tab cycles through matching files and directories on the local filesystem.
        /// </summary>
        /// <returns>The input line, or null if the input stream was closed.</returns>
        private static string ReadLineWithFileCompletion()
        {
            List<char> buffer = new List<char>();
            int cursorPos = 0;

            List<string> completionCandidates = new List<string>();
            int completionIndex = 0;
            string textBeforeCompletion = null;

            while (true)
            {
                ConsoleKeyInfo keyInfo = Console.ReadKey(intercept: true);

                if (keyInfo.Key == ConsoleKey.Enter)
                {
                    Console.WriteLine();
                    return new string(buffer.ToArray());
                }

                if (keyInfo.Key == ConsoleKey.Tab)
                {
                    string currentText = new string(buffer.ToArray());

                    if (textBeforeCompletion == null)
                    {
                        // Start new completion cycle
                        textBeforeCompletion = currentText;
                        completionCandidates = GetLocalFileCompletions(textBeforeCompletion);
                        completionIndex = 0;
                    }
                    else
                    {
                        // Cycle to next candidate
                        if (completionCandidates.Count > 0)
                        {
                            bool reverse = (keyInfo.Modifiers & ConsoleModifiers.Shift) != 0;
                            if (reverse)
                            {
                                completionIndex--;
                                if (completionIndex < 0)
                                {
                                    completionIndex = completionCandidates.Count - 1;
                                }
                            }
                            else
                            {
                                completionIndex++;
                                if (completionIndex >= completionCandidates.Count)
                                {
                                    completionIndex = 0;
                                }
                            }
                        }
                    }

                    if (completionCandidates.Count > 0)
                    {
                        string completion = completionCandidates[completionIndex];

                        // Erase current text on screen
                        RedrawInput(buffer, cursorPos, string.Empty);

                        buffer.Clear();
                        buffer.AddRange(completion);
                        cursorPos = buffer.Count;

                        // Draw new text
                        Console.Write(completion);
                    }

                    continue;
                }

                // Any non-Tab key resets completion state
                textBeforeCompletion = null;
                completionCandidates.Clear();

                if (keyInfo.Key == ConsoleKey.Backspace)
                {
                    if (cursorPos > 0)
                    {
                        buffer.RemoveAt(cursorPos - 1);
                        cursorPos--;

                        // Redraw from cursor position
                        Console.Write("\b");
                        string remaining = new string(buffer.ToArray(), cursorPos, buffer.Count - cursorPos);
                        Console.Write(remaining + " ");
                        Console.Write(new string('\b', remaining.Length + 1));
                    }

                    continue;
                }

                if (keyInfo.Key == ConsoleKey.Escape)
                {
                    RedrawInput(buffer, cursorPos, string.Empty);
                    buffer.Clear();
                    cursorPos = 0;
                    continue;
                }

                if (keyInfo.Key == ConsoleKey.LeftArrow)
                {
                    if (cursorPos > 0)
                    {
                        cursorPos--;
                        Console.Write("\b");
                    }

                    continue;
                }

                if (keyInfo.Key == ConsoleKey.RightArrow)
                {
                    if (cursorPos < buffer.Count)
                    {
                        Console.Write(buffer[cursorPos]);
                        cursorPos++;
                    }

                    continue;
                }

                if (keyInfo.KeyChar >= 32)
                {
                    buffer.Insert(cursorPos, keyInfo.KeyChar);
                    cursorPos++;

                    // Write char and any text after cursor
                    string tail = new string(buffer.ToArray(), cursorPos - 1, buffer.Count - cursorPos + 1);
                    Console.Write(tail);
                    if (tail.Length > 1)
                    {
                        Console.Write(new string('\b', tail.Length - 1));
                    }
                }
            }
        }

        /// <summary>
        /// Gets local filesystem path completions matching the given prefix.
        /// Returns matching files and directories sorted alphabetically.
        /// </summary>
        /// <param name="prefix">The current path prefix to complete.</param>
        /// <returns>List of matching file and directory paths.</returns>
        private static List<string> GetLocalFileCompletions(string prefix)
        {
            List<string> candidates = new List<string>();

            try
            {
                string directory;
                string searchPrefix;

                int lastSeparator = prefix.LastIndexOfAny(new[] { '\\', '/' });
                if (lastSeparator >= 0)
                {
                    directory = prefix.Substring(0, lastSeparator + 1);
                    searchPrefix = prefix.Substring(lastSeparator + 1);
                }
                else
                {
                    directory = ".\\";
                    searchPrefix = prefix;
                }

                // Normalize the directory path to resolve .. and . segments
                try
                {
                    if (!string.IsNullOrEmpty(directory))
                    {
                        directory = Path.GetFullPath(directory);
                    }
                }
                catch
                {
                    // If normalization fails, use original path
                }

                if (!Directory.Exists(directory))
                {
                    return candidates;
                }

                foreach (string dir in Directory.GetDirectories(directory))
                {
                    string dirName = Path.GetFileName(dir);
                    if (string.IsNullOrEmpty(searchPrefix) || dirName.StartsWith(searchPrefix, StringComparison.OrdinalIgnoreCase))
                    {
                        candidates.Add(dir + "\\");
                    }
                }

                foreach (string file in Directory.GetFiles(directory))
                {
                    string fileName = Path.GetFileName(file);
                    if (string.IsNullOrEmpty(searchPrefix) || fileName.StartsWith(searchPrefix, StringComparison.OrdinalIgnoreCase))
                    {
                        candidates.Add(file);
                    }
                }

                candidates.Sort(StringComparer.OrdinalIgnoreCase);
            }
            catch
            {
                // Ignore errors during completion (access denied, etc.)
            }

            return candidates;
        }

        /// <summary>
        /// Erases the current buffer text on screen and optionally writes replacement text.
        /// </summary>
        /// <param name="buffer">The current buffer characters.</param>
        /// <param name="cursorPos">The current cursor position within the buffer.</param>
        /// <param name="replacement">The replacement text to write (empty to just erase).</param>
        private static void RedrawInput(List<char> buffer, int cursorPos, string replacement)
        {
            // Move cursor to start of input
            if (cursorPos > 0)
            {
                Console.Write(new string('\b', cursorPos));
            }

            // Overwrite with spaces
            Console.Write(new string(' ', buffer.Count));

            // Move back to start
            Console.Write(new string('\b', buffer.Count));

            // Write replacement
            Console.Write(replacement);
        }

        /// <summary>
        /// Reads input from the console with masked characters (asterisks).
        /// Each typed character is displayed as '*' to prevent secret values
        /// from being visible on screen.
        /// </summary>
        /// <returns>The entered string.</returns>
        private static string ReadMaskedInput()
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
