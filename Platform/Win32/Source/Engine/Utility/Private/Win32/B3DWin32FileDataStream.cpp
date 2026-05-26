//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/Win32/B3DWin32FileDataStream.h"

#include "FileSystem/B3DDataStream.h"
#include "Debug/B3DDebug.h"
#include "String/B3DUnicode.h"
#include <windows.h>

using namespace b3d;

namespace
{
	// Maximum number of bytes to transfer in a single ReadFile/WriteFile call (DWORD-sized count).
	constexpr u64 kMaxTransferPerCall = 0xFFFF0000ull;
}

struct Win32FileDataStream::AsyncChunkReadRequest
{
	OVERLAPPED Overlapped; /**< Must be the first member so the OVERLAPPED can be recovered via CONTAINING_RECORD. */
	Win32FileDataStream* Stream = nullptr;
	void* Buffer = nullptr;
	bool OwnsBuffer = false;
	u64 BaseOffset = 0; /**< File offset where the read began. */
	u64 TotalByteCount = 0; /**< Total number of bytes to read (clamped to the end of file). */
	u64 BytesRead = 0; /**< Bytes read so far across all chunks. */
	HANDLE Event = nullptr; /**< Manual-reset completion event; also stored in Overlapped.hEvent. */
	HANDLE WaitHandle = nullptr; /**< Threadpool wait registration for the current chunk. */
	TAsyncOp<TShared<MemoryDataStream>> Op;
};

// ************************************************************************************************************************
// Win32FileDataStream
// ************************************************************************************************************************

Win32FileDataStream::Win32FileDataStream(const Path& path, FileAccessFlags access)
	: mPath(path), mAccess(access)
{ }

Win32FileDataStream::~Win32FileDataStream()
{
	Close();
}

bool Win32FileDataStream::Open()
{
	const bool wantRead = mAccess.IsSet(FileAccessFlag::Read);
	const bool wantWrite = mAccess.IsSet(FileAccessFlag::Write);
	mIsOverlapped = mAccess.IsSet(FileAccessFlag::Async);

	DWORD access = 0;
	if(wantRead)
		access |= GENERIC_READ;

	if(wantWrite)
		access |= GENERIC_WRITE;

	// Write-only mirrors CreateAndOpenFile: create the file (truncating any existing one). Otherwise open an existing file.
	const DWORD disposition = (wantWrite && !wantRead) ? CREATE_ALWAYS : OPEN_EXISTING;

	// Streams opened with the ASYNC flag use overlapped IO so ReadAsync() can issue true asynchronous reads. The same
	// overlapped handle also serves synchronous Read()/Write() (which issue an overlapped operation and wait for it).
	DWORD flags = FILE_ATTRIBUTE_NORMAL;
	if(mIsOverlapped)
		flags |= FILE_FLAG_OVERLAPPED;

	// Match std::fstream's sharing mode (MSVC opens with _SH_DENYNO == FILE_SHARE_READ | FILE_SHARE_WRITE). Sharing
	// writes is required because the engine holds the same file open for writing while opening it again for reading.
	// Without FILE_SHARE_WRITE the second open fails with ERROR_SHARING_VIOLATION even though the file exists.
	const WString widePath = UTF8::ToWide(mPath.ToString());
	HANDLE handle = CreateFileW(widePath.c_str(), access, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, disposition, flags, nullptr);
	if(handle == INVALID_HANDLE_VALUE)
	{
		B3D_LOG(Error, LogFileSystem, "Failed to open file '{0}' (error {1}).", mPath, (u32)GetLastError());
		return false;
	}

	mHandle = handle;

	if(mIsOverlapped)
	{
		// Manual-reset event used by the synchronous Read()/Write() path to wait on overlapped completions. Async reads
		// use their own per-operation events, so this can't be confused with them.
		mSyncEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		if(mSyncEvent == nullptr)
		{
			B3D_LOG(Error, LogFileSystem, "Failed to create a synchronization event for file '{0}' (error {1}).", mPath, (u32)GetLastError());
			CloseHandle(handle);
			mHandle = nullptr;
			return false;
		}
	}

	LARGE_INTEGER size{};
	if(GetFileSizeEx(handle, &size))
		mSize = (size_t)size.QuadPart;
	else
		mSize = 0;

	mCursor = 0;
	mEof = false;

	return true;
}

