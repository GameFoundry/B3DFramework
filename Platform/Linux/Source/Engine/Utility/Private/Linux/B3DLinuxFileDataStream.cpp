//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/Linux/B3DLinuxFileDataStream.h"

#include "Debug/B3DDebug.h"

#include <errno.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <cstring>

using namespace b3d;

namespace
{
	/** Maximum number of in-flight chunks for the entire process. */
	constexpr u32 kRingQueueDepth = 256;

	/** Per-chunk read size for the shared ring. 16MB, Larger requests are chained across CQEs by the reaper. */
	constexpr u64 kAsyncChunkBytes = 0x1000000ull;

	/** Sentinel user_data for the eventfd-poll SQE so the reaper can distinguish wake-ups from actual chunk completions. */
	constexpr u64 kEventFdSentinel = 1ull;

	/**
	 * io_uring_wait_cqe failure log throttle: log at counts 1, 10, 100; after this many consecutive failures the
	 * reaper gives up and exits to avoid wedging the process in an infinite log-spew loop.
	 */
	constexpr u32 kWaitFailureGiveUp = 1000;
}

// ************************************************************************************************************************
// LinuxFileIOManager
// ************************************************************************************************************************

void LinuxFileIOManager::OnStartUp()
{
	// Bring up the ring. Any failure here leaves IsRunning() == false
	const int ringErr = ::io_uring_queue_init(kRingQueueDepth, &mRing, 0);
	if(ringErr < 0)
	{
		B3D_LOG(Warning, LogFileSystem, "Failed to initialize the shared io_uring instance (error {0}). Linux async file reads will fall back to synchronous I/O.", String(strerror(-ringErr)));
		return;
	}

	mEventFd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if(mEventFd < 0)
	{
		B3D_LOG(Warning, LogFileSystem, "Failed to create the shared io_uring eventfd (error {0}). Linux async file reads will fall back to synchronous I/O.", String(strerror(errno)));
		::io_uring_queue_exit(&mRing);
		return;
	}

	// Arm the eventfd poll before spawning the reaper so the very first RequestRead() can wake it. The sentinel
	// user_data lets the reaper distinguish wake-ups from chunk completions; the reaper re-arms the poll after each
	// wake.
	struct io_uring_sqe* sqe = ::io_uring_get_sqe(&mRing);
	if(sqe == nullptr)
	{
		B3D_LOG(Warning, LogFileSystem, "Failed to obtain initial SQE for the shared io_uring. Linux async file reads will fall back to synchronous I/O.");
		::close(mEventFd);
		mEventFd = -1;
		::io_uring_queue_exit(&mRing);
		return;
	}

	::io_uring_prep_poll_add(sqe, mEventFd, POLLIN);
	::io_uring_sqe_set_data(sqe, (void*)kEventFdSentinel);

	// liburing returns negative errno directly; do not consult the global errno.
	const int submitRet = ::io_uring_submit(&mRing);
	if(submitRet < 0)
	{
		B3D_LOG(Warning, LogFileSystem, "Failed to submit initial eventfd poll for the shared io_uring (error {0}). Linux async file reads will fall back to synchronous I/O.", String(strerror(-submitRet)));
		::close(mEventFd);
		mEventFd = -1;
		::io_uring_queue_exit(&mRing);
		return;
	}

	mReaper = Thread([this] { ReaperThreadMain(); });
	mRunning.store(true, std::memory_order_release);
}

void LinuxFileIOManager::OnShutDown()
{
	if(!mRunning.load(std::memory_order_acquire))
		return; // OnStartUp() failed; nothing to tear down.

	// Stop accepting new submissions and wake the reaper. The reaper drains any pending queue (failing each request),
	// waits for in-flight CQEs to come back from the kernel, then exits its loop. mRunning flips to false here so any
	// late RequestRead() callers fail their op inline rather than wedging on the joined reaper thread.
	mStopping.store(true, std::memory_order_release);
	mRunning.store(false, std::memory_order_release);

	const uint64_t one = 1;
	if(::write(mEventFd, &one, sizeof(one)) != (ssize_t)sizeof(one))
	{
		if(errno != EAGAIN)
			B3D_LOG(Warning, LogFileSystem, "Failed to signal shutdown to the shared io_uring reaper (error {0}).", String(strerror(errno)));
	}

	mReaper.WaitUntilComplete();

	::io_uring_queue_exit(&mRing);
	::close(mEventFd);
	mEventFd = -1;
}

