// <copyright file="ComStream.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Iso
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
        private readonly Stream m_stream;

        private bool m_disposed = false;

        /// <summary>
        /// Initializes a new instance of the <see cref="ComStream"/> class.
        /// </summary>
        /// <param name="stream">The stream to wrap.</param>
        public ComStream(Stream stream)
        {
            m_stream = stream ?? throw new ArgumentNullException(nameof(stream));
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
            if (!m_disposed)
            {
                if (disposing)
                {
                    m_stream.Dispose();
                }

                m_disposed = true;
            }
        }

        /// <summary>
        /// Finalizer to ensure resources are cleaned up if not disposed properly.
        /// </summary>
        ~ComStream()
        {
            Dispose(false);
        }

        /// <summary>
        /// Throws an <see cref="ObjectDisposedException"/> if this instance has been disposed.
        /// </summary>
        private void ThrowIfDisposed()
        {
            if (m_disposed)
            {
                throw new ObjectDisposedException(nameof(ComStream));
            }
        }

        /// <inheritdoc/>
        public void Read(byte[] pv, int cb, IntPtr pcbRead)
        {
            ThrowIfDisposed();

            int bytesRead = m_stream.Read(pv, 0, cb);
            if (pcbRead != IntPtr.Zero)
            {
                Marshal.WriteInt32(pcbRead, bytesRead);
            }
        }

        /// <inheritdoc/>
        public void Write(byte[] pv, int cb, IntPtr pcbWritten)
        {
            ThrowIfDisposed();

            m_stream.Write(pv, 0, cb);
            if (pcbWritten != IntPtr.Zero)
            {
                Marshal.WriteInt32(pcbWritten, cb);
            }
        }

        /// <inheritdoc/>
        public void Seek(long dlibMove, int dwOrigin, IntPtr plibNewPosition)
        {
            ThrowIfDisposed();

            long pos = m_stream.Seek(dlibMove, (SeekOrigin)dwOrigin);
            if (plibNewPosition != IntPtr.Zero)
            {
                Marshal.WriteInt64(plibNewPosition, pos);
            }
        }

        /// <inheritdoc/>
        public void SetSize(long libNewSize)
        {
            ThrowIfDisposed();

            m_stream.SetLength(libNewSize);
        }

        /// <inheritdoc/>
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
                int bytesRead = m_stream.Read(buffer, 0, toRead);
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

        /// <inheritdoc/>
        public void Commit(int grfCommitFlags)
        {
            ThrowIfDisposed();

            m_stream.Flush();
        }

        /// <inheritdoc/>
        public void Revert()
        {
            throw new NotSupportedException();
        }

        /// <inheritdoc/>
        public void LockRegion(long libOffset, long cb, int dwLockType)
        {
            throw new NotSupportedException();
        }

        /// <inheritdoc/>
        public void UnlockRegion(long libOffset, long cb, int dwLockType)
        {
            throw new NotSupportedException();
        }

        /// <inheritdoc/>
        public void Stat(out ComStatStg pstatstg, int grfStatFlag)
        {
            ThrowIfDisposed();

            pstatstg = new ComStatStg
            {
                cbSize = m_stream.CanSeek ? m_stream.Length : 0,
                type = 2 // STGTY_STREAM
            };
        }

        /// <inheritdoc/>
        public void Clone(out IStream ppstm)
        {
            ppstm = null;
            throw new NotSupportedException();
        }
    }
}
