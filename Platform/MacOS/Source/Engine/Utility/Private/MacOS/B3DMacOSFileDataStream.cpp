//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/MacOS/B3DMacOSFileDataStream.h"

#include "FileSystem/B3DDataStream.h"
#include "Debug/B3DDebug.h"

#include <cstring>
#include <errno.h>

using namespace b3d;

struct MacOSFileDataStream::AsyncReadRequest
{
	void* Buffer = nullptr;
	bool OwnsBuffer = false;
	u64 BytesRead = 0; /**< Bytes accumulated across all dispatch_io handler invocations. */
	TAsyncOp<TShared<MemoryDataStream>> Op;
};

MacOSFileDataStream::MacOSFileDataStream(const Path& path, FileAccessFlags access)
	: UnixFileDataStream(path, access)
{ }

MacOSFileDataStream::~MacOSFileDataStream()
{
	MacOSFileDataStream::Close();
}

bool MacOSFileDataStream::Open()
{
	if(!UnixFileDataStream::Open())
		return false;

	if(!mAccess.IsSet(FileAccessFlag::Async))
		return true;

	mChannel = dispatch_io_create(DISPATCH_IO_RANDOM, mFd, dispatch_get_global_queue(QOS_CLASS_UTILITY, 0), ^(int /*error*/){});
	if(mChannel == nullptr)
	{
		B3D_LOG(Warning, LogFileSystem, "Failed to create a dispatch_io channel for asynchronous reads of file '{0}'. Falling back to synchronous reads.", mPath);
		return true;
	}

	mAsyncReady = true;
	return true;
}

TAsyncOp<TShared<MemoryDataStream>> MacOSFileDataStream::ReadAsync(u64 offset, size_t byteCount, TOptional<DataRange> userSuppliedMemory)
{
	// No async backend (either not requested or initialization failed): use the synchronous default.
	if(!mAsyncReady)
		return UnixFileDataStream::ReadAsync(offset, byteCount, userSuppliedMemory);

	TAsyncOp<TShared<MemoryDataStream>> op;

	if(byteCount == 0)
	{
		op.CompleteOperation(B3DMakeShared<MemoryDataStream>());
		return op;
	}

	const u64 available = offset < mSize ? (mSize - offset) : 0;
	const size_t length = byteCount > available ? (size_t)available : byteCount;

	if(length == 0)
	{
		op.CompleteOperation(B3DMakeShared<MemoryDataStream>());
		return op;
	}

	{
		Lock lock(mAsyncMutex);
		if(mClosed || mFd < 0)
		{
			op.CompleteOperation(nullptr);
			return op;
		}

		mOutstandingReads++;
	}

	AsyncReadRequest* request = B3DNew<AsyncReadRequest>();
	request->Op = op;

	if(userSuppliedMemory.has_value())
	{
		request->Buffer = userSuppliedMemory->Data;
		request->OwnsBuffer = false;
	}
	else
	{
		request->Buffer = B3DAllocate(length);
		request->OwnsBuffer = true;

		if(request->Buffer == nullptr)
		{
			B3D_LOG(Error, LogFileSystem, "Failed to allocate {0} bytes for an asynchronous read.", (u64)length);
			FinalizeAsyncRead(request, 0, false);
			return op;
		}
	}

	dispatch_io_read(mChannel, (off_t)offset, length, dispatch_get_global_queue(QOS_CLASS_UTILITY, 0), ^(bool done, dispatch_data_t data, int error)
	{
		if(data != nullptr && dispatch_data_get_size(data) > 0)
		{
			dispatch_data_apply(data, ^bool(dispatch_data_t /*region*/, size_t /*regionOffset*/, const void* buffer, size_t size)
			{
				std::memcpy((u8*)request->Buffer + request->BytesRead, buffer, size);
				request->BytesRead += size;
				return true;
			});
		}

		if(done)
		{
			// ECANCELED arrives here when Close() issued dispatch_io_close(..., DISPATCH_IO_STOP) on an in-flight read
			// (e.g. an unconsumed speculative read-ahead); it's the standard cancellation path and finalizes as a
			// silent failure (mirrors Win32 ERROR_OPERATION_ABORTED).
			const bool succeeded = (error == 0);
			if(!succeeded && error != ECANCELED)
				B3D_LOG(Warning, LogFileSystem, "Asynchronous read failed (error {0}).", String(strerror(error)));

			FinalizeAsyncRead(request, (size_t)request->BytesRead, succeeded);
		}
	});

	return op;
}

void MacOSFileDataStream::FinalizeAsyncRead(AsyncReadRequest* request, size_t bytesRead, bool succeeded)
{
	TShared<MemoryDataStream> result;
	if(succeeded)
	{
		if(request->OwnsBuffer)
			result = B3DMakeShared<MemoryDataStream>(request->Buffer, bytesRead, true);
		else
			result = B3DMakeShared<MemoryDataStream>(request->Buffer, bytesRead);
	}
	else if(request->OwnsBuffer && request->Buffer != nullptr)
	{
		B3DFree(request->Buffer);
	}

	request->Op.CompleteOperation(result);
	B3DDelete(request);

	Lock lock(mAsyncMutex);
	B3D_ASSERT(mOutstandingReads > 0);
	mOutstandingReads--;

	if(mOutstandingReads == 0)
		mAllReadsComplete.notify_all();
}

TShared<DataStream> MacOSFileDataStream::Clone(bool /*copyData*/) const
{
	TShared<MacOSFileDataStream> clone = B3DMakeShared<MacOSFileDataStream>(mPath, mAccess);

	// A writable clone stays closed, as opening it would truncate the file this stream is still writing
	if(!mAccess.IsSet(FileAccessFlag::Write))
		clone->Open();

	return clone;
}

bool MacOSFileDataStream::Close()
{
	{
		Lock lock(mAsyncMutex);
		if(mClosed)
			return UnixFileDataStream::Close();

		mClosed = true;
	}

	if(mAsyncReady)
	{
		// DISPATCH_IO_STOP causes in-flight handlers to fire shortly with ECANCELED, after which we can safely drain.
		// Without this, the wait below would block forever for outstanding reads on a large file.
		dispatch_io_close(mChannel, DISPATCH_IO_STOP);

		{
			Lock lock(mAsyncMutex);
			mAllReadsComplete.wait(lock, [this]() { return mOutstandingReads == 0; });
		}

		dispatch_release(mChannel);
		mChannel = nullptr;
		mAsyncReady = false;
	}

	return UnixFileDataStream::Close();
}