size_t Win32FileDataStream::Read(void* outData, size_t byteCount) const
{
	if(!B3D_ENSURE(mHandle != nullptr))
		return 0;

	u8* out = static_cast<u8*>(outData);
	size_t totalBytesRead = 0;
	while(totalBytesRead < byteCount)
	{
		const u64 remaining = byteCount - totalBytesRead;
		const DWORD bytesToRead = remaining > kMaxTransferPerCall ? (DWORD)kMaxTransferPerCall : (DWORD)remaining;

		DWORD bytesRead = 0;
		if(mIsOverlapped)
		{
			// Overlapped handles ignore the file pointer, so the read position is supplied via the OVERLAPPED and the
			// completion is waited on synchronously. The per-stream event makes the wait unambiguous even if async reads
			// are concurrently in flight on the same handle.
			const u64 fileOffset = mCursor + totalBytesRead;

			OVERLAPPED ov{};
			ov.Offset = (DWORD)(fileOffset & 0xFFFFFFFFull);
			ov.OffsetHigh = (DWORD)(fileOffset >> 32);
			ov.hEvent = (HANDLE)mSyncEvent;
			ResetEvent((HANDLE)mSyncEvent);

			if(!ReadFile((HANDLE)mHandle, out + totalBytesRead, bytesToRead, &bytesRead, &ov))
			{
				const DWORD error = GetLastError();
				if(error == ERROR_IO_PENDING)
				{
					if(!GetOverlappedResult((HANDLE)mHandle, &ov, &bytesRead, TRUE))
					{
						const DWORD waitError = GetLastError();
						if(waitError == ERROR_HANDLE_EOF)
							bytesRead = 0;
						else
						{
							B3D_LOG(Error, LogFileSystem, "Error while reading from file '{0}' (error {1}).", mPath, (u32)waitError);
							break;
						}
					}
				}
				else if(error == ERROR_HANDLE_EOF)
				{
					bytesRead = 0;
				}
				else
				{
					B3D_LOG(Error, LogFileSystem, "Error while reading from file '{0}' (error {1}).", mPath, (u32)error);
					break;
				}
			}
		}
		else
		{
			if(!ReadFile((HANDLE)mHandle, out + totalBytesRead, bytesToRead, &bytesRead, nullptr))
			{
				B3D_LOG(Error, LogFileSystem, "Error while reading from file '{0}' (error {1}).", mPath, (u32)GetLastError());
				break;
			}
		}

		if(bytesRead == 0) // End of file reached.
			break;

		totalBytesRead += bytesRead;
	}

	mCursor += totalBytesRead;
	if(totalBytesRead < byteCount)
		mEof = true;

	return totalBytesRead;
}

TAsyncOp<TShared<MemoryDataStream>> Win32FileDataStream::ReadAsync(u64 offset, size_t byteCount, TOptional<DataRange> userSuppliedMemory)
{
	// Non-overlapped handles can't issue native asynchronous reads, so use the synchronous default implementation.
	if(!mIsOverlapped)
		return DataStream::ReadAsync(offset, byteCount, userSuppliedMemory);

	TAsyncOp<TShared<MemoryDataStream>> op;

	if(byteCount == 0)
	{
		op.CompleteOperation(B3DMakeShared<MemoryDataStream>());
		return op;
	}

	{
		Lock lock(mMutex);
		if(mClosed || mHandle == nullptr)
		{
			op.CompleteOperation(nullptr);
			return op;
		}

		mOutstandingReads++;
	}

	AsyncChunkReadRequest* request = B3DNew<AsyncChunkReadRequest>();
	request->Stream = this;
	request->BaseOffset = offset;
	request->Op = op;
	request->Event = CreateEventW(nullptr, TRUE, FALSE, nullptr); // Manual-reset, initially nonsignaled.

	if(request->Event == nullptr)
	{
		B3D_LOG(Error, LogFileSystem, "Failed to create an event for an asynchronous read (error {0}).", (u32)GetLastError());
		FinalizeAsyncRead(request, 0, false);
		return op;
	}

	if(userSuppliedMemory.has_value())
	{
		request->Buffer = userSuppliedMemory->Data;
		request->OwnsBuffer = false;
	}
	else
	{
		request->Buffer = B3DAllocate(byteCount);
		request->OwnsBuffer = true;

		if(request->Buffer == nullptr)
		{
			B3D_LOG(Error, LogFileSystem, "Failed to allocate {0} bytes for an asynchronous read.", (u64)byteCount);
			FinalizeAsyncRead(request, 0, false);
			return op;
		}
	}

	// Clamp the read to the bytes actually available so ReadFile never crosses the end of file. This keeps the
	// completion behaviour unambiguous and matches the synchronous default. Reads entirely at/after EOF complete inline.
	const u64 available = offset < mSize ? (mSize - offset) : 0;
	request->TotalByteCount = byteCount > available ? available : byteCount;

	if(request->TotalByteCount == 0)
	{
		FinalizeAsyncRead(request, 0, true);
		return op;
	}

	IssueAsyncChunkRead(request);
	return op;
}

