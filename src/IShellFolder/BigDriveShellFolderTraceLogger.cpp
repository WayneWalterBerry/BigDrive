// <copyright file="BigDriveShellFolderTraceLogger.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveShellFolderTraceLogger.h"

// Provider Id: {A356D4CC-CDAC-4894-A93D-35C4C3F84944}
TRACELOGGING_DEFINE_PROVIDER(
    g_hMyProvider,
    "BigDrive.ShellFolder",
    (0xa356d4cc, 0xcdac, 0x4894, 0xa9, 0x3d, 0x35, 0xc4, 0xc3, 0xf8, 0x49, 0x44)
);

/// <summary>
/// Registers the trace logging provider.
/// </summary>
void BigDriveShellFolderTraceLogger::Initialize()
{
    TraceLoggingRegister(g_hMyProvider);
}

/// <summary>
/// Unregisters the trace logging provider.
/// </summary>
void BigDriveShellFolderTraceLogger::Uninitialize()
{
    TraceLoggingUnregister(g_hMyProvider);
}

/// <summary>
/// Logs a custom event message.
/// </summary>
/// <param name="message">The message to log.</param>
void BigDriveShellFolderTraceLogger::LogEvent(const char* message)
{
    TraceLoggingWrite(g_hMyProvider, "BigDriveShellFolderEvent", TraceLoggingValue(message, "Message"));
}

/// <summary>
/// Logs entry into a function.
/// </summary>
/// <param name="functionName">The name of the function being entered.</param>
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName)
{
    TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"));
}