//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsCommandBuffer.h"
#include "Managers/BsCommandBufferManager.h"

namespace bs { namespace ct
{
	void CommandSyncMask::AddDependency(const SPtr<CommandBuffer>& buffer)
	{
		if (buffer == nullptr)
			return;

		mMask |= getGlobalQueueMask(buffer->getType(), buffer->getQueueIdx());
	}

	UINT32 CommandSyncMask::GetGlobalQueueMask(GpuQueueType type, UINT32 queueIdx)
	{
		UINT32 bitShift = 0;
		switch (type)
		{
		case GQT_GRAPHICS:
			break;
		case GQT_COMPUTE:
			bitShift = 8;
			break;
		case GQT_UPLOAD:
			bitShift = 16;
			break;
		default:
			break;
		}

		return (1 << queueIdx) << bitShift;
	}

	UINT32 CommandSyncMask::GetGlobalQueueIdx(GpuQueueType type, UINT32 queueIdx)
	{
		switch (type)
		{
		case GQT_COMPUTE:
			return 8 + queueIdx;
		case GQT_UPLOAD:
			return 16 + queueIdx;
		default:
			return queueIdx;
		}
	}

	UINT32 CommandSyncMask::GetQueueIdxAndType(UINT32 globalQueueIdx, GpuQueueType& type)
	{
		if(globalQueueIdx >= 16)
		{
			type = GQT_UPLOAD;
			return globalQueueIdx - 16;
		}

		if(globalQueueIdx >= 8)
		{
			type = GQT_COMPUTE;
			return globalQueueIdx - 8;
		}

		type = GQT_GRAPHICS;
		return globalQueueIdx;
	}

	CommandBuffer::CommandBuffer(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx, bool secondary)
		:mType(type), mDeviceIdx(deviceIdx), mQueueIdx(queueIdx), mIsSecondary(secondary)
	{

	}

	SPtr<CommandBuffer> CommandBuffer::create(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx,
		bool secondary)
	{
		return CommandBufferManager::Instance().create(type, deviceIdx, queueIdx, secondary);
	}
}}
