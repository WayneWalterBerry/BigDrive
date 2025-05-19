// <copyright file="BigDriveETWLogger.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// Include standard Windows headers first
#include <windows.h>
#include <winmeta.h>
#include <evntprov.h>
#include <strsafe.h>

// Include the generated ETW header
#include "Generated/BigDriveEvents.h"

#include "LaunchDebugger.h"

/// <summary>
/// Helper class for BigDrive ETW (Event Tracing for Windows) logging operations.
/// Provides static methods to initialize the ETW provider, log information and error events,
/// and clean up resources when done.
/// </summary>
class BigDriveETWLogger
{
public:
    /// <summary>
    /// Initializes the BigDrive ETW provider.
    /// Must be called before any logging operations can be performed.
    /// </summary>
    /// <returns>
    /// TRUE if the ETW provider was successfully registered; FALSE otherwise.
    /// </returns>
    static BOOL Initialize()
    {
        ULONG status = EventRegisterBigDriveAnalytic();
        return (status == ERROR_SUCCESS);
    }

    /// <summary>
    /// Unregisters the BigDrive ETW provider and releases associated resources.
    /// Should be called when the application is shutting down.
    /// </summary>
    static void Cleanup()
    {
        EventRegisterBigDriveAnalytic();
    }

    /// <summary>
    /// Logs an informational event with the specified message.
    /// Only writes the event if the BIGDRIVE_EVENT_INFO event is enabled in the ETW session.
    /// </summary>
    /// <param name="message">The message to log. Must be a null-terminated wide string.</param>
    static void LogInfo(LPCWSTR message)
    {
        if (EventEnabledBIGDRIVE_EVENT_INFO())
        {
            EventWriteBIGDRIVE_EVENT_INFO(message);
        }
    }

    /// <summary>
    /// Logs an error event with the specified message.
    /// Only writes the event if the BIGDRIVE_EVENT_ERROR event is enabled in the ETW session.
    /// </summary>
    /// <param name="message">The message to log. Must be a null-terminated wide string.</param>
    static void LogError(LPCWSTR message)
    {
        if (EventEnabledBIGDRIVE_EVENT_ERROR())
        {
            EventWriteBIGDRIVE_EVENT_ERROR(message);
        }
    }

    /// <summary>
    /// Logs a function entry event with the specified function name and line number.
    /// Only writes the event if the BIGDRIVE_EVENT_ENTER event is enabled in the ETW session.
    /// This is useful for tracing execution flow through the application.
    /// </summary>
    /// <param name="functionName">The name of the function. Must be a null-terminated ANSI string.</param>
    /// <param name="lineNumber">The line number in the source file where the function begins.</param>
    static void LogEnter(LPCSTR functionName, UINT32 lineNumber)
    {
        if (EventEnabledBIGDRIVE_EVENT_ENTER())
        {
            EventWriteBIGDRIVE_EVENT_ENTER(functionName, lineNumber);
        }
    }

    /// <summary>
    /// Logs a function exit event with the specified function name, line number, and result.
    /// Only writes the event if the BIGDRIVE_EVENT_LEAVE event is enabled in the ETW session.
    /// This is useful for tracing execution flow and return values through the application.
    /// </summary>
    /// <param name="functionName">The name of the function. Must be a null-terminated ANSI string.</param>
    /// <param name="lineNumber">The line number in the source file where the function returns.</param>
    /// <param name="hr">The HRESULT return value of the function, which will be displayed in hexadecimal.</param>
    static void LogLeave(LPCSTR functionName, UINT32 lineNumber, HRESULT hr)
    {
        if (EventEnabledBIGDRIVE_EVENT_LEAVE())
        {
            EventWriteBIGDRIVE_EVENT_LEAVE(functionName, lineNumber, hr);
        }
    }
};