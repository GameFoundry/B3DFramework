//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCoreThreadAccessor.h"
#include "BsCommandQueue.h"
#include "BsRenderAPI.h"
#include "BsBlendState.h"
#include "BsRasterizerState.h"
#include "BsDepthStencilState.h"
#include "BsGpuResourceData.h"
#include "BsIndexBuffer.h"
#include "BsVertexBuffer.h"
#include "BsVideoModeInfo.h"
#include "BsGpuParams.h"
#include "BsPass.h"
#include "BsMaterial.h"
#include "BsCoreThread.h"

namespace bs
{
	CoreThreadAccessorBase::CoreThreadAccessorBase(CommandQueueBase* commandQueue)
		:mCommandQueue(commandQueue)
	{

	}

	CoreThreadAccessorBase::~CoreThreadAccessorBase()
	{
		bs_delete(mCommandQueue);
	}

	AsyncOp CoreThreadAccessorBase::queueReturnCommand(std::function<void(AsyncOp&)> commandCallback)
	{
		return mCommandQueue->queueReturn(commandCallback);
	}

	void CoreThreadAccessorBase::queueCommand(std::function<void()> commandCallback)
	{
		mCommandQueue->queue(commandCallback);
	}

	void CoreThreadAccessorBase::submitToCoreThread(bool blockUntilComplete)
	{
		Queue<QueuedCommand>* commands = mCommandQueue->flush();

		gCoreThread().queueCommand(std::bind(&CommandQueueBase::playback, mCommandQueue, commands), blockUntilComplete);
	}

	void CoreThreadAccessorBase::cancelAll()
	{
		// Note that this won't free any Frame data allocated for all the canceled commands since
		// frame data will only get cleared at frame start
		mCommandQueue->cancelAll();
	}
}