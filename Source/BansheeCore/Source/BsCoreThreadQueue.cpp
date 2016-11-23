//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCoreThreadQueue.h"
#include "BsCommandQueue.h"
#include "BsMaterial.h"
#include "BsCoreThread.h"

namespace bs
{
	CoreThreadQueueBase::CoreThreadQueueBase(CommandQueueBase* commandQueue)
		:mCommandQueue(commandQueue)
	{

	}

	CoreThreadQueueBase::~CoreThreadQueueBase()
	{
		bs_delete(mCommandQueue);
	}

	AsyncOp CoreThreadQueueBase::queueReturnCommand(std::function<void(AsyncOp&)> commandCallback)
	{
		return mCommandQueue->queueReturn(commandCallback);
	}

	void CoreThreadQueueBase::queueCommand(std::function<void()> commandCallback)
	{
		mCommandQueue->queue(commandCallback);
	}

	void CoreThreadQueueBase::submitToCoreThread(bool blockUntilComplete)
	{
		Queue<QueuedCommand>* commands = mCommandQueue->flush();

		gCoreThread().queueCommand(std::bind(&CommandQueueBase::playback, mCommandQueue, commands), 
			CTQF_InternalQueue | CTQF_BlockUntilComplete);
	}

	void CoreThreadQueueBase::cancelAll()
	{
		// Note that this won't free any Frame data allocated for all the canceled commands since
		// frame data will only get cleared at frame start
		mCommandQueue->cancelAll();
	}
}