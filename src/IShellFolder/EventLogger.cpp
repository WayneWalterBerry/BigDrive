// <copyright file="EventLogger.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <windows.h>
#include <cwchar>

#include "EventLogger.h"

// Static method to write to the Event Viewer
HRESULT EventLogger::WriteToEventViewer(LPCWSTR message, WORD eventType)
{
    // Define the event source name
    LPCWSTR eventSourceName = L"BigDrive.ShellFolder";

    // Write the event
    ReportEventW(
        hEventLog,   // Event log handle
        eventType,                // Event type (e.g., information, warning, error)
        0,                        // Event category
        0x1000,                   // Event identifier (customize as needed)
        nullptr,                  // No user security identifier
        1,                        // Number of strings
        0,                        // No binary data
        &message,                 // Array of strings
        nullptr                   // No binary data
    );

    return S_OK;
}

// Overload function to WriteToEventViewer with LPCSTR formatter and arguments
HRESULT EventLogger::WriteToEventViewer(LPCSTR formatter, WORD eventType, ...)
{
    // Buffer to hold the formatted message
    char buffer[1024];

    // Initialize variable argument list
    va_list args;
    va_start(args, eventType);

    // Format the message
    ::vsnprintf(buffer, sizeof(buffer), formatter, args);

    // End variable argument list
    va_end(args);

    // Convert the formatted message to wide string
    wchar_t wBuffer[1024];
    MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wBuffer, sizeof(wBuffer) / sizeof(wBuffer[0]));

    // Call the original WriteToEventViewer with the formatted wide string
    WriteToEventViewer(wBuffer, eventType);

    return S_OK;
}