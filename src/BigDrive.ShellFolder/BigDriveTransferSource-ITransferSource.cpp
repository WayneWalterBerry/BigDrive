// <copyright file="BigDriveTransferSource-ITransferSource.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveTransferSource.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"

/// <inheritdoc/>
/// <summary>
/// Stub implementations for ITransferSource methods.
/// </summary>

HRESULT __stdcall BigDriveTransferSource::Advise(
    ITransferAdviseSink* /*psink*/,
    DWORD* /*pdwCookie*/)
{
    // Not implemented
    return E_NOTIMPL;
}

HRESULT __stdcall BigDriveTransferSource::Unadvise(
    DWORD /*dwCookie*/)
{
    // Not implemented
    return E_NOTIMPL;
}

HRESULT __stdcall BigDriveTransferSource::SetProperties(
    IPropertyChangeArray* /*pproparray*/)
{
    // Not implemented
    return E_NOTIMPL;
}

HRESULT __stdcall BigDriveTransferSource::OpenItem(
    IShellItem* /*psi*/,
    TRANSFER_SOURCE_FLAGS /*flags*/,
    REFIID /*riid*/,
    void** /*ppv*/)
{
    // Not implemented
    return E_NOTIMPL;
}

HRESULT __stdcall BigDriveTransferSource::MoveItem(
    IShellItem* /*psi*/,
    IShellItem* /*psiParentDest*/,
    LPCWSTR /*pszNewName*/,
    TRANSFER_SOURCE_FLAGS /*flags*/,
    IShellItem** /*ppsiNew*/)
{
    // Not implemented
    return E_NOTIMPL;
}

HRESULT __stdcall BigDriveTransferSource::RecycleItem(
    IShellItem* /*psi*/,
    IShellItem* /*psiParentDest*/,
    TRANSFER_SOURCE_FLAGS /*flags*/,
    IShellItem** /*ppsiNew*/)
{
    // Not implemented
    return E_NOTIMPL;
}

HRESULT __stdcall BigDriveTransferSource::RemoveItem(
    IShellItem* /*psi*/,
    TRANSFER_SOURCE_FLAGS /*flags*/)
{
    // Not implemented
    return E_NOTIMPL;
}

HRESULT __stdcall BigDriveTransferSource::RenameItem(
    IShellItem* /*psi*/,
    LPCWSTR /*pszNewName*/,
    TRANSFER_SOURCE_FLAGS /*flags*/,
    IShellItem** /*ppsiNew*/)
{
    // Not implemented
    return E_NOTIMPL;
}

HRESULT __stdcall BigDriveTransferSource::LinkItem(
    IShellItem* /*psiSource*/,
    IShellItem* /*psiParentDest*/,
    LPCWSTR /*pszName*/,
    TRANSFER_SOURCE_FLAGS /*flags*/,
    IShellItem** /*ppsiNewDest*/)
{
    // Not implemented
    return E_NOTIMPL;
}

HRESULT __stdcall BigDriveTransferSource::ApplyPropertiesToItem(
    IShellItem* /*psiSource*/,
    IShellItem** /*ppsiNew*/)
{
    // Not implemented
    return E_NOTIMPL;
}

HRESULT __stdcall BigDriveTransferSource::GetDefaultDestinationName(
    IShellItem* /*psiSource*/,
    IShellItem* /*psiParentDest*/,
    LPWSTR* /*ppszDestinationName*/)
{
    // Not implemented
    return E_NOTIMPL;
}

HRESULT __stdcall BigDriveTransferSource::EnterFolder(
    IShellItem* /*psiChildFolderDest*/)
{
    // Not implemented
    return E_NOTIMPL;
}

HRESULT __stdcall BigDriveTransferSource::LeaveFolder(
    IShellItem* /*psiChildFolderDest*/)
{
    // Not implemented
    return E_NOTIMPL;
}