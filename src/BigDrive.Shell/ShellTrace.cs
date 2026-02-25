// <copyright file="ShellTrace.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Diagnostics;

    /// <summary>
    /// Provides centralized tracing and debug output for the BigDrive Shell.
    /// When debug mode is enabled (-d or --debug), trace messages are written to the console.
    /// </summary>
    public static class ShellTrace
    {
        /// <summary>
        /// The trace source for shell operations.
        /// </summary>
        private static readonly TraceSource s_traceSource;

        /// <summary>
        /// Console trace listener for debug mode.
        /// </summary>
        private static ConsoleTraceListener s_consoleListener;

        /// <summary>
        /// Whether debug mode is enabled.
        /// </summary>
        private static bool s_debugEnabled;

        /// <summary>
        /// Initializes static members of the <see cref="ShellTrace"/> class.
        /// </summary>
        static ShellTrace()
        {
            s_traceSource = new TraceSource("BigDrive.Shell", SourceLevels.All);
            s_traceSource.Listeners.Clear();
            s_debugEnabled = false;
        }

        /// <summary>
        /// Gets or sets a value indicating whether debug mode is enabled.
        /// </summary>
        public static bool DebugEnabled
        {
            get { return s_debugEnabled; }
            set
            {
                s_debugEnabled = value;
                ConfigureListeners();
            }
        }

        /// <summary>
        /// Gets the trace source for advanced configuration.
        /// </summary>
        public static TraceSource TraceSource
        {
            get { return s_traceSource; }
        }

        /// <summary>
        /// Writes a verbose trace message.
        /// </summary>
        /// <param name="message">The message to write.</param>
        public static void Verbose(string message)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Verbose, message);
            }
        }

        /// <summary>
        /// Writes a verbose trace message with format arguments.
        /// </summary>
        /// <param name="format">The format string.</param>
        /// <param name="args">The format arguments.</param>
        public static void Verbose(string format, params object[] args)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Verbose, string.Format(format, args));
            }
        }

        /// <summary>
        /// Writes an information trace message.
        /// </summary>
        /// <param name="message">The message to write.</param>
        public static void Info(string message)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Information, message);
            }
        }

        /// <summary>
        /// Writes an information trace message with format arguments.
        /// </summary>
        /// <param name="format">The format string.</param>
        /// <param name="args">The format arguments.</param>
        public static void Info(string format, params object[] args)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Information, string.Format(format, args));
            }
        }

        /// <summary>
        /// Writes a warning trace message.
        /// </summary>
        /// <param name="message">The message to write.</param>
        public static void Warning(string message)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Warning, message);
            }
        }

        /// <summary>
        /// Writes a warning trace message with format arguments.
        /// </summary>
        /// <param name="format">The format string.</param>
        /// <param name="args">The format arguments.</param>
        public static void Warning(string format, params object[] args)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Warning, string.Format(format, args));
            }
        }

        /// <summary>
        /// Writes an error trace message.
        /// </summary>
        /// <param name="message">The message to write.</param>
        public static void Error(string message)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Error, message);
            }
        }

        /// <summary>
        /// Writes an error trace message with format arguments.
        /// </summary>
        /// <param name="format">The format string.</param>
        /// <param name="args">The format arguments.</param>
        public static void Error(string format, params object[] args)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Error, string.Format(format, args));
            }
        }

        /// <summary>
        /// Traces entry into a method.
        /// </summary>
        /// <param name="className">The class name.</param>
        /// <param name="methodName">The method name.</param>
        public static void Enter(string className, string methodName)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Verbose, "[ENTER] {0}.{1}", className, methodName);
            }
        }

        /// <summary>
        /// Traces entry into a method with parameters.
        /// </summary>
        /// <param name="className">The class name.</param>
        /// <param name="methodName">The method name.</param>
        /// <param name="parameters">Parameter description.</param>
        public static void Enter(string className, string methodName, string parameters)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Verbose, "[ENTER] {0}.{1}({2})", className, methodName, parameters);
            }
        }

        /// <summary>
        /// Traces exit from a method.
        /// </summary>
        /// <param name="className">The class name.</param>
        /// <param name="methodName">The method name.</param>
        public static void Exit(string className, string methodName)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Verbose, "[EXIT] {0}.{1}", className, methodName);
            }
        }

        /// <summary>
        /// Traces exit from a method with result.
        /// </summary>
        /// <param name="className">The class name.</param>
        /// <param name="methodName">The method name.</param>
        /// <param name="result">The result description.</param>
        public static void Exit(string className, string methodName, string result)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Verbose, "[EXIT] {0}.{1} => {2}", className, methodName, result);
            }
        }

        /// <summary>
        /// Traces a COM call to a provider.
        /// </summary>
        /// <param name="interfaceName">The interface name.</param>
        /// <param name="methodName">The method name.</param>
        /// <param name="driveGuid">The drive GUID.</param>
        public static void ComCall(string interfaceName, string methodName, Guid driveGuid)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Information, "[COM] {0}.{1}(driveGuid={2})", interfaceName, methodName, driveGuid);
            }
        }

        /// <summary>
        /// Traces a COM call to a provider with additional parameters.
        /// </summary>
        /// <param name="interfaceName">The interface name.</param>
        /// <param name="methodName">The method name.</param>
        /// <param name="parameters">The parameters.</param>
        public static void ComCall(string interfaceName, string methodName, string parameters)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Information, "[COM] {0}.{1}({2})", interfaceName, methodName, parameters);
            }
        }

        /// <summary>
        /// Traces the result of a COM call.
        /// </summary>
        /// <param name="interfaceName">The interface name.</param>
        /// <param name="methodName">The method name.</param>
        /// <param name="hresult">The HRESULT returned.</param>
        public static void ComResult(string interfaceName, string methodName, int hresult)
        {
            if (s_debugEnabled)
            {
                string status = hresult == 0 ? "S_OK" : "0x" + hresult.ToString("X8");
                TraceEventType eventType = hresult == 0 ? TraceEventType.Information : TraceEventType.Warning;
                WriteTrace(eventType, "[COM] {0}.{1} returned {2}", interfaceName, methodName, status);
            }
        }

        /// <summary>
        /// Traces a COM call result with additional info.
        /// </summary>
        /// <param name="interfaceName">The interface name.</param>
        /// <param name="methodName">The method name.</param>
        /// <param name="hresult">The HRESULT.</param>
        /// <param name="additionalInfo">Additional result info.</param>
        public static void ComResult(string interfaceName, string methodName, int hresult, string additionalInfo)
        {
            if (s_debugEnabled)
            {
                string status = hresult == 0 ? "S_OK" : "0x" + hresult.ToString("X8");
                TraceEventType eventType = hresult == 0 ? TraceEventType.Information : TraceEventType.Warning;
                WriteTrace(eventType, "[COM] {0}.{1} returned {2}: {3}", interfaceName, methodName, status, additionalInfo);
            }
        }

        /// <summary>
        /// Traces path resolution.
        /// </summary>
        /// <param name="input">The input path.</param>
        /// <param name="resolved">The resolved path.</param>
        /// <param name="pathType">The type of path (BigDrive, OS, Relative).</param>
        public static void PathResolution(string input, string resolved, string pathType)
        {
            if (s_debugEnabled)
            {
                WriteTrace(TraceEventType.Verbose, "[PATH] \"{0}\" => \"{1}\" ({2})", input, resolved, pathType);
            }
        }

        /// <summary>
        /// Writes the trace message with timestamp and formatting.
        /// </summary>
        /// <param name="eventType">The event type.</param>
        /// <param name="message">The message.</param>
        private static void WriteTrace(TraceEventType eventType, string message)
        {
            string prefix = GetPrefix(eventType);
            ConsoleColor originalColor = Console.ForegroundColor;

            try
            {
                Console.ForegroundColor = GetColor(eventType);
                Console.WriteLine("{0} [{1:HH:mm:ss.fff}] {2}", prefix, DateTime.Now, message);
            }
            finally
            {
                Console.ForegroundColor = originalColor;
            }
        }

        /// <summary>
        /// Writes the trace message with timestamp and formatting using format args.
        /// </summary>
        /// <param name="eventType">The event type.</param>
        /// <param name="format">The format string.</param>
        /// <param name="args">The format arguments.</param>
        private static void WriteTrace(TraceEventType eventType, string format, params object[] args)
        {
            WriteTrace(eventType, string.Format(format, args));
        }

        /// <summary>
        /// Gets the prefix for the event type.
        /// </summary>
        /// <param name="eventType">The event type.</param>
        /// <returns>The prefix string.</returns>
        private static string GetPrefix(TraceEventType eventType)
        {
            switch (eventType)
            {
                case TraceEventType.Error:
                    return "ERR ";
                case TraceEventType.Warning:
                    return "WARN";
                case TraceEventType.Information:
                    return "INFO";
                case TraceEventType.Verbose:
                    return "DBG ";
                default:
                    return "    ";
            }
        }

        /// <summary>
        /// Gets the console color for the event type.
        /// </summary>
        /// <param name="eventType">The event type.</param>
        /// <returns>The console color.</returns>
        private static ConsoleColor GetColor(TraceEventType eventType)
        {
            switch (eventType)
            {
                case TraceEventType.Error:
                    return ConsoleColor.Red;
                case TraceEventType.Warning:
                    return ConsoleColor.Yellow;
                case TraceEventType.Information:
                    return ConsoleColor.Cyan;
                case TraceEventType.Verbose:
                    return ConsoleColor.DarkGray;
                default:
                    return ConsoleColor.Gray;
            }
        }

        /// <summary>
        /// Configures trace listeners based on debug mode.
        /// </summary>
        private static void ConfigureListeners()
        {
            if (s_debugEnabled)
            {
                if (s_consoleListener == null)
                {
                    s_consoleListener = new ConsoleTraceListener();
                    s_traceSource.Listeners.Add(s_consoleListener);
                }

                s_traceSource.Switch.Level = SourceLevels.All;
            }
            else
            {
                if (s_consoleListener != null)
                {
                    s_traceSource.Listeners.Remove(s_consoleListener);
                    s_consoleListener.Dispose();
                    s_consoleListener = null;
                }

                s_traceSource.Switch.Level = SourceLevels.Off;
            }
        }
    }
}
