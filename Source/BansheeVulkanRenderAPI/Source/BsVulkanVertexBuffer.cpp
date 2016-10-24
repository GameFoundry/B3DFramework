//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanVertexBuffer.h"
#include "BsRenderStats.h"

namespace BansheeEngine
{
	VulkanVertexBufferCore::VulkanVertexBufferCore(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:VertexBufferCore(desc, deviceMask)
	{ }

	VulkanVertexBufferCore::~VulkanVertexBufferCore()
	{
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_VertexBuffer);
	}

	void* VulkanVertexBufferCore::map(UINT32 offset, UINT32 length, GpuLockOptions options)
	{
#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_VertexBuffer);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_VertexBuffer);
		}
#endif

		return nullptr;
	}

	void VulkanVertexBufferCore::unmap()
	{

	}

	void VulkanVertexBufferCore::readData(UINT32 offset, UINT32 length, void* dest)
	{
		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_VertexBuffer);
	}

	void VulkanVertexBufferCore::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags)
	{
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_VertexBuffer);
	}

	void VulkanVertexBufferCore::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer)
	{

	}

	void VulkanVertexBufferCore::initialize()
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_VertexBuffer);
		VertexBufferCore::initialize();
	}
}