void LinuxFileIOManager::RequestRead(const AsyncReadRequest& request)
{
	AsyncReadOperation* operation = AllocateOperation();
	operation->Request = request;

	// Defensive: the stream is supposed to check IsRunning() before calling, but a race between that check and here can
	// land us with mStopping flipped under us. Finalize the operation inline so the stream's outstanding-read counter
	// still rebalances and Close() can make progress.
	if(mStopping.load(std::memory_order_acquire))
	{
		FinalizeOperation(operation, 0, false);
		return;
	}

	{
		Lock lock(mSubmitMutex);
		mPending.push_back(operation);
	}

	const uint64_t one = 1;
	if(::write(mEventFd, &one, sizeof(one)) != (ssize_t)sizeof(one))
	{
		// EAGAIN means the eventfd counter is saturated, which still wakes the reaper - prior writes haven't been
		// drained yet. Any other error here is unexpected; the request stays queued and will be picked up on the next
		// wake.
		if(errno != EAGAIN)
			B3D_LOG(Warning, LogFileSystem, "Failed to signal eventfd for an asynchronous read (error {0}).", String(strerror(errno)));
	}
}

void LinuxFileIOManager::ReaperThreadMain()
{
	Thread::SetName("io_uring reaper");

	// Reaper-local in-flight counter. Tracks SQEs the reaper has posted that have not yet had their final CQE arrive
	// (counting chunk-chained submissions for a single request as one in-flight slot, since the CQE for chunk N posts
	// the SQE for chunk N+1 from this same thread). Only the reaper touches this; no atomic needed.
	u32 inFlightCount = 0;

	while(true)
	{
		// Exit when shutdown is in progress, the pending queue is drained, and every SQE we ever posted has had its
		// CQE reaped. Without all three the ring still has work to do and tearing it down would orphan kernel writes.
		{
			Lock lock(mSubmitMutex);
			if(mStopping.load(std::memory_order_acquire) && mPending.empty() && inFlightCount == 0)
				break;
		}

		struct io_uring_cqe* cqe = nullptr;
		const int waitErr = ::io_uring_wait_cqe(&mRing, &cqe);
		if(waitErr < 0)
		{
			if(waitErr == -EINTR)
			{
				// Signal-interrupted; not a real failure. Don't trip the throttle counter.
				continue;
			}

			// Throttled logging: 1, 10, 100. After kWaitFailureGiveUp consecutive failures the ring is wedged;
			// flip mStopping ourselves so the exit condition catches and we tear down rather than spinning in here.
			mWaitFailureCount++;
			if(mWaitFailureCount == 1 || mWaitFailureCount == 10 || mWaitFailureCount == 100)
				B3D_LOG(Warning, LogFileSystem, "io_uring_wait_cqe failed for the shared ring (error {0}, occurrence {1}).", String(strerror(-waitErr)), mWaitFailureCount);

			if(mWaitFailureCount >= kWaitFailureGiveUp)
			{
				B3D_LOG(Error, LogFileSystem, "io_uring_wait_cqe failed {0} consecutive times; giving up on the shared ring.", mWaitFailureCount);
				mStopping.store(true, std::memory_order_release);
				mRunning.store(false, std::memory_order_release);
			}
			continue;
		}

		mWaitFailureCount = 0;

		void* userData = ::io_uring_cqe_get_data(cqe);
		const int32_t res = cqe->res;

		if((u64)userData == kEventFdSentinel)
		{
			// Drain the eventfd counter so subsequent writes can re-arm the poll. Multiple writes coalesce into one
			// counter value, so one read is always sufficient.
			uint64_t counter = 0;
			const ssize_t drainRet = ::read(mEventFd, &counter, sizeof(counter));
			if(drainRet < 0 && errno != EAGAIN)
				B3D_LOG(Warning, LogFileSystem, "Failed to drain the shared io_uring eventfd (error {0}).", String(strerror(errno)));

			// Swap the pending vector with our reaper-local buffer under the lock, then iterate unlocked - SubmitChunk
			// and io_uring_submit can be slow and there's no reason to hold the caller-facing mutex while they run.
			// Both vectors retain capacity across wakes, so after the first growth this swap+drain pair allocates
			// nothing.
			B3D_ASSERT(mReadyOperations.empty()); // Previous iteration ended with clear(); must be empty before swap.
			{
				Lock lock(mSubmitMutex);
				mReadyOperations.swap(mPending);
			}

			const bool stopping = mStopping.load(std::memory_order_acquire);
			for(AsyncReadOperation* operation : mReadyOperations)
			{
				if(stopping)
				{
					// Manager going away: fail the operation inline so the stream's outstanding counter rebalances.
					FinalizeOperation(operation, 0, false);
				}
				else if(SubmitChunk(operation))
				{
					inFlightCount++;
				}
				// SubmitChunk already finalized on failure; nothing more to do.
			}
			mReadyOperations.clear(); // Preserves capacity for the next wake.

			// Re-arm the eventfd poll so subsequent RequestRead() calls can wake us. Even during shutdown we re-arm:
			// the exit condition handles termination cleanly once everything drains, and the SQE is freed by
			// io_uring_queue_exit() in OnShutDown().
			struct io_uring_sqe* pollSqe = ::io_uring_get_sqe(&mRing);
			if(pollSqe == nullptr)
			{
				::io_uring_submit(&mRing);
				pollSqe = ::io_uring_get_sqe(&mRing);
			}

			if(pollSqe != nullptr)
			{
				::io_uring_prep_poll_add(pollSqe, mEventFd, POLLIN);
				::io_uring_sqe_set_data(pollSqe, (void*)kEventFdSentinel);
				const int rearmRet = ::io_uring_submit(&mRing);
				if(rearmRet < 0)
					B3D_LOG(Error, LogFileSystem, "Failed to submit eventfd poll re-arm on the shared ring (error {0}). Subsequent RequestRead() wake-ups may be delayed until an in-flight read completes.", String(strerror(-rearmRet)));
			}
			else
			{
				B3D_LOG(Error, LogFileSystem, "Failed to obtain SQE for eventfd poll re-arm on the shared ring. Subsequent RequestRead() wake-ups may be delayed until an in-flight read completes.");
			}
		}
		else
		{
			AsyncReadOperation* operation = static_cast<AsyncReadOperation*>(userData);

			if(res < 0)
			{
				// Negative result is a kernel error. The bytes already accumulated from prior chunks are discarded
				// to match the Win32 behaviour where any in-flight failure aborts the whole operation.
				B3D_LOG(Warning, LogFileSystem, "Asynchronous read failed (error {0}).", String(strerror(-res)));
				FinalizeOperation(operation, (size_t)operation->BytesRead, false);
				inFlightCount--;
			}
			else if(res == 0)
			{
				// EOF before the clamped total was reached - finish with whatever we have. Reads are clamped to file
				// size at issue time so this is rare in practice (only happens if the file was truncated under us).
				FinalizeOperation(operation, (size_t)operation->BytesRead, true);
				inFlightCount--;
			}
			else
			{
				operation->BytesRead += (u64)res;

				if(operation->BytesRead < operation->Request.TotalByteCount)
				{
					// Chain the next chunk. SubmitChunk reuses the same in-flight slot (the SQE we just had a CQE
					// for is being replaced); on submit failure it finalizes the operation and we drop the slot.
					if(!SubmitChunk(operation))
						inFlightCount--;
				}
				else
				{
					FinalizeOperation(operation, (size_t)operation->BytesRead, true);
					inFlightCount--;
				}
			}
		}

		::io_uring_cqe_seen(&mRing, cqe);
	}
}

