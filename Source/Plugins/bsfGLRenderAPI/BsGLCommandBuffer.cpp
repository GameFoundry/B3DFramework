//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLCommandBuffer.h"

namespace bs { namespace ct
{
	GLCommandBuffer::GLCommandBuffer(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx, bool secondary)
		: CommandBuffer(type, deviceIdx, queueIdx, secondary)
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
}}