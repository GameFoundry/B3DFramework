//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanHardwareBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsException.h"

namespace BansheeEngine
{
	VulkanHardwareBuffer::VulkanHardwareBuffer(GpuBufferUsage usage, const VkMemoryRequirements& reqs, bool useSystemMem, 
		GpuDeviceFlags deviceMask)
		: HardwareBuffer(usage, useSystemMem), mAllocations{}
	{
		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());

		UINT32 deviceIdx = 0;
		if(deviceMask == GDF_DEFAULT)
		{
			const Vector<SPtr<VulkanDevice>>& devices = rapi._getPrimaryDevices();
			for (auto& device : devices)
				mAllocations[deviceIdx++].device = device;
		}
		else
		{
			for(UINT32 i = 0; i < BS_MAX_DEVICES; i++)
			{
				if ((1 << i) & deviceMask)
					mAllocations[deviceIdx++].device = rapi._getDevice(i);
			}
		}

		VkMemoryPropertyFlags flags = useSystemMem ?
			(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) : // Note: Try using cached uncoherent memory
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mAllocations[i].device == nullptr)
				break;

			mAllocations[i].memory = mAllocations[i].device->allocateMemory(reqs, flags);
		}

		mSizeInBytes = reqs.size;
	}

	VulkanHardwareBuffer::~VulkanHardwareBuffer()
	{
		for(auto& alloc : mAllocations)
			alloc.device->freeMemory(alloc.memory);
	}

	void* VulkanHardwareBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 syncMask)
	{
		if ((offset + length) > mSizeInBytes)
			BS_EXCEPT(RenderingAPIException, "Provided offset(" + toString(offset) + ") + length(" + toString(length) + ") "
				"is larger than the buffer " + toString(mSizeInBytes) + ".");		

		switch (options)
		{
		case GBL_WRITE_ONLY_DISCARD:

			break;
		case GBL_WRITE_ONLY_NO_OVERWRITE:

			break;
		case GBL_WRITE_ONLY:

			break;
		case GBL_READ_WRITE:

			break;
		case GBL_READ_ONLY:

			break;
		}

		return nullptr;
	}

	void VulkanHardwareBuffer::unmap()
	{

	}

	void VulkanHardwareBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer, UINT32 syncMask)
	{

	}

	void VulkanHardwareBuffer::readData(UINT32 offset, UINT32 length, void* pDest, UINT32 syncMask)
	{
		
	}

	void VulkanHardwareBuffer::writeData(UINT32 offset, UINT32 length, const void* pSource, BufferWriteType writeFlags, UINT32 syncMask)
	{
		
	}
}