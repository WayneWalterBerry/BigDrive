# Dragging Virtual Files from BigDrive to Notepad

When a user drags virtual files from BigDrive's shell namespace extension to an application like Notepad, several steps occur behind the scenes:

## Step 1: Initial Drag Operation

1. User selects one or more virtual files in BigDrive's namespace extension view.
2. User begins dragging (mouse down + movement).
3. The shell folder's view object detects the drag operation.

## Step 2: Data Object Creation

1. Windows Shell calls `BigDriveShellFolder::GetUIObjectOf()` with `IID_IDataObject` as the `riid` parameter.
2. The implementation creates and returns an `IDataObject` containing representations of the dragged items.

## Step 3: Format Preparation

BigDrive's `IDataObject` offers data in formats Notepad can understand:


``` C#
// Main formats for file transfers
CF_HDROP                    // Traditional file paths
CFSTR_FILEDESCRIPTOR        // File metadata (name, size, etc.)
CFSTR_FILECONTENTS          // Actual file content
```


## Step 4: Drop Target Interaction

1. As the user drags over Notepad, Notepad's `IDropTarget::DragEnter()` and `DragOver()` are called.
2. Notepad calls BigDrive's `IDataObject::QueryGetData()` to check available formats.
3. Based on supported formats, Notepad shows visual feedback (cursor changes).

## Step 5: Content Transfer at Drop Time

When dropped on Notepad:

1. Notepad's `IDropTarget::Drop()` method is called.
2. Notepad requests data through `IDataObject::GetData()`.
3. Notepad First requests the `CF_HDROP` format to get file paths.
   - If `CF_HDROP` is available, Notepad opens the files directly from disk.
4. Notepad continues by requesting `CFSTR_FILEDESCRIPTOR / CFSTR_FILECONTENTS` (This is the virtual file mechanism)
   - If `CFSTR_FILEDESCRIPTOR` is available, Notepad can enumerate the files.
   - If `CFSTR_FILECONTENTS` is requested, it gets the actual file data for each file by index.

## Virtual File Method (CFSTR_FILEDESCRIPTOR/CFSTR_FILECONTENTS)

This approach allows BigDrive to provide virtual files directly to Notepad (and other applications) without creating temporary files.

---

### 1. Register Clipboard Formats

Register the clipboard formats at module initialization: The virtual file method is an elegant for truly virtual content, while the CF_HDROP method is simpler but requires temporary file creation.

```cpp
UINT g_cfFileDescriptor = ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTORW);
UINT g_cfFileContents   = ::RegisterClipboardFormat(CFSTR_FILECONTENTS);
```

### 2. IDataObject::GetData Implementation

`BigDriveDataObject` should support requests for both formats.

#### a. CFSTR_FILEDESCRIPTOR

When `GetData` is called with `CFSTR_FILEDESCRIPTOR`, return a `FILEGROUPDESCRIPTORW` structure describing all files being dragged.

``` cpp
if (pformatetc->cfFormat == g_cfFileDescriptor) {
    // Allocate and fill FILEGROUPDESCRIPTORW
    // For each file, fill FILEDESCRIPTORW (name, size, attributes, etc.)
    // Set pmedium->tymed = TYMED_HGLOBAL and assign the HGLOBAL
}
```

To trigger a request for `CFSTR_FILECONTENTS` after providing `CFSTR_FILEDESCRIPTOR`, BigDrive should set the cFileName field of each `FILEDESCRIPTORW`` structure to the display name of the virtual file (e.g., "MyDocument.txt"), not a full or absolute path.

##### Key points

- The string in cFileName is just the file name (with extension), not a path.
- Each file you describe in the FILEGROUPDESCRIPTORW array gets an index (0-based).
- When the drop target (e.g., Notepad) wants the file's content, it will call GetData with `CFSTR_FILECONTENTS`` and set lindex to the index of the file in your descriptor array.
- The value of cFileName is used by the drop target to name the file, but the mapping to content is by index, not by path.

#### b. CFSTR_FILECONTENTS

When `GetData` is called with `CFSTR_FILECONTENTS`, return the file data for the requested file index (see `lindex` in `FORMATETC`).

``` cpp
if (pformatetc->cfFormat == g_cfFileContents) {
    // lindex specifies which file's content is requested
    // Allocate HGLOBAL, fill with file data from your virtual source
    // Set pmedium->tymed = TYMED_HGLOBAL and assign the HGLOBAL
}
```

### 3. FORMATETC and STGMEDIUM

- Support `TYMED_HGLOBAL` for both formats.
- For `CFSTR_FILECONTENTS`, you may also support `TYMED_ISTREAM` for large files (optional).

---


## Notes

- The shell and Notepad will first request the file descriptors, then request the contents for each file by index.
- No temporary files are created; all data is provided on demand from BigDrive’s virtual storage.

---

**References:**
- [MSDN: Virtual File Data Transfer](https://learn.microsoft.com/en-us/windows/win32/shell/clipboard#virtual-file-data-transfer)
- [MSDN: CFSTR_FILEDESCRIPTOR](https://learn.microsoft.com/en-us/windows/win32/shell/clipboard#cfstr_filedescriptora-cfstr_filedescriptorw)
- [MSDN: CFSTR_FILECONTENTS](https://learn.microsoft.com/en-us/windows/win32/shell/clipboard#cfstr_filecontents)