// <copyright file="BigDriveShellFolderEventLogger.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <iostream>

#include "BigDriveShellFolderEventLogger.h"

/// <summary>
/// Constructor to initialize the event log handle with the specified event source name.
/// </summary>
/// <param name="eventSourceName">The name of the event source to register with the Event Viewer.</param>
BigDriveShellFolderEventLogger::BigDriveShellFolderEventLogger(LPCWSTR eventSourceName)
{
    // Register the event source
    hEventLog = RegisterEventSourceW(nullptr, eventSourceName);
    if (hEventLog == nullptr)
    {

    }
}

/// <summary>
/// Destructor to clean up the event log handle.
/// </summary>
BigDriveShellFolderEventLogger::~BigDriveShellFolderEventLogger()
{
    if (hEventLog != nullptr)
    {
        DeregisterEventSource(hEventLog);
    }
}

/// <summary>
/// Write an error message to the Event Viewer.
/// </summary>
HRESULT BigDriveShellFolderEventLogger::WriteError(LPCWSTR message)
{
    return WriteToEventViewer(message, EVENTLOG_ERROR_TYPE);
}

/// <summary>
/// Write a formatted error message to the Event Viewer.
/// </summary>
HRESULT BigDriveShellFolderEventLogger::WriteErrorFormmated(LPCWSTR formatter, ...)
{
    va_list args;
    va_start(args, formatter);

    wchar_t buffer[1024];
    ::vswprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), formatter, args);

    va_end(args);

    return WriteToEventViewer(buffer, EVENTLOG_ERROR_TYPE);
}

/// <summary>
/// Write an informational message to the Event Viewer.
/// </summary>
HRESULT BigDriveShellFolderEventLogger::WriteInfo(LPCWSTR formatter, ...)
{
    va_list args;
    va_start(args, formatter);

    wchar_t buffer[1024];
    ::vswprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), formatter, args);

    va_end(args);

    return WriteToEventViewer(buffer, EVENTLOG_INFORMATION_TYPE);
}

/// <summary>
/// Write to the Event Viewer.
/// </summary>
HRESULT BigDriveShellFolderEventLogger::WriteToEventViewer(LPCWSTR message, WORD eventType)
{
    // Write to the Visual Studio Debug Output window
    OutputDebugStringW(message);
    OutputDebugStringW(L"\n"); // Optional: add a newline for readability

    if (hEventLog == nullptr)
    {
        return E_FAIL; // Event log handle is not initialized
    }

    // Write the event
    if (!ReportEventW(
        hEventLog,   // Event log handle
        eventType,   // Event type (e.g., information, warning, error)
        0,           // Event category
        0x1000,      // Event identifier (customize as needed)
        nullptr,     // No user security identifier
        1,           // Number of strings
        0,           // No binary data
        &message,    // Array of strings
        nullptr))    // No binary data
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

/// <summary>
/// Overload function to WriteToEventViewer with LPCSTR formatter and arguments.
/// </summary>
HRESULT BigDriveShellFolderEventLogger::WriteToEventViewer(LPCSTR formatter, WORD eventType, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, eventType);

    ::vsnprintf(buffer, sizeof(buffer), formatter, args);

    va_end(args);

    wchar_t wBuffer[1024];
    MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wBuffer, sizeof(wBuffer) / sizeof(wBuffer[0]));

    return WriteToEventViewer(wBuffer, eventType);
}
