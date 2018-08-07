//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsVertexBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Vulkan implementation of a vertex buffer. */
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		~VulkanVertexBuffer();

		/** @copydoc VertexBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc VertexBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 0) override;

		/** @copydoc VertexBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** 
		 * Gets the resource wrapping the buffer object, on the specified device. If GPU param block buffer's device mask
		 * doesn't include the provided device, null is returned. 
		 */
		VulkanBuffer* getResource(UINT32 deviceIdx) const;

	protected: 
		/** @copydoc VertexBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc VertexBuffer::unmap */
		void unmap() override;

		/** @copydoc VertexBuffer::initialize */
		void initialize() override;

	private:
		VulkanHardwareBuffer* mBuffer = nullptr;
		GpuBufferUsage mUsage;
		GpuDeviceFlags mDeviceMask;
	};

	/** @} */
}}