//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanIndexBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	VulkanIndexBuffer::VulkanIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:IndexBuffer(desc, deviceMask),  mUsage(desc.usage), mDeviceMask(deviceMask)
	{ }

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		if (mBuffer != nullptr)
			bs_pool_delete(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_IndexBuffer);
	}

	void VulkanIndexBuffer::initialize()
	{
		mBuffer = bs_pool_new<VulkanHardwareBuffer>(VulkanHardwareBuffer::BT_INDEX, BF_UNKNOWN, mUsage, mSize, mDeviceMask);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_IndexBuffer);
		IndexBuffer::initialize();
	}

	void* VulkanIndexBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
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

	void VulkanIndexBuffer::unmap()
	{
		mBuffer->unlock();
	}

	void VulkanIndexBuffer::readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		mBuffer->readData(offset, length, dest, deviceIdx, queueIdx);

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
	}

	void VulkanIndexBuffer::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags, UINT32 queueIdx)
	{
		mBuffer->writeData(offset, length, source, writeFlags, queueIdx);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
	}

	void VulkanIndexBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
		bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		mBuffer->copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer, commandBuffer);
	}

	VulkanBuffer* VulkanIndexBuffer::getResource(UINT32 deviceIdx) const
	{
		return mBuffer->getResource(deviceIdx);
	}
}}