bool LinuxFileIOManager::SubmitChunk(AsyncReadOperation* operation)
{
	const AsyncReadRequest& request = operation->Request;
	const u64 fileOffset = request.BaseOffset + operation->BytesRead;
	const u64 remaining = request.TotalByteCount - operation->BytesRead;
	const u32 chunkLen = remaining > kAsyncChunkBytes ? (u32)kAsyncChunkBytes : (u32)remaining;

	// Only the reaper touches the SQ, so a null SQE here means the queue genuinely filled up. Flush what we have and
	// retry; rare enough that the extra syscall is negligible.
	struct io_uring_sqe* sqe = ::io_uring_get_sqe(&mRing);
	if(sqe == nullptr)
	{
		const int flushRet = ::io_uring_submit(&mRing);
		if(flushRet < 0)
		{
			B3D_LOG(Warning, LogFileSystem, "io_uring_submit failed while flushing for the shared ring (error {0}).", String(strerror(-flushRet)));
			FinalizeOperation(operation, (size_t)operation->BytesRead, false);
			return false;
		}

		sqe = ::io_uring_get_sqe(&mRing);
		if(sqe == nullptr)
		{
			B3D_LOG(Warning, LogFileSystem, "io_uring submission queue exhausted on the shared ring.");
			FinalizeOperation(operation, (size_t)operation->BytesRead, false);
			return false;
		}
	}

	::io_uring_prep_read(sqe, request.FileDescriptor, static_cast<u8*>(request.Buffer) + operation->BytesRead, chunkLen, fileOffset);
	::io_uring_sqe_set_data(sqe, operation);

	// liburing returns negative errno directly on failure - don't consult the global errno.
	const int submitRet = ::io_uring_submit(&mRing);
	if(submitRet < 0)
	{
		B3D_LOG(Warning, LogFileSystem, "io_uring_submit failed on the shared ring (error {0}).", String(strerror(-submitRet)));
		FinalizeOperation(operation, (size_t)operation->BytesRead, false);
		return false;
	}

	return true;
}