bool Win32FileDataStream::IssueAsyncChunkRead(AsyncChunkReadRequest* request)
{
	// A single ReadFile transfers at most a DWORD's worth of bytes, so reads larger than that are split into chunks and
	// chained from the completion callback. This keeps the per-call count valid while supporting reads beyond 4 GiB.
	const u64 fileOffset = request->BaseOffset + request->BytesRead;
	const u64 remaining = request->TotalByteCount - request->BytesRead;
	const DWORD bytesToRead = remaining > kMaxTransferPerCall ? (DWORD)kMaxTransferPerCall : (DWORD)remaining;

	ZeroMemory(&request->Overlapped, sizeof(OVERLAPPED));
	request->Overlapped.Offset = (DWORD)(fileOffset & 0xFFFFFFFFull);
	request->Overlapped.OffsetHigh = (DWORD)(fileOffset >> 32);
	request->Overlapped.hEvent = request->Event;
	ResetEvent(request->Event);

	u8* const out = static_cast<u8*>(request->Buffer) + request->BytesRead;

	// Register the wait on the completion event before issuing the read so the completion can't be missed.
	// The wait fires once (WT_EXECUTEONLYONCE) when the event is signaled, and is re-registered for each chunk.
	if(!RegisterWaitForSingleObject(&request->WaitHandle, request->Event, &OnAsyncChunkReadComplete, request, INFINITE, WT_EXECUTEONLYONCE))
	{
		B3D_LOG(Warning, LogFileSystem, "Failed to register an IO wait for an asynchronous read (error {0}).", (u32)GetLastError());

		request->WaitHandle = nullptr;
		FinalizeAsyncRead(request, (size_t)request->BytesRead, false);

		return false;
	}

	const BOOL ok = ReadFile((HANDLE)mHandle, out, bytesToRead, nullptr, &request->Overlapped);
	if(!ok)
	{
		const DWORD error = GetLastError();
		if(error != ERROR_IO_PENDING)
		{
			// The read failed synchronously; the event won't be signaled, so the registered wait would never fire.
			// Unregister it (blocking - we're not in the callback) and finalize. ERROR_HANDLE_EOF shouldn't happen
			// (reads are clamped) but is treated as a clean end of data.
			UnregisterWaitEx(request->WaitHandle, INVALID_HANDLE_VALUE);
			request->WaitHandle = nullptr;

			if(error == ERROR_HANDLE_EOF)
			{
				FinalizeAsyncRead(request, (size_t)request->BytesRead, true);
			}
			else
			{
				B3D_LOG(Warning, LogFileSystem, "Asynchronous read failed (error {0}).", (u32)error);
				FinalizeAsyncRead(request, (size_t)request->BytesRead, false);
			}

			return false;
		}
	}

	// On both ERROR_IO_PENDING and synchronous success the event is signaled on completion, so the registered wait fires.
	return true;
}

