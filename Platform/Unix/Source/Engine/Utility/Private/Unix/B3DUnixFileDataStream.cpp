//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/Unix/B3DUnixFileDataStream.h"

#include "Debug/B3DDebug.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

using namespace b3d;

namespace
{
	// Linux read()/write() (and the related positioned variants) transfer at most 0x7ffff000 bytes per call (man 2 read).
	// Cap individual pread()/pwrite() transfers to that limit so the per-call count is always valid; larger requests are
	// issued as a loop of chunks.
	constexpr u64 kMaxTransferPerCall = 0x7ffff000ull;

	// Files this stream opens can exceed 2 GiB; pread/pwrite offsets are off_t, so a 32-bit off_t would silently truncate
	// mCursor values past 2 GiB. Framework runs only on 64-bit POSIX targets in practice, but make the assumption explicit
	// here so a future 32-bit build immediately surfaces the issue rather than corrupting reads at runtime.
	static_assert(sizeof(off_t) >= sizeof(u64), "off_t must be 64-bit; define _FILE_OFFSET_BITS=64 on 32-bit POSIX builds");
}

UnixFileDataStream::UnixFileDataStream(const Path& path, FileAccessFlags access)
	: mPath(path), mAccess(access)
{ }

UnixFileDataStream::~UnixFileDataStream()
{
	UnixFileDataStream::Close();
}

bool UnixFileDataStream::Open()
{
	// Open() must be called on a stream that isn't already open; re-opening would overwrite (and leak) the live descriptor.
	if(!B3D_ENSURE(mFd < 0))
		return false;

	const bool wantRead = mAccess.IsSet(FileAccessFlag::Read);
	const bool wantWrite = mAccess.IsSet(FileAccessFlag::Write);

	int flags;
	if(wantRead && wantWrite)
		flags = O_RDWR | O_CLOEXEC;
	else if(wantWrite)
		// Write-only mirrors CreateAndOpenFile: create the file (truncating any existing one). Otherwise open an existing file.
		flags = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC;
	else
		flags = O_RDONLY | O_CLOEXEC;

	const String pathStr = mPath.ToString();
	const int fd = ::open(pathStr.c_str(), flags, 0644);
	if(fd < 0)
	{
		B3D_LOG(Error, LogFileSystem, "Failed to open file '{0}' (error {1}).", mPath, String(strerror(errno)));
		return false;
	}

	// Strict sharing: read-only handles permit other readers (FILE_SHARE_READ); any write-capable handle is exclusive
	// (dwShareMode = 0). Shared permits sharing.
	// Note: flock() is advisory and does not interoperate with fcntl() byte-range locks or all NFS setups; local-disk targets only.
	if(!mAccess.IsSet(FileAccessFlag::Shared))
	{
		const int lockOp = wantWrite ? (LOCK_EX | LOCK_NB) : (LOCK_SH | LOCK_NB);
		if(::flock(fd, lockOp) != 0)
		{
			B3D_LOG(Error, LogFileSystem, "Failed to acquire {0} lock on file '{1}' (error {2}).",
					wantWrite ? "exclusive" : "shared", mPath, String(strerror(errno)));
			::close(fd);
			return false;
		}
	}

	mFd = fd;

	struct stat st{};
	if(::fstat(fd, &st) == 0)
		mSize = (size_t)st.st_size;
	else
		mSize = 0;

	mCursor = 0;
	mEof = false;

	return true;
}

