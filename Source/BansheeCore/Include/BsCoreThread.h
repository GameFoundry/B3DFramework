//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsCommandQueue.h"
#include "BsCoreThreadQueue.h"
#include "BsThreadPool.h"

namespace bs
{
	/** @addtogroup CoreThread-Internal
	 *  @{
	 */

	/** Flags that control how is a command submitted to the command queue. */
	enum CoreThreadQueueFlag
	{
		/** 
		 * Default flag, meaning the commands will be added to the per-thread queue and only begin executing after
		 * submit() has been called.
		 */
		CTQF_Default = 0,
		/** 
		 * Specifies that the queued command should be executed on the internal queue. Internal queue doesn't require
		 * a separate CoreThread::submit() call, and the queued command is instead immediately visible to the core thread.
		 * The downside is that the queue requires additional synchronization and is slower than the normal queue.
		 */
		CTQF_InternalQueue = 1 << 0,
		/**
		 * If true, the method will block until the command finishes executing on the core thread. Only relevant for the
		 * internal queue commands since contents of the normal queue won't be submitted to the core thread until the
		 * CoreThread::submit() call.
		 */
		CTQF_BlockUntilComplete = 1 << 1
	};

	typedef Flags<CoreThreadQueueFlag> CoreThreadQueueFlags;
	BS_FLAGS_OPERATORS(CoreThreadQueueFlag)

	/**
	 * Manager for the core thread. Takes care of starting, running, queuing commands and shutting down the core thread.
	 * 				
	 * How threading works:
	 *  - Commands from various threads can be queued for execution on the core thread by calling queueCommand() or
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
	class BS_CORE_EXPORT CoreThread : public Module<CoreThread>
	{
		/** Contains data about an queue for a specific thread. */
		struct ThreadQueueContainer
		{
			SPtr<TCoreThreadQueue<CommandQueueNoSync>> queue;
			bool isMain;
		};

		/** Wrapper for the thread-local variable because MSVC can't deal with a thread-local variable marked with dllimport or dllexport,  
		 *  and we cannot use per-member dllimport/dllexport specifiers because Module's members will then not be exported and its static
		 *  members will not have external linkage. */
		struct QueueData
		{
			static BS_THREADLOCAL ThreadQueueContainer* current;
		};

	public:
		CoreThread();
		~CoreThread();

		/** Returns the id of the core thread.  */
		ThreadId getCoreThreadId() { return mCoreThreadId; }

		/** Submits the commands from all queues and starts executing them on the core thread. */
		void submitAll(bool blockUntilComplete = false);

		/** Submits the commands from the current thread's queue and starts executing them on the core thread. */
		void submit(bool blockUntilComplete = false);

		/**
		 * Queues a new command that will be added to the command queue. Command returns a value.
		 * 		
		 * @param[in]	commandCallback		Command to queue.
		 * @param[in]	flags				Flags that further control command submission.
		 * @return							Structure that can be used to check if the command completed execution,
		 *									and to retrieve the return value once it has.
		 * 	
		 * @see		CommandQueue::queueReturn()
		 * @note	Thread safe
		 */
		AsyncOp queueReturnCommand(std::function<void(AsyncOp&)> commandCallback, CoreThreadQueueFlags flags = CTQF_Default);

		/**
		 * Queues a new command that will be to the global command queue. 
		 * 	
		 * @param[in]	commandCallback		Command to queue.
		 * @param[in]	flags				Flags that further control command submission.
		 *
		 * @see		CommandQueue::queue()
		 * @note	Thread safe
		 */
		void queueCommand(std::function<void()> commandCallback, CoreThreadQueueFlags flags = CTQF_Default);

		/**
		 * Called once every frame.
		 * 			
		 * @note	Must be called before sim thread schedules any core thread operations for the frame. 
		 */
		void update();

		/**
		 * Returns a frame allocator that should be used for allocating temporary data being passed to the core thread. As the 
		 * name implies the data only lasts one frame, so you need to be careful not to use it for longer than that.
		 * 			
		 * @note	Sim thread only.
		 */
		FrameAlloc* getFrameAlloc() const;

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
		static const int NUM_SYNC_BUFFERS = 2;
	private:
		/**
		 * Double buffered frame allocators. Means sim thread cannot be more than 1 frame ahead of core thread (If that changes
		 * you should be able to easily add more).
		 */
		FrameAlloc* mFrameAllocs[NUM_SYNC_BUFFERS];
		UINT32 mActiveFrameAlloc;

		static QueueData mPerThreadQueue;
		Vector<ThreadQueueContainer*> mAllQueues;

		volatile bool mCoreThreadShutdown;

		HThread mCoreThread;
		bool mCoreThreadStarted;
		ThreadId mSimThreadId;
		ThreadId mCoreThreadId;
		Mutex mCommandQueueMutex;
		Mutex mCoreQueueMutex;
		Signal mCommandReadyCondition;
		Mutex mCommandNotifyMutex;
		Signal mCommandCompleteCondition;
		Mutex mThreadStartedMutex;
		Signal mCoreThreadStartedCondition;

		CommandQueue<CommandQueueSync>* mCommandQueue;

		UINT32 mMaxCommandNotifyId; /**< ID that will be assigned to the next command with a notifier callback. */
		Vector<UINT32> mCommandsCompleted; /**< Completed commands that have notifier callbacks set up */

		/** Starts the core thread worker method. Should only be called once. */
		void initCoreThread();

		/**	Main worker method of the core thread. Called once thread is started. */
		void runCoreThread();

		/** Shutdowns the core thread. It will complete all ready commands before shutdown. */
		void shutdownCoreThread();

		/** Creates or retrieves a queue for the calling thread. */
		SPtr<TCoreThreadQueue<CommandQueueNoSync>> getQueue();

		/**
		 * Blocks the calling thread until the command with the specified ID completes. Make sure that the specified ID 
		 * actually exists, otherwise this will block forever.
		 */
		void blockUntilCommandCompleted(UINT32 commandId);

		/**
		 * Callback called by the command list when a specific command finishes executing. This is only called on commands that
		 * have a special notify on complete flag set.
		 *
		 * @param[in]	commandId	Identifier for the command.
		 */
		void commandCompletedNotify(UINT32 commandId);
	};

	/**
	 * Returns the core thread manager used for dealing with the core thread from external threads.
	 * 			
	 * @see		CoreThread
	 */
	BS_CORE_EXPORT CoreThread& gCoreThread();

	/**	Throws an exception if current thread isn't the core thread. */
	BS_CORE_EXPORT void throwIfNotCoreThread();

	/** Throws an exception if current thread is the core thread. */
	BS_CORE_EXPORT void throwIfCoreThread();

#if BS_DEBUG_MODE
#define THROW_IF_NOT_CORE_THREAD throwIfNotCoreThread();
#define THROW_IF_CORE_THREAD throwIfCoreThread();
#else
#define THROW_IF_NOT_CORE_THREAD 
#define THROW_IF_CORE_THREAD
#endif

	/** @} */

	/** @addtogroup CoreThread
	 *  @{
	 */

	/** @} */
}

