// <copyright file="BigDriveShellFolderTraceLogger.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// Provider GUID for BigDriveShellFolderTraceLogger: {A356D4CC-CDAC-4894-A93D-35C4C3F84944}

#pragma once

#include <TraceLoggingProvider.h>

// Provider Id: {A356D4CC-CDAC-4894-A93D-35C4C3F84944}
TRACELOGGING_DEFINE_PROVIDER(
	g_hMyProvider,
	"BigDrive.ShellFolder",
	(0xa356d4cc, 0xcdac, 0x4894, 0xa9, 0x3d, 0x35, 0xc4, 0xc3, 0xf8, 0x49, 0x44)
);

class TraceLogger
{
public:

	static void Initialize()
	{
		TraceLoggingRegister(g_hMyProvider);
	}

	static void Uninitialize()
	{
		TraceLoggingUnregister(g_hMyProvider);
	}

	static void LogEvent(const char* message)
	{
		TraceLoggingWrite(g_hMyProvider, "BigDriveShellFolderEvent", TraceLoggingValue(message, "Message"));
	}
};