void __stdcall Win32FileDataStream::OnAsyncChunkReadComplete(void* parameter, unsigned char /*timedOut*/)
{
	AsyncChunkReadRequest* request = static_cast<AsyncChunkReadRequest*>(parameter);

	// One-shot wait: unregister it (non-blocking - we're inside its own callback, so a blocking unregister would
	// deadlock) so the registration is released and the next chunk can register a fresh wait.
	UnregisterWaitEx(request->WaitHandle, nullptr);
	request->WaitHandle = nullptr;

	// The outstanding-read counter keeps the handle alive until all async reads finish, so it's valid here even if
	// Close() cancelled this read (in which case GetOverlappedResult reports ERROR_OPERATION_ABORTED).
	DWORD bytesTransferred = 0;
	DWORD errorCode = ERROR_SUCCESS;
	if(!GetOverlappedResult((HANDLE)request->Stream->mHandle, &request->Overlapped, &bytesTransferred, FALSE))
		errorCode = GetLastError();

	request->Stream->FinalizeAsyncChunkRead(request, errorCode, (size_t)bytesTransferred);
}

void Win32FileDataStream::FinalizeAsyncChunkRead(AsyncChunkReadRequest* request, unsigned long errorCode, size_t bytesTransferred)
{
	// Reads are clamped to the file size, so ERROR_HANDLE_EOF isn't expected, but treat it (and a zero-byte transfer)
	// as a clean end of data. Any other error - including ERROR_OPERATION_ABORTED from Close() cancelling in-flight
	// reads - finishes the operation as a failure, discarding whatever was read so far.
	if(errorCode != ERROR_SUCCESS && errorCode != ERROR_HANDLE_EOF)
	{
		FinalizeAsyncRead(request, (size_t)request->BytesRead, false);
		return;
	}

	request->BytesRead += bytesTransferred;

	const bool reachedEof = (errorCode == ERROR_HANDLE_EOF) || (bytesTransferred == 0);
	const bool moreToRead = request->BytesRead < request->TotalByteCount;

	if(moreToRead && !reachedEof)
	{
		bool closing;
		{
			Lock lock(mMutex);
			closing = mClosed;
		}

		if(!closing)
		{
			IssueAsyncChunkRead(request); // Chains the next chunk
			return;
		}

		// Close() was requested between chunks: stop chaining and finish as a failure (matching an in-flight chunk that
		// gets cancelled via CancelIoEx).
		FinalizeAsyncRead(request, (size_t)request->BytesRead, false);
		return;
	}

	FinalizeAsyncRead(request, (size_t)request->BytesRead, true);
}

void Win32FileDataStream::FinalizeAsyncRead(AsyncChunkReadRequest* request, size_t bytesRead, bool succeeded)
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

	if(request->Event != nullptr)
		CloseHandle(request->Event);

	request->Op.CompleteOperation(result);
	B3DDelete(request);

	Lock lock(mMutex);
	B3D_ASSERT(mOutstandingReads > 0);
	mOutstandingReads--;

	if(mOutstandingReads == 0)
		mAllReadsComplete.notify_all();
}

size_t Win32FileDataStream::Write(const void* data, size_t byteCount)
{
	if(!B3D_ENSURE(mHandle != nullptr))
		return 0;

	if(!B3D_ENSURE(IsWriteable()))
		return 0;

	const u8* in = static_cast<const u8*>(data);
	size_t totalBytesWritten = 0;
	while(totalBytesWritten < byteCount)
	{
		const u64 remaining = byteCount - totalBytesWritten;
		const DWORD bytesToWrite = remaining > kMaxTransferPerCall ? (DWORD)kMaxTransferPerCall : (DWORD)remaining;

		DWORD bytesWritten = 0;
		if(mIsOverlapped)
		{
			const u64 fileOffset = mCursor + totalBytesWritten;

			OVERLAPPED ov{};
			ov.Offset = (DWORD)(fileOffset & 0xFFFFFFFFull);
			ov.OffsetHigh = (DWORD)(fileOffset >> 32);
			ov.hEvent = (HANDLE)mSyncEvent;
			ResetEvent((HANDLE)mSyncEvent);

			if(!WriteFile((HANDLE)mHandle, in + totalBytesWritten, bytesToWrite, &bytesWritten, &ov))
			{
				const DWORD error = GetLastError();
				if(error == ERROR_IO_PENDING)
				{
					if(!GetOverlappedResult((HANDLE)mHandle, &ov, &bytesWritten, TRUE))
					{
						B3D_LOG(Error, LogFileSystem, "Error while writing to file '{0}' (error {1}).", mPath, (u32)GetLastError());
						break;
					}
				}
				else
				{
					B3D_LOG(Error, LogFileSystem, "Error while writing to file '{0}' (error {1}).", mPath, (u32)error);
					break;
				}
			}
		}
		else
		{
			if(!WriteFile((HANDLE)mHandle, in + totalBytesWritten, bytesToWrite, &bytesWritten, nullptr))
			{
				B3D_LOG(Error, LogFileSystem, "Error while writing to file '{0}' (error {1}).", mPath, (u32)GetLastError());
				break;
			}
		}

		totalBytesWritten += bytesWritten;
	}

	mCursor += totalBytesWritten;
	if(mCursor > mSize)
		mSize = (size_t)mCursor;

	return totalBytesWritten;
}