void LinuxFileIOManager::FinalizeOperation(AsyncReadOperation* operation, size_t bytesRead, bool succeeded)
{
	AsyncReadRequest& request = operation->Request;

	TShared<MemoryDataStream> result;
	if(succeeded)
	{
		if(request.OwnsBuffer)
			result = B3DMakeShared<MemoryDataStream>(request.Buffer, bytesRead, true);
		else
			result = B3DMakeShared<MemoryDataStream>(request.Buffer, bytesRead);
	}
	else if(request.OwnsBuffer && request.Buffer != nullptr)
	{
		B3DFree(request.Buffer);
	}

	request.Op.CompleteOperation(result);

	// Tell the owning stream the read finalized before the operation is recycled - the stream uses this signal to
	// decrement its per-stream outstanding count and wake any Close()-side drain.
	if(request.Stream != nullptr)
		request.Stream->OnAsyncReadComplete();

	ReleaseOperation(operation);
}

LinuxFileIOManager::AsyncReadOperation* LinuxFileIOManager::AllocateOperation()
{
	Lock lock(mOperationMutex);
	return mOperationPool.Allocate();
}

void LinuxFileIOManager::ReleaseOperation(AsyncReadOperation* operation)
{
	// Releasing destructs the operation, dropping its AsyncOp/stream references (and stale chunk progress); the next
	// Allocate() default-constructs a clean slot. The caller no longer touches the operation, so this is safe.
	Lock lock(mOperationMutex);
	mOperationPool.Release(operation);
}

// ************************************************************************************************************************
// LinuxFileDataStream
// ************************************************************************************************************************

LinuxFileDataStream::LinuxFileDataStream(const Path& path, FileAccessFlags access)
	: UnixFileDataStream(path, access)
{ }

LinuxFileDataStream::~LinuxFileDataStream()
{
	LinuxFileDataStream::Close();
}

