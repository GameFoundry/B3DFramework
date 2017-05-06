//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "BsRenderStats.h"
#include "BsException.h"

namespace bs { namespace ct
{
	VulkanGpuBuffer::VulkanGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuBuffer(desc, deviceMask), mBuffer(nullptr), mDeviceMask(deviceMask)
	{
		if (desc.type != GBT_STANDARD)
			assert(desc.format == BF_UNKNOWN && "Format must be set to BF_UNKNOWN when using non-standard buffers");
		else
			assert(desc.elementSize == 0 && "No element size can be provided for standard buffer. Size is determined from format.");
	}

	VulkanGpuBuffer::~VulkanGpuBuffer()
	{ 
		if (mBuffer != nullptr)
			bs_delete(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuBuffer);
	}

	void VulkanGpuBuffer::initialize()
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);

		const GpuBufferProperties& props = getProperties();

		VulkanHardwareBuffer::BufferType bufferType;
		if (props.getType() == GBT_STRUCTURED)
			bufferType = VulkanHardwareBuffer::BT_STRUCTURED;
		else
		{
			if (props.getRandomGpuWrite())
				bufferType = VulkanHardwareBuffer::BT_STORAGE;
			else
				bufferType = VulkanHardwareBuffer::BT_GENERIC;
		}

		UINT32 size = props.getElementCount() * props.getElementSize();;
		mBuffer = bs_new<VulkanHardwareBuffer>(bufferType, props.getFormat(), props.getUsage(), size, mDeviceMask);

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

		return mBuffer->lock(offset, length, options, deviceIdx, queueIdx);
	}

	void VulkanGpuBuffer::unlock()
	{
		mBuffer->unlock();
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

	void VulkanGpuBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer, UINT32 queueIdx)
	{
		mBuffer->copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer, queueIdx);
	}

	VulkanBuffer* VulkanGpuBuffer::getResource(UINT32 deviceIdx) const
	{
		return mBuffer->getResource(deviceIdx);
	}
}}