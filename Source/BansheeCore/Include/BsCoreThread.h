//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsCommandQueue.h"
#include "BsCoreThreadAccessor.h"
#include "BsThreadPool.h"

namespace bs
{
	/** @addtogroup CoreThread-Internal
	 *  @{
	 */

	/**
	 * Manager for the core thread. Takes care of starting, running, queuing commands and shutting down the core thread.
	 * 			
	 * @note	
	 * How threading works:
	 * 	- This class contains a queue which is filled by commands from other threads via queueCommand() and queueReturnCommand()  
	 * 	- Commands are executed on the core thread as soon as they are queued (if core thread is not busy with previous commands)  
	 * 	- Core thread accessors are helpers for queuing commands. They perform better than queuing each command directly 
	 *    using queueCommand() or queueReturnCommand().
	 * 	- Accessors contain a command queue of their own, and queuing commands in them will not automatically start 
	 *    executing the commands like with queueCommand or queueReturnCommand. Instead you must manually call 
	 *    submitAccessors() when you are ready to send their commands to the core thread. Sending commands "in bulk" like 
	 *    this is what makes them faster than directly queuing commands.
	 * 	- Synced accessor is a special type of accessor which may be accessed from any thread. Its commands are always 
	 *    executed after all other non-synced accessors. It is primarily useful when multiple threads are managing the same
	 *    resource and you must ensure proper order of operations. You should use normal accessors whenever possible as 
	 *    synced accessors involve potentially slow synchronization operations.
	 */
	class BS_CORE_EXPORT CoreThread : public Module<CoreThread>
	{
		/** Contains data about an accessor for a specific thread. */
		struct AccessorContainer
		{
			SPtr<CoreThreadAccessor<CommandQueueNoSync>> accessor;
			bool isMain;
		};

		/** Wrapper for the thread-local variable because MSVC can't deal with a thread-local variable marked with dllimport or dllexport,  
		 *  and we cannot use per-member dllimport/dllexport specifiers because Module's members will then not be exported and its static
		 *  members will not have external linkage. */
		struct AccessorData
		{
			static BS_THREADLOCAL AccessorContainer* current;
		};

public:
	CoreThread();
	~CoreThread();

	/** Returns the id of the core thread.  */
	ThreadId getCoreThreadId() { return mCoreThreadId; }

	/**
	 * Creates or retrieves an accessor that you can use for executing commands on the core thread from a non-core thread. 
	 * The accessor will be bound to the thread you call this method on.
	 * 			
	 * @note		
	 * Accessors contain their own command queue and their commands will only start to get executed once that queue is 
	 * submitted to the core thread via submitAccessors() method.
	 */
	SPtr<CoreThreadAccessor<CommandQueueNoSync>> getAccessor();

	/**
	 * Retrieves an accessor that you can use for executing commands on the core thread from a non-core thread. There is 
	 * only one synchronized accessor and you may access it from any thread you wish. Note however that it is much more 
	 * efficient to retrieve a separate non-synchronized accessor for each thread you will be using it on.
	 * 			
	 * @note	
	 * Accessors contain their own command queue and their commands will only start to get executed once that queue
	 * is submitted to the core thread via submitAccessors() method.
	 * @note			
	 * Synced accessor commands are sent after all non-synced accessor commands are sent.
	 */
	SyncedCoreAccessor& getSyncedAccessor();

	/** Queues all the accessor commands and starts executing them on the core thread. */
	void submitAccessors(bool blockUntilComplete = false);

	/**
	 * Queues a new command that will be added to the global command queue. You are allowed to call this from any thread,
	 * however be aware that it involves possibly slow synchronization primitives, so limit your usage.
	 * 		
	 * @param[in]	commandCallback		Command to queue.
	 * @param[in]	blockUntilComplete	If true the thread will be blocked until the command executes. Be aware that there 
	 *									may be many commands queued before it and they all need to be executed in order 
	 *									before the current command is reached, which might take a long time.
	 * 	
	 * @see		CommandQueue::queueReturn()
	 */
	AsyncOp queueReturnCommand(std::function<void(AsyncOp&)> commandCallback, bool blockUntilComplete = false);

	/**
	 * Queues a new command that will be added to the global command queue.You are allowed to call this from any thread,
	 * however be aware that it involves possibly slow synchronization primitives, so limit your usage.
	 * 	
	 * @param[in]	commandCallback		Command to queue.
	 * @param[in]	blockUntilComplete	If true the thread will be blocked until the command executes. Be aware that there 
	 *									may be many commands queued before it and they all need to be executed in order 
	 *									before the current command is reached, which might take a long time.
	 *
	 * @see		CommandQueue::queue()
	 */
	void queueCommand(std::function<void()> commandCallback, bool blockUntilComplete = false);

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

	static AccessorData mAccessor;
	Vector<AccessorContainer*> mAccessors;

	volatile bool mCoreThreadShutdown;

	HThread mCoreThread;
	bool mCoreThreadStarted;
	ThreadId mSimThreadId;
	ThreadId mCoreThreadId;
	Mutex mCommandQueueMutex;
	Mutex mAccessorMutex;
	Signal mCommandReadyCondition;
	Mutex mCommandNotifyMutex;
	Signal mCommandCompleteCondition;
	Mutex mThreadStartedMutex;
	Signal mCoreThreadStartedCondition;

	CommandQueue<CommandQueueSync>* mCommandQueue;

	UINT32 mMaxCommandNotifyId; /**< ID that will be assigned to the next command with a notifier callback. */
	Vector<UINT32> mCommandsCompleted; /**< Completed commands that have notifier callbacks set up */

	SyncedCoreAccessor* mSyncedCoreAccessor;

	/** Starts the core thread worker method. Should only be called once. */
	void initCoreThread();

	/**	Main worker method of the core thread. Called once thread is started. */
	void runCoreThread();

	/** Shutdowns the core thread. It will complete all ready commands before shutdown. */
	void shutdownCoreThread();

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

	/** 
	 * Creates or retrieves an accessor that you can use for executing commands on the core thread from a non-core thread. 
	 * The accessor will be bound to the thread you call this method on. 
	 */
	BS_CORE_EXPORT CoreThreadAccessor<CommandQueueNoSync>& gCoreAccessor();

	/**
	 * Retrieves an accessor that you can use for executing commands on the core thread from a non-core thread. There is 
	 * only one synchronized accessor and you may access it from any thread you wish. Note however that it is much more 
	 * efficient to retrieve a separate non-synchronized accessor for each thread you will be using it on.
	 */
	BS_CORE_EXPORT CoreThreadAccessor<CommandQueueSync>& gSyncedCoreAccessor();

	/** @} */
}

