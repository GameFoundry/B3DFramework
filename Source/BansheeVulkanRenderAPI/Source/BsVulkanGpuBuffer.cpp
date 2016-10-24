//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGpuBuffer.h"
#include "BsRenderStats.h"
#include "BsException.h"

namespace BansheeEngine
{
	VulkanGpuBufferCore::VulkanGpuBufferCore(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuBufferCore(desc, deviceMask)
	{
		if (desc.type != GBT_STANDARD)
			assert(desc.format == BF_UNKNOWN && "Format must be set to BF_UNKNOWN when using non-standard buffers");
		else
			assert(desc.elementSize == 0 && "No element size can be provided for standard buffer. Size is determined from format.");
	}

	VulkanGpuBufferCore::~VulkanGpuBufferCore()
	{ 
		clearBufferViews();
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuBuffer);
	}

	void VulkanGpuBufferCore::initialize()
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);

		GpuBufferCore::initialize();
	}

	void* VulkanGpuBufferCore::lock(UINT32 offset, UINT32 length, GpuLockOptions options)
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

	void VulkanGpuBufferCore::readData(UINT32 offset, UINT32 length, void* pDest)
	{
		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);
	}

	void VulkanGpuBufferCore::writeData(UINT32 offset, UINT32 length, const void* pSource, BufferWriteType writeFlags)
	{
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);
	}

	void VulkanGpuBufferCore::copyData(GpuBufferCore& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer)
	{

	}

	GpuBufferView* VulkanGpuBufferCore::createView()
	{
		// Not used
		return nullptr;
	}

	void VulkanGpuBufferCore::destroyView(GpuBufferView* view)
	{
		// Not used
	}
}