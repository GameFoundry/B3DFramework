//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanHardwareBuffer.h"
#include "BsException.h"

namespace BansheeEngine
{
	VulkanHardwareBuffer::VulkanHardwareBuffer(GpuBufferUsage usage, UINT32 elementCount, UINT32 elementSize,
		bool useSystemMem, bool streamOut, bool randomGpuWrite, bool useCounter)
		: HardwareBuffer(usage, useSystemMem), mElementCount(elementCount), mElementSize(elementSize)
		, mRandomGpuWrite(randomGpuWrite), mUseCounter(useCounter)
	{
		
	}

	VulkanHardwareBuffer::~VulkanHardwareBuffer()
	{

	}

	void* VulkanHardwareBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options)
	{
		if (length > mSizeInBytes)
			BS_EXCEPT(RenderingAPIException, "Provided length " + toString(length) + " larger than the buffer " + toString(mSizeInBytes) + ".");		

		return nullptr;
	}

	void VulkanHardwareBuffer::unmap()
	{

	}

	void VulkanHardwareBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer)
	{

	}

	void VulkanHardwareBuffer::readData(UINT32 offset, UINT32 length, void* pDest)
	{
		
	}

	void VulkanHardwareBuffer::writeData(UINT32 offset, UINT32 length, const void* pSource, BufferWriteType writeFlags)
	{
		
	}
}