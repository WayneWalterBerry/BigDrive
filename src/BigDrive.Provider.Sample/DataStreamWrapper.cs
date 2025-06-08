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
    public class DataStreamWrapper : IStream
    {
        private readonly Stream _stream;

        /// <summary>
        /// Initializes a new instance of the <see cref="DataStreamWrapper"/> class.
        /// </summary>
        /// <param name="stream">The stream to wrap.</param>
        public DataStreamWrapper(Stream stream)
        {
            _stream = stream ?? throw new ArgumentNullException(nameof(stream));
        }

        public void Read(byte[] pv, int cb, IntPtr pcbRead)
        {
            int bytesRead = _stream.Read(pv, 0, cb);
            if (pcbRead != IntPtr.Zero)
            {
                Marshal.WriteInt32(pcbRead, bytesRead);
            }
        }

        public void Write(byte[] pv, int cb, IntPtr pcbWritten)
        {
            _stream.Write(pv, 0, cb);
            if (pcbWritten != IntPtr.Zero)
            {
                Marshal.WriteInt32(pcbWritten, cb);
            }
        }

        public void Seek(long dlibMove, int dwOrigin, IntPtr plibNewPosition)
        {
            long pos = _stream.Seek(dlibMove, (SeekOrigin)dwOrigin);
            if (plibNewPosition != IntPtr.Zero)
            {
                Marshal.WriteInt64(plibNewPosition, pos);
            }
        }

        public void SetSize(long libNewSize)
        {
            _stream.SetLength(libNewSize);
        }

        public void CopyTo(IStream pstm, long cb, IntPtr pcbRead, IntPtr pcbWritten)
        {
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