//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanIndexBuffer.h"
#include "BsRenderStats.h"

namespace BansheeEngine
{
	VulkanIndexBufferCore::VulkanIndexBufferCore(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:IndexBufferCore(desc, deviceMask)
	{
		
	}

	VulkanIndexBufferCore::~VulkanIndexBufferCore()
	{
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_IndexBuffer);
	}

	void VulkanIndexBufferCore::initialize()
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_IndexBuffer);
		IndexBufferCore::initialize();
	}

	void* VulkanIndexBufferCore::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 syncMask)
	{
#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD 
			|| options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
		}
#endif

		return nullptr;
	}

	void VulkanIndexBufferCore::unmap()
	{

	}

	void VulkanIndexBufferCore::readData(UINT32 offset, UINT32 length, void* pDest, UINT32 syncMask)
	{
		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
	}

	void VulkanIndexBufferCore::writeData(UINT32 offset, UINT32 length, const void* pSource, BufferWriteType writeFlags, UINT32 syncMask)
	{
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
	}

	void VulkanIndexBufferCore::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer, UINT32 syncMask)
	{

	}
}