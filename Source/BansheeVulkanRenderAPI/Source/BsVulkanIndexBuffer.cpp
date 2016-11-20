//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanIndexBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "BsRenderStats.h"

namespace bs
{
	VulkanIndexBufferCore::VulkanIndexBufferCore(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:IndexBufferCore(desc, deviceMask), mBuffer(nullptr),  mUsage(desc.usage), mDeviceMask(deviceMask)
	{ }

	VulkanIndexBufferCore::~VulkanIndexBufferCore()
	{
		if (mBuffer != nullptr)
			bs_delete(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_IndexBuffer);
	}

	void VulkanIndexBufferCore::initialize()
	{
		mBuffer = bs_new<VulkanHardwareBuffer>(VulkanHardwareBuffer::BT_INDEX, BF_UNKNOWN, mUsage, mSize, mDeviceMask);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_IndexBuffer);
		IndexBufferCore::initialize();
	}

	void* VulkanIndexBufferCore::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
		}
#endif

		return mBuffer->lock(offset, length, options, deviceIdx, queueIdx);
	}

	void VulkanIndexBufferCore::unmap()
	{
		mBuffer->unlock();
	}

	void VulkanIndexBufferCore::readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		mBuffer->readData(offset, length, dest, deviceIdx, queueIdx);

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
	}

	void VulkanIndexBufferCore::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags, UINT32 queueIdx)
	{
		mBuffer->writeData(offset, length, source, writeFlags, queueIdx);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
	}

	void VulkanIndexBufferCore::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
										UINT32 dstOffset, UINT32 length, bool discardWholeBuffer, UINT32 queueIdx)
	{
		mBuffer->copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer, queueIdx);
	}

	VulkanBuffer* VulkanIndexBufferCore::getResource(UINT32 deviceIdx) const
	{
		return mBuffer->getResource(deviceIdx);
	}
}