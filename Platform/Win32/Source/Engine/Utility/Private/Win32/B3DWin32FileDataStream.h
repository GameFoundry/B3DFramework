//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "FileSystem/B3DDataStream.h"
#include "Threading/B3DThreading.h"

namespace b3d
{
	/** @addtogroup Filesystem
	 *  @{
	 */

	/**
	 * Data stream for handling files using native Win32 file APIs (CreateFile/ReadFile/WriteFile).
	 *
	 * When opened with FileAccessFlag::Async the underlying handle is created with FILE_FLAG_OVERLAPPED, enabling true
	 * asynchronous reads via ReadAsync(). The same overlapped handle also serves the synchronous Read()/Write() calls
	 * (which issue an overlapped operation and wait for it to complete). Without the Async flag the handle is a regular
	 * synchronous handle and ReadAsync() falls back to the synchronous implementation.
	 */
	class Win32FileDataStream final : public DataStream
	{
	public:
		/**
		 * Constructs a file stream.
		 *
		 * @param	filePath	Path of the file to open.
		 * @param	access		Combination of FileAccessFlag values determining how the file is accessed. May include
		 *						FileAccessFlag::Async to enable overlapped IO and native asynchronous reads.
		 */
		Win32FileDataStream(const Path& filePath, FileAccessFlags access = FileAccessFlag::Read);
		~Win32FileDataStream() override;

		/** Opens the file stream. Must be called before any actions on the stream. Returns false if not successful. */
		bool Open();
		bool IsFile() const override { return true; }
		bool IsReadable() const override { return mAccess.IsSet(FileAccessFlag::Read); }
		bool IsWriteable() const override { return mAccess.IsSet(FileAccessFlag::Write); }
		size_t Read(void* data, size_t byteCount) const override;
		TAsyncOp<TShared<MemoryDataStream>> ReadAsync(u64 offset, size_t byteCount, TOptional<DataRange> userSuppliedMemory = TOptional<DataRange>()) override;
		size_t Write(const void* data, size_t byteCount) override;
		size_t Skip(size_t count) override;
		size_t Seek(size_t pos) override;
		size_t Tell() const override;
		bool Eof() const override;
		TShared<DataStream> Clone(bool copyData = true) const override;
		bool Flush() override;
		bool Close() override;

		/** Returns the path of the file opened by the stream. */
		const Path& GetPath() const { return mPath; }

	private:
		/** State for a single in-flight asynchronous read of a single chunk. The OVERLAPPED must be the first member so it can be recovered via CONTAINING_RECORD. */
		struct AsyncChunkReadRequest;

		/** Callback fired when an overlapped read's event is signaled. Advances to the next check or finalizes the read. */
		static void __stdcall OnAsyncChunkReadComplete(void* parameter, unsigned char timedOut);

		/**
		 * Issues an overlapped read for the next outstanding chunk of @p context and registers a wait on its
		 * completion event. Reads larger than a single ReadFile call are split into chunks and chained across
		 * completions. Returns false if the read failed synchronously, in which case the operation has already been
		 * finalized.
		 */
		bool IssueAsyncChunkRead(AsyncChunkReadRequest* request);

		/**
		 * Completion handler for a single chunk. Accumulates the bytes read and either chains the next chunk or
		 * finalizes the operation.
		 */
		void FinalizeAsyncChunkRead(AsyncChunkReadRequest* request, unsigned long errorCode, size_t bytesTransferred);

		/** Builds the result, completes the operation, releases the context and decrements the outstanding read count. */
		void FinalizeAsyncRead(AsyncChunkReadRequest* request, size_t bytesRead, bool succeeded);

		Path mPath;
		FileAccessFlags mAccess; /**< How the file was opened (read/write/async). */
		void* mHandle = nullptr; /**< Win32 HANDLE; null when the stream is closed. */
		bool mIsOverlapped = false; /**< True if the handle was opened with FILE_FLAG_OVERLAPPED (FileAccessFlag::Async). */
		void* mSyncEvent = nullptr; /**< Event used to wait on synchronous overlapped Read()/Write() calls. */
		mutable u64 mCursor = 0; /**< Current read/write byte offset from the start of the file. */
		mutable bool mEof = false; /**< Set once a read couldn't satisfy the full request (end of file reached). */

		// Bookkeeping for outstanding asynchronous reads, so Close() can cancel and drain them before closing the handle.
		Mutex mMutex;
		ConditionVariable mAllReadsComplete;
		u32 mOutstandingReads = 0;
		bool mClosed = false;
	};

	/** @} */
} // namespace b3d
