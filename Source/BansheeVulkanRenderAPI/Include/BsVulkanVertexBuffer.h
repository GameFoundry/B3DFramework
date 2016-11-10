//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVertexBuffer.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Vulkan implementation of a vertex buffer. */
	class VulkanVertexBufferCore : public VertexBufferCore
	{
	public:
		VulkanVertexBufferCore(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		~VulkanVertexBufferCore();

		/** @copydoc VertexBufferCore::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 queueIdx = 1) override;

		/** @copydoc VertexBufferCore::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 1) override;

		/** @copydoc VertexBufferCore::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false, UINT32 queueIdx = 1) override;

	protected: 
		/** @copydoc VertexBufferCore::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 queueIdx) override;

		/** @copydoc VertexBufferCore::unmap */
		void unmap(void) override;

		/** @copydoc VertexBufferCore::initialize */
		void initialize() override;
	};

	/** @} */
}