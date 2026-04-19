//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Threading/B3DSingleConsumerQueue.h"
#include "Threading/B3DThreadPool.h"

namespace b3d
{
	/** @addtogroup RenderThread
	 *  @{
	 */

	/**
	 * Manager for the render thread. Takes care of starting, running, queuing commands and shutting down the render thread.
	 *
	 * Commands from various threads can be queued for execution on the render thread by calling PostCommand()
	 */
	class B3D_EXPORT RenderThread : public Module<RenderThread>
	{
	public:
		RenderThread();
		~RenderThread();

		void OnStartUp() override;

		/** Returns the id of the render thread.  */
		ThreadId GetThreadId() const { return mRenderThreadId; }

		/**
		 * Queues a new command that will be added to the render thread command queue.
		 *
		 * @param commandCallback			Command to queue.
		 * @param debugName					Optional name for identifying the command more easily.
		 * @param waitUntilComplete			If true, the caller will block until the command finishes executing.
		 * @param extraDebugInformation		Additional information for debugging (e.g. associated object name).
		 *
		 * @note	Thread safe.
		 */
		void PostCommand(std::function<void()>&& commandCallback, const char* debugName = "Render thread command", bool waitUntilComplete = false, const String& extraDebugInformation = StringUtility::kBlank);

		/**
		 * @name Internal
		 * @{
		 */

#if B3D_SWAP_RENDER_AND_MAIN_THREAD
		/** Runs the render thread loop as soon as RenderThread module is started. */
		static void RunInternal();
#endif

		/** @} */

		/**
		 * Returns number of buffers needed to sync data between render and main thread. Currently the main thread can be one frame
		 * ahead of the render thread, meaning we need two buffers. If this situation changes increase this number.
		 *
		 * For example:
		 *  - Main thread frame starts, it writes some data to buffer 0.
		 *  - Render thread frame starts, it reads some data from buffer 0.
		 *  - Main thread frame finishes
		 *  - New main thread frame starts, it writes some data to buffer 1.
		 *  - Render thread still working, reading from buffer 0. (If we were using just one buffer at this point render thread would be reading wrong data).
		 *  - Main thread waiting for render thread (application defined that it cannot go ahead more than one frame)
		 *  - Render thread frame finishes.
		 *  - New render thread frame starts, it reads some data from buffer 1.
		 *  - ...
		 */
		static constexpr int kSyncBufferCount = 2;

		/** Maximum number of frames that can be simultaneously in-flight. One submitted on the GPU, one on the submit thread waiting submission, and one the render thread is recording. */
		static constexpr u32 kMaximumFramesInFlight = 3;

	private:
		bool mRenderThreadStarted = false;
		Scheduler mScheduler;
		ThreadId mRenderThreadId;
		Mutex mThreadStartedMutex;
		ConditionVariable mThreadStartedCondition;
#if B3D_SWAP_RENDER_AND_MAIN_THREAD
		static bool sAppStarted;
		static Mutex sAppStartedMutex;
		static Signal sAppStartedCondition;
#else
		SPtr<PooledThread> mRenderThread;
#endif

		SingleConsumerQueue mCommandQueue;
	};

	/**
	 * Returns the render thread manager used for dealing with the render thread from external threads.
	 *
	 * @see		RenderThread
	 */
	B3D_EXPORT RenderThread& GetRenderThread();

	/**	Throws an exception if current thread isn't the render thread. */
	B3D_EXPORT void AssertIfNotRenderThread();

	/** Throws an exception if current thread is the render thread. */
	B3D_EXPORT void AssertIfRenderThread();

	/** Returns false if currently not at the render thread, and triggers an ensure. */
	B3D_EXPORT inline bool EnsureRenderThread()
	{
		return B3D_ENSURE(B3D_CURRENT_THREAD_ID == RenderThread::Instance().GetThreadId());
	}

#if B3D_DEBUG
#	define ASSERT_IF_NOT_RENDER_THREAD AssertIfNotRenderThread();
#	define ASSERT_IF_RENDER_THREAD AssertIfRenderThread();
#else
#	define ASSERT_IF_NOT_RENDER_THREAD 
#	define ASSERT_IF_RENDER_THREAD 
#endif

	/** @} */
} // namespace b3d
