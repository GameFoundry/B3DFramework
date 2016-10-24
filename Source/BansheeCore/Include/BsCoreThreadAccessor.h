//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCommandQueue.h"
#include "BsAsyncOp.h"

namespace BansheeEngine
{
	/** @addtogroup CoreThread
	 *  @{
	 */

	/**
	 * Contains base functionality used for CoreThreadAccessor.
	 * 			
	 * @see		CoreThreadAccesor
	 */
	class BS_CORE_EXPORT CoreThreadAccessorBase
	{
	public:
		CoreThreadAccessorBase(CommandQueueBase* commandQueue);
		virtual ~CoreThreadAccessorBase();

		/**
		 * Queues a new generic command that will be added to the command queue. Returns an async operation object that you 
		 * may use to check if the operation has finished, and to retrieve the return value once finished.
		 */
		AsyncOp queueReturnCommand(std::function<void(AsyncOp&)> commandCallback);

		/** Queues a new generic command that will be added to the command queue. */
		void queueCommand(std::function<void()> commandCallback);

		/**
		 * Makes all the currently queued commands available to the core thread. They will be executed as soon as the core 
		 * thread is ready. All queued commands are removed from the accessor.
		 *
		 * @param[in]	blockUntilComplete	If true, the calling thread will block until the core thread finished executing
		 *									all currently queued commands. This is usually very expensive and should only be
		 *									used in performance non-critical code.
		 */
		void submitToCoreThread(bool blockUntilComplete = false);

		/** Cancels all commands in the queue. */
		void cancelAll();

	private:
		CommandQueueBase* mCommandQueue;
	};

	/**
	 * Core thread accessor allows you to schedule core commands outside of the core thread. 
	 * 			
	 * @note	Queued commands are only executed after the call to submitToCoreThread(), in the order they were submitted.
	 */
	template <class CommandQueueSyncPolicy = CommandQueueNoSync>
	class BS_CORE_EXPORT CoreThreadAccessor : public CoreThreadAccessorBase
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param[in]	threadId		Identifier for the thread that created the accessor.
		 */
		CoreThreadAccessor(ThreadId threadId)
			:CoreThreadAccessorBase(bs_new<CommandQueue<CommandQueueSyncPolicy>>(threadId))
		{

		}
	};

	/** @} */
}