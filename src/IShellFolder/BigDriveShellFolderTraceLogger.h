// <copyright file="BigDriveShellFolderTraceLogger.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// Provider GUID for BigDriveShellFolderTraceLogger: {A356D4CC-CDAC-4894-A93D-35C4C3F84944}

#pragma once

#include <TraceLoggingProvider.h>

/// <summary>
/// Provides static methods for trace logging events related to the BigDrive Shell Folder.
/// </summary>
class BigDriveShellFolderTraceLogger
{
public:
    /// <summary>
    /// Registers the trace logging provider.
    /// </summary>
    static void Initialize();

    /// <summary>
    /// Unregisters the trace logging provider.
    /// </summary>
    static void Uninitialize();

    /// <summary>
    /// Logs a custom event message.
    /// </summary>
    /// <param name="message">The message to log.</param>
    static void LogEvent(const char* message);

    /// <summary>
    /// Logs entry into a function.
    /// </summary>
    /// <param name="functionName">The name of the function being entered.</param>
    static void LogEnter(LPCSTR functionName);
};