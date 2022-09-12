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
	static void DeleteBuffer(HardwareBuffer* buffer)
	{
		bs_pool_delete(static_cast<VulkanHardwareBuffer*>(buffer));
	}

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

				VulkanBuffer* buffer = static_cast<VulkanHardwareBuffer*>(mBuffer)->GetResource(i);
				buffer->FreeView(mBufferViews[i]);
			}
		}

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuBuffer);
	}

	void VulkanGpuBuffer::Initialize()
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);

		const GpuBufferProperties& props = getProperties();
		mBufferDeleter = &deleteBuffer;

		// Create a new buffer if external buffer is not provided
		if(!mBuffer)
		{
			VulkanHardwareBuffer::BufferType bufferType;
			if (props.GetType() == GBT_STRUCTURED)
				bufferType = VulkanHardwareBuffer::BT_STRUCTURED;
			else
				bufferType = VulkanHardwareBuffer::BT_GENERIC;

			UINT32 size = props.GetElementCount() * props.getElementSize();
			mBuffer = bs_pool_new<VulkanHardwareBuffer>(bufferType, props.GetFormat(), props.getUsage(), size, mDeviceMask);
		}

		updateViews();

		GpuBuffer::initialize();
	}

	void* VulkanGpuBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
		void* data = GpuBuffer::map(offset, length, options, deviceIdx, queueIdx);
		updateViews();

		return data;
	}

	void VulkanGpuBuffer::Unmap()
	{
		GpuBuffer::unmap();
		updateViews();
	}

	void VulkanGpuBuffer::ReadData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		GpuBuffer::readData(offset, length, dest, deviceIdx, queueIdx);
		updateViews();
	}

	void VulkanGpuBuffer::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags,
		UINT32 queueIdx)
	{
		GpuBuffer::writeData(offset, length, source, writeFlags, queueIdx);
		updateViews();
	}

	VulkanBuffer* VulkanGpuBuffer::getResource(UINT32 deviceIdx) const
	{
		return static_cast<VulkanHardwareBuffer*>(mBuffer)->GetResource(deviceIdx);
	}

	VkBufferView VulkanGpuBuffer::GetView(UINT32 deviceIdx) const
	{
		return mBufferViews[deviceIdx];
	}

	void VulkanGpuBuffer::UpdateViews()
	{
		if(mProperties.GetType() == GBT_STRUCTURED)
			return;

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			VulkanBuffer* buffer = static_cast<VulkanHardwareBuffer*>(mBuffer)->GetResource(i);

			VkBuffer newBufferHandle = VK_NULL_HANDLE;

			if(buffer)
				newBufferHandle = buffer->GetHandle();

			if (mCachedBuffers[i] != newBufferHandle)
			{
				if(newBufferHandle != VK_NULL_HANDLE)
					mBufferViews[i] = buffer->GetView(VulkanUtility::getBufferFormat(mProperties.GetFormat()));
				else
					mBufferViews[i] = VK_NULL_HANDLE;

				mCachedBuffers[i] = newBufferHandle;
			}
		}
	}
}}
