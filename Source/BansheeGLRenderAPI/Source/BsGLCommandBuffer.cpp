//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLCommandBuffer.h"

namespace BansheeEngine
{
	GLCommandBuffer::GLCommandBuffer(CommandBufferType type, UINT32 deviceIdx, UINT32 syncMask, bool secondary)
		: CommandBuffer(type, syncMask, secondary), mDeviceIdx(deviceIdx), mCurrentDrawOperation(DOT_TRIANGLE_LIST)
	{
		if (deviceIdx != 0)
			BS_EXCEPT(InvalidParametersException, "Only a single device supported on DX11.");
	}

	void GLCommandBuffer::queueCommand(const std::function<void()> command)
	{
		mCommands.push_back(command);
	}

	void GLCommandBuffer::appendSecondary(const SPtr<GLCommandBuffer>& secondaryBuffer)
	{
#if BS_DEBUG_MODE
		if(!secondaryBuffer->mIsSecondary)
		{
			LOGERR("Cannot append a command buffer that is not secondary.");
			return;
		}

		if(mIsSecondary)
		{
			LOGERR("Cannot append a buffer to a secondary command buffer.");
			return;
		}
#endif

		for (auto& entry : secondaryBuffer->mCommands)
			mCommands.push_back(entry);
	}

	void GLCommandBuffer::executeCommands()
	{
#if BS_DEBUG_MODE
		if (mIsSecondary)
		{
			LOGERR("Cannot execute commands on a secondary buffer.");
			return;
		}
#endif

		for (auto& entry : mCommands)
			entry();
	}

	void GLCommandBuffer::clear()
	{
		mCommands.clear();
	}
}