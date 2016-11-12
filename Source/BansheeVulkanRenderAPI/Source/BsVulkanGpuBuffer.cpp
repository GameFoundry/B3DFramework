//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "BsRenderStats.h"
#include "BsException.h"

namespace BansheeEngine
{
	VulkanGpuBufferCore::VulkanGpuBufferCore(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuBufferCore(desc, deviceMask), mBuffer(nullptr), mDeviceMask(deviceMask)
	{
		if (desc.type != GBT_STANDARD)
			assert(desc.format == BF_UNKNOWN && "Format must be set to BF_UNKNOWN when using non-standard buffers");
		else
			assert(desc.elementSize == 0 && "No element size can be provided for standard buffer. Size is determined from format.");
	}

	VulkanGpuBufferCore::~VulkanGpuBufferCore()
	{ 
		if (mBuffer != nullptr)
			bs_delete(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuBuffer);
	}

	void VulkanGpuBufferCore::initialize()
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);

		const GpuBufferProperties& props = getProperties();

		VulkanHardwareBuffer::BufferType bufferType;
		if (props.getRandomGpuWrite())
			bufferType = VulkanHardwareBuffer::BT_STORAGE;
		else
			bufferType = VulkanHardwareBuffer::BT_GENERIC;;

		UINT32 size = props.getElementCount() * props.getElementSize();;
		mBuffer = bs_new<VulkanHardwareBuffer>(bufferType, props.getFormat(), props.getUsage(), size, mDeviceMask);

		GpuBufferCore::initialize();
	}

	void* VulkanGpuBufferCore::lock(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
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

		return nullptr;;
	}

	void VulkanGpuBufferCore::unlock()
	{
		
	}

	void VulkanGpuBufferCore::readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);
	}

	void VulkanGpuBufferCore::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags,
										UINT32 queueIdx)
	{
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);
	}

	void VulkanGpuBufferCore::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer, UINT32 queueIdx)
	{

	}

	VulkanBuffer* VulkanGpuBufferCore::getResource(UINT32 deviceIdx) const
	{
		return mBuffer->getResource(deviceIdx);
	}
}