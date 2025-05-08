// <copyright file="EventLogger.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <cwchar>

class EventLogger
{
private: 

    /// <summary>
    /// Private property to store the event log handle
    /// </summary>
    HANDLE hEventLog;

public:

    // Get the singleton instance
    static EventLogger& GetInstance()
    {
        static EventLogger instance;
        return instance;
    }

    /// <summary>
    /// Write a error message to the Event Viewer
    /// </summary>
    HRESULT WriteError(LPCWSTR message)
    {
        return WriteToEventViewer(message, EVENTLOG_ERROR_TYPE);
    }

    /// <summary>
    /// Write Formatted Error Message to Event Viewer
    /// </summary>
    HRESULT WriteError(LPCWSTR formatter, ...)
    {
        va_list args;
        va_start(args, formatter);

        wchar_t buffer[1024];
        ::vswprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), formatter, args);

        va_end(args);

        return WriteToEventViewer(buffer, EVENTLOG_ERROR_TYPE);
    }

    /// <summary>
    /// Write Informational Message To Event Viewer
    /// </summary>
    HRESULT WriteInfo(LPCWSTR formatter, ...)
    {
        va_list args;
        va_start(args, formatter);

        wchar_t buffer[1024];
        ::vswprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), formatter, args);

        va_end(args);

        return WriteToEventViewer(buffer, EVENTLOG_INFORMATION_TYPE);
    }

private:

    /// <summary>
    /// Constructor to initialize the event log handle
    /// </summary>
    EventLogger()
    {
        LPCWSTR eventSourceName = L"BigDrive.ShellFolder";

        // Register the event source
        hEventLog = RegisterEventSourceW(nullptr, eventSourceName);
        if (hEventLog == nullptr)
        {
            // Failed to register the event source
            // Handle the error appropriately (e.g., log to a file or throw an exception)
        }
    }

    /// <summary>
    /// Destructor to clean up the event log handle
    /// </summary>
    ~EventLogger()
    {
        if (hEventLog != nullptr)
        {
            DeregisterEventSource(hEventLog);
        }
    }

    // Delete copy constructor and assignment operator
    EventLogger(const EventLogger&) = delete;
    EventLogger& operator=(const EventLogger&) = delete;

    /// <summary>
    /// Write To the Event Viewer
    /// </summary>
    HRESULT WriteToEventViewer(LPCWSTR message, WORD eventType = EVENTLOG_INFORMATION_TYPE);

    /// <summary>
    /// Overload function to WriteToEventViewer with LPCSTR formatter and arguments
    /// </summary>
    HRESULT WriteToEventViewer(LPCSTR formatter, WORD eventType, ...);
};

