//************************************ bs::framework - Copyright 2022 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCommandQueue.h"
#include "BsCorePrerequisites.h"
#include "Threading/BsAsyncOp.h"
#include "Threading/BsThreadPool.h"

namespace bs
{
	/** @addtogroup Threading
	 *  @{
	 */

	/** Starts a worker thread that processes commands queued on it. */
	class B3D_CORE_EXPORT WorkerThreadWithCommandQueue
	{
	public:
		WorkerThreadWithCommandQueue(const char* threadName);
		~WorkerThreadWithCommandQueue();

		/** Returns the ID of the worker thread the queue is being processed on. */
		ThreadId GetThreadId() const { return mThreadId; }

		/**
		 * Queue up a new command to execute. Returns an object that can be used to track command completion and return value.
		 *
		 * @param	commandCallback		Command to queue for execution.
		 * @param	name				Name of the command, primarily for debugging.
		 * @param	waitUntilComplete	If true, the calling thread will block until the command executes.
		 * @return						Async operation object that you can check for command completion and return value.
		 */
		AsyncOp QueueCommand(std::function<void(AsyncOp&)>&& commandCallback, const char* name, bool waitUntilComplete = false);

		/**
		 * Queue up a new command to execute. 
		 *
		 * @param	commandCallback		Command to queue for execution.
		 * @param	name				Name of the command, primarily for debugging.
		 * @param	waitUntilComplete	If true, the calling thread will block until the command executes.
		 */
		void QueueCommand(std::function<void()>&& commandCallback, const char* name, bool waitUntilComplete = false);

	private:
		/** Blocks the calling thread until the command with the specified ID completes. */
		void WaitUntilCommandCompletes(u32 commandId);

		HThread mThread;
		ThreadId mThreadId;
		bool mIsWorkerStarted = false;
		bool mIsWorkerShutdownRequested = false;

		Signal mThreadStartedSignal;
		Mutex mThreadStartedMutex;
		Signal mCommandQueuedSignal;
		Mutex mCommandQueuedMutex;
		Signal mCommandCompleteSignal;
		Mutex mCommandCompleteMutex;

		CommandQueue<> mCommandQueue;

		u32 mNextBlockableCommandId = 0;
		Vector<u32> mCompletedCommandIds;
	};


	/** @} */
} // namespace bs