size_t UnixFileDataStream::Read(void* outData, size_t byteCount) const
{
	if(!B3D_ENSURE(mFd >= 0))
		return 0;

	u8* out = static_cast<u8*>(outData);
	size_t totalBytesRead = 0;
	while(totalBytesRead < byteCount)
	{
		const u64 remaining = byteCount - totalBytesRead;
		const size_t bytesToRead = remaining > kMaxTransferPerCall ? (size_t)kMaxTransferPerCall : (size_t)remaining;

		const off_t offset = (off_t)(mCursor + totalBytesRead);
		const ssize_t bytesRead = ::pread(mFd, out + totalBytesRead, bytesToRead, offset);
		if(bytesRead < 0)
		{
			if(errno == EINTR) // Interrupted by signal before transferring any data; safe to retry.
				continue;

			B3D_LOG(Error, LogFileSystem, "Error while reading from file '{0}' (error {1}).", mPath, String(strerror(errno)));
			break;
		}

		if(bytesRead == 0) // End of file reached.
			break;

		totalBytesRead += (size_t)bytesRead;
	}

	mCursor += totalBytesRead;
	if(totalBytesRead < byteCount)
		mEof = true;

	return totalBytesRead;
}

size_t UnixFileDataStream::Write(const void* data, size_t byteCount)
{
	if(!B3D_ENSURE(mFd >= 0))
		return 0;

	if(!B3D_ENSURE(IsWriteable()))
		return 0;

	const u8* in = static_cast<const u8*>(data);
	size_t totalBytesWritten = 0;
	while(totalBytesWritten < byteCount)
	{
		const u64 remaining = byteCount - totalBytesWritten;
		const size_t bytesToWrite = remaining > kMaxTransferPerCall ? (size_t)kMaxTransferPerCall : (size_t)remaining;

		const off_t offset = (off_t)(mCursor + totalBytesWritten);
		const ssize_t bytesWritten = ::pwrite(mFd, in + totalBytesWritten, bytesToWrite, offset);
		if(bytesWritten < 0)
		{
			if(errno == EINTR)
				continue;

			B3D_LOG(Error, LogFileSystem, "Error while writing to file '{0}' (error {1}).", mPath, String(strerror(errno)));
			break;
		}

		if(bytesWritten == 0)
			break;

		totalBytesWritten += (size_t)bytesWritten;
	}

	mCursor += totalBytesWritten;
	if(mCursor > mSize)
		mSize = (size_t)mCursor;

	return totalBytesWritten;
}

size_t UnixFileDataStream::Skip(size_t count)
{
	if(!B3D_ENSURE(mFd >= 0))
		return 0;

	// pread/pwrite are positioned, so the cursor is tracked purely in software.
	mCursor += count;
	mEof = false;
	return count;
}

size_t UnixFileDataStream::Seek(size_t pos)
{
	if(!B3D_ENSURE(mFd >= 0))
		return (size_t)mCursor;

	// pread/pwrite are positioned, so the cursor is tracked purely in software.
	mCursor = (u64)pos;
	mEof = false;
	return (size_t)mCursor;
}

size_t UnixFileDataStream::Tell() const
{
	return (size_t)mCursor;
}

bool UnixFileDataStream::Eof() const
{
	return mEof;
}

TShared<DataStream> UnixFileDataStream::Clone(bool /*copyData*/) const
{
	return B3DMakeShared<UnixFileDataStream>(mPath, mAccess);
}

bool UnixFileDataStream::Flush()
{
	if(!B3D_ENSURE(mFd >= 0))
		return false;

	if(mAccess.IsSet(FileAccessFlag::Write))
	{
		if(::fsync(mFd) != 0)
		{
			B3D_LOG(Error, LogFileSystem, "Error while flushing file '{0}' (error {1}).", mPath, String(strerror(errno)));
			return false;
		}
	}

	return true;
}

bool UnixFileDataStream::Close()
{
	bool flushResult = true;
	if(mFd >= 0)
	{
		if(mAccess.IsSet(FileAccessFlag::Write))
		{
			if(::fsync(mFd) != 0)
				flushResult = false;
		}

		::close(mFd);
		mFd = -1;
	}

	// Reset cursor/EOF state so a subsequent Open() on the same stream object doesn't start in EOF from the previous lifetime.
	mCursor = 0;
	mEof = false;

	return flushResult;
}