size_t Win32FileDataStream::Skip(size_t count)
{
	if(!B3D_ENSURE(mHandle != nullptr))
		return 0;

	if(mIsOverlapped)
	{
		// Overlapped IO is positioned, so the cursor is tracked purely in software.
		mCursor += count;
		mEof = false;
		return count;
	}

	LARGE_INTEGER distance;
	distance.QuadPart = (LONGLONG)count;

	LARGE_INTEGER newPointer{};
	if(!SetFilePointerEx((HANDLE)mHandle, distance, &newPointer, FILE_CURRENT))
	{
		B3D_LOG(Error, LogFileSystem, "Error while seeking in file '{0}' (error {1}).", mPath, (u32)GetLastError());
		return 0;
	}

	const u64 previous = mCursor;
	mCursor = (u64)newPointer.QuadPart;
	mEof = false;

	return (size_t)(mCursor - previous);
}

size_t Win32FileDataStream::Seek(size_t pos)
{
	if(!B3D_ENSURE(mHandle != nullptr))
		return (size_t)mCursor;

	if(mIsOverlapped)
	{
		// Overlapped IO is positioned, so the cursor is tracked purely in software.
		mCursor = (u64)pos;
		mEof = false;
		return (size_t)mCursor;
	}

	LARGE_INTEGER distance;
	distance.QuadPart = (LONGLONG)pos;

	LARGE_INTEGER newPointer{};
	if(!SetFilePointerEx((HANDLE)mHandle, distance, &newPointer, FILE_BEGIN))
	{
		B3D_LOG(Error, LogFileSystem, "Error while seeking in file '{0}' (error {1}).", mPath, (u32)GetLastError());
		return (size_t)mCursor;
	}

	mCursor = (u64)newPointer.QuadPart;
	mEof = false;

	return (size_t)mCursor;
}

size_t Win32FileDataStream::Tell() const
{
	return (size_t)mCursor;
}

bool Win32FileDataStream::Eof() const
{
	return mEof;
}

TShared<DataStream> Win32FileDataStream::Clone(bool copyData) const
{
	return B3DMakeShared<Win32FileDataStream>(mPath, mAccess);
}

bool Win32FileDataStream::Flush()
{
	if(!B3D_ENSURE(mHandle != nullptr))
		return false;

	if(mAccess.IsSet(FileAccessFlag::Write))
		return FlushFileBuffers((HANDLE)mHandle) != 0;

	return true;
}

bool Win32FileDataStream::Close()
{
	{
		Lock lock(mMutex);
		if(mClosed)
			return true;

		mClosed = true;
	}

	// Cancel any in-flight asynchronous reads so their completion events fire promptly (with ERROR_OPERATION_ABORTED),
	// then wait for them to drain before releasing the handle.
	if(mHandle != nullptr && mIsOverlapped)
		CancelIoEx((HANDLE)mHandle, nullptr);

	{
		Lock lock(mMutex);
		mAllReadsComplete.wait(lock, [this]() { return mOutstandingReads == 0; });
	}

	bool flushResult = true;
	if(mHandle != nullptr)
	{
		if(mAccess.IsSet(FileAccessFlag::Write))
			flushResult = FlushFileBuffers((HANDLE)mHandle) != 0;

		CloseHandle((HANDLE)mHandle);
		mHandle = nullptr;
	}

	if(mSyncEvent != nullptr)
	{
		CloseHandle((HANDLE)mSyncEvent);
		mSyncEvent = nullptr;
	}

	return flushResult;
}
