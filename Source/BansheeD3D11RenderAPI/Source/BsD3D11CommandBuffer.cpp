//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11CommandBuffer.h"

namespace BansheeEngine
{
	D3D11CommandBuffer::D3D11CommandBuffer(CommandBufferType type, UINT32 deviceIdx, UINT32 syncMask, bool secondary)
		: CommandBuffer(type, syncMask, secondary), mDeviceIdx(deviceIdx), mActiveDrawOp(DOT_TRIANGLE_LIST)
	{
		if (deviceIdx != 0)
			BS_EXCEPT(InvalidParametersException, "Only a single device supported on DX11.");
	}

	void D3D11CommandBuffer::queueCommand(const std::function<void()> command)
	{
		mCommands.push_back(command);
	}

	void D3D11CommandBuffer::appendSecondary(const SPtr<D3D11CommandBuffer>& secondaryBuffer)
	{
#if BS_DEBUG_MODE
		if (!secondaryBuffer->mIsSecondary)
		{
			LOGERR("Cannot append a command buffer that is not secondary.");
			return;
		}

		if (mIsSecondary)
		{
			LOGERR("Cannot append a buffer to a secondary command buffer.");
			return;
		}
#endif

		for (auto& entry : secondaryBuffer->mCommands)
			mCommands.push_back(entry);
	}

	void D3D11CommandBuffer::executeCommands()
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

	void D3D11CommandBuffer::clear()
	{
		mCommands.clear();
	}
}