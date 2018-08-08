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

	VulkanGpuBuffer::VulkanGpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
		: GpuBuffer(desc, std::move(underlyingBuffer))
	{ }

	VulkanGpuBuffer::~VulkanGpuBuffer()
	{ 
		if (mBuffer)
		{
			for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
			{
				if (mBufferViews[i] == VK_NULL_HANDLE)
					continue;

				VulkanBuffer* buffer = static_cast<VulkanHardwareBuffer*>(mBuffer)->getResource(i);
				vkDestroyBufferView(buffer->getDevice().getLogical(), mBufferViews[i], gVulkanAllocator);
			}

			if(!mExternalBuffer)
				bs_pool_delete(static_cast<VulkanHardwareBuffer*>(mBuffer));
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

	void* VulkanGpuBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
		void* data = GpuBuffer::map(offset, length, options, deviceIdx, queueIdx);
		updateViews();

		return data;
	}

	void VulkanGpuBuffer::unmap()
	{
		GpuBuffer::unmap();
		updateViews();
	}

	VulkanBuffer* VulkanGpuBuffer::getResource(UINT32 deviceIdx) const
	{
		return static_cast<VulkanHardwareBuffer*>(mBuffer)->getResource(deviceIdx);
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
			VulkanBuffer* buffer = static_cast<VulkanHardwareBuffer*>(mBuffer)->getResource(i);

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
