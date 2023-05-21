//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Threading/BsFiberSignal.h"

namespace bs
{
	class Fiber;
}

namespace bs
{
	/**
	 * @name Threading
	 */

	/** Allows you to queue sequential commands safely from any thread/fiber, which are then processed by a single worker thread/fiber. */
	class B3D_UTILITY_EXPORT SingleConsumerQueue
	{
	public:
		/** Command queue for execution. */
		struct QueuedCommand
		{
			QueuedCommand(Function<void()>&& callback = nullptr, const char* debugName = nullptr)
				: Callback(std::move(callback)), DebugName(debugName)
			{}

			Function<void()> Callback; /**< Callback associated with the command. */
			const char* DebugName; /**< Name of the command, for easier debugging. */
		};

		SingleConsumerQueue();
		~SingleConsumerQueue();

		/** Returns the thread id on which the queue commands are being processed on. Only valid after RunUntilShutdown() is called. */
		u32 GetThreadId() const { return mThreadId; }

		/** Posts a command for execution on the queue. Optionally blocks the calling fiber/thread until the command completes. Thread safe. */
		void PostCommand(Function<void()>&& callback, const char* debugName = nullptr, bool waitUntilComplete = false);

		/** Posts a special command that requests shutdown. Optionally blocks the calling fiber/thread until the command completes. Thread safe. */
		void PostRequestShutdownCommand(bool waitUntilComplete);

		/** Processes all currently queued commands and then returns. Must only be called from the queue's own thread/fiber. */
		void RunUntilIdle();

		/**
		 * Runs an infinite loop that processes commands until shutdown is requested via RequestShutdown(). You must call this once
		 * on a thread or fiber that will be processing the commands.
		 */
		void RunUntilShutdown();

		/** Cancels all currently queued commands. Thread safe. */
		void CancelAll();

		/**	Returns true if no commands are queued. Thread safe. */
		bool IsEmpty();

	private:
		u32 mThreadId = 0;

		Queue<QueuedCommand>* mCommandQueue;
		Stack<Queue<QueuedCommand>*> mEmptyCommandQueues; /**< List of empty queues for reuse. */
		bool mIsShutdownRequested = false;
		Mutex mCommandQueueMutex;
		FiberSignal mSignal;
	};

	/** @} */
} // namespace bs