bool LinuxFileDataStream::Open()
{
	if(!UnixFileDataStream::Open())
		return false;

	if(!mAccess.IsSet(FileAccessFlag::Async))
		return true;

	mAsyncReady = Module<LinuxFileIOManager>::IsStarted() && LinuxFileIOManager::Instance().IsRunning();
	if(!mAsyncReady)
		B3D_LOG(Warning, LogFileSystem, "Async backend not available for file '{0}'. Falling back to synchronous reads.", mPath);

	return true;
}

TAsyncOp<TShared<MemoryDataStream>> LinuxFileDataStream::ReadAsync(u64 offset, size_t byteCount, TOptional<DataRange> userSuppliedMemory)
{
	// No async backend: manager isn't running (init failed, or the stream wasn't opened with Async). Use the
	// synchronous default - it issues a positioned Read and completes inline.
	if(!mAsyncReady)
		return UnixFileDataStream::ReadAsync(offset, byteCount, userSuppliedMemory);

	TAsyncOp<TShared<MemoryDataStream>> op;

	if(byteCount == 0)
	{
		op.CompleteOperation(B3DMakeShared<MemoryDataStream>());
		return op;
	}

	const u64 available = offset < mSize ? (mSize - offset) : 0;
	const size_t totalByteCount = byteCount > available ? (size_t)available : byteCount;

	if(totalByteCount == 0)
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

	// The manager copies this into a pooled operation, so a stack-local request is fine.
	LinuxFileIOManager::AsyncReadRequest request;
	request.Stream = this;
	request.FileDescriptor = mFd;
	request.BaseOffset = offset;
	request.TotalByteCount = totalByteCount;
	request.Op = op;

	if(userSuppliedMemory.has_value())
	{
		request.Buffer = userSuppliedMemory->Data;
		request.OwnsBuffer = false;
	}
	else
	{
		request.Buffer = B3DAllocate(totalByteCount);
		request.OwnsBuffer = true;

		if(request.Buffer == nullptr)
		{
			B3D_LOG(Error, LogFileSystem, "Failed to allocate {0} bytes for an asynchronous read.", (u64)totalByteCount);
			// Bypass the manager: with no buffer there is nothing to submit. Complete the op with null and rebalance the
			// outstanding counter we bumped above so Close() can make progress.
			op.CompleteOperation(nullptr);
			OnAsyncReadComplete();
			return op;
		}
	}

	LinuxFileIOManager::Instance().RequestRead(request);
	return op;
}

void LinuxFileDataStream::OnAsyncReadComplete()
{
	// Called from the LinuxFileIOManager reaper after a request the stream submitted has finalized. Decrements the
	// per-stream outstanding count and wakes the Close()-side drain.
	Lock lock(mAsyncMutex);
	B3D_ASSERT(mOutstandingReads > 0);
	mOutstandingReads--;

	if(mOutstandingReads == 0)
		mAllReadsComplete.notify_all();
}

TShared<DataStream> LinuxFileDataStream::Clone(bool /*copyData*/) const
{
	TShared<LinuxFileDataStream> clone = B3DMakeShared<LinuxFileDataStream>(mPath, mAccess);

	// A writable clone stays closed, as opening it would truncate the file this stream is still writing
	if(!mAccess.IsSet(FileAccessFlag::Write))
		clone->Open();

	return clone;
}

bool LinuxFileDataStream::Close()
{
	{
		Lock lock(mAsyncMutex);
		if(mClosed)
			return UnixFileDataStream::Close();

		mClosed = true;
	}

	if(mAsyncReady)
	{
		// Block until every in-flight read finalizes via OnAsyncReadComplete(). In-flight kernel reads complete
		// naturally; cancelling them with IORING_OP_ASYNC_CANCEL is possible but adds complexity for what is
		// typically a millisecond-scale wait. The drain is bounded by the chunk currently inside the kernel (at most
		// kAsyncChunkBytes = 16 MiB), which on any modern storage is well under a frame.
		Lock lock(mAsyncMutex);
		mAllReadsComplete.wait(lock, [this] { return mOutstandingReads == 0; });
		mAsyncReady = false;
	}

	return UnixFileDataStream::Close();
}
