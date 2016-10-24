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
		void readData(UINT32 offset, UINT32 length, void* dest) override;

		/** @copydoc VertexBufferCore::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BufferWriteType::Normal) override;

		/** @copydoc VertexBufferCore::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false) override;

	protected: 
		/** @copydoc VertexBufferCore::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options) override;

		/** @copydoc VertexBufferCore::unmap */
		void unmap(void) override;

		/** @copydoc VertexBufferCore::initialize */
		void initialize() override;
	};

	/** @} */
}