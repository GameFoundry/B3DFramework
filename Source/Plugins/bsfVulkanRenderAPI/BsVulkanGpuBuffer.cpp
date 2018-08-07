//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "Profiling/BsRenderStats.h"
#include "Error/BsException.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"

namespace bs { namespace ct
{
	VulkanGpuBuffer::VulkanGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuBuffer(desc, deviceMask)
	{ }

	VulkanGpuBuffer::VulkanGpuBuffer(const GPU_BUFFER_DESC& desc, const SPtr<VulkanHardwareBuffer>& underlyingBuffer)
		: GpuBuffer(desc, underlyingBuffer), mBuffer(underlyingBuffer.get())
	{ }

	VulkanGpuBuffer::~VulkanGpuBuffer()
	{ 
		if (mBuffer)
		{
			for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
			{
				if (mBufferViews[i] == VK_NULL_HANDLE)
					continue;

				VulkanBuffer* buffer = mBuffer->getResource(i);
				vkDestroyBufferView(buffer->getDevice().getLogical(), mBufferViews[i], gVulkanAllocator);
			}

			if(!mExternalBuffer)
				bs_pool_delete(mBuffer);
		}

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuBuffer);
	}

	void VulkanGpuBuffer::initialize()
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);

		const GpuBufferProperties& props = getProperties();

		// Create a new buffer if external buffer is not provided
		if(!mBuffer)
		{
			VulkanHardwareBuffer::BufferType bufferType;
			if (props.getType() == GBT_STRUCTURED)
				bufferType = VulkanHardwareBuffer::BT_STRUCTURED;
			else
				bufferType = VulkanHardwareBuffer::BT_GENERIC;

			UINT32 size = props.getElementCount() * props.getElementSize();
			mBuffer = bs_pool_new<VulkanHardwareBuffer>(bufferType, props.getFormat(), props.getUsage(), size, mDeviceMask);
		}

		mViewCI.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		mViewCI.pNext = nullptr;
		mViewCI.flags = 0;
		mViewCI.format = VulkanUtility::getBufferFormat(props.getFormat());
		mViewCI.offset = 0;
		mViewCI.range = VK_WHOLE_SIZE;

		updateViews();

		GpuBuffer::initialize();
	}

	void* VulkanGpuBuffer::lock(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);
		}
#endif

		void* data = mBuffer->lock(offset, length, options, deviceIdx, queueIdx);
		updateViews();

		return data;
	}

	void VulkanGpuBuffer::unlock()
	{
		mBuffer->unlock();
		updateViews();
	}

	void VulkanGpuBuffer::readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		mBuffer->readData(offset, length, dest, deviceIdx, queueIdx);

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);
	}

	void VulkanGpuBuffer::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags,
										UINT32 queueIdx)
	{
		mBuffer->writeData(offset, length, source, writeFlags, queueIdx);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);
	}

	void VulkanGpuBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
		bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanGpuBuffer& vkSrcBuffer = static_cast<VulkanGpuBuffer&>(srcBuffer);

		mBuffer->copyData(*vkSrcBuffer.mBuffer, srcOffset, dstOffset, length, discardWholeBuffer, commandBuffer);
	}

	VulkanBuffer* VulkanGpuBuffer::getResource(UINT32 deviceIdx) const
	{
		return mBuffer->getResource(deviceIdx);
	}

	VkBufferView VulkanGpuBuffer::getView(UINT32 deviceIdx) const
	{
		return mBufferViews[deviceIdx];
	}

	void VulkanGpuBuffer::updateViews()
	{
		if(mProperties.getType() == GBT_STRUCTURED)
			return;

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			VulkanBuffer* buffer = mBuffer->getResource(i);

			VkBuffer newBufferHandle = VK_NULL_HANDLE;

			if(buffer)
				newBufferHandle = buffer->getHandle();

			if (mCachedBuffers[i] != newBufferHandle)
			{
				if (mBufferViews[i] != VK_NULL_HANDLE)
				{
					vkDestroyBufferView(buffer->getDevice().getLogical(), mBufferViews[i], gVulkanAllocator);
					mBufferViews[i] = VK_NULL_HANDLE;
				}

				if(newBufferHandle != VK_NULL_HANDLE)
				{
					mViewCI.buffer = newBufferHandle;

					VkResult result = vkCreateBufferView(buffer->getDevice().getLogical(), &mViewCI, gVulkanAllocator,
						&mBufferViews[i]);
					assert(result == VK_SUCCESS);
				}

				mCachedBuffers[i] = newBufferHandle;
			}
		}
	}
}}
