// <copyright file="LaunchDebugger.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include <windows.h>
#include <wchar.h>

bool LaunchDebugger()
{
    // Get System directory (typically C:\Windows\System32)
    wchar_t systemDir[MAX_PATH];
    if (::GetSystemDirectoryW(systemDir, MAX_PATH) == 0) 
    {
        return false;
    }

    // Get process ID and create the command line
    DWORD pid = ::GetCurrentProcessId();
    wchar_t cmdLine[MAX_PATH * 2];
    int count = ::swprintf_s(cmdLine, L"%s\\vsjitdebugger.exe -p %lu", systemDir, static_cast<unsigned long>(pid));
    if (count < 0) {
        return false;
    }

    // Start debugger process
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (!::CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) 
    {
        return false;
    }

    // Close debugger process handles to prevent resource leaks
    ::CloseHandle(pi.hThread);
    ::CloseHandle(pi.hProcess);

    // Wait for the debugger to attach
    while (!::IsDebuggerPresent()) 
    {
        Sleep(100);
    }

    // Trigger a breakpoint so the debugger takes over
    DebugBreak();

    return true;
}
