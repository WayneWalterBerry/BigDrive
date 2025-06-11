// <copyright file="DataStreamWrapper.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    using ComStatStg = System.Runtime.InteropServices.ComTypes.STATSTG;

    /// <summary>
    /// Wraps a .NET Stream as a COM IStream.
    /// </summary>
    public class ComStream : IStream, IDisposable
    {
        private readonly Stream _stream;
        private bool _disposed = false;

        /// <summary>
        /// Initializes a new instance of the <see cref="ComStream"/> class.
        /// </summary>
        /// <param name="stream">The stream to wrap.</param>
        public ComStream(Stream stream)
        {
            _stream = stream ?? throw new ArgumentNullException(nameof(stream));
        }

        /// <summary>
        /// Releases all resources used by the <see cref="ComStream"/>.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases the unmanaged resources used by the <see cref="ComStream"/> and optionally releases the managed resources.
        /// </summary>
        /// <param name="disposing">true to release both managed and unmanaged resources; false to release only unmanaged resources.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (disposing)
                {
                    // Dispose managed resources
                    _stream.Dispose();
                }

                // Set large fields to null to help the garbage collector
                _disposed = true;
            }
        }

        /// <summary>
        /// Finalizer to ensure resources are cleaned up if not disposed properly.
        /// </summary>
        ~ComStream()
        {
            Dispose(false);
        }

        // Add this check to methods that access the stream
        private void ThrowIfDisposed()
        {
            if (_disposed)
            {
                throw new ObjectDisposedException(nameof(ComStream));
            }
        }

        public void Read(byte[] pv, int cb, IntPtr pcbRead)
        {
            ThrowIfDisposed();

            int bytesRead = _stream.Read(pv, 0, cb);
            if (pcbRead != IntPtr.Zero)
            {
                Marshal.WriteInt32(pcbRead, bytesRead);
            }
        }

        public void Write(byte[] pv, int cb, IntPtr pcbWritten)
        {
            ThrowIfDisposed();

            _stream.Write(pv, 0, cb);
            if (pcbWritten != IntPtr.Zero)
            {
                Marshal.WriteInt32(pcbWritten, cb);
            }
        }

        public void Seek(long dlibMove, int dwOrigin, IntPtr plibNewPosition)
        {
            ThrowIfDisposed();

            long pos = _stream.Seek(dlibMove, (SeekOrigin)dwOrigin);
            if (plibNewPosition != IntPtr.Zero)
            {
                Marshal.WriteInt64(plibNewPosition, pos);
            }
        }

        public void SetSize(long libNewSize)
        {
            ThrowIfDisposed();

            _stream.SetLength(libNewSize);
        }

        public void CopyTo(IStream pstm, long cb, IntPtr pcbRead, IntPtr pcbWritten)
        {
            ThrowIfDisposed();

            const int bufferSize = 4096;
            byte[] buffer = new byte[bufferSize];
            long remaining = cb;
            long totalRead = 0;
            long totalWritten = 0;

            while (remaining > 0)
            {
                int toRead = (int)Math.Min(bufferSize, remaining);
                int bytesRead = _stream.Read(buffer, 0, toRead);
                if (bytesRead == 0)
                {
                    break;
                }
                totalRead += bytesRead;
                remaining -= bytesRead;

                pstm.Write(buffer, bytesRead, IntPtr.Zero);
                totalWritten += bytesRead;
            }

            if (pcbRead != IntPtr.Zero)
            {
                Marshal.WriteInt64(pcbRead, totalRead);
            }
            if (pcbWritten != IntPtr.Zero)
            {
                Marshal.WriteInt64(pcbWritten, totalWritten);
            }
        }

        public void Commit(int grfCommitFlags)
        {
            ThrowIfDisposed();

            _stream.Flush();
        }

        public void Revert()
        {
            throw new NotSupportedException();
        }

        public void LockRegion(long libOffset, long cb, int dwLockType)
        {
            throw new NotSupportedException();
        }

        public void UnlockRegion(long libOffset, long cb, int dwLockType)
        {
            throw new NotSupportedException();
        }

        public void Stat(out ComStatStg pstatstg, int grfStatFlag)
        {
            ThrowIfDisposed();

            pstatstg = new ComStatStg
            {
                cbSize = _stream.CanSeek ? _stream.Length : 0,
                type = 2 // STGTY_STREAM
            };
        }

        public void Clone(out IStream ppstm)
        {
            ppstm = null;
            throw new NotSupportedException();
        }
    }
}