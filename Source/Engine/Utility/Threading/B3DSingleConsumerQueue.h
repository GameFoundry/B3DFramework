//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DSignalEvent.h"
#include "B3DUtilityPrerequisites.h"
#include "Threading/B3DSignal.h"

namespace b3d
{
	class Fiber;
}

namespace b3d
{
	/**
	 * @name Threading
	 */

	/** Allows you to queue sequential commands safely from any thread/fiber, which are then processed by a single worker thread/fiber. */
	class B3D_EXPORT SingleConsumerQueue
	{
	public:
		/** Command queue for execution. */
		struct QueuedCommand
		{
			QueuedCommand(Function<void()>&& callback = nullptr, const char* debugName = nullptr, const String& debugExtraInformation = StringUtility::kBlank)
				: Callback(std::move(callback))
#if !B3D_BUILD_TYPE_SHIPPING && !B3D_BUILD_TYPE_PROFILING
				, DebugName(debugName)
				, DebugExtraInformation(debugExtraInformation)
#endif
			{}

			Function<void()> Callback; /**< Callback associated with the command. */

#if !B3D_BUILD_TYPE_SHIPPING && !B3D_BUILD_TYPE_PROFILING
			const char* DebugName; /**< Name of the command, for easier debugging. */
			const String DebugExtraInformation; /**< Additional information for debugging. */
#endif
		};

		SingleConsumerQueue();
		~SingleConsumerQueue();

		/** Returns the thread id on which the queue commands are being processed on. Only valid after RunUntilShutdown() is called. */
		u32 GetThreadId() const { return mThreadId; }

		/** Returns the scheduler thread that the queue is executing commands on. May be null if not scheduler is associated with the thread. */
		const TShared<SchedulerThread>& GetSchedulerThread() const { return mSchedulerThread; }

		/** Posts a command for execution on the queue. Optionally blocks the calling fiber/thread until the command completes. Thread safe. */
		void PostCommand(Function<void()>&& callback, const char* debugName = nullptr, bool waitUntilComplete = false, const String& extraInformation = StringUtility::kBlank);

		/** Posts a special command that requests shutdown. Optionally blocks the calling fiber/thread until the command completes. Thread safe. */
		void PostRequestShutdownCommand(bool waitUntilComplete);

		/**
		 * Processes all currently queued commands and then returns. Optionally if timeout is specified, it returns if the duration is reached even if not all commands have been processed.
		 *
		 * @param startTime		Start time used for checking when the timeout happens compared to current time. Only relevant if timeout is specified.
		 * @param timeout		If non-zero, the method will return even if not all commands have been processed, but the timeout was reached.
		 * @return				True if timeout was reached, false if all commands were processed.
		 */
		bool RunUntilIdle(TimePoint startTime = Clock::now(), Milliseconds timeout = 10ms);

		/**
		 * Runs an infinite loop that processes commands until shutdown is requested via RequestShutdown(). You must call this method or the other overload
		 * once on a thread or fiber that will be processing the commands. Note that if running this on the fiber, a busy queue can completely block other
		 * tasks from running on the fiber thread - consider using ScheduleRunUntilShutdown() instead.
		 */
		void RunUntilShutdown();

		/**
		 * Schedules a task that processes commands until shutdown is requested via RequestShutdown(). Optionally yields the fiber at specified intervals, allowing other tasks
		 * on the thread to execute.
		 *
		 * @param scheduler					Scheduler on which to post the task on.
		 * @param runOnCallingThread		If true, the task will run on the calling thread, otherwise an arbitrary thread assigned by the scheduler.
		 * @param yieldInterval				If non-zero, the scheduler task will end after this interval is reached, and it will be re-queued with the scheduler. This puts it back in the scheduler's task queue, allowing other tasks to run.
		 * @param blockUntilDone			If true, the calling fiber will be blocked until shutdown is requested.
		 */
		void ScheduleRunUntilShutdown(Scheduler& scheduler, bool runOnCallingThread, Milliseconds yieldInterval = 10ms, bool blockUntilDone = false);

		/** Cancels all currently queued commands. Thread safe. */
		void CancelAll();

		/**	Returns true if no commands are queued. Thread safe. */
		bool IsEmpty();

	private:
		u32 mThreadId = 0;
		TShared<SchedulerThread> mSchedulerThread;

		Queue<QueuedCommand>* mCommandQueue;
		Stack<Queue<QueuedCommand>*> mEmptyCommandQueues; /**< List of empty queues for reuse. */
		bool mIsShutdownRequested = false;
		Mutex mCommandQueueMutex;
		Signal mCommandAddedSignal;
		SignalEvent mCommandCompletedSignalEvent;
	};

	/** @} */
} // namespace b3d
