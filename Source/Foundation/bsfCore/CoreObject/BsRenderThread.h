//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Threading/BsSingleConsumerQueue.h"
#include "Threading/BsThreadPool.h"

namespace bs
{
	/** @addtogroup CoreThread-Internal
	 *  @{
	 */

	/**
	 * Manager for the core thread. Takes care of starting, running, queuing commands and shutting down the core thread.
	 *
	 * How threading works:
	 *  - Commands from various threads can be queued for execution on the core thread by calling QueueCommand() or
	 *    queueReturnCommand().
	 *   - Internally each thread maintains its own separate queue of commands, so you cannot interleave commands from
	 *     different threads.
	 *   - There is also the internal command queue, which is the only queue directly visible from the core thread.
	 *    - Core thread continually polls the internal command queue for new commands, and executes them in order they were
	 *      submitted.
	 *    - Commands queued on the per-thread queues are submitted to the internal command queue by calling submit(), at
	 *      which point they are made visible to the core thread, and will begin executing.
	 * 	  - Commands can also be submitted directly to the internal command queue (via a special flag), but with a
	 * 	    performance cost due to extra synchronization required.
	 */
	class B3D_CORE_EXPORT CoreThread : public Module<CoreThread>
	{
	public:
		CoreThread();
		~CoreThread();

		void OnStartUp() override;

		/** Returns the id of the core thread.  */
		ThreadId GetCoreThreadId() const { return mCoreThreadId; }

		/**
		 * Queues a new command that will be added to the core thread command queue.
		 *
		 * @param	commandCallback		Command to queue.
		 * @param	waitUntilComplete	If true, the caller will block until the command finishes executing.
		 *
		 * @note	Thread safe.
		 */
		void PostCommand(std::function<void()>&& commandCallback, bool waitUntilComplete = false);

		/**
		 * @name Internal
		 * @{
		 */

#if BS_CORE_THREAD_IS_MAIN
		/** Runs the core thread loop as soon as CoreThread module is started. */
		static void RunInternal();
#endif

		/** @} */

		/**
		 * Returns number of buffers needed to sync data between core and sim thread. Currently the sim thread can be one frame
		 * ahead of the core thread, meaning we need two buffers. If this situation changes increase this number.
		 *
		 * For example:
		 *  - Sim thread frame starts, it writes some data to buffer 0.
		 *  - Core thread frame starts, it reads some data from buffer 0.
		 *  - Sim thread frame finishes
		 *  - New sim thread frame starts, it writes some data to buffer 1.
		 *  - Core thread still working, reading from buffer 0. (If we were using just one buffer at this point core thread
		 *	  would be reading wrong data).
		 *  - Sim thread waiting for core thread (application defined that it cannot go ahead more than one frame)
		 *  - Core thread frame finishes.
		 *  - New core thread frame starts, it reads some data from buffer 1.
		 *  - ...
		 */
		static constexpr int kSyncBufferCount = 2;

	private:
		bool mCoreThreadStarted = false;
		Scheduler mScheduler;
		ThreadId mCoreThreadId;
		Mutex mThreadStartedMutex;
		ConditionVariable mCoreThreadStartedCondition;
#if BS_CORE_THREAD_IS_MAIN
		static bool sAppStarted;
		static Mutex sAppStartedMutex;
		static Signal sAppStartedCondition;
#else
		SPtr<PooledThread> mCoreThread;
#endif

		SingleConsumerQueue mCommandQueue;
	};

	/**
	 * Returns the core thread manager used for dealing with the core thread from external threads.
	 *
	 * @see		CoreThread
	 */
	B3D_CORE_EXPORT CoreThread& GetCoreThread();

	/**	Throws an exception if current thread isn't the render thread. */
	B3D_CORE_EXPORT void AssertIfNotRenderThread();

	/** Throws an exception if current thread is the render thread. */
	B3D_CORE_EXPORT void AssertIfRenderThread();

	/** Returns false if currently not at the render thread, and triggers an ensure. */
	B3D_CORE_EXPORT inline bool EnsureRenderThread()
	{
		return B3D_ENSURE(B3D_CURRENT_THREAD_ID == CoreThread::Instance().GetCoreThreadId());
	}

#if B3D_DEBUG
#	define ASSERT_IF_NOT_RENDER_THREAD AssertIfNotRenderThread();
#	define ASSERT_IF_RENDER_THREAD AssertIfRenderThread();
#else
#	define ASSERT_IF_NOT_RENDER_THREAD 
#	define ASSERT_IF_RENDER_THREAD 
#endif

	/** @} */

	/** @addtogroup CoreThread
	 *  @{
	 */

	/** @} */
} // namespace bs
