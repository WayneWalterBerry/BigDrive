// <copyright file="BigDriveShellFolderEventLogger.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <cwchar>

/// <summary>
/// Provides functionality to log messages to the Windows Event Viewer.
/// </summary>
class BigDriveShellFolderEventLogger
{
private:
    /// <summary>
    /// Private property to store the event log handle.
    /// </summary>
    HANDLE hEventLog;

public:
    /// <summary>
    /// Constructor to initialize the event log handle with the specified event source name.
    /// </summary>
    /// <param name="eventSourceName">The name of the event source to register with the Event Viewer.</param>
    explicit BigDriveShellFolderEventLogger(LPCWSTR eventSourceName);

    /// <summary>
    /// Destructor to clean up the event log handle.
    /// </summary>
    ~BigDriveShellFolderEventLogger();

    /// <summary>
    /// Write an error message to the Event Viewer.
    /// </summary>
    HRESULT WriteError(LPCWSTR message);

    /// <summary>
    /// Write a formatted error message to the Event Viewer.
    /// </summary>
    HRESULT WriteErrorFormmated(LPCWSTR formatter, ...);

    /// <summary>
    /// Write an informational message to the Event Viewer.
    /// </summary>
    HRESULT WriteInfo(LPCWSTR formatter, ...);

private:
    /// <summary>
    /// Write to the Event Viewer.
    /// </summary>
    HRESULT WriteToEventViewer(LPCWSTR message, WORD eventType = EVENTLOG_INFORMATION_TYPE);

    /// <summary>
    /// Overload function to WriteToEventViewer with LPCSTR formatter and arguments.
    /// </summary>
    HRESULT WriteToEventViewer(LPCSTR formatter, WORD eventType, ...);